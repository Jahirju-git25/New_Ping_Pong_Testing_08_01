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
#ifndef _NETSIM_LTENR_GTPU_H_
#define _NETSIM_LTENR_GTPU_H_

#ifdef  __cplusplus
extern "C" {
#endif
	// //Specification -- TS29.281

#pragma region HEADER_FILES
#include "stdafx.h"
#pragma endregion

#pragma region MACROS
/**
* A predefine constant GTPU header size : 12 bytes
*/
#define GTPU_HEADER_SIZE 12
/**
* A predefine constant GTPU UDP port : 2152, according to specification
*/
#define GTPU_UDP_PORT 2152
#pragma endregion

#pragma region GTPU_HEADER
	/**
	* @strut stru_LTENR_GTPU_HEADER
	* @brief Create GTP-U header 
	*/
	typedef struct stru_LTENR_GTPU_HEADER {
		/**
		* This field is used to determine the version of the GTPU-U protocol. The version number shall be set to 1.
		*/
		NETSIM_ID V; 

		/**
		* This bit is used as a protocol discriminator between GTP (when PT is '1') and GTP' (when
		* PT is '0').
		*/
		bool PT;

		/**
		* This flag indicates the presence of a meaningful value of the Next Extension Header
		* field. When it is set to '0', the Next Extension Header field either is not present
		* or, if present, shall not be interpreted. When it is set to '1', the Next Extension
		* Header field is present, and shall be interpreted
		*/
		bool E;

		/**
		* This flag indicates the presence of a meaningful value of the Sequence Number field.
		* When it is set to '0', the Sequence Number field either is not present or, if present,
		* shall not be interpreted. When it is set to '1', the Sequence Number field is present,
		* and shall be interpreted
		*/
		bool S;

		/**
		* This flag indicates the presence of a meaningful value of the N-PDU Number field.
		* When it is set to '0', the N-PDU Number field either is not present, or, if present,
		* shall not be interpreted. When it is set to '1', the N-PDU Number field is present,
		* and shall be interpreted
		*/
		bool PN;

		/**
		*  This field indicates the type of GTP-U message
		*/
		NETSIM_ID messageType;

		/**
		*  This field indicates the length in octets of the payload, i.e. the rest of the packet
		*  following the mandatory part of the GTP header (that is the first 8 octets).
		*  The Sequence Number, the N-PDU Number or any Extension headers shall be considered
		*  to be part of the payload, i.e. included in the length count
		*/
		NETSIM_ID length;

		/**
		* This field unambiguously identifies a tunnel endpoint in the receiving GTP-U
		* protocol entity. The receiving end side of a GTP tunnel locally assigns the
		* TEID value the transmitting side has to use.
		*/
		NETSIM_ID TEID;

		/**
		* OPTIONAL FIELD
		* If Sequence Number field is used for G-PDUs (T-PDUs+headers), an increasing
		* sequence number for T-PDUs is transmitted via GTP-U tunnels, when transmission
		* order must be preserved
		*/
		NETSIM_ID SequenceNumber;

		/**
		* OPTIONAL FIELD
		* This field is used at the Inter SGSN Routeing Area Update procedure and some
		* inter-system handover procedures (e.g. between 2G and 3G radio access networks).
		* This field is used to co-ordinate the data transmission for acknowledged mode
		* of communication between the MS and the SGSN
		*/
		NETSIM_ID PduNumber;

		/**
		* OPTIONAL FIELD
		* This field defines the type of Extension Header that follows this field in
		* the GTP-PDU
		*/
		NETSIM_ID NextExtensionType;

		struct stru_NetSim_Packet_AppLayer* pstruAppData;
	}LTENR_GTPU_HEADER, *ptrLTENR_GTPU_HEADER;
#pragma endregion

#pragma region FUN_DEF
	/**
	* Function to create default GPTU header
	*/
	ptrLTENR_GTPU_HEADER fn_NetSim_LTENR_GTPU_DEFAULT_HEADER();

	/**
	* Function to free GPTU header
	* @param hdr Header pointer which have to free from memory
	*/
	void fn_NetSim_LTENR_GTPU_HEADER_FREE(ptrLTENR_GTPU_HEADER hdr);

	/**
	* Function to add GPTU header into packet
	* @param packet Packet pointer
	* @prama TEID Tunnel endpoint identifier
	*/
	void fn_NetSim_LTENR_ADD_GTPU_HEADER(NetSim_PACKET* packet, NETSIM_ID TEID);

	/**
	* Function to remove GPTU header from packet
	* @param packet Packet pointer
	*/
	void fn_NetSim_LTENR_REMOVE_GTPU_HEADER(NetSim_PACKET* packet);
#pragma endregion

#ifdef  __cplusplus
}
#endif
#endif /* _NETSIM_LTENR_GTPU_H_ */