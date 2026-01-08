/************************************************************************************
* Copyright (C) 2023																*
* TETCOS, Bangalore. India															*
*																					*
* Tetcos owns the intellectual property rights in the Product and its content.		*
* The copying, redistribution, reselling or publication of any or all of the		*
* Product or its content without express prior written consent of Tetcos is			*
* prohibited. Ownership and / or any other right relating to the software and all	*
* intellectual property rights therein shall remain at all times with Tetcos.		*
*																					*
* This source code is licensed per the NetSim license agreement.					*
*																					*
* No portion of this source code may be used as the basis for a derivative work,	*
* or used, for any purpose other than its intended use per the NetSim license		*
* agreement.																		*
*																					*
* This source code and the algorithms contained within it are confidential trade	*
* secrets of TETCOS and may not be used as the basis for any other software,		*
* hardware, product or service.														*
*																					*
* Author:    Shashi Kant Suman	                                                    *
*										                                            *
* ----------------------------------------------------------------------------------*/
#ifndef _NETSIM_APPLICATION_H_
#define _NETSIM_APPLICATION_H_

//#define AES_ENCRYPT
//#define DES_ENCRYPT

#include "main.h"
#include "Stack.h"
#define roundoff(d) ((long)(d+0.5))
#define MAX_TTL 255
#define MAX_PORT 65535

#define VANET_APP_RANDOM_WAIT_TIME_DEFAULT	5*MILLISECOND // SAE j2735
#define RANDOM_STARTUP_DELAY				0.1*SECOND

unsigned int nApplicationCount;

typedef struct stru_Application_DataInfo APP_DATA_INFO;
typedef struct stru_Application_VoiceInfo APP_VOICE_INFO;
typedef struct stru_Application_VideoInfo APP_VIDEO_INFO;
typedef struct stru_Application_HTTPInfo APP_HTTP_INFO;
typedef struct stru_Application_InteractiveGamingInfo APP_GAMING_INFO;//Interactive Gaming
typedef struct stru_Application_EMAILInfo APP_EMAIL_INFO;
typedef struct stru_Application_P2PInfo APP_P2P_INFO;
typedef struct stru_Application_CallInfo APP_CALL_INFO;
typedef	struct stru_Application_EmulationInfo APP_EMULATION_INFO;
typedef struct stru_Application_COAPInfo APP_COAP_INFO;
/// Enumeration for application events
typedef enum
{
	event_APP_START = PROTOCOL_APPLICATION * 100 + 1,
	event_APP_END,
	event_APP_RESTART,
}APP_EVENT;

typedef enum
{
	Encryption_None,
	Encryption_XOR,
	Encryption_TEA,
	Encryption_AES,
	Encryption_DES,
}ENCRYPTION;

/// Enumeration for application control packets
typedef enum
{
	packet_HTTP_REQUEST = PROTOCOL_APPLICATION * 100 + 1,
	packet_COAP_REQUEST = PROTOCOL_APPLICATION * 100 + 2,
}APP_CONTROL_PACKET;

/// Enumeration for application state.
typedef enum
{
	appState_Started,
	appState_Paused,
	appState_Terminated,
}APP_STATE;

/// Structure to store socket information
struct stru_SocketInfo
{
	NETSIM_ID nSourceId;
	UINT destCount;
	NETSIM_ID* nDestinationId;
	NETSIM_ID nSourcePort;
	NETSIM_ID nDestPort;
	ptrSOCKETINTERFACE s;
	_ele* ele;
};
#define SOCKETINFO_ALLOC() (struct stru_SocketInfo*)list_alloc(sizeof(struct stru_SocketInfo),offsetof(struct stru_SocketInfo,ele))
/// Structure to store application information
typedef struct stru_Application_Info
{
	//config variable
	NETSIM_ID id;
	NETSIM_ID nConfigId;
	TRANSMISSION_TYPE nTransmissionType;
	APPLICATION_TYPE nAppType;
	unsigned int nSourceCount;
	unsigned int nDestCount;
	NETSIM_ID* sourceList;
	NETSIM_ID* destList;
	double dStartTime;
	double dEndTime;
	double* dGenerationRate;
	char* name;
	QUALITY_OF_SERVICE qos;
	TRANSPORT_LAYER_PROTOCOL trxProtocol;
	APPLICATION_LAYER_PROTOCOL protocol;
	NETSIM_ID applicationInstanceCount;

	unsigned long long int nPacketId;
	UINT16 sourcePort;
	UINT16 destPort;
	APP_STATE nAppState;
	void* appMetrics;
	void* appData;

	ENCRYPTION encryption;
	struct stru_SocketInfo* socketInfo;

	//Broadcast Application
	NETSIM_ID* recvList;
	unsigned int* recvPort;

	//Startup delay
	bool isStartupDelay;

	//Application Packet Logs
	struct stru_application
	{
		double prevPacketLatency;
		double prevStartTime;
		double prevEndTime;
		UINT64 firstPacketReceived;
	}uplink, downlink;

	// Application Generation Logs 
	struct stru_GenerationLogs
	{
		double lastPacketTime;
		double lastPacketSize;
		double prevLastPacketTime;
	}ul, dl;

	//Multicast application
	NETSIM_IPAddress multicastDestIP;
}APPLICATION_INFO, * ptrAPPLICATION_INFO;
ptrAPPLICATION_INFO* applicationInfo;

/// Structure for Data information such as packet size and inter arrival time,this is applicable for custom,FTP,Database Traffic
struct stru_Application_DataInfo
{
	DISTRIBUTION packetSizeDistribution;
	double* dPacketSize;
	DISTRIBUTION IATDistribution;
	double* dIAT;
};

/// Structure for Data information such as packet size and inter arrival time,this is applicable for Vanet application
typedef struct stru_Application_BSMInfo
{
	DISTRIBUTION packetSizeDistribution;
	double* dPacketSize;
	DISTRIBUTION IATDistribution;
	double* dIAT;
	double dRandomWaitTime;
}APP_BSM_INFO, * PAPP_BSM_INFO;

/// Structure for voice information such as packet size, inter arrival time and service type,this is applicable for voice traffic
struct stru_Application_VoiceInfo
{
	DISTRIBUTION packetSizeDistribution;
	double dPacketSize;
	DISTRIBUTION IATDistribution;
	double dIAT;
	SERVICE_TYPE nServiceType;
	SUPPRESSION_MODEL nSuppressionModel;
	/*for Deterministic*/
	int nSuccessRatio;
	/*For Markov*/
	int nTSL;//Talk spurt length
	double dSAF;//Speech activity factor
};

/* Enumerator for video model*/
typedef enum
{
	VidModel_NULL = 0,
	VidModel_INDEPENDENT_GAUSSIAN = 1,
	VidModel_FIRST_ORDER_DEPENDENT_GAUSSIAN,
	VidModel_H_261,
	VidModel_H_263,
	VidModel_MPEG1_Low_Res,
	VidModel_MPEG1_High_Res,
	VidModel_MPEG2_Low_Res,
	VidModel_MPEG2_High_Res,
	VidModel_BUFFERED_VIDEO_STREAMING_1,
	VidModel_BUFFERED_VIDEO_STREAMING_2,
	VidModel_BUFFERED_VIDEO_STREAMING_3,
	VidModel_BUFFERED_VIDEO_STREAMING_4,
	VidModel_BUFFERED_VIDEO_STREAMING_5,
	VidModel_BUFFERED_VIDEO_STREAMING_6,
	VidModel_LAST,
}VIDEOMODEL;
static const char* strVIDEOMODEL[] = { "","INDEPENDENT_GAUSSIAN",
"FIRST_ORDER_DEPENDENT_GAUSSIAN", "H_261", "H_263","MPEG1_Low_Res",
"MPEG1_High_Res","MPEG2_Low_Res","MPEG2_High_Res","BUFFERED_VIDEO_STREAMING_1",
"BUFFERED_VIDEO_STREAMING_2","BUFFERED_VIDEO_STREAMING_3",
"BUFFERED_VIDEO_STREAMING_4","BUFFERED_VIDEO_STREAMING_5",
"BUFFERED_VIDEO_STREAMING_6","Unknown" };

/// structure to store the video application information
struct stru_Application_VideoInfo
{
	VIDEOMODEL nVidModel;
	int fps;
	int ppf;
	double mu;
	double sigma;
	double const_a;
	double const_b;
	double eta;

	DISTRIBUTION packetSizeDistribution;
	double* dPacketSizeArgs; /* Mean packet size for Exponential and constant distribution
							 * Scale and Shape parameter for Weibull distrinution
							 */

	DISTRIBUTION IATDistribution;
	double dIATArgs; // Mean IAT for Exponential and constant distribution
};
///Structure to store the emulation application information
struct stru_Application_EmulationInfo
{
	bool isMulticast;
	bool isBroadcast;
	UINT count;
	NETSIM_ID nDestinationId;
	NETSIM_ID* nSourceId;
	NETSIM_ID nSourcePort;
	NETSIM_ID nDestinationPort;
	NETSIM_IPAddress* realSourceIP;
	NETSIM_IPAddress realDestIP;
	NETSIM_IPAddress* simSourceIP;
	NETSIM_IPAddress simDestIP;

	//Only for fast emulation
	char* protocol;
	char* filterString;
	UINT priority;
};
/// Structure to store the HTTP application information
struct stru_Application_HTTPInfo
{
	double pageIAT;
	DISTRIBUTION pageIATDistribution;
	unsigned int pageCount;
	double pageSize;
	DISTRIBUTION pageSizeDistribution;
};

/// Structure to store the Interactive Gaming application information
struct stru_Application_InteractiveGamingInfo
{
	double ul_a_size;
	double ul_b_size;
	double ul_b_IAT;
	double dl_a_size;
	double dl_b_size;
	double dl_a_IAT;
	double dl_b_IAT;
	UINT64 ul_id;
	UINT64 dl_id;
};

/// Structure to store the COAP application information

struct stru_Application_COAPInfo
{
	double pageIAT;
	DISTRIBUTION pageIATDistribution;
	double pageSize;
	DISTRIBUTION pageSizeDistribution;
	bool ackRequired;
	bool multicastResponse;
	int variableResponseTime; //in miliseconds
	int piggybackedTime; //in miliseconds
	int ackResponseTime; //in milliseconds
	int ackTimeOut; //in milliseconds
	double ackRandomFactor;
	int maxRetransmit;
	int nstart;
	int defaultLeisure;
	int probingRate;
	struct stru_NetSim_COAP_data* pCOAPData;
};

/// Structure to store email application information
struct stru_Application_EMAILInfo
{
	struct stru_emailinfo
	{
		double dDuration;
		DISTRIBUTION durationDistribution;
		double dEmailSize;
		DISTRIBUTION sizeDistribution;
	}SEND, RECEIVE;
};

/// Structure for Seeder list of peer_to_peer traffic
typedef struct stru_Application_P2P_SeederList
{
	NetSim_PACKET* packet;
	unsigned int nSeederCount;
	NETSIM_ID* nDeviceIds;
}SEEDER_LIST;
/// Structure for peer list of peer_to_peer traffic
typedef struct stru_Application_p2P_PeerList
{
	NETSIM_ID nDeviceId;
	unsigned int yetToReceiveCount;
	unsigned int* flag;
}PEER_LIST;
/// Structure fot peer_to_peer application
struct stru_Application_P2PInfo
{
	double dFileSize;
	DISTRIBUTION fizeSizeDistribution;
	double dPiecesSize;

	unsigned int nPieceCount;	//Pieces count
	NetSim_PACKET** pieceList; //Pieces
	SEEDER_LIST** seederList; //For each pieces
	PEER_LIST** peerList;	//For each peers
};
/// Structure for Erlang_call application
struct stru_Application_CallInfo
{
	DISTRIBUTION callDurationDistribution;
	double dCallDuration;
	DISTRIBUTION callIATDistribution;
	double dCallIAT;
	int** nCallStatus;
	double** dCallEndTime;
	NetSim_PACKET*** nextPacket;
	APP_VOICE_INFO VoiceInfo;

	unsigned long long int** nEventId;
	unsigned long long int** nAppoutevent;
	int(*fn_BlockCall)(ptrAPPLICATION_INFO appInfo, NETSIM_ID nSourceId, NETSIM_ID nDestinationId, double time);
};

//For Email.c
typedef struct stru_email_detail
{
	int type;
	ptrAPPLICATION_INFO appInfo;
}DETAIL;

#include "NetSim_Plot.h"
//Statistics
bool nApplicationThroughputPlotFlag;
char* szApplicationThroughputPlotVal;
bool nApplicationThroughputPlotRealTime;
PNETSIMPLOT* applicationThroughputPlots;
bool* nAppPlotFlag;

NetSim_PACKET* fn_NetSim_Application_GeneratePacket(ptrAPPLICATION_INFO info,
	double ldArrivalTime,
	NETSIM_ID nSourceId,
	UINT destCount,
	NETSIM_ID* nDestination,
	unsigned long long int nPacketId,
	APPLICATION_TYPE nAppType,
	QUALITY_OF_SERVICE nQOS,
	unsigned int sourcePort,
	unsigned int destPort);

int fn_NetSim_Application_GenerateNextPacket(ptrAPPLICATION_INFO appInfo,
	NETSIM_ID nSource,
	UINT destCount,
	NETSIM_ID* nDestination,
	double time);

/* Distribution Function */
_declspec(dllexport) int fnDistribution(DISTRIBUTION nDistributionType, double* fDistOut,
	unsigned long* uSeed, unsigned long* uSeed1, double* args);

/* Random number generator */
_declspec(dllexport) int fnRandomNo(long lm, double* fRandNo, unsigned long* uSeed, unsigned long* uSeed1);

/* HTTP Application */
int fn_NetSim_Application_StartHTTPAPP(ptrAPPLICATION_INFO appInfo, double time);
int fn_NetSim_Application_ConfigureHTTPTraffic(ptrAPPLICATION_INFO appInfo, void* xmlNetSimNode);
int fn_NetSim_Application_HTTP_ProcessRequest(ptrAPPLICATION_INFO pstruappinfo, NetSim_PACKET* pstruPacket);

/*Interactive Gaming Application*/
int fn_NetSim_Application_ConfigureInteractiveGamingTraffic(ptrAPPLICATION_INFO appInfo, void* xmlNetSimNode);
int fn_NetSim_Application_StartInteractiveGamingULAPP(ptrAPPLICATION_INFO appInfo, double time);
int fn_NetSim_Application_StartInteractiveGamingDLAPP(ptrAPPLICATION_INFO appInfo, double time);
int fn_NetSim_Application_InteractiveGamingUL_GenerateNextPacket(ptrAPPLICATION_INFO appInfo, NetSim_PACKET* pstruPacket, double time);
int fn_NetSim_Application_InteractiveGamingDL_GenerateNextPacket(ptrAPPLICATION_INFO appInfo, NetSim_PACKET* pstruPacket, double time);

/* COAP Application*/
int fn_NetSim_Application_StartCOAPAPP(ptrAPPLICATION_INFO appInfo, double time);
int fn_NetSim_Application_ConfigureCOAPTraffic(ptrAPPLICATION_INFO appInfo, void* xmlNetSimNode);
int fn_NetSim_Application_COAP_ProcessRequest(ptrAPPLICATION_INFO pstruappinfo, NetSim_PACKET* pstruPacket);

/* Video Application */
int fn_NetSim_Application_StartVideoAPP(ptrAPPLICATION_INFO appInfo, double time);
int fn_NetSim_Application_ConfigureVideoTraffic(ptrAPPLICATION_INFO appInfo, void* xmlNetSimNode);
_declspec(dllexport) int fn_NetSim_TrafficGenerator_Video(APP_VIDEO_INFO* info,
	double* fPacketSize,
	double* ldArrival,
	unsigned long* uSeed,
	unsigned long* uSeed1);
//Functions for Video Models H_261, H_263, MPEG1_M, MPEG1_H, MPEG2_M, MPEG2_H

int fn_NetSim_TrafficGenerator_MPEGVideo(APP_VIDEO_INFO* info,
	double* fPacketSize,
	double* ldArrival,
	unsigned long* uSeed,
	unsigned long* uSeed1);
int fn_NetSim_Application_ConfigureMPEGVideo(APP_VIDEO_INFO* info, void* xmlNetSimNode);

/* Voice Application */
int fn_NetSim_Application_StartVoiceAPP(ptrAPPLICATION_INFO appInfo, double time);
int fn_NetSim_Application_ConfigureVoiceTraffic(ptrAPPLICATION_INFO appInfo, void* xmlNetSimNode);
_declspec(dllexport) int fn_NetSim_TrafficGenerator_Voice(APP_VOICE_INFO* info,
	double* fSize,
	double* ldArrival,
	unsigned long* uSeed,
	unsigned long* uSeed1);

/* Peer To Peer Application */
int fn_NetSim_Application_P2P_GenerateFile(ptrAPPLICATION_INFO appInfo);
int fn_NetSim_Application_P2P_InitSeederList(ptrAPPLICATION_INFO appInfo);
int fn_NetSim_Application_P2P_InitPeers(ptrAPPLICATION_INFO appInfo);
int fn_NetSim_Application_P2P_SendNextPiece(ptrAPPLICATION_INFO appInfo, NETSIM_ID destination, double time);
int fn_NetSim_Application_StartP2PAPP(ptrAPPLICATION_INFO appInfo, double time);
int fn_NetSim_Application_ConfigureP2PTraffic(ptrAPPLICATION_INFO appInfo, void* xmlNetSimNode);
int fn_NetSim_Application_P2P_MarkReceivedPacket(ptrAPPLICATION_INFO pstruappinfo, NetSim_PACKET* pstruPacket);

/* Email Application */
int fn_NetSim_Application_StartEmailAPP(ptrAPPLICATION_INFO appInfo, double time);
int fn_NetSim_Application_ConfigureEmailTraffic(ptrAPPLICATION_INFO appInfo, void* xmlNetSimNode);
ptrAPPLICATION_INFO fn_NetSim_Application_Email_GenerateNextPacket(DETAIL* detail, NETSIM_ID nSourceId, NETSIM_ID nDestinationId, double time);
ptrAPPLICATION_INFO get_email_app_info(void* detail);

/* Custom, FTP, Database Application */
int fn_NetSim_Application_StartDataAPP(ptrAPPLICATION_INFO appInfo, double time);
int fn_NetSim_Application_ConfigureDataTraffic(ptrAPPLICATION_INFO appInfo, void* xmlNetSimNode);
int fn_NetSim_Application_ConfigureDatabaseTraffic(ptrAPPLICATION_INFO appInfo, void* xmlNetSimNode);
int fn_NetSim_Application_ConfigureFTPTraffic(ptrAPPLICATION_INFO appInfo, void* xmlNetSimNode);
_declspec(dllexport) int fn_NetSim_TrafficGenerator_Custom(APP_DATA_INFO* info,
	double* fSize,
	double* ldArrival,
	unsigned long* uSeed,
	unsigned long* uSeed1,
	unsigned long* uSeed2,
	unsigned long* uSeed3);

/* Erlang Call */
int fn_NetSim_Application_ErlangCall_StartCall(NetSim_EVENTDETAILS* pstruEventDetails);
int fn_NetSim_Application_ErlangCall_EndCall(NetSim_EVENTDETAILS* pstruEventDetails);
int fn_NetSim_Application_StartErlangCallAPP(ptrAPPLICATION_INFO appInfo, double time);
int fn_NetSim_Application_ConfigureErlangCallTraffic(ptrAPPLICATION_INFO appInfo, void* xmlNetSimNode);

/* Emulation */
int fn_NetSim_Application_ConfigureEmulationTraffic(ptrAPPLICATION_INFO appInfo, void* xmlNetSimNode);
void fn_NetSim_Emulation_StartApplication(ptrAPPLICATION_INFO appInfo);

/* Vanet Application*/
int fn_NetSim_Application_StartBSM(ptrAPPLICATION_INFO appInfo, double time);
int fn_NetSim_Application_BSM(PAPP_BSM_INFO info,
	double* fSize,
	double* ldArrival,
	unsigned long* uSeed,
	unsigned long* uSeed1,
	unsigned long* uSeed2,
	unsigned long* uSeed3);
bool add_sae_j2735_payload(NetSim_PACKET* packet, ptrAPPLICATION_INFO info);
void process_saej2735_packet(NetSim_PACKET* packet);

//Multicast
void add_multicast_route(ptrAPPLICATION_INFO info);
void join_multicast_group(ptrAPPLICATION_INFO info, double time);

/* Application API's */
_declspec(dllexport) int fn_NetSim_Application_Init_F();
_declspec(dllexport) int fn_NetSim_Application_Configure_F(void** var);
_declspec(dllexport) int fn_NetSim_Application_Metrics_F(PMETRICSWRITER metricsWriter);
int fn_NetSim_App_RestartApplication();
_declspec(dllexport) int fn_NetSim_Application_Plot(NetSim_PACKET* pstruPacket);
PACKET_TYPE fn_NetSim_Application_GetPacketTypeBasedOnApplicationType(APPLICATION_TYPE nAppType);
ptrSOCKETINTERFACE fnGetSocket(NETSIM_ID nAppId,
	NETSIM_ID nSourceId,
	NETSIM_ID nSourcePort,
	NETSIM_ID nDestPort);

//Application metrics
void free_app_metrics(ptrAPPLICATION_INFO appInfo);
void appmetrics_src_add(ptrAPPLICATION_INFO appInfo, _In_ NetSim_PACKET* packet);
void appmetrics_dest_add(ptrAPPLICATION_INFO appInfo, NetSim_PACKET* packet, NETSIM_ID dest);
int fn_NetSim_Application_Metrics_F(PMETRICSWRITER metricsWriter);

//Application Interface function
void fnCreatePort(ptrAPPLICATION_INFO info);
int fnCreateSocketBuffer(ptrAPPLICATION_INFO appInfo);
void P2P_create_socket(ptrAPPLICATION_INFO appInfo, NETSIM_ID src, NETSIM_ID dest);
void Interactive_Gaming_createUL_socket(ptrAPPLICATION_INFO appInfo, NETSIM_ID src, NETSIM_ID dest);
void Interactive_Gaming_createDL_socket(ptrAPPLICATION_INFO appInfo, NETSIM_ID src, NETSIM_ID dest);

int fn_NetSim_Add_DummyPayload(NetSim_PACKET* packet, ptrAPPLICATION_INFO);

//Encryption
char xor_encrypt(char ch, long key);
int aes256(char* str, int* len);
int des(char* buf, int* len);

//Application event handler
void handle_app_out();

//Application Packet Log
void init_application_log();
void print_application_log(ptrAPPLICATION_INFO pstruappinfo, NetSim_PACKET* pstruPacket);
void close_application_log();
bool get_protocol_log_status(char* logname);
void update_latency();

// Generation Logs
void init_application_generation_log();
double get_generation_rate(ptrAPPLICATION_INFO pstruappinfo, NetSim_PACKET* packet);
void close_application_generation_log();
void print_application_generation_log(ptrAPPLICATION_INFO pstruappinfo, NetSim_PACKET* packet);
double calculate_generation_rate(ptrAPPLICATION_INFO pstruappinfo, NetSim_PACKET* packet);
#endif
