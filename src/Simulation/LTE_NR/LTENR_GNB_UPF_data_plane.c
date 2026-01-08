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
* Author:   Kumar Gaurav		                                                    *
*										                                            *
* ----------------------------------------------------------------------------------*/

#pragma region HEADER_FILES
#include "stdafx.h"
#include "LTENR_SDAP.h"
#include "LTENR_GTPU.h"
#pragma endregion

#pragma region GNB_OUT
void LTENR_GNB_UPF_DATA_PLANE_FORWARDTOOTHERINTERFACE(NETSIM_ID d, NETSIM_ID in,
	NetSim_PACKET* packet) {
	ptrLTENR_GTPU_HEADER gtpu = fn_NetSim_LTENR_GTPU_DEFAULT_HEADER(packet);

	bool isCopy = false;
	if (DEVICE(d)->nNumOfInterface > 2) isCopy = true;
	NetSim_PACKET* p = packet;
	NETSIM_ID outIn = fn_NetSim_LTENR_CORE_INTERFACE(d, nGC_INTERFACE_N3);
	if (isCopy) p = fn_NetSim_Packet_CopyPacket(packet);
	p->pstruNetworkData->dOverhead -= IPV4_HEADER_SIZE;
	p->pstruNetworkData->dPacketSize -= IPV4_HEADER_SIZE;
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = pstruEventDetails->dEventTime;
	pevent.dPacketSize = p->pstruNetworkData->dPacketSize;
	if (p->pstruAppData)
	{
		pevent.nApplicationId = p->pstruAppData->nApplicationId;
		pevent.nSegmentId = p->pstruAppData->nSegmentId;
	}
	pevent.nDeviceId = d;
	pevent.nInterfaceId = outIn;
	pevent.nDeviceType = DEVICE_TYPE(d);
	pevent.nEventType = NETWORK_OUT_EVENT;
	pevent.nPacketId = p->nPacketId;
	pevent.pPacket = p;
	pevent.nProtocolId = NW_PROTOCOL_IPV4;
	fnpAddEvent(&pevent);

	p->pstruMacData->szSourceMac = DEVICE_HWADDRESS(d, outIn);
	if (isCopy) fn_NetSim_Packet_FreePacket(packet);
}
#pragma endregion

#pragma region GET_GTPU_HEADER
void fn_NetSim_LTENR_GNB_UPF_DATA_PLANE_RECV(NetSim_PACKET* packet) {
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	ptrLTENR_GTPU_HEADER gtpu = packet->pstruAppData->Packet_AppProtocol;
	packet->pstruAppData->Packet_AppProtocol = NULL;
	memcpy(packet->pstruAppData, gtpu->pstruAppData, sizeof * packet->pstruAppData);

	bool isCopy = false;
	if (DEVICE(d)->nNumOfInterface > 2) isCopy = true;
	NetSim_PACKET* p = packet;
	NETSIM_ID outIn = fn_NetSim_LTENR_CORE_INTERFACE(d, nGC_INTERFACE_N3);
	if (isCopy) p = fn_NetSim_Packet_CopyPacket(packet);
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = pstruEventDetails->dEventTime;
	pevent.dPacketSize = p->pstruNetworkData->dPacketSize;
	if (p->pstruAppData)
	{
		pevent.nApplicationId = p->pstruAppData->nApplicationId;
		pevent.nSegmentId = p->pstruAppData->nSegmentId;
	}
	pevent.nDeviceId = d;
	pevent.nInterfaceId = outIn;
	pevent.nDeviceType = DEVICE_TYPE(d);
	pevent.nEventType = NETWORK_OUT_EVENT;
	pevent.nPacketId = p->nPacketId;
	pevent.pPacket = p;
	pevent.nProtocolId = NW_PROTOCOL_IPV4;
	fnpAddEvent(&pevent);
}
#pragma endregion