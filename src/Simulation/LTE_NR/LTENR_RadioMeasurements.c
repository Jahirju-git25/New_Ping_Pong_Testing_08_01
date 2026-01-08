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
* Author:    Kanak				                                                    *
*										                                            *
* ----------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "LTENR_MAC.h"
#include "LTENR_PHY.h"
#include "Stack.h"

static FILE* fpRMlog = NULL;

UINT UE_ID_LIST[] = { 0 };//When { 0 } log is written for all UE's. Specific UE ID's can be defined
//for example {1,2,3} log will be written only for UE ID's 1, 2 and 3

static bool validate_log(NETSIM_ID UE_ID)
{
	int count = sizeof(UE_ID_LIST) / sizeof(int);
	if (count == 1 && UE_ID_LIST[0] == 0) return true;
	else
	{
		for (int i = 0; i < count; i++)
		{
			if (UE_ID_LIST[i] == DEVICE_CONFIGID(UE_ID))
				return true;
		}
	}
	return false;
}

static void init_radio_measurements_log()
{
	if (get_protocol_log_status("LTENR_Radio_Measurements_Log"))
	{
	char s[BUFSIZ];
	sprintf(s, "%s\\%s", pszIOLogPath, "LTENR_Radio_Measurements_Log.csv");
	fpRMlog = fopen(s, "w");
	if (!fpRMlog)
	{
		fnSystemError("Unable to open %s file", s);
		perror(s);
	}
	else
	{
		fprintf(fpRMlog, "%s,%s,%s,%s,",
			"Time(ms)", "gNB or eNB Name", "UE Name", "Distance(m)");

		fprintf(fpRMlog, "%s,%s,%s,%s,%s,%s,",
			"isAssociated", "CC_ID", "Band", "Channel", "Layer ID", "Tx_Power(dBm)");

		fprintf(fpRMlog, "%s,%s,%s,%s,",
			"LoS State", "TotalLoss(dB)", "PathLoss(dB)", "ShadowFadingLoss(dB)");

			fprintf(fpRMlog, "%s,%s,%s,%s,%s,",
				"O2I_Loss(dBm)", "Additional_Loss(dB)", "Antenna Gain(dB)", "Rx_Power(dBm)", "SNR(dB)");

		fprintf(fpRMlog, "%s,%s,%s,",
			"SINR(dB)", "ThermalNoise(dBm)","InterferencePower(dBm)");

		fprintf(fpRMlog, "%s,",
			"BeamFormingGain(dB)");

		fprintf(fpRMlog, "%s,%s,",
			"CQI Index", "MCS Index");

		fprintf(fpRMlog, "\n");
		if (nDbgFlag) fflush(fpRMlog);
	}
	}
}

static void close_radio_measurements_log()
{
	if (fpRMlog)
		fclose(fpRMlog);
}

void LTENR_RadioMeasurements_Init()
{
	init_radio_measurements_log();
}

void LTENR_RadioMeasurements_Finish()
{
	close_radio_measurements_log();
}

void LTENR_RadioMeasurements_PDSCH_Log(ptrLTENR_GNBPHY phy, int CA_ID,
	ptrLTENR_ASSOCIATEDUEPHYINFO info)
{
	if (fpRMlog == NULL || !validate_log(info->ueId)) return;

	UINT layerCount;
	ptrLTENR_PROPAGATIONCONFIG propagation = phy->propagationConfig;
	ptrLTENR_PROPAGATIONINFO pinfo = info->propagationInfo[CA_ID];
	string pszassoc_status = info->isAssociated ? "true" : "false";
	ptrLTENR_CA ca = phy->spectrumConfig->CA[CA_ID];
	LTENR_LOS_NLOS_STATE los_state = pinfo->propagationConfig->state;

	if (!info->isAssociated || (ca->configSlotType == SLOT_UPLINK)) return;
	layerCount = pinfo->downlink.layerCount;
	for (UINT i = 0; i < layerCount; i++)
	{
		fprintf(fpRMlog, "%.2lf,%s,%s,%lf,",
			ldEventTime / MILLISECOND, DEVICE_NAME(phy->gnbId),
			DEVICE_NAME(info->ueId),
			DEVICE_DISTANCE(phy->gnbId, info->ueId));

		fprintf(fpRMlog, "%s,%d,%s,%s,%d,%lf,",
			pszassoc_status, CA_ID + 1, ca->operatingBand, "PDSCH",
			i + 1, pinfo->downlink.txPower_dbm);

		if (propagation->pathLossModel == LTENR_PATHLOSS_MODEL_LOG_DISTANCE)
			fprintf(fpRMlog, "%s,%lf,%lf,",
				"N/A", pinfo->dTotalLoss, pinfo->dPathLoss);
		else
			fprintf(fpRMlog, "%s,%lf,%lf,",
				strLTENR_STATE[los_state], pinfo->dTotalLoss, pinfo->dPathLoss);

		if (pinfo->propagationConfig->isShadowFadingEnabled)
			fprintf(fpRMlog, "%lf,",
				pinfo->dShadowFadingLoss);
		else
			fprintf(fpRMlog, "%s,",
				"N/A");

		fprintf(fpRMlog, "%lf,%lf,%lf,%lf,%lf,",
			pinfo->dO2ILoss, pinfo->dAdditionalLoss, pinfo->downlink.antennaGain, pinfo->downlink.rxPower_dbm[i], pinfo->downlink.SNR_db[i]);

		fprintf(fpRMlog, "%lf,%lf,%lf,",
			pinfo->downlink.SINR_db[i],pinfo->downlink.thermalNoise, pinfo->downlink.InterferencePower_dbm[i]);

		if (propagation->fastFadingModel == LTENR_FASTFADING_MODEL_AWGN_WITH_RAYLEIGH_FADING)
			fprintf(fpRMlog, "%lf,",
				pinfo->downlink.beamFormingGain[i]);
		else if (propagation->fastFadingModel == LTENR_FASTFADING_MODEL_NO_FADING_MIMO_ARRAY_GAIN)
			fprintf(fpRMlog, "%lf,",
				pinfo->downlink.ArrayGain);
		else
			fprintf(fpRMlog, "%s,",
				"N/A");

		fprintf(fpRMlog, "%d,%d,",
			info->downlinkAMCInfo[CA_ID][i]->cqiTable.CQIIndex,
			info->downlinkAMCInfo[CA_ID][i]->mcsTable.mcsIndex);

		fprintf(fpRMlog, "\n");
		if (nDbgFlag) fflush(fpRMlog);
	}
}

void LTENR_RadioMeasurements_PUSCH_Log(ptrLTENR_GNBPHY phy, int CA_ID,
	ptrLTENR_ASSOCIATEDUEPHYINFO info)
{
	if (fpRMlog == NULL || !validate_log(info->ueId)) return;

	UINT layerCount;
	ptrLTENR_PROPAGATIONCONFIG propagation = phy->propagationConfig;
	ptrLTENR_PROPAGATIONINFO pinfo = info->propagationInfo[CA_ID];
	string pszassoc_status = info->isAssociated ? "true" : "false";
	ptrLTENR_CA ca = phy->spectrumConfig->CA[CA_ID];
	LTENR_LOS_NLOS_STATE los_state = pinfo->propagationConfig->state;

	if (!info->isAssociated || (ca->configSlotType == SLOT_DOWNLINK)) return;
	layerCount = pinfo->uplink.layerCount;
	for (UINT i = 0; i < layerCount; i++)
	{
		fprintf(fpRMlog, "%.2lf,%s,%s,%lf,",
			ldEventTime / MILLISECOND, DEVICE_NAME(phy->gnbId),
			DEVICE_NAME(info->ueId),
			DEVICE_DISTANCE(phy->gnbId, info->ueId));

		fprintf(fpRMlog, "%s,%d,%s,%s,%d,%lf,",
			pszassoc_status, CA_ID + 1, ca->operatingBand, "PUSCH",
			i + 1, pinfo->uplink.txPower_dbm);
		if (propagation->pathLossModel == LTENR_PATHLOSS_MODEL_LOG_DISTANCE)
			fprintf(fpRMlog, "%s,%lf,%lf,",
				"N/A", pinfo->dTotalLoss, pinfo->dPathLoss);
		else
			fprintf(fpRMlog, "%s,%lf,%lf,",
				strLTENR_STATE[los_state], pinfo->dTotalLoss, pinfo->dPathLoss);

		if (pinfo->propagationConfig->isShadowFadingEnabled)
			fprintf(fpRMlog, "%lf,",
				pinfo->dShadowFadingLoss);
		else
			fprintf(fpRMlog, "%s,",
				"N/A");

		fprintf(fpRMlog, "%lf,%lf,%lf,%lf,%lf,",
			pinfo->dO2ILoss, pinfo->dAdditionalLoss, pinfo->uplink.antennaGain, pinfo->uplink.rxPower_dbm[i], pinfo->uplink.SNR_db[i]);

		fprintf(fpRMlog, "%lf,%lf,%lf,",
			pinfo->uplink.SINR_db[i],pinfo->uplink.thermalNoise, pinfo->uplink.InterferencePower_dbm[i]);

		if (propagation->fastFadingModel == LTENR_FASTFADING_MODEL_AWGN_WITH_RAYLEIGH_FADING)
			fprintf(fpRMlog, "%lf,",
				pinfo->uplink.beamFormingGain[i]);
		else if (propagation->fastFadingModel == LTENR_FASTFADING_MODEL_NO_FADING_MIMO_ARRAY_GAIN)
			fprintf(fpRMlog, "%lf,",
				pinfo->uplink.ArrayGain);
		else
			fprintf(fpRMlog, "%s,",
				"N/A");

		fprintf(fpRMlog, "%d,%d,",
			info->uplinkAMCInfo[CA_ID][i]->cqiTable.CQIIndex,
			info->uplinkAMCInfo[CA_ID][i]->mcsTable.mcsIndex);

		fprintf(fpRMlog, "\n");
		if (nDbgFlag) fflush(fpRMlog);
	}
}

void LTENR_RadioMeasurements_PBSCH_Log(ptrLTENR_GNBPHY phy, int CA_ID,
	ptrLTENR_ASSOCIATEDUEPHYINFO info)
{
	if (fpRMlog == NULL || !validate_log(info->ueId)) return;

	ptrLTENR_PROPAGATIONCONFIG propagation = phy->propagationConfig;
	ptrLTENR_PROPAGATIONINFO pinfo = info->propagationInfo[CA_ID];
	string pszassoc_status = info->isAssociated ? "true" : "false";
	ptrLTENR_CA ca = phy->spectrumConfig->CA[CA_ID];
	LTENR_LOS_NLOS_STATE los_state = pinfo->propagationConfig->state;

	if ((ca->configSlotType == SLOT_UPLINK)) return;

	fprintf(fpRMlog, "%.2lf,%s,%s,%lf,",
		ldEventTime / MILLISECOND, DEVICE_NAME(phy->gnbId),
		DEVICE_NAME(info->ueId),
		DEVICE_DISTANCE(phy->gnbId, info->ueId));

	fprintf(fpRMlog, "%s,%d,%s,%s,%s,%lf,",
		pszassoc_status, CA_ID + 1, ca->operatingBand, "SSB",
		"N/A", pinfo->downlink.totaltxpower_dbm);

	if (propagation->pathLossModel == LTENR_PATHLOSS_MODEL_LOG_DISTANCE)
		fprintf(fpRMlog, "%s,%lf,%lf,",
			"N/A", pinfo->dTotalLoss, pinfo->dPathLoss);
	else
		fprintf(fpRMlog, "%s,%lf,%lf,",
			strLTENR_STATE[los_state], pinfo->dTotalLoss, pinfo->dPathLoss);

	if (pinfo->propagationConfig->isShadowFadingEnabled)
		fprintf(fpRMlog, "%lf,",
			pinfo->dShadowFadingLoss);
	else
		fprintf(fpRMlog, "%s,",
			"N/A");

	fprintf(fpRMlog, "%lf,%lf,%lf,%lf,%lf,",
		pinfo->dO2ILoss, pinfo->dAdditionalLoss, pinfo->downlink.antennaGain, pinfo->downlink.SSB_rxPower_dBm, pinfo->downlink.SSB_SNR_dB);

	fprintf(fpRMlog, "%s,%s,%s,",
		"N/A","N/A", "N/A");

	fprintf(fpRMlog, "%lf,",
		pinfo->downlink.SSB_AnalogBeamformingGain_dB);

	fprintf(fpRMlog, "%s,%s,",
		"N/A", "N/A");

	fprintf(fpRMlog, "\n");
	if (nDbgFlag) fflush(fpRMlog);
}