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
#ifndef _NETSIM_LTENR_CORE_H_
#define _NETSIM_LTENR_CORE_H_

#ifdef  __cplusplus
extern "C" {
#endif

#pragma region HEADER_FILES
#include "stdafx.h"
#include "List.h"
#include "LTENR_EPC.h"
#include "LTE_NR.h"
#include "LTENR_NAS.h"
#include "LTENR_EPSBearer.h"
#include "LTENR_GNB_CORE.h"
#include "LTENR_GNBRRC.h"
#include "LTENR_GTPU.h"
#include "LTENR_GTPC.h"
#include "../IP/IP.h"
#pragma endregion

	//Specification -- TS23.501 TS23.502 TS29.274

#pragma region ngCore_registration_type
	/**
	* @struct enum_ngCore_REGISTRATION_TYPE
	* @brief Types of registration
	*/
	typedef enum enum_ngCore_REGISTRATION_TYPE {
		ngCORE_REGISTRATION_TYPE_INITIAL_REGISTRATION,
		ngCORE_REGISTRATION_TYPE_MOBILITY_REGISTRATION_UPDATING,
		ngCORE_REGISTRATION_TYPE_PERIODIC_UPDATING,
		ngCORE_REGISTRATION_TYPE_EMERGENCY_REGISTRATION,
	}ngCORE_REGISTRATION_TYPE;
#pragma endregion

#pragma region USER_LOCATION_INFO
	/**
	* @struct stru_LTENR_USER_LOCATION_INFO
	* @brief User location information
	*/
	typedef struct stru_LTENR_USER_LOCATION_INFO {
		NETSIM_ID gci;
	}USER_LOCATION_INFO,*ptrUSER_LOCATION_INFO;
#pragma endregion

#pragma region FULLY_QUALIFIED_TEID
	/**
	* @struct stru_LTENR_FULLY_QUALIFIED_TEID
	* @brief Fully qualified traffic information
	*/
	typedef struct stru_LTENR_FULLY_QUALIFIED_TEID {
		NETSIM_ID smfteid; /**< SMF traffic enable ID*/
		NETSIM_IPAddress TXaddress; /**< Associated gNB IP address*/
	}LTENR_FULLY_QUALIFIED_TEID,*ptrLTENR_FULLY_QUALIFIED_TEID;
#pragma endregion

#pragma region BEARER_CONTEXT_CREATE
	/**
	* @struct stru_LTENR_Bearer_Context
	* @brief Create a list of EPS bearer which to be context 
	*/
	typedef struct stru_LTENR_Bearer_Context {
		NETSIM_ID epsBearerID; /**< EPS bearer ID*/
		ptrLTENR_EPSBEARER epsBearer; /**< Hold information about EPS bearer*/
		_ptr_ele ele; /**< Pointer for next element into list*/
		//TFT yet to implemet
	}LTENR_Bearer_Context,*ptrLTENR_Bearer_Context;
#define LTENR_Bearer_Context_ALLOC()			(list_alloc(sizeof(LTENR_Bearer_Context),offsetof(LTENR_Bearer_Context,ele)))
#define LTENR_Bearer_Context_NEXT(info)			(LIST_NEXT(info))
#define LTENR_Bearer_Context_ADD(info,e)		(LIST_ADD_LAST(&(info),e))
#define LTENR_Bearer_Context_REMOVE(info,e)		(LIST_REMOVE(&(info),e))
#pragma endregion

#pragma region BEARER_CONTEXT_ACTIVATE
	/**
	* @struct stru_LTENR_Bearer_Activate
	* @brief  Hold information about to be activate bearer into bearer list
	*/
	typedef struct stru_LTENR_Bearer_Activate {
		NETSIM_ID epsBearerID;/**< EPS bearer ID*/
		ptrLTENR_EPSBEARER epsBearer;/**< EPS Bearer information*/
		ptrLTENR_FULLY_QUALIFIED_TEID smfte;/**< Hold information about qualified traffic*/
		_ptr_ele ele;/**< Pointer of next element into list*/
		//TFT yet to implemet
	}LTENR_Bearer_Activate, * ptrLTENR_Bearer_Activate;
#define LTENR_Bearer_Activate_ALLOC()			(list_alloc(sizeof(LTENR_Bearer_Activate),offsetof(LTENR_Bearer_Activate,ele)))
#define LTENR_Bearer_Activate_NEXT(info)			(LIST_NEXT(info))
#define LTENR_Bearer_Activate_ADD(info,e)		(LIST_ADD_LAST(&(info),e))
#define LTENR_Bearer_Activate_REMOVE(info,e)		(LIST_REMOVE(&(info),e))
#pragma endregion

#pragma region BEARER_CONTEXT_SETUP
	/**
	* @struct stru_LTENR_Bearer_Setup
	* @brief List of bearer to be setup
	*/
	typedef struct stru_LTENR_Bearer_Setup {
		NETSIM_ID epsBearerID; /**< EPS Bearer ID*/
		ptrLTENR_EPSBEARER epsBearer; /**< EPS Bearer information*/
		NETSIM_ID smfteid; /**< Traffic enable identifier*/
		NETSIM_IPAddress transportLayerIP; /**< Associate gNB NW layer information(IP address)*/
		_ptr_ele ele;/**< Pointer for next element into list*/
		//TFT yet to implemet
	}LTENR_Bearer_Setup, * ptrLTENR_Bearer_Setup;
#define LTENR_Bearer_Setup_ALLOC()			(list_alloc(sizeof(LTENR_Bearer_Setup),offsetof(LTENR_Bearer_Setup,ele)))
#define LTENR_Bearer_Setup_NEXT(info)			(LIST_NEXT(info))
#define LTENR_Bearer_Setup_ADD(info,e)		(LIST_ADD_LAST(&(info),e))
#define LTENR_Bearer_Setup_REMOVE(info,e)		(LIST_REMOVE(&(info),e))
#pragma endregion

#pragma region AMF_UE_INFO
	/**
	* @struct stru_LTENR_AMF_UE_LIST
	* @brief Hold associate UE information as a form of list for AMF
	*/
	typedef struct stru_LTENR_AMF_UE_LIST {
		NETSIM_ID UEID;/**< UE identifier*/
		NETSIM_ID UEIF;/**< UE interface indetifier*/
		_ptr_ele ele;/**< Pointer of next elemnt into list*/
	}LTENR_AMF_UE_LIST, *ptr_LTENR_AMF_UE_LIST;
#define LTENR_AMF_UE_LIST_ALLOC()			(list_alloc(sizeof(LTENR_AMF_UE_LIST),offsetof(LTENR_AMF_UE_LIST,ele)))
#define LTENR_AMF_UE_LIST_NEXT(info)	(LIST_NEXT(info))
#define LTENR_AMF_UE_LIST_ADD(info,e)		(LIST_ADD_LAST(&(info),e))
#define LTENR_AMF_UE_LIST_REMOVE(info,e)	(LIST_REMOVE(&(info),e)) 

	/**
	* @struct stru_LTENR_AMF_INFO
	* @brief Hold associate gNB information as a form of list for AMF
	*/
	typedef struct stru_LTENR_AMF_INFO {
		NETSIM_ID gnbN2ID;/**< gNB indetifier*/
		NETSIM_ID gnbN2IF;/**< gNB N2 interface identifier*/
		NETSIM_ID gnbLTENRIF;/**< gNB (R)AN interface identifier*/
		ptr_LTENR_AMF_UE_LIST ueList;/**< List of information about associate UEs*/
		_ptr_ele ele;/**< Pointer for next element into list*/
		//flow yet to implement
	}LTENR_AMF_INFO, *ptrLTENR_AMF_INFO;
#define LTENR_AMF_INFO_ALLOC()			(list_alloc(sizeof(LTENR_AMF_INFO),offsetof(LTENR_AMF_INFO,ele)))
#define LTENR_AMF_INFO_NEXT(info)	(LIST_NEXT(info))
#define LTENR_AMF_INFO_ADD(info,e)		(LIST_ADD_LAST(&(info),e))
#define LTENR_AMF_INFO_REMOVE(info,e)	(LIST_REMOVE(&(info),e)) 

	/**
	* @struct stru_LTENR_AMF_SMF_Info
	* @brief Association information between AMF and SMF
	*/
	typedef struct stru_LTENR_AMF_SMF_Info {
		NETSIM_ID SMFID;/**< SMF device identifier*/
		NETSIM_ID SMFIF;/**< SMF interface identifier*/
	}LTENR_AMF_SMF_Info,*ptrLTENR_AMF_SMF_Info;

	/**
	* @struct stru_LTENR_AMF_VAR
	* @brief AMF entity
	*/
	typedef struct stru_LTENR_AMF_VAR {
		NETSIM_ID amfN2ID;/**< AMF device identifier*/
		NETSIM_ID amfN2IF;/**< AMF interface identifier*/
		LTENR_INTERFACE_TYPE interfacetype;/**< AMF interface type*/
		ptrLTENR_AMF_INFO info;/**< AMF association information*/
		ptrLTENR_AMF_SMF_Info smfInfo;/**< AMF SMF association information*/
		_ptr_ele ele;
	}LTENR_AMF_VAR,*ptrLTENR_AMF_VAR;
#define LTENR_AMF_VAR_ALLOC()			(list_alloc(sizeof(LTENR_AMF_VAR),offsetof(LTENR_AMF_VAR,ele)))
#define LTENR_AMF_VAR_NEXT(info)	(LIST_NEXT(info))
#define LTENR_AMF_VAR_ADD(info,e)		(LIST_ADD_LAST(&(info),e))
#define LTENR_AMF_VAR_REMOVE(info,e)	(LIST_REMOVE(&(info),e)) 
#pragma endregion

#pragma region Create_Session_Request_Msg
	/**
	* @struct stru_LTENR_AMF_SMF_CREATE_SESSION_REQUEST_MSG
	* @brief packet information for create session request
	*/
	typedef struct stru_LTENR_AMF_SMF_CREATE_SESSION_REQUEST_MSG {
		NETSIM_ID UEID;/**< UE identifier*/
		NETSIM_ID UEIF;/**< UE interface identifier*/
		NETSIM_ID gNBID;/**< gNB indetifier*/
		NETSIM_ID gNBIF;/**< gNB N2 interface identifier*/
		NETSIM_ID AMFID;/**< AMF device identifier*/
		NETSIM_ID AMFIF;/**< AMF N11 interface identifier*/
		//ptrUSER_LOCATION_INFO uli;
		NETSIM_ID uli; /**< User location identifier*/
		ptrLTENR_Bearer_Context bearer;/**< Hold information about EPS Bearer to be context*/
	}LTENR_AMF_SMF_CREATE_SESSION_REQUEST_MSG,*ptrLTENR_AMF_SMF_CREATE_SESSION_REQUEST_MSG;
#pragma endregion

#pragma region INITIAL_CONTEXT_SETUP_REQUEST
	/**
	* @struct stru_LTENR_AMF_INITIAL_CONTEXT_REQUEST
	* @brief Packet information for initial context request
	*/
	typedef struct stru_LTENR_AMF_INITIAL_CONTEXT_REQUEST {
		NETSIM_ID gNBID;/**< gNB identifier*/
		NETSIM_ID AMFID;/**< AMF identifier*/
		NETSIM_ID UEID;/**< UE identifer*/
		NETSIM_ID pduSessionID;/**< PDU session identifier*/
		ptrLTENR_Bearer_Setup bearer;/**< Hold EPS bearer information to be setup*/
	}LTENR_AMF_INITIAL_CONTEXT_REQUEST, *ptrLTENR_AMF_INITIAL_CONTEXT_REQUEST;
#pragma endregion

#pragma region MODIFY_BEARER_REQUEST
	/**
	* @struct enum_LTENR_AMF_MODIFY_BEARER_CAUSE
	* @brief Modify Bearer cause
	*/
	typedef enum enum_LTENR_AMF_MODIFY_BEARER_CAUSE {
		MODIFY_BEARER_CAUSE_REQUEST_ACCEPT,
		MODIFY_BEARER_CAUSE_REQUEST_ACCEPT_PARTICALLY,
		MODIFY_BEARER_CAUSE_REQUEST_NOT_ACCEPT,
		MODIFY_BEARER_CAUSE_REQUEST_NOT_FOUND_CONTEXT,
	}LTENR_AMF_MODIFY_BEARER_CAUSE;

	/**
	* @struct stru_LTENR_AMF_MODIFY_BEARER_REQUEST
	* @brief Packet information for modify bearer request
	*/
	typedef struct stru_LTENR_AMF_MODIFY_BEARER_REQUEST {
		NETSIM_ID SgNBID;/**< Source gNB indetifier*/
		NETSIM_ID SAMFID;/**< Source AMF indetifier*/
		NETSIM_ID TgNBID;/**< Target gNB indetifier*/
		NETSIM_ID TAMFID;/**< Target AMF indetifier*/
		NETSIM_ID UEID;/**<UE indetifier*/
		LTENR_AMF_MODIFY_BEARER_CAUSE cause;/**< Cause for modification*/
		ptrLTENR_Bearer_Setup bearer;/**< Hold info about bearer to be setup*/
	}LTENR_AMF_MODIFY_BEARER_REQUEST, * ptrLTENR_AMF_MODIFY_BEARER_REQUEST;
#pragma endregion

#pragma region NGAP_HEADER
	/**
	* @enum enum_LTENR_CORE_NGAP_PROCEDURECODE
	* @brief Procedure code for NGAP packets
	*/
	typedef enum enum_LTENR_CORE_NGAP_PROCEDURECODE {
		NGAP_PROCEDURECODE_INITIAL_UE_MSG,
		NGAP_PROCEDURECODE_CREATE_SESSION_REQUEST,
		NGAP_PROCEDURECODE_CREATE_SESSION_RESPONSE,
		NGAP_PROCEDURECODE_INITIAL_CONTEXT_SETUP_REQUEST,
		NGAP_PROCEDURECODE_INITIAL_CONTEXT_SETUP_RESPONSE,
		NGAP_PROCEDURECODE_REGISTRATION_COMPLETE,
		NGAP_PROCEDURECODE_PATH_SWITCH,
		NGAP_PROCEDURECODE_PATH_SWITCH_ACK,
	}LTENR_CORE_NGAP_PROCEDURECODE;

	/**
	* @struct stru_LTENR_CORE_NGAP_HEADER
	* @brief NGAP header for application layer
	*/
	typedef struct stru_LTENR_CORE_NGAP_HEADER {
		LTENR_CORE_NGAP_PROCEDURECODE procedureCode;/**< Procedure code for Header*/
		NETSIM_ID noofIEs;/**< No of information elements*/
		NETSIM_ID lenghtofIEs;/**< length of information elements*/
	}LTENR_CORE_NGAP_HEADER,*ptrLTENR_CORE_NGAP_HEADER;
#pragma endregion

#pragma region AMF_FUNCTION_DEF
	//Def from LTENR_AMF.C

	/**
	* Traffic enable identifier
	*/
	NETSIM_ID smfFTEID;

	/**
	* EPS bearer identifier
	*/
	NETSIM_ID epsBearer;

	/**
	* PDU session identifier
	*/
	NETSIM_ID pduSessionID;

	/**
	* Find associate AMF for UE
	* @param UEID UE identifier
	* @param UEIF UE interface identifier
	*/
	NETSIM_ID LTENR_AMF_GetUeConnectedAmf(NETSIM_ID UEID, NETSIM_ID UEIF);

	/**
	* Find New AMF for UE
	* @param UEID UE identifier
	* @param UEIF UE interface identifier
	*/
	NETSIM_ID LTENR_AMF_GetNewAMF(NETSIM_ID UEID, NETSIM_ID UEIF);

	/**
	* Find New UPF for UE
	* @param UEID UE identifier
	* @param UEIF UE interface identifier
	*/
	NETSIM_ID LTENR_UPF_GetNewUPF(NETSIM_ID UEID, NETSIM_ID UEIF);

	/**
	* Find New SMF for UE
	* @param UEID UE identifier
	* @param UEIF UE interface identifier
	*/
	NETSIM_ID LTENR_SMF_GetNewSMF(NETSIM_ID UEID, NETSIM_ID UEIF);

	/**
	* Association of  gNB and AMF
	* @param AMFID AMF identifier
	* @param AMFIF AMF interface identifier
	* @param gNBID gNB identifier
	* @param gNBIF gNB interface identifier
	*/
	void LTENR_AMF_AddgNB(NETSIM_ID AMFID, NETSIM_ID AMFIF,
		NETSIM_ID gNBID, NETSIM_ID gNBIF);

	/**
	* Association of SMF and AMF
	* @param AMFID AMF identifier
	* @param AMFIF AMF interface identifier
	* @param SMFID SMF identifier
	* @param SMFIF SMF interface identifier
	* @param AMFVar AMF Var information
	*/
	void LTENR_AMF_AddSMF(NETSIM_ID AMFID, NETSIM_ID AMFIF,
		NETSIM_ID SMFID, NETSIM_ID SMFIF, ptrLTENR_AMF_VAR AMFVar);

	/**
	* Find associate gNB for AMF
	* @param AMFID AMF identifier
	* @param AMFIF AMF interface identifier
	* @param gNBID gNB identifier
	* @param gNBIF gNB interface identifier
	*/
	ptrLTENR_AMF_INFO LTENR_AMF_Find_gNB_Info(NETSIM_ID AMFID, NETSIM_ID AMFIF,
		NETSIM_ID gNBID, NETSIM_ID gNBIF);

	/**
	* Add a new UE to the AMF. 
	* @param AMFID AMF identifier
	* @param AMFIF AMF interface identifier
	* @param gNBID gNB identifier
	* @param gNBIF gNB interface identifier
	* @param UEID UE identifier
	* @param UEIF UE interface identifier
	*/
	void LTENR_AMF_AddUE(NETSIM_ID AMFID, NETSIM_ID AMFIF,
		NETSIM_ID gNBID, NETSIM_ID gNBIF,
		NETSIM_ID UEID, NETSIM_ID UEIF);

	/**
	* Delete a existing UE to the AMF.
	* @param AMFID AMF identifier
	* @param AMFIF AMF interface identifier
	* @param gNBID gNB identifier
	* @param gNBIF gNB interface identifier
	* @param UEID UE identifier
	* @param UEIF UE interface identifier
	*/
	void LTENR_AMF_DeleteUE(NETSIM_ID AMFID, NETSIM_ID AMFIF,
		NETSIM_ID gNBID, NETSIM_ID gNBIF,
		NETSIM_ID UEID, NETSIM_ID UEIF);

	/**
	* Send current packet to MAC layer directly
	* @param d Device identifier
	* @param outIn Interface for which packet have to send 
	*/
	void LTENR_CORE_NETWORKOUT(NETSIM_ID d, NETSIM_ID outIn);

	/**
	* Init N2 interface for AMF
	* @param AMFID AMF identifier
	*/
	void LTENR_AMF_N2_INTERFACE_INIT(NETSIM_ID AMFID);

	/**
	* Init N11 interface for AMF
	* @param AMFID AMF identifier
	*/
	void LTENR_AMF_N11_INTERFACE_INIT(NETSIM_ID AMFID);

	/**
	* packet creation for create session request
	* @param hdr Core header
	* @param bearer EPS Bearer information
	*/
	void LTENR_AMF_CREATE_SESSION_REQUEST(ptrLTENR_CORE_HDR hdr, ptrLTENR_EPSBEARER bearer);

	/**
	* Receive handler for INITIAL msg
	* @param hdr Core header
	*/
	void fn_NetSim_LTENR_AMF_INITIAL_MSG_RECV(ptrLTENR_CORE_HDR hdr);

	/**
	* Receive handler for create session response
	* @param hdr Core header
	*/
	void fn_NetSim_LTENR_AMF_CREATE_SESSION_RESPONSE_RECV(ptrLTENR_CORE_HDR hdr);

	/**
	* send initial context setup request
	* @param hdr Core header
	* @param bearer EPS Bearer information
	*/
	void fn_NetSim_LTENR_AMF_INITIAL_CONTEXT_SETUP_REQUEST(ptrLTENR_CORE_HDR hdr, ptrLTENR_Bearer_Setup bearer);

	/**
	* Receive handler for initial context setup response
	* @param hdr Core header
	*/
	void fn_NetSim_LTENR_AMF_INITIAL_CONTEXT_SETUP_RESPONSE_RECV(ptrLTENR_CORE_HDR hdr);


	void fn_NetSim_LTENR_GNB_CORE_PATH_SWITCH_MSG(ptrLTENR_HANDOVER_Hdr hdr);
#pragma endregion

#ifdef __cplusplus
}
#endif // __cplusplus
#endif //_NETSIM_LTENR_CORE_H_
