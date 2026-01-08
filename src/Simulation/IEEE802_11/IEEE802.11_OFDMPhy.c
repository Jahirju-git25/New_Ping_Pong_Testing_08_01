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
#include "main.h"
#include "IEEE802_11.h"
#include "IEEE802_11_Phy.h"
#include "ErrorModel.h"

/// Data structure for physical layer parameters
struct stru_802_11_Phy_Parameters_OFDM
{
	int nIndex;
	double dRxSensitivity;
	PHY_MODULATION nModulation;
	double dDataRate;
	double dCodingRate;
	int nNBPSC;
	int nNCBPS;
	int nNDBPS;
	double dInterferenceThershold;
};

static struct stru_802_11_Phy_Parameters_OFDM struPhyParameters_20MHz[8] =
{
	//IEEE802.11-OFDM Phy
	{1,-82,Modulation_BPSK,6,1.0 / 2.0,1,48,24,0},
	{2,-81,Modulation_BPSK,9,3.0 / 4.0,1,48,36,0},
	{3,-79,Modulation_QPSK,12,1.0 / 2.0,2,96,48,0},
	{4,-77,Modulation_QPSK,18,3.0 / 4.0,2,96,72,0},
	{5,-74,Modulation_16_QAM,24,1.0 / 2.0,4,192,96,0},
	{6,-70,Modulation_16_QAM,36,3.0 / 4.0,4,192,144,0},
	{7,-66,Modulation_64_QAM,48,2.0 / 3.0,6,288,192,0},
	{8,-65,Modulation_64_QAM,54,3.0 / 4.0,6,288,216,0},
};

static struct stru_802_11_Phy_Parameters_OFDM struPhyParameters_10MHz[8] =
{
	//IEEE802.11-OFDM Phy
	{1,-85,Modulation_BPSK,3,1.0/2.0,1,48,24,0},
	{2,-84,Modulation_BPSK,4.5,3.0/4.0,1,48,36,0},
	{3,-82,Modulation_QPSK,6,1.0/2.0,2,96,48,0},
	{4,-80,Modulation_QPSK,9,3.0/4.0,2,96,72,0},
	{5,-77,Modulation_16_QAM,12,1.0/2.0,4,192,96,0},
	{6,-73,Modulation_16_QAM,18,3.0/4.0,4,192,144,0},
	{7,-69,Modulation_64_QAM,24,2.0/3.0,6,288,192,0},
	{8,-68,Modulation_64_QAM,27,3.0/4.0,6,288,216,0},
};

static struct stru_802_11_Phy_Parameters_OFDM struPhyParameters_5MHz[8] =
{
	//IEEE802.11-OFDM Phy
	{1,-88,Modulation_BPSK,1.5,1.0/2.0,1,48,24},
	{2,-87,Modulation_BPSK,2.25,3.0/4.0,1,48,36},
	{3,-85,Modulation_QPSK,3,1.0/2.0,2,96,48},
	{4,-83,Modulation_QPSK,4.5,3.0/4.0,2,96,72},
	{5,-80,Modulation_16_QAM,6,1.0/2.0,4,192,96},
	{6,-76,Modulation_16_QAM,9,3.0/4.0,4,192,144},
	{7,-72,Modulation_64_QAM,12,2.0/3.0,6,288,192},
	{8,-71,Modulation_64_QAM,13.5,3.0/4.0,6,288,216},
};
#define MAX_RATE_INDEX 7
#define MIN_RATE_INDEX 0

/*
18.3.10.2 Receiver minimum input sensitivity
The packet error ratio (PER) shall be 10% or less when the PSDU length is 1000 octets
*/
#define TARGET_PEP 0.1
#define REF_PACKET_SIZE 1000 //Bytes

unsigned int get_ofdm_phy_max_index()
{
	return MAX_RATE_INDEX;
}

unsigned int get_ofdm_phy_min_index()
{
	return MIN_RATE_INDEX;
}

void get_ofdm_phy_all_rate(int bandwidth_MHz, double* rate, UINT* len)
{
	int i;
	for (i = MIN_RATE_INDEX; i <= MAX_RATE_INDEX; i++)
		if (bandwidth_MHz == 10)
			rate[i] = struPhyParameters_10MHz[i].dDataRate;
		else if (bandwidth_MHz == 5)
			rate[i] = struPhyParameters_5MHz[i].dDataRate;
		else
			rate[i] = struPhyParameters_20MHz[i].dDataRate;
	*len = MAX_RATE_INDEX - MIN_RATE_INDEX + 1;
}

int fn_NetSim_IEEE802_11_OFDMPhy_UpdateParameter()
{
#ifdef _RECALCULATE_RX_SENSITIVITY_BASED_ON_PEP_

	int i;
	fprintf(stderr, "Calculating receiver sensitivity for OFDM phy\n"
			"Please wait as this may take some time...\n");

	char s[BUFSIZ];
	sprintf(s, "%s\\%s", pszIOLogPath, "rx_sensitivity_wlan_OFDMPhy.csv");
	FILE* fp = fopen(s, "w");
	fprintf(fp, "Bandwidth,NSS,Modulation,Datarate,receiversensitivity\n");

	for (i = MIN_RATE_INDEX; i <= MAX_RATE_INDEX; i++)
	{
		if (struPhyParameters_5MHz[i].dDataRate > 0)
		{
			struPhyParameters_5MHz[i].dRxSensitivity = calculate_rxpower_by_per(TARGET_PEP, REF_PACKET_SIZE,
				struPhyParameters_5MHz[i].nModulation,
				struPhyParameters_5MHz[i].dDataRate,
				(double)5.0);

			fprintf(fp, "%d,%d,%s,%lf,%lf\n",
				5, 1,
				strPHY_MODULATION[struPhyParameters_5MHz[i].nModulation],
				struPhyParameters_5MHz[i].dDataRate, struPhyParameters_5MHz[i].dRxSensitivity);
			fflush(fp);
		}

		if (struPhyParameters_10MHz[i].dDataRate > 0)
		{
			struPhyParameters_10MHz[i].dRxSensitivity = calculate_rxpower_by_per(TARGET_PEP, REF_PACKET_SIZE,
																				 struPhyParameters_10MHz[i].nModulation,
																				 struPhyParameters_10MHz[i].dDataRate,
																				 (double)10.0);

			fprintf(fp, "%d,%d,%s,%lf,%lf\n",
					10, 1,
					strPHY_MODULATION[struPhyParameters_10MHz[i].nModulation],
					struPhyParameters_10MHz[i].dDataRate, struPhyParameters_10MHz[i].dRxSensitivity);
			fflush(fp);
		}

		if (struPhyParameters_20MHz[i].dDataRate > 0)
		{
			struPhyParameters_20MHz[i].dRxSensitivity = calculate_rxpower_by_per(TARGET_PEP, REF_PACKET_SIZE,
																				 struPhyParameters_20MHz[i].nModulation,
																				 struPhyParameters_20MHz[i].dDataRate,
																				 (double)20.0);

			fprintf(fp, "%d,%d,%s,%lf,%lf\n",
					20, 1,
					strPHY_MODULATION[struPhyParameters_20MHz[i].nModulation],
					struPhyParameters_20MHz[i].dDataRate, struPhyParameters_20MHz[i].dRxSensitivity);
			fflush(fp);
		}
	}
	fclose(fp);
#endif // _RECALCULATE_RX_SENSITIVITY_BASED_ON_PEP_
	return 0;
}

static struct stru_802_11_Phy_Parameters_OFDM* get_phy_parameter_OFDM(PIEEE802_11_PHY_VAR phy)
{
	switch((int)phy->dChannelBandwidth)
	{
	case 5:
		return struPhyParameters_5MHz;
	case 10:
		return struPhyParameters_10MHz;
	case 20:
		return struPhyParameters_20MHz;
	default:
		fnNetSimError("Unknown channel bandwidth %lf in %s\n",phy->dChannelBandwidth,__FUNCTION__);
		return NULL;
	}
}

void fn_NetSim_IEEE802_11_OFDMPhy_SetEDThreshold(PIEEE802_11_PHY_VAR phy)
{
	struct stru_802_11_Phy_Parameters_OFDM *struPhyParameters = get_phy_parameter_OFDM(phy);
	phy->dEDThreshold = struPhyParameters[MIN_RATE_INDEX].dRxSensitivity + CSRANGEDIFF;
}

/**
This function is used to calculate the data rate for IEEE 802.11-OFDM Phy
*/
int fn_NetSim_IEEE802_11_OFDMPhy_DataRate(NETSIM_ID nDeviceId, NETSIM_ID nInterfaceId, NETSIM_ID nReceiverId,NetSim_PACKET* packet,double time)
{
	PIEEE802_11_MAC_VAR mac = IEEE802_11_MAC(nDeviceId,nInterfaceId);
	PIEEE802_11_PHY_VAR pstruPhy = IEEE802_11_PHY(nDeviceId,nInterfaceId);
	struct stru_802_11_Phy_Parameters_OFDM *struPhyParameters = get_phy_parameter_OFDM(pstruPhy);

	if(mac->rate_adaptationAlgo== RATEADAPTATION_GENERIC)
	{
		unsigned int index = get_rate_index(nDeviceId,nInterfaceId,nReceiverId);
		if(isIEEE802_11_CtrlPacket(packet))
			goto CONTROL_RATE;
		if(nReceiverId)
		{
			pstruPhy->PHY_TYPE.ofdmPhy.dDataRate = struPhyParameters[index].dDataRate;
			pstruPhy->dCurrentRxSensitivity_dbm = struPhyParameters[index].dRxSensitivity;
			pstruPhy->PHY_TYPE.ofdmPhy.modulation = struPhyParameters[index].nModulation;
			pstruPhy->PHY_TYPE.ofdmPhy.dCodingRate = struPhyParameters[index].dCodingRate;
			pstruPhy->PHY_TYPE.ofdmPhy.nNBPSC = struPhyParameters[index].nNBPSC;
			pstruPhy->PHY_TYPE.ofdmPhy.nNCBPS = struPhyParameters[index].nNCBPS;
			pstruPhy->PHY_TYPE.ofdmPhy.nNDBPS = struPhyParameters[index].nNDBPS;
			pstruPhy->MCS = struPhyParameters[index].nIndex - 1;
		}
		else // Broadcast packets
		{
			goto BROADCAST_RATE;
		}
	}
	else if(mac->rate_adaptationAlgo== RATEADAPTATION_MINSTREL)
	{
		unsigned int index = get_minstrel_rate_index(nDeviceId,nInterfaceId,nReceiverId);
		if(isIEEE802_11_CtrlPacket(packet))
			goto CONTROL_RATE;
		if(nReceiverId)
		{
			pstruPhy->PHY_TYPE.ofdmPhy.dDataRate = struPhyParameters[index].dDataRate;
			pstruPhy->dCurrentRxSensitivity_dbm = struPhyParameters[index].dRxSensitivity;
			pstruPhy->PHY_TYPE.ofdmPhy.modulation = struPhyParameters[index].nModulation;
			pstruPhy->PHY_TYPE.ofdmPhy.dCodingRate = struPhyParameters[index].dCodingRate;
			pstruPhy->PHY_TYPE.ofdmPhy.nNBPSC = struPhyParameters[index].nNBPSC;
			pstruPhy->PHY_TYPE.ofdmPhy.nNCBPS = struPhyParameters[index].nNCBPS;
			pstruPhy->PHY_TYPE.ofdmPhy.nNDBPS = struPhyParameters[index].nNDBPS;
			pstruPhy->MCS = struPhyParameters[index].nIndex - 1;
		}
		else // Broadcast packets
		{
			goto BROADCAST_RATE;
		}
	}
	else
	{
		if(nReceiverId)
		{
			if(isIEEE802_11_CtrlPacket(packet))
			{
				CONTROL_RATE:
				pstruPhy->PHY_TYPE.ofdmPhy.dDataRate = struPhyParameters[0].dDataRate;
				pstruPhy->dCurrentRxSensitivity_dbm = struPhyParameters[0].dRxSensitivity;
				pstruPhy->PHY_TYPE.ofdmPhy.modulation = struPhyParameters[0].nModulation;
				pstruPhy->PHY_TYPE.ofdmPhy.dCodingRate = struPhyParameters[0].dCodingRate;
				pstruPhy->PHY_TYPE.ofdmPhy.nNBPSC = struPhyParameters[0].nNBPSC;
				pstruPhy->PHY_TYPE.ofdmPhy.nNCBPS = struPhyParameters[0].nNCBPS;
				pstruPhy->PHY_TYPE.ofdmPhy.nNDBPS = struPhyParameters[0].nNDBPS;
				pstruPhy->MCS = struPhyParameters[0].nIndex - 1;
			}
			else
			{
				NETSIM_ID in = fn_NetSim_Stack_GetConnectedInterface(nDeviceId, nInterfaceId, nReceiverId);
				double rssi =GET_RX_POWER_dbm(nDeviceId, nInterfaceId, nReceiverId, in, time);
				int i;
				for(i=MAX_RATE_INDEX;i>=MIN_RATE_INDEX;i--)
				{
					if(rssi >= struPhyParameters[i].dRxSensitivity || i==MIN_RATE_INDEX)
					{
						pstruPhy->PHY_TYPE.ofdmPhy.dDataRate = struPhyParameters[i].dDataRate;
						pstruPhy->dCurrentRxSensitivity_dbm = struPhyParameters[i].dRxSensitivity;
						pstruPhy->PHY_TYPE.ofdmPhy.modulation = struPhyParameters[i].nModulation;
						pstruPhy->PHY_TYPE.ofdmPhy.dCodingRate = struPhyParameters[i].dCodingRate;
						pstruPhy->PHY_TYPE.ofdmPhy.nNBPSC = struPhyParameters[i].nNBPSC;
						pstruPhy->PHY_TYPE.ofdmPhy.nNCBPS = struPhyParameters[i].nNCBPS;
						pstruPhy->PHY_TYPE.ofdmPhy.nNDBPS = struPhyParameters[i].nNDBPS;
						pstruPhy->MCS = struPhyParameters[i].nIndex - 1;
						break;
					}
				}
				//FIXED_MCS_DATA_RATE
				if (pstruPhy->Fixed_MCS)
				{
					if (pstruPhy->Fixed_Data_Rate_MCS > 8)
						fnNetSimError("\nInvalid MCS %u detected.\n", pstruPhy->Fixed_Data_Rate_MCS);

					pstruPhy->PHY_TYPE.ofdmPhy.dDataRate = struPhyParameters[pstruPhy->Fixed_Data_Rate_MCS].dDataRate;
					pstruPhy->dCurrentRxSensitivity_dbm = struPhyParameters[pstruPhy->Fixed_Data_Rate_MCS].dRxSensitivity;
					pstruPhy->PHY_TYPE.ofdmPhy.modulation = struPhyParameters[pstruPhy->Fixed_Data_Rate_MCS].nModulation;
					pstruPhy->PHY_TYPE.ofdmPhy.dCodingRate = struPhyParameters[pstruPhy->Fixed_Data_Rate_MCS].dCodingRate;
					pstruPhy->PHY_TYPE.ofdmPhy.nNBPSC = struPhyParameters[pstruPhy->Fixed_Data_Rate_MCS].nNBPSC;
					pstruPhy->PHY_TYPE.ofdmPhy.nNCBPS = struPhyParameters[pstruPhy->Fixed_Data_Rate_MCS].nNCBPS;
					pstruPhy->PHY_TYPE.ofdmPhy.nNDBPS = struPhyParameters[pstruPhy->Fixed_Data_Rate_MCS].nNDBPS;
					pstruPhy->MCS = struPhyParameters[pstruPhy->Fixed_Data_Rate_MCS].nIndex - 1;
				}
			}
		}
		else // Broadcast packets
		{
			BROADCAST_RATE:
			pstruPhy->PHY_TYPE.ofdmPhy.dDataRate = struPhyParameters[0].dDataRate;
			pstruPhy->dCurrentRxSensitivity_dbm = struPhyParameters[0].dRxSensitivity;
			pstruPhy->PHY_TYPE.ofdmPhy.modulation = struPhyParameters[0].nModulation;
			pstruPhy->PHY_TYPE.ofdmPhy.dCodingRate = struPhyParameters[0].dCodingRate;
			pstruPhy->PHY_TYPE.ofdmPhy.nNBPSC = struPhyParameters[0].nNBPSC;
			pstruPhy->PHY_TYPE.ofdmPhy.nNCBPS = struPhyParameters[0].nNCBPS;
			pstruPhy->PHY_TYPE.ofdmPhy.nNDBPS = struPhyParameters[0].nNDBPS;		
			pstruPhy->MCS = struPhyParameters[0].nIndex - 1;
		}
	}
	return 0;
}
double ofdmphy_get_min_rxSensitivity(double bandwidth)
{
	if (bandwidth == 20.0)
		return struPhyParameters_20MHz[0].dRxSensitivity;
	else if (bandwidth == 10.0)
		return struPhyParameters_10MHz[0].dRxSensitivity;
	else if (bandwidth==5.0)
		return struPhyParameters_5MHz[0].dRxSensitivity;
	else return NEGATIVE_DBM;
}
