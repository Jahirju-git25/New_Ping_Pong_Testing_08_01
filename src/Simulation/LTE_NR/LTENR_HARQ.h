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
#pragma once
#ifndef _NETSIM_LTENR_HARQ_H_
#define _NETSIM_LTENR_HARQ_H_
#ifdef  __cplusplus
extern "C" {
#endif

#include "main.h"
#include "LTE_NR.h"

#pragma region COSTANT
#define MAX_HARQ_PROCESS		16
#define MAX_CBGCOUNT_PER_TB		8
#pragma endregion

	typedef struct stru_LTENR_HARQ_PacketBuffer
	{
		NetSim_PACKET* head;
		NetSim_PACKET* tail;
	}LTENR_HARQ_PACKETBUFFER, * ptrLTENR_HARQ_PACKETBUFFER;

	typedef struct stru_LTENR_CBG
	{
		UINT CBCount;
		PACKET_STATUS* CBErrorStatus;
		PACKET_STATUS isErrored;
	}LTENR_CBG, * ptrLTENR_CBG;

	typedef struct stru_LTENR_HARQ_Buffer
	{
		UINT CBGCount;

		UINT bitsPerPRB;
		double bler;
		double sinr;

		bool isNonEmpty;
		PACKET_STATUS isErrored;
		UINT Totalsize;
		UINT PendingSize;
		LTENR_CBG CBGList[MAX_CBGCOUNT_PER_TB];
	}LTENR_HARQ_BUFFER, *ptrLTENR_HARQ_BUFFER;

	typedef struct stru_LTENR_HARQ_Process
	{
		bool isUPLinkProcess;
		int caId;
		int slotId;
		int frameId;
		int subFrameId;

		UINT processId;
		UINT transmissionNumber;
		bool NDI;
		UINT PRBReqdForRetx;

		LTENR_HARQ_PACKETBUFFER packetBuffer;

		LTENR_HARQ_BUFFER transmissionBuffer[MAX_LAYER_COUNT];

		UINT layerCount;
		ptrLTENR_AMCINFO* amcInfo;
	}LTENR_HARQ_PROCESS, * ptrLTENR_HARQ_PROCESS;

	typedef struct stru_LTENR_HARQ_Entity
	{
		NETSIM_ID gNBId;
		NETSIM_ID gNBIf;
		NETSIM_ID ueId;
		NETSIM_ID ueIf;
		UINT maxProcessCount;
		UINT retryLimit;
		UINT maxCBGPerTB;
		UINT currentProcessId;
		UINT BlerTableId;
		bool isUpinkEntity;
		LTENR_HARQ_PROCESS harqProcess[MAX_HARQ_PROCESS];
	}LTENR_HARQ_ENTITY, * ptrLTENR_HARQ_ENTITY;

	typedef struct stru_LTENR_HARQ_var
	{
		LTENR_HARQ_ENTITY downlinkHARQEntity[MAX_CA_COUNT];
		LTENR_HARQ_ENTITY uplinkHARQEntity[MAX_CA_COUNT];
	}LTENR_HARQ_VAR, * ptrLTENR_HARQ_VAR;
#ifdef  __cplusplus
}
#endif
#endif //_NETSIM_LTENR_HARQ_H_
