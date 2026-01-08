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
#define  _CRT_SECURE_NO_WARNINGS

#pragma region HEADER_FILES
#include "stdafx.h"
#include "LTENR_Core.h"
#include "LTENR_SMF_UPF.h"
#pragma endregion

#pragma region FIND_SMF_VAR
ptrLTENR_SMF_VAR LTENR_GTP_C_FIND_SMF_VAR(NETSIM_ID SMFID, NETSIM_ID SMFIF) {
	ptrLTENR_SMF_VAR var = LTENR_SMF_NAS_SM_GET(SMFID);
	while (var) {
		if (var->SMFID == SMFID && var->SMFIF == SMFIF)
			return var;
	}
	return NULL;
}

ptrLTENR_SMF_VAR LTENR_PFCP_FIND_SMF_VAR(NETSIM_ID SMFID, NETSIM_ID SMFIF) {
	ptrLTENR_SMF_VAR var = LTENR_SMF_PFCP_GET(SMFID);
	while (var) {
		if (var->SMFID == SMFID && var->SMFIF == SMFIF)
			return var;
	}
	return NULL;
}

ptrLTENR_UPF_VAR LTENR_PFCP_FIND_UPF_VAR(NETSIM_ID UPFID, NETSIM_ID UPFIF) {
	ptrLTENR_UPF_VAR var = LTENR_UPF_PFCP_GET(UPFID);
	while (var) {
		if (var->UPFID == UPFID && var->UPFIF == UPFIF)
			return var;
	}
	return NULL;
}

ptrLTENR_UPF_VAR LTENR_GTP_U_FIND_UPF_VAR(NETSIM_ID UPFID, NETSIM_ID UPFIF) {
	ptrLTENR_UPF_VAR var = LTENR_UPF_GTP_U_GET(UPFID);
	while (var) {
		if (var->UPFID == UPFID && var->UPFIF == UPFIF)
			return var;
	}
	return NULL;
}
#pragma endregion

#pragma region UPF_SELECTION
NETSIM_ID LTENR_UPF_GetNewUPF(NETSIM_ID UEID, NETSIM_ID UEIF) {
	ptrLTENR_UERRC rrc = LTENR_UERRC_GET(UEID, UEIF);
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(rrc->SelectedCellID, rrc->SelectedCellIF);
	return fn_NetSim_Stack_GetDeviceId_asName(pd->connectedUPFName);
}
#pragma endregion

#pragma region SMF_SELECTION
NETSIM_ID LTENR_SMF_GetNewSMF(NETSIM_ID UEID, NETSIM_ID UEIF) {
	ptrLTENR_UERRC rrc = LTENR_UERRC_GET(UEID, UEIF);
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(rrc->SelectedCellID, rrc->SelectedCellIF);
	return fn_NetSim_Stack_GetDeviceId_asName(pd->connectedSMFName);
}
#pragma endregion

#pragma region SMF_ASSOCIATION
void LTENR_SMF_AddgNB(NETSIM_ID SMFID, NETSIM_ID SMFIF,
					  NETSIM_ID gNBID, NETSIM_ID gNBIF) {
	ptrLTENR_SMF_VAR var = LTENR_GTP_C_FIND_SMF_VAR(SMFID, SMFIF);
	ptrLTENR_SMF_GNB_Info info = LTENR_SMF_GNB_Info_ALLOC();
	info->gNBID = gNBID;
	info->gNBIF = gNBIF;
	info->SMFID = SMFID;
	info->SMFIF = SMFIF;
	info->gNBIP = DEVICE_NWADDRESS(gNBID, gNBIF);
	info->SMFIP = DEVICE_NWADDRESS(SMFID, SMFIF);
	LTENR_SMF_GNB_Info_ADD(var->gnbList, info);
}

ptrLTENR_SMF_GNB_Info LTENR_SMF_FINDgNB(NETSIM_ID SMFID, NETSIM_ID SMFIF,
	NETSIM_ID gNBID, NETSIM_ID gNBIF) {
	ptrLTENR_SMF_VAR var = LTENR_GTP_C_FIND_SMF_VAR(SMFID, SMFIF);
	ptrLTENR_SMF_GNB_Info info = var->gnbList;
	while (info) {
		if (info->gNBID == gNBID && info->gNBIF == gNBIF)
			return info;
		info = LTENR_SMF_GNB_Info_NEXT(info);
	}
	return NULL;
}

void LTENR_SMF_AddUE(NETSIM_ID SMFID, NETSIM_ID SMFIF,
	NETSIM_ID gNBID, NETSIM_ID gNBIF,
	NETSIM_ID UEID, NETSIM_ID UEIF) {
	ptrLTENR_UERRC rrc = LTENR_UERRC_GET(UEID, UEIF);
	rrc->SelectedSMFID = SMFID;
	rrc->SelectedSMFIF = SMFIF;
	ptrLTENR_SMF_VAR var = LTENR_GTP_C_FIND_SMF_VAR(SMFID, SMFIF);
	ptrLTENR_SMF_UE_Info info = LTENR_SMF_UE_Info_ALLOC();
	info->gnbID = gNBID;
	info->gnbIF = gNBIF;
	info->UEID = UEID;
	info->UEIF = UEIF;
	info->gnbLTENRIP = DEVICE_NWADDRESS(rrc->SelectedCellID, rrc->SelectedCellIF);
	info->ueIP = DEVICE_NWADDRESS(UEID, UEIF);
	LTENR_SMF_UE_Info_ADD(var->ueInfo, info);
}

void LTENR_SMF_DeleteUE(NETSIM_ID SMFID, NETSIM_ID SMFIF,
						NETSIM_ID UEID, NETSIM_ID UEIF) {
	ptrLTENR_SMF_VAR var = LTENR_GTP_C_FIND_SMF_VAR(SMFID, SMFIF);
	ptrLTENR_SMF_UE_Info info = var->ueInfo;
	while (info) {
		if (info->UEID == UEID && info->UEIF == UEIF)
			LTENR_SMF_UE_Info_REMOVE(var->ueInfo, info);
		info = LIST_NEXT(info);
	}
}

ptrLTENR_SMF_UE_Info LTENR_SMF_FindUE(NETSIM_ID SMFID, NETSIM_ID SMFIF,
										NETSIM_ID UEID, NETSIM_ID UEIF) {
	ptrLTENR_SMF_VAR var = LTENR_GTP_C_FIND_SMF_VAR(SMFID, SMFIF);
	ptrLTENR_SMF_UE_Info info = var->ueInfo;
	while (info) {
		if (info->UEID == UEID && info->UEIF == UEIF)
			return info;
		info = LIST_NEXT(info);
	}
	return NULL;
}

void LTENR_SMF_AddAMF(NETSIM_ID SMFID, NETSIM_ID SMFIF,
					  NETSIM_ID AMFID, NETSIM_ID AMFIF, ptrLTENR_SMF_VAR var) {
	ptrLTENR_SMF_AMF_Info info = LTENR_SMF_AMF_Info_ALLOC();
	info->AMFID = AMFID;
	info->AMFIF = AMFIF;
	LTENR_SMF_AMF_Info_ADD(var->AMFInfo, info);
}

void LTENR_SMF_AddUPF(NETSIM_ID SMFID, NETSIM_ID SMFIF,
					  NETSIM_ID UPFID, NETSIM_ID UPFIF, ptrLTENR_SMF_VAR var) {
	ptrLTENR_SMF_UPF_Info info = LTENR_SMF_AMF_Info_ALLOC();
	info->UPFID = UPFID;
	info->UPFIF = UPFIF;
	LTENR_SMF_AMF_Info_ADD(var->UPFInfo, info);
}

void LTENR_SMF_AddBearer(ptrLTENR_SMF_UE_Info info, ptrLTENR_Bearer_Context BreaerInfo) {
	info->associateBearer = BreaerInfo;
}

void LTENR_SMF_RemoveBearer(ptrLTENR_SMF_UE_Info info, NETSIM_ID bearerID) {
	ptrLTENR_Bearer_Context BearerContext = info->associateBearer;
	while (BearerContext) {
		if (BearerContext->epsBearerID == bearerID)
			LTENR_Bearer_Context_REMOVE(info->associateBearer, BearerContext);
		BearerContext = LIST_NEXT(BearerContext);
	}
}
#pragma endregion

#pragma region CREATE_SESSIONS_REPONSE
void LTENR_SMF_CREATE_SESSION_REPONSE(ptrLTENR_CORE_HDR hdr, 
									  ptrLTENR_Bearer_Context bearer) {

	hdr->msgType = LTENR_MSG_CORE_CREATE_SESSION_RESPONSE;
	ptrLTENR_AMF_SMF_CREATE_SESSION_RESPONSE_MSG msg = calloc(1, sizeof * msg);
	msg->AMFID = hdr->AMFID;
	msg->gNBID = hdr->gNBID;
	msg->UEID = hdr->UEID;
	ptrLTENR_Bearer_Activate ActivateBearer = LTENR_Bearer_Activate_ALLOC();
	ActivateBearer->epsBearer = LTENR_EPSBEARER_Copy_Bearer(bearer->epsBearer);
	ActivateBearer->epsBearerID = bearer->epsBearerID;
	ActivateBearer->smfte = calloc(1, sizeof * ActivateBearer->smfte);
	ActivateBearer->smfte->smfteid = ++smfTEID;
	ActivateBearer->smfte->TXaddress = DEVICE_NWADDRESS(hdr->gNBID, hdr->gNBIF);
	LTENR_Bearer_Activate_ADD(msg->Bearer, ActivateBearer);
	hdr->msg = msg;

	ptrLTENR_GTPC_HEADER apphdr = fn_NetSim_LTENR_GTPC_DEFAULT_HEADER();
	hdr->apphdr = apphdr;

	NETSIM_IPAddress srcIP = DEVICE_NWADDRESS(hdr->SMFID,hdr->SMFIF);
	NETSIM_IPAddress destIP = DEVICE_NWADDRESS(hdr->AMFID, 
		fn_NetSim_LTENR_CORE_INTERFACE(hdr->AMFID, nGC_INTERFACE_N11));
	LTENR_Core_Send_Packet(hdr->SMFID, hdr->AMFID, srcIP, destIP, hdr, sizeof * hdr,
		LTENR_MSG_CORE_CREATE_SESSION_RESPONSE,
		strLTENR_MSGTYPE[LTENR_MSG_CORE_CREATE_SESSION_RESPONSE % (MAC_PROTOCOL_LTE_NR * 100)],
		CreateSessionResponseID);
}
#pragma endregion

#pragma region PDCP_SESSION_RESPONSE
void fn_NetSim_LTENR_SMF_UPF_PFCP_SESSION_RESPONSE(ptrLTENR_CORE_HDR hdr) {
	NETSIM_IPAddress srcIP = DEVICE_NWADDRESS(hdr->UPFID,
		fn_NetSim_LTENR_CORE_INTERFACE(hdr->UPFID, nGC_INTERFACE_N4));
	NETSIM_IPAddress destIP = DEVICE_NWADDRESS(hdr->SMFID, hdr->SMFN4IF);
	LTENR_Core_Send_Packet(hdr->UPFID, hdr->SMFID, srcIP, destIP, hdr, sizeof * hdr,
		LTENR_MSG_CORE_PFCP_SESSION_RESPONSE,
		strLTENR_MSGTYPE[LTENR_MSG_CORE_PFCP_SESSION_RESPONSE % (MAC_PROTOCOL_LTE_NR * 100)],
		PFCPSessionResponseID);
}

void fn_NetSim_LTENR_SMF_UPF_GENERATE_PFCP_SESSION_RESPONSE_RECV(ptrLTENR_CORE_HDR hdr) {
	ptrLTENR_SMF_UPF_PFCP_SESSION_RESPONSE ret = hdr->msg;
	ptrLTENR_Bearer_Context bearer = LTENR_Bearer_Context_ALLOC();
	memcpy(bearer, ret->Bearer, sizeof * bearer);
	bearer->epsBearer = LTENR_EPSBEARER_Copy_Bearer(ret->Bearer->epsBearer);

	ptrLTENR_CORE_HDR msg = calloc(1, sizeof * msg);
	memcpy(msg, hdr, sizeof * msg);
	msg->msgType = LTENR_MSG_CORE_CREATE_SESSION_RESPONSE;
	msg->msg = NULL;

	LTENR_SMF_CREATE_SESSION_REPONSE(msg, bearer);
}
#pragma endregion

#pragma region PFCP_SESSION_REQUEST
void fn_NetSim_LTENR_SMF_UPF_PFCP_SESSION_REQUEST(ptrLTENR_CORE_HDR hdr, ptrLTENR_Bearer_Context bearer) {
	ptrLTENR_SMF_UPF_PFCP_SESSION_REQUEST msg = calloc(1, sizeof * msg);
	msg->UPFID = hdr->UPFID;
	msg->gNBID = hdr->gNBID;
	msg->SMFID = hdr->SMFID;
	msg->UEID = hdr->UEID;
	msg->Bearer = bearer;
	hdr->msg = msg;
	hdr->msgType = LTENR_MSG_CORE_PFCP_SESSION_REQUSET;
	NETSIM_IPAddress srcIP = DEVICE_NWADDRESS(hdr->SMFID,hdr->SMFN4IF);
	NETSIM_IPAddress destIP = DEVICE_NWADDRESS(hdr->UPFID,
							  fn_NetSim_LTENR_CORE_INTERFACE(hdr->UPFID, nGC_INTERFACE_N4));
	LTENR_Core_Send_Packet(hdr->SMFID, hdr->UPFID, srcIP, destIP, hdr, sizeof * hdr,
		LTENR_MSG_CORE_PFCP_SESSION_REQUSET,
		strLTENR_MSGTYPE[LTENR_MSG_CORE_PFCP_SESSION_REQUSET % (MAC_PROTOCOL_LTE_NR * 100)],
		PFCPSessionRequestID);
}

void fn_NetSim_LTENR_SMF_UPF_GENERATE_PFCP_SESSION_REQUEST_RECV(ptrLTENR_CORE_HDR hdr) {
	ptrLTENR_SMF_UPF_PFCP_SESSION_REQUEST ret = hdr->msg;
	ptrLTENR_Bearer_Context bearer = LTENR_Bearer_Context_ALLOC();
	memcpy(bearer, ret->Bearer, sizeof * bearer);
	bearer->epsBearer = LTENR_EPSBEARER_Copy_Bearer(ret->Bearer->epsBearer);

	ptrLTENR_CORE_HDR msg = calloc(1, sizeof * msg);
	memcpy(msg, hdr, sizeof * msg);
	msg->msgType = LTENR_MSG_CORE_PFCP_SESSION_RESPONSE;
	 
	ptrLTENR_SMF_UPF_PFCP_SESSION_RESPONSE res = calloc(1, sizeof * res);
	res->Bearer = bearer;
	res->gNBID = msg->gNBID;
	res->SMFID = msg->SMFID;
	res->UEID = msg->UEID;
	res->UPFID = msg->UPFID;
	msg->msg = res;

	fn_NetSim_LTENR_SMF_UPF_PFCP_SESSION_RESPONSE(msg);
}
#pragma endregion

#pragma region CREATE_SESSION_REQUEST_RECV
void fn_NetSim_SMF_UPF_LTENR_CREATE_SESSION_REQUEST_RECV(ptrLTENR_CORE_HDR hdr) {
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	ptrLTENR_CORE_HDR msg = LTENR_CORE_CREATE_SESSION_REQUEST_COPY(hdr);
	ptrLTENR_AMF_SMF_CREATE_SESSION_REQUEST_MSG req = msg->msg;
	ptrLTENR_Bearer_Context bearer = LTENR_Bearer_Context_ALLOC();
	memcpy(bearer, req->bearer, sizeof * bearer);
	bearer->epsBearer = LTENR_EPSBEARER_Copy_Bearer(req->bearer->epsBearer);
	msg->msg = NULL;

	//ptrLTENR_SMF_VAR var = LTENR_SMF_GET(msg->SMFID, msg->SMFIF);
	LTENR_SMF_AddgNB(msg->SMFID, msg->SMFIF, msg->gNBID, msg->gnbUuIF);
	LTENR_SMF_AddUE(msg->SMFID, msg->SMFIF, msg->gNBID, msg->gnbUuIF, msg->UEID, msg->UEIF);
	ptrLTENR_SMF_UE_Info info = LTENR_SMF_FindUE(msg->SMFID, msg->SMFIF, msg->UEID, msg->UEIF);
	LTENR_SMF_AddBearer(info, bearer);

	NETSIM_ID gnbN3IF = fn_NetSim_LTENR_CORE_INTERFACE(hdr->gNBID, nGC_INTERFACE_N3);
	fn_NetSim_LTENR_SMF_UPF_PFCP_SESSION_REQUEST(msg, bearer);
}
#pragma endregion

#pragma region MODIFY_BEARER_RESPONSE
void fn_NetSim_LTENR_AMF_MODIFY_BEARER_RESPONSE(ptrLTENR_HANDOVER_Hdr hdr) {
	hdr->msgType = LTENR_MSG_CORE_MODIFY_BEARER_RESPONSE;
	ptrLTENR_AMF_MODIFY_BEARER_REQUEST msg = calloc(1, sizeof * msg);
	msg->SAMFID = hdr->AMFID;
	msg->SgNBID = hdr->serveringCellID;
	msg->TgNBID = hdr->targetCellID;
	msg->TAMFID = hdr->targetAMFID;
	msg->UEID = hdr->UEID;
	hdr->msg = msg;

	NETSIM_ID SMFIF = fn_NetSim_LTENR_CORE_INTERFACE(hdr->associateSMFID, nGC_INTERFACE_N11);
	NETSIM_ID AMFIF = fn_NetSim_LTENR_CORE_INTERFACE(hdr->targetAMFID, nGC_INTERFACE_N11);
	NETSIM_IPAddress srcIP = DEVICE_NWADDRESS(hdr->associateSMFID, SMFIF);
	NETSIM_IPAddress destIP = DEVICE_NWADDRESS(hdr->targetAMFID, AMFIF);

	if (!LTENR_SMF_FINDgNB(hdr->associateSMFID, SMFIF, hdr->targetCellID, hdr->targetCellIF))
		LTENR_SMF_AddgNB(hdr->associateSMFID, SMFIF, hdr->targetCellID, hdr->targetCellIF);

	LTENR_SMF_DeleteUE(hdr->associateSMFID, SMFIF, hdr->UEID, hdr->UEIF);
	LTENR_SMF_AddUE(hdr->associateSMFID, SMFIF, hdr->serveringCellID, hdr->serveringCellIF, hdr->UEID, hdr->UEIF);

	LTENR_Core_Send_Packet(hdr->associateSMFID, hdr->targetAMFID, srcIP, destIP, hdr, sizeof * hdr,
		LTENR_MSG_CORE_MODIFY_BEARER_RESPONSE,
		strLTENR_MSGTYPE[LTENR_MSG_CORE_MODIFY_BEARER_RESPONSE % (MAC_PROTOCOL_LTE_NR * 100)],
		ModifyBearerResponseID);
}
#pragma endregion

#pragma region MODIFY_BEARER_REQUEST_RECV
void fn_NetSim_SMF_UPF_LTENR_MODIFY_BEARER_REQUEST_RECV(ptrLTENR_HANDOVER_Hdr hdr) {
	//ptrLTENR_AMF_MODIFY_BEARER_REQUEST bearer = hdr->msg;
	//NETSIM_ID SMFIF = fn_NetSim_LTENR_CORE_INTERFACE(hdr->associateSMFID, nGC_INTERFACE_N11);
	//ptrLTENR_SMF_UE_Info info = LTENR_SMF_FindUE(hdr->associateSMFID, SMFIF, hdr->UEID, hdr->UEIF);
	//LTENR_SMF_RemoveBearer(info, bearer->bearer->epsBearerID);
	//Modify bearer if you have QCI different diffrernt for each UE
	ptrLTENR_HANDOVER_Hdr msg = calloc(1, sizeof * msg);
	memcpy(msg, hdr, sizeof * msg);
	msg->msg = NULL;
	fn_NetSim_LTENR_AMF_MODIFY_BEARER_RESPONSE(msg);
}
#pragma endregion

#pragma region RECV_FROM_N6_INTERFACE
void fn_NetSim_LTENR_UPF_RECV_FROM_N6_INTERFACE(NetSim_PACKET* packet) {
	/*
	if (LTENR_IS_UPLANE_MSG(packet)) {
		NETSIM_ID TEID = fn_NetSim_LTENR_Get_TEID(packet);
		fn_NetSim_LTENR_ADD_GTPU_HEADER(packet, TEID);
		fn_NetSIM_LTENR_GNB_CORE_SEND_N3_USER_PLANE(packet);
	}
	else {
		
	}*/
}
#pragma endregion

#pragma region SEND_TO_N6_INTERFACE
void fn_NetSim_LTENR_SEND_TO_N6_INTERFACE(NetSim_PACKET* packet) {
	/*fn_NetSim_LTENR_REMOVE_GTPU_HEADER(packet);
	SEND_TO_N6_INTERFACE(packet);*/
}
#pragma endregion

#pragma region SMF_UPF_INIT
void fn_NetSim_LTENR_SMF_N4_Init(NETSIM_ID SMFID) {
	ptrLTENR_SMF_VAR var = NULL;
	ptrLTENR_PROTODATA data = LTENR_PROTODATA_GET(SMFID, 0);
	ptrLTE_NR_PFCP_DATA list = data->PFCPDATA;
	NETSIM_ID SMFIF;
	while (list) {
		list->in = fn_NetSim_LTENR_CORE_INTERFACE(SMFID, list->IFType);
		SMFIF = list->in;
		ptrLTENR_SMF_VAR info = LTENR_SMF_VAR_ALLOC();
		info->SMFID = SMFID;
		info->SMFIF = SMFIF;
		info->IFType = nGC_INTERFACE_N4;
		LTENR_SMF_VAR_ADD(var, info);

		NETSIM_ID r, rin;
		fn_NetSim_Stack_GetConnectedDevice(SMFID, SMFIF, &r, &rin);
		LTENR_SMF_AddUPF(SMFID, SMFIF, r, rin, info);

		iptable_add(IP_WRAPPER_GET(SMFID),
			list->destIPaddr,
			STR_TO_IP4("255.255.255.255"),
			0,
			list->destIPaddr,
			1,
			&DEVICE_NWADDRESS(SMFID, SMFIF),
			&SMFIF,
			1,
			"LTE_NR");
		list = LTENR_PFCP_DATA_NEXT(list);
	}
	LTENR_SMF_PFCP_SET(SMFID, var);
}

void fn_NetSim_LTENR_SMF_N11_Init(NETSIM_ID SMFID) {
	ptrLTENR_PROTODATA data = LTENR_PROTODATA_GET(SMFID, 0);
	ptrLTE_NR_N11STACK_DATA list = data->N11STACKDATA;
	NETSIM_ID SMFIF;
	ptrLTENR_SMF_VAR var = NULL;
	while (list) {
		list->in = fn_NetSim_LTENR_CORE_INTERFACE(SMFID, list->IFType);
		SMFIF = list->in;
		ptrLTENR_SMF_VAR info = LTENR_SMF_VAR_ALLOC();
		info->SMFID = SMFID;
		info->SMFIF = SMFIF;
		info->IFType = nGC_INTERFACE_N11;
		LTENR_SMF_VAR_ADD(var, info);

		NETSIM_ID r, rin;
		fn_NetSim_Stack_GetConnectedDevice(SMFID, SMFIF, &r, &rin);
		LTENR_SMF_AddAMF(SMFID, SMFIF, r, rin, info);

		iptable_add(IP_WRAPPER_GET(SMFID),
			list->destIPaddr,
			STR_TO_IP4("255.255.255.255"),
			0,
			list->destIPaddr,
			1,
			&DEVICE_NWADDRESS(SMFID, SMFIF),
			&SMFIF,
			1,
			"LTE_NR");
		list = LTENR_N11STACK_DATA_NEXT(list);
	}
	LTENR_SMF_NAS_SM_SET(SMFID, var);
}

void fn_MetSim_LTENR_UPF_N3_Init(NETSIM_ID UPFID) {
	ptrLTENR_PROTODATA data = LTENR_PROTODATA_GET(UPFID, 0);
	ptrLTENR_UPF_VAR var = NULL;
	ptrLTE_NR_GTPU_DATA list = data->GTPUDATA;
	NETSIM_ID UPFIF;
	while (list) {
		list->in = fn_NetSim_LTENR_CORE_INTERFACE(UPFID, list->IFType);
		UPFIF = list->in;
		ptrLTENR_UPF_VAR info = LTENR_UPF_VAR_ALLOC();
		info->UPFID = UPFID;
		info->UPFIF = UPFIF;
		info->IFType = nGC_INTERFACE_N3;
		LTENR_UPF_VAR_ADD(var, info);

		iptable_add(IP_WRAPPER_GET(UPFID),
			list->destIPaddr,
			STR_TO_IP4("255.255.255.255"),
			0,
			list->destIPaddr,
			1,
			&DEVICE_NWADDRESS(UPFID, UPFIF),
			&UPFIF,
			1,
			"LTE_NR");

		list = LTENR_GTPU_DATA_NEXT(list);
	}
	LTENR_UPF_GTP_U_SET(UPFID, var);
	iptable_delete_by_type(IP_WRAPPER_GET(UPFID), "LOCAL");
}

void fn_MetSim_LTENR_UPF_N4_Init(NETSIM_ID UPFID) {
	ptrLTENR_UPF_VAR var = NULL;
	ptrLTENR_PROTODATA data = LTENR_PROTODATA_GET(UPFID, 0);
	ptrLTE_NR_PFCP_DATA list = data->PFCPDATA;
	NETSIM_ID UPFIF;
	while (list) {
		list->in = fn_NetSim_LTENR_CORE_INTERFACE(UPFID, list->IFType);
		UPFIF = list->in;

		ptrLTENR_UPF_VAR info = LTENR_UPF_VAR_ALLOC();
		info->UPFID = UPFID;
		info->UPFIF = UPFIF;
		info->IFType = nGC_INTERFACE_N4;
		LTENR_UPF_VAR_ADD(var, info);

		iptable_add(IP_WRAPPER_GET(UPFID),
			list->destIPaddr,
			STR_TO_IP4("255.255.255.255"),
			0,
			list->destIPaddr,
			1,
			&DEVICE_NWADDRESS(UPFID, UPFIF),
			&UPFIF,
			1,
			"LTE_NR");
		list = LTENR_PFCP_DATA_NEXT(list);
	}
	LTENR_UPF_PFCP_SET(UPFID, var);
}

void fn_MetSim_LTENR_UPF_N6_Init(NETSIM_ID UPFID) {
	NETSIM_ID n6If = fn_NetSim_Stack_GetInterfaceIdByName(UPFID, "5G_N6");
	NETSIM_ID n3If = fn_NetSim_Stack_GetInterfaceIdByName(UPFID, "5G_N3");
	NETSIM_ID rid, rin;
	if (n6If) {
		if (fn_NetSim_Stack_GetConnectedDevice(UPFID, n6If, &rid, &rin)) {
			NETSIM_IPAddress nwAddress = IP_NETWORK_ADDRESS_IPV4(DEVICE_NWADDRESS(UPFID, n3If), DEVICE_SUBNETMASK(UPFID, n3If));
			iptable_add(IP_WRAPPER_GET(rid),
				nwAddress,
				DEVICE_SUBNETMASK(UPFID, n3If),
				0,
				DEVICE_NWADDRESS(UPFID, n6If),
				1,
				&DEVICE_NWADDRESS(rid, rin),
				&rin,
				1,
				"LTE_NR");
		}
	}
}

void fn_NetSim_LTENR_SMF_INIT(NETSIM_ID SMFID) {
	ptrLTENR_PROTODATA data = LTENR_PROTODATA_GET(SMFID, 0);
	if (data->isN11STACK) {
		fn_NetSim_LTENR_SMF_N11_Init(SMFID);
	}
	if (data->isPFCPDATA) {
		fn_NetSim_LTENR_SMF_N4_Init(SMFID);
	}
}

void fn_NetSim_LTENR_UPF_INIT(NETSIM_ID UPFID) {
	ptrLTENR_PROTODATA data = LTENR_PROTODATA_GET(UPFID, 0);
	if (data->isGTPUDATA) {
		fn_MetSim_LTENR_UPF_N3_Init(UPFID);
	}
	if (data->isPFCPDATA) {
		fn_MetSim_LTENR_UPF_N4_Init(UPFID);
	}
	fn_MetSim_LTENR_UPF_N6_Init(UPFID);
}
#pragma endregion

#pragma region SMF_OUT_AND_IN
void fn_NetSim_LTENR_SMF_UPF_PACKET_HANDLER() {
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	ptrLTENR_CORE_HDR hdr = NULL;
	ptrLTENR_HANDOVER_Hdr handoverHdr = NULL;

	switch (packet->nControlDataType) {
	case LTENR_MSG_CORE_CREATE_SESSION_REQUEST:
		hdr = LTENR_CORE_HDR_GET_FROM_PACKET(packet, CreateSessionRequestID);
		fn_NetSim_SMF_UPF_LTENR_CREATE_SESSION_REQUEST_RECV(hdr);
		fn_NetSim_Packet_FreePacket(packet);
		break;
	case LTENR_MSG_CORE_PFCP_SESSION_REQUSET:
		hdr = LTENR_CORE_HDR_GET_FROM_PACKET(packet, PFCPSessionRequestID);
		fn_NetSim_LTENR_SMF_UPF_GENERATE_PFCP_SESSION_REQUEST_RECV(hdr);
		fn_NetSim_Packet_FreePacket(packet);
		break;
	case LTENR_MSG_CORE_PFCP_SESSION_RESPONSE:
		hdr = LTENR_CORE_HDR_GET_FROM_PACKET(packet, PFCPSessionResponseID);
		fn_NetSim_LTENR_SMF_UPF_GENERATE_PFCP_SESSION_RESPONSE_RECV(hdr);
		fn_NetSim_Packet_FreePacket(packet);
		break;
	case LTENR_MSG_CORE_MODIFY_BEARER_REQUEST:
		handoverHdr = LTENR_CORE_HDR_GET_FROM_PACKET(packet, ModifyBearerRequestID);
		fn_NetSim_SMF_UPF_LTENR_MODIFY_BEARER_REQUEST_RECV(handoverHdr);
		fn_NetSim_Packet_FreePacket(packet);
		break;
	default:
		break;
	}
}
#pragma endregion