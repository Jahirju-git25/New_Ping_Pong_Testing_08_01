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
* Author:    Sreerang                                                              *
*                                                                                  *
* ---------------------------------------------------------------------------------*/
#include "Application.h"

int fn_NetSim_Application_ConfigureInteractiveGamingTraffic(ptrAPPLICATION_INFO appInfo, void* xmlNetSimNode)
{
	char* szVal;
	APP_GAMING_INFO* info = calloc(1, sizeof * info);
	void* xmlChild;
	appInfo->appData = info;
	xmlChild = fn_NetSim_xmlGetChildElement(xmlNetSimNode, "UL_PACKET_SIZE", 0);
	if (xmlChild)
	{
		szVal = fn_NetSim_xmlConfig_GetVal(xmlChild, "A_VALUE", 1);
		if (szVal)
			info->ul_a_size = atof(szVal);
		free(szVal);
		szVal = fn_NetSim_xmlConfig_GetVal(xmlChild, "B_VALUE", 1);
		if (szVal)
			info->ul_b_size = atof(szVal);
		free(szVal);
	}
	else
		return 0;
	xmlChild = fn_NetSim_xmlGetChildElement(xmlNetSimNode, "UL_INTER_ARRIVAL_TIME", 0);
	if (xmlChild)
	{
		szVal = fn_NetSim_xmlConfig_GetVal(xmlChild, "B_VALUE", 1);
		if (szVal)
			info->ul_b_IAT = atof(szVal) * MICROSECOND;
		free(szVal);
	}
	else
		return 0;
	xmlChild = fn_NetSim_xmlGetChildElement(xmlNetSimNode, "DL_PACKET_SIZE", 0);
	if (xmlChild)
	{
		szVal = fn_NetSim_xmlConfig_GetVal(xmlChild, "A_VALUE", 1);
		if (szVal)
			info->dl_a_size = atof(szVal);
		free(szVal);
		szVal = fn_NetSim_xmlConfig_GetVal(xmlChild, "B_VALUE", 1);
		if (szVal)
			info->dl_b_size = atof(szVal);
		free(szVal);
	}
	else
		return 0;
	xmlChild = fn_NetSim_xmlGetChildElement(xmlNetSimNode, "DL_INTER_ARRIVAL_TIME", 0);
	if (xmlChild)
	{
		szVal = fn_NetSim_xmlConfig_GetVal(xmlChild, "A_VALUE", 1);
		if (szVal)
			info->dl_a_IAT = atof(szVal) * MICROSECOND;
		free(szVal);
		szVal = fn_NetSim_xmlConfig_GetVal(xmlChild, "B_VALUE", 1);
		if (szVal)
			info->dl_b_IAT = atof(szVal) * MICROSECOND;
		free(szVal);
	}
	else
		return 0;
	return 1;
}

int fn_NetSim_Application_StartInteractiveGamingULAPP(ptrAPPLICATION_INFO appInfo, double time)
{
	double ulpacketSize = 0.0;
	double ularrivalTime = 0.0;
	APP_GAMING_INFO* info = appInfo->appData;
	double R = NETSIM_RAND_01();
	if (appInfo->dEndTime <= time)
		return 0;

	fnCreatePort(appInfo);

	NETSIM_ID nSource = appInfo->sourceList[0];
	NETSIM_ID nDestination = appInfo->destList[0];

	//Create the socket buffer
	Interactive_Gaming_createUL_socket(appInfo, nSource, nDestination);

	//Generate the app start event
	ularrivalTime = time;
	ulpacketSize = roundl(info->ul_a_size - info->ul_b_size * (logl(-logl(R))));
	if (ularrivalTime < 0)
		return fn_NetSim_Application_StartInteractiveGamingULAPP(appInfo, time);
	if (ulpacketSize <= 0)
		return fn_NetSim_Application_StartInteractiveGamingULAPP(appInfo, time);

	if (appInfo->dEndTime <= ularrivalTime)
		return 0;

	pstruEventDetails->dEventTime = ularrivalTime;

	pstruEventDetails->nApplicationId = appInfo->id;
	pstruEventDetails->nDeviceId = nDestination;
	pstruEventDetails->nDeviceType = DEVICE_TYPE(nDestination);
	pstruEventDetails->nEventType = TIMER_EVENT;
	pstruEventDetails->nInterfaceId = 0;
	pstruEventDetails->nPacketId = ++info->ul_id;
	pstruEventDetails->nProtocolId = PROTOCOL_APPLICATION;
	pstruEventDetails->nSegmentId = 0;
	pstruEventDetails->nSubEventType = event_APP_START;
	pstruEventDetails->pPacket = fn_NetSim_Application_GeneratePacket(appInfo,
		pstruEventDetails->dEventTime,
		nDestination,
		1,
		&nSource,
		pstruEventDetails->nPacketId,
		appInfo->nAppType,
		appInfo->qos,
		appInfo->destPort,
		appInfo->sourcePort);
	pstruEventDetails->dPacketSize = min(ulpacketSize, fn_NetSim_Stack_GetMSS(pstruEventDetails->pPacket) - 1);
	pstruEventDetails->szOtherDetails = appInfo;
	fnpAddEvent(pstruEventDetails);
	return 0;
}
int fn_NetSim_Application_StartInteractiveGamingDLAPP(ptrAPPLICATION_INFO appInfo, double time)
{
	double dlarrivalTime = 0.0;
	double dlpacketSize = 0.0;
	APP_GAMING_INFO* info = appInfo->appData;
	double R = NETSIM_RAND_01();
	double B = NETSIM_RAND_RN(info->ul_b_IAT, 0);
	if (appInfo->dEndTime <= time)
		return 0;

	NETSIM_ID nSource = appInfo->sourceList[0];
	NETSIM_ID nDestination = appInfo->destList[0];

	//Create the socket buffer
	Interactive_Gaming_createDL_socket(appInfo, nSource, nDestination);
	//Generate the app start event
	dlarrivalTime = time + B;
	dlpacketSize = roundl(info->dl_a_size - info->dl_b_size * (logl(-logl(R))));

	if (dlarrivalTime < 0)
		return fn_NetSim_Application_StartInteractiveGamingDLAPP(appInfo, time);
	if (dlpacketSize <= 0)
		return fn_NetSim_Application_StartInteractiveGamingDLAPP(appInfo, time);

	if (appInfo->dEndTime <= dlarrivalTime)
		return 0;

	pstruEventDetails->dEventTime = dlarrivalTime;

	pstruEventDetails->nApplicationId = appInfo->id;
	pstruEventDetails->nDeviceId = nSource;
	pstruEventDetails->nDeviceType = DEVICE_TYPE(nSource);
	pstruEventDetails->nEventType = TIMER_EVENT;
	pstruEventDetails->nInterfaceId = 0;
	pstruEventDetails->nPacketId = ++info->dl_id;
	pstruEventDetails->nProtocolId = PROTOCOL_APPLICATION;
	pstruEventDetails->nSegmentId = 0;
	pstruEventDetails->nSubEventType = event_APP_START;
	pstruEventDetails->pPacket = fn_NetSim_Application_GeneratePacket(appInfo,
		pstruEventDetails->dEventTime,
		nSource,
		1,
		&nDestination,
		pstruEventDetails->nPacketId,
		appInfo->nAppType,
		appInfo->qos,
		appInfo->sourcePort,
		appInfo->destPort);
	pstruEventDetails->dPacketSize = min(dlpacketSize, fn_NetSim_Stack_GetMSS(pstruEventDetails->pPacket) - 1);
	pstruEventDetails->szOtherDetails = appInfo;
	fnpAddEvent(pstruEventDetails);
	return 0;
}
int fn_NetSim_Application_InteractiveGamingUL_GenerateNextPacket(ptrAPPLICATION_INFO pstruappinfo, NetSim_PACKET* pstruPacket, double time)
{
	NETSIM_ID nSourceId = get_first_dest_from_packet(pstruPacket);
	NETSIM_ID nDestinationId = pstruPacket->nSourceId;
	APP_GAMING_INFO* info = pstruappinfo->appData;
	double ulpacketSize = 0.0;
	double ularrivalTime = 0.0;
	double R = NETSIM_RAND_01();
	ularrivalTime = info->ul_b_IAT;
	ulpacketSize = roundl(info->ul_a_size - info->ul_b_size * (logl(-logl(R))));
	if (ularrivalTime <= 0)
		return fn_NetSim_Application_InteractiveGamingUL_GenerateNextPacket(pstruappinfo, pstruPacket, time);
	if (ulpacketSize <= 0)
		return fn_NetSim_Application_InteractiveGamingUL_GenerateNextPacket(pstruappinfo, pstruPacket, time);
	if (pstruappinfo->dEndTime <= time + ularrivalTime)
		return 0;
	pstruEventDetails->dEventTime = time + ularrivalTime;
	pstruEventDetails->dPacketSize = ulpacketSize;
	pstruEventDetails->nApplicationId = pstruappinfo->id;
	pstruEventDetails->nDeviceId = nDestinationId;
	pstruEventDetails->nDeviceType = DEVICE_TYPE(nDestinationId);
	pstruEventDetails->nEventType = APPLICATION_OUT_EVENT;
	pstruEventDetails->nInterfaceId = 0;
	pstruEventDetails->nPacketId = ++info->ul_id;
	pstruEventDetails->nProtocolId = PROTOCOL_APPLICATION;
	pstruEventDetails->nSubEventType = 0;
	pstruEventDetails->pPacket = fn_NetSim_Application_GeneratePacket(pstruappinfo,
		pstruEventDetails->dEventTime,
		nDestinationId,
		1,
		&nSourceId,
		pstruEventDetails->nPacketId,
		TRAFFIC_INTERACTIVE_GAMING,
		pstruappinfo->qos,
		pstruappinfo->destPort,
		pstruappinfo->sourcePort);
	pstruEventDetails->szOtherDetails = pstruappinfo;
	fnpAddEvent(pstruEventDetails);
	return 1;
}

int fn_NetSim_Application_InteractiveGamingDL_GenerateNextPacket(ptrAPPLICATION_INFO pstruappinfo, NetSim_PACKET* pstruPacket, double time)
{
	NETSIM_ID nSourceId = pstruPacket->nSourceId;
	NETSIM_ID nDestinationId = get_first_dest_from_packet(pstruPacket);
	APP_GAMING_INFO* info = pstruappinfo->appData;
	double dlpacketSize = 0.0;
	double dlarrivalTime = 0.0;
	double R = NETSIM_RAND_01();
	dlarrivalTime = info->dl_a_IAT - info->dl_b_IAT * (logl(-logl(R)));
	dlpacketSize = roundl(info->dl_a_size - info->dl_b_size * (logl(-logl(R))));
	if (dlarrivalTime <= 0.0)
		return fn_NetSim_Application_InteractiveGamingDL_GenerateNextPacket(pstruappinfo, pstruPacket, time);
	if (dlpacketSize <= 0.0)
		return fn_NetSim_Application_InteractiveGamingDL_GenerateNextPacket(pstruappinfo, pstruPacket, time);
	if (pstruappinfo->dEndTime <= time + dlarrivalTime)
		return 0;
	pstruEventDetails->dEventTime = time + dlarrivalTime;
	pstruEventDetails->dPacketSize = dlpacketSize;
	pstruEventDetails->nApplicationId = pstruappinfo->id;
	pstruEventDetails->nDeviceId = nSourceId;
	pstruEventDetails->nDeviceType = DEVICE_TYPE(nSourceId);
	pstruEventDetails->nEventType = APPLICATION_OUT_EVENT;
	pstruEventDetails->nInterfaceId = 0;
	pstruEventDetails->nPacketId = ++info->dl_id;
	pstruEventDetails->nProtocolId = PROTOCOL_APPLICATION;
	pstruEventDetails->nSubEventType = 0;
	pstruEventDetails->pPacket = fn_NetSim_Application_GeneratePacket(pstruappinfo,
		pstruEventDetails->dEventTime,
		nSourceId,
		1,
		&nDestinationId,
		pstruEventDetails->nPacketId,
		TRAFFIC_INTERACTIVE_GAMING,
		pstruappinfo->qos,
		pstruappinfo->sourcePort,
		pstruappinfo->destPort);
	pstruEventDetails->szOtherDetails = pstruappinfo;
	fnpAddEvent(pstruEventDetails);
	return 1;
}
