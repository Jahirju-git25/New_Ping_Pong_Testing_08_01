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
#ifndef _NETSIM_LTENR_SPECTRUM_H_
#define _NETSIM_LTENR_SPECTRUM_H_
#ifdef  __cplusplus
extern "C" {
#endif

#pragma region SLOT_TYPE
	typedef enum enum_SLOTTYPE
	{
		SLOT_UPLINK,
		SLOT_DOWNLINK,
		SLOT_MIXED,
	}LTENR_SLOTTYPE;
	static char strLTENR_SLOTTYPE[][50] = { "UPLink","Downlink","Mixed" };
#pragma endregion

#pragma region UE_LIST_CA
	typedef struct stru_LTENR_CA_UE_LIST
	{
		NETSIM_ID ue_id;
		NETSIM_ID ue_in;
		struct stru_LTENR_CA_UE_LIST* next;
	}LTENR_CA_UE_LIST, * ptrLTENR_CA_UE_LIST;
#pragma endregion

#pragma region CA
	typedef struct stru_LTENR_CA {

		char* frequencyRange;
		char* operatingBand;

		UINT Flow_MHZ;
		UINT Fhigh_MHz;
		long double original_channelBandwidth_mHz;
		long double channelBandwidth_mHz;
		double guardBand_kHz;
		UINT PRBCount;
		UINT PRBoffset;

		UINT8 mu; //Subcarrier spacing configuration
		UINT8 subCarrierSpacing_kHz;
		UINT16 prbBandwidth_kHz;
		char* cyclicPrefix;
		UINT8 symbolPerSlot;
		UINT8 slotPerFrame;
		UINT8 slotPerSubframe;
		double slotDuration_us;
		double symbolDuration_us;

		LTENR_SLOTTYPE configSlotType;
		double dlSlotRatio;
		UINT64 dlSlotCount;
		UINT64 totalSlotCount;

		//BWP
		bool isBWPEnabled;
		UINT bwp_count;
		UINT bwp_id;

        //Config parameter - PHY_OVERHEAD
		double OverheadDL;
		double OverheadUL;

		ptrLTENR_CA_UE_LIST ue_list;
	}LTENR_CA, * ptrLTENR_CA;
#pragma endregion

#pragma region SPECTRUM_CONFIG
	typedef struct stru_LTENR_SpectrumConfig
	{
		char* duplexMode;

		UINT frameDuration;
		UINT subFrameDuration;
		UINT subFramePerFrame;
		UINT8 numSubcarrierPerPRB;

		ptrLTENR_CA CA[MAX_CA_COUNT];
		ptrLTENR_CA CA_original[MAX_CA_COUNT];

		int Current_CA_ID;

		int Series3GPP;
	}LTENR_SPECTRUMCONFIG, * ptrLTENR_SPECTRUMCONFIG;
#pragma endregion

#pragma region PRB
	typedef struct stru_LTENR_PRB
	{
		UINT prbId;
		double lFrequency_MHz;
		double uFrequency_MHz;
		double centralFrequency_MHz;
		double prbBandwidth_MHz;

	}LTENR_PRB, * ptrLTENR_PRB;
#pragma endregion

#pragma region MACROS
#define LTENR_MAX_CQI		15
#define LTENR_MAX_MCS		32
#define NUM_OF_RE_IN_DMRS	0
#define LTENR_DEFAULT_SPECTRAL_EFFICIENCY	0.16
#pragma endregion

#pragma region CQI_TABLE
	typedef struct stru_LTENR_CQITable
	{
		UINT CQIIndex;
		PHY_MODULATION modulation;
		UINT codeRate; //R
		double efficiency;
	}LTENR_CQITable, * ptrLTENR_CQITable;
#define CQIINDEX_ISVALID(index) (((index)>0) && ((index)<=LTENR_MAX_CQI))
#define CQITABLE_EFFICIENCY(table,index) (CQIINDEX_ISVALID(index)?(((table)[(index)]).efficiency):0)
#pragma endregion

#pragma region MCS_INDEX
	typedef struct stru_LTENR_MCSIndexTable
	{
		int mcsIndex;
		int modulationOrder; //Qm
		PHY_MODULATION modulation;
		double codeRate; //R*1024
		double spectralEfficiency;
	}LTENR_MCSINDEXTABLE, * ptrLTENR_MCSINDEXTABLE;
#pragma endregion

#pragma region TBS_SIZE
	typedef struct stru_TBSTable
	{
		UINT index;
		UINT TBS;
	}LTENR_TBSTABLE, * ptrLTENR_TBSTABLE;
#pragma endregion

#pragma region AMC_INFO
	typedef struct stru_AMCInfo
	{
		double SpectralEfficiency;
		
		LTENR_CQITable cqiTable;

		LTENR_MCSINDEXTABLE mcsTable;
		UINT tbsSize;
		UINT BG;	// LDPC base Graph
		UINT CBS;   //Code Block Size
		UINT CBS_;   //value of K_
		UINT nCodeBlock;      //C, Number of Code Blocks
	}LTENR_AMCINFO, * ptrLTENR_AMCINFO;
#pragma endregion
	
#pragma region FUN_DEF
	//Function prototype
	LTENR_CQITable LTENR_GetCQITableFromSpectralEfficiency(ptrLTENR_CQITable table, double efficiency);
	LTENR_MCSINDEXTABLE LTENR_GetMCSIndexTableFromSpectralEfficiency(ptrLTENR_MCSINDEXTABLE table, double efficiency);
	UINT64 LTENR_calculateTBSSize(void* gnbPhy,
								  UINT allocatedPRB,
								  LTENR_MCSINDEXTABLE mcsTable, int CA_ID);
#pragma endregion

#ifdef  __cplusplus
}
#endif
#endif /* _NETSIM_LTENR_SPECTRUM_H_ */
