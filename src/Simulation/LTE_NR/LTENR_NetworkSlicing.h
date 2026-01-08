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
#ifndef _NETSIM_LTENR_NETWORKSLICING_H_
#define _NETSIM_LTENR_NETWORKSLICING_H_

#pragma region HEADER_FILES_AND_WARNING_REMOVAL
#include "List.h"
#pragma warning ( disable : 4090 )
#pragma warning ( disable : 4100 )
#pragma warning ( disable : 4189 )
#pragma warning ( disable : 4244 )
#pragma endregion

#ifdef  __cplusplus
extern "C" {
#endif

	//Standard
	// 3GPP 24.501 - 9.11.2.8

	typedef int SLICE_DIFFERENTIATOR;
#define SLICE_DIFFERENTIATOR_NO_SLICE	((int)0xffffffff)
#define MIN_SLICE_DIFFERENTIATOR		((int)0xffffffff)
#define MAX_SLICE_DIFFERENTIATOR		0xfffffffe

	typedef enum
	{
		SLICE_SERVICE_TYPE_NONE = 0x00000000,
		SLICE_SERVICE_TYPE_EMBB = 0x00000001,
		SLICE_SERVICE_TYPE_URLLC = 0x00000002,
		SLICE_SERVICE_TYPE_MIOT = 0x00000003,
		SLICE_SERVICE_TYPE_V2X = 0x00000004,
		SLICE_SERVICE_TYPE_UNKNOWN,
	}SLICE_SERVICE_TYPE;
	static const char* strSLICE_SERVICE_TYPE[] = { "BE","EMBB","URLLC","MIOT","V2X","UNKNOWN" };

	typedef enum
	{
		SLICE_STATUS_UNKNOWN = 0x00000000,
		SLICE_STATUS_CONFIGURED = 0x00000001,
		SLICE_STATUS_ALLOWED = 0x00000002,
		SLICE_STATUS_REJECTED_NOT_AVAILABLE_IN_PLMN = 0x00000003,
		SLICE_STATUS_REJECTED_NOT_AVAILABLE_IN_REGISTERED_AREA = 0x00000004,
	}SLICE_STATUS;

	typedef struct stru_SliceBandwidth
	{
		double maxGuaranteedBitRate_mbps;
		double minGuaranteedBitRate_mbps;
		double aggregateRateGuarantee_mbps;
		double resourceAllocationPercentage;
		double nu;
		double sumAvgThroughput;
	}SLICEBANDWIDTH, * ptrSLICEBANDWIDTH;

	typedef enum
	{
		LTENR_RESOURCE_SHARING_STATIC,
		LTENR_RESOURCE_SHARING_DYNAMIC,
	}LTENR_RESOURCE_SHARING_TECHNIQUE;
	static const char* strLTENR_RESOURCE_SHARING_TECHNIQUE[] = {"STATIC","DYNAMIC" };

	typedef struct stru_NetworkSliceInfo
	{
		NETSIM_ID sliceId;
		SLICE_DIFFERENTIATOR sliceDifferentiator;
		SLICE_SERVICE_TYPE sliceServiceType;
		SLICE_STATUS sliceStatus;

		SLICEBANDWIDTH uplinkBandwidth;
		SLICEBANDWIDTH downlinkBandwidth;
	}NetworkSliceInfo, * ptrNetworkSliceInfo;

	typedef struct stru_LTE_NR_Network_Slicing
	{
		bool isSlicing;
		NETSIM_ID sliceCount;
		NETSIM_ID* ueSliceId;
		char* slicingFileName;		
		double EWMA_Learning_Rate;
		double LagrangeMultiplierLearningRate;
		LTENR_RESOURCE_SHARING_TECHNIQUE rsrcSharingTechnique;
		NetworkSliceInfo Info[9];
	}LTENR_NWSLICING, * ptrLTENR_NWSLICING;
	ptrLTENR_NWSLICING nws;

	void LTENR_ConfigureNETWORK_SLICINGConfig();

#ifdef  __cplusplus
}
#endif
#endif /* _NETSIM_LTENR_NETWORKSLICING_H_ */