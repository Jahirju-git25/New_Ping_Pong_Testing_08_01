#pragma once
#ifndef AdvancedPlots_h
#define AdvancedPlots_h
#ifdef  __cplusplus
extern "C" {
#endif
	_declspec(dllexport) void fnNetSim_buffer_log_init();
	_declspec(dllexport) void fn_NetSim_buffer_log_close();
	_declspec(dllexport) void fnNetSim_buffer_log_write(char* format, ...);
	// Link Packet Logs
	_declspec(dllexport) void init_linkpacketlog();
	_declspec(dllexport) void LinkPacketLog();
	bool get_protocol_log_status(char* logname);
	_declspec(dllexport) void LinkPacketLog_close();

#ifdef  __cplusplus
}
#endif
#endif //AdvancedPlots_h
