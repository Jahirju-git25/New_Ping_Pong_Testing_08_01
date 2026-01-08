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

#pragma region HEADER_FILES
#include "stdafx.h"
#include "LTENR_EPSBearer.h"
#pragma endregion

#pragma region ISGBR
bool LTENR_EPSBearer_IsGbr(LTENR_EPSBEARER_QCI qci) {
    switch (qci) {
        case GBR_CONV_VOICE:
        case GBR_CONV_VIDEO:
        case GBR_GAMING:
        case GBR_NON_CONV_VIDEO:
            return true;
        case NGBR_IMS:
        case NGBR_VIDEO_TCP_OPERATOR:
        case NGBR_VOICE_VIDEO_GAMING:
        case NGBR_VIDEO_TCP_PREMIUM:
        case NGBR_VIDEO_TCP_DEFAULT:
            return false;
        default:
            fnNetSimError("unknown QCI value\n");
            return false;
    }
}
#pragma endregion

#pragma region GetPriority
NETSIM_ID LTENR_EPSBearer_GetPriority(LTENR_EPSBEARER_QCI qci) {
    switch (qci) {
        case GBR_CONV_VOICE:
            return 2;
        case GBR_CONV_VIDEO:
            return 4;
        case GBR_GAMING:
            return 3;
        case GBR_NON_CONV_VIDEO:
            return 5;
        case NGBR_IMS:
            return 1;
        case NGBR_VIDEO_TCP_OPERATOR:
            return 6;
        case NGBR_VOICE_VIDEO_GAMING:
            return 7;
        case NGBR_VIDEO_TCP_PREMIUM:
            return 8;
        case NGBR_VIDEO_TCP_DEFAULT:
            return 9;
        default:
            fnNetSimError("unknown QCI value\n");
            return 0;
    }
}
#pragma endregion

#pragma region GetPacketDelayBudgetMs
NETSIM_ID LTENR_EPSBearer_GetPacketDelayBudgetMs(LTENR_EPSBEARER_QCI qci) {
    switch (qci) {
        case GBR_CONV_VOICE:
            return 100;
        case GBR_CONV_VIDEO:
            return 150;
        case GBR_GAMING:
            return 50;
        case GBR_NON_CONV_VIDEO:
            return 300;
        case NGBR_IMS:
            return 100;
        case NGBR_VIDEO_TCP_OPERATOR:
            return 300;
        case NGBR_VOICE_VIDEO_GAMING:
            return 100;
        case NGBR_VIDEO_TCP_PREMIUM:
            return 300;
        case NGBR_VIDEO_TCP_DEFAULT:
            return 300;
        default:
            fnNetSimError("unknown QCI value");
            return 0;
    }
}
#pragma endregion

#pragma region GetPacketErrorLossRate
double LTENR_EPSBearer_GetPacketErrorLossRate(LTENR_EPSBEARER_QCI qci) {
    switch (qci) {
        case GBR_CONV_VOICE:
            return 1.0e-2;
        case GBR_CONV_VIDEO:
            return 1.0e-3;
        case GBR_GAMING:
            return 1.0e-3;
        case GBR_NON_CONV_VIDEO:
            return 1.0e-6;
        case NGBR_IMS:
            return 1.0e-6;
        case NGBR_VIDEO_TCP_OPERATOR:
            return 1.0e-6;
        case NGBR_VOICE_VIDEO_GAMING:
            return 1.0e-3;
        case NGBR_VIDEO_TCP_PREMIUM:
            return 1.0e-6;
        case NGBR_VIDEO_TCP_DEFAULT:
            return 1.0e-6;
        default:
            fnNetSimError("unknown QCI value");
            return 0;
    }
}
#pragma endregion

#pragma region SET_GBR_QoS_INFO_AND_ARP
void LTENR_EPSBearer_Set_GBR_QoS_Info(ptrLTENR_EPSBEARER_GBR_QOS_INFO info,
    NETSIM_ID gbrDl, NETSIM_ID gbrUl,
    NETSIM_ID mbrDl, NETSIM_ID mbrUl) {
    info->gbrDl = gbrDl;
    info->gbrUl = gbrUl;
    info->mbrDl = mbrDl;
    info->mbrUl = mbrUl;
}

void LTENR_EPSBearer_Set_ARP(ptrLTENR_AllocationRetentionPriority info,
    NETSIM_ID priorityLevel,
    bool preemptionCapability,
    bool preemptionVulnerability) {
    info->priorityLevel = priorityLevel;
    info->preemptionCapability = preemptionCapability;
    info->preemptionVulnerability = preemptionVulnerability;
}
#pragma endregion

#pragma region EPSBEARER_CREATE
ptrLTENR_EPSBEARER LTENR_EPSBEARER_CREATE(LTENR_EPSBEARER_QCI qci) {
    ptrLTENR_EPSBEARER eps = calloc(1, sizeof * eps);
    eps->qci = qci;
    eps->isGBR = LTENR_EPSBearer_IsGbr(qci);
    eps->priority = LTENR_EPSBearer_GetPriority(qci);
    eps->PER = LTENR_EPSBearer_GetPacketErrorLossRate(qci);
    eps->PDB = LTENR_EPSBearer_GetPacketDelayBudgetMs(qci);
    eps->ARP = calloc(1, sizeof * eps->ARP);
    LTENR_EPSBearer_Set_ARP(eps->ARP, 0, false, false);
    eps->GBR_QoS_Info = calloc(1, sizeof * eps->GBR_QoS_Info);
    LTENR_EPSBearer_Set_GBR_QoS_Info(eps->GBR_QoS_Info, 0, 0, 0, 0);
    return eps;
}

ptrLTENR_EPSBEARER LTENR_EPSBEARER_Create_Default() {
    return LTENR_EPSBEARER_CREATE(NGBR_VIDEO_TCP_DEFAULT);
}
#pragma endregion

#pragma region DELETE_BEARER
void LTENR_EPSBEARER_Delete_Bearer(ptrLTENR_EPSBEARER info) {

    if (info->ARP)
        free(info->ARP);

    if (info->GBR_QoS_Info)
        free(info->GBR_QoS_Info);

    free(info);
}
#pragma endregion

#pragma region COPY_BEARER
ptrLTENR_EPSBEARER LTENR_EPSBEARER_Copy_Bearer(ptrLTENR_EPSBEARER info) {
    ptrLTENR_EPSBEARER bearer = calloc(1, sizeof * bearer);
    bearer->GBR_QoS_Info = calloc(1, sizeof * bearer->GBR_QoS_Info);
    bearer->ARP = calloc(1, sizeof * bearer->ARP);
    bearer->qci = info->qci;
    bearer->priority = info->priority;
    bearer->PER = info->PER;
    bearer->PDB = info->PDB;
    bearer->isGBR = info->isGBR;
   // memcpy(bearer->ARP, info->ARP, sizeof * bearer->ARP);
   // memcpy(bearer->GBR_QoS_Info, info->GBR_QoS_Info, sizeof * bearer->GBR_QoS_Info);
    bearer->ARP->priorityLevel = info->ARP->priorityLevel;
    bearer->ARP->preemptionCapability = info->ARP->preemptionCapability;
    bearer->ARP->preemptionVulnerability = info->ARP->preemptionVulnerability;

    bearer->GBR_QoS_Info->gbrDl = info->GBR_QoS_Info->gbrDl;
    bearer->GBR_QoS_Info->gbrUl = info->GBR_QoS_Info->gbrUl;
    bearer->GBR_QoS_Info->mbrDl = info->GBR_QoS_Info->mbrDl;
    bearer->GBR_QoS_Info->mbrUl = info->GBR_QoS_Info->mbrUl;
    return bearer;
}
#pragma endregion