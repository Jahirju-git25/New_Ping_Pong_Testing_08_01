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
#include "LTENR_Core.h"
#include "LTENR_SMF_UPF.h"
#pragma endregion

#pragma region CORE_INIT
void fn_NetSim_LTENR_CORE_MSG_HDR_INIT() {
	InitalUEMsgHDRID = LTENR_HDR_REGISTER("INITIAL_UE_MSG", NULL, 
						LTENR_CORE_UE_INITIAL_MSG_COPY,
						LTENR_CORE_UE_INITIAL_MSG_FREEHDR,
						LTENR_CORE_UE_INITIAL_MSG_TRACE);
	CreateSessionRequestID = LTENR_HDR_REGISTER("CREATE_SESSION_REQUEST", NULL, 
						LTENR_CORE_CREATE_SESSION_REQUEST_COPY,
						LTENR_CORE_CREATE_SESSION_REQUEST_FREEHDR, 
						LTENR_CORE_CREATE_SESSION_REQUEST_TRACE);
	CreateSessionResponseID = LTENR_HDR_REGISTER("CREATE_SESSION_RESPONSE", NULL,
						LTENR_CORE_CREATE_SESSION_RESPONSE_COPY,
						LTENR_CORE_CREATE_SESSION_RESPONSE_FREEHDR,
						LTENR_CORE_CREATE_SESSION_RESPONSE_TRACE);
	NsmfPDUSessionUpdateSMRequestID = LTENR_HDR_REGISTER("NSMF_PDU_SESSION_UPDATE_SM_CONTEXT_REQUEST",
		NULL, NULL, NULL, NULL);
	NsmfPDUSessionUpdateSMResponseID = LTENR_HDR_REGISTER("NSMF_PDU_SESSION_UPDATE_SM_CONTEXT_RESPONSE",
		NULL, NULL, NULL, NULL);
	PFCPSessionRequestID = LTENR_HDR_REGISTER("PFCP_SESSION_REQUEST",
						NULL, LTENR_PFCP_SESSION_REQUEST_COPY, 
						LTENR_CORE_PFCP_SESSION_REQUEST_FREEHDR, 
						LTENR_CORE_PFCP_SESSION_REQUEST_TRACE);
	PFCPSessionResponseID = LTENR_HDR_REGISTER("PFCP_SESSION_RESPONSE",
						NULL, LTENR_PFCP_SESSION_RESPONSE_COPY,
						LTENR_CORE_PFCP_SESSION_RESPONSE_FREEHDR,
						LTENR_CORE_PFCP_SESSION_RESPONSE_TRACE);
	InitialContextSetupRequestID = LTENR_HDR_REGISTER("INITIAL_CONTEXT_SETUP_REQUEST", 
						NULL, LTENR_CORE_INITIAL_CONTEXT_SETUP_REQUEST_COPY, 
						LTENR_CORE_INITIAL_CONTEXT_SETUP_REQUEST_FREEHDR,
						LTENR_CORE_INITIAL_CONTEXT_SETUP_REQUEST_TRACE);
	InitialContextSetupResponseID = LTENR_HDR_REGISTER("INITIAL_CONTEXT_SETUP_RESPONSE", 
						NULL, LTENR_CORE_INITIAL_CONTEXT_SETUP_RESPONSE_COPY, 
						LTENR_CORE_INITIAL_CONTEXT_SETUP_RESPONSE_FREEHDR, 
						LTENR_CORE_INITIAL_CONTEXT_SETUP_RESPONSE_TRACE);
	RegistrationCompleteID = LTENR_HDR_REGISTER("REGISTRATION_COMPLETE", NULL, NULL, NULL, NULL);
	ModifyBearerRequestID = LTENR_HDR_REGISTER("MODIFY_BEARER_REQUEST", NULL, NULL, NULL, NULL);
	ModifyBearerResponseID = LTENR_HDR_REGISTER("MODIFY_BEARER_RESPONSE", NULL, NULL, NULL, NULL);
}

void fn_NetSim_LTENR_CORE_Init() {
	fn_NetSim_LTENR_CORE_MSG_HDR_INIT();
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENT_GENERATE_CORE_NSMF_PDUSESSION_UPDATESMCONTEXT_REQUEST,
		"LTENR_GENERATE_CORE_NSMF_PDUSESSION_UPDATESMCONTEXT_REQUEST",
		NULL);
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENT_GENERATE_CORE_NSMF_PDUSESSION_UPDATESMCONTEXT_RESPONSE,
		"LTENR_GENERATE_CORE_NSMF_PDUSESSION_UPDATESMCONTEXT_RESPONSE",
		NULL);
}
#pragma endregion

#pragma region INITIAL_UE_MSG
void fn_NetSim_LTENR_GNB_CORE_INITIAL_UE_MSG(ptrLTENR_CORE_HDR hdr) {
	NETSIM_IPAddress srcIP = DEVICE_NWADDRESS(hdr->gNBID, hdr->gNBIF);
	NETSIM_IPAddress destIP = DEVICE_NWADDRESS(hdr->AMFID, hdr->AMFIF);
	ptrLTENR_GNB_CORE_UE_INITIAL_MEAASAGE msg = calloc(1, sizeof * msg);
	msg->amfN2UEID = hdr->AMFID;
	msg->establishmentCause = EstablishmentCause_mo_data;
	msg->gnbN2UEID = hdr->gNBID;
	msg->nssi = LTENR_CORE_NSSI_NOT_ALLOWED;
	hdr->msg = msg;

	ptrLTENR_CORE_NGAP_HEADER apphdr = calloc(1, sizeof * apphdr);
	apphdr->procedureCode = NGAP_PROCEDURECODE_INITIAL_UE_MSG;
	apphdr->lenghtofIEs = 0;
	apphdr->noofIEs = 0;
	hdr->apphdr = apphdr;

	LTENR_Core_Send_Packet(hdr->gNBID, hdr->AMFID, srcIP, destIP, hdr, sizeof * hdr,
		LTENR_MSG_CORE_INITIAL_UE_MESSAGE,
		strLTENR_MSGTYPE[LTENR_MSG_CORE_INITIAL_UE_MESSAGE % (MAC_PROTOCOL_LTE_NR * 100)],
		InitalUEMsgHDRID);
}
#pragma endregion

#pragma region INITIAL_CONTEXT_SETUP_RESPONSE
void fn_NetSim_LTENR_GNB_CORE_INTITAL_CONTEXT_SETUP_RESPONSE(ptrLTENR_CORE_HDR hdr) {

	ptrLTENR_AMF_INITIAL_CONTEXT_RESPONSE msg = calloc(1, sizeof * msg);
	msg->AMFID = hdr->AMFID;
	msg->gNBID = hdr->gNBID;
	msg->UEID = hdr->UEID;
	hdr->msg = msg;
	hdr->msgType = LTENR_MSG_CORE_INITIAL_CONTEXT_SETUP_RESPONSE;

	ptrLTENR_CORE_NGAP_HEADER apphdr = calloc(1, sizeof * apphdr);
	apphdr->procedureCode = NGAP_PROCEDURECODE_INITIAL_CONTEXT_SETUP_RESPONSE;
	apphdr->lenghtofIEs = 0;
	apphdr->noofIEs = 0;
	hdr->apphdr = apphdr;

	NETSIM_IPAddress srcIP = DEVICE_NWADDRESS(hdr->gNBID, hdr->gNBIF);
	NETSIM_IPAddress destIP = DEVICE_NWADDRESS(hdr->AMFID, hdr->AMFIF);
	LTENR_Core_Send_Packet(hdr->gNBID, hdr->AMFID, srcIP, destIP, hdr, sizeof * hdr,
		LTENR_MSG_CORE_INITIAL_CONTEXT_SETUP_RESPONSE,
		strLTENR_MSGTYPE[LTENR_MSG_CORE_INITIAL_CONTEXT_SETUP_RESPONSE % (MAC_PROTOCOL_LTE_NR * 100)],
		InitialContextSetupResponseID);
}
#pragma endregion 

#pragma region INITIAL_CONTEXT_SETUP_REQUEST_RECV
void fn_NetSIM_LTENR_GNB_CORE_GENERATE_INITIAL_CONTEXT_SETUP_REQUEST_RECV(ptrLTENR_CORE_HDR hdr) {
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;

	ptrLTENR_AMF_INITIAL_CONTEXT_REQUEST hdrMsg = hdr->msg;

	//Add BearerInfoto setup into ue
	ptrLTENR_UERRC rrc = LTENR_UERRC_GET(hdr->UEID, hdr->UEIF);

	ptrLTENR_Bearer_Setup bearer = LTENR_Bearer_Setup_ALLOC();
	bearer->epsBearer = LTENR_EPSBEARER_Copy_Bearer(hdrMsg->bearer->epsBearer);
	bearer->epsBearerID = hdrMsg->bearer->epsBearerID;
	bearer->smfteid = hdrMsg->bearer->smfteid;
	bearer->transportLayerIP = IP_COPY(hdrMsg->bearer->transportLayerIP);
	rrc->bearer = bearer;

	ptrLTENR_CORE_HDR msg = calloc(1, sizeof * msg);
	memcpy(msg, hdr, sizeof * msg);
	msg->msgType = LTENR_MSG_CORE_INITIAL_CONTEXT_SETUP_RESPONSE;
	msg->msg = NULL;
	fn_NetSim_LTENR_GNB_CORE_INTITAL_CONTEXT_SETUP_RESPONSE(msg);
}
#pragma endregion

#pragma region PATH_SWITCH_ACK_RECV
void fn_NetSim_GNB_CORE_PATH_SWITCH_ACK_RECV(ptrLTENR_HANDOVER_Hdr hdr) {
	ptrLTENR_HANDOVER_Hdr msg = calloc(1, sizeof * msg);
	memcpy(msg, hdr, sizeof * msg);
	msg->msg = NULL;
	LTENR_HANDOVER_UE_CONTEXT_RELEASE(msg);
}
#pragma endregion

#pragma region PATH_SWITCH
void fn_NetSim_LTENR_GNB_CORE_PATH_SWITCH_MSG(ptrLTENR_HANDOVER_Hdr hdr) {
	NETSIM_IPAddress srcIP = DEVICE_NWADDRESS(hdr->targetCellID, 
		fn_NetSim_LTENR_CORE_INTERFACE(hdr->targetCellID, nGC_INTERFACE_N1_N2));
	NETSIM_IPAddress destIP = DEVICE_NWADDRESS(hdr->targetAMFID, hdr->targetAMFIF);
	ptrLTENR_NAS_PATH_SWITCH_REQUEST msg = calloc(1, sizeof * msg);
	msg->targetGNBID = hdr->targetCellID;
	msg->associatedEPCtoTarget = hdr->targetAMFID;
	hdr->msg = msg;

	ptrLTENR_CORE_NGAP_HEADER apphdr = calloc(1, sizeof * apphdr);
	apphdr->procedureCode = NGAP_PROCEDURECODE_PATH_SWITCH;
	apphdr->lenghtofIEs = 0;
	apphdr->noofIEs = 0;
	hdr->apphdr = apphdr;

	LTENR_Core_Send_Packet(hdr->targetCellID, hdr->targetAMFID, srcIP, destIP, hdr, sizeof * hdr,
		LTENR_MSG_NAS_HANDOVER_PATH_SWITCH,
		strLTENR_MSGTYPE[LTENR_MSG_NAS_HANDOVER_PATH_SWITCH % (MAC_PROTOCOL_LTE_NR * 100)],
		nasPathSwitch);
}
#pragma endregion

#pragma region RECV_FROM_NR_INTERFACE
void fn_NetSim_LTENNR_RECV_FROM_NR_INTERFACE(NetSim_PACKET* packet) {
	if (LTENR_IS_UPLANE_MSG(packet)) {
		//NETSIM_ID TEID = fn_NetSim_LTENR_Get_TEID(packet);
		//fn_NetSim_LTENR_ADD_GTPU_HEADER(packet, TEID);
		//fn_NetSIM_LTENR_GNB_CORE_SEND_USER_PLANE(packet);
	}
	else {
		//fn_NetSIM_LTENR_GNB_CORE_SEND_CONTROL_PLANE(packet);
	}
}
#pragma endregion

#pragma region SEND_TO_NR_INTERFACE
void fn_NetSim_LTENNR_SEND_TO_NR_INTERFACE(NetSim_PACKET* packet, NETSIM_ID TEID) {
	/*fn_NetSim_LTENR_REMOVE_GTPU_HEADER(packet);
	fn_NetSim_LTENR_Set_TEID(packet, TEID);
	PASS_TO_NR_INTERFACE(packet);*/
}
#pragma endregion

#pragma region CORE_HDR_ADD_INTO_PACKET
void fn_NetSIm_LTENR_CORE_ADD_HDR_INTO_PACKET(NetSim_PACKET* packet,
		ptrLTENR_CORE_HDR hdr, UINT hdrID, LTENR_MSGTYPE type) {
	LTENR_RRC_HDR_ADD_TO_PACKET(packet, hdr, hdrID, sizeof(hdr), type);
}

ptrLTENR_CORE_HDR LTENR_CORE_HDR_GET(NetSim_PACKET* packet, UINT hdrID)
{
	return (ptrLTENR_CORE_HDR)LTENR_HDR_GET_FROM_PACKET(packet, hdrID);
}

void LTENR_CORE_HDR_REMOVE(NetSim_PACKET* packet, UINT hdrID)
{
	LTENR_CORE_HDR_FREE_FROM_PACKET(packet, hdrID);
}
#pragma endregion

#pragma region OUT_TO_AMF
void LTENR_CORE_NETWORKOUT(NETSIM_ID d, NETSIM_ID outIn)
{
	NetSim_PACKET* packet = pstruEventDetails->pPacket;

	if (isBroadcastPacket(packet) ||
		isMulticastPacket(packet))
	{
		//No broadcast and multicast
		fn_NetSim_Packet_FreePacket(packet);
		return;
	}

	NETSIM_ID dest = get_first_dest_from_packet(packet);
	if (!outIn)
	{
		print_ltenr_log("Device %d is not connected to DEVICE %d\n", dest, d);
		fn_NetSim_Packet_FreePacket(packet);
		return;
	}

	NetSim_BUFFER* buf = DEVICE_ACCESSBUFFER(d, outIn);
		NetSim_EVENTDETAILS pevent;
		memset(&pevent, 0, sizeof pevent);
		pevent.dEventTime = pstruEventDetails->dEventTime;
		pevent.nDeviceId = d;
		pevent.nDeviceType = DEVICE_TYPE(d);
		pevent.nEventType = APPLICATION_OUT_EVENT;
		pevent.nInterfaceId = outIn;
		pevent.nPacketId = pstruEventDetails->nPacketId;
		pevent.nProtocolId = APP_PROTOCOL_NULL;
		pevent.pPacket = pstruEventDetails->pPacket;
		fnpAddEvent(&pevent);
		fn_NetSim_Packet_AddPacketToList(buf, packet, 0);
		pstruEventDetails->pPacket = NULL;
}
#pragma endregion

#pragma region GNB_CORE_PACKET_HANDLER
void fn_NetSim_LTENR_GNB_CORE_PACKET_HANDLER() {
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	ptrLTENR_CORE_HDR hdr = packet->pstruAppData->Packet_AppProtocol;
	ptrLTENR_HANDOVER_Hdr handoverHdr = NULL;

	switch (packet->nControlDataType) {
	case LTENR_MSG_CORE_INITIAL_CONTEXT_SETUP_REQUEST:
		hdr = LTENR_CORE_HDR_GET_FROM_PACKET(packet, InitialContextSetupRequestID);
		fn_NetSIM_LTENR_GNB_CORE_GENERATE_INITIAL_CONTEXT_SETUP_REQUEST_RECV(hdr);
		fn_NetSim_Packet_FreePacket(packet);
		break;
	case LTENR_MSG_NAS_HANDOVER_PATH_SWITCH_ACK:
		handoverHdr = LTENR_CORE_HDR_GET_FROM_PACKET(packet, naspathSwitchAck);
		fn_NetSim_GNB_CORE_PATH_SWITCH_ACK_RECV(handoverHdr);
		fn_NetSim_Packet_FreePacket(packet);
		break;
	default:
		break;
	}
}
#pragma endregion

#pragma region NONRANGNB_INIT
void fn_MetSim_LTENR_NON_RAN_GNB_N3_Init(NETSIM_ID d) {
	ptrLTENR_PROTODATA data = LTENR_PROTODATA_GET(d, 0);
	ptrLTE_NR_GTPU_DATA list = data->GTPUDATA;
	NETSIM_ID in;
	while (list) {
		list->in = fn_NetSim_LTENR_CORE_INTERFACE(d, list->IFType);
		in = list->in;
		iptable_add(IP_WRAPPER_GET(d),
			list->destIPaddr,
			STR_TO_IP4("255.255.255.255"),
			0,
			list->destIPaddr,
			1,
			&DEVICE_NWADDRESS(d, in),
			&in,
			1,
			"LTE_NR");
		list = LTENR_GTPU_DATA_NEXT(list);
	}
}
void fn_MetSim_LTENR_NON_RAN_GNB_N2_Init(NETSIM_ID d) {
	ptrLTENR_PROTODATA data = LTENR_PROTODATA_GET(d, 0);
	ptrLTE_NR_NGAP_DATA list = data->NGAPDATA;
	NETSIM_ID in;
	while (list) {
		list->in = fn_NetSim_LTENR_CORE_INTERFACE(d, list->IFType);
		in = list->in;
		iptable_add(IP_WRAPPER_GET(d),
			list->destIPaddr,
			STR_TO_IP4("255.255.255.255"),
			0,
			list->destIPaddr,
			1,
			&DEVICE_NWADDRESS(d, in),
			&in,
			1,
			"LTE_NR");
		list = LTENR_NGAP_DATA_NEXT(list);
	}
}
void fn_NetSim_LTENR_NON_RAN_GNB_INIT(NETSIM_ID GNBID) {
	ptrLTENR_PROTODATA data = LTENR_PROTODATA_GET(GNBID, 0);
	if (data->isGTPUDATA) {
		fn_MetSim_LTENR_NON_RAN_GNB_N3_Init(GNBID);
	}
	if (data->isNGAPDATA) {
		fn_MetSim_LTENR_NON_RAN_GNB_N2_Init(GNBID);
	}
}
#pragma endregion

#pragma region UE_INITIAL_MSG_COPY_TRACE_REMOVE
void LTENR_CORE_UE_INITIAL_MSG_TRACE(ptrLTENR_CORE_HDR hdr, char* s)
{
	ptrLTENR_GNB_CORE_UE_INITIAL_MEAASAGE msg = hdr->msg;
	char info[BUFSIZ];
	sprintf(info, "Hdr Type = %s\n"
		"amfN2UEID = %d\ngnbN2UEID = %d\n"
		"establishmentCause = %s\n"
		"nssi = %s\n",
		"UE_INITIAL_MSG", msg->amfN2UEID, msg->gnbN2UEID, 
		strLTENR_RRC_EstablishmentCause[msg->establishmentCause],
		strLTENR_CORE_NSSI[msg->nssi]);
	strcat(s, info);
}

ptrLTENR_CORE_HDR LTENR_CORE_UE_INITIAL_MSG_COPY(ptrLTENR_CORE_HDR hdr)
{
	ptrLTENR_CORE_HDR ret = calloc(1, sizeof * ret);
	memcpy(ret, hdr, sizeof * ret);
	ptrLTENR_GNB_CORE_UE_INITIAL_MEAASAGE msg = calloc(1, sizeof * msg);
	memcpy(msg, hdr->msg, sizeof * msg);
	ret->msg = msg;
	ptrLTENR_CORE_NGAP_HEADER ngap = calloc(1, sizeof * ngap);
	memcpy(ngap, hdr->apphdr, sizeof * ngap);
	ret->apphdr = ngap;
	return ret;
}

void LTENR_CORE_UE_INITIAL_MSG_FREEHDR(ptrLTENR_CORE_HDR hdr) {
	ptrLTENR_GNB_CORE_UE_INITIAL_MEAASAGE msg = hdr->msg;
	ptrLTENR_CORE_NGAP_HEADER ngap = hdr->apphdr;
	free(ngap);
	free(msg);
	free(hdr);
}
#pragma endregion

#pragma region CREATE_SESSION_REQUEST_COPY_TRACE_REMOVE
void LTENR_CORE_CREATE_SESSION_REQUEST_TRACE(ptrLTENR_CORE_HDR hdr, char* s)
{
	ptrLTENR_AMF_SMF_CREATE_SESSION_REQUEST_MSG msg = hdr->msg;
	char info[BUFSIZ];
	sprintf(info, "Hdr Type = %s\n"
		"ULI = %d\nEPS Bearer ID = %d\n",
		"CREATE_SESSION_REQUEST", msg->uli, msg->bearer->epsBearerID);
	strcat(s, info);
}

ptrLTENR_CORE_HDR LTENR_CORE_CREATE_SESSION_REQUEST_COPY(ptrLTENR_CORE_HDR hdr)
{
	ptrLTENR_CORE_HDR ret = calloc(1, sizeof * ret);
	memcpy(ret, hdr, sizeof * ret);
	ptrLTENR_AMF_SMF_CREATE_SESSION_REQUEST_MSG msg = calloc(1, sizeof * msg);
	memcpy(msg, hdr->msg, sizeof * msg);
	ptrLTENR_AMF_SMF_CREATE_SESSION_REQUEST_MSG temp = hdr->msg;
	ptrLTENR_EPSBEARER bearer = LTENR_EPSBEARER_Copy_Bearer(temp->bearer->epsBearer);
	ptrLTENR_Bearer_Context bearerContext = calloc(1, sizeof * bearerContext);
	memcpy(bearerContext, temp->bearer, sizeof * bearerContext);
	bearerContext->epsBearer = NULL;
	bearerContext->epsBearer = bearer;
	msg->bearer = bearerContext;
	ret->msg = msg;

	ptrLTENR_GTPC_HEADER GTPC = calloc(1, sizeof * GTPC);
	memcpy(GTPC, hdr->apphdr, sizeof * GTPC);
	ret->apphdr = GTPC;
	return ret;
}

void LTENR_CORE_CREATE_SESSION_REQUEST_FREEHDR(ptrLTENR_CORE_HDR hdr) {
	ptrLTENR_AMF_SMF_CREATE_SESSION_REQUEST_MSG msg = hdr->msg;
	ptrLTENR_GTPC_HEADER GTPC = hdr->apphdr;
	free(GTPC);
	LTENR_EPSBEARER_Delete_Bearer(msg->bearer->epsBearer);
	free(msg->bearer);
	free(msg);
	free(hdr);
}
#pragma endregion

#pragma region CREATE_SESSION_RESPONSE_COPY_TRACE_REMOVE
void LTENR_CORE_CREATE_SESSION_RESPONSE_TRACE(ptrLTENR_CORE_HDR hdr, char* s)
{
	ptrLTENR_AMF_SMF_CREATE_SESSION_RESPONSE_MSG msg = hdr->msg;
	char info[BUFSIZ];
	sprintf(info, "Hdr Type = %s\n"
		"AMFID = %d\n SMFTEID = %d\n EPS Bearer ID = %d\n",
		"CREATE_SESSION_RESPONSE", msg->AMFID, msg->Bearer->smfte->smfteid,msg->Bearer->epsBearerID);
	strcat(s, info);
}

ptrLTENR_CORE_HDR LTENR_CORE_CREATE_SESSION_RESPONSE_COPY(ptrLTENR_CORE_HDR hdr)
{
	ptrLTENR_CORE_HDR ret = calloc(1, sizeof * ret);
	memcpy(ret, hdr, sizeof * ret);
	ptrLTENR_AMF_SMF_CREATE_SESSION_RESPONSE_MSG msg = calloc(1, sizeof * msg);
	memcpy(msg, hdr->msg, sizeof * msg);
	ptrLTENR_AMF_SMF_CREATE_SESSION_RESPONSE_MSG temp = hdr->msg;
	ptrLTENR_Bearer_Activate bearer = LTENR_Bearer_Activate_ALLOC();
	bearer->epsBearerID = temp->Bearer->epsBearerID;
	bearer->epsBearer = LTENR_EPSBEARER_Copy_Bearer(temp->Bearer->epsBearer);
	msg->Bearer = bearer;
	ptrLTENR_FULLY_QUALIFIED_TEID teid = calloc(1, sizeof * teid);
	teid->smfteid = temp->Bearer->smfte->smfteid;
	teid->TXaddress = IP_COPY(temp->Bearer->smfte->TXaddress);
	msg->Bearer->smfte = teid;
	ret->msg = msg;

	ptrLTENR_GTPC_HEADER GTPC = calloc(1, sizeof * GTPC);
	memcpy(GTPC, hdr->apphdr, sizeof * GTPC);
	ret->apphdr = GTPC;
	return ret;
}

void LTENR_CORE_CREATE_SESSION_RESPONSE_FREEHDR(ptrLTENR_CORE_HDR hdr) {
	ptrLTENR_AMF_SMF_CREATE_SESSION_RESPONSE_MSG msg = hdr->msg;
	ptrLTENR_GTPC_HEADER GTPC = hdr->apphdr;
	free(GTPC);
	if(msg->Bearer->epsBearer)
		LTENR_EPSBEARER_Delete_Bearer(msg->Bearer->epsBearer);
	if (msg->Bearer->smfte) {
		if(msg->Bearer->smfte->TXaddress)
			IP_FREE(msg->Bearer->smfte->TXaddress);
		free(msg->Bearer->smfte);
	}
	free(msg->Bearer);
	free(msg);
	free(hdr);
}
#pragma endregion

#pragma region Nsmf_PFCP_SESSION_REQUEST_COPY_TRACE_REMOVE
void LTENR_CORE_PFCP_SESSION_REQUEST_TRACE(ptrLTENR_CORE_HDR hdr, char* s)
{
	ptrLTENR_SMF_UPF_PFCP_SESSION_REQUEST msg = hdr->msg;
	char info[BUFSIZ];
	sprintf(info, "Hdr Type = %s\n"
		"UPFID = %d\nSMFID = %d\n"
		"GNBID = %d\n"
		"UEID = %d\n",
		"PFCP_SESSION_ESTABLISHMENT_REQUEST",
		msg->UPFID, msg->SMFID,
		msg->gNBID, msg->UEID);
	strcat(s, info);
}

ptrLTENR_CORE_HDR LTENR_PFCP_SESSION_REQUEST_COPY(ptrLTENR_CORE_HDR hdr)
{
	ptrLTENR_CORE_HDR ret = calloc(1, sizeof * ret);
	memcpy(ret, hdr, sizeof * ret);
	ptrLTENR_SMF_UPF_PFCP_SESSION_REQUEST msgInfo = hdr->msg;
	ptrLTENR_SMF_UPF_PFCP_SESSION_REQUEST msg = calloc(1, sizeof * msg);
	memcpy(msg, hdr->msg, sizeof * msg);
	ptrLTENR_Bearer_Context bearer = LTENR_Bearer_Context_ALLOC();
	bearer->epsBearer = LTENR_EPSBEARER_Copy_Bearer(msgInfo->Bearer->epsBearer);
	bearer->epsBearerID = msgInfo->Bearer->epsBearerID;
	msg->Bearer = bearer;
	ret->msg = msg;
	return ret;
}

void LTENR_CORE_PFCP_SESSION_REQUEST_FREEHDR(ptrLTENR_CORE_HDR hdr) {
	ptrLTENR_SMF_UPF_PFCP_SESSION_REQUEST msg = hdr->msg;
	LTENR_EPSBEARER_Delete_Bearer(msg->Bearer->epsBearer);
	free(msg->Bearer);
	free(msg);
	free(hdr);
}
#pragma endregion

#pragma region Nsmf_PFCP_SESSION_RESPONSE_COPY_TRACE_REMOVE
void LTENR_CORE_PFCP_SESSION_RESPONSE_TRACE(ptrLTENR_CORE_HDR hdr, char* s)
{
	ptrLTENR_SMF_UPF_PFCP_SESSION_RESPONSE msg = hdr->msg;
	char info[BUFSIZ];
	sprintf(info, "Hdr Type = %s\n"
		"UPFID = %d\nSMFID = %d\n"
		"GNBID = %d\n"
		"UEID = %d\n",
		"PFCP_SESSION_ESTABLISHMENT_RESPONSE",
		msg->UPFID, msg->SMFID,
		msg->gNBID, msg->UEID);
	strcat(s, info);
}

ptrLTENR_CORE_HDR LTENR_PFCP_SESSION_RESPONSE_COPY(ptrLTENR_CORE_HDR hdr)
{
	ptrLTENR_CORE_HDR ret = calloc(1, sizeof * ret);
	memcpy(ret, hdr, sizeof * ret);
	ptrLTENR_SMF_UPF_PFCP_SESSION_RESPONSE msgInfo = hdr->msg;
	ptrLTENR_SMF_UPF_PFCP_SESSION_RESPONSE msg = calloc(1, sizeof * msg);
	memcpy(msg, hdr->msg, sizeof * msg);
	ptrLTENR_Bearer_Context bearer = LTENR_Bearer_Context_ALLOC();
	bearer->epsBearer = LTENR_EPSBEARER_Copy_Bearer(msgInfo->Bearer->epsBearer);
	bearer->epsBearerID = msgInfo->Bearer->epsBearerID;
	msg->Bearer = bearer;
	ret->msg = msg;
	return ret;
}

void LTENR_CORE_PFCP_SESSION_RESPONSE_FREEHDR(ptrLTENR_CORE_HDR hdr) {
	ptrLTENR_SMF_UPF_PFCP_SESSION_RESPONSE msg = hdr->msg;
	LTENR_EPSBEARER_Delete_Bearer(msg->Bearer->epsBearer);
	free(msg->Bearer);
	free(msg);
	free(hdr);
}
#pragma endregion

#pragma region INITIAL_CONTEXT_SETUP_REQUEST_COPY_TRACE_REMOVE
void LTENR_CORE_INITIAL_CONTEXT_SETUP_REQUEST_TRACE(ptrLTENR_CORE_HDR hdr, char* s)
{
	ptrLTENR_AMF_INITIAL_CONTEXT_REQUEST msg = hdr->msg;
	char info[BUFSIZ];
	sprintf(info, "Hdr Type = %s\n"
		"AMFID = %d\nBearerID = %d\n"
		"GNBID = %d\n"
		"UEID = %d\n",
		"INITIAL_CONTEXT_SETUP_REQUEST",
		msg->AMFID, msg->bearer->epsBearerID,
		msg->gNBID, msg->UEID);
	strcat(s, info);
}

ptrLTENR_CORE_HDR LTENR_CORE_INITIAL_CONTEXT_SETUP_REQUEST_COPY(ptrLTENR_CORE_HDR hdr)
{
	ptrLTENR_CORE_HDR ret = calloc(1, sizeof * ret);
	memcpy(ret, hdr, sizeof * ret);
	ptrLTENR_AMF_INITIAL_CONTEXT_REQUEST msgInfo = hdr->msg;
	ptrLTENR_AMF_INITIAL_CONTEXT_REQUEST msg = calloc(1, sizeof * msg);
	memcpy(msg, hdr->msg, sizeof * msg);
	ptrLTENR_Bearer_Setup bearer = LTENR_Bearer_Setup_ALLOC();
	bearer->epsBearer = LTENR_EPSBEARER_Copy_Bearer(msgInfo->bearer->epsBearer);
	bearer->epsBearerID = msgInfo->bearer->epsBearerID;
	bearer->smfteid = msgInfo->bearer->smfteid;
	bearer->transportLayerIP = IP_COPY(msgInfo->bearer->transportLayerIP);
	msg->bearer = bearer;
	ret->msg = msg;

	ptrLTENR_CORE_NGAP_HEADER NGAP = calloc(1, sizeof * NGAP);
	memcpy(NGAP, hdr->apphdr, sizeof * NGAP);
	ret->apphdr = NGAP;
	return ret;
}

void LTENR_CORE_INITIAL_CONTEXT_SETUP_REQUEST_FREEHDR(ptrLTENR_CORE_HDR hdr) {
	ptrLTENR_AMF_INITIAL_CONTEXT_REQUEST msg = hdr->msg;
	ptrLTENR_CORE_NGAP_HEADER NGAP = hdr->apphdr;
	free(NGAP);
	LTENR_EPSBEARER_Delete_Bearer(msg->bearer->epsBearer);
	free(msg->bearer);
	free(msg);
	free(hdr);
}
#pragma endregion

#pragma region INITIAL_CONTEXT_SETUP_RESPONSE_COPY_TRACE_REMOVE
void LTENR_CORE_INITIAL_CONTEXT_SETUP_RESPONSE_TRACE(ptrLTENR_CORE_HDR hdr, char* s)
{
	ptrLTENR_AMF_INITIAL_CONTEXT_RESPONSE msg = hdr->msg;
	char info[BUFSIZ];
	sprintf(info, "Hdr Type = %s\n"
		"AMFID = %d\n"
		"GNBID = %d\n"
		"UEID = %d\n",
		"INITIAL_CONTEXT_SETUP_RESPONSE",
		msg->AMFID,
		msg->gNBID, msg->UEID);
	strcat(s, info);
}

ptrLTENR_CORE_HDR LTENR_CORE_INITIAL_CONTEXT_SETUP_RESPONSE_COPY(ptrLTENR_CORE_HDR hdr)
{
	ptrLTENR_CORE_HDR ret = calloc(1, sizeof * ret);
	memcpy(ret, hdr, sizeof * ret);
	ptrLTENR_AMF_INITIAL_CONTEXT_RESPONSE msg = calloc(1, sizeof * msg);
	memcpy(msg, hdr->msg, sizeof * msg);
	ret->msg = msg;

	ptrLTENR_CORE_NGAP_HEADER NGAP = calloc(1, sizeof * NGAP);
	memcpy(NGAP, hdr->apphdr, sizeof * NGAP);
	ret->apphdr = NGAP;
	return ret;
}

void LTENR_CORE_INITIAL_CONTEXT_SETUP_RESPONSE_FREEHDR(ptrLTENR_CORE_HDR hdr) {
	ptrLTENR_AMF_INITIAL_CONTEXT_RESPONSE msg = hdr->msg;
	ptrLTENR_CORE_NGAP_HEADER NGAP = hdr->apphdr;
	free(NGAP);
	free(msg);
	free(hdr);
}
#pragma endregion

#pragma region PACKET_FORWARD
void gNB_N3_Forward_packet_to_RAN(NETSIM_ID d, NETSIM_ID in, NETSIM_ID rin, NetSim_PACKET* packet)
{
	if(!fn_NetSim_GetBufferStatus(DEVICE_ACCESSBUFFER(d,rin)))
	{
		NetSim_EVENTDETAILS pevent;
		memset(&pevent, 0, sizeof pevent);
		pevent.dEventTime = ldEventTime;
		pevent.dPacketSize = packet->pstruNetworkData->dPacketSize;
		if (packet->pstruAppData)
		{
			pevent.nApplicationId = packet->pstruAppData->nApplicationId;
			pevent.nSegmentId = packet->pstruAppData->nSegmentId;
		}
		pevent.nDeviceId = d;
		pevent.nDeviceType = DEVICE_TYPE(d);
		pevent.nEventType = MAC_OUT_EVENT;
		pevent.nInterfaceId = rin;
		pevent.nPacketId = packet->nPacketId;
		pevent.nProtocolId = MAC_PROTOCOL_LTE_NR;
		fnpAddEvent(&pevent);
	}
	fn_NetSim_Packet_AddPacketToList(DEVICE_ACCESSBUFFER(d, rin), packet, 0);
}

void gNB_N3_Forward_packet_to_MAC(NETSIM_ID d, NETSIM_ID rin, NetSim_PACKET* packet)
{
	packet->pstruMacData->szSourceMac = (fn_NetSim_Stack_GetMacAddressFromIP(packet->pstruNetworkData->szGatewayIP));
	NETSIM_IPAddress szDestIPaddr = packet->pstruNetworkData->szNextHopIp;
	if (isBroadcastIP(szDestIPaddr))
		packet->pstruMacData->szDestMac = BROADCAST_MAC;
	else if (isMulticastIP(szDestIPaddr))
		packet->pstruMacData->szDestMac = multicastIP_to_Mac(szDestIPaddr);
	else
		packet->pstruMacData->szDestMac = fn_NetSim_Stack_GetMacAddressFromIP(szDestIPaddr);

	if (!fn_NetSim_GetBufferStatus(DEVICE_ACCESSBUFFER(d, rin)))
	{
		NetSim_EVENTDETAILS pevent;
		memset(&pevent, 0, sizeof pevent);
		pevent.dEventTime = ldEventTime;
		pevent.dPacketSize = packet->pstruNetworkData->dPacketSize;
		if (packet->pstruAppData)
		{
			pevent.nApplicationId = packet->pstruAppData->nApplicationId;
			pevent.nSegmentId = packet->pstruAppData->nSegmentId;
		}
		pevent.nDeviceId = d;
		pevent.nDeviceType = DEVICE_TYPE(d);
		pevent.nEventType = MAC_OUT_EVENT;
		pevent.nInterfaceId = rin;
		pevent.nPacketId = packet->nPacketId;
		pevent.nProtocolId = DEVICE_MACLAYER(d,rin)->nMacProtocolId;
		fnpAddEvent(&pevent);
	}
	fn_NetSim_Packet_AddPacketToList(DEVICE_ACCESSBUFFER(d, rin), packet, 0);
}

void fn_NetSim_LTENR_LTE_S1_CONTROL_PACKET_TO_RAN(NETSIM_ID d, NETSIM_ID RANIF, NetSim_PACKET* packet) {
	NetSim_BUFFER* buf = DEVICE_ACCESSBUFFER(d, RANIF);
	NetSim_PACKET* p = packet;
	if (!fn_NetSim_GetBufferStatus(buf))
	{
		NetSim_EVENTDETAILS pevent;
		memset(&pevent, 0, sizeof pevent);
		pevent.dEventTime = pstruEventDetails->dEventTime;
		pevent.dPacketSize = p->pstruMacData->dPacketSize;
		if (p->pstruAppData)
		{
			pevent.nApplicationId = p->pstruAppData->nApplicationId;
			pevent.nSegmentId = p->pstruAppData->nSegmentId;
		}
		pevent.nDeviceId = d;
		pevent.nDeviceType = DEVICE_TYPE(d);
		pevent.nEventType = MAC_OUT_EVENT;
		pevent.nInterfaceId = RANIF;
		pevent.nPacketId = p->nPacketId;
		pevent.nProtocolId = DEVICE_MACLAYER(d, RANIF)->nMacProtocolId;
		fnpAddEvent(&pevent);
	}
	fn_NetSim_Packet_AddPacketToList(buf, p, 0);
}
#pragma endregion
