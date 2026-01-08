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

#pragma region HEADER_FILES
#include "stdafx.h"
#include "LTENR_MAC.h"
#include "LTENR_NetworkSlicing.h"
#pragma endregion

#pragma region MAC_INIT
void fn_NetSim_LTENR_MAC_Init()
{

}

void fn_NetSim_LTENR_UEMAC_Init(NETSIM_ID ueId, NETSIM_ID ueIf)
{
}

static LTENR_GnbSchedulingType fn_NetSim_LTENR_SchedulingType(char* var) {
	if (!_stricmp(var, "ROUND_ROBIN"))
		return LTENR_MAC_SCHEDULING_ROUNDROBIN;
	else if (!_stricmp(var, "PROPORTIONAL_FAIR"))
		return LTENR_MAC_SCHEDULING_PROPORTIONALFAIR;
	else if (!_stricmp(var, "MAX_THROUGHPUT"))
		return LTENR_MAC_SCHEDULING_MAXTHROUGHTPUT;
	else if (!_stricmp(var, "FAIR_SCHEDULING"))
		return LTENR_MAC_SCHEDULING_FAIR_SCHEDULING;
	else {
		fnNetSimError("Unknown Scheduling Type");
		return LTENR_MAC_SCHEDULING_ROUNDROBIN;
	}
}

void fn_NetSim_LTENR_GNBMAC_Init(NETSIM_ID gnbId, NETSIM_ID gnbIf)
{
	ptrLTENR_GNBMAC mac = calloc(1, sizeof * mac);
	mac->gnbId = gnbId;
	mac->gnbIf = gnbIf;
	LTENR_GNBMAC_SET(gnbId, gnbIf, mac);

	if (fn_NetSim_LTENR_IS_S1_INTERFACE_EXISTS(gnbId)) {
		LTENR_GNB_SETEPC(gnbId, gnbIf, &mac->epcId, &mac->epcIf);
	}
}
#pragma endregion

#pragma region MAC_TEMP_FUNCTION

void macOut()
{
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	NETSIM_ID rin = LTENR_FIND_ASSOCIATEINTERFACE(d, in, packet->nReceiverId);

	if (LTENR_IS_UPLANE_MSG(packet))
	{
		if (isGNB(d, in)) fn_NetSim_LTENR_HARQ_StoreDLPacket();
		else fn_NetSim_LTENR_HARQ_StoreULPacket();
	}
	else
	{
		pstruEventDetails->nEventType = PHYSICAL_OUT_EVENT;
		fnpAddEvent(pstruEventDetails);
	}
}

void macIn()
{
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	NETSIM_ID remote;
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	//ptrLTENR_MSG msg = packet->pstruMacData->Packet_MACProtocol;
	NetSim_EVENTDETAILS pevent;
	memcpy(&pevent, pstruEventDetails, sizeof pevent);

	while (packet)
	{
		memcpy(pstruEventDetails, &pevent, sizeof * pstruEventDetails);
		pstruEventDetails->pPacket = packet;
		packet = packet->pstruNextPacket;
		pstruEventDetails->pPacket->pstruNextPacket = NULL;
		remote = pstruEventDetails->pPacket->nTransmitterId;
		NETSIM_ID rin = LTENR_FIND_ASSOCIATEINTERFACE(d, in, remote);

		//Association might change due to handover. Check for valid association
		if (rin == 0)
		{
			pstruEventDetails->pPacket->nPacketStatus = PacketStatus_Dropped;
			pstruEventDetails->pPacket->pstruPhyData->dEndTime = ldEventTime;
			fn_NetSim_WritePacketTrace(pstruEventDetails->pPacket);
			if((isLTENRControlPacket(pstruEventDetails->pPacket)))
				LTENR_CallRLCIn();
			else
			fn_NetSim_Packet_FreePacket(pstruEventDetails->pPacket);
		}
		else
		{
			pstruEventDetails->pPacket->pstruPhyData->dEndTime = ldEventTime;
			pstruEventDetails->pPacket->nPacketStatus = PacketStatus_NoError;
			fn_NetSim_WritePacketTrace(pstruEventDetails->pPacket);
			LTENR_CallRLCIn();
		}
	}
}
#pragma endregion

#pragma region MAC_SCHEDULERINTERFACE
ptrLTENR_UESCHEDULERINFO LTENR_MACScheduler_FindInfoForUE(ptrLTENR_SCHEDULERINFO si,
	NETSIM_ID d, NETSIM_ID in,
	bool isUplink)
{
	ptrLTENR_UESCHEDULERINFO info = isUplink ? si->uplinkInfo : si->downlinkInfo;
	while (info)
	{
		if (info->ueId == d && info->ueIf == in)
			return info;
		info = info->next;
	}
	return NULL;
}

static void LTENR_MACScheduler_RemoveInfoForUE(ptrLTENR_SCHEDULERINFO si,
	NETSIM_ID d, NETSIM_ID in,
	bool isUplink)
{
	ptrLTENR_UESCHEDULERINFO info = isUplink ? si->uplinkInfo : si->downlinkInfo;
	ptrLTENR_UESCHEDULERINFO p = NULL;
	while (info)
	{
		if (info->ueId == d && info->ueIf == in)
		{
			if (!p)
			{
				if (isUplink)
					si->uplinkInfo = info->next;
				else
					si->downlinkInfo = info->next;

				free(info);
				break;
			}
			else
			{
				p->next = info->next;
				free(info);
				break;
			}
		}
		p = info;
		info = info->next;
	}	
}

static ptrLTENR_UESCHEDULERINFO LTENR_MACScheduler_InitInfoForUE(ptrLTENR_SCHEDULERINFO si,
																 NETSIM_ID d, NETSIM_ID in,
																 bool isUplink)
{
	ptrLTENR_UESCHEDULERINFO info = calloc(1, sizeof * info);
	info->ueId = d;
	info->ueIf = in;
	ptrLTENR_UESCHEDULERINFO l = isUplink ? si->uplinkInfo : si->downlinkInfo;
	if (l)
	{
		while (l->next)
			l = l->next;
		l->next = info;
	}
	else
	{
		if (isUplink) si->uplinkInfo = info;
		else si->downlinkInfo = info;
	}
	return info;
}

static void LTENR_MACScheduler_UpdateInfoForUE(ptrLTENR_GNBMAC mac, ptrLTENR_GNBPHY phy,
											   ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo,
											   ptrLTENR_UESCHEDULERINFO info, bool isUplink, int CA_ID)
{
	ptrLTENR_UEPHY uePhy = LTENR_UEPHY_GET(assocInfo->ueId, assocInfo->ueIf);
	UINT layerCount = LTENR_PHY_GET_LAYER_COUNT(uePhy, isUplink);
	
	UINT64 bitsPerPRB = 0,TBS = 0;

	if (fn_NetSim_LTENR_HARQ_GetNDIFlag(assocInfo, isUplink, CA_ID) == false)
	{
		info->NDI = false;
		info->PRBReqdForHARQRetransmission = fn_NetSim_LTENR_HARQ_GetPRBReqdForRetx(assocInfo, isUplink, CA_ID);
		info->bitsPerPRB = 0;
		return;
	}

	info->NDI = true;
	info->PRBReqdForHARQRetransmission = 0;

	for (UINT i = 0; i < layerCount; i++)
	{
		ptrLTENR_AMCINFO amc = isUplink ? assocInfo->uplinkAMCInfo[CA_ID][i] : assocInfo->downlinkAMCInfo[CA_ID][i];
		if (amc->cqiTable.CQIIndex != 0) {
			TBS = LTENR_calculateTBSSize(phy, 1, amc->mcsTable, CA_ID);
			bitsPerPRB += TBS;
			info->bitsPerPRBLayer[i] = TBS;
		}
	}

	ptrLTENR_SCHEDULERINFO si = mac->schedulerInfo[CA_ID];

	if (fn_NetSim_LTENR_RRC_isActive(mac->gnbId, mac->gnbIf, assocInfo->ueId, assocInfo->ueIf)) {
		info->bitsPerPRB = bitsPerPRB;
		//assert(info->bitsPerPRB != 0);
		si->isUERRCSetUpCompleted = true;
	}
	else info->bitsPerPRB = 0;
	
	if (isUplink)
		info->bufferSize = fn_NetSim_LTENR_RLC_BufferStatusNotificaton(assocInfo->ueId, assocInfo->ueIf,
																	   mac->gnbId, mac->gnbIf,
																	   LTENR_LOGICALCHANNEL_DTCH);
	else
		info->bufferSize = fn_NetSim_LTENR_RLC_BufferStatusNotificaton(mac->gnbId, mac->gnbIf,
																	   assocInfo->ueId, assocInfo->ueIf,
																	   LTENR_LOGICALCHANNEL_DTCH);
}

static void LTENR_MAC_AddSchedulerStausForUE(ptrLTENR_GNBMAC mac, ptrLTENR_GNBPHY phy,
											 ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo, ptrLTENR_SCHEDULERINFO si, int CA_ID)
{
	ptrLTENR_UESCHEDULERINFO uinfo = LTENR_MACScheduler_FindInfoForUE(si, assocInfo->ueId, assocInfo->ueIf, true);
	ptrLTENR_UESCHEDULERINFO dinfo = LTENR_MACScheduler_FindInfoForUE(si, assocInfo->ueId, assocInfo->ueIf, false);
	if (!uinfo)
		uinfo = LTENR_MACScheduler_InitInfoForUE(si, assocInfo->ueId, assocInfo->ueIf, true);
	if (!dinfo)
		dinfo = LTENR_MACScheduler_InitInfoForUE(si, assocInfo->ueId, assocInfo->ueIf, false);

	if(si->slotType == SLOT_UPLINK)
		LTENR_MACScheduler_UpdateInfoForUE(mac, phy, assocInfo, uinfo, true, CA_ID);
	else if(si->slotType == SLOT_DOWNLINK)
		LTENR_MACScheduler_UpdateInfoForUE(mac, phy, assocInfo, dinfo, false, CA_ID);

	if (si->slotType == SLOT_DOWNLINK) si->TotalPRBReqdForHARQRetransmission += dinfo->PRBReqdForHARQRetransmission;
	else si->TotalPRBReqdForHARQRetransmission += uinfo->PRBReqdForHARQRetransmission;
} 

static void LTENR_MAC_InitSchedulerStatus(ptrLTENR_GNBMAC mac)
{
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(mac->gnbId, mac->gnbIf);
	int CA_ID = phy->currentFrameInfo->Current_CA_ID;
	ptrLTENR_CA ca = phy->spectrumConfig->CA[CA_ID];
	ptrLTENR_SPECTRUMCONFIG sc = phy->spectrumConfig;
	ptrLTENR_SCHEDULERINFO info = NULL;

	if (mac->schedulerInfo[CA_ID])
	{
		info = mac->schedulerInfo[CA_ID];
		info->slotType = phy->currentFrameInfo->slotType;

		ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(mac->gnbId, mac->gnbIf);
		info->schedulingType = fn_NetSim_LTENR_SchedulingType(pd->schedulingType);
		info->alpha = pd->EWMA_Learning_Rate;
	}
	else
	{
		info = calloc(1, sizeof * info);
		mac->schedulerInfo[CA_ID] = info;
		info->gnbId = mac->gnbId;
		info->gnbIf = mac->gnbIf;
		info->PRBCount = ca->PRBCount;
		info->OH_DL = ca->OverheadDL;
		info->OH_UL = ca->OverheadUL;
		info->slotType = phy->currentFrameInfo->slotType;

		ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(mac->gnbId, mac->gnbIf);
		info->schedulingType = fn_NetSim_LTENR_SchedulingType(pd->schedulingType);
	}

	info->TotalPRBReqdForHARQRetransmission = 0;

	ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo = phy->associatedUEPhyInfo;
	while (assocInfo)
	{
		if (assocInfo->isAssociated)
			LTENR_MAC_AddSchedulerStausForUE(mac, phy, assocInfo, info, CA_ID);
		else
		{
			ptrLTENR_UESCHEDULERINFO uinfo = LTENR_MACScheduler_FindInfoForUE(info, assocInfo->ueId, assocInfo->ueIf, true);
			ptrLTENR_UESCHEDULERINFO dinfo = LTENR_MACScheduler_FindInfoForUE(info, assocInfo->ueId, assocInfo->ueIf, false);
			if (uinfo)
				LTENR_MACScheduler_RemoveInfoForUE(info, assocInfo->ueId, assocInfo->ueIf, true);
			if (dinfo)
				LTENR_MACScheduler_RemoveInfoForUE(info, assocInfo->ueId, assocInfo->ueIf, false);
		}
		LTENR_ASSOCIATEDUEPHYINFO_NEXT(assocInfo);	
	}
}
#pragma endregion

#pragma region MAC_RLC_INTERFACE
static void LTENR_MAC_NotifyRLCForTransmission_forControl(NETSIM_ID d, NETSIM_ID in,
														  NETSIM_ID r, NETSIM_ID rin)
{
	fn_NetSim_LTENR_RLC_TransmissionStatusNotification(d, in,
													   r, rin,
													   0,
													   LTENR_LOGICALCHANNEL_BCCH);
	fn_NetSim_LTENR_RLC_TransmissionStatusNotification(d, in,
													   r, rin,
													   0,
													   LTENR_LOGICALCHANNEL_CCCH);
	fn_NetSim_LTENR_RLC_TransmissionStatusNotification(d, in,
													   r, rin,
													   0,
													   LTENR_LOGICALCHANNEL_DCCH);
	fn_NetSim_LTENR_RLC_TransmissionStatusNotification(d, in,
													   r, rin,
													   0,
													   LTENR_LOGICALCHANNEL_PCCH);
}

static void LTENR_MAC_NotifyRLCForTransmission_forDTCH(NETSIM_ID d, NETSIM_ID in,
													   ptrLTENR_UESCHEDULERINFO si, bool isUplink)
{
	while (si)
	{
		if (isUplink)
			fn_NetSim_LTENR_RLC_TransmissionStatusNotification(si->ueId, si->ueIf,
															   d, in, (UINT)(si->allocatedPRBCount*si->bitsPerPRB/8.0),
															   LTENR_LOGICALCHANNEL_DTCH);
		else
			fn_NetSim_LTENR_RLC_TransmissionStatusNotification(d, in,
															   si->ueId, si->ueIf,
				(UINT)(si->allocatedPRBCount * si->bitsPerPRB / 8.0),
															   LTENR_LOGICALCHANNEL_DTCH);
		si = si->next;
	}
}

void LTENR_MAC_NotifyRLCForTransmission(ptrLTENR_GNBMAC mac)
{
	NETSIM_ID gnbId = mac->gnbId;
	NETSIM_ID gnbIf = mac->gnbIf;
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);
	int CA_ID = phy->currentFrameInfo->Current_CA_ID;
	ptrLTENR_SCHEDULERINFO si = mac->schedulerInfo[CA_ID];
	
	LTENR_SLOTTYPE slotType = si->slotType;

	ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo = phy->associatedUEPhyInfo;

	// bwp modify
	while (assocInfo)
	{
		if (assocInfo->isAssociated)
		{
			if (slotType == SLOT_UPLINK || slotType == SLOT_MIXED)
				LTENR_MAC_NotifyRLCForTransmission_forControl(assocInfo->ueId, assocInfo->ueIf,
					gnbId, gnbIf);

			if (slotType == SLOT_DOWNLINK || slotType == SLOT_MIXED)
				LTENR_MAC_NotifyRLCForTransmission_forControl(gnbId, gnbIf,
					assocInfo->ueId, assocInfo->ueIf);
		}
		LTENR_ASSOCIATEDUEPHYINFO_NEXT(assocInfo);
	}

	if (slotType == SLOT_DOWNLINK || slotType == SLOT_MIXED)
		LTENR_MAC_NotifyRLCForTransmission_forControl(gnbId, gnbIf,
			0, 0);//for rrc MIB and SIB1 broadcast msg

	if (slotType == SLOT_UPLINK || slotType == SLOT_MIXED)
		LTENR_MAC_NotifyRLCForTransmission_forDTCH(gnbId, gnbIf, si->uplinkInfo, true);

	if (slotType == SLOT_DOWNLINK || slotType == SLOT_MIXED)
		LTENR_MAC_NotifyRLCForTransmission_forDTCH(gnbId, gnbIf, si->downlinkInfo, false);
}
#pragma endregion

#pragma region MAC_PHY_INTERFACE
void LTENR_NotifyMACForStartingSlot()
{
	NETSIM_ID gnbId = pstruEventDetails->nDeviceId;
	NETSIM_ID gnbIf = pstruEventDetails->nInterfaceId;
	ptrLTENR_GNBMAC mac = LTENR_GNBMAC_GET(gnbId, gnbIf);
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);
	int CA_ID = phy->currentFrameInfo->Current_CA_ID;
	
	fn_NetSim_LTENR_HARQ_HandleSlotStart(gnbId, gnbIf, CA_ID);

	LTENR_MAC_InitSchedulerStatus(mac);
	
	LTENR_PRB_Scheduler(mac->schedulerInfo[CA_ID]);

	fn_NetSim_LTENR_HARQ_AllocateCBG(gnbId, gnbIf, CA_ID);

	//print_ltenr_log("Allocated PRB = %d and CA_ID= %d\n", mac->schedulerInfo[CA_ID]->downlinkInfo->allocatedPRBCount, CA_ID);

	//notify RLC for transmission status
	LTENR_MAC_NotifyRLCForTransmission(mac);

	fn_NetSim_LTENR_HARQ_Transmit(gnbId, gnbIf, CA_ID);
}
#pragma endregion
