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
* Author:    Kumar Gaurav	                                                        *
*										                                            *
* ----------------------------------------------------------------------------------*/
#pragma region HEADER_FILES
#include "LTENR_SN_HandOver.h"
#pragma endregion

#pragma region DC_HO_SN_UE_CONTEXT_RELEASE_ACK
void fn_NetSim_LTENR_DC_HO_UE_CONTEXT_RELEASE_ACK(ptrLTENR_SN_HANDOVER_HDR hdr) {

	ptrLTENR_HO_SN_ADDITION_REQUEST msg = calloc(1, sizeof * msg);
	msg->cause = EstablishmentCause_mo_data;
	msg->MNID = hdr->MNID;
	msg->ueID = hdr->ueID;
	msg->SNID = hdr->SNID;
	hdr->msg = msg;
	hdr->msgType = LTENR_MSG_DC_HO_SEC_CELL_UE_CONTEXT_RELEASE_ACK;

	NETSIM_IPAddress destIP = NULL;
	NETSIM_IPAddress srcIP = NULL;
	if (!fn_NetSim_LTENR_NSA_IS_CORE_SPLIT_EXISTS(hdr->MNID, hdr->MNIF)) {
		destIP = DEVICE_NWADDRESS(hdr->MNID,
			fn_NetSim_LTENR_CORE_INTERFACE(hdr->MNID, nGC_INTERFACE_XN));
		srcIP = DEVICE_NWADDRESS(hdr->SNID,
			fn_NetSim_LTENR_CORE_INTERFACE(hdr->SNID, nGC_INTERFACE_XN));
	}
	else {
		if (fn_NetSim_LTENR_IS_S1_INTERFACE_EXISTS(hdr->MNID)) {
			destIP = DEVICE_NWADDRESS(hdr->MNID,
				fn_NetSim_LTENR_CORE_INTERFACE(hdr->MNID, nGC_INTERFACE_LTE_S1));
			srcIP = DEVICE_NWADDRESS(hdr->SNID,
				fn_NetSim_LTENR_CORE_INTERFACE(hdr->SNID, nGC_INTERFACE_LTE_S1));
		}
		else {
			destIP = DEVICE_NWADDRESS(hdr->MNID,
				fn_NetSim_LTENR_CORE_INTERFACE(hdr->MNID, nGC_INTERFACE_N3));
			srcIP = DEVICE_NWADDRESS(hdr->SNID,
				fn_NetSim_LTENR_CORE_INTERFACE(hdr->SNID, nGC_INTERFACE_N3));
		}
	}

	LTENR_Handover_Send_Packet(hdr->SNID, hdr->MNID, srcIP, destIP, hdr, sizeof * hdr,
		LTENR_MSG_DC_HO_SEC_CELL_UE_CONTEXT_RELEASE_ACK,
		strLTENR_MSGTYPE[LTENR_MSG_DC_HO_SEC_CELL_UE_CONTEXT_RELEASE_ACK % (MAC_PROTOCOL_LTE_NR * 100)],
		DC_SN_HO_UE_CONTEXT_RELEASE_ACK);
}

void fn_NetSim_LTENR_DC_HO_SN_UE_CONTEXT_RELEASE_ACK_RECV(ptrLTENR_SN_HANDOVER_HDR hdr) {
	//nothing to do
}
#pragma endregion

#pragma region DC_HO_SN_UE_CONTEXT_RELEASE
void fn_NetSim_LTENR_DC_HO_UE_CONTEXT_RELEASE(ptrLTENR_SN_HANDOVER_HDR hdr) {

	ptrLTENR_HO_SN_ADDITION_REQUEST msg = calloc(1, sizeof * msg);
	msg->cause = EstablishmentCause_mo_data;
	msg->MNID = hdr->MNID;
	msg->ueID = hdr->ueID;
	msg->SNID = hdr->SNID;
	hdr->msg = msg;
	hdr->msgType = LTENR_MSG_DC_HO_SEC_CELL_UE_CONTEXT_RELEASE;

	NETSIM_IPAddress destIP = NULL;
	NETSIM_IPAddress srcIP = NULL;
	if (!fn_NetSim_LTENR_NSA_IS_CORE_SPLIT_EXISTS(hdr->MNID, hdr->MNIF)) {
		srcIP = DEVICE_NWADDRESS(hdr->MNID,
			fn_NetSim_LTENR_CORE_INTERFACE(hdr->MNID, nGC_INTERFACE_XN));
		destIP = DEVICE_NWADDRESS(hdr->SNID,
			fn_NetSim_LTENR_CORE_INTERFACE(hdr->SNID, nGC_INTERFACE_XN));
	}
	else {
		if (fn_NetSim_LTENR_IS_S1_INTERFACE_EXISTS(hdr->MNID)) {
			srcIP = DEVICE_NWADDRESS(hdr->MNID,
				fn_NetSim_LTENR_CORE_INTERFACE(hdr->MNID, nGC_INTERFACE_LTE_S1));
			destIP = DEVICE_NWADDRESS(hdr->SNID,
				fn_NetSim_LTENR_CORE_INTERFACE(hdr->SNID, nGC_INTERFACE_LTE_S1));
		}
		else {
			srcIP = DEVICE_NWADDRESS(hdr->MNID,
				fn_NetSim_LTENR_CORE_INTERFACE(hdr->MNID, nGC_INTERFACE_N3));
			destIP = DEVICE_NWADDRESS(hdr->SNID,
				fn_NetSim_LTENR_CORE_INTERFACE(hdr->SNID, nGC_INTERFACE_N3));
		}
	}

	LTENR_Handover_Send_Packet(hdr->MNID, hdr->SNID, srcIP, destIP, hdr, sizeof * hdr,
		LTENR_MSG_DC_HO_SEC_CELL_UE_CONTEXT_RELEASE,
		strLTENR_MSGTYPE[LTENR_MSG_DC_HO_SEC_CELL_UE_CONTEXT_RELEASE % (MAC_PROTOCOL_LTE_NR * 100)],
		DC_SN_HO_UE_CONTEXT_RELEASE);
}

void fn_NetSim_LTENR_DC_HO_SN_UE_CONTEXT_RELEASE_RECV(ptrLTENR_SN_HANDOVER_HDR hdr) {
	ptrLTENR_SN_HANDOVER_HDR msg = calloc(1, sizeof * msg);
	memcpy(msg, hdr, sizeof * msg);
	msg->msg = NULL;
	fn_NetSim_LTENR_DC_HO_UE_CONTEXT_RELEASE_ACK(msg);
}
#pragma endregion

#pragma region DC_HO_SN_RELEASE_ACK
void fn_NetSim_LTENR_DC_HO_SN_RELEASE_ACK(ptrLTENR_SN_HANDOVER_HDR hdr) {

	ptrLTENR_HO_SN_ADDITION_REQUEST msg = calloc(1, sizeof * msg);
	msg->cause = EstablishmentCause_mo_data;
	msg->MNID = hdr->MNID;
	msg->ueID = hdr->ueID;
	msg->SNID = hdr->SNID;
	hdr->msg = msg;
	hdr->msgType = LTENR_MSG_DC_HO_SEC_CELL_RELEASE_REQUEST_ACK;

	NETSIM_ID sin = fn_NetSim_LTENR_SEC_INTEFACE_FOR_UE(hdr->ueID, hdr->ueIF);
	ptrLTENR_UERRC uerrc_secondary = LTENR_UERRC_GET(hdr->ueID, sin);
	ptrLTENR_UERRC uerrc = LTENR_UERRC_GET(hdr->ueID, hdr->ueIF);
	if (!uerrc->isMNHandoverComplete)
		uerrc_secondary->ueRRCState = UERRC_INACTIVE;
	uerrc_secondary->isSecondaryAssociated = true;
	NETSIM_IPAddress destIP = NULL;
	NETSIM_IPAddress srcIP = NULL;
	if (!fn_NetSim_LTENR_NSA_IS_CORE_SPLIT_EXISTS(hdr->MNID, hdr->MNIF)) {
		destIP = DEVICE_NWADDRESS(hdr->MNID,
			fn_NetSim_LTENR_CORE_INTERFACE(hdr->MNID, nGC_INTERFACE_XN));
		srcIP = DEVICE_NWADDRESS(hdr->SNID,
			fn_NetSim_LTENR_CORE_INTERFACE(hdr->SNID, nGC_INTERFACE_XN));
	}
	else {
		if (fn_NetSim_LTENR_IS_S1_INTERFACE_EXISTS(hdr->MNID)) {
			destIP = DEVICE_NWADDRESS(hdr->MNID,
				fn_NetSim_LTENR_CORE_INTERFACE(hdr->MNID, nGC_INTERFACE_LTE_S1));
			srcIP = DEVICE_NWADDRESS(hdr->SNID,
				fn_NetSim_LTENR_CORE_INTERFACE(hdr->SNID, nGC_INTERFACE_LTE_S1));
		}
		else {
			destIP = DEVICE_NWADDRESS(hdr->MNID,
				fn_NetSim_LTENR_CORE_INTERFACE(hdr->MNID, nGC_INTERFACE_N3));
			srcIP = DEVICE_NWADDRESS(hdr->SNID,
				fn_NetSim_LTENR_CORE_INTERFACE(hdr->SNID, nGC_INTERFACE_N3));
		}
	}

	LTENR_Handover_Send_Packet(hdr->SNID, hdr->MNID, srcIP, destIP, hdr, sizeof * hdr,
		LTENR_MSG_DC_HO_SEC_CELL_RELEASE_REQUEST_ACK,
		strLTENR_MSGTYPE[LTENR_MSG_DC_HO_SEC_CELL_RELEASE_REQUEST_ACK % (MAC_PROTOCOL_LTE_NR * 100)],
		DC_SN_HO_RELEASE_REQUEST_ACK);
}

void fn_NetSim_LTENR_DC_HO_SN_RELEASE_REQUEST_ACK_RECV(ptrLTENR_SN_HANDOVER_HDR hdr) {
	ptrLTENR_SN_HANDOVER_HDR msg = calloc(1, sizeof * msg);
	memcpy(msg, hdr, sizeof * msg);
	msg->msg = NULL;
	fn_NetSim_LTENR_DC_HO_UE_CONTEXT_RELEASE(msg);
}
#pragma endregion

#pragma region DC_HO_SN_RELEASE
void fn_NetSim_LTENR_DC_HO_SN_RELEASE_REQUEST(ptrLTENR_SN_HANDOVER_HDR hdr) {

	ptrLTENR_HO_SN_ADDITION_REQUEST msg = calloc(1, sizeof * msg);
	msg->cause = EstablishmentCause_mo_data;
	msg->MNID = hdr->MNID;
	msg->ueID = hdr->ueID;
	msg->SNID = hdr->SNID;
	hdr->msg = msg;
	hdr->msgType = LTENR_MSG_DC_HO_SEC_CELL_RELEASE_REQUEST;

	NETSIM_IPAddress destIP = NULL;
	NETSIM_IPAddress srcIP = NULL;
	if (!fn_NetSim_LTENR_NSA_IS_CORE_SPLIT_EXISTS(hdr->MNID, hdr->MNIF)) {
		srcIP = DEVICE_NWADDRESS(hdr->MNID,
			fn_NetSim_LTENR_CORE_INTERFACE(hdr->MNID, nGC_INTERFACE_XN));
		destIP = DEVICE_NWADDRESS(hdr->SNID,
			fn_NetSim_LTENR_CORE_INTERFACE(hdr->SNID, nGC_INTERFACE_XN));
	}
	else {
		if (fn_NetSim_LTENR_IS_S1_INTERFACE_EXISTS(hdr->MNID)) {
			srcIP = DEVICE_NWADDRESS(hdr->MNID,
				fn_NetSim_LTENR_CORE_INTERFACE(hdr->MNID, nGC_INTERFACE_LTE_S1));
			destIP = DEVICE_NWADDRESS(hdr->SNID,
				fn_NetSim_LTENR_CORE_INTERFACE(hdr->SNID, nGC_INTERFACE_LTE_S1));
		}
		else {
			srcIP = DEVICE_NWADDRESS(hdr->MNID,
				fn_NetSim_LTENR_CORE_INTERFACE(hdr->MNID, nGC_INTERFACE_N3));
			destIP = DEVICE_NWADDRESS(hdr->SNID,
				fn_NetSim_LTENR_CORE_INTERFACE(hdr->SNID, nGC_INTERFACE_N3));
		}
	}

	LTENR_Handover_Send_Packet(hdr->MNID, hdr->SNID, srcIP, destIP, hdr, sizeof * hdr,
		LTENR_MSG_DC_HO_SEC_CELL_RELEASE_REQUEST,
		strLTENR_MSGTYPE[LTENR_MSG_DC_HO_SEC_CELL_RELEASE_REQUEST % (MAC_PROTOCOL_LTE_NR * 100)],
		DC_SN_HO_RELEASE_REQUEST);
}

void fn_NetSim_LTENR_DC_HO_SN_RELEASE_RECV(ptrLTENR_SN_HANDOVER_HDR hdr) {

	NETSIM_ID d = hdr->SNID;
	NETSIM_ID in = hdr->SNIF;
	NETSIM_ID r = hdr->ueID;
	NETSIM_ID rin = hdr->ueIF;
	NETSIM_ID t = hdr->TSNID;
	NETSIM_ID tin = hdr->TSNIF;
	ptrLTENR_PROTODATA data = LTENR_PROTODATA_GET(r, rin);

	//Association and Dissociation of interface
	LTENR_ASSOCIATEINFO_REMOVE(d, in, r, rin);
	LTENR_ASSOCIATEINFO_ADD(t, tin, r, rin);

	LTENR_PHY_ASSOCIATION(t, tin, r, rin, true);
	LTENR_RLC_ASSOCIATION(t, tin, r, rin, true);

	if (fn_NetSim_LTENR_NSA_IS_CORE_SPLIT_EXISTS(r, rin) ||
		fn_NetSim_LTENR_NSA_IS_OPTION_X_EXISTS(r, rin)) {
		LTENR_PDCP_ASSOCIATION(t, tin, r, rin, true);

		ptrLTENR_PDCPVAR serPDCPVar = LTENR_GNBPDCP_GET(d, in);
		ptrLTENR_PDCPVAR tarPDCPVar = LTENR_GNBPDCP_GET(t, tin);
		ptrLTENR_PDCPVAR uePDCPVar = LTENR_UEPDCP_GET(r, rin);

		//Buffer transfer GNB PDCP 
		ptrLTENR_PDCPENTITY serentity = LTENR_PDCP_FindEntity(serPDCPVar, d, in, r, rin, serPDCPVar->pdcpEntity->rlcMode);
		ptrLTENR_PDCPENTITY tarentity = LTENR_PDCP_FindEntity(tarPDCPVar, t, tin, r, rin, tarPDCPVar->pdcpEntity->rlcMode);
		fn_NetSim_LTENR_NAS_PDCP_BUFFER_TRANSFER(serentity, tarentity, 0);

		//Buffer transfer UE PDCP
		serentity = LTENR_PDCP_FindEntity(uePDCPVar, d, in, r, rin, uePDCPVar->pdcpEntity->rlcMode);
		tarentity = LTENR_PDCP_FindEntity(uePDCPVar, t, tin, r, rin, uePDCPVar->pdcpEntity->rlcMode);
		fn_NetSim_LTENR_NAS_PDCP_BUFFER_TRANSFER(serentity, tarentity, 1);

		serentity = LTENR_PDCP_FindEntity(serPDCPVar, d, in, r, rin, serPDCPVar->pdcpEntity->rlcMode);
		tarentity = LTENR_PDCP_FindEntity(tarPDCPVar, t, tin, r, rin, tarPDCPVar->pdcpEntity->rlcMode);
		fn_NetSim_LTENR_NAS_PDCP_BUFFER_TRANSFER(serentity, tarentity, 0);

		//Buffer transfer UE PDCP
		serentity = LTENR_PDCP_FindEntity(uePDCPVar, d, in, r, rin, uePDCPVar->pdcpEntity->rlcMode);
		tarentity = LTENR_PDCP_FindEntity(uePDCPVar, t, tin, r, rin, uePDCPVar->pdcpEntity->rlcMode);
		fn_NetSim_LTENR_NAS_PDCP_BUFFER_TRANSFER(serentity, tarentity, 1);

		LTENR_PDCP_ASSOCIATION(d, in, r, rin, false);
	}

	fn_NetSim_LTENR_NAS_RLC_GNB_BUFFER_TRANSFER(d, in, r, rin, t, tin);
	fn_NetSim_LTENR_NAS_RLC_UE_BUFFER_TRANSFER(d, in, r, rin, t, tin);

	ptrLTENR_UERRC ueRRC = LTENR_UERRC_GET(r, 
		fn_NetSim_LTENR_MASTER_INTEFACE_FOR_UE(r, rin));
	//rrc reconfig remaining 
	ueRRC->SNID = hdr->TSNID;
	ueRRC->SNIF = hdr->TSNIF;

	//disassociation
	LTENR_PHY_ASSOCIATION(d, in, r, rin, false);
	LTENR_RLC_ASSOCIATION(d, in, r, rin, false);

	//EPC Dissociation
	if (fn_NetSim_LTENR_NSA_IS_CORE_SPLIT_EXISTS(r, rin) ||
		fn_NetSim_LTENR_NSA_IS_OPTION_X_EXISTS(r, rin)) {
		if (fn_NetSim_LTENR_IS_S1_INTERFACE_EXISTS(d)) {
			NETSIM_ID epcIn = LTENR_NSA_EPC_HLR_FindOutInterface_FOR_GNB(hdr->AMFID, hdr->ueID, hdr->SNID);
			ptrLTENR_EPC_HLR hlr = LTENR_EPC_FindHLR_For_GNB(hdr->AMFID, epcIn, hdr->ueID, hdr->SNID);
			ptrLTENR_EPC epc = LTENR_EPC_GET(hdr->AMFID, epcIn);
			LTENR_EPC_HLR_REMOVE(epc, hlr);

			hlr = LTENR_EPC_FindHLR_For_GNB(hdr->AMFID, epcIn, hdr->ueID, hdr->SNID);
			if (hlr != NULL) {
				LTENR_EPC_HLR_REMOVE(epc, hlr);
			}
		}
		else {
			ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(hdr->SNID, hdr->SNIF);
			NETSIM_ID UPFID = fn_NetSim_Stack_GetDeviceId_asName(pd->connectedUPFName);
			NETSIM_ID UPFIF = fn_NetSim_LTENR_CORE_INTERFACE(UPFID, nGC_INTERFACE_N3);
			iptable_delete(IP_WRAPPER_GET(UPFID),
				DEVICE_NWADDRESS(hdr->ueID, hdr->ueIF),
				DEVICE_NWADDRESS(UPFID, UPFIF), "LTE_NR");

			iptable_add(IP_WRAPPER_GET(UPFID),
				DEVICE_NWADDRESS(hdr->ueID, hdr->ueIF),
				STR_TO_IP4("255.255.255.255"),
				0,
				DEVICE_NWADDRESS(hdr->TSNID, fn_NetSim_LTENR_CORE_INTERFACE(hdr->TSNID, nGC_INTERFACE_N3)),
				1,
				&DEVICE_NWADDRESS(UPFID, fn_NetSim_LTENR_CORE_INTERFACE(UPFID, nGC_INTERFACE_N3)),
				&UPFIF,
				1,
				"LTE_NR");
		}
	}
	NETSIM_ID id = DEVICE_PHYLAYER(d, in)->nLinkId;
	NetSim_LINKS* link = NETWORK->ppstruNetSimLinks[id - 1];
	fn_NetSim_Stack_RemoveDeviceFromlink(r, rin, DEVICE_PHYLAYER(d, in)->nLinkId);
	fn_NetSim_Stack_AddDeviceTolink(r, rin, DEVICE_PHYLAYER(t, tin)->nLinkId);
	ptrLTENR_SN_HANDOVER_HDR msg = calloc(1, sizeof * msg);
	memcpy(msg, hdr, sizeof * msg);
	msg->msg = NULL;
	fn_NetSim_LTENR_DC_HO_SN_RELEASE_ACK(msg);
}
#pragma endregion

#pragma region DC_HO_SN_ADDITION_REQUEST_RECV
void fn_NetSim_LTENR_DC_HO_SN_ADDITION_REQUEST_ACK(ptrLTENR_SN_HANDOVER_HDR hdr) {

	ptrLTENR_HO_SN_ADDITION_REQUEST msg = calloc(1, sizeof * msg);
	msg->cause = EstablishmentCause_mo_data;
	msg->MNID = hdr->MNID;
	msg->ueID = hdr->ueID;
	msg->SNID = hdr->SNID;
	hdr->msg = msg;
	hdr->msgType = LTENR_MSG_DC_HO_SEC_CELL_ADDITION_REQUEST_ACK;

	NETSIM_IPAddress destIP = NULL;
	NETSIM_IPAddress srcIP = NULL;
	if (!fn_NetSim_LTENR_NSA_IS_CORE_SPLIT_EXISTS(hdr->MNID, hdr->MNIF)) {
		destIP = DEVICE_NWADDRESS(hdr->MNID,
			fn_NetSim_LTENR_CORE_INTERFACE(hdr->MNID, nGC_INTERFACE_XN));
		srcIP = DEVICE_NWADDRESS(hdr->TSNID,
			fn_NetSim_LTENR_CORE_INTERFACE(hdr->TSNID, nGC_INTERFACE_XN));
	}
	else {
		if (fn_NetSim_LTENR_IS_S1_INTERFACE_EXISTS(hdr->MNID)) {
			destIP = DEVICE_NWADDRESS(hdr->MNID,
				fn_NetSim_LTENR_CORE_INTERFACE(hdr->MNID, nGC_INTERFACE_LTE_S1));
			srcIP = DEVICE_NWADDRESS(hdr->TSNID,
				fn_NetSim_LTENR_CORE_INTERFACE(hdr->TSNID, nGC_INTERFACE_LTE_S1));
		}
		else {
			destIP = DEVICE_NWADDRESS(hdr->MNID,
				fn_NetSim_LTENR_CORE_INTERFACE(hdr->MNID, nGC_INTERFACE_N3));
			srcIP = DEVICE_NWADDRESS(hdr->TSNID,
				fn_NetSim_LTENR_CORE_INTERFACE(hdr->TSNID, nGC_INTERFACE_N3));
		}
	}

	LTENR_Handover_Send_Packet(hdr->TSNID, hdr->MNID, srcIP, destIP, hdr, sizeof * hdr,
		LTENR_MSG_DC_HO_SEC_CELL_ADDITION_REQUEST_ACK,
		strLTENR_MSGTYPE[LTENR_MSG_DC_HO_SEC_CELL_ADDITION_REQUEST_ACK % (MAC_PROTOCOL_LTE_NR * 100)],
		DC_SN_HO_ADDITION_REQUEST_ACK);
}

void fn_NetSim_LTENR_DC_HO_SN_ADDITION_REQUEST_ACK_RECV(ptrLTENR_SN_HANDOVER_HDR hdr) {
	ptrLTENR_SN_HANDOVER_HDR msg = calloc(1, sizeof * msg);
	memcpy(msg, hdr, sizeof * msg);
	msg->msg = NULL;
	fn_NetSim_LTENR_DC_HO_SN_RELEASE_REQUEST(msg);
}
#pragma endregion

#pragma region DC_HO_SN_ADDITION_REQUEST
void fn_NetSim_LTENR_DC_HO_SN_ADDITION_REQUEST(ptrLTENR_SN_HANDOVER_HDR hdr) {

	ptrLTENR_HO_SN_ADDITION_REQUEST msg = calloc(1, sizeof * msg);
	msg->cause = EstablishmentCause_mo_data;
	msg->MNID = hdr->MNID;
	msg->ueID = hdr->ueID;
	msg->SNID = hdr->SNID;
	hdr->msg = msg;
	hdr->msgType = LTENR_MSG_DC_HO_SEC_CELL_ADDITION_REQUEST;
	hdr->SNHandoverStartTime = pstruEventDetails->dEventTime;
	NETSIM_IPAddress destIP = NULL;
	NETSIM_IPAddress srcIP = NULL;
	if (!fn_NetSim_LTENR_NSA_IS_CORE_SPLIT_EXISTS(hdr->MNID, hdr->MNIF)) {
		srcIP = DEVICE_NWADDRESS(hdr->MNID,
			fn_NetSim_LTENR_CORE_INTERFACE(hdr->MNID, nGC_INTERFACE_XN));
		destIP = DEVICE_NWADDRESS(hdr->TSNID,
			fn_NetSim_LTENR_CORE_INTERFACE(hdr->TSNID, nGC_INTERFACE_XN));
	}
	else {
		if (fn_NetSim_LTENR_IS_S1_INTERFACE_EXISTS(hdr->MNID)) {
			srcIP = DEVICE_NWADDRESS(hdr->MNID,
				fn_NetSim_LTENR_CORE_INTERFACE(hdr->MNID, nGC_INTERFACE_LTE_S1));
			destIP = DEVICE_NWADDRESS(hdr->TSNID,
				fn_NetSim_LTENR_CORE_INTERFACE(hdr->TSNID, nGC_INTERFACE_LTE_S1));
		}
		else {
			srcIP = DEVICE_NWADDRESS(hdr->MNID,
				fn_NetSim_LTENR_CORE_INTERFACE(hdr->MNID, nGC_INTERFACE_N3));
			destIP = DEVICE_NWADDRESS(hdr->TSNID,
				fn_NetSim_LTENR_CORE_INTERFACE(hdr->TSNID, nGC_INTERFACE_N3));
		}
	}

	LTENR_Handover_Send_Packet(hdr->MNID, hdr->TSNID, srcIP, destIP, hdr, sizeof * hdr,
		LTENR_MSG_DC_HO_SEC_CELL_ADDITION_REQUEST,
		strLTENR_MSGTYPE[LTENR_MSG_DC_HO_SEC_CELL_ADDITION_REQUEST % (MAC_PROTOCOL_LTE_NR * 100)],
		DC_SN_HO_ADDITION_REQUEST);
}

void fn_NetSim_LTENR_DC_HO_SN_ADDITION_REQUEST_RECV(ptrLTENR_SN_HANDOVER_HDR hdr) {
	ptrLTENR_SN_HANDOVER_HDR msg = calloc(1, sizeof * msg);
	memcpy(msg, hdr, sizeof * msg);
	msg->msg = NULL;
	fn_NetSim_LTENR_DC_HO_SN_ADDITION_REQUEST_ACK(msg);
}
#pragma endregion

#pragma region HO_PROCESSING
void fn_NetSim_DC_HO_SN_PROCESSIONG(NETSIM_ID ueID, NETSIM_ID ueIF,ptrLTENR_RRC_UE_MEASUREMENT_REPORT report) {
	double serveingSINR = 0;
	double serveringRSRP = 0;
	double serveringRSRQ = 0;
	double targetSINR = INT_MIN;
	NETSIM_ID target = 0;
	ptrLTENR_RRC_UE_MEASUREMENT_REPORT report1 = report;
	ptrLTENR_PROTODATA data = LTENR_PROTODATA_GET(ueID, ueIF);
	ptrLTENR_UERRC rrc = LTENR_UERRC_GET(ueID, ueIF);
	ptrLTENR_GNBRRC gnbrrc = LTENR_GNBRRC_GET(rrc->SNID, rrc->SNIF);
	while (report1)
	{
		if (data->isDCEnable) {
			if (data->SecCellType == MMWAVE_CELL_TYPE) {
				if (!fn_NetSim_isDeviceLTENR(report1->cellID, report1->cellIF)) {
					report1 = LIST_NEXT(report1);
					continue;
				}
			}
			else {
				if (fn_NetSim_isDeviceLTENR(report1->cellID, report1->cellIF)) {
					report1 = LIST_NEXT(report1);
					continue;
				}
			}
		}

		if (report1->cellID == rrc->SNID)
		{
			serveingSINR = report1->sinr;
			serveringRSRP = report1->rsrp;
			serveringRSRQ = report1->rsrq;
		}
		if (targetSINR < report1->sinr) {
			targetSINR = report1->sinr;
			target = report1->cellID;
		}
		report1 = LIST_NEXT(report1);
	}

	if (serveingSINR + gnbrrc->HandoverMargin < targetSINR && target > 0 && rrc->SNID != target) {
		ptrLTENR_SN_HANDOVER_HDR hdr = calloc(1, sizeof * hdr);
		hdr->ueID = ueID;
		hdr->ueIF = fn_NetSim_LTENR_SEC_INTEFACE_FOR_UE(ueID, ueIF);
		hdr->MNID = rrc->SelectedCellID;
		hdr->MNIF = rrc->SelectedCellIF;
		hdr->SNID = rrc->SNID;
		hdr->SNIF = rrc->SNIF;
		hdr->TSNID = target;
		hdr->TSNIF = fn_NetSim_Get_LTENR_INTERFACE_ID_FROM_DEVICE_ID(target);
		if (fn_NetSim_LTENR_IS_S1_INTERFACE_EXISTS(hdr->MNID)) {
			ptrLTENR_GNBMAC mac = LTENR_GNBMAC_GET(hdr->MNID, hdr->MNIF);
			hdr->AMFID = mac->epcId;
			hdr->AMFIF = mac->epcIf;
		}
		else {
			hdr->AMFID = rrc->SelectedAMFID;
			hdr->AMFIF = rrc->SelectedAMFIF;
			ptrLTENR_PROTODATA trrc = LTENR_PROTODATA_GET(hdr->TSNID, hdr->TSNIF);
			hdr->targetAMFID = fn_NetSim_Stack_GetDeviceId_asName(trrc->connectedAMFName);
			hdr->targetAMFIF = fn_NetSim_LTENR_CORE_INTERFACE(hdr->targetAMFID, nGC_INTERFACE_N1_N2);
			hdr->associateSMFID = fn_NetSim_Stack_GetDeviceId_asName(trrc->connectedSMFName);
		}
		fn_NetSim_LTENR_DC_HO_SN_ADDITION_REQUEST(hdr);
	}
}
#pragma endregion

#pragma region PACKET_FREE_HANDLER
void LTENR_NSA_DC_HO_SEC_ADDITION_REQ_FREE(ptrLTENR_SN_HANDOVER_HDR hdr) {
	ptrLTENR_HO_SN_ADDITION_REQUEST msg = hdr->msg;
	free(msg);
	free(hdr);
}

void fn_NetSim_LTENR_NSA_SN_HANDOVER_INIT() {
	DC_SN_HO_ADDITION_REQUEST = LTENR_HDR_REGISTER("DC_HO_SEC_CELL_ADDITION_REQUEST", NULL, NULL, 
		LTENR_NSA_DC_HO_SEC_ADDITION_REQ_FREE, NULL);
	DC_SN_HO_ADDITION_REQUEST_ACK = LTENR_HDR_REGISTER("DC_HO_SEC_CELL_ADDITION_REQUEST_ACK", NULL, NULL,
		LTENR_NSA_DC_HO_SEC_ADDITION_REQ_FREE, NULL);
	DC_SN_HO_RELEASE_REQUEST = LTENR_HDR_REGISTER("DC_HO_SEC_CELL_RELEASE_REQUEST", NULL, NULL,
		LTENR_NSA_DC_HO_SEC_ADDITION_REQ_FREE, NULL);
	DC_SN_HO_RELEASE_REQUEST_ACK = LTENR_HDR_REGISTER("DC_HO_SEC_CELL_RELEASE_REQUEST_ACK", NULL, NULL,
		LTENR_NSA_DC_HO_SEC_ADDITION_REQ_FREE, NULL);
	DC_SN_HO_UE_CONTEXT_RELEASE = LTENR_HDR_REGISTER("DC_HO_SEC_CELL_UE_CONTEXT_RELEASE", NULL, NULL,
		LTENR_NSA_DC_HO_SEC_ADDITION_REQ_FREE, NULL);
	DC_SN_HO_UE_CONTEXT_RELEASE_ACK = LTENR_HDR_REGISTER("DC_HO_SEC_CELL_UE_CONTEXT_RELEASE_ACK", NULL, NULL,
		LTENR_NSA_DC_HO_SEC_ADDITION_REQ_FREE, NULL);
}
#pragma endregion

#pragma region PACKET_HANDLER
void fn_NetSim_SN_HANDOVER_PACKET_HANDLER() {
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	ptrLTENR_SN_HANDOVER_HDR hdr = NULL;
	switch (packet->nControlDataType) {
	case LTENR_MSG_DC_HO_SEC_CELL_ADDITION_REQUEST:
		hdr = LTENR_HANDOVER_HDR_GET_FROM_PACKET(packet, DC_SN_HO_ADDITION_REQUEST);
		fn_NetSim_LTENR_DC_HO_SN_ADDITION_REQUEST_RECV(hdr);
		LTENR_HANDOVER_HDR_FREE_FROM_PACKET(packet, DC_SN_HO_ADDITION_REQUEST);
		break;
	case LTENR_MSG_DC_HO_SEC_CELL_ADDITION_REQUEST_ACK:
		hdr = LTENR_HANDOVER_HDR_GET_FROM_PACKET(packet, DC_SN_HO_ADDITION_REQUEST_ACK);
		fn_NetSim_LTENR_DC_HO_SN_ADDITION_REQUEST_ACK_RECV(hdr);
		LTENR_HANDOVER_HDR_FREE_FROM_PACKET(packet, DC_SN_HO_ADDITION_REQUEST_ACK);
		break;
	case LTENR_MSG_DC_HO_SEC_CELL_RELEASE_REQUEST:
		hdr = LTENR_HANDOVER_HDR_GET_FROM_PACKET(packet, DC_SN_HO_RELEASE_REQUEST);
		fn_NetSim_LTENR_DC_HO_SN_RELEASE_RECV(hdr);
		LTENR_HANDOVER_HDR_FREE_FROM_PACKET(packet, DC_SN_HO_RELEASE_REQUEST);
		break;
	case LTENR_MSG_DC_HO_SEC_CELL_RELEASE_REQUEST_ACK:
		hdr = LTENR_HANDOVER_HDR_GET_FROM_PACKET(packet, DC_SN_HO_RELEASE_REQUEST_ACK);
		fn_NetSim_LTENR_DC_HO_SN_RELEASE_REQUEST_ACK_RECV(hdr);
		LTENR_HANDOVER_HDR_FREE_FROM_PACKET(packet, DC_SN_HO_RELEASE_REQUEST_ACK);
		break;
	case LTENR_MSG_DC_HO_SEC_CELL_UE_CONTEXT_RELEASE:
		hdr = LTENR_HANDOVER_HDR_GET_FROM_PACKET(packet, DC_SN_HO_UE_CONTEXT_RELEASE);
		fn_NetSim_LTENR_DC_HO_SN_UE_CONTEXT_RELEASE_RECV(hdr);
		LTENR_HANDOVER_HDR_FREE_FROM_PACKET(packet, DC_SN_HO_UE_CONTEXT_RELEASE);
		break;
	case LTENR_MSG_DC_HO_SEC_CELL_UE_CONTEXT_RELEASE_ACK:
		hdr = LTENR_HANDOVER_HDR_GET_FROM_PACKET(packet, DC_SN_HO_UE_CONTEXT_RELEASE_ACK);
		fn_NetSim_LTENR_DC_HO_SN_UE_CONTEXT_RELEASE_ACK_RECV(hdr);
		LTENR_HANDOVER_HDR_FREE_FROM_PACKET(packet, DC_SN_HO_UE_CONTEXT_RELEASE_ACK);
		break;
	default:
		break;
	}
}
#pragma endregion