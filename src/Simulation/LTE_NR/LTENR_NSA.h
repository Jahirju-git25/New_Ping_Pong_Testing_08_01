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
* Author:    Kumar Gaurav		                                                    *
*										                                            *
* ----------------------------------------------------------------------------------*/
#ifndef _NETSIM_LTENR_NSA_H_
#define _NETSIM_LTENR_NSA_H_

#ifdef  __cplusplus
extern "C" {
#endif

#pragma region HEADER_FILES
#include"stdafx.h"
#include "LTE_NR.h"
#include "LTENR_GNBRRC.h"
#pragma endregion

#pragma region SEC_NODE_ADDITION_HEADER
	typedef struct stru_LTENR_DC_HDR {
		LTENR_MSGTYPE msgType;
		void* msg;
	}LTENR_DC_HDR, * ptrLTENR_DC_HDR;

	typedef struct stru_LTENR_SEC_NODE_ADD_HDR {
		NETSIM_ID UEID;
		NETSIM_ID UEIF;
		NETSIM_ID MgNBID;
		NETSIM_ID MgNBIF;
		NETSIM_ID SgNBID;
		NETSIM_ID SgNBIF;
	}LTENR_SEC_NODE_ADD_HDR, * ptrLTENR_SEC_NODE_ADD_HDR;

	void fn_Netsim_DC_Secondary_Cell_Addition(NETSIM_ID UEID, NETSIM_ID UEIF,
		NETSIM_ID MGNBID, NETSIM_ID MGNBIF,
		NETSIM_ID SGNBID, NETSIM_ID SGNBIF);
#pragma endregion

#pragma region LTENR_XN_HDR
	typedef struct stru_LTENR_XN_HDR {
		NETSIM_ID UEID;
		NETSIM_ID UEIF;
		NETSIM_ID gnbID;
		NETSIM_ID gnbIF;
		bool flag;
		void* msg;
		char* gateway;
		char* nexthop;
	}LTENR_XN_HDR,*ptrLTENR_XN_HDR;
#pragma endregion

#pragma region API
	UINT DC_SN_CELL_ADDITION_REQUEST;
	UINT DC_SN_CELL_ADDITION_RESPONSE;
	void fn_NetSim_LTENR_NSA_INIT();
	void fn_Netsim_NSA_Secondary_Cell_Addition(NETSIM_ID UEID, NETSIM_ID UEIF,
		NETSIM_ID MGNBID, NETSIM_ID MGNBIF,
		NETSIM_ID SGNBID, NETSIM_ID SGNBIF);
	void fn_Netsim_NSA_Secondary_Cell_Addition_Request_Recv(ptrLTENR_DC_HDR header);
	void fn_Netsim_NSA_Secondary_Cell_Addition_Response_Recv(ptrLTENR_DC_HDR header);

	void LTENR_NSA_DC_SN_CELL_ADDITION_REQUEST_FREEHDR(ptrLTENR_DC_HDR hdr);
	void LTENR_NSA_DC_SN_CELL_ADDITION_RESPONSE_FREEHDR(ptrLTENR_DC_HDR hdr);

	void LTENR_NSA_XN_INTERFACE_TO_RAN_Handle();
#pragma endregion

#ifdef __cplusplus
}
#endif // __cplusplus
#endif //_NETSIM_LTENR_NSA_H_
