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
* Author:   Kumar Gaurav		                                                    *
*										                                            *
* ----------------------------------------------------------------------------------*/
#ifndef _NETSIM_LTENR_SN_HANDOVER_H_
#define _NETSIM_LTENR_SN_HANDOVER_H_

#ifdef  __cplusplus
extern "C" {
#endif

#pragma region HEADER_FILES
#include "stdafx.h"
#include "LTE_NR.h"
#include "LTENR_GNBRRC.h"
#include "LTENR_NAS.h"
#include "LTENR_PHY.h"
#include "LTENR_EPC.h"
#include "LTENR_MAC.h"
#include "../IP/IP.h"
#pragma endregion

	// SPEC 3GPP TS37.340

#pragma region STRU_HEADER_FOR_SN_HO
	typedef struct stru_LTENR_SN_HANDOVER_HDR {
		LTENR_MSGTYPE msgType;
		void* msg;
		void* apphdr;
		NETSIM_ID ueID;
		NETSIM_ID ueIF;
		NETSIM_ID MNID;
		NETSIM_ID MNIF;
		NETSIM_ID SNID;
		NETSIM_ID SNIF;
		NETSIM_ID TSNID;
		NETSIM_ID TSNIF;
		NETSIM_ID AMFID;
		NETSIM_ID AMFIF;
		NETSIM_ID targetAMFID;
		NETSIM_ID targetAMFIF;
		NETSIM_ID associateSMFID;
		double SNHandoverStartTime;
	}LTENR_SN_HANDOVER_HDR, *ptrLTENR_SN_HANDOVER_HDR;
#pragma endregion

#pragma region DC_HO_SN_ADDITION_REQUEST
	typedef struct stru_LTENR_HO_SN_ADDITION_REQUEST {
		NETSIM_ID ueID;
		NETSIM_ID SNID;
		NETSIM_ID MNID;
		LTENR_RRC_EstablishmentCause cause;
	}LTENR_HO_SN_ADDITION_REQUEST, *ptrLTENR_HO_SN_ADDITION_REQUEST;
#pragma endregion

#pragma region FUN_DEF
	UINT DC_SN_HO_ADDITION_REQUEST;
	UINT DC_SN_HO_ADDITION_REQUEST_ACK;
	UINT DC_SN_HO_RELEASE_REQUEST;
	UINT DC_SN_HO_RELEASE_REQUEST_ACK;
	UINT DC_SN_HO_PATH_SWITCH;
	UINT DC_SN_HO_PATH_SWITCH_ACK;
	UINT DC_SN_HO_UE_CONTEXT_RELEASE;
	UINT DC_SN_HO_UE_CONTEXT_RELEASE_ACK;
	void fn_NetSim_DC_HO_SN_PROCESSIONG(NETSIM_ID ueID, NETSIM_ID ueIF, ptrLTENR_RRC_UE_MEASUREMENT_REPORT report);
#pragma endregion

#ifdef  __cplusplus
}
#endif
#endif /* _NETSIM_LTENR_SN_HANDOVER_H_ */