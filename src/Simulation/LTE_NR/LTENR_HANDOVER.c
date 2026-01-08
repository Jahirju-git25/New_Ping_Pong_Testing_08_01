/************************************************************************************
* Copyright (C) 2023
* TETCOS, Bangalore. India
************************************************************************************/

#pragma region HEADER_FILES
#include "stdafx.h"
#include "LTENR_GNBRRC.h"
#include "LTENR_NAS.h"
#include "LTENR_PHY.h"
#include "LTENR_EPC.h"
#include <winsock2.h>
#pragma endregion

/* ------------------------------------------------------------------------- */
/* ---------------------  PING-PONG RUNTIME INTERFACE  ---------------------- */
/* ------------------------------------------------------------------------- */

#pragma region PINGPONG_RUNTIME_INTERFACE

extern SOCKET clientSocket;   // reused from Power-Control runtime

#define MAX_UE 1024

static int lastServingCell[MAX_UE] = { 0 };
static double lastHOTime[MAX_UE] = { 0 };

#pragma endregion

/* ------------------------------------------------------------------------- */
/* -------------------------  PING-PONG FUNCTION  --------------------------- */
/* ------------------------------------------------------------------------- */

void send_pingpong_event(ptrLTENR_HANDOVER_Hdr hdr)
{
	int ue = hdr->UEID;
	int prev = lastServingCell[ue];
	int curr = hdr->targetCellID;
	double now = pstruEventDetails->dEventTime;

	if (prev == 0) {
		lastServingCell[ue] = curr;
		lastHOTime[ue] = now;
		return;
	}

	/* Ping-pong: UE returns to same cell within 2 seconds */
	if (prev == curr && (now - lastHOTime[ue]) < 2.0) {
		char msg[256];
		sprintf(msg,
			"PINGPONG,UE,%d,FROM,%d,TO,%d,DT,%lf\n",
			ue,
			hdr->serveringCellID,
			hdr->targetCellID,
			now - lastHOTime[ue]);

		send(clientSocket, msg, (int)strlen(msg), 0);
	}

	lastServingCell[ue] = curr;
	lastHOTime[ue] = now;
}

/* ------------------------------------------------------------------------- */
/* ------------------ UE CONTEXT RELEASE ACK -------------------------------- */
/* ------------------------------------------------------------------------- */

void LTENR_HANDOVER_UE_CONTEXT_RELEASE_ACK(ptrLTENR_HANDOVER_Hdr hdr)
{
	hdr->msgType = LTENR_MSG_NAS_HANDOVER_UE_CONTEXT_RELEASE_ACK;

	ptrLTENR_NAS_UECONTEXTRELEASECOMMAND msg = calloc(1, sizeof * msg);
	msg->ueID = hdr->UEID;
	msg->outCome = HANDOVER_OUTCOME_SUCESSFUL;
	msg->cause = HANDOVER_CAUSE_RADIO_NETWORK;
	hdr->msg = msg;

	NETSIM_IPAddress srcIP = DEVICE_NWADDRESS(
		hdr->serveringCellID,
		fn_NetSim_LTENR_CORE_INTERFACE(hdr->serveringCellID, nGC_INTERFACE_XN));

	NETSIM_IPAddress destIP = DEVICE_NWADDRESS(
		hdr->targetCellID,
		fn_NetSim_LTENR_CORE_INTERFACE(hdr->targetCellID, nGC_INTERFACE_XN));

	LTENR_Handover_Send_Packet(
		hdr->serveringCellID,
		hdr->targetCellID,
		srcIP,
		destIP,
		hdr,
		sizeof * hdr,
		LTENR_MSG_NAS_HANDOVER_UE_CONTEXT_RELEASE_ACK,
		strLTENR_MSGTYPE[LTENR_MSG_NAS_HANDOVER_UE_CONTEXT_RELEASE_ACK %
		(MAC_PROTOCOL_LTE_NR * 100)],
		nasUEContextReleaseACK);
}

void LTENR_HANDOVER_UE_CONTEXT_RELEASE_ACK_RECV(ptrLTENR_HANDOVER_Hdr hdr)
{
	ptrLTENR_NAS_UECONTEXTRELEASECOMMAND msg = hdr->msg;

	if (msg->outCome != HANDOVER_OUTCOME_SUCESSFUL) {
		fnNetSimError("UE context release unsuccessful");
		return;
	}

	/* 🔥 Ping-pong detection happens here (handover complete) */
	send_pingpong_event(hdr);
}

/* ------------------------------------------------------------------------- */
/* ------------------ UE CONTEXT RELEASE ------------------------------------ */
/* ------------------------------------------------------------------------- */

void LTENR_HANDOVER_UE_CONTEXT_RELEASE(ptrLTENR_HANDOVER_Hdr hdr)
{
	hdr->msgType = LTENR_MSG_NAS_HANDOVER_UE_CONTEXT_RELEASE;

	ptrLTENR_NAS_UECONTEXTRELEASEREQUEST msg = calloc(1, sizeof * msg);
	msg->cause = HANDOVER_CAUSE_RADIO_NETWORK;
	msg->gnb_UE_ID = hdr->targetCellIF;
	msg->mme_UE_ID = hdr->AMFID;
	hdr->msg = msg;

	NETSIM_IPAddress srcIP = DEVICE_NWADDRESS(
		hdr->targetCellID,
		fn_NetSim_LTENR_CORE_INTERFACE(hdr->targetCellID, nGC_INTERFACE_XN));

	NETSIM_IPAddress destIP = DEVICE_NWADDRESS(
		hdr->serveringCellID,
		fn_NetSim_LTENR_CORE_INTERFACE(hdr->serveringCellID, nGC_INTERFACE_XN));

	LTENR_Handover_Send_Packet(
		hdr->targetCellID,
		hdr->serveringCellID,
		srcIP,
		destIP,
		hdr,
		sizeof * hdr,
		LTENR_MSG_NAS_HANDOVER_UE_CONTEXT_RELEASE,
		strLTENR_MSGTYPE[LTENR_MSG_NAS_HANDOVER_UE_CONTEXT_RELEASE %
		(MAC_PROTOCOL_LTE_NR * 100)],
		nasUEContextRelease);
}

void LTENR_HANDOVER_UE_CONTEXT_RELEASE_RECV(ptrLTENR_HANDOVER_Hdr hdr)
{
	ptrLTENR_HANDOVER_Hdr msg = calloc(1, sizeof * msg);
	memcpy(msg, hdr, sizeof * msg);
	msg->msg = NULL;
	LTENR_HANDOVER_UE_CONTEXT_RELEASE_ACK(msg);
}

/* ------------------------------------------------------------------------- */
/* ------------------ HANDOVER REQUEST -------------------------------------- */
/* ------------------------------------------------------------------------- */

void LTENR_HANDOVER_REQUEST(ptrLTENR_HANDOVER_Hdr hdr)
{
	ptrLTENR_NAS_HANDOVER_REQUEST msg = calloc(1, sizeof * msg);
	msg->MME_UE_ID = hdr->AMFID;
	msg->type = HANDOVER_TYPE_INTRA_LTENR;
	msg->cause = HANDOVER_CAUSE_RADIO_NETWORK;
	msg->targetID = hdr->targetCellID;
	msg->lastVisitedCellID = hdr->serveringCellID;
	msg->LastVisitedCellType = CELL_TYPE_LARGE;

	hdr->msg = msg;
	hdr->msgType = LTENR_MSG_NAS_HANDOVER_REQUEST;

	NETSIM_IPAddress srcIP = DEVICE_NWADDRESS(
		hdr->serveringCellID,
		fn_NetSim_LTENR_CORE_INTERFACE(hdr->serveringCellID, nGC_INTERFACE_XN));

	NETSIM_IPAddress destIP = DEVICE_NWADDRESS(
		hdr->targetCellID,
		fn_NetSim_LTENR_CORE_INTERFACE(hdr->targetCellID, nGC_INTERFACE_XN));

	LTENR_Handover_Send_Packet(
		hdr->serveringCellID,
		hdr->targetCellID,
		srcIP,
		destIP,
		hdr,
		sizeof * hdr,
		LTENR_MSG_NAS_HANDOVER_REQUEST,
		strLTENR_MSGTYPE[LTENR_MSG_NAS_HANDOVER_REQUEST %
		(MAC_PROTOCOL_LTE_NR * 100)],
		nasHandoverRequest);
}

void LTENR_HANDOVER_REQUEST_RECV(ptrLTENR_HANDOVER_Hdr hdr)
{
	ptrLTENR_HANDOVER_Hdr msg = calloc(1, sizeof * msg);
	memcpy(msg, hdr, sizeof * msg);
	msg->msg = NULL;
	LTENR_HANDOVER_REQUEST_ACK(msg);
}

/* ------------------------------------------------------------------------- */
/* ------------------ HANDOVER REQUEST ACK ---------------------------------- */
/* ------------------------------------------------------------------------- */

void LTENR_HANDOVER_REQUEST_ACK(ptrLTENR_HANDOVER_Hdr hdr)
{
	ptrLTENR_NAS_HANDOVER_REQUEST_ACK msg = calloc(1, sizeof * msg);
	msg->GNB_UE_ID = hdr->serveringCellID;
	msg->MME_UE_ID = hdr->AMFID;
	msg->outCome = HANDOVER_OUTCOME_SUCESSFUL;

	hdr->msgType = LTENR_MSG_NAS_HANDOVER_REQUEST_ACK;
	hdr->msg = msg;

	NETSIM_IPAddress srcIP = DEVICE_NWADDRESS(
		hdr->targetCellID,
		fn_NetSim_LTENR_CORE_INTERFACE(hdr->targetCellID, nGC_INTERFACE_XN));

	NETSIM_IPAddress destIP = DEVICE_NWADDRESS(
		hdr->serveringCellID,
		fn_NetSim_LTENR_CORE_INTERFACE(hdr->serveringCellID, nGC_INTERFACE_XN));

	LTENR_Handover_Send_Packet(
		hdr->targetCellID,
		hdr->serveringCellID,
		srcIP,
		destIP,
		hdr,
		sizeof * hdr,
		LTENR_MSG_NAS_HANDOVER_REQUEST_ACK,
		strLTENR_MSGTYPE[LTENR_MSG_NAS_HANDOVER_REQUEST_ACK %
		(MAC_PROTOCOL_LTE_NR * 100)],
		nasHandoverRequestAck);
}

void LTENR_HANDOVER_REQUEST_ACK_RECV(ptrLTENR_HANDOVER_Hdr hdr)
{
	ptrLTENR_HANDOVER_Hdr msg = calloc(1, sizeof * msg);
	memcpy(msg, hdr, sizeof * msg);
	msg->msg = NULL;
	fn_NetSim_LTENR_NAS_HANDOVER_COMMAND(msg);
}

/* ------------------------------------------------------------------------- */
/* ------------------ PACKET RECEIVE HANDLER -------------------------------- */
/* ------------------------------------------------------------------------- */

void fn_NetSim_LTENR_Handover_RECV()
{
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	ptrLTENR_HANDOVER_Hdr hdr = NULL;

	switch (packet->nControlDataType)
	{
	case LTENR_MSG_NAS_HANDOVER_REQUEST:
		hdr = LTENR_HANDOVER_HDR_GET_FROM_PACKET(packet, nasHandoverRequest);
		LTENR_HANDOVER_REQUEST_RECV(hdr);
		LTENR_HANDOVER_HDR_FREE_FROM_PACKET(packet, nasHandoverRequest);
		break;

	case LTENR_MSG_NAS_HANDOVER_REQUEST_ACK:
		hdr = LTENR_HANDOVER_HDR_GET_FROM_PACKET(packet, nasHandoverRequestAck);
		LTENR_HANDOVER_REQUEST_ACK_RECV(hdr);
		LTENR_HANDOVER_HDR_FREE_FROM_PACKET(packet, nasHandoverRequestAck);
		break;

	case LTENR_MSG_NAS_HANDOVER_UE_CONTEXT_RELEASE:
		hdr = LTENR_HANDOVER_HDR_GET_FROM_PACKET(packet, nasUEContextRelease);
		LTENR_HANDOVER_UE_CONTEXT_RELEASE_RECV(hdr);
		LTENR_HANDOVER_HDR_FREE_FROM_PACKET(packet, nasUEContextRelease);
		break;

	case LTENR_MSG_NAS_HANDOVER_UE_CONTEXT_RELEASE_ACK:
		hdr = LTENR_HANDOVER_HDR_GET_FROM_PACKET(packet, nasUEContextReleaseACK);
		LTENR_HANDOVER_UE_CONTEXT_RELEASE_ACK_RECV(hdr);
		LTENR_HANDOVER_HDR_FREE_FROM_PACKET(packet, nasUEContextReleaseACK);
		break;

	default:
		break;
	}
}
