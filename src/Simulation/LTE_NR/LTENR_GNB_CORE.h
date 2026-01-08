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

#ifndef _NETSIM_LTENR_GNB_CORE_H_
#define _NETSIM_LTENR_GNB_CORE_H_

#ifdef  __cplusplus
extern "C" {
#endif

#pragma region HEADER_FILES
#include "stdafx.h"
#include "LTENR_Core.h"
#pragma endregion

#pragma region CORE_HDR
	/**
	* @struct stru_LTENR_CORE_HDR
	* @brief Core header 
	*/
	typedef struct stru_LTENR_CORE_HDR {
		LTENR_MSGTYPE msgType;/**< Msg type*/
		void* msg;/**< Msg header*/
		void* apphdr;/**< Application header for core*/
		NETSIM_ID UEID;/**< UE identifier*/
		NETSIM_ID UEIF;/**< UE interface identifier*/
		NETSIM_ID gNBID;/**< gNB identifier*/
		NETSIM_ID gNBIF;/**< gNB N2 interface identifier*/
		NETSIM_ID gnbUuIF;/**< gNB (R)AN interface identifier*/
		NETSIM_ID AMFID;/**< AMF identifier*/
		NETSIM_ID AMFIF;/**< AMF interface identifier*/
		NETSIM_ID SMFID;/**< SMF identifier*/
		NETSIM_ID SMFIF;/**< SMF N11 interface identifier*/
		NETSIM_ID SMFN4IF;/**< SMF N4 interface identifier*/
		NETSIM_ID UPFID;/**< UPF identifier*/
		NETSIM_ID UPFIF;/**< UPF interface identifier*/
	}LTENR_CORE_HDR,*ptrLTENR_CORE_HDR;
#pragma endregion

#pragma region UE_INTIAL_MESSAGE
	/**
	* @enum enum_LTENR_CORE_NSSI
	* @brief Slice selection information
	*/
	typedef enum enum_LTENR_CORE_NSSI {
		LTENR_CORE_NSSI_ALLOWED,
		LTENR_CORE_NSSI_NOT_ALLOWED,
	}LTENR_CORE_NSSI;
	static const char* strLTENR_CORE_NSSI[] = { "NSSI_ALLOWED","NOT_NSSI_ALLOWED" };

	/**
	* @struct stru_LTENR_GNB_CORE_UE_INITIAL_MEAASAGE
	* @brief UE intital msg header
	*/
	typedef struct stru_LTENR_GNB_CORE_UE_INITIAL_MEAASAGE {
		NETSIM_ID amfN2UEID;/**< AMF identifer*/
		NETSIM_ID gnbN2UEID;/**< gNB identifer*/
		LTENR_RRC_EstablishmentCause establishmentCause;/**< Connection establishment cause*/
		LTENR_CORE_NSSI nssi;/**< slicing of network not allowed till now*/
		//NAS PDU for handover
		//UE_CONTXET_REQUEST used for handover 
	}LTENR_GNB_CORE_UE_INITIAL_MEAASAGE, * ptrLTENR_GNB_CORE_UE_INITIAL_MEAASAGE;
#pragma endregion

#pragma region INITIAL_CONTEXT_SETUP_REQUEST
	/**
	* @struct stru_LTENR_AMF_INITIAL_CONTEXT_RESPONSE
	* @brief initial context response header
	*/
	typedef struct stru_LTENR_AMF_INITIAL_CONTEXT_RESPONSE {
		NETSIM_ID gNBID;/**< gNB identifier*/
		NETSIM_ID AMFID;/**< AMF identifier*/
		NETSIM_ID UEID;/**< UE identifier*/
	}LTENR_AMF_INITIAL_CONTEXT_RESPONSE, * ptrLTENR_AMF_INITIAL_CONTEXT_RESPONSE;
#pragma endregion

#pragma region FUN_DEF
	/**
	* Initial UE msg header identifier
	*/
	NETSIM_ID InitalUEMsgHDRID;

	/**
	* Create session request header identifier
	*/
	NETSIM_ID CreateSessionRequestID;

	/**
	* Create session response header identifier
	*/
	NETSIM_ID CreateSessionResponseID;

	/**
	* Initial context setup request header identifier
	*/
	NETSIM_ID InitialContextSetupRequestID;

	/**
	* Initial context setup response header identifier
	*/
	NETSIM_ID InitialContextSetupResponseID;

	/**
	* Registration Completet header identifier
	*/
	NETSIM_ID RegistrationCompleteID;

	/**
	* PDU session update SM request identifier
	*/
	NETSIM_ID NsmfPDUSessionUpdateSMRequestID;

	/**
	* PDU session update SM response identifier
	*/
	NETSIM_ID NsmfPDUSessionUpdateSMResponseID;

	/**
	* PFCP session request identifier
	*/
	NETSIM_ID PFCPSessionRequestID;

	/**
	* PFCP session response identifier
	*/
	NETSIM_ID PFCPSessionResponseID;

	/**
	* Modify bearer request identifier
	*/
	NETSIM_ID ModifyBearerRequestID;

	/**
	* Modify bearer response identifier
	*/
	NETSIM_ID ModifyBearerResponseID;

	/**
	* Init function for core msg and header indetifier
	*/
	void fn_NetSim_LTENR_CORE_MSG_HDR_INIT();

	/**
	* Send initial UE msg
	* @param hdr Core header
	*/
	void fn_NetSim_LTENR_GNB_CORE_INITIAL_UE_MSG(ptrLTENR_CORE_HDR hdr);

	/**
	* Receiver handler for initial UE msg
	* @param hdr Core header
	*/
	void fn_NetSim_LTENR_AMF_INITIAL_MSG_RECV(ptrLTENR_CORE_HDR hdr);

	/**
	* Send initial context setup response
	* @param hdr Core header
	*/
	void fn_NetSim_LTENR_GNB_CORE_INTITAL_CONTEXT_SETUP_RESPONSE(ptrLTENR_CORE_HDR hdr);

	/**
	* Receiver handler for initial context request
	* @param hdr Core header
	*/
	void fn_NetSIM_LTENR_GNB_CORE_GENERATE_INITIAL_CONTEXT_SETUP_REQUEST_RECV(ptrLTENR_CORE_HDR hdr);

	//HDR

	/**
	* Add core header into packet using header idnetifier
	* @param packet pointer of packet
	* @param hdr Core header
	* @param hdrID header identifer
	* @param type Msg type
	*/
	void fn_NetSIm_LTENR_CORE_ADD_HDR_INTO_PACKET(NetSim_PACKET* packet,
		ptrLTENR_CORE_HDR hdr, UINT hdrID, LTENR_MSGTYPE type);

	/**
	* Retrive core header from packet using header identifer
	* @param packet pointer for packet
	* @param hdrID header identifier
	*/
	ptrLTENR_CORE_HDR LTENR_CORE_HDR_GET(NetSim_PACKET* packet, UINT hdrID);

	/**
	* Remove core header from packet using header identifer
	* @param packet pointer for packet
	* @param hdrID header identifier
	*/
	void LTENR_CORE_HDR_REMOVE(NetSim_PACKET* packet, UINT hdrID);


	/**
	* Write intial UE msg header into packet trace
	* @param hdr Core header
	* @param s Pointer for string which will wrtitten in trace 
	*/
	void LTENR_CORE_UE_INITIAL_MSG_TRACE(ptrLTENR_CORE_HDR hdr, char* s);

	/**
	* Copy initial ue msg header into another header
	* @param hdr Core header 
	*/
	ptrLTENR_CORE_HDR LTENR_CORE_UE_INITIAL_MSG_COPY(ptrLTENR_CORE_HDR hdr);

	/**
	* Free Core header
	* @param hdr Core header
	*/
	void LTENR_CORE_UE_INITIAL_MSG_FREEHDR(ptrLTENR_CORE_HDR hdr);

	/**
	* Write create session request msg header into packet trace
	* @param hdr Core header
	* @param s Pointer for string which will written in trace
	*/
	void LTENR_CORE_CREATE_SESSION_REQUEST_TRACE(ptrLTENR_CORE_HDR hdr, char* s);

	/**
	* Copy create session request msg header into another header
	* @param hdr Core header
	*/
	ptrLTENR_CORE_HDR LTENR_CORE_CREATE_SESSION_REQUEST_COPY(ptrLTENR_CORE_HDR hdr);

	/**
	* Free Core header
	* @param hdr Core header
	*/
	void LTENR_CORE_CREATE_SESSION_REQUEST_FREEHDR(ptrLTENR_CORE_HDR hdr);

	/**
	* Write create session response msg header into packet trace
	* @param hdr Core header
	* @param s Pointer for string which will written in trace
	*/
	void LTENR_CORE_CREATE_SESSION_RESPONSE_TRACE(ptrLTENR_CORE_HDR hdr, char* s);

	/**
	* Copy create session response msg header into another header
	* @param hdr Core header
	*/
	ptrLTENR_CORE_HDR LTENR_CORE_CREATE_SESSION_RESPONSE_COPY(ptrLTENR_CORE_HDR hdr);

	/**
	* Free Core header
	* @param hdr Core header
	*/
	void LTENR_CORE_CREATE_SESSION_RESPONSE_FREEHDR(ptrLTENR_CORE_HDR hdr);

	/**
	* Write PFCP session request msg header into packet trace
	* @param hdr Core header
	* @param s Pointer for string which will written in trace
	*/
	void LTENR_CORE_PFCP_SESSION_REQUEST_TRACE(ptrLTENR_CORE_HDR hdr, char* s);

	/**
	* Copy PFCP session request msg header into another header
	* @param hdr Core header
	*/
	ptrLTENR_CORE_HDR LTENR_PFCP_SESSION_REQUEST_COPY(ptrLTENR_CORE_HDR hdr);

	/**
	* Free Core header
	* @param hdr Core header
	*/
	void LTENR_CORE_PFCP_SESSION_REQUEST_FREEHDR(ptrLTENR_CORE_HDR hdr);

	/**
	* Write PFCP session response msg header into packet trace
	* @param hdr Core header
	* @param s Pointer for string which will written in trace
	*/
	void LTENR_CORE_PFCP_SESSION_RESPONSE_TRACE(ptrLTENR_CORE_HDR hdr, char* s);

	/**
	* Copy PFCP session response msg header into another header
	* @param hdr Core header
	*/
	ptrLTENR_CORE_HDR LTENR_PFCP_SESSION_RESPONSE_COPY(ptrLTENR_CORE_HDR hdr);

	/**
	* Free Core header
	* @param hdr Core header
	*/
	void LTENR_CORE_PFCP_SESSION_RESPONSE_FREEHDR(ptrLTENR_CORE_HDR hdr);

	/**
	* Write initila context setup request msg header into packet trace
	* @param hdr Core header
	* @param s Pointer for string which will written in trace
	*/
	void LTENR_CORE_INITIAL_CONTEXT_SETUP_REQUEST_TRACE(ptrLTENR_CORE_HDR hdr, char* s);

	/**
	* Copy initial context setup request msg header into another header
	* @param hdr Core header
	*/
	ptrLTENR_CORE_HDR LTENR_CORE_INITIAL_CONTEXT_SETUP_REQUEST_COPY(ptrLTENR_CORE_HDR hdr);

	/**
	* Free Core header
	* @param hdr Core header
	*/
	void LTENR_CORE_INITIAL_CONTEXT_SETUP_REQUEST_FREEHDR(ptrLTENR_CORE_HDR hdr);

	/**
	* Write initila context setup request msg header into packet trace
	* @param hdr Core header
	* @param s Pointer for string which will written in trace
	*/
	void LTENR_CORE_INITIAL_CONTEXT_SETUP_RESPONSE_TRACE(ptrLTENR_CORE_HDR hdr, char* s);

	/**
	* Copy initial context setup response msg header into another header
	* @param hdr Core header
	*/
	ptrLTENR_CORE_HDR LTENR_CORE_INITIAL_CONTEXT_SETUP_RESPONSE_COPY(ptrLTENR_CORE_HDR hdr);

	/**
	* Free Core header
	* @param hdr Core header
	*/
	void LTENR_CORE_INITIAL_CONTEXT_SETUP_RESPONSE_FREEHDR(ptrLTENR_CORE_HDR hdr);
    /**
    * Select and Add sec cell
    */
	void fn_NetSim_LTENR_SEC_CELL_SelectionAddition(NETSIM_ID UE_ID, NETSIM_ID UE_IF);
#pragma endregion

#ifdef __cplusplus
}
#endif // __cplusplus
#endif //_NETSIM_LTENR_GNB_CORE_H_