#!/usr/bin/env python3
"""
Real-time handover & pingpong listener + plotter for NetSim.
- Listens on TCP (default 0.0.0.0:12349) for newline-delimited CSV/text lines sent by NetSim runtime.
- Supports messages starting with "HANDOVER" or "PINGPONG" (simple CSV/text format).
- Appends events to a CSV logfile `LTENR_Handover_PingPong_Log.csv` and shows a live matplotlib plot.

Usage: python handover_pingpong_listener.py [--host HOST] [--port PORT] [--log LOGFILE]
"""

import argparse
import socket
import threading
import queue
import csv
import time
import os
from collections import defaultdict

import matplotlib.pyplot as plt
import matplotlib.animation as animation


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
            # permissive parsing: look for UE,<val> and TIME,<val>
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
            # expected: PINGPONG,TIME,<t>,UE,<id>,FROM,<a>,TO,<b>,DT,<dt>,... or the Waiting_Struct_Listener_v2 format
            d = {'type': 'PINGPONG', 'time_ms': None, 'ue': None, 'serving': None, 'target': None, 'dt': None, 'remark': ''}
            i = 1
            while i + 1 <= len(parts):
                key = parts[i].upper() if i < len(parts) else ''
                val = parts[i+1] if i+1 < len(parts) else ''
                if key == 'TIME':
                    try:
                        # some messages use seconds, try to detect
                        t = float(val)
                        # treat as seconds if value small (<1e6), convert to ms
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
                    # ignore counts
                    i += 2
                else:
                    i += 1
            return d
        else:
            # try CSV row Time(ms),UE Id,Serving Cell,Target Cell,Serving SSB SNR(dB),Target SSB SNR(dB),Remarks
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
        self.q = queue.Queue()
        self.events = []
        self.lock = threading.Lock()
        self.ue_index = {}

    def push(self, ev):
        if not ev:
            return
        if not ev.get('ue'):
            ev['ue'] = '<unknown>'
        self.q.put(ev)

    def drain(self):
        changed = False
        while True:
            try:
                ev = self.q.get_nowait()
            except queue.Empty:
                break
            with self.lock:
                if ev['ue'] not in self.ue_index:
                    self.ue_index[ev['ue']] = len(self.ue_index)
                self.events.append(ev)
                changed = True
        return changed


def tcp_server(host, port, collector, stop_event, logfile):
    srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    srv.bind((host, port))
    srv.listen(1)
    srv.settimeout(1.0)
    print('Listening on %s:%d' % (host, port))
    conn = None
    try:
        while not stop_event.is_set():
            if conn is None:
                try:
                    conn, addr = srv.accept()
                    conn.settimeout(1.0)
                    print('Connected from', addr)
                except socket.timeout:
                    continue
            try:
                data = conn.recv(8192)
                if not data:
                    print('Connection closed by remote')
                    conn.close()
                    conn = None
                    continue
                text = data.decode('utf-8', errors='replace')
                for line in text.splitlines():
                    ev = parse_line(line)
                    if ev:
                        collector.push(ev)
                        # append to log file
                        try:
                            with open(logfile, 'a', encoding='utf-8') as f:
                                if ev['type'] == 'HANDOVER':
                                    f.write('{:.3f},HANDOVER,{},{},{},{}\n'.format(
                                        ev.get('time_ms') or time.time()*1000.0,
                                        ev.get('ue') or '', ev.get('serving') or '', ev.get('target') or '', ev.get('remark') or ''))
                                else:
                                    f.write('{:.3f},PINGPONG,{},{},{},DT={}\n'.format(
                                        ev.get('time_ms') or time.time()*1000.0,
                                        ev.get('ue') or '', ev.get('serving') or '', ev.get('target') or '', ev.get('dt') or ''))
                        except Exception:
                            pass
            except socket.timeout:
                continue
            except Exception as e:
                print('socket error', e)
                if conn:
                    try: conn.close()
                    except: pass
                conn = None
    finally:
        try: srv.close()
        except: pass


def start_plot(collector, stop_event, refresh_interval=1000):
    try:
        import seaborn as sns
        sns.set_style('darkgrid')
    except Exception:
        try:
            plt.style.use('seaborn-darkgrid')
        except Exception:
            plt.style.use('ggplot')

    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 6), gridspec_kw={'height_ratios': [3, 1]})

    def update(frame):
        collector.drain()
        with collector.lock:
            events = list(collector.events)
            ue_index = dict(collector.ue_index)
        if not events:
            return
        plotted = [e for e in events if e.get('time_ms') is not None]
        if plotted:
            xs = [e['time_ms'] for e in plotted]
            ys = [ue_index.get(e['ue'], 0) for e in plotted]
            colors = ['red' if e['type'] == 'PINGPONG' else 'blue' for e in plotted]
            markers = ['x' if e['type'] == 'PINGPONG' else 'o' for e in plotted]
            ax1.clear()
            for x, y, c, m, e in zip(xs, ys, colors, markers, plotted):
                ax1.scatter(x, y, c=c, marker=m, alpha=0.8, s=60)
                # optional label on hover? keep simple
            ax1.set_xlabel('Time (ms)')
            ax1.set_ylabel('UE')
            labels_order = [k for k, _ in sorted(ue_index.items(), key=lambda kv: kv[1])]
            ax1.set_yticks(list(range(len(labels_order))))
            ax1.set_yticklabels(labels_order)
            ax1.set_title('Handover & PingPong events timeline')

        counts = defaultdict(int)
        for e in events:
            counts[e['ue']] += 1
        ax2.clear()
        labels = list(counts.keys())
        vals = [counts[l] for l in labels]
        ax2.bar(labels, vals)
        ax2.set_ylabel('Events')
        ax2.set_title('Events per UE')
        plt.tight_layout()

    ani = animation.FuncAnimation(fig, update, interval=refresh_interval, cache_frame_data=False)
    try:
        plt.show()
    except KeyboardInterrupt:
        stop_event.set()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--host', default='0.0.0.0')
    parser.add_argument('--port', type=int, default=12349)
    parser.add_argument('--log', default='LTENR_Handover_PingPong_Log.csv')
    args = parser.parse_args()

    collector = Collector()
    stop_event = threading.Event()

    # ensure logfile header
    if not os.path.isfile(args.log):
        with open(args.log, 'w', encoding='utf-8') as f:
            f.write('Time(ms),Type,UE,Serving,Target,Remark\n')

    t = threading.Thread(target=tcp_server, args=(args.host, args.port, collector, stop_event, args.log), daemon=True)
    t.start()

    try:
        start_plot(collector, stop_event)
    finally:
        stop_event.set()
        time.sleep(0.2)


if __name__ == '__main__':
    main()
