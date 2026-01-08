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
#ifndef _NETSIM_LTENR_EPS_BEARER_H_
#define _NETSIM_LTENR_EPS_BEARER_H_

#ifdef  __cplusplus
extern "C" {
#endif

#pragma region HEADER_FILES
#include "stdafx.h"
#pragma endregion

// SPEC 3GPP 23.203 Section 6.1.7.2  Section 6.1.7.3(AllocationRetentionPriority)

#pragma region QCI
    /**
     * @enum enum_LTENR_EPSBEARER_QCI
     * @ brief Mission Critical, V2X and Discrete Automation are Ignored for Both GBR and NGBR.
     *   User can modify and add new QCI and accordingly other peoperties.
     */
    typedef enum enum_LTENR_EPSBEARER_QCI {
        GBR_CONV_VOICE = 1,
        GBR_CONV_VIDEO = 2,
        GBR_GAMING = 3,
        GBR_NON_CONV_VIDEO = 4,
        NGBR_IMS = 5,
        NGBR_VIDEO_TCP_OPERATOR = 6,
        NGBR_VOICE_VIDEO_GAMING = 7,
        NGBR_VIDEO_TCP_PREMIUM = 8,
        NGBR_VIDEO_TCP_DEFAULT = 9,
    } LTENR_EPSBEARER_QCI;
#pragma endregion

#pragma region GBR_QOS_INFO
    /**
    * @struct stru_LTENR_EPSBEARER_GBR_QOS_INFO
    * @brief GBR QoS information
    */
    typedef struct stru_LTENR_EPSBEARER_GBR_QOS_INFO {
        NETSIM_ID gbrDl;  /**< Guaranteed Bit Rate in downlink */
        NETSIM_ID gbrUl;  /**< Guaranteed Bit Rate in uplink */
        NETSIM_ID mbrDl;  /**< Maximum Bit Rate in downlink */
        NETSIM_ID mbrUl;  /**< Maximum Bit Rate in uplink */
    }LTENR_EPSBEARER_GBR_QOS_INFO,*ptrLTENR_EPSBEARER_GBR_QOS_INFO;
#pragma endregion

#pragma region ALLOCATION_RETENTION
    /**
    * @struct stru_LTENR_AllocationRetentionPriority
    * @brief ARP information
    */
    typedef struct stru_LTENR_AllocationRetentionPriority{
        NETSIM_ID priorityLevel;            /**< 1-15; 1 = highest  15 = lowest */
        bool preemptionCapability;          /**< true if bearer can preempt others */
        bool preemptionVulnerability;       /**< true if bearer can be preempted by others */
    }LTENR_AllocationRetentionPriority,*ptrLTENR_AllocationRetentionPriority;
#pragma endregion

#pragma region EPS_BEARER
    /**
    * @struct stru_LTENR_EPSBEARER
    * @brief EPS bearer header
    */
    typedef struct stru_LTENR_EPSBEARER {
        LTENR_EPSBEARER_QCI qci;/**< QCI*/
        bool isGBR; /**< GBR*/
        NETSIM_ID priority; /**< Priority for QCI*/
        NETSIM_ID PDB;/**<Packet Dealy Budget*/
        double PER;/**<Packet Error Rate*/
        ptrLTENR_EPSBEARER_GBR_QOS_INFO GBR_QoS_Info;/**< GBR QoS information*/
        ptrLTENR_AllocationRetentionPriority ARP;/**< ARP information*/
    }LTENR_EPSBEARER,*ptrLTENR_EPSBEARER;
#pragma endregion

#pragma region FUN_DEF
    /**
    * This function will check QCI is GBR or not
    * @param qci QCI
    */
    bool LTENR_EPSBearer_IsGbr(LTENR_EPSBEARER_QCI qci);

    /**
    * This function find priority based on QCI
    * @param qci QCI
    */
    NETSIM_ID LTENR_EPSBearer_GetPriority(LTENR_EPSBEARER_QCI qci);

    /**
    * This function find PDB based on QCI
    * @param qci QCI
    */
    NETSIM_ID LTENR_EPSBearer_GetPacketDelayBudgetMs(LTENR_EPSBEARER_QCI qci);

    /**
    * This function find PER based on QCI
    * @param qci QCI
    */
    double LTENR_EPSBearer_GetPacketErrorLossRate(LTENR_EPSBEARER_QCI qci);

    /**
    * This function set GBR information
    * @param info Pointer for GBR information
    * @param gbrDl guranteed bit rate Downlink
    * @param gbrUl guranteed bit rate UPlink
    * @param mbrDl maximum bit rate Downlink
    * @param mbrUl maximum bit rate UPlink
    */
    void LTENR_EPSBearer_Set_GBR_QoS_Info(ptrLTENR_EPSBEARER_GBR_QOS_INFO info,
        NETSIM_ID gbrDl, NETSIM_ID gbrUl,
        NETSIM_ID mbrDl, NETSIM_ID mbrUl);

    /**
    * This function set ARP
    * @param info Pointer of APR information
    * @param priorityLevel Priority level
    * @param preemptionCapability Preemption Capability
    * @param preemptionVulnerability Preemption Vulnerability
    */
    void LTENR_EPSBearer_Set_ARP(ptrLTENR_AllocationRetentionPriority info,
        NETSIM_ID priorityLevel,
        bool preemptionCapability,
        bool preemptionVulnerability);

    /**
    * Create EPS bearer based on QCI
    * @param qci QCI
    */
    ptrLTENR_EPSBEARER LTENR_EPSBEARER_CREATE(LTENR_EPSBEARER_QCI qci);

    /**
    * Create default EPS Bearer
    */
    ptrLTENR_EPSBEARER LTENR_EPSBEARER_Create_Default();

    /**
    * Delete EPS bearer
    * @prama info Pointer of bearer
    */
    void LTENR_EPSBEARER_Delete_Bearer(ptrLTENR_EPSBEARER info);

    /**
    * Copy EPS bearer
    * @prama info Pointer of bearer
    */
    ptrLTENR_EPSBEARER LTENR_EPSBEARER_Copy_Bearer(ptrLTENR_EPSBEARER info);
#pragma endregion

#ifdef  __cplusplus
}
#endif
#endif /* _NETSIM_LTENR_EPS_BEARER_H_ */
