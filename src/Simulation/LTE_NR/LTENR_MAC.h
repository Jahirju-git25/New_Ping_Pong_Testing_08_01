#pragma once
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
#ifndef _NETSIM_LTENR_MAC_H_
#define _NETSIM_LTENR_MAC_H_
#ifdef  __cplusplus
extern "C" {
#endif

#pragma region HEADER_FILES
#include "LTENR_PHY.h"
#pragma endregion

#pragma region SCHEDULING_TYPE
	typedef enum enum_LTENR_MAC_GnbSchedulingType {
		LTENR_MAC_SCHEDULING_ROUNDROBIN,
		LTENR_MAC_SCHEDULING_PROPORTIONALFAIR,
		LTENR_MAC_SCHEDULING_MAXTHROUGHTPUT,
		LTENR_MAC_SCHEDULING_FAIR_SCHEDULING,
	}LTENR_GnbSchedulingType;
#pragma endregion

#pragma region UE_SCHEDULER_INFO
	typedef struct stru_LTENR_UESchedulerInfo
	{
		__IN__ NETSIM_ID ueId;
		__IN__ NETSIM_ID ueIf;
		__IN__ UINT bufferSize;
		__IN__ UINT64 bitsPerPRB;
		__IN__ UINT64 bitsPerPRBLayer[MAX_LAYER_COUNT];
		__IN__ UINT64 TBSLayer[MAX_LAYER_COUNT];
		__IN__ double initRank;
		__IN__ double rank;
		__IN__ double var2; // Store past throughput performance for proportional fair
		//Network Slicing
		__IN__ double averageThroughput;

		__IN__ bool NDI;
		__IN__ UINT PRBReqdForHARQRetransmission;

		__OUT__ UINT allocatedPRBCount;
		struct stru_LTENR_UESchedulerInfo* next;
	}LTENR_UESCHEDULERINFO, * ptrLTENR_UESCHEDULERINFO;
#pragma endregion

#pragma region SCHEDULER_INFO
	typedef struct stru_LTENR_SchedulerInfo
	{
		__IN__ NETSIM_ID gnbId;
		__IN__ NETSIM_ID gnbIf;

		__IN__ UINT PRBCount;
		__IN__ double OH_UL;
		__IN__ double OH_DL;
		__IN__ BOOL isPRBRankInit;
		__IN__ BOOL isUERRCSetUpCompleted;
		__IN__ LTENR_GnbSchedulingType schedulingType;
		__IN__ double alpha;
		__IN__ __OUT__ LTENR_SLOTTYPE slotType;
		__IN__ UINT TotalPRBReqdForHARQRetransmission;

		__IN__ ptrLTENR_UESCHEDULERINFO uplinkInfo;
		__IN__ ptrLTENR_UESCHEDULERINFO downlinkInfo;
	}LTENR_SCHEDULERINFO, * ptrLTENR_SCHEDULERINFO;
#pragma endregion

#pragma region GNB_MAC
	typedef struct stru_LTENR_GnbMac
	{
		NETSIM_ID gnbId;
		NETSIM_ID gnbIf;

		NETSIM_ID epcId;
		NETSIM_ID epcIf;

		ptrLTENR_SCHEDULERINFO schedulerInfo[MAX_CA_COUNT];
	}LTENR_GNBMAC, * ptrLTENR_GNBMAC;
#pragma endregion

#pragma region BWP_SWITCH
	typedef struct stru_LTENR_BwpSwitch
	{
		UINT prb_count;
		UINT ca_new;
		bool bwp_active;
	}LTENR_BwpSwitch, * ptrLTENR_BwpSwitch;
#pragma endregion

#pragma region VARIABLE_AND_FUN_DEF
	double initTotalPRBAvailable;
	void LTENR_PRB_Scheduler(ptrLTENR_SCHEDULERINFO schedulerInfo);
	ptrLTENR_UESCHEDULERINFO LTENR_MACScheduler_FindInfoForUE(ptrLTENR_SCHEDULERINFO si,
		NETSIM_ID d, NETSIM_ID in,
		bool isUplink);;
	//BWP-PRB
	ptrLTENR_BwpSwitch LTENR_BWP_Switch(ptrLTENR_SCHEDULERINFO schedulerInfo, ptrLTENR_UESCHEDULERINFO curr, UINT PRB_needed);
#pragma endregion

#ifdef  __cplusplus
}
#endif
#endif /* _NETSIM_LTENR_MAC_H_ */