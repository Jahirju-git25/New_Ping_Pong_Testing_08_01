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
#include "main.h"
#include "SATELLITE.h"
#include "Satellite_PHY.h"
#include "Satellite_MAC.h"
#include "Satellite_Frame.h"

#pragma region FUNCTIONPROTOTYPE

static FILE* fpRMlog = NULL;

static void create_tx_info_for_ut(NETSIM_ID d, NETSIM_ID in,
								  PTX_INFO info, LINKTYPE linkType);
static void create_tx_info_for_gw(NETSIM_ID d, NETSIM_ID in,
								  PTX_INFO info, LINKTYPE linkType);
static void fill_propagation_from_link(NETSIM_ID d, NETSIM_ID in,
									   PPROPAGATION prop);
#pragma endregion

#pragma region PROPAGATIONMODEL_INIT
void satellite_propgation_ut_init(NETSIM_ID d, NETSIM_ID in)
{
	ptrSATELLITE_UT_PHY phy = SATELLITE_UTPHY_GET(d, in);
	ptrSATELLITE_UT_MAC mac = SATELLITE_UTMAC_GET(d, in);

	TX_INFO txInfo;
	create_tx_info_for_ut(d, in, &txInfo, LINKTYPE_RETURN);

	PROPAGATION prop;
	fill_propagation_from_link(d, in, &prop);

	phy->txPropagationInfo = propagation_create_propagation_info(d, in,
																 mac->satelliteId, mac->satelliteIf,
																 NULL, &txInfo, &prop);

	create_tx_info_for_ut(d, in, &txInfo, LINKTYPE_FORWARD);
	phy->rxPropagationInfo = propagation_create_propagation_info(mac->satelliteId, mac->satelliteIf, 
																 d, in,
																 NULL, &txInfo, &prop);
}

void satellite_propgation_gw_init(NETSIM_ID d, NETSIM_ID in)
{
	ptrSATELLITE_GW_PHY phy = SATELLITE_GWPHY_GET(d, in);
	ptrSATELLITE_GW_MAC mac = SATELLITE_GWMAC_GET(d, in);

	TX_INFO txInfo;
	create_tx_info_for_gw(d, in, &txInfo, LINKTYPE_FORWARD);

	PROPAGATION prop;
	fill_propagation_from_link(d, in, &prop);

	phy->txPropagationInfo = propagation_create_propagation_info(d, in,
																 mac->satelliteId, mac->satelliteIf,
																 NULL, &txInfo, &prop);

	create_tx_info_for_gw(d, in, &txInfo, LINKTYPE_RETURN);
	phy->rxPropagationInfo = propagation_create_propagation_info(mac->satelliteId, mac->satelliteIf,
																 d, in,
																 NULL, &txInfo, &prop);
}
#pragma endregion

#pragma region PROPAGATIONMODEL_HELPER
static void create_tx_info_for_ut(NETSIM_ID d, NETSIM_ID in,
								  PTX_INFO info, LINKTYPE linkType)
{
	memset(info, 0, sizeof * info);
	ptrSATELLITE_UT_PHY phy = SATELLITE_UTPHY_GET(d, in);
	ptrSATELLITE_UT_MAC mac = SATELLITE_UTMAC_GET(d, in);

	NETSIM_ID s = mac->satelliteId;
	NETSIM_ID sin = mac->satelliteIf;
	ptrSATELLITE_MAC smac = SATELLITE_MAC_GET(s, sin);
	ptrSATELLITE_PHY sphy = SATELLITE_PHY_GET(s, sin);

	if (linkType == LINKTYPE_FORWARD)
	{
		ptrSUPERFRAME sf = smac->forwardFLinkSuperFrame;
		info->dRxGain = phy->rxAntennaGain_db;
		info->dTxGain = sphy->txAntennaGain_db;
		info->dTxPower_dbm = sphy->txPower_dbm;
		info->dTxPower_mw = sphy->txPower_mw;
		info->dCentralFrequency = sf->centralFrequency_Hz / MHZ;
	}
	else if (linkType == LINKTYPE_RETURN)
	{
		ptrSUPERFRAME sf = smac->returnLinkSuperFrame;
		info->dTxGain = phy->txAntennaGain_db;
		info->dRxGain = sphy->rxAntennaGain_db;
		info->dTxPower_dbm = phy->txPower_dbm;
		info->dTxPower_mw = phy->txPower_mw;
		info->dCentralFrequency = sf->centralFrequency_Hz / MHZ;
	}

	info->d0 = 1;
	info->dTx_Rx_Distance = DEVICE_DISTANCE(d, s);
}

static void fill_propagation_from_link(NETSIM_ID d, NETSIM_ID in,
									   PPROPAGATION prop)
{
	memset(prop, 0, sizeof * prop);
	NetSim_LINKS* link = DEVICE_PHYLAYER(d, in)->pstruNetSimLinks;
	memcpy(prop, link->puniMedProp.pstruWirelessLink.propagation, sizeof * prop);
	prop->pathlossVar.d0 = 1;
}

static void create_tx_info_for_gw(NETSIM_ID d, NETSIM_ID in,
								  PTX_INFO info, LINKTYPE linkType)
{
	memset(info, 0, sizeof * info);
	ptrSATELLITE_GW_PHY phy = SATELLITE_GWPHY_GET(d, in);
	ptrSATELLITE_GW_MAC mac = SATELLITE_GWMAC_GET(d, in);

	NETSIM_ID s = mac->satelliteId;
	NETSIM_ID sin = mac->satelliteIf;
	ptrSATELLITE_MAC smac = SATELLITE_MAC_GET(s, sin);
	ptrSATELLITE_PHY sphy = SATELLITE_PHY_GET(s, sin);

	if (linkType == LINKTYPE_FORWARD)
	{
		ptrSUPERFRAME sf = smac->forwardFLinkSuperFrame;
		info->dTxGain = phy->txAntennaGain_db;
		info->dRxGain = sphy->rxAntennaGain_db;
		info->dTxPower_dbm = phy->txPower_dbm;
		info->dTxPower_mw = phy->txPower_mw;
		info->dCentralFrequency = sf->centralFrequency_Hz / MHZ;
	}
	else if (linkType == LINKTYPE_RETURN)
	{
		ptrSUPERFRAME sf = smac->returnLinkSuperFrame;
		info->dRxGain = phy->rxAntennaGain_db;
		info->dTxGain = sphy->txAntennaGain_db;
		info->dTxPower_dbm = sphy->txPower_dbm;
		info->dTxPower_mw = sphy->txPower_mw;
		info->dCentralFrequency = sf->centralFrequency_Hz / MHZ;
	}

	info->d0 = 1;
	info->dTx_Rx_Distance = DEVICE_DISTANCE(d, s);
}
#pragma endregion

#pragma region PROPAGATIONMODEL_CALCULATE
void satellite_propagation_ut_calculate_rxpower(NETSIM_ID d, NETSIM_ID in, double time)
{
	ptrSATELLITE_UT_PHY phy = SATELLITE_UTPHY_GET(d, in);
	_propagation_calculate_received_power(phy->txPropagationInfo, time);
	_propagation_calculate_received_power(phy->rxPropagationInfo, time);
}

void satellite_propagation_gw_calculate_rxpower(NETSIM_ID d, NETSIM_ID in, double time)
{
	ptrSATELLITE_GW_PHY phy = SATELLITE_GWPHY_GET(d, in);
	_propagation_calculate_received_power(phy->txPropagationInfo, time);
	_propagation_calculate_received_power(phy->rxPropagationInfo, time);
}
#pragma endregion

#pragma region SATELLITE_PACKET_ERROR
static double satellite_get_rx_power_dbm(NETSIM_ID t, NETSIM_ID ti, NETSIM_ID r, NETSIM_ID ri)
{
	if (isUT(t, ti))
	{
		ptrSATELLITE_UT_PHY phy = SATELLITE_UTPHY_GET(t, ti);
		return _propagation_get_received_power_dbm(phy->txPropagationInfo, 0);
	}
	else if (isGW(t, ti))
	{
		ptrSATELLITE_GW_PHY phy = SATELLITE_GWPHY_GET(t, ti);
		return _propagation_get_received_power_dbm(phy->txPropagationInfo, 0);
	}
	else
	{
		if (isUT(r, ri))
		{
			ptrSATELLITE_UT_PHY phy = SATELLITE_UTPHY_GET(r, ri);
			return _propagation_get_received_power_dbm(phy->rxPropagationInfo, 0);
		}
		else if (isGW(r, ri))
		{
			ptrSATELLITE_GW_PHY phy = SATELLITE_GWPHY_GET(r, ri);
			return _propagation_get_received_power_dbm(phy->rxPropagationInfo, 0);
		}
	}
	return 0;
}

static double satellite_get_tx_power_db(NETSIM_ID t, NETSIM_ID ti)
{
	if (isUT(t, ti))
	{
		ptrSATELLITE_UT_PHY phy = SATELLITE_UTPHY_GET(t, ti);		
		return phy->txPower_dbm;
	}
	else if (isGW(t, ti))
	{
		ptrSATELLITE_GW_PHY phy = SATELLITE_GWPHY_GET(t, ti);
		return phy->txPower_dbm;
	}
	else if (isSATELLITE(t, ti))
	{
		ptrSATELLITE_PHY phy = SATELLITE_PHY_GET(t, ti);
		return phy->txPower_dbm;
	}
	return 0;
}

static double satellite_get_totalloss_db(NETSIM_ID t, NETSIM_ID ti, NETSIM_ID r, NETSIM_ID ri, double Time)
{
	if (isUT(t, ti))
	{
		ptrSATELLITE_UT_PHY phy = SATELLITE_UTPHY_GET(t, ti);
		return propagation_get_Totalloss(phy->txPropagationInfo, Time);
	}
	else if (isGW(t, ti))
	{
		ptrSATELLITE_GW_PHY phy = SATELLITE_GWPHY_GET(t, ti);
		return propagation_get_Totalloss(phy->txPropagationInfo, Time);
	}
	else
	{
		if (isUT(r, ri))
		{
			ptrSATELLITE_UT_PHY phy = SATELLITE_UTPHY_GET(r, ri);
			return propagation_get_Totalloss(phy->rxPropagationInfo, Time);
		}
		else if (isGW(r, ri))
		{
			ptrSATELLITE_GW_PHY phy = SATELLITE_GWPHY_GET(r, ri);
			return propagation_get_Totalloss(phy->rxPropagationInfo, Time);
		}
	}
	return 0;
}

static double satellite_get_pathloss_db(NETSIM_ID t, NETSIM_ID ti, NETSIM_ID r, NETSIM_ID ri, double Time)
{
	if (isUT(t, ti))
	{
		ptrSATELLITE_UT_PHY phy = SATELLITE_UTPHY_GET(t, ti);
		return propagation_get_Pathloss(phy->txPropagationInfo, Time);
	}
	else if (isGW(t, ti))
	{
		ptrSATELLITE_GW_PHY phy = SATELLITE_GWPHY_GET(t, ti);
		return propagation_get_Pathloss(phy->txPropagationInfo, Time);
	}
	else
	{
		if (isUT(r, ri))
		{
			ptrSATELLITE_UT_PHY phy = SATELLITE_UTPHY_GET(r, ri);
			return propagation_get_Pathloss(phy->rxPropagationInfo, Time);
		}
		else if (isGW(r, ri))
		{
			ptrSATELLITE_GW_PHY phy = SATELLITE_GWPHY_GET(r, ri);
			return propagation_get_Pathloss(phy->rxPropagationInfo, Time);
		}
	}
	return 0;
}

static double satellite_get_fading_loss_db(NETSIM_ID t, NETSIM_ID ti, NETSIM_ID r, NETSIM_ID ri)
{
	if (isUT(t, ti))
	{
		ptrSATELLITE_UT_PHY phy = SATELLITE_UTPHY_GET(t, ti);
		return _propagation_calculate_fadingloss(phy->txPropagationInfo);
	}
	else if (isGW(t, ti))
	{
		ptrSATELLITE_GW_PHY phy = SATELLITE_GWPHY_GET(t, ti);
		return _propagation_calculate_fadingloss(phy->txPropagationInfo);
	}
	else
	{
		if (isUT(r, ri))
		{
			ptrSATELLITE_UT_PHY phy = SATELLITE_UTPHY_GET(r, ri);
			return _propagation_calculate_fadingloss(phy->rxPropagationInfo);
		}
		else if (isGW(r, ri))
		{
			ptrSATELLITE_GW_PHY phy = SATELLITE_GWPHY_GET(r, ri);
			return _propagation_calculate_fadingloss(phy->rxPropagationInfo);
		}
	}
	return 0;
}

#define BOLTZMAAN_CONSTANT	1.38064852e-23 // m2 kg s-2 K-1
static double satellite_calculate_noise(NETSIM_ID t, NETSIM_ID ti, NETSIM_ID r, NETSIM_ID ri)
{
	double noise_w = 0;
	if (isUT(t, ti))
	{
		ptrSATELLITE_UT_PHY phy = SATELLITE_UTPHY_GET(t, ti);
		ptrSATELLITE_PHY sphy = SATELLITE_PHY_GET(r, ri);
		noise_w = BOLTZMAAN_CONSTANT * phy->tempeature_k * sphy->returnLinkSuperFrame->carrierConfig->allocatedBandwidth_Hz;
	}
	else if (isGW(t, ti))
	{
		ptrSATELLITE_GW_PHY phy = SATELLITE_GWPHY_GET(t, ti);
		ptrSATELLITE_PHY sphy = SATELLITE_PHY_GET(r, ri);
		noise_w = BOLTZMAAN_CONSTANT * phy->tempeature_k * sphy->forwardLinkSuperFrame->carrierConfig->allocatedBandwidth_Hz;
	}
	else
	{
		if (isUT(r, ri))
		{
			ptrSATELLITE_UT_PHY phy = SATELLITE_UTPHY_GET(r, ri);
			ptrSATELLITE_PHY sphy = SATELLITE_PHY_GET(t, ti);
			noise_w = BOLTZMAAN_CONSTANT * phy->tempeature_k * sphy->forwardLinkSuperFrame->carrierConfig->allocatedBandwidth_Hz;
		}
		else if (isGW(r, ri))
		{
			ptrSATELLITE_GW_PHY phy = SATELLITE_GWPHY_GET(r, ri);
			ptrSATELLITE_PHY sphy = SATELLITE_PHY_GET(t, ti);
			noise_w = BOLTZMAAN_CONSTANT * phy->tempeature_k * sphy->returnLinkSuperFrame->carrierConfig->allocatedBandwidth_Hz;
		}
	}

	return MW_TO_DBM(noise_w * 1000);
}

static double satellite_calculate_snr(double pdbm, double ndbm)
{
	return MW_TO_DBM(DBM_TO_MW(pdbm) / DBM_TO_MW(ndbm));
}

static ptrSUPERFRAME find_superframe(NETSIM_ID t, NETSIM_ID ti, NETSIM_ID r, NETSIM_ID ri)
{
	if (isUT(t, ti))
	{
		ptrSATELLITE_PHY phy = SATELLITE_PHY_GET(r, ri);
		return phy->returnLinkSuperFrame;
	}
	else if (isGW(t, ti))
	{
		ptrSATELLITE_PHY phy = SATELLITE_PHY_GET(r, ri);
		return phy->forwardLinkSuperFrame;
	}
	else
	{
		if (isUT(r, ri))
		{
			ptrSATELLITE_PHY phy = SATELLITE_PHY_GET(t, ti);
			return phy->forwardLinkSuperFrame;
		}
		else if (isGW(r, ri))
		{
			ptrSATELLITE_PHY phy = SATELLITE_PHY_GET(t, ti);
			return phy->returnLinkSuperFrame;
		}
	}
	return NULL;
}

static double satellite_calculate_ber(ptrSUPERFRAME sf, double snr)
{
	ptrCARRIERCONF cc = sf->carrierConfig;
	if (cc->berModel == BERMODEL_Fixed)
		return cc->berValue;
	else if (cc->berModel == BERMODEL_File)
		return calculate_ber(snr, cc->berTable, cc->berTableLen);
	else if (cc->berModel == BERMODEL_Model)
	{
		double dataRate = sf->slotConfig->bitsPerSlot / sf->slotConfig->slotLength_us;
		return Calculate_ber_by_calculation(snr, cc->modulation, dataRate, cc->allocatedBandwidth_Hz / MHZ);
	}
	return 0;
}

static PACKET_STATUS satellite_isPacketErrored(ptrSUPERFRAME sf, double bytes, double snr)
{
	double ber = satellite_calculate_ber(sf, snr);
	return fn_NetSim_Packet_DecideError(ber, bytes);
}

static void init_radio_measurements_log()
{
	if (get_protocol_log_status("SATELLITE_PROPAGATION_LOG"))
{
	char s[BUFSIZ];
		sprintf(s, "%s/log/Satellite_Radio_Measurements_Log.csv",
			pszIOPath);
		fpRMlog = fopen(s, "w");
		if (!fpRMlog)
	{
		fnSystemError("Unable to open %s\n", s);
		perror(s);
		return NULL;
	}
		fprintf(fpRMlog, "Time(ms),Transmitter Name,Receiver Name,Tx_Power(dBm),PathLoss(dB),FadingLoss(dB),TotalLoss(dB),Rx_Power(dBm),Noise(dBm),SNR(dB)\n");
		if (nDbgFlag) fflush(fpRMlog);
	}
}

static void log_radio_measurements(NETSIM_ID t, NETSIM_ID ti, NETSIM_ID r, NETSIM_ID ri, double tx_power, double pathloss, double f, double n,
					   double totalloss, double p, double snr)
{
	if (fpRMlog == NULL) return;

	fprintf(fpRMlog, "%lf,%s,%s,%lf,%lf,%lf,%lf,%lf,%lf,%lf,\n",
		(ldEventTime/MILLISECOND), DEVICE_NAME(t), DEVICE_NAME(r), tx_power, pathloss, f, totalloss, p, n, snr);
	if (nDbgFlag) fflush(fpRMlog);
}

static void close_radio_measurements_log()
{
	if (fpRMlog)
		fclose(fpRMlog);
	}

void Satellite_RadioMeasurements_Init()
{
	init_radio_measurements_log();
}

void Satellite_RadioMeasurements_Finish()
{
	close_radio_measurements_log();
}

void satellite_check_for_packet_error(NETSIM_ID t, NETSIM_ID ti, NETSIM_ID r, NETSIM_ID ri,
									  NetSim_PACKET* packet)
{
	double pdbm = satellite_get_rx_power_dbm(t, ti, r, ri);
	double fdb = satellite_get_fading_loss_db(t, ti, r, ri);
	pdbm -= fdb;

	double ndbm = satellite_calculate_noise(t, ti, r, ri);
	double snr = satellite_calculate_snr(pdbm, ndbm);

	double pathloss = satellite_get_pathloss_db(t, ti, r, ri, ldEventTime);
	double tx_power = satellite_get_tx_power_db(t, ti);
	double totalloss = satellite_get_totalloss_db(t, ti, r, ri, ldEventTime);

	log_radio_measurements(t, ti, r, ri, tx_power, pathloss, fdb, ndbm, totalloss, pdbm + fdb, snr);
	
	double bytes = packet->pstruPhyData->dPacketSize;

	ptrSUPERFRAME sf = find_superframe(t, ti, r, ri);
	packet->nPacketStatus = satellite_isPacketErrored(sf, bytes, snr);
}
#pragma endregion
