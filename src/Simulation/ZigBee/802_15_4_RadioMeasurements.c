/************************************************************************************
* Copyright (C) 2022																*
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
# include "802_15_4.h"


static FILE* fpRMlog = NULL;

UINT DEVICE_ID_LIST[] = { 0 };//When { 0 } log is written for all ZigBee Devices. Specific ZigBee Device ID's can be defined
//for example {1,2,3} log will be written only for ZigBee Device ID's 1, 2 and 3 as transmitter/receiver

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
	if (get_protocol_log_status("IEEE802_15_4_RADIO_MEASUREMENTS_LOG"))
	{
	char s[BUFSIZ];
	sprintf(s, "%s\\%s", pszIOLogPath, "IEEE802_15_4_Radio_Measurements_Log.csv");
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


		fprintf(fpRMlog, "%s,%s,%s,%s,",
			"Total Loss(dB)", "Rx_Power(dBm)", "SNR(dB)", "BER");


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

void RadioMeasurements_802_15_4_Init()
{
	init_radio_measurements_log();
}

void RadioMeasurements_802_15_4_Finish()
{
	close_radio_measurements_log();
}

void RadioMeasurements_802_15_4_Log(NetSim_PACKET* packet, NETSIM_ID txid, NETSIM_ID txif, NETSIM_ID rxid, NETSIM_ID rxif)
{
	if (fpRMlog == NULL || (!validate_log(txid) && !validate_log(rxid))) { return; }


	PPROPAGATION_INFO txinfo = find_propagation_info(txid, txif, rxid, rxif);
	//PPROPAGATION_INFO rxinfo = find_propagation_info(rxid, rxif, txid, txif);
	ptrIEEE802_15_4_PHY_VAR txPhy = WSN_PHY(txid);
	double rxPowerdBm = GET_RX_POWER_dbm(packet->nTransmitterId, packet->nReceiverId, pstruEventDetails->dEventTime);
	double FadingLoss = propagation_get_Fadingloss(txinfo, pstruEventDetails->dEventTime);
	rxPowerdBm -= FadingLoss;
	double ber = 0;
	double SNR = calculate_sinr(rxPowerdBm, NEGATIVE_DBM, 5);
	ber = fn_NetSim_Zigbee_CalculateBER(SNR);

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
		MW_TO_DBM(txPhy->dTransmitterPower_mw),
		propagation_get_Pathloss(txinfo, pstruEventDetails->dEventTime),
		propagation_get_Shadowloss(txinfo, pstruEventDetails->dEventTime),
		FadingLoss);


	fprintf(fpRMlog, "%lf,%lf,%lf,%lf,",
		propagation_get_Totalloss(txinfo, pstruEventDetails->dEventTime),
		rxPowerdBm,
		SNR,
		ber);


	fprintf(fpRMlog, "\n");
	if (nDbgFlag) fflush(fpRMlog);
}

