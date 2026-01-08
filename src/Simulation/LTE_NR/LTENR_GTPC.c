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
#include "LTENR_GTPC.h"
#pragma endregion

#pragma region DEFULT_HEADER
ptrLTENR_GTPC_HEADER fn_NetSim_LTENR_GTPC_DEFAULT_HEADER() {
	ptrLTENR_GTPC_HEADER hdr = calloc(1, sizeof * hdr);
	hdr->V = 2;
	hdr->P = false;
	hdr->T = false;
	hdr->MP = false;
	hdr->messageType = 0;
	hdr->length = 0;
	hdr->TEID = 0;
	hdr->SequenceNumber = 0;
	hdr->NextExtensionType = 0;
	return hdr;
}
#pragma endregion

#pragma region FREE_HDR
void fn_NetSim_LTENR_GTPC_HEADER_FREE(ptrLTENR_GTPC_HEADER hdr) {
	free(hdr);
}
#pragma endregion

#pragma region ADD_GTPC_HEADER_INTO_PACKET
void fn_NetSim_LTENR_ADD_GTPC_HEADER(NetSim_PACKET* packet, NETSIM_ID TEID) {
	ptrLTENR_GTPC_HEADER hdr = fn_NetSim_LTENR_GTPC_DEFAULT_HEADER();
	hdr->TEID = TEID;
	hdr->length = packet->pstruAppData->dPayload + GTPC_HEADER_SIZE;
	packet->pstruAppData->Packet_AppProtocol = hdr;
}
#pragma endregion

#pragma region REMOVE_GTPC_HEADER_FROM_PACKET
void fn_NetSim_LTENR_REMOVE_GTPC_HEADER(NetSim_PACKET* packet) {
	ptrLTENR_GTPC_HEADER hdr = packet->pstruAppData->Packet_AppProtocol;
	fn_NetSim_LTENR_GTPC_HEADER_FREE(hdr);
}
#pragma endregion