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
#include "LTENR_Core.h"
#include "LTENR_SMF_UPF.h"
#pragma endregion

#pragma region FIND_AMF_VAR
ptrLTENR_AMF_VAR LTENR_FIND_NGAP_AMF_VAR(NETSIM_ID AMFID, NETSIM_ID AMFIF) {
	ptrLTENR_AMF_VAR var = LTENR_AMF_NGAP_GET(AMFID);
	while (var) {
		if (var->amfN2ID == AMFID && var->amfN2IF == AMFIF)
			return var;
	}
	return NULL;
}

ptrLTENR_AMF_VAR LTENR_FIND_GTP_C_AMF_VAR(NETSIM_ID AMFID, NETSIM_ID AMFIF) {
	ptrLTENR_AMF_VAR var = LTENR_AMF_NAS_MM_GET(AMFID);
	while (var) {
		if (var->amfN2ID == AMFID && var->amfN2IF == AMFIF)
			return var;
	}
	return NULL;
}
#pragma endregion

#pragma region AMF_SELECTION
NETSIM_ID LTENR_AMF_GetUeConnectedAmf(NETSIM_ID UEID, NETSIM_ID UEIF) {
	ptrLTENR_UERRC rrc = LTENR_UERRC_GET(UEID, UEIF);
	return rrc->SelectedAMFID;
}

NETSIM_ID LTENR_AMF_GetNewAMF(NETSIM_ID UEID, NETSIM_ID UEIF) {
	ptrLTENR_UERRC rrc = LTENR_UERRC_GET(UEID, UEIF);
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(rrc->SelectedCellID, rrc->SelectedCellIF);
	return fn_NetSim_Stack_GetDeviceId_asName(pd->connectedAMFName);
}
#pragma endregion

#pragma region ADDITION_OF_NODES
void LTENR_AMF_AddgNB(NETSIM_ID AMFID, NETSIM_ID AMFIF,
					  NETSIM_ID gNBID, NETSIM_ID gNBIF) {
	ptrLTENR_AMF_VAR AMFVar = LTENR_FIND_NGAP_AMF_VAR(AMFID, AMFIF);
	ptrLTENR_AMF_INFO gnbInfo = LTENR_AMF_INFO_ALLOC();
	gnbInfo->gnbN2ID = gNBID;
	gnbInfo->gnbLTENRIF = fn_NetSim_LTENR_CORE_INTERFACE(gNBID, nGC_INTERFACE_RAN);
	gnbInfo->gnbN2IF = gNBIF;
	LTENR_AMF_INFO_ADD(AMFVar->info, gnbInfo);
}

void LTENR_AMF_AddSMF(NETSIM_ID AMFID, NETSIM_ID AMFIF,
					  NETSIM_ID SMFID, NETSIM_ID SMFIF, ptrLTENR_AMF_VAR AMFVar) {
	ptrLTENR_AMF_SMF_Info smfInfo = calloc(1, sizeof * smfInfo);
	smfInfo->SMFID = SMFID;
	smfInfo->SMFIF = SMFID;
	AMFVar->smfInfo = smfInfo;
}

ptrLTENR_AMF_INFO LTENR_AMF_Find_gNB_Info(NETSIM_ID AMFID, NETSIM_ID AMFIF,
						   NETSIM_ID gNBID, NETSIM_ID gNBIF) {
	ptrLTENR_AMF_VAR AMFVar = LTENR_FIND_NGAP_AMF_VAR(AMFID, AMFIF);
	ptrLTENR_AMF_INFO info = AMFVar->info;
	while (info) {
		if (info->gnbN2ID == gNBID && info->gnbN2IF == gNBIF)
			return info;
		info = LIST_NEXT(info);
	}
	return NULL;
}

void LTENR_AMF_AddUE(NETSIM_ID AMFID, NETSIM_ID AMFIF,
					NETSIM_ID gNBID, NETSIM_ID gNBIF,
					NETSIM_ID UEID, NETSIM_ID UEIF) {
	ptrLTENR_AMF_INFO info = LTENR_AMF_Find_gNB_Info(AMFID, AMFIF, gNBID, gNBIF);
	ptr_LTENR_AMF_UE_LIST list = LTENR_AMF_UE_LIST_ALLOC();
	list->UEID = UEID;
	list->UEIF = UEIF;
	LTENR_AMF_UE_LIST_ADD(info->ueList,list);
}

void LTENR_AMF_DeleteUE(NETSIM_ID AMFID, NETSIM_ID AMFIF,
						NETSIM_ID gNBID, NETSIM_ID gNBIF,
						NETSIM_ID UEID, NETSIM_ID UEIF) {
	ptrLTENR_AMF_INFO info = LTENR_AMF_Find_gNB_Info(AMFID, AMFIF, gNBID, gNBIF);
	ptr_LTENR_AMF_UE_LIST list = info->ueList;
	while (list) {
		if (list->UEID == UEID && list->UEIF == UEIF)
			LTENR_AMF_UE_LIST_REMOVE(info->ueList,list);
		list = LIST_NEXT(list);
	}
}
#pragma endregion

#pragma region CREATE_SESSION_REQUEST
void LTENR_AMF_CREATE_SESSION_REQUEST(ptrLTENR_CORE_HDR hdr, ptrLTENR_EPSBEARER bearer) {

	ptrLTENR_AMF_SMF_CREATE_SESSION_REQUEST_MSG msg = calloc(1, sizeof * msg);
	msg->AMFID = hdr->AMFID;
	msg->AMFIF = hdr->AMFIF;
	msg->gNBID = hdr->gNBID;
	msg->gNBIF = hdr->gNBIF;
	msg->UEID = hdr->UEID;
	msg->UEIF = hdr->UEIF;
	msg->uli = hdr->gNBID;
	ptrLTENR_Bearer_Context bearerRequest = LTENR_Bearer_Context_ALLOC();
	bearerRequest->epsBearer = bearer;
	bearerRequest->epsBearerID = ++epsBearer;
	LTENR_Bearer_Context_ADD(msg->bearer, bearerRequest);
	hdr->msg = msg;
	hdr->msgType = LTENR_MSG_CORE_CREATE_SESSION_REQUEST;

	ptrLTENR_GTPC_HEADER apphdr = fn_NetSim_LTENR_GTPC_DEFAULT_HEADER();
	hdr->apphdr = apphdr;

	NETSIM_IPAddress srcIP = DEVICE_NWADDRESS(hdr->AMFID, 
		fn_NetSim_LTENR_CORE_INTERFACE(hdr->AMFID, nGC_INTERFACE_N11));
	NETSIM_IPAddress destIP = DEVICE_NWADDRESS(hdr->SMFID, hdr->SMFIF);
	LTENR_Core_Send_Packet(hdr->AMFID, hdr->SMFID, srcIP, destIP, hdr, sizeof * hdr,
		LTENR_MSG_CORE_CREATE_SESSION_REQUEST,
		strLTENR_MSGTYPE[LTENR_MSG_CORE_CREATE_SESSION_REQUEST % (MAC_PROTOCOL_LTE_NR * 100)], 
		CreateSessionRequestID);
}
#pragma endregion

#pragma region INITIAL_MSG_RECV
void fn_NetSim_LTENR_AMF_INITIAL_MSG_RECV(ptrLTENR_CORE_HDR hdr) {
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	LTENR_AMF_AddUE(hdr->AMFID, hdr->AMFIF,
					hdr->gNBID, hdr->gNBIF,
					hdr->UEID, hdr->UEIF);

	ptrLTENR_CORE_HDR msg = calloc(1, sizeof * msg);
	memcpy(msg, hdr, sizeof * msg);
	msg->msg = NULL;
	//ACTIVATE BEARER
	ptrLTENR_EPSBEARER bearer = LTENR_EPSBEARER_Create_Default();
	LTENR_AMF_CREATE_SESSION_REQUEST(msg, bearer);
}
#pragma endregion

#pragma region INITIAL_CONTEXT_SETUP_RESPONSE_RECV
void fn_NetSim_LTENR_SEC_CELL_SelectionAddition(NETSIM_ID UE_ID, NETSIM_ID UE_IF) {
	ptrLTENR_UERRC rrc = LTENR_UERRC_GET(UE_ID, UE_IF);
	ptrLTENR_PROTODATA data = LTENR_PROTODATA_GET(UE_ID, UE_IF);
	if (data->isDCEnable && !rrc->isSNSconnected) {
		NETSIM_ID ueInterface = fn_NetSim_LTENR_SEC_INTEFACE_FOR_UE(UE_ID, UE_IF);
		NETSIM_ID SNID = fn_NetSim_LTENR_FIND_BEST_SN_CELL_FOR_UE(UE_ID, ueInterface);
		if (SNID != 0) {
			NETSIM_ID SNIF = fn_NetSim_LTENR_INTERFACE_ID_FROM_CONNECTED_DEVICE(UE_ID, ueInterface, SNID);
			fn_Netsim_NSA_Secondary_Cell_Addition(UE_ID, ueInterface, rrc->SelectedCellID,
				rrc->SelectedCellIF, SNID, SNIF);
			rrc->SNID = SNID;
			rrc->SNIF = SNIF;
		}
	}
}
void fn_NetSim_LTENR_AMF_INITIAL_CONTEXT_SETUP_RESPONSE_RECV(ptrLTENR_CORE_HDR hdr) {
	//yet to implement
	//Add BearerInfoto setup into ue
	ptrLTENR_UERRC rrc = LTENR_UERRC_GET(hdr->UEID, hdr->UEIF);
	ptrLTENR_PROTODATA data = LTENR_PROTODATA_GET(hdr->UEID, hdr->UEIF);
	rrc->ueCMState = UE_CM_CONNECTED;
	NETSIM_ID UPFIF = fn_NetSim_LTENR_CORE_INTERFACE(hdr->UPFID, nGC_INTERFACE_N3);
	iptable_add(IP_WRAPPER_GET(hdr->UPFID),
		DEVICE_NWADDRESS(hdr->UEID, hdr->UEIF),
		STR_TO_IP4("255.255.255.255"),
		0,
		DEVICE_NWADDRESS(hdr->gNBID, fn_NetSim_LTENR_CORE_INTERFACE(hdr->gNBID, nGC_INTERFACE_N3)),
		1,
		&DEVICE_NWADDRESS(hdr->UPFID, fn_NetSim_LTENR_CORE_INTERFACE(hdr->UPFID, nGC_INTERFACE_N3)),
		&UPFIF,
		1,
		"LTE_NR");
}
#pragma endregion

#pragma region INITIAL_CONTEXT_SETUP_REQUEST
void fn_NetSim_LTENR_AMF_INITIAL_CONTEXT_SETUP_REQUEST(ptrLTENR_CORE_HDR hdr, ptrLTENR_Bearer_Setup bearer) {
	hdr->msgType = LTENR_MSG_CORE_INITIAL_CONTEXT_SETUP_REQUEST;
	ptrLTENR_AMF_INITIAL_CONTEXT_REQUEST msg = calloc(1, sizeof * msg);
	msg->AMFID = hdr->AMFID;
	msg->gNBID = hdr->gNBID;
	msg->UEID = hdr->UEID;
	msg->pduSessionID = ++pduSessionID;
	msg->bearer = bearer;
	hdr->msg = msg;

	ptrLTENR_CORE_NGAP_HEADER apphdr = calloc(1, sizeof * apphdr);
	apphdr->procedureCode = NGAP_PROCEDURECODE_INITIAL_CONTEXT_SETUP_REQUEST;
	apphdr->lenghtofIEs = 0;
	apphdr->noofIEs = 0;
	hdr->apphdr = apphdr;

	NETSIM_IPAddress srcIP = DEVICE_NWADDRESS(hdr->AMFID,
		fn_NetSim_LTENR_CORE_INTERFACE(hdr->AMFID, nGC_INTERFACE_N1_N2));
	NETSIM_IPAddress destIP = DEVICE_NWADDRESS(hdr->gNBID,hdr->gNBIF);

	LTENR_Core_Send_Packet(hdr->AMFID, hdr->gNBID, srcIP, destIP, hdr, sizeof * hdr,
		LTENR_MSG_CORE_INITIAL_CONTEXT_SETUP_REQUEST,
		strLTENR_MSGTYPE[LTENR_MSG_CORE_INITIAL_CONTEXT_SETUP_REQUEST % (MAC_PROTOCOL_LTE_NR * 100)],
		InitialContextSetupRequestID);
}
#pragma endregion

#pragma region AMF_CREATE_SESSION_RESPONSE_RECV
void fn_NetSim_LTENR_AMF_CREATE_SESSION_RESPONSE_RECV(ptrLTENR_CORE_HDR hdr) {
	ptrLTENR_AMF_SMF_CREATE_SESSION_RESPONSE_MSG msg = hdr->msg;
	ptrLTENR_Bearer_Activate bearer = msg->Bearer;

	ptrLTENR_Bearer_Setup bearerSetup = LTENR_Bearer_Setup_ALLOC();
	bearerSetup->epsBearerID = bearer->epsBearerID;
	bearerSetup->epsBearer = LTENR_EPSBEARER_Copy_Bearer(bearer->epsBearer);
	bearerSetup->smfteid = bearer->smfte->smfteid;
	bearerSetup->transportLayerIP = bearer->smfte->TXaddress;

	ptrLTENR_CORE_HDR temp_HDR = calloc(1, sizeof * temp_HDR);
	memcpy(temp_HDR, hdr, sizeof * temp_HDR);
	temp_HDR->msgType = LTENR_MSG_CORE_INITIAL_CONTEXT_SETUP_REQUEST;
	temp_HDR->msg = NULL;
	fn_NetSim_LTENR_AMF_INITIAL_CONTEXT_SETUP_REQUEST(temp_HDR, bearerSetup);
}
#pragma endregion

#pragma region PATH_SWITCH_ACK
void fn_NetSim_LTENR_GNB_CORE_PATH_SWITCH_ACK_MSG(ptrLTENR_HANDOVER_Hdr hdr) {
	NETSIM_IPAddress destIP = DEVICE_NWADDRESS(hdr->targetCellID,
		fn_NetSim_LTENR_CORE_INTERFACE(hdr->targetCellID, nGC_INTERFACE_N1_N2));
	NETSIM_IPAddress srcIP = DEVICE_NWADDRESS(hdr->targetAMFID, hdr->targetAMFIF);
	ptrLTENR_NAS_PATH_SWITCH_REQUEST_ACK msg = calloc(1, sizeof * msg);
	msg->targetGNBID = hdr->targetCellID;
	msg->associatedEPCtoTarget = hdr->targetAMFID;
	hdr->msg = msg;
	hdr->msgType = LTENR_MSG_NAS_HANDOVER_PATH_SWITCH_ACK;

	ptrLTENR_CORE_NGAP_HEADER apphdr = calloc(1, sizeof * apphdr);
	apphdr->procedureCode = NGAP_PROCEDURECODE_PATH_SWITCH_ACK;
	apphdr->lenghtofIEs = 0;
	apphdr->noofIEs = 0;
	hdr->apphdr = apphdr;

	LTENR_Core_Send_Packet(hdr->targetAMFID, hdr->targetCellID, srcIP, destIP, hdr, sizeof * hdr,
		LTENR_MSG_NAS_HANDOVER_PATH_SWITCH_ACK,
		strLTENR_MSGTYPE[LTENR_MSG_NAS_HANDOVER_PATH_SWITCH_ACK % (MAC_PROTOCOL_LTE_NR * 100)],
		naspathSwitchAck);
}
#pragma endregion

#pragma region MODIFY_BEARER_REQUEST
void fn_NetSim_LTENR_AMF_MODIFY_BEARER_REQUEST(ptrLTENR_HANDOVER_Hdr hdr) {
	ptrLTENR_UERRC rrc = LTENR_UERRC_GET(hdr->UEID, hdr->UEIF);
	hdr->msgType = LTENR_MSG_CORE_MODIFY_BEARER_REQUEST;
	ptrLTENR_AMF_MODIFY_BEARER_REQUEST msg = calloc(1, sizeof * msg);
	msg->SAMFID = hdr->AMFID;
	msg->SgNBID = hdr->serveringCellID;
	msg->TgNBID = hdr->targetCellID;
	msg->TAMFID = hdr->targetAMFID;
	msg->UEID = hdr->UEID;
	msg->bearer = (ptrLTENR_Bearer_Setup)rrc->bearer;
	hdr->msg = msg;

	NETSIM_ID SMFIF = fn_NetSim_LTENR_CORE_INTERFACE(hdr->associateSMFID, nGC_INTERFACE_N11);
	NETSIM_ID AMFIF = fn_NetSim_LTENR_CORE_INTERFACE(hdr->targetAMFID, nGC_INTERFACE_N11);
	NETSIM_IPAddress destIP = DEVICE_NWADDRESS(hdr->associateSMFID, SMFIF);
	NETSIM_IPAddress srcIP = DEVICE_NWADDRESS(hdr->targetAMFID, AMFIF);

	LTENR_Core_Send_Packet(hdr->targetAMFID, hdr->associateSMFID, srcIP, destIP, hdr, sizeof * hdr,
		LTENR_MSG_CORE_MODIFY_BEARER_REQUEST,
		strLTENR_MSGTYPE[LTENR_MSG_CORE_MODIFY_BEARER_REQUEST % (MAC_PROTOCOL_LTE_NR * 100)],
		ModifyBearerRequestID);
}
#pragma endregion

#pragma region PATH_SWITCH_RECV
void fn_NetSim_SMF_UPF_LTENR_PATH_SWITCH_RECV(ptrLTENR_HANDOVER_Hdr hdr) {
	ptrLTENR_HANDOVER_Hdr msg = calloc(1, sizeof * msg);
	memcpy(msg, hdr, sizeof * msg);
	msg->msg = NULL;
	ptrLTENR_PROTODATA data = LTENR_PROTODATA_GET(hdr->targetCellID, hdr->targetCellIF);
	NETSIM_ID UPFID = fn_NetSim_Stack_GetDeviceId_asName(data->connectedUPFName);
	NETSIM_ID UPFIF = fn_NetSim_LTENR_CORE_INTERFACE(UPFID, nGC_INTERFACE_N3);
	NETSIM_ID SgnbN2IF = fn_NetSim_LTENR_CORE_INTERFACE(hdr->serveringCellID, nGC_INTERFACE_N1_N2);
	NETSIM_ID TgnbN2IF = fn_NetSim_LTENR_CORE_INTERFACE(hdr->targetCellID, nGC_INTERFACE_N1_N2);

	if (!LTENR_AMF_Find_gNB_Info(hdr->targetAMFID, hdr->targetAMFIF, hdr->targetCellID, TgnbN2IF))
		LTENR_AMF_AddgNB(hdr->targetAMFID, hdr->targetAMFIF, hdr->targetCellID, TgnbN2IF);

	LTENR_AMF_DeleteUE(hdr->AMFID, hdr->AMFIF, hdr->serveringCellID, SgnbN2IF, hdr->UEID, hdr->UEIF);
	LTENR_AMF_AddUE(hdr->targetAMFID, hdr->targetAMFIF, hdr->targetCellID, TgnbN2IF, hdr->UEID, hdr->UEIF);

	iptable_delete(IP_WRAPPER_GET(UPFID),
		DEVICE_NWADDRESS(hdr->UEID, hdr->UEIF),
		DEVICE_NWADDRESS(UPFID, UPFIF), "LTE_NR");

	iptable_add(IP_WRAPPER_GET(UPFID),
		DEVICE_NWADDRESS(hdr->UEID, hdr->UEIF),
		STR_TO_IP4("255.255.255.255"),
		0,
		DEVICE_NWADDRESS(hdr->targetCellID, fn_NetSim_LTENR_CORE_INTERFACE(hdr->targetCellID, nGC_INTERFACE_N3)),
		1,
		&DEVICE_NWADDRESS(UPFID, fn_NetSim_LTENR_CORE_INTERFACE(UPFID, nGC_INTERFACE_N3)),
		&UPFIF,
		1,
		"LTE_NR");

	fn_NetSim_LTENR_AMF_MODIFY_BEARER_REQUEST(msg);
}
#pragma endregion

#pragma region MODIFY_BEARER_RESPONSE_RECV
void fn_NetSim_SMF_UPF_LTENR_MODIFY_BEARER_RESPONSE_RECV(ptrLTENR_HANDOVER_Hdr hdr) {
	ptrLTENR_HANDOVER_Hdr msg = calloc(1, sizeof * msg);
	memcpy(msg, hdr, sizeof * msg);
	msg->msg = NULL;
	fn_NetSim_LTENR_GNB_CORE_PATH_SWITCH_ACK_MSG(msg);
}
#pragma endregion

#pragma region AMF_OUT_AND_IN
void fn_NetSim_LTENR_AMF_PACKET_HANDLER() {
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	ptrLTENR_CORE_HDR hdr = NULL;
	ptrLTENR_HANDOVER_Hdr handoverHdr = NULL;

	switch (packet->nControlDataType)
	{
		case LTENR_MSG_CORE_INITIAL_UE_MESSAGE:
			hdr = LTENR_CORE_HDR_GET_FROM_PACKET(packet, InitalUEMsgHDRID);
			fn_NetSim_LTENR_AMF_INITIAL_MSG_RECV(hdr);
			fn_NetSim_Packet_FreePacket(packet);
			break;
		case LTENR_MSG_CORE_CREATE_SESSION_RESPONSE:
			hdr = LTENR_CORE_HDR_GET_FROM_PACKET(packet, CreateSessionResponseID);
			fn_NetSim_LTENR_AMF_CREATE_SESSION_RESPONSE_RECV(hdr);
			fn_NetSim_Packet_FreePacket(packet);
			break;
		case LTENR_MSG_CORE_INITIAL_CONTEXT_SETUP_RESPONSE:
			hdr = LTENR_CORE_HDR_GET_FROM_PACKET(packet, InitialContextSetupResponseID);
			fn_NetSim_LTENR_AMF_INITIAL_CONTEXT_SETUP_RESPONSE_RECV(hdr);
			fn_NetSim_Packet_FreePacket(packet);
			break;
		case LTENR_MSG_NAS_HANDOVER_PATH_SWITCH:
			handoverHdr = LTENR_CORE_HDR_GET_FROM_PACKET(packet, nasPathSwitch);
			fn_NetSim_SMF_UPF_LTENR_PATH_SWITCH_RECV(handoverHdr);
			fn_NetSim_Packet_FreePacket(packet);
			break;
		case LTENR_MSG_CORE_MODIFY_BEARER_RESPONSE:
			handoverHdr = LTENR_CORE_HDR_GET_FROM_PACKET(packet, ModifyBearerResponseID);
			fn_NetSim_SMF_UPF_LTENR_MODIFY_BEARER_RESPONSE_RECV(handoverHdr);
			fn_NetSim_Packet_FreePacket(packet);
			break;
		default:
			break;
	}
}
#pragma endregion

#pragma region AMF_INIT
void LTENR_AMF_N2_INTERFACE_INIT(NETSIM_ID AMFID) {
	ptrLTENR_PROTODATA data = LTENR_PROTODATA_GET(AMFID, 0);
	ptrLTE_NR_NGAP_DATA list = data->NGAPDATA;
	ptrLTENR_AMF_VAR var = NULL;
	NETSIM_ID AMFIF;
	while (list) {
		list->in = fn_NetSim_LTENR_CORE_INTERFACE(AMFID, list->IFType);
		AMFIF = list->in;
		ptrLTENR_AMF_VAR info = LTENR_AMF_VAR_ALLOC();
		info->amfN2ID = AMFID;
		info->amfN2IF = AMFIF;
		info->interfacetype = nGC_INTERFACE_N1_N2;
		LTENR_AMF_VAR_ADD(var, info);

		iptable_add(IP_WRAPPER_GET(AMFID),
			list->destIPaddr,
			STR_TO_IP4("255.255.255.255"),
			0,
			list->destIPaddr,
			1,
			&DEVICE_NWADDRESS(AMFID, AMFIF),
			&AMFIF,
			1,
			"LTE_NR");
		list = LTENR_NGAP_DATA_NEXT(list);
	}
	LTENR_AMF_NGAP_SET(AMFID, var);
}

void LTENR_AMF_N11_INTERFACE_INIT(NETSIM_ID AMFID) {
	ptrLTENR_PROTODATA data = LTENR_PROTODATA_GET(AMFID, 0);
	ptrLTE_NR_N11STACK_DATA list = data->N11STACKDATA;
	ptrLTENR_AMF_VAR var = NULL;
	NETSIM_ID AMFIF;
	while (list) {
		list->in = fn_NetSim_LTENR_CORE_INTERFACE(AMFID, list->IFType);
		AMFIF = list->in;
		ptrLTENR_AMF_VAR info = LTENR_AMF_VAR_ALLOC();
		info->amfN2ID = AMFID;
		info->amfN2IF = AMFIF;
		info->interfacetype = nGC_INTERFACE_N11;
		LTENR_AMF_VAR_ADD(var, info);
		//AMF--N11--SMF coonected via p2p link
		NETSIM_ID r, rin;
		fn_NetSim_Stack_GetConnectedDevice(AMFID, AMFIF, &r, &rin);
		LTENR_AMF_AddSMF(AMFID, AMFIF, r, rin, info);

		iptable_add(IP_WRAPPER_GET(AMFID),
			list->destIPaddr,
			STR_TO_IP4("255.255.255.255"),
			0,
			list->destIPaddr,
			1,
			&DEVICE_NWADDRESS(AMFID, AMFIF),
			&AMFIF,
			1,
			"LTE_NR");
		list = LTENR_N11STACK_DATA_NEXT(list);
	}
	LTENR_AMF_NAS_MM_SET(AMFID, var);
}

void fn_NetSim_LTENR_AMF_INIT(NETSIM_ID AMFID) {
	ptrLTENR_PROTODATA data = LTENR_PROTODATA_GET(AMFID, 0);

	if (data->isN11STACK) {
		LTENR_AMF_N11_INTERFACE_INIT(AMFID);
	}
	if (data->isNGAPDATA) {
		LTENR_AMF_N2_INTERFACE_INIT(AMFID);
	}
}
#pragma endregion
