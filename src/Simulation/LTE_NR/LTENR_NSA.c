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
#include "LTENR_NSA.h"
#include "LTENR_PHY.h"
#include "LTENR_Core.h"
#pragma endregion

#pragma region SEC_CELL_ADDITION_RESPONSE
void fn_Netsim_NSA_Secondary_Cell_Addition_Response(NETSIM_ID UEID, NETSIM_ID UEIF,
	NETSIM_ID MGNBID, NETSIM_ID MGNBIF,
	NETSIM_ID SGNBID, NETSIM_ID SGNBIF) {

	ptrLTENR_SEC_NODE_ADD_HDR msg = calloc(1, sizeof * msg);
	msg->UEID = UEID;
	msg->UEIF = UEIF;
	msg->MgNBID = MGNBID;
	msg->MgNBIF = MGNBIF;
	msg->SgNBID = SGNBID;
	msg->SgNBIF = SGNBIF;

	ptrLTENR_DC_HDR hdr = calloc(1, sizeof * hdr);
	hdr->msg = msg;
	hdr->msgType = LTENR_MSG_DC_SEC_CELL_ADDITION_RESPONSE;
	NETSIM_IPAddress srcIP = NULL;
	NETSIM_IPAddress destIP = NULL;
	if (!fn_NetSim_LTENR_NSA_IS_CORE_SPLIT_EXISTS(msg->MgNBID, msg->MgNBIF)) {
		srcIP = DEVICE_NWADDRESS(SGNBID,
			fn_NetSim_LTENR_CORE_INTERFACE(SGNBID, nGC_INTERFACE_XN));
		destIP = DEVICE_NWADDRESS(MGNBID,
			fn_NetSim_LTENR_CORE_INTERFACE(MGNBID, nGC_INTERFACE_XN));
	}
	else {
		if (fn_NetSim_LTENR_IS_S1_INTERFACE_EXISTS(SGNBID)) {
			srcIP = DEVICE_NWADDRESS(SGNBID,
				fn_NetSim_LTENR_CORE_INTERFACE(SGNBID, nGC_INTERFACE_LTE_S1));
			destIP = DEVICE_NWADDRESS(MGNBID,
				fn_NetSim_LTENR_CORE_INTERFACE(MGNBID, nGC_INTERFACE_LTE_S1));
		}
		else {
			srcIP = DEVICE_NWADDRESS(SGNBID,
				fn_NetSim_LTENR_CORE_INTERFACE(SGNBID, nGC_INTERFACE_N3));
			destIP = DEVICE_NWADDRESS(MGNBID,
				fn_NetSim_LTENR_CORE_INTERFACE(MGNBID, nGC_INTERFACE_N3));
		}
	}

	LTENR_Handover_Send_Packet(SGNBID, MGNBID, srcIP, destIP, hdr, sizeof * hdr,
		LTENR_MSG_DC_SEC_CELL_ADDITION_RESPONSE,
		strLTENR_MSGTYPE[LTENR_MSG_DC_SEC_CELL_ADDITION_RESPONSE % (MAC_PROTOCOL_LTE_NR * 100)],
		DC_SN_CELL_ADDITION_RESPONSE);
}
void fn_Netsim_NSA_Secondary_Cell_Addition_Response_Recv(ptrLTENR_DC_HDR header) {
	ptrLTENR_SEC_NODE_ADD_HDR msg = header->msg;
	ptrLTENR_UERRC rrc = LTENR_UERRC_GET(msg->UEID, msg->UEIF);
	rrc->ueSNRRCState = UERRC_CONNECTED;
	rrc->ueSNCMState = UE_CM_CONNECTED;
	rrc->ueRRCState = UERRC_CONNECTED;
	rrc->ueCMState = UE_CM_CONNECTED;
	ptrLTENR_PROTODATA data = LTENR_PROTODATA_GET(msg->SgNBID, msg->SgNBIF);
	//path update msg mn to epc or amf
	if (fn_NetSim_LTENR_NSA_IS_CORE_SPLIT_EXISTS(msg->UEID, msg->UEIF) ||
		fn_NetSim_LTENR_NSA_IS_OPTION_X_EXISTS(msg->UEID, msg->UEIF)) {
		if (!fn_NetSim_LTENR_IS_S1_INTERFACE_EXISTS(msg->SgNBID)) {
			NETSIM_ID upfID = fn_NetSim_Stack_GetDeviceId_asName(data->connectedUPFName);
			NETSIM_ID UPFIF = fn_NetSim_LTENR_CORE_INTERFACE(upfID, nGC_INTERFACE_N3);
				iptable_add(IP_WRAPPER_GET(upfID),
					DEVICE_NWADDRESS(msg->UEID, msg->UEIF),
					STR_TO_IP4("255.255.255.255"),
					0,
					DEVICE_NWADDRESS(msg->SgNBID, fn_NetSim_LTENR_CORE_INTERFACE(msg->SgNBID, nGC_INTERFACE_N3)),
					1,
					&DEVICE_NWADDRESS(upfID, fn_NetSim_LTENR_CORE_INTERFACE(upfID, nGC_INTERFACE_N3)),
					&UPFIF,
					1,
					"LTE_NR");
		}
	}

}
#pragma endregion

#pragma region SEC_CELL_ADDITION_REQ
void fn_Netsim_NSA_Secondary_Cell_Addition(NETSIM_ID UEID, NETSIM_ID UEIF,
	NETSIM_ID MGNBID, NETSIM_ID MGNBIF,
	NETSIM_ID SGNBID, NETSIM_ID SGNBIF) {

	ptrLTENR_SEC_NODE_ADD_HDR msg = calloc(1, sizeof * msg);
	msg->UEID = UEID;
	msg->UEIF = UEIF;
	msg->MgNBID = MGNBID;
	msg->MgNBIF = MGNBIF;
	msg->SgNBID = SGNBID;
	msg->SgNBIF = SGNBIF;

	ptrLTENR_DC_HDR hdr = calloc(1, sizeof * hdr);
	hdr->msg = msg;
	hdr->msgType = LTENR_MSG_DC_SEC_CELL_ADDITION_REQUEST;
	NETSIM_IPAddress srcIP = NULL;
	NETSIM_IPAddress destIP = NULL;
	if (!fn_NetSim_LTENR_NSA_IS_CORE_SPLIT_EXISTS(msg->MgNBID, msg->MgNBIF)) {
		 srcIP = DEVICE_NWADDRESS(MGNBID,
			fn_NetSim_LTENR_CORE_INTERFACE(MGNBID, nGC_INTERFACE_XN));
		 destIP = DEVICE_NWADDRESS(SGNBID,
			fn_NetSim_LTENR_CORE_INTERFACE(SGNBID, nGC_INTERFACE_XN));
	}
	else {
		if (fn_NetSim_LTENR_IS_S1_INTERFACE_EXISTS(MGNBID)) {
			srcIP = DEVICE_NWADDRESS(MGNBID,
				fn_NetSim_LTENR_CORE_INTERFACE(MGNBID, nGC_INTERFACE_LTE_S1));
			destIP = DEVICE_NWADDRESS(SGNBID,
				fn_NetSim_LTENR_CORE_INTERFACE(SGNBID, nGC_INTERFACE_LTE_S1));
		}
		else {
			srcIP = DEVICE_NWADDRESS(MGNBID,
				fn_NetSim_LTENR_CORE_INTERFACE(MGNBID, nGC_INTERFACE_N3));
			destIP = DEVICE_NWADDRESS(SGNBID,
				fn_NetSim_LTENR_CORE_INTERFACE(SGNBID, nGC_INTERFACE_N3));
		}
	}

	LTENR_Handover_Send_Packet(MGNBID, SGNBID, srcIP, destIP, hdr, sizeof * hdr,
		LTENR_MSG_DC_SEC_CELL_ADDITION_REQUEST,
		strLTENR_MSGTYPE[LTENR_MSG_DC_SEC_CELL_ADDITION_REQUEST % (MAC_PROTOCOL_LTE_NR * 100)],
		DC_SN_CELL_ADDITION_REQUEST);
}

void fn_Netsim_NSA_Secondary_Cell_Addition_Request_Recv(ptrLTENR_DC_HDR header) {
	ptrLTENR_SEC_NODE_ADD_HDR msg = header->msg;
	ptrLTENR_UERRC rrc = LTENR_UERRC_GET(msg->UEID, msg->UEIF);
	LTENR_ASSOCIATEINFO_ADD(msg->SgNBID, msg->SgNBIF, msg->UEID, msg->UEIF);
	LTENR_PHY_ASSOCIATION(msg->SgNBID, msg->SgNBIF, msg->UEID, msg->UEIF, true);
	LTENR_RLC_ASSOCIATION(msg->SgNBID, msg->SgNBIF, msg->UEID, msg->UEIF, true);
	if (fn_NetSim_LTENR_NSA_IS_CORE_SPLIT_EXISTS(msg->UEID, msg->UEIF) ||
		fn_NetSim_LTENR_NSA_IS_OPTION_X_EXISTS(msg->UEID, msg->UEIF))
		LTENR_PDCP_ASSOCIATION(msg->SgNBID, msg->SgNBIF, msg->UEID, msg->UEIF, true);
	//rrc->SelectedCellID = msg->SgNBID;
	//rrc->SelectedCellIF = msg->SgNBIF;
	fn_NetSim_Stack_AddDeviceTolink(msg->UEID, msg->UEIF, DEVICE_PHYLAYER(msg->SgNBID, msg->SgNBIF)->nLinkId);
	fn_Netsim_NSA_Secondary_Cell_Addition_Response(msg->UEID, msg->UEIF, msg->MgNBID,
		msg->MgNBIF, msg->SgNBID, msg->SgNBIF);
}
#pragma endregion

#pragma region NSA_RECV
void fn_NetSim_LTENR_NSA_RECV() {
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	ptrLTENR_DC_HDR hdr = NULL;
	switch (packet->nControlDataType) {
	case LTENR_MSG_DC_SEC_CELL_ADDITION_REQUEST:
		hdr = LTENR_HANDOVER_HDR_GET_FROM_PACKET(packet, DC_SN_CELL_ADDITION_REQUEST);
		fn_Netsim_NSA_Secondary_Cell_Addition_Request_Recv(hdr);
		LTENR_HANDOVER_HDR_FREE_FROM_PACKET(packet, DC_SN_CELL_ADDITION_REQUEST);
		break;
	case LTENR_MSG_DC_SEC_CELL_ADDITION_RESPONSE:
		hdr = LTENR_HANDOVER_HDR_GET_FROM_PACKET(packet, DC_SN_CELL_ADDITION_RESPONSE);
		fn_Netsim_NSA_Secondary_Cell_Addition_Response_Recv(hdr);
		LTENR_HANDOVER_HDR_FREE_FROM_PACKET(packet, DC_SN_CELL_ADDITION_REQUEST);
		break;
	default:
		break;
	}
}
#pragma endregion

#pragma region NSA_XN_INTERFACE_TO_RLC
void LTENR_NSA_XN_INTERFACE_TO_RAN(NETSIM_ID d, NETSIM_ID in, NetSim_PACKET* packet)
{
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = pstruEventDetails->dEventTime;
	pevent.nDeviceId = d;
	pevent.nDeviceType = DEVICE_TYPE(pevent.nDeviceId);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = in;
	pevent.pPacket = packet;
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(d, in);
	LTENR_SET_SUBEVENT(pd, &pevent, LTENR_SUBEVENT_GENERATE_NSA_XN_TO_RAN);
	fnpAddEvent(&pevent);
}

void LTENR_NSA_XN_INTERFACE_TO_RAN_Handle()
{
	LTENR_CallRLCOut();
}
#pragma endregion

#pragma region NSA_INTI
void fn_NetSim_LTENR_NSA_INIT() {
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENT_GENERATE_NSA_XN_TO_RAN,
		"LTENR_NSA_XN_TO_RAN",
		LTENR_NSA_XN_INTERFACE_TO_RAN_Handle);
	DC_SN_CELL_ADDITION_REQUEST = LTENR_HDR_REGISTER("DC_SEC_CELL_ADDITION_REQUEST", NULL, NULL,
		LTENR_NSA_DC_SN_CELL_ADDITION_REQUEST_FREEHDR, NULL);
	DC_SN_CELL_ADDITION_RESPONSE = LTENR_HDR_REGISTER("DC_SEC_CELL_ADDITION_RESPONSE", NULL, NULL,
		LTENR_NSA_DC_SN_CELL_ADDITION_RESPONSE_FREEHDR, NULL);
}
void LTENR_NSA_DC_SN_CELL_ADDITION_REQUEST_FREEHDR(ptrLTENR_DC_HDR hdr) {
	ptrLTENR_SEC_NODE_ADD_HDR msg = hdr->msg;
	free(msg);
	free(hdr);

}
void LTENR_NSA_DC_SN_CELL_ADDITION_RESPONSE_FREEHDR(ptrLTENR_DC_HDR hdr) {
	ptrLTENR_SEC_NODE_ADD_HDR msg = hdr->msg;
	free(msg);
	free(hdr);

}
#pragma endregion

#pragma region NW_LAYER_XN
void fn_NetSim_LTENR_SNED_TO_XN_INTERFACE(NetSim_PACKET* packet, NETSIM_ID d, NETSIM_ID in, 
	NETSIM_ID r, NETSIM_ID rin, char*gateway, char* nexthop, bool flag)
{
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = pstruEventDetails->dEventTime;
	pevent.dPacketSize = packet->pstruNetworkData->dPacketSize;
	pevent.nDeviceId = d;
	pevent.nInterfaceId = fn_NetSim_LTENR_CORE_INTERFACE(pevent.nDeviceId, nGC_INTERFACE_XN);
	pevent.nDeviceType = DEVICE_TYPE(pevent.nDeviceId);
	pevent.nEventType = NETWORK_OUT_EVENT;
	pevent.nPacketId = packet->nPacketId;
	if (packet->pstruAppData)
	{
		pevent.nApplicationId = packet->pstruAppData->nApplicationId;
		pevent.nSegmentId = packet->pstruAppData->nSegmentId;
	}
	ptrLTENR_XN_HDR hdr = calloc(1, sizeof * hdr);
	hdr->gnbID = d;
	hdr->gnbIF = in;
	hdr->UEID = r;
	hdr->UEIF = rin;
	hdr->flag = flag;
	hdr->gateway = gateway;
	hdr->nexthop = nexthop;
	hdr->msg = packet->pstruMacData->Packet_MACProtocol;
	packet->pstruNetworkData->Packet_NetworkProtocol = hdr;
	packet->pstruMacData->Packet_MACProtocol = NULL;
	pevent.pPacket = packet;
	pevent.nProtocolId = NW_PROTOCOL_IPV4;
	fnpAddEvent(&pevent);

}
#pragma endregion

#pragma region SPLITTING_ALGO
NETSIM_ID fn_NetSim_LTENR_NSA_Splitting_Alogrithm_For_EPC(NETSIM_ID dest) {
	NETSIM_ID in = fn_NetSim_Get_LTENR_INTERFACE_ID_FROM_DEVICE_ID(dest);
	in = fn_NetSim_LTENR_MASTER_INTEFACE_FOR_UE(dest, in);
	ptrLTENR_UERRC rrc = LTENR_UERRC_GET(dest, in);

	if (!rrc->ueRRCState == UERRC_CONNECTED)
		return 0;

	ptrLTENR_PROTODATA data = LTENR_PROTODATA_GET(dest, in);
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	if (data->isDCEnable) {
		if (data->NSA_MODE == NSA_MODE_OPTION3A) {
			if (packet->nQOS == QOS_GBR)//GRB LTE
				return rrc->SelectedCellID;
			else if(packet->nQOS != QOS_GBR && rrc->SNID)
				return rrc->SNID;
			else
				return rrc->SelectedCellID;
		}
		if (data->NSA_MODE == NSA_MODE_OPTION3) {
			return rrc->SelectedCellID;
		}
	}


	if(!fn_NetSim_LTENR_NSA_IS_OPTION_X_EXISTS(dest, in)){
		if (rrc->isAlternativePacket) {
			rrc->isAlternativePacket = false;
			if (rrc->SNID)
				return rrc->SNID;
			else
				return rrc->SelectedCellID;
		}
		else {
			rrc->isAlternativePacket = true;
			if (rrc->SelectedCellID)
				return rrc->SelectedCellID;
			else
				return rrc->SNID;
		}
	}
	else {
		if (rrc->SNID)
			return rrc->SNID;
		else
			return rrc->SelectedCellID;
	}
}
#pragma endregion