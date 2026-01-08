#ifndef SERVER_H
#define SERVER_H

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

	// Function to start the server
	EXPORT void init_waiting_struct_socket1();
	EXPORT void handle_Send_Receive(struct SINR_Values_Reward* Param1, struct gNB_Powers* Param2);
	EXPORT void send_SINRS_Rewards_at_Time_Step(struct SINR_Values_Reward* Var);
	//EXPORT void recv_gNB_Powers(struct gNB_Powers* Var);
	EXPORT void send_Random_SINRS_at_Episode_Start();
	//EXPORT void init_Debug_log();
	//EXPORT void close_Debug_log();
#ifdef __cplusplus
}
#endif

#endif  // SERVER_H
