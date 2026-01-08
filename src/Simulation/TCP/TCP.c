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
#include "main.h"
#include "TCP.h"
#include "TCP_Header.h"
#include "TCP_enum.h"
#include "NetSim_Plot.h"
#include "NetSim_utility.h"
#include "../Application/Application.h"

char* GetStringTCP_Subevent(NETSIM_ID);
static FILE* fpTCP_CWlog = NULL;

#pragma comment(lib,"TCP.lib")

//Function prototype
int fn_NetSim_TCP_Configure_F(void **var);
int fn_NetSim_TCP_Init_F(struct stru_NetSim_Network* net,
						 NetSim_EVENTDETAILS* pevent,
						 char* appPath,
						 char* iopath,
						 int version,
						 void** fnPointer);
int fn_NetSim_TCP_CopyPacket_F(NetSim_PACKET* dst,
							   NetSim_PACKET* src);
int fn_NetSim_TCP_FreePacket_F(NetSim_PACKET* packet);
int fn_NetSim_TCP_Finish_F();
int fn_NetSim_TCP_Metrics_F(PMETRICSWRITER metricsWriter);
char* fn_NetSim_TCP_ConfigPacketTrace_F(const void* xmlNetSimNode);
int fn_NetSim_TCP_WritePacketTrace_F(NetSim_PACKET* pstruPacket, char** ppszTrace);

static int fn_NetSim_TCP_HandleTransportOut();
static int fn_NetSim_TCP_HandleTransportIn();
static int fn_NetSim_TCP_HandleTimer();
static void handle_time_wait_timeout();

/**
This function is called by NetworkStack.dll, whenever the event gets triggered
inside the NetworkStack.dll for the Transport layer TCP protocol
It includes TRANSPORT_OUT,TRANSPORT_IN and TIMER_EVENT.
*/
_declspec (dllexport) int fn_NetSim_TCP_Run()
{
	switch (pstruEventDetails->nEventType)
	{
	case TRANSPORT_OUT_EVENT:
		return fn_NetSim_TCP_HandleTransportOut();
		break;
	case TRANSPORT_IN_EVENT:
		return fn_NetSim_TCP_HandleTransportIn();
		break;
	case TIMER_EVENT:
		return fn_NetSim_TCP_HandleTimer();
		break;
	default:
		fnNetSimError("Unknown event %d for TCP protocol in %s\n",
					  pstruEventDetails->nEventType,
					  __FUNCTION__);
		return -1;
	}
}

/**
This function is called by NetworkStack.dll, while configuring the device
TRANSPORT layer for TCP protocol.
*/
_declspec(dllexport) int fn_NetSim_TCP_Configure(void** var)
{
	return fn_NetSim_TCP_Configure_F(var);
}

/**
This function initializes the TCP parameters.
*/
_declspec (dllexport) int fn_NetSim_TCP_Init(struct stru_NetSim_Network *NETWORK_Formal,
											 NetSim_EVENTDETAILS *pstruEventDetails_Formal,
											 char *pszAppPath_Formal,
											 char *pszWritePath_Formal,
											 int nVersion_Type,
											 void **fnPointer)
{
	init_TCP_congestion_log();
	return fn_NetSim_TCP_Init_F(NETWORK_Formal,
								pstruEventDetails_Formal,
								pszAppPath_Formal,
								pszWritePath_Formal,
								nVersion_Type,
								fnPointer);
}

/**
This function is called by NetworkStack.dll, once simulation end to free the
allocated memory for the network.
*/
_declspec(dllexport) int fn_NetSim_TCP_Finish()
{
	close_TCP_congestion_log();
	return fn_NetSim_TCP_Finish_F();
}

/**
This function is called by NetworkStack.dll, while writing the event trace
to get the sub event as a string.
*/
_declspec (dllexport) char *fn_NetSim_TCP_Trace(int nSubEvent)
{
	return (GetStringTCP_Subevent(nSubEvent));
}

/**
This function is called by NetworkStack.dll, to free the TCP protocol
pstruTransportData->Packet_TransportProtocol.
*/
_declspec(dllexport) int fn_NetSim_TCP_FreePacket(NetSim_PACKET* pstruPacket)
{
	return fn_NetSim_TCP_FreePacket_F(pstruPacket);
}

/**
This function is called by NetworkStack.dll, to copy the TCP protocol
pstruTransportData->Packet_TransportProtocol from source packet to destination.
*/
_declspec(dllexport) int fn_NetSim_TCP_CopyPacket(NetSim_PACKET* pstruDestPacket, NetSim_PACKET* pstruSrcPacket)
{
	return fn_NetSim_TCP_CopyPacket_F(pstruDestPacket, pstruSrcPacket);
}

/**
This function write the Metrics in Metrics.txt
*/
_declspec(dllexport) int fn_NetSim_TCP_Metrics(PMETRICSWRITER metricsWriter)
{
	return fn_NetSim_TCP_Metrics_F(metricsWriter);
}

/**
This function will return the string to write packet trace heading.
*/
_declspec(dllexport) char* fn_NetSim_TCP_ConfigPacketTrace(const void* xmlNetSimNode)
{
	return fn_NetSim_TCP_ConfigPacketTrace_F(xmlNetSimNode);
}

/**
This function will return the string to write packet trace.
*/
_declspec(dllexport) int fn_NetSim_TCP_WritePacketTrace(NetSim_PACKET* pstruPacket, char** ppszTrace)
{
	return fn_NetSim_TCP_WritePacketTrace_F(pstruPacket, ppszTrace);
}

static int fn_NetSim_TCP_HandleTransportOut()
{
	switch (pstruEventDetails->nSubEventType)
	{
	case 0:
		return packet_arrive_from_application_layer();
		break;
	default:
		fnNetSimError("Unknown sub-event %d for TCP in %s", 
					  pstruEventDetails->nSubEventType,
					  __FUNCTION__);
		return -1;
	}
}

static int fn_NetSim_TCP_HandleTransportIn()
{
	switch (pstruEventDetails->nSubEventType)
	{
	case 0:
		packet_arrive_from_network_layer();
		break;
	default:
		fnNetSimError("Unknown sub-event %d for TCP transport in event\n",
					  pstruEventDetails->nSubEventType);
		break;
	}
	return 0;
}

static int fn_NetSim_TCP_HandleTimer()
{
	switch (pstruEventDetails->nSubEventType)
	{
	case TCP_RTO_TIMEOUT:
		handle_rto_timer();
		break;
	case TCP_TIME_WAIT_TIMEOUT:
		handle_time_wait_timeout();
		break;
	default:
		fnNetSimError("Unknown sub-event %d in %s\n",
					  pstruEventDetails->nSubEventType,
					  __FUNCTION__);
		break;
	}
	return 0;
}

TCPVARIANT get_tcp_variant_from_str(char* szVal)
{
	if (!_stricmp(szVal, "OLD_TAHOE"))
		return TCPVariant_OLDTAHOE;
	if (!_stricmp(szVal, "TAHOE"))
		return TCPVariant_TAHOE;
	if (!_stricmp(szVal, "RENO"))
		return TCPVariant_RENO;
	if (!_stricmp(szVal, "NEW_RENO"))
		return TCPVariant_NEWRENO;
	if (!_stricmp(szVal, "BIC"))
		return TCPVariant_BIC;
	if (!_stricmp(szVal, "CUBIC"))
		return TCPVariant_CUBIC;
	else
		fnNetSimError("Unknown TCP Variant %s\n",szVal);
	return TCPVariant_TAHOE;
}

TCPACKTYPE get_tcp_ack_type_from_str(char* szVal)
{
	if (!_stricmp(szVal, "Delayed"))
		return TCPACKTYPE_DELAYED;
	if (!_stricmp(szVal, "Undelayed"))
		return TCPACKTYPE_UNDELAYED;
	fnNetSimError("Unknown TCP ack type %s\n", szVal);
	return TCPACKTYPE_UNDELAYED;
}

void start_timewait_timer(PNETSIM_SOCKET s)
{
	NetSim_EVENTDETAILS pevent;
	memcpy(&pevent, pstruEventDetails, sizeof pevent);
	pevent.dEventTime += s->tcb->timeWaitTimer;
	pevent.dPacketSize = 0;
	pevent.nEventType = TIMER_EVENT;
	pevent.nPacketId = 0;
	pevent.nProtocolId = TX_PROTOCOL_TCP;
	pevent.nSegmentId = 0;
	pevent.nSubEventType = TCP_TIME_WAIT_TIMEOUT;
	pevent.pPacket = NULL;
	pevent.szOtherDetails = s;
	fnpAddEvent(&pevent);

	print_tcp_log("Adding Time-wait_timer at %0.2lf",
				  pevent.dEventTime);
	print_tcp_log("Canceling all other timer.");

	s->tcb->isOtherTimerCancel = true;
}

static void handle_time_wait_timeout()
{
	PNETSIM_SOCKET s = pstruEventDetails->szOtherDetails;
	tcp_change_state(s, TCPCONNECTION_CLOSED);
	delete_tcb(s);
	tcp_close_socket(s, pstruEventDetails->nDeviceId);
}


void init_TCP_congestion_log()
{
	if (get_protocol_log_status("TCP_Congestion_Log"))
	{
		char s[BUFSIZ];
		sprintf(s, "%s\\%s", pszIOLogPath, "TCP_Congestion_Window_Log.csv");
		fpTCP_CWlog = fopen(s, "w");
		if (!fpTCP_CWlog)
		{
			fnSystemError("Unable to open %s file", s);
			perror(s);
		}
		else
		{
			fprintf(fpTCP_CWlog, "%s,%s,%s,%s,%s,%s,%s,",
				"Time (Microseconds)", "Application ID", "Application Name", "Source Device Name", "Source ID", "Destination Device Name", "Destination ID");

			fprintf(fpTCP_CWlog, "%s,%s,%s,",
				"Source Socket Address", "Destination Socket Address", "Sender Window Size(Bytes)");

			fprintf(fpTCP_CWlog, "\n");
			if (nDbgFlag) fflush(fpTCP_CWlog);
		}
	}
}

void write_congestion_plot(PNETSIM_SOCKET s, NetSim_PACKET* packet)
{
		UINT win = get_cwnd_print(s);

	if (fpTCP_CWlog == NULL || !packet->nPacketId) return;

	ptrAPPLICATION_INFO pstruappinfo = ((ptrAPPLICATION_INFO*)NETWORK->appInfo)[packet->pstruAppData->nApplicationId - 1];

	fprintf(fpTCP_CWlog, "%lf,%d,%s,%s,%d,%s,%d,", pstruEventDetails->dEventTime,pstruappinfo->id,pstruappinfo->name,
		DEVICE_NAME(s->localDeviceId),s->localDeviceId,DEVICE_NAME(s->remoteDeviceId),s->remoteDeviceId);

	fprintf(fpTCP_CWlog, "%s,%s,%d,", s->tcpMetrics->localAddr, s->tcpMetrics->remoteAddr,s->tcpMetrics->prevWindowSize);

	fprintf(fpTCP_CWlog, "\n");

	fprintf(fpTCP_CWlog, "%lf,%d,%s,%s,%d,%s,%d,", pstruEventDetails->dEventTime, pstruappinfo->id, pstruappinfo->name,
		DEVICE_NAME(s->localDeviceId), s->localDeviceId, DEVICE_NAME(s->remoteDeviceId), s->remoteDeviceId);

	fprintf(fpTCP_CWlog, "%s,%s,%d,", s->tcpMetrics->localAddr, s->tcpMetrics->remoteAddr, win);

	fprintf(fpTCP_CWlog, "\n");

		s->tcpMetrics->prevWindowSize = win;
	}


void close_TCP_congestion_log()
{
	if (fpTCP_CWlog)
		fclose(fpTCP_CWlog);
}

bool isTCPlog()
{
	if (get_protocol_log_status("TCP_LOG"))
		return true;
	else
		return false;
}

bool get_protocol_log_status(char* logname)
{
	FILE* fp;
	char str[BUFSIZ];
	char data[BUFSIZ];

	sprintf(str, "%s/%s", pszIOPath, "ProtocolLogsConfig.txt");
	fp = fopen(str, "r");
	if (!fp)
		return false;

	sprintf(str, "%s=true", logname);

	while (fgets(data, BUFSIZ, fp))
	{
		lskip(data);
		if (!_strnicmp(data, str, strlen(str)))
		{
			fclose(fp);
			return true;
		}
	}
	fclose(fp);

	return false;
}
