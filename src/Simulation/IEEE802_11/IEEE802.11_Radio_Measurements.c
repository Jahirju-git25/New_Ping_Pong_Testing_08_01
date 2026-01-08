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

#include "main.h"
#include "Medium.h"
#include "ErrorModel.h"
#include "PropagationModel.h"
#include "IEEE802_11.h"
#include "IEEE802_11_Phy.h"

static FILE* fpRMlog = NULL;

UINT DEVICE_ID_LIST[] = { 0 };//When { 0 } log is written for all WiFi Devices. Specific WiFi Device ID's can be defined
//for example {1,2,3} log will be written only for WiFi Device ID's 1, 2 and 3 as transmitter/receiver

static bool validate_log(NETSIM_ID nDevID)
{
	int count = sizeof(DEVICE_ID_LIST) / sizeof(int);
	if (count == 1 && DEVICE_ID_LIST[0] == 0) return true;
	else
	{
		for (int i = 0; i < count; i++)
		{
			if (DEVICE_ID_LIST[i] == DEVICE_CONFIGID(nDevID))
				return true;
		}
	}
	return false;
}

static void init_radio_measurements_log()
{
	if (get_protocol_log_status("IEEE_802_11_RADIO_MEASUREMENTS_LOG"))
	{
	char s[BUFSIZ];
		sprintf(s, "%s\\%s", pszIOLogPath, "IEEE_802_11_Radio_Measurements_Log.csv");
	fpRMlog = fopen(s, "w");
	if (!fpRMlog)
	{
		fnSystemError("Unable to open %s file", s);
		perror(s);
	}
	else
	{
		fprintf(fpRMlog, "%s,%s,%s,%s,",
			"Time(ms)", "Transmitter Name", "Receiver Name", "Distance(m)");

		fprintf(fpRMlog, "%s,%s,%s,",
			"Packet ID", "Packet Type", "Control Packet Type");

		fprintf(fpRMlog, "%s,%s,%s,%s,",
			"Tx_Power(dBm)", "Path Loss(dB)", "Shadowing Loss(dB)", "Fading Loss(dB)");


			fprintf(fpRMlog, "%s,%s,%s,",
				"Total Loss(dB)", "Tx Antenna Gain(dB)", "Rx Antenna Gain(dB)");

			fprintf(fpRMlog, "%s,%s,%s,",
				"Rx_Power(dBm)", "Interference(dBm)", "SNR(dB)");

		fprintf(fpRMlog, "%s,%s,%s,%s,",
			"SINR(dB)", "NSS", "BER", "MCS Index");

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

void IEEE_802_11RadioMeasurements_Init()
{
	init_radio_measurements_log();
}

void IEEE_802_11RadioMeasurements_Finish()
{
	close_radio_measurements_log();
}

void IEEE_802_11RadioMeasurements_Log(NetSim_PACKET* packet, NETSIM_ID txid, NETSIM_ID txif, NETSIM_ID rxid, NETSIM_ID rxif)
{
	if (fpRMlog == NULL || (!validate_log(txid) && !validate_log(rxid))) { return; }


	PPROPAGATION_INFO txinfo = find_propagation_info(txid, txif, rxid, rxif);
	PIEEE802_11_PHY_VAR txPhy = IEEE802_11_PHY(txid, txif);
	PIEEE802_11_PHY_VAR rxPhy = IEEE802_11_PHY(rxid, rxif);

	double rxPowerdBm = _propagation_get_received_power_dbm(txinfo, pstruEventDetails->dEventTime);
	double interference = medium_get_interference(packet, txid, txif, rxid, rxif);
	double FadingLoss = propagation_get_Fadingloss(txinfo, pstruEventDetails->dEventTime);
	rxPowerdBm -= FadingLoss;
	double ber = 0;
	switch (txPhy->PhyProtocol)
	{
	case IEEE_802_11b:
		if (strcmp(strBERMODEL[txPhy->Ber_Model], "SINR_BER_PER_TABLE") == 0)
			ber = calculate_BER_Using_Table(txPhy->PHY_TYPE.dsssPhy.modulation, rxPowerdBm, interference, txPhy->dChannelBandwidth, txPhy->MCS, strWLANProtocol[txPhy->PhyProtocol]);
		else
		ber = calculate_FEC_BER(txPhy->PHY_TYPE.dsssPhy.modulation, txPhy->PHY_TYPE.dsssPhy.dCodeRate, rxPowerdBm, interference, txPhy->dChannelBandwidth, txPhy->PHY_TYPE.dsssPhy.dDataRate);
		break;
	case IEEE_802_11a:
	case IEEE_802_11g:
	case IEEE_802_11p:
		if (strcmp(strBERMODEL[txPhy->Ber_Model], "SINR_BER_PER_TABLE") == 0)
			ber = calculate_BER_Using_Table(txPhy->PHY_TYPE.ofdmPhy.modulation, rxPowerdBm, interference, txPhy->dChannelBandwidth, txPhy->MCS, strWLANProtocol[txPhy->PhyProtocol]);
		else
		ber = calculate_FEC_BER(txPhy->PHY_TYPE.ofdmPhy.modulation, txPhy->PHY_TYPE.ofdmPhy.dCodingRate, rxPowerdBm, interference, txPhy->dChannelBandwidth, txPhy->PHY_TYPE.ofdmPhy.dDataRate);
		break;
	case IEEE_802_11n:
		if (strcmp(strBERMODEL[txPhy->Ber_Model], "SINR_BER_PER_TABLE") == 0)
			ber = calculate_BER_Using_Table(txPhy->PHY_TYPE.ofdmPhy_11n.modulation, rxPowerdBm, interference, txPhy->dChannelBandwidth, txPhy->MCS, strWLANProtocol[txPhy->PhyProtocol]);
		else
		ber = calculate_FEC_BER(txPhy->PHY_TYPE.ofdmPhy_11n.modulation, txPhy->PHY_TYPE.ofdmPhy_11n.dCodingRate, rxPowerdBm, interference, txPhy->dChannelBandwidth, txPhy->PHY_TYPE.ofdmPhy_11n.dDataRate);
		break;
	case IEEE_802_11ac:
		if (strcmp(strBERMODEL[txPhy->Ber_Model], "SINR_BER_PER_TABLE") == 0)
			ber = calculate_BER_Using_Table(txPhy->PHY_TYPE.ofdmPhy_11ac.modulation, rxPowerdBm, interference, txPhy->dChannelBandwidth, txPhy->MCS, strWLANProtocol[txPhy->PhyProtocol]);
		else
		ber = calculate_FEC_BER(txPhy->PHY_TYPE.ofdmPhy_11ac.modulation, txPhy->PHY_TYPE.ofdmPhy_11ac.dCodingRate, rxPowerdBm, interference, txPhy->dChannelBandwidth, txPhy->PHY_TYPE.ofdmPhy_11ac.dDataRate);
		break;
	default:
		fnNetSimError("IEEE802.11--- Unknown protocol %d in %s\n", txPhy->PhyProtocol, __FUNCTION__);
		break;
	}
	char type[BUFSIZ];
	char ptype[BUFSIZ];
	fprintf(fpRMlog, "%lf,%s,%s,%lf,",
		(ldEventTime/MILLISECOND), DEVICE_NAME(txid),
		DEVICE_NAME(rxid),
		DEVICE_DISTANCE(txid, rxid));

	fprintf(fpRMlog, "%lld,%s,%s,",
		packet->nPacketId,
		fn_NetSim_Config_GetPacketTypeAsString(packet->nPacketType, ptype),
		fn_NetSim_Config_GetControlPacketType(packet, type));

	fprintf(fpRMlog, "%lf,%lf,%lf,%lf,",
		MW_TO_DBM(txPhy->nTxPower_mw),
		propagation_get_Pathloss(txinfo, pstruEventDetails->dEventTime),
		propagation_get_Shadowloss(txinfo, pstruEventDetails->dEventTime),
		FadingLoss);


	fprintf(fpRMlog, "%lf,%lf,%lf,%lf,%lf,",
		propagation_get_Totalloss(txinfo, pstruEventDetails->dEventTime),
		txinfo->txInfo.dTxGain,
		txinfo->txInfo.dRxGain,
		rxPowerdBm,
		interference);


	fprintf(fpRMlog, "%lf,%lf,%d,%lf,%d,",
		calculate_sinr(rxPowerdBm, NEGATIVE_DBM, txPhy->dChannelBandwidth),
		calculate_sinr(rxPowerdBm, interference, txPhy->dChannelBandwidth),
		txPhy->NSS,
		ber,
		txPhy->MCS);

	fprintf(fpRMlog, "\n");
	if (nDbgFlag) fflush(fpRMlog);
}

