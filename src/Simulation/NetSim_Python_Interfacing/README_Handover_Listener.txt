To run the realtime handover & ping-pong listener and plotter:

1. Ensure the simulator is configured to start the runtime listener on localhost:12349 (default). The code attempts to start py listener in `fn_NetSim_LTENR_GNBRRC_Init` using port 12349.

2. Start the Python listener before running the simulation (or let simulator accept connection):
   python NetSim_Python_Interfacing/handover_pingpong_listener.py --host 0.0.0.0 --port 12349 --log LTENR_Handover_PingPong_Log.csv

3. The simulator will send lines of form:
   HANDOVER,UE,<ue>,SERVING,<s>,TARGET,<t>,TIME,<time_ms>,SERVING_SINR,<sval>,TARGET_SINR,<tval>,REMARK,<remark>\n
   and ping-pong lines:
   PINGPONG,UE,<ue>,FROM,<s>,TO,<t>,DT,<delta>\n
4. The Python tool will append events to CSV and show a live plot of events per UE.

Notes:
- The C code writes `LTENR_Handover_Log.csv` in the working directory.
- The Python script produces `LTENR_Handover_PingPong_Log.csv` by default; you can change with --log.
- The runtime communication is best-effort: socket errors are ignored to avoid disturbing simulation.
