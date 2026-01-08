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
#ifndef _NETSIM_LTENR_PHY_H_
#define _NETSIM_LTENR_PHY_H_
#ifdef  __cplusplus
extern "C" {
#endif

#pragma region HEADER_FILES
#include "LTENR_Spectrum.h"
#include "LTENR_PropagationModel.h"
#include "LTENR_AntennaModel.h"
#include "LTENR_Multiplexing.h"
#include "LTENR_HARQ.h"
#pragma endregion

	//struct gNB_Powers {
	//	double gNBlist[num_gNBs];
	//};
	struct gNB_Powers {
		double* gNBlist;
	};

	//struct SINR_Values_Reward {
	//	double SINRlist[num_UEs];
	//	double reward;
	//};
	struct SINR_Values_Reward {
		double* SINRlist;
		double* throughputs_NETSIM;
		double reward;
	};

	//struct SINR_Values_Reward valuesToSend_Python= { {0.0,0.0,0.0,0.0,0.0,0.0}, 0.0 };
//struct gNB_Powers receivedValue_From_Python = { {0.0,0.0,0.0} };

	struct gNB_Powers* receivedValue_From_Python;
	struct SINR_Values_Reward* valuesToSend_Python;


#pragma region PROPAGATION_INFO
	typedef struct stru_LTENR_propagationInfo
	{
		__IN__ NETSIM_ID gnbId;
		__IN__ NETSIM_ID gnbIf;
		__IN__ NETSIM_ID ueId;
		__IN__ NETSIM_ID ueIf;
		__IN__ NetSim_COORDINATES gnbPos;
		__IN__ NetSim_COORDINATES uePos;
		__IN__ double centralFrequency_MHz;
		__IN__ double bandwidth_MHz;
		__IN__ ptrLTENR_PROPAGATIONCONFIG propagationConfig;

		__OUT__ double dTotalLoss;
		__OUT__ double dPathLoss;
		__OUT__ double dShadowFadingLoss;
		__OUT__ double dO2ILoss;
		__OUT__ double dAdditionalLoss;

		//Will be calculated based on input and output
		struct stru_powers
		{
			__IN__ UINT layerCount;
			__IN__ UINT txAntennaCount;
			__IN__ UINT rxAntennaCount;
			__IN__ double txPower_dbm; //layerwise Tx power 
			__IN__ double totaltxpower_dbm;

			double thermalNoise;
			
			double* rxPower_dbm;
			double* EB_by_N0;
			double* SNR_db;
			double* SINR_db;
			double* InterferencePower_dbm;
			double* spectralEfficiency;
			//Sector Antenna
			__OUT__ double antennaGain;
			//Analog Beamforming on DL Broadcast Channel (SSB/PBCH)
			//Total TX power will be used and no layerwise split 			
			double SSB_rxPower_dBm;
			double SSB_EB_by_N0;
			__OUT__ double SSB_SNR_dB;			
			__OUT__ double SSB_AnalogBeamformingGain_dB;
			//Digital Beamforming on Data Channel (PDSCH/PUSCH)
			__OUT__ double* beamFormingGain;
			__OUT__ double  ArrayGain;
			void* beamformingHandle;
			UINT* beamFormingIndex;
		}uplink, downlink;

		//Local to Propagation model
		struct {
			bool isConstructiveShadow;
			double Gset;
			double Iset;
		}SHADOWVAR;

		LTENR_LOCATION uePosition;
		LTENR_LOCATION gnbPosition;

		double frequency_gHz;

		LTENR_SCENARIO currentScenario;
		double dist2D;
		double dist3D;
		double dist2Dindoor;
		double dist2Doutdoor;

	}LTENR_PROPAGATIONINFO, * ptrLTENR_PROPAGATIONINFO;
#pragma endregion

#pragma region UE_PHY
	typedef struct stru_LTENR_UEPHY
	{
		NETSIM_ID ueId;
		NETSIM_ID ueIf;
		NETSIM_ID gnBId;
		NETSIM_ID gnbIf;

		ptrLTENR_PROPAGATIONCONFIG propagationConfig;
		ptrLTENR_ANTENNA antenna;
		bool isBWPEnabled;
		__IN__ UINT default_bwp_id;
		__IN__ UINT first_active_bwp_id;
	}LTENR_UEPHY, * ptrLTENR_UEPHY;
	ptrLTENR_UEPHY LTENR_UEPHY_NEW(NETSIM_ID ueId, NETSIM_ID ueIf);
#pragma endregion

#pragma region ASSOC_UE_PHY_INFO
	struct stru_LTENR_AssociatedUEPhyInfo
	{
		NETSIM_ID ueId;
		NETSIM_ID ueIf;
		bool isAssociated;
		//bool isActive;

		ptrLTENR_AMCINFO* uplinkAMCInfo[MAX_CA_COUNT];
		ptrLTENR_AMCINFO* downlinkAMCInfo[MAX_CA_COUNT];

		ptrLTENR_PROPAGATIONINFO propagationInfo[MAX_CA_COUNT];

		ptrLTENR_HARQ_VAR harq;
		
		void* OLLAContainer;

		_ptr_ele ele;
	};
#define LTENR_ASSOCIATEDUEPHYINFO_ALLOC()			(list_alloc(sizeof(LTENR_ASSOCIATEDUEPHYINFO),offsetof(LTENR_ASSOCIATEDUEPHYINFO,ele)))
#define LTENR_ASSOCIATEDUEPHYINFO_ADD(phy,info)		(LIST_ADD_LAST(&(phy)->associatedUEPhyInfo,(info)))
#define LTENR_ASSOCIATEDUEPHYINFO_NEXT(info)		((info) = LIST_NEXT((info)))
#define LTENR_ASSOCIATEDUEPHYINFO_REMOVE(phy,info)  (LIST_FREE(&(phy)->associatedUEPhyInfo,(info)))
#pragma endregion

#pragma region FRAME_INFO
	typedef struct stru_LTENR_FrameInfo
	{
		UINT frameId;
		double frameStartTime;
		double frameEndTime;

		UINT subFrameId;
		double subFrameStartTime;
		double subFrameEndTime;

		UINT slotId;
		double slotStartTime;
		double slotEndTime;

		LTENR_SLOTTYPE slotType;
		LTENR_SLOTTYPE prevSlotType;

		UINT Current_CA_ID;
	}LTENR_FRAMEINFO, * ptrLTENR_FRAMEINFO;
#pragma endregion

#pragma region GNB_PHY
	struct stru_LTENR_GNBPHY
	{
		NETSIM_ID gnbId;
		NETSIM_ID gnbIf;

		ptrLTENR_SPECTRUMCONFIG spectrumConfig;

		//config parameter -- PDSCHConfig
		struct stru_pdschConfig
		{
			void* mcsTable;
			UINT xOverhead;
		}PDSCHConfig;

		//config parameter -- PUSCHConfig
		struct stru_puschConfig
		{
			void* mcsTable;
			bool isTransformPrecoding;
		}PUSCHConfig;

		//config parameter -- CSI-Reporting
		struct stru_CSIReportConfig
		{
			UINT tableId;
			void* cqiTable;
		}CSIReportConfig;

		bool isBLEREnable;

		ptrLTENR_PROPAGATIONCONFIG propagationConfig;

		ptrLTENR_PRB prbList;

		ptrLTENR_FRAMEINFO frameInfo[MAX_CA_COUNT];

		ptrLTENR_FRAMEINFO currentFrameInfo;

		UINT associatedUECount;
		ptrLTENR_ASSOCIATEDUEPHYINFO associatedUEPhyInfo;

		bool isBWPEnabled;

		UINT old_ca_count;
		UINT ca_count;
		UINT bwp_count;
		char* ca_type;
		char* ca_configuration;

		ptrLTENR_ANTENNA antenna;
	};
#define LTENR_GNBPHY_GETCURRENTCAID(d,in) ((ptrLTENR_GNBPHY)LTENR_GNBPHY_GET(d, in))->currentFrameInfo->Current_CA_ID
#pragma endregion

#pragma region FUN_DEF
	//LTENR-Interference
	void LTENR_Calculate_Interference(ptrLTENR_PROPAGATIONINFO info, double rxPower_dbm, double* Interference,int layerId, int CA_ID, bool isDownlink);
	ptrLTENR_ASSOCIATEDUEPHYINFO LTENR_ASSOCIATEDUEPHYINFO_GET(NETSIM_ID gnbId, NETSIM_ID gnbIf,NETSIM_ID ueId, NETSIM_ID ueIf);
	ptrLTENR_GNBPHY LTENR_GNBPHY_NEW(NETSIM_ID gnbId, NETSIM_ID gnbIf);
	
	//LTENR-AMC
	void LTENR_SetCQITable(ptrLTENR_GNBPHY phy, char* table);
	void LTENR_SetPDSCHMCSIndexTable(ptrLTENR_GNBPHY phy, char* table);
	void LTENR_SetPUSCHMCSIndexTable(ptrLTENR_GNBPHY phy, char* table);

	//Spectrum config
#define LTENR_PHY_GET_SPECTRUMCONFIG(d,in) (((ptrLTENR_GNBPHY)LTENR_GNBPHY_GET((d),(in)))->spectrumConfig)
	void LTENR_PHY_GET_OH(ptrLTENR_SPECTRUMCONFIG sc,
						  double* dlOH,
						  double* ulOH);

	//Propagation model prototypes
	_declspec(dllexport) void LTENR_Propagation_TotalLoss(ptrLTENR_PROPAGATIONINFO info);
	double LTENR_PHY_Send_SINR_to_RRC(ptrLTENR_GNBPHY phy);
	double LTENR_PHY_Send_RSRP_to_RRC(ptrLTENR_GNBPHY phy);
	void LTENR_PHY_ASSOCIATION(NETSIM_ID gnbId, NETSIM_ID gnbIf,
							   NETSIM_ID ueId, NETSIM_ID ueIf,
							   bool isAssociated);
	double LTENR_PHY_RRC_RSRP_SINR(NETSIM_ID gnbId, NETSIM_ID gnbIf,
		NETSIM_ID ueId, NETSIM_ID ueIf);
	double LTENR_PHY_GetDownlinkSpectralEfficiency(ptrLTENR_PROPAGATIONINFO info, int layerId,int CA_ID);
	double LTENR_PHY_GetUplinkSpectralEfficiency(ptrLTENR_PROPAGATIONINFO info, int layerId,int CA_ID);

	//Matlab
	void netsim_matlab_configure_loss_model(ptrLTENR_PROPAGATIONCONFIG config, void* xmlNetSimNode);
	double netsim_matlab_calculate_loss(ptrLTENR_PROPAGATIONINFO info);

	//BeamForming
	void LTENR_BeamForming_InitCoherenceTimer(ptrLTENR_GNBPHY phy);
	void LTENR_BeamForming_Init(ptrLTENR_PROPAGATIONINFO propagation);

	//Python Interface
	void LTENR_addSendReceive(double time);
	void init_Python_log();
	void close_Python_log();
	void LTENR_Python_Log(char* format, ...);

#define LTENR_BeamForming_Uplink_GetValue(info,layerId) (info->uplink.beamFormingGain? info->uplink.beamFormingGain[layerId]:0)
#define LTENR_BeamForming_Downlink_GetValue(info,layerId) (info->downlink.beamFormingGain? info->downlink.beamFormingGain[layerId]:0)
	// Antenna 
	_declspec(dllexport) double LTENR_PHY_GetAntennaGain(ptrLTENR_PROPAGATIONINFO info, ptrLTENR_ANTENNA antenna);
	
#pragma endregion

#ifdef  __cplusplus
}
#endif
#endif /* _NETSIM_LTENR_PHY_H_ */
