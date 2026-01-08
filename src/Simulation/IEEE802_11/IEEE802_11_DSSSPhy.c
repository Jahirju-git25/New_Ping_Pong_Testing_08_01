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

/// Data structure for physical layer parameters
struct stru_802_11_Phy_Parameters_DSSS
{
	int nIndex;				
	double dRxSensitivity;
	PHY_MODULATION nModulation;
	double dDataRate;
	IEEE802_11_DSSS_PLCP_SIGNAL dsssRate;
	double processingGain;
};

static struct stru_802_11_Phy_Parameters_DSSS struPhyParameters[4] =
{
	//IEEE802.11-DSSS Phy
	{1,-82,Modulation_DBPSK,1,Rate_1Mbps,11},
	{2,-80,Modulation_DQPSK,2,Rate_2Mbps,5.5},
	{3,-78,Modulation_CCK5_5,5.5,Rate_5dot5Mbps,2},
	{4,-76,Modulation_CCK11,11,Rate_11Mbps,1},
};
#define MAX_RATE_INDEX 3
#define MIN_RATE_INDEX 0
#define DEFAULT_BANDWIDTH	20 //MHz

/*
16.4.8.2 Receiver minimum input level sensitivity
The FER shall be less than 8×10–2 at an MPDU length of 1024 octets for an input level of –80 dBm
measured at the antenna connector. This FER shall be specified for 2 Mb/s DQPSK modulation. The test for
the minimum input level sensitivity shall be conducted with the ED threshold set ≤ –80 dBm
*/
#define TARGET_PEP 0.08
#define REF_PACKET_SIZE 1024 //Bytes

unsigned int get_dsss_phy_max_index()
{
	return MAX_RATE_INDEX;
}

unsigned int get_dsss_phy_min_index()
{
	return MIN_RATE_INDEX;
}

void get_dsss_phy_all_rate(double* rate, UINT* len)
{
	int i;
	for (i = MIN_RATE_INDEX; i <= MAX_RATE_INDEX; i++)
		rate[i] = struPhyParameters[i].dDataRate;
	*len = MAX_RATE_INDEX - MIN_RATE_INDEX + 1;
}

int fn_NetSim_IEEE802_11_DSSPhy_UpdateParameter()
{
#ifdef _RECALCULATE_RX_SENSITIVITY_BASED_ON_PEP_
	int i;
	fprintf(stderr, "Calculating receiver sensitivity for DSSS phy\n"
			"Please wait as this may take some time...\n");

	char s[BUFSIZ];
	sprintf(s, "%s\\%s", pszIOLogPath, "rx_sensitivity_wlan_DSSSPhy.csv");
	FILE* fp = fopen(s, "w");
	fprintf(fp, "Bandwidth,NSS,Modulation,Datarate,receiversensitivity\n");

	for (i = MIN_RATE_INDEX; i <= MAX_RATE_INDEX; i++)
	{
		if (struPhyParameters[i].dDataRate > 0)
		{
			struPhyParameters[i].dRxSensitivity = calculate_rxpower_by_per(TARGET_PEP, REF_PACKET_SIZE,
																		   struPhyParameters[i].nModulation,
																		   struPhyParameters[i].dDataRate,
																		   20.0);

			fprintf(fp, "%d,%d,%s,%lf,%lf\n",
					20, 1,
					strPHY_MODULATION[struPhyParameters[i].nModulation],
					struPhyParameters[i].dDataRate, struPhyParameters[i].dRxSensitivity);
			fflush(fp);
		}
	}
	fclose(fp);
#endif //_RECALCULATE_RX_SENSITIVITY_BASED_ON_PEP_
	return 0;
}

void fn_NetSim_IEEE802_11_DSSPhy_SetEDThreshold(PIEEE802_11_PHY_VAR phy)
{
	phy->dEDThreshold = struPhyParameters[0].dRxSensitivity + CSRANGEDIFF;
}

/**
This function is used to calculate the data rate for IEEE 802.11-DSSS Phy
*/
int fn_NetSim_IEEE802_11_DSSSPhy_DataRate(NETSIM_ID nDeviceId, NETSIM_ID nInterfaceId, NETSIM_ID nReceiverId,NetSim_PACKET* packet,double time)
{
	PIEEE802_11_PHY_VAR pstruPhy = IEEE802_11_PHY(nDeviceId,nInterfaceId);
	PIEEE802_11_MAC_VAR mac = IEEE802_11_MAC(nDeviceId,nInterfaceId);

	if(mac->rate_adaptationAlgo== RATEADAPTATION_GENERIC)
	{
		unsigned int index = get_rate_index(nDeviceId,nInterfaceId,nReceiverId);
		if(isIEEE802_11_CtrlPacket(packet))
			goto CONTROL_RATE;
		if(nReceiverId)
		{
			pstruPhy->PHY_TYPE.dsssPhy.dDataRate = struPhyParameters[index].dDataRate;
			pstruPhy->dCurrentRxSensitivity_dbm = struPhyParameters[index].dRxSensitivity;
			pstruPhy->PHY_TYPE.dsssPhy.modulation = struPhyParameters[index].nModulation;
			pstruPhy->PHY_TYPE.dsssPhy.dCodeRate = 1.0;
			pstruPhy->PHY_TYPE.dsssPhy.dsssrate = struPhyParameters[index].dsssRate;
			pstruPhy->PHY_TYPE.dsssPhy.dProcessingGain = struPhyParameters[index].processingGain;
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
			pstruPhy->PHY_TYPE.dsssPhy.dDataRate = struPhyParameters[index].dDataRate;
			pstruPhy->dCurrentRxSensitivity_dbm = struPhyParameters[index].dRxSensitivity;
			pstruPhy->PHY_TYPE.dsssPhy.modulation = struPhyParameters[index].nModulation;
			pstruPhy->PHY_TYPE.dsssPhy.dCodeRate = 1.0;
			pstruPhy->PHY_TYPE.dsssPhy.dsssrate = struPhyParameters[index].dsssRate;
			pstruPhy->PHY_TYPE.dsssPhy.dProcessingGain = struPhyParameters[index].processingGain;
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
				pstruPhy->PHY_TYPE.dsssPhy.dDataRate = struPhyParameters[0].dDataRate; // 1Mbps
				pstruPhy->dCurrentRxSensitivity_dbm = struPhyParameters[0].dRxSensitivity;
				pstruPhy->PHY_TYPE.dsssPhy.modulation = struPhyParameters[0].nModulation;
				pstruPhy->PHY_TYPE.dsssPhy.dCodeRate = 1.0;
				pstruPhy->PHY_TYPE.dsssPhy.dsssrate = struPhyParameters[0].dsssRate;
				pstruPhy->PHY_TYPE.dsssPhy.dProcessingGain = struPhyParameters[0].processingGain;
				pstruPhy->MCS = struPhyParameters[0].nIndex - 1;
			}
			else
			{
				NETSIM_ID in = fn_NetSim_Stack_GetConnectedInterface(nDeviceId, nInterfaceId, nReceiverId);
				if (in == 0)
				{
					fnNetSimError("No link is found between %d:%d-%d\n", nDeviceId, nInterfaceId, nReceiverId);
				}
				double power=GET_RX_POWER_dbm(nDeviceId, mac->parentInterfaceId, nReceiverId, in, time);

				int i;
				for(i=MAX_RATE_INDEX;i>=MIN_RATE_INDEX;i--)
				{
					if(power >= struPhyParameters[i].dRxSensitivity || i==MIN_RATE_INDEX)
					{
						pstruPhy->PHY_TYPE.dsssPhy.dDataRate = struPhyParameters[i].dDataRate;
						pstruPhy->dCurrentRxSensitivity_dbm = struPhyParameters[i].dRxSensitivity;
						pstruPhy->PHY_TYPE.dsssPhy.modulation = struPhyParameters[i].nModulation;
						pstruPhy->PHY_TYPE.dsssPhy.dCodeRate = 1.0;
						pstruPhy->PHY_TYPE.dsssPhy.dsssrate = struPhyParameters[i].dsssRate;
						pstruPhy->PHY_TYPE.dsssPhy.dProcessingGain = struPhyParameters[i].processingGain;
						pstruPhy->MCS = struPhyParameters[i].nIndex - 1;
						break;
					}
				}
				//FIXED_MCS_DATA_RATE
				if (pstruPhy->Fixed_MCS) 
				{
					if (pstruPhy->Fixed_Data_Rate_MCS > 4)
						fnNetSimError("\nInvalid MCS %u detected.\n", pstruPhy->Fixed_Data_Rate_MCS);

					pstruPhy->PHY_TYPE.dsssPhy.dDataRate = struPhyParameters[pstruPhy->Fixed_Data_Rate_MCS].dDataRate;
					pstruPhy->dCurrentRxSensitivity_dbm = struPhyParameters[pstruPhy->Fixed_Data_Rate_MCS].dRxSensitivity;
					pstruPhy->PHY_TYPE.dsssPhy.modulation = struPhyParameters[pstruPhy->Fixed_Data_Rate_MCS].nModulation;
					pstruPhy->PHY_TYPE.dsssPhy.dCodeRate = 1.0;
					pstruPhy->PHY_TYPE.dsssPhy.dsssrate = struPhyParameters[pstruPhy->Fixed_Data_Rate_MCS].dsssRate;
					pstruPhy->PHY_TYPE.dsssPhy.dProcessingGain = struPhyParameters[pstruPhy->Fixed_Data_Rate_MCS].processingGain;
					pstruPhy->MCS = struPhyParameters[pstruPhy->Fixed_Data_Rate_MCS].nIndex - 1;
				}
			}
		}
		else // Broadcast packets
		{
			BROADCAST_RATE:
			pstruPhy->PHY_TYPE.dsssPhy.dDataRate = struPhyParameters[0].dDataRate; // 1Mbps
			pstruPhy->dCurrentRxSensitivity_dbm = struPhyParameters[0].dRxSensitivity;
			pstruPhy->PHY_TYPE.dsssPhy.modulation = struPhyParameters[0].nModulation;
			pstruPhy->PHY_TYPE.dsssPhy.dCodeRate = 1.0;
			pstruPhy->PHY_TYPE.dsssPhy.dsssrate = struPhyParameters[0].dsssRate;
			pstruPhy->PHY_TYPE.dsssPhy.dProcessingGain = struPhyParameters[0].processingGain;		
			pstruPhy->MCS = struPhyParameters[0].nIndex - 1;
		}
	}
	return 0;
}
double DSSSPhy_get_min_rxSensitivity()
{
	return struPhyParameters[0].dRxSensitivity;
}