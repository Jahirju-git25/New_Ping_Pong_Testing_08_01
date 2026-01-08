#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "main.h"
#include "Wireless.h"
#include "ErrorModel.h"

// Data structure for physical layer parameters
struct stru_802_11_BER_Parameters
{
	int MCS;
	double SINR;
};

//MCS-SINR Table 802.11b 20MHz
static struct stru_802_11_BER_Parameters struPhyParameters_802_11b_20MHz[4] =
{
	//Below the SINR Value 4 dB, PEP = 1
	//{MCS, SINR}
	{0, 4},
	{1, 7},
	{2, 11},
	{3, 16}
};

//MCS-SINR Table 802.11agp 20MHz
static struct stru_802_11_BER_Parameters struPhyParameters_802_11ag_20MHz[8] =
{
	{0, 2},
	{1, 4},
	{2, 5},
	{3, 9},
	{4, 11},
	{5, 15},
	{6, 18},
	{7, 20}
};

//MCS-SINR Table 802.11n 20MHz
static struct stru_802_11_BER_Parameters struPhyParameters_802_11n_20MHz[8] =
{
	{0, 2},
	{1, 5},
	{2, 9},
	{3, 11},
	{4, 15},
	{5, 18},
	{6, 20},
	{7, 25}
};

//MCS-SINR Table 802.11n 40MHz
static struct stru_802_11_BER_Parameters struPhyParameters_802_11n_40MHz[8] =
{
	{0, 5},
	{1, 8},
	{2, 12},
	{3, 14},
	{4, 18},
	{5, 21},
	{6, 23},
	{7, 28}
};

//MCS-SINR Table 802.11ac 20MHz
static struct stru_802_11_BER_Parameters struPhyParameters_802_11ac_20MHz[10] = {
	{0, 2},
	{1, 5},
	{2, 9},
	{3, 11},
	{4, 15},
	{5, 18},
	{6, 20},
	{7, 25},
	{8, 29},
	{9, 31}
};

//MCS-SINR Table 802.11ac 40MHz
static struct stru_802_11_BER_Parameters struPhyParameters_802_11ac_40MHz[10] = {
	{0, 5},
	{1, 8},
	{2, 12},
	{3, 14},
	{4, 18},
	{5, 21},
	{6, 23},
	{7, 28},
	{8, 32},
	{9, 34}
};

//MCS-SINR Table 802.11ac 80MHz
static struct stru_802_11_BER_Parameters struPhyParameters_802_11ac_80MHz[10] = {
	{0, 8},
	{1, 11},
	{2, 15},
	{3, 17},
	{4, 21},
	{5, 24},
	{6, 26},
	{7, 31},
	{8, 35},
	{9, 37}
};

//MCS-SINR Table 802.11ac 160MHz
static struct stru_802_11_BER_Parameters struPhyParameters_802_11ac_160MHz[10] = {
	{0, 11},
	{1, 14},
	{2, 18},
	{3, 20},
	{4, 24},
	{5, 27},
	{6, 29},
	{7, 34},
	{8, 38},
	{9, 40}
};



double GET_BER_802_11_n(double sinr, double dBandwidth_MHz, int MCS)
{
	double BER = 1; // Default PEP value

	switch ((int)dBandwidth_MHz)
	{
	case 20:
		if (MCS >= 0 && MCS < sizeof(struPhyParameters_802_11n_20MHz))
		{
			if (sinr >= struPhyParameters_802_11n_20MHz[MCS].SINR)
			{
				BER = 0;
			}
		}
		break;
	case 40:
		if (MCS >= 0 && MCS < sizeof(struPhyParameters_802_11n_40MHz))
		{
			if (sinr >= struPhyParameters_802_11n_40MHz[MCS].SINR)
			{
				BER = 0;
			}
		}
		break;
		default:
			fnNetSimError("Unknown channel bandwidth %lf in %s\n", dBandwidth_MHz, __FUNCTION__);
			break;
	}
	return BER;
}

double GET_BER_802_11_ac(double sinr, double dBandwidth_MHz, int MCS)
{
	double BER = 1; // Default PEP value
	switch ((int)dBandwidth_MHz)
	{
	case 20:
		if (MCS >= 0 && MCS < sizeof(struPhyParameters_802_11ac_20MHz))
		{
			if (sinr >= struPhyParameters_802_11ac_20MHz[MCS].SINR)
			{
				BER = 0;
			}
		}
		break;
	case 40:
			if (MCS >= 0 && MCS < sizeof(struPhyParameters_802_11ac_40MHz))
		{
				if (sinr >= struPhyParameters_802_11ac_40MHz[MCS].SINR)
			{
				BER = 0;
			}
		}
		break;
	case 80:
			if (MCS >= 0 && MCS < sizeof(struPhyParameters_802_11ac_80MHz))
		{
				if (sinr >= struPhyParameters_802_11ac_80MHz[MCS].SINR)
			{
				BER = 0;
			}
		}
		break;
	case 160:
			if (MCS >= 0 && MCS < sizeof(struPhyParameters_802_11ac_160MHz))
		{
				if (sinr >= struPhyParameters_802_11ac_160MHz[MCS].SINR)
			{
				BER = 0;
			}
		}
		break;
		default:
			fnNetSimError("Unknown channel bandwidth %lf in %s\n", dBandwidth_MHz, __FUNCTION__);
			break;
	}
	return BER;
}

double GET_BER(double sinr, int MCS, double dBandwidth_MHz, string protocol)
{
	double BER = 1; // Default PEP value
	if (protocol)
	{
		if (!_stricmp(protocol, "IEEE_802_11b")) 
		{
			if (MCS >= 0 && MCS < sizeof(struPhyParameters_802_11b_20MHz))
			{
				if (sinr >= struPhyParameters_802_11b_20MHz[MCS].SINR)
				{
					BER = 0;
				}
			}
		}
		else if ((!_stricmp(protocol, "IEEE_802_11a")) || (!_stricmp(protocol, "IEEE_802_11p")) || (!_stricmp(protocol, "IEEE_802_11g")))
		{
			if (MCS >= 0 && MCS < sizeof(struPhyParameters_802_11ag_20MHz))
			{
				if (sinr >= struPhyParameters_802_11ag_20MHz[MCS].SINR)
				{
					BER = 0;
				}
			}
		}
			else if (!_stricmp(protocol, "IEEE_802_11n"))
				BER = GET_BER_802_11_n(sinr, dBandwidth_MHz, MCS);
			else if (!_stricmp(protocol, "IEEE_802_11ac"))
				BER = GET_BER_802_11_ac(sinr, dBandwidth_MHz, MCS);
		else
		{
			NetSimxmlError("WLAN---PHY Protocol Configuration Unknown PHY STANDARD", NULL,NULL);
		}
	}
	return BER;
}

/**
Function to calculate the PEP using SNR-PEP Table
*/
_declspec(dllexport) double calculate_BER_Using_Table(PHY_MODULATION modulation,
	double dReceivedPower_dBm,
	double dInterferencePower_dBm,
	double dBandwidth_MHz,
	int MCS,
	string Protocol)
{
	double ldEbByN0 = calculate_sinr(dReceivedPower_dBm, dInterferencePower_dBm, dBandwidth_MHz);
	double BER = 0.0;
	BER = GET_BER(ldEbByN0, MCS, dBandwidth_MHz, Protocol);
	return BER;
}