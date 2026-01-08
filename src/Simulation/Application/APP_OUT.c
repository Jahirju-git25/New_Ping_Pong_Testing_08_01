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
#include "Application.h"
static FILE* fpGenerationLogs = NULL;

static void add_app_info_to_packet(NetSim_PACKET* packet, double time, double size, UINT appId)
{
	packet->pstruAppData->dArrivalTime = time;
	packet->pstruAppData->dEndTime = time;
	if (packet->nPacketType != PacketType_Control)
	{
		packet->pstruAppData->dOverhead = 0;
		packet->pstruAppData->dPayload = size;
	}
	else
	{
		packet->pstruAppData->dPayload = 0;
		packet->pstruAppData->dOverhead = size;
	}
	packet->pstruAppData->dPacketSize = packet->pstruAppData->dOverhead + packet->pstruAppData->dPayload;
	packet->pstruAppData->dStartTime = ldEventTime;
	packet->pstruAppData->nApplicationId = appId;
}

static void set_app_end_and_generate_next_packet(NetSim_PACKET* pstruPacket, ptrAPPLICATION_INFO appInfo,
												 UINT destCount, NETSIM_ID* dest)
{
	APPLICATION_TYPE nappType = pstruPacket->pstruAppData->nAppType;

	if (nappType == TRAFFIC_FTP || nappType == TRAFFIC_DATABASE)
	{
		NetSim_PACKET* packet = pstruPacket;
		while (packet->pstruNextPacket)
			packet = packet->pstruNextPacket;
		packet->pstruAppData->nAppEndFlag = 1;
		fn_NetSim_Application_StartDataAPP(appInfo, pstruEventDetails->dEventTime);
	}
	else if (nappType == TRAFFIC_HTTP)
	{
		//Do nothing
		NetSim_PACKET* packet = pstruPacket;
		if (pstruPacket->pstruAppData->nAppEndFlag)
		{
			while (packet->pstruNextPacket)
			{
				packet->pstruAppData->nAppEndFlag = 0;
				packet = packet->pstruNextPacket;
			}
			packet->pstruAppData->nAppEndFlag = 1;
		}
	}
	else if (nappType == TRAFFIC_INTERACTIVE_GAMING)
	{
		if (get_first_dest_from_packet(pstruPacket) == appInfo->sourceList[0])
		{
			strcpy(pstruPacket->szPacketType, appInfo->name);
			strcat(pstruPacket->szPacketType, "_UL");
			fn_NetSim_Application_InteractiveGamingUL_GenerateNextPacket(appInfo, pstruPacket, ldEventTime);
		}
		else
		{
			strcpy(pstruPacket->szPacketType, appInfo->name);
			strcat(pstruPacket->szPacketType, "_DL");
			fn_NetSim_Application_InteractiveGamingDL_GenerateNextPacket(appInfo, pstruPacket, ldEventTime);
		}
	}
	else if (nappType == TRAFFIC_COAP)
	{
		//DO nothing
	}
	else if (nappType == TRAFFIC_EMAIL)
	{
		appInfo = fn_NetSim_Application_Email_GenerateNextPacket((DETAIL*)appInfo,
																 pstruPacket->nSourceId,
																 get_first_dest_from_packet(pstruPacket),
																 pstruEventDetails->dEventTime);
	}
	else if (nappType == TRAFFIC_PEER_TO_PEER)
	{
		NetSim_PACKET* packet = pstruPacket;
		while (packet->pstruNextPacket)
			packet = packet->pstruNextPacket;
		packet->pstruAppData->nAppEndFlag = 1;
	}
	else if (nappType == TRAFFIC_EMULATION)
	{
		//do nothing
	}
	else
	{
		fn_NetSim_Application_GenerateNextPacket(appInfo,
												 pstruPacket->nSourceId,
												 destCount,
												 dest,
												 pstruEventDetails->dEventTime);
	}
}

static void send_to_transport_layer(ptrSOCKETINTERFACE s, ptrAPPLICATION_INFO appInfo,
									NetSim_PACKET* packet, NETSIM_ID nDeviceId)
{
	packet->pstruTransportData->nTransportProtocol = appInfo->trxProtocol;
	if (!fn_NetSim_Socket_GetBufferStatus(s))
	{
		//Socket buffer is NULL
		//Create event for transport out
		pstruEventDetails->dEventTime = ldEventTime;
		pstruEventDetails->nEventType = TRANSPORT_OUT_EVENT;
		pstruEventDetails->pPacket = NULL;
		pstruEventDetails->nProtocolId = appInfo->trxProtocol;
		pstruEventDetails->nSubEventType = 0;
#pragma warning(disable:4312)
		pstruEventDetails->szOtherDetails = (void*)s;
#pragma warning(default:4312)
		//Add Transport out event
		fnpAddEvent(pstruEventDetails);
	}

	//Place the packet to socket buffer
	fn_NetSim_Socket_PassPacketToInterface(nDeviceId, packet, s);
}

static void send_to_next_protocol(ptrSOCKETINTERFACE s, ptrAPPLICATION_INFO appInfo,
								  NetSim_PACKET* packet, NETSIM_ID nDeviceId)
{
	NetSim_EVENTDETAILS pevent;
	while (packet)
	{
		memset(&pevent, 0, sizeof pevent);
		pevent.dEventTime = ldEventTime;
		pevent.dPacketSize = packet->pstruAppData->dPacketSize;
		pevent.nApplicationId = packet->pstruAppData->nApplicationId;
		pevent.nDeviceId = nDeviceId;
		pevent.nDeviceType = DEVICE_TYPE(nDeviceId);
		pevent.nEventType = APPLICATION_OUT_EVENT;
		pevent.nPacketId = packet->nPacketId;
		pevent.nProtocolId = appInfo->protocol;
		pevent.nSegmentId = packet->pstruAppData->nSegmentId;
		pevent.pPacket = packet;
		pevent.szOtherDetails = (void*)s;
		packet = packet->pstruNextPacket;
		pevent.pPacket->pstruNextPacket = NULL;
		fnpAddEvent(&pevent);
	}
}

static void appout_send_packet(ptrSOCKETINTERFACE s, ptrAPPLICATION_INFO appInfo,
							   NetSim_PACKET* packet, NETSIM_ID nDeviceId)
{
	if (appInfo->protocol == APP_PROTOCOL_NULL)
		send_to_transport_layer(s, appInfo, packet, nDeviceId);
	else
		send_to_next_protocol(s, appInfo, packet, nDeviceId);
}

static ptrAPPLICATION_INFO get_app_info(void* detail, APPLICATION_TYPE type)
{
	if (type == TRAFFIC_EMAIL)
		return get_email_app_info(detail);

	return detail;
}

void handle_app_out()
{
	void* otherDetails = pstruEventDetails->szOtherDetails;
	double ldEventTime = pstruEventDetails->dEventTime;
	NETSIM_ID nDeviceId = pstruEventDetails->nDeviceId;

	NetSim_PACKET* pstruPacket = pstruEventDetails->pPacket;
	if (!pstruPacket) return; //No packet to send

	APPLICATION_TYPE nappType = pstruPacket->pstruAppData->nAppType;

	UINT destCount;
	NETSIM_ID* dest = get_dest_from_packet(pstruPacket, &destCount);

	ptrSOCKETINTERFACE s;
	s = fnGetSocket(pstruEventDetails->nApplicationId,
					pstruPacket->nSourceId,
					pstruPacket->pstruTransportData->nSourcePort,
					pstruPacket->pstruTransportData->nDestinationPort);
	if (!s)
	{
		fnNetSimError("Socket is NULL for application %d.\n",
					  pstruEventDetails->nApplicationId);
		return;
	}

	//Initialize the application data
	add_app_info_to_packet(pstruPacket, ldEventTime, 
						   pstruEventDetails->dPacketSize, pstruEventDetails->nApplicationId);

	

	

	ptrAPPLICATION_INFO appInfo = get_app_info(pstruEventDetails->szOtherDetails, nappType);
	if (!appInfo)
	{
		fnNetSimError("Application info is NULL in application out event id = %lld\n", pstruEventDetails->nEventId);
		return;
	}

	pstruPacket->pstruTransportData->nTransportProtocol = appInfo->trxProtocol;

	//Fragment the packet
	int nSegmentCount = 0;
	double  segmentsize = fn_NetSim_Stack_GetMSS(pstruPacket);
	print_application_generation_log(appInfo, pstruPacket);
	nSegmentCount = fn_NetSim_Stack_FragmentPacket(pstruPacket, (int)fn_NetSim_Stack_GetMSS(pstruPacket));


	//Add the dummy payload to packet
	fn_NetSim_Add_DummyPayload(pstruPacket, appInfo);

	appmetrics_src_add(appInfo, pstruPacket);

	appout_send_packet(s, appInfo, pstruPacket, nDeviceId);

	set_app_end_and_generate_next_packet(pstruPacket, otherDetails, destCount, dest);

}


void init_application_generation_log()
{
	if (get_protocol_log_status("APPLICATION_GENERATION_LOG"))
	{
		char s[BUFSIZ];
		sprintf(s, "%s\\%s", pszIOLogPath, "Application_Generation_Log.csv");
		fpGenerationLogs = fopen(s, "w");
		if (!fpGenerationLogs)
		{
			fnSystemError("Unable to open %s file", s);
			perror(s);
		}
		else
		{
			fprintf(fpGenerationLogs, "%s,%s,%s,%s,%s,%s,%s,",
				"Time(ms)","Application Name", "Application ID", "Source", "Destination", "Packet ID", "Generation Rate(Mbps)");

			fprintf(fpGenerationLogs, "\n");
		}
	}
}


void print_application_generation_log(ptrAPPLICATION_INFO pstruappinfo, NetSim_PACKET* packet)
{
	if (fpGenerationLogs == NULL)
		return;
	double dPacketSize = packet->pstruAppData->dPacketSize;
	NETSIM_ID nDestinationId = get_first_dest_from_packet(packet);
	double generationRate = calculate_generation_rate(pstruappinfo, packet);
	char DestinationName[BUFSIZ];
	char Appname[BUFSIZ];
	strcpy(Appname, pstruappinfo->name);
	
	if (pstruappinfo->nAppType == TRAFFIC_EMAIL || pstruappinfo->nAppType == TRAFFIC_INTERACTIVE_GAMING || pstruappinfo->nAppType == TRAFFIC_HTTP)
	{
		if (get_first_dest_from_packet(packet) == pstruappinfo->sourceList[0]) // UL
		{
			strcat(Appname, "_Send"); // Send
		}
		else
			strcat(Appname, "_Receive"); // Receive
	}

	if (nDestinationId)
		strcpy(DestinationName, DEVICE_NAME(nDestinationId));
	else
		strcpy(DestinationName, "BROADCAST");

	fprintf(fpGenerationLogs, "%lf,%s,%d,%s,",
		ldEventTime/MILLISECOND,Appname, pstruappinfo->nConfigId, DEVICE_NAME(packet->nSourceId));

	fprintf(fpGenerationLogs, "%s,%lld,%lf,",
		DestinationName, packet->nPacketId, generationRate);

	fprintf(fpGenerationLogs, "\n");
}

void close_application_generation_log()
{
	if (fpGenerationLogs)
		fclose(fpGenerationLogs);
}

double calculate_generation_rate(ptrAPPLICATION_INFO pstruappinfo, NetSim_PACKET* packet)
{
	double genRate = -1;
	double dPacketSize = packet->pstruAppData->dPacketSize;

	if (get_first_dest_from_packet(packet) == pstruappinfo->sourceList[0]) //Sent 
	{
		if (pstruappinfo->dl.lastPacketTime == -1 && ldEventTime == 0)
		{
			genRate = 0;
			pstruappinfo->dl.lastPacketTime = ldEventTime;
			pstruappinfo->dl.lastPacketSize = dPacketSize;
		}
		else if (pstruappinfo->dl.lastPacketTime == -1 && ldEventTime >= 0)
		{
			genRate = dPacketSize * 8 / (ldEventTime);
			pstruappinfo->dl.lastPacketTime = ldEventTime;
			pstruappinfo->dl.lastPacketSize = dPacketSize;
		}

		else if (ldEventTime > pstruappinfo->dl.lastPacketTime)
		{
			genRate = dPacketSize * 8 / (ldEventTime - pstruappinfo->dl.lastPacketTime);
			pstruappinfo->dl.prevLastPacketTime = pstruappinfo->dl.lastPacketTime;
			pstruappinfo->dl.lastPacketTime = ldEventTime;
			pstruappinfo->dl.lastPacketSize = dPacketSize;
		}

		else if (ldEventTime == pstruappinfo->dl.lastPacketTime && ldEventTime != 0)
		{
			if (pstruappinfo->dl.prevLastPacketTime == -1)
			{
				pstruappinfo->dl.lastPacketSize += dPacketSize;
				genRate = pstruappinfo->dl.lastPacketSize * 8 / (ldEventTime);
			}
			else
			{
				pstruappinfo->dl.lastPacketSize += dPacketSize;
				genRate = pstruappinfo->dl.lastPacketSize * 8 / (ldEventTime - pstruappinfo->dl.lastPacketTime);
			}
		}

		else
			genRate = 0;
	}

	else // Receive
	{
		if (pstruappinfo->ul.lastPacketTime == -1 && ldEventTime == 0)
		{
			genRate = 0;
			pstruappinfo->ul.lastPacketTime = ldEventTime;
			pstruappinfo->ul.lastPacketSize = dPacketSize;
		}
		else if (pstruappinfo->ul.lastPacketTime == -1 && ldEventTime >= 0)
		{
			genRate = dPacketSize * 8 / (ldEventTime);
			pstruappinfo->ul.lastPacketTime = ldEventTime;
			pstruappinfo->ul.lastPacketSize = dPacketSize;
		}

		else if (ldEventTime > pstruappinfo->ul.lastPacketTime)
		{
			genRate = dPacketSize * 8 / (ldEventTime - pstruappinfo->ul.lastPacketTime);
			pstruappinfo->ul.prevLastPacketTime = pstruappinfo->ul.lastPacketTime;
			pstruappinfo->ul.lastPacketTime = ldEventTime;
			pstruappinfo->ul.lastPacketSize = dPacketSize;
		}

		else if (ldEventTime == pstruappinfo->ul.lastPacketTime && ldEventTime != 0 )
		{
			if (pstruappinfo->ul.prevLastPacketTime == -1)
			{
				pstruappinfo->ul.lastPacketSize += dPacketSize;
				genRate = pstruappinfo->ul.lastPacketSize * 8 / (ldEventTime);
			}
			else
			{
				pstruappinfo->ul.lastPacketSize += dPacketSize;
				genRate = pstruappinfo->ul.lastPacketSize * 8 / (ldEventTime - pstruappinfo->ul.prevLastPacketTime);
			}
		}

		else
			genRate = 0;
	}

	return genRate;
}