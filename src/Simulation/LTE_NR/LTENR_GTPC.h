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
#ifndef _NETSIM_LTENR_GTPC_H_
#define _NETSIM_LTENR_GTPC_H_

#ifdef  __cplusplus
extern "C" {
#endif
    // //Specification -- TS29.274

#pragma region HEADER_FILES
#include "stdafx.h"
#pragma endregion

#pragma region MACROS
/**
* A predefine constant GTPC header size : 12 bytes
*/
#define GTPC_HEADER_SIZE 12
/**
* A predefine constant GTPC UDP port : 2152, according to specification
*/
#define GTPC_UDP_PORT 2152
#pragma endregion

#pragma region GTPC_HEADER

    /**
    * @strut stru_LTENR_GTPC_HEADER
    * @brief Create GTP-C header
    */
    typedef struct stru_LTENR_GTPC_HEADER {
        /**
        * This field is used to determine the version of the GTP-C protocol. The version number shall be set to 2(010).
        */
        NETSIM_ID V;

        /**
        * This bit is used to represents a "P" flag. If the "P" flag is set to "0",
        * no piggybacked message shall be present. If the "P" flag is set to "1",
        * then another GTPv2-C message with its own header and body shall be present
        * at the end of the current message.
        */
        bool P;

        /**
        * This flag represents a "T" flag, which indicates if TEID field is present in the GTP-C header or not.
        * If the "T" flag is set to 0, then the TEID field shall not be present in the GTP-C header.
        * If the "T" flag is set to 1, then the TEID field shall immediately follow the Length field, in octets 5 to 8.
        */
        bool T;

        /**
        * This flag represents a "MP" flag. If the "MP" flag is set to "1",
        * then bits 8 to 5 of octet 12 shall indicate the message priority.
        */
        bool MP;

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
        * This field defines the type of Extension Header that follows this field in
        * the GTP-PDU
        */
        NETSIM_ID NextExtensionType;
    }LTENR_GTPC_HEADER, * ptrLTENR_GTPC_HEADER;
#pragma endregion

#pragma region FUN_DEF
    /**
    * Function to create default GPTC header
    */
    ptrLTENR_GTPC_HEADER fn_NetSim_LTENR_GTPC_DEFAULT_HEADER();

    /**
    * Function to free GPTC header
    * @param hdr Header pointer which have to free from memory
    */
    void fn_NetSim_LTENR_GTPC_HEADER_FREE(ptrLTENR_GTPC_HEADER hdr);

    /**
    * Function to add GPTC header into packet
    * @param packet Packet pointer
    * @prama TEID Tunnel endpoint identifier
    */
    void fn_NetSim_LTENR_ADD_GTPC_HEADER(NetSim_PACKET* packet, NETSIM_ID TEID);

    /**
    * Function to remove GPTC header from packet
    * @param packet Packet pointer
    */
    void fn_NetSim_LTENR_REMOVE_GTPC_HEADER(NetSim_PACKET* packet);
#pragma endregion

#ifdef  __cplusplus
}
#endif
#endif /* _NETSIM_LTENR_GTPC_H_ */