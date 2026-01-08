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


#ifndef _NETSIM_LTENR_SMF_UPF_H_
#define _NETSIM_LTENR_SMF_UPF_H_

#ifdef  __cplusplus
extern "C" {
#endif

#pragma region HEADER_FILES
#include "stdafx.h"
#include "LTENR_Core.h"
#pragma endregion

#pragma region SMF_ASSOCIATION
	/**
	* @strut stru_LTENR_SMF_GNB_Info
	* @brief Association between SMF and GNB
	*/
	typedef struct stru_LTENR_SMF_GNB_Info {
		NETSIM_ID gNBID;
		NETSIM_ID gNBIF;
		NETSIM_ID SMFID;
		NETSIM_ID SMFIF;
		NETSIM_IPAddress SMFIP;
		NETSIM_IPAddress gNBIP;//IP address assoicate with SMF
		_ptr_ele ele;
	}LTENR_SMF_GNB_Info,*ptrLTENR_SMF_GNB_Info;
#define LTENR_SMF_GNB_Info_ALLOC()			(list_alloc(sizeof(LTENR_SMF_GNB_Info),offsetof(LTENR_SMF_GNB_Info,ele)))
#define LTENR_SMF_GNB_Info_NEXT(info)			(LIST_NEXT(info))
#define LTENR_SMF_GNB_Info_ADD(info,e)		(LIST_ADD_LAST(&(info),e))
#define LTENR_SMF_GNB_Info_REMOVE(info,e)		(LIST_REMOVE(&(info),e))

	/**
	* @strut stru_LTENR_SMF_UE_Info
	* @brief Association between SMF and UE
	*/
	typedef struct stru_LTENR_SMF_UE_Info {
		NETSIM_ID UEID;
		NETSIM_ID UEIF;
		NETSIM_ID gnbID;
		NETSIM_ID gnbIF;//LTENR Interface id
		NETSIM_IPAddress gnbLTENRIP;//IP address of LTENR interface
		NETSIM_IPAddress ueIP;
		ptrLTENR_Bearer_Context associateBearer;
		_ptr_ele ele;
	}LTENR_SMF_UE_Info,*ptrLTENR_SMF_UE_Info;
#define LTENR_SMF_UE_Info_ALLOC()			(list_alloc(sizeof(LTENR_SMF_UE_Info),offsetof(LTENR_SMF_UE_Info,ele)))
#define LTENR_SMF_UE_Info_NEXT(info)			(LIST_NEXT(info))
#define LTENR_SMF_UE_Info_ADD(info,e)		(LIST_ADD_LAST(&(info),e))
#define LTENR_SMF_UE_Info_REMOVE(info,e)		(LIST_REMOVE(&(info),e))

	/**
	* @strut stru_LTENR_SMF_UPF_Info
	* @brief Association between SMF and UPF
	*/
	typedef struct stru_LTENR_SMF_UPF_Info {
		NETSIM_ID UPFID;
		NETSIM_ID UPFIF;
		_ptr_ele ele;
	}LTENR_SMF_UPF_Info, *ptrLTENR_SMF_UPF_Info;
#define LTENR_SMF_UPF_Info_ALLOC()			(list_alloc(sizeof(LTENR_SMF_UPF_Info),offsetof(LTENR_SMF_UPF_Info,ele)))
#define LTENR_SMF_UPF_Info_NEXT(info)			(LIST_NEXT(info))
#define LTENR_SMF_UPF_Info_ADD(info,e)		(LIST_ADD_LAST(&(info),e))
#define LTENR_SMF_UPF_Info_REMOVE(info,e)		(LIST_REMOVE(&(info),e))

	/**
	* @strut stru_LTENR_SMF_AMF_Info
	* @brief Association between SMF and AMF
	*/
	typedef struct stru_LTENR_SMF_AMF_Info {
		NETSIM_ID AMFID;
		NETSIM_ID AMFIF;
		_ptr_ele ele;
	}LTENR_SMF_AMF_Info, *ptrLTENR_SMF_AMF_Info;
#define LTENR_SMF_AMF_Info_ALLOC()			(list_alloc(sizeof(LTENR_SMF_AMF_Info),offsetof(LTENR_SMF_AMF_Info,ele)))
#define LTENR_SMF_AMF_Info_NEXT(info)			(LIST_NEXT(info))
#define LTENR_SMF_AMF_Info_ADD(info,e)		(LIST_ADD_LAST(&(info),e))
#define LTENR_SMF_AMF_Info_REMOVE(info,e)		(LIST_REMOVE(&(info),e))

	/**
	* @strut stru_LTENR_SMF_VAR
	* @brief SMF association entity
 	*/
	typedef struct stru_LTENR_SMF_VAR {
		NETSIM_ID SMFID;
		NETSIM_ID SMFIF;
		LTENR_INTERFACE_TYPE IFType;
		ptrLTENR_SMF_GNB_Info gnbList;
		ptrLTENR_SMF_UE_Info ueInfo;
		ptrLTENR_SMF_AMF_Info AMFInfo;
		ptrLTENR_SMF_UPF_Info UPFInfo;
		_ptr_ele ele;
	}LTENR_SMF_VAR,*ptrLTENR_SMF_VAR;
#define LTENR_SMF_VAR_ALLOC()			(list_alloc(sizeof(LTENR_SMF_VAR),offsetof(LTENR_SMF_VAR,ele)))
#define LTENR_SMF_VAR_NEXT(info)	(LIST_NEXT(info))
#define LTENR_SMF_VAR_ADD(info,e)		(LIST_ADD_LAST(&(info),e))
#define LTENR_SMF_VAR_REMOVE(info,e)	(LIST_REMOVE(&(info),e))
#pragma endregion

#pragma region UPF_ASSOCIATION
	/**
	* @strut stru_LTENR_UPF_Info
	* @brief UPF association with gNB and UE
	*/
	typedef struct stru_LTENR_UPF_Info {
		NETSIM_ID UEID;
		NETSIM_ID UEIF;
		NETSIM_ID gnbID;
		NETSIM_ID gnbIF;
		NETSIM_ID gnbN3IF;
		_ptr_ele ele;
	}LTENR_UPF_Info, * ptrLTENR_UPF_Info;
#define LTENR_UPF_Info_ALLOC()			(list_alloc(sizeof(LTENR_UPF_Info),offsetof(LTENR_UPF_Info,ele)))
#define LTENR_UPF_Info_NEXT(info)			(LIST_NEXT(info))
#define LTENR_UPF_Info_ADD(info,e)		(LIST_ADD_LAST(&(info),e))
#define LTENR_UPF_Info_REMOVE(info,e)		(LIST_REMOVE(&(info),e))

	/**
	* @strut stru_LTENR_UPF_VAR
	* @brief UPF association entity
	*/
	typedef struct stru_LTENR_UPF_VAR {
		NETSIM_ID UPFID;
		NETSIM_ID UPFIF;
		LTENR_INTERFACE_TYPE IFType;
		ptrLTENR_UPF_Info info;
		_ptr_ele ele;
	}LTENR_UPF_VAR, *ptrLTENR_UPF_VAR;
#define LTENR_UPF_VAR_ALLOC()			(list_alloc(sizeof(LTENR_UPF_VAR),offsetof(LTENR_UPF_VAR,ele)))
#define LTENR_UPF_VAR_NEXT(info)	(LIST_NEXT(info))
#define LTENR_UPF_VAR_ADD(info,e)		(LIST_ADD_LAST(&(info),e))
#define LTENR_UPF_VAR_REMOVE(info,e)	(LIST_REMOVE(&(info),e))
#pragma endregion

#pragma region CREATE_SESSION_RESPONSE_MSG
	/**
	* @strut stru_LTENR_AMF_SMF_CREATE_SESSION_RESPONSE_MSG
	* @brief Create session response msg
	* @param UEID UE-ID
	* @param gNBID gNB-ID
	* @param AMFID AMF-ID
	* @param Bearer Hold info on an EPS bearer to be context
	*/
	typedef struct stru_LTENR_AMF_SMF_CREATE_SESSION_RESPONSE_MSG {
		NETSIM_ID UEID;
		NETSIM_ID gNBID;
		NETSIM_ID AMFID;
		ptrLTENR_Bearer_Activate Bearer;
	}LTENR_AMF_SMF_CREATE_SESSION_RESPONSE_MSG, * ptrLTENR_AMF_SMF_CREATE_SESSION_RESPONSE_MSG;
#pragma endregion

#pragma region SMF_UPF_PFCP_SESSION_REQUEST
	/**
	* @strut stru_LTENR_SMF_UPF_PFCP_SESSION_REQUEST
	* @brief PFCP_SESSION_REQUEST msg
	* @param UEID UE-ID
	* @param gNBID gNB-ID
	* @param UPFID UPF-ID
	* @param SMFID SMF-ID
	* @param Bearer Hold info on an EPS bearer to be context
	*/
	typedef struct stru_LTENR_SMF_UPF_PFCP_SESSION_REQUEST {
		NETSIM_ID UEID;
		NETSIM_ID gNBID;
		NETSIM_ID UPFID;
		NETSIM_ID SMFID;
		ptrLTENR_Bearer_Context Bearer;
	}LTENR_SMF_UPF_PFCP_SESSION_REQUEST, *ptrLTENR_SMF_UPF_PFCP_SESSION_REQUEST;
#pragma endregion

#pragma region SMF_UPF_PFCP_SESSION_RESPONSE
	/**
	* @strut stru_LTENR_SMF_UPF_PFCP_SESSION_RESPONSE
	* @brief PFCP_SESSION_RESPONSE msg
	* @param UEID UE-ID
	* @param gNBID gNB-ID
	* @param UPFID UPF-ID
	* @param SMFID SMF-ID
	* @param Bearer Hold info on an EPS bearer to be context
	*/
	typedef struct stru_LTENR_SMF_UPF_PFCP_SESSION_RESPONSE {
		NETSIM_ID UEID;
		NETSIM_ID gNBID;
		NETSIM_ID UPFID;
		NETSIM_ID SMFID;
		ptrLTENR_Bearer_Context Bearer;
	}LTENR_SMF_UPF_PFCP_SESSION_RESPONSE, * ptrLTENR_SMF_UPF_PFCP_SESSION_RESPONSE;
#pragma endregion

#pragma region FUN_DEF
	/**
	* SMF Traffic ID
	*/
	NETSIM_ID smfTEID;
	
	/**
	* Handler for receive create session request packet
	* @param hdr Core header of packet
	*/
	void fn_NetSim_SMF_UPF_LTENR_CREATE_SESSION_REQUEST_RECV(ptrLTENR_CORE_HDR hdr);

	/**
	* Add new gNB to the SMF
	* @param SMFID SMF-ID
	* @param SMFIF SMF interface ID
	* @param gNBID gNB device ID
	* @param gNBIF gNB interface ID
	*/
	void LTENR_SMF_AddgNB(NETSIM_ID SMFID, NETSIM_ID SMFIF,
		NETSIM_ID gNBID, NETSIM_ID gNBIF);

	/**
	* Add new UE to the SMF
	* @param SMFID SMF-ID
	* @param SMFIF SMF interface ID
	* @param gNBID gNB device ID
	* @param gNBIF gNB interface ID
	* @param UEID UE device ID
	* @param UEIF UE interface ID
	*/
	void LTENR_SMF_AddUE(NETSIM_ID SMFID, NETSIM_ID SMFIF,
		NETSIM_ID gNBID, NETSIM_ID gNBIF,
		NETSIM_ID UEID, NETSIM_ID UEIF);

	/**
	* Delete existing UE from the SMF
	* @param SMFID SMF-ID
	* @param SMFIF SMF interface ID
	* @param UEID UE device ID
	* @param UEIF UE interface ID
	*/
	void LTENR_SMF_DeleteUE(NETSIM_ID SMFID, NETSIM_ID SMFIF,
		NETSIM_ID UEID, NETSIM_ID UEIF);

	/**
	* Find existing UE from the SMF
	* @param SMFID SMF-ID
	* @param SMFIF SMF interface ID
	* @param UEID UE device ID
	* @param UEIF UE interface ID
	*/
	ptrLTENR_SMF_UE_Info LTENR_SMF_FindUE(NETSIM_ID SMFID, NETSIM_ID SMFIF,
		NETSIM_ID UEID, NETSIM_ID UEIF);

	/**
	* Add new AMF from the SMF
	* @param SMFID SMF-ID
	* @param SMFIF SMF interface ID
	* @param AMFID AMF device ID
	* @param AMFIF AMF interface ID
	* @param var SMF var info
	*/
	void LTENR_SMF_AddAMF(NETSIM_ID SMFID, NETSIM_ID SMFIF,
		NETSIM_ID AMFID, NETSIM_ID AMFIF, ptrLTENR_SMF_VAR var);

	/**
	* Add new UPF from the SMF
	* @param SMFID SMF-ID
	* @param SMFIF SMF interface ID
	* @param UPFID UPF device ID
	* @param UPFIF UPF interface ID
	* @param var SMF var
	*/
	void LTENR_SMF_AddUPF(NETSIM_ID SMFID, NETSIM_ID SMFIF,
		NETSIM_ID UPFID, NETSIM_ID UPFIF, ptrLTENR_SMF_VAR var);

	/**
	* Add an EPS bearer to the list of bearers to be activated for this
	* UE. The bearer will be activated when the UE enters the CM
	* connected state.
	* @param info Association information between SMF and UE
	* @param BreaerInfo Hold info on an EPS bearer to be context
	*/
	void LTENR_SMF_AddBearer(ptrLTENR_SMF_UE_Info info, ptrLTENR_Bearer_Context BreaerInfo);

	/**
	* This Function erases all contexts of bearer from SMF side 
    * @param info UE information pointer
    * @param bearerID Bearer Id which need to be removed corresponding to UE
	*/
	void LTENR_SMF_RemoveBearer(ptrLTENR_SMF_UE_Info info, NETSIM_ID bearerID);

	/**
	* Init function for SMF N4 interface
	* @param SMFID SMF device ID
	*/
	void fn_NetSim_LTENR_SMF_N4_Init(NETSIM_ID SMFID);

	/**
	* Init function for SMF N11 interface
	* @param SMFID SMF device ID
	*/
	void fn_NetSim_LTENR_SMF_N11_Init(NETSIM_ID SMFID);

	/**
	* Init function for UPF N3 interface
	* @param UPFID UPF device ID
	*/
	void fn_MetSim_LTENR_UPF_N3_Init(NETSIM_ID UPFID);

	/**
	* Init function for UPF N4 interface
	* @param UPFID UPF device ID
	*/
	void fn_MetSim_LTENR_UPF_N4_Init(NETSIM_ID UPFID);

	/**
	* Create packet for session response
	* @param hdr Core header
	* @param bearer  Hold info on an EPS bearer to be context
	*/
	void LTENR_SMF_CREATE_SESSION_REPONSE(ptrLTENR_CORE_HDR hdr,
		ptrLTENR_Bearer_Context bearer);
#pragma endregion

#ifdef __cplusplus
}
#endif // __cplusplus
#endif //_NETSIM_LTENR_SMF_UPF_H_