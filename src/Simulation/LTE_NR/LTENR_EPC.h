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
#ifndef _NETSIM_LTENR_EPC_H_
#define _NETSIM_LTENR_EPC_H_

#pragma region HEADER_FILES
#include "List.h"
#pragma endregion

#ifdef  __cplusplus
extern "C" {
#endif

#pragma region EPC_HLR
	typedef struct stru_LTENR_EPC_HLR
	{
		NETSIM_ID ueId;
		NETSIM_ID ueIf;
		NETSIM_ID gnbId;
		NETSIM_ID gnbIf;
		_ptr_ele ele;
	}LTENR_EPC_HLR, * ptrLTENR_EPC_HLR;
#define LTENR_EPC_HLR_ALLOC()			((ptrLTENR_EPC_HLR)list_alloc(sizeof(LTENR_EPC_HLR),offsetof(LTENR_EPC_HLR,ele)))
#define LTENR_EPC_HLR_ADD(epc,h)		(LIST_ADD_LAST(&(epc)->hlr,(h)))
#define LTENR_EPC_HLR_REMOVE(epc,h)		(LIST_FREE(&(epc)->hlr,(h)))
#define LTENR_EPC_HLR_NEXT(h)			((h)=LIST_NEXT((h)))
#pragma endregion

#pragma region LTENR_EPC_STRUC
	typedef struct stru_LTENR_EPC
	{
		NETSIM_ID d;
		NETSIM_ID in;
		ptrLTENR_EPC_HLR hlr;
	}LTENR_EPC, * ptrLTENR_EPC;
#pragma endregion

#pragma region FUN_DEF
	NETSIM_ID LTENR_EPC_HLR_FindOutInterface(NETSIM_ID d, NETSIM_ID dest);
	ptrLTENR_EPC_HLR LTENR_EPC_FindHLR(NETSIM_ID d, NETSIM_ID in, NETSIM_ID dest);
	NETSIM_ID LTENR_NSA_EPC_HLR_FindOutInterface_FOR_GNB(NETSIM_ID d, NETSIM_ID dest, NETSIM_ID gnbID);
	ptrLTENR_EPC_HLR LTENR_EPC_FindHLR_For_GNB(NETSIM_ID d, NETSIM_ID in, NETSIM_ID dest, NETSIM_ID gnbID);
#pragma endregion

#ifdef  __cplusplus
}
#endif
#endif /* _NETSIM_LTENR_EPC_H_ */