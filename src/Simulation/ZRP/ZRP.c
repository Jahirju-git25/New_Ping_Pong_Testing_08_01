/************************************************************************************
 * Copyright (C) 2023                                                               *
 * TETCOS, Bangalore. India                                                         *
 *                                                                                  *
 * Tetcos owns the intellectual property rights in the Product and its content.     *
 * The copying, redistribution, reselling or publication of any or all of the       *
 * Product or its content without express prior written consent of Tetcos is        *
 * prohibited. Ownership and / or any other right relating to the software and all  *
 * intellectual property rights therein shall remain at all times with Tetcos.      *
 *                                                                                  *
 * Author:    Shashi Kant Suman                                                     *
 *                                                                                  *
 * ---------------------------------------------------------------------------------*/
#include "main.h"
#include "ZRP.h"
#include "ZRP_Enum.h"
#include "NetSim_utility.h"

static bool isCalledOnce = false;

/**
ZRP Init function initializes the ZRP parameters.
*/
_declspec (dllexport) int fn_NetSim_ZRP_Init()
{
	if (!isCalledOnce)
	{
		init_MPR_log();
		isCalledOnce = true;
	}

	return fn_NetSim_ZRP_Init_F();
}

/**
	This function is called by NetworkStack.dll, whenever the event gets triggered		
	inside the NetworkStack.dll for the ZRP protocol									
*/
_declspec (dllexport) int fn_NetSim_ZRP_Run()
{
	switch(pstruEventDetails->nEventType)
	{
	case NETWORK_OUT_EVENT:
		{
			NetSim_PACKET* packet = pstruEventDetails->pPacket;
			if(packet->nControlDataType/100 == NW_PROTOCOL_OLSR ||
				DEVICE_NWLAYER(pstruEventDetails->nDeviceId)->nRoutingProtocolId == NW_PROTOCOL_OLSR || isBroadcastPacket(packet))
				return 0; //IP layer will route the packet

			//Call ierp to route packet
			fn_NetSim_IERP_RoutePacket();
		}
		break;
	case NETWORK_IN_EVENT:
		{
			NetSim_PACKET* packet = pstruEventDetails->pPacket;
			if(packet->nControlDataType/100 == NW_PROTOCOL_ZRP ||
				packet->nControlDataType/100 == NW_PROTOCOL_OLSR)
			{
				switch(packet->nControlDataType)
				{
				case OLSR_CONTROL_PACKET(HELLO_MESSAGE):
					fn_NetSim_NDP_ReceiveHello();
					break;
				case OLSR_CONTROL_PACKET(TC_MESSAGE):
					fn_NetSim_OLSR_PacketProcessing();
					break;
				case IERP_ROUTE_REPLY_WITH_BRP_HEADER:
				case IERP_ROUTE_REQUEST_WITH_BRP_HEADER:
						fn_NetSim_BRP_ProcessPacket();
					break;
				case IERP_ROUTE_REPLY:
					fn_NetSim_IERP_ProcessRouteReply();
					break;
				default:
					fnNetSimError("Unknown control packet %d for ZRP protocol",packet->nControlDataType);
					break;
				}
			}
		}
		break;
	case TIMER_EVENT:
		{
			switch(pstruEventDetails->nSubEventType)
			{
			case NDP_ScheduleHelloTransmission:
				{
					double time=pstruEventDetails->dEventTime;
					NETSIM_ID nodeId=pstruEventDetails->nDeviceId;
					fn_NetSim_NDP_TransmitHello();
					fn_NetSim_NDP_ScheduleHelloTransmission(nodeId,time);
				}
				break;
			case OLSR_ScheduleTCTransmission:
				fn_NetSim_OLSR_TransmitTCMessage();
				fn_NetSim_OLSR_ScheduleTCTransmission(pstruEventDetails->nDeviceId,pstruEventDetails->dEventTime);
				break;
			case OLSR_LINK_TUPLE_Expire:
				fn_NetSim_OLSR_LinkTupleExpire();
				break;
			default:
				fnNetSimError("Unknown subevent %d fro ZRP protocol",pstruEventDetails->nSubEventType);
				break;
			}
		}
		break;
	default:
		fnNetSimError("Unknow event %d for ZRP protocol",pstruEventDetails->nEventType);
		break;
	}
	return 0;
}

/**
	This function is called by NetworkStack.dll, once simulation end to free the 
	allocated memory for the network.	
*/
_declspec(dllexport) int fn_NetSim_ZRP_Finish()
{
	close_MPR_log();
	return fn_NetSim_ZRP_Finish_F();
}

/**
	This function is called by NetworkStack.dll, while writing the evnt trace 
	to get the sub event as a string.
*/
_declspec (dllexport) char* fn_NetSim_ZRP_Trace(NETSIM_ID nSubEvent)
{
	return fn_NetSim_ZRP_Trace_F(nSubEvent);
}

/**
	This function is called by NetworkStack.dll, while configuring the device 
	for ZRP protocol.	
*/
_declspec(dllexport) int fn_NetSim_ZRP_Configure(void** var)
{
	return fn_NetSim_ZRP_Configure_F(var);
}

/**
	This function is called by NetworkStack.dll, to free the ZRP protocol data.
*/
_declspec(dllexport) int fn_NetSim_ZRP_FreePacket(NetSim_PACKET* pstruPacket)
{
	return fn_NetSim_ZRP_FreePacket_F(pstruPacket);
}

/**
	This function is called by NetworkStack.dll, to copy the ZRP protocol
	details from source packet to destination.
*/
_declspec(dllexport) int fn_NetSim_ZRP_CopyPacket(NetSim_PACKET* pstruDestPacket,NetSim_PACKET* pstruSrcPacket)
{
	return fn_NetSim_ZRP_CopyPacket_F(pstruDestPacket,pstruSrcPacket);
}

/**
This function write the Metrics 	
*/
_declspec(dllexport) int fn_NetSim_ZRP_Metrics()
{
	return 0;
}

/**
This function will return the string to write packet trace heading.
*/
_declspec(dllexport) char* fn_NetSim_ZRP_ConfigPacketTrace()
{
	return "";
}

/**
 This function will return the string to write packet trace.																									
*/
_declspec(dllexport) char* fn_NetSim_ZRP_WritePacketTrace()
{
	return "";
}

int addToZoneList(NODE_ZRP* zrp,NETSIM_IPAddress ip)
{
	ZRP_ZONE* zone=ZRP_ZONE_ALLOC();
	zone->zoneNodeIP=IP_COPY(ip);
	LIST_ADD_LAST((void**)&zrp->zone,zone);
	return 0;
}

bool checkDestInZone(NetSim_PACKET* packet)
{
	NETSIM_IPAddress dest=packet->pstruNetworkData->szDestIP;
	NODE_ZRP* zrp=(NODE_ZRP*)DEVICE_NWROUTINGVAR(pstruEventDetails->nDeviceId);
	ZRP_ZONE* zone=zrp->zone;
	while(zone)
	{
		if(!IP_COMPARE(zone->zoneNodeIP,dest))
			return true;
		zone=(ZRP_ZONE*)LIST_NEXT(zone);
	}
	return false;
}

static void init_MPR_log()
{
	if (get_protocol_log_status("OLSR_MPR_LOG"))
	{
		sprintf(MPR_Log_filename, "%s\\%s", pszIOLogPath, "OLSR_MPR_Log.csv");
		fpRMlog = fopen(MPR_Log_filename, "w");
		if (!fpRMlog)
		{
			fnSystemError("Unable to open %s file", MPR_Log_filename);
			perror(MPR_Log_filename);
		}
		else
		{
			fprintf(fpRMlog, "%s,%s,%s,%s",
				"Time(ms)", "Device Name", "Device ID", "MPR Nodes");
			fprintf(fpRMlog, "\n");
			if (nDbgFlag) fflush(fpRMlog);
		}
	}
}

static void close_MPR_log()
{
	if (fpRMlog)
		fclose(fpRMlog);
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
