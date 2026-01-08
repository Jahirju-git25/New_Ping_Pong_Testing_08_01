
"""
Real-time handover & pingpong listener (headless) for NetSim.
- Connects to simulator TCP (default 127.0.0.1:12349) for newline-delimited CSV/text lines sent by NetSim runtime.

Usage: python handover_pingpong_listener.py [--host HOST] [--port PORT] [--log LOGFILE]
"""

import argparse
import socket
import threading
import queue
import csv
import time
import os
from collections import defaultdict, deque


def parse_line(line):
    """Parse a single incoming line into a dict with keys:
    - type: 'HANDOVER' or 'PINGPONG'
    - time_ms (float) -- simulation time in ms when available (or wall-clock ms)
    - ue (str)
    - serving (str)
    - target (str)
    - remark (str)
    - dt (float) for pingpong delta time seconds
    """
    try:
        s = line.strip()
        if not s:
            return None
        parts = [p.strip() for p in s.split(',')]
        if parts[0].upper().startswith('HANDOVER'):
            d = {'type': 'HANDOVER', 'time_ms': None, 'ue': None, 'serving': None, 'target': None, 'remark': ''}
            i = 1
            while i + 1 <= len(parts):
                key = parts[i].upper() if i < len(parts) else ''
                val = parts[i+1] if i+1 < len(parts) else ''
                if key == 'UE':
                    d['ue'] = val
                    i += 2
                elif key == 'SERVING':
                    d['serving'] = val
                    i += 2
                elif key == 'TARGET':
                    d['target'] = val
                    i += 2
                elif key == 'TIME':
                    try:
                        d['time_ms'] = float(val)
                    except Exception:
                        d['time_ms'] = None
                    i += 2
                elif key == 'REMARK':
                    d['remark'] = ','.join(parts[i+1:])
                    break
                else:
                    i += 1
            return d
        elif parts[0].upper().startswith('PINGPONG'):
            d = {'type': 'PINGPONG', 'time_ms': None, 'ue': None, 'serving': None, 'target': None, 'dt': None, 'remark': ''}
            i = 1
            while i + 1 <= len(parts):
                key = parts[i].upper() if i < len(parts) else ''
                val = parts[i+1] if i+1 < len(parts) else ''
                if key == 'TIME':
                    try:
                        t = float(val)
                        if t < 1e6:
                            d['time_ms'] = t * 1000.0
                        else:
                            d['time_ms'] = float(val)
                    except Exception:
                        d['time_ms'] = None
                    i += 2
                elif key == 'UE':
                    d['ue'] = val
                    i += 2
                elif key == 'FROM' or key == 'FROM_CELL' or key == 'FROMCELL':
                    d['serving'] = val
                    i += 2
                elif key == 'TO' or key == 'TO_CELL' or key == 'TOCELL':
                    d['target'] = val
                    i += 2
                elif key == 'DT':
                    try:
                        d['dt'] = float(val)
                    except Exception:
                        d['dt'] = None
                    i += 2
                elif key == 'TOTAL_HO' or key == 'TOTAL_PP':
                    i += 2
                else:
                    i += 1
            return d
        else:
            reader = csv.reader([s])
            row = next(reader)
            if len(row) >= 4:
                return {
                    'type': 'HANDOVER',
                    'time_ms': float(row[0]) if row[0] else None,
                    'ue': row[1] if len(row) > 1 else None,
                    'serving': row[2] if len(row) > 2 else None,
                    'target': row[3] if len(row) > 3 else None,
                    'remark': row[6] if len(row) > 6 else ''
                }
    except Exception as e:
        print('parse error:', e, 'line:', line)
    return None


class Collector:
    def __init__(self):
        # queue is used by reader to push parsed events and by writer to pop and write immediately
        self.q = queue.Queue()
        # keep events in memory too (small overhead) for final write/inspection
        self.events = []
        self.lock = threading.Lock()
        self.ue_index = {}
        # deque of recent pingpong event timestamps (seconds since epoch)
        self.pp_times = deque()
        # whether we already alerted during current burst
        self.pp_alerted = False

    def push(self, ev):
        if not ev:
            return
        if not ev.get('ue'):
            ev['ue'] = '<unknown>'
        # push into queue for writer thread
        self.q.put(ev)
        # detect quick successive PINGPONG events within 3 seconds
        try:
            if ev.get('type') == 'PINGPONG':
                now = time.time()
                with self.lock:
                    self.pp_times.append(now)
                    # remove entries older than 3 seconds
                    cutoff = now - 3.0
                    while self.pp_times and self.pp_times[0] < cutoff:
                        self.pp_times.popleft()
                    if len(self.pp_times) >= 2 and not self.pp_alerted:
                        print('Pingpong detected')
                        self.pp_alerted = True
                    # reset alerted flag when burst subsides
                    if len(self.pp_times) < 2 and self.pp_alerted:
                        self.pp_alerted = False
        except Exception:
            pass

    def add_to_memory(self, ev):
        with self.lock:
            if ev['ue'] not in self.ue_index:
                self.ue_index[ev['ue']] = len(self.ue_index)
            self.events.append(ev)

    def drain_memory(self):
        with self.lock:
            return list(self.events)


def tcp_client_once(host, port, collector, stop_event):
    """Connect once to the simulator listener, read until remote closes or stop_event set, then return."""
    print('Attempting to connect to simulator at %s:%d' % (host, port))
    sock = None
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5.0)
        sock.connect((host, port))
        print('Connected to %s:%d' % (host, port))
        sock.settimeout(1.0)
        buf = b''
        while not stop_event.is_set():
            try:
                data = sock.recv(8192)
                if not data:
                    print('Connection closed by remote')
                    break
                # decode and split lines quickly
                try:
                    text = data.decode('utf-8', errors='replace')
                except Exception:
                    text = data.decode('latin1', errors='replace')
                for raw_line in text.splitlines():
                    ev = parse_line(raw_line)
                    if ev:
                        collector.push(ev)
            except socket.timeout:
                continue
            except Exception as e:
                print('recv error', e)
                break
    except Exception as e:
        msg = str(e)
        if '10049' in msg or 'address is not valid' in msg.lower():
            print('connection error', e, "-- invalid remote address. Ensure you passed the server's reachable IP (not 0.0.0.0).")
        else:
            print('connection error', e)
    finally:
        if sock:
            try:
                sock.close()
            except Exception:
                pass


def writer_thread(logfile, collector, stop_event, flush_interval=1.0):
    """Consume events from collector.q and append to logfile immediately to avoid backpressure.
    Also populate in-memory events list for post-mortem.
    """
    # ensure directory exists
    logdir = os.path.dirname(os.path.abspath(logfile))
    if logdir and not os.path.exists(logdir):
        try:
            os.makedirs(logdir, exist_ok=True)
        except Exception:
            pass

    need_header = not os.path.isfile(logfile)
    try:
        with open(logfile, 'a', encoding='utf-8', buffering=1) as f:  # line buffered
            if need_header:
                f.write('Time(ms),Type,UE,Serving,Target,Remark\n')
                f.flush()
            last_flush = time.time()
            while not stop_event.is_set() or not collector.q.empty():
                try:
                    ev = collector.q.get(timeout=0.5)
                except queue.Empty:
                    # periodically flush if needed
                    if time.time() - last_flush >= flush_interval:
                        try:
                            f.flush()
                        except Exception:
                            pass
                        last_flush = time.time()
                    continue
                # write line
                t = ev.get('time_ms') or (time.time() * 1000.0)
                if ev.get('type') == 'HANDOVER':
                    line = '{:.3f},HANDOVER,{},{},{},{}\n'.format(t, ev.get('ue') or '', ev.get('serving') or '', ev.get('target') or '', ev.get('remark') or '')
                else:
                    remark = ''
                    if ev.get('dt') is not None:
                        remark = 'DT={}'.format(ev.get('dt'))
                    line = '{:.3f},PINGPONG,{},{},{},{}\n'.format(t, ev.get('ue') or '', ev.get('serving') or '', ev.get('target') or '', remark)
                try:
                    f.write(line)
                except Exception:
                    # ignore write errors but continue draining
                    pass
                # also keep in memory
                collector.add_to_memory(ev)
                # immediate flush to reduce simulator backpressure risk
                try:
                    f.flush()
                except Exception:
                    pass
            # final flush
            try:
                f.flush()
            except Exception:
                pass
    except Exception as e:
        print('Writer thread failed to open/write logfile:', e)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--host', default='127.0.0.1')
    parser.add_argument('--port', type=int, default=12349)
    parser.add_argument('--log', default='LTENR_Handover_PingPong_Log.csv')
    args = parser.parse_args()

    if args.host == '0.0.0.0' or args.host == '::':
        print("Warning: '0.0.0.0' or '::' is not a valid remote address to connect to. Use the server's IP (e.g. 127.0.0.1 or 192.168.x.x). Falling back to 127.0.0.1.")
        args.host = '127.0.0.1'

    collector = Collector()
    stop_event = threading.Event()

    # start writer thread first so reader can push quickly without waiting
    writer = threading.Thread(target=writer_thread, args=(args.log, collector, stop_event), daemon=True)
    writer.start()

    # start tcp reader
    reader = threading.Thread(target=tcp_client_once, args=(args.host, args.port, collector, stop_event))
    reader.start()

    try:
        while reader.is_alive():
            reader.join(timeout=0.5)
    except KeyboardInterrupt:
        print('Interrupted by user, shutting down...')
        stop_event.set()
        reader.join()

    # signal writer to finish and wait
    stop_event.set()
    writer.join(timeout=5.0)

    # final report
    print('Listener finished. Log file at:', os.path.abspath(args.log))


if __name__ == '__main__':
    main()
