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
#include "IEEE802_11.h"
#include "IEEE802_11_Phy.h"

/**
This function is called to initialize the MIMO and OFDM parameters relevant to 
IEEE802.11n. 
This function is called from the "fn_NetSim_WLAN_PHY_Configuration()" for HT PHY.
Initialize time parameters as per the Table 20-5—Timing-related constants 802.11n-2009 pdf 
also initialize 802.11n PHY header relevant parameters.
*/
void fn_NetSim_IEEE802_11n_OFDM_MIMO_init(NETSIM_ID nDeviceId,NETSIM_ID nInterfaceId)
{
	UINT nNSTS_MAX = 4;
	UINT nNSS_MAX = 4;
	PIEEE802_11_PHY_VAR phy = IEEE802_11_PHY(nDeviceId,nInterfaceId);

	// Initialize OFDM timing parameters According Table 20-5—Timing-related constants 802.11n-2009 pdf

	if(phy->dChannelBandwidth == 20)
	{
		phy->PHY_TYPE.ofdmPhy_11n.nCH_BANDWIDTH = HT_CBW20;
		phy->PHY_TYPE.ofdmPhy_11n.nNSD = 52; // NSD Number of complex data numbers	
		phy->PHY_TYPE.ofdmPhy_11n.nNSP = 4;	 // NSP Number of pilot values
		phy->PHY_TYPE.ofdmPhy_11n.nNST = 56; // 52 + 4
		phy->PHY_TYPE.ofdmPhy_11n.nNSR = 28; // NSR Highest data subcarrier index
		phy->PHY_TYPE.ofdmPhy_11n.nFFT = 64; // FFT length	
	}
	else
	{
		phy->PHY_TYPE.ofdmPhy_11n.nCH_BANDWIDTH = HT_CBW40;
		phy->PHY_TYPE.ofdmPhy_11n.nNSD = 108; // NSD Number of complex data numbers	
		phy->PHY_TYPE.ofdmPhy_11n.nNSP = 6;	  // NSP Number of pilot values
		phy->PHY_TYPE.ofdmPhy_11n.nNST = 114; // 108 + 6
		phy->PHY_TYPE.ofdmPhy_11n.nNSR = 58;  // NSR Highest data subcarrier index
		phy->PHY_TYPE.ofdmPhy_11n.nFFT = 128; // FFT length
	}

	// Assign all the timing relevant parameters
	phy->PHY_TYPE.ofdmPhy_11n.dDeltaF = phy->dChannelBandwidth/ (phy->PHY_TYPE.ofdmPhy_11n.nFFT * 1.0);//SubcarrierFrequencySpacing 312.5KHz (20 MHz/64) or (40 MHz/128)
	phy->PHY_TYPE.ofdmPhy_11n.dTDFT = 3.2;  // IDFT/DFT period 3.2 MicroSecs
	phy->PHY_TYPE.ofdmPhy_11n.dTGI = phy->PHY_TYPE.ofdmPhy_11n.dTDFT/4; // 3.2/4 = 0.8 MicroSecs  Guard interval duration 0.8 = TDFT/4  
	phy->PHY_TYPE.ofdmPhy_11n.dTGI2 = phy->PHY_TYPE.ofdmPhy_11n.dTGI * 2; // 0.8 * 2 = 1.6 MicroSecs Double guard interval 1.6 
	phy->PHY_TYPE.ofdmPhy_11n.dTGIS = phy->PHY_TYPE.ofdmPhy_11n.dTDFT/8; // Short guard interval duration   dDFT/8 = 0.4 
	phy->PHY_TYPE.ofdmPhy_11n.nTL_STF = 8;     // Non-HT short training sequence duration 8 = 10*TDFT/4 
	phy->PHY_TYPE.ofdmPhy_11n.nTHT_GF_STF = 8; // HT-greenfield short training field duration 8 =10* TDFT/4 
	phy->PHY_TYPE.ofdmPhy_11n.nTL_LTF = 8;     // Non-HT long training field duration 8 =2 * TDFT + TGI2 
	phy->PHY_TYPE.ofdmPhy_11n.dTSYM  = 4.0;   //TSYM: Symbol interval 4 =  TDFT + TGI       
	phy->PHY_TYPE.ofdmPhy_11n.dTSYMS = 3.6;  // TSYMS: Short GI symbol interval = TDFT+TGIS 3.6 
	phy->PHY_TYPE.ofdmPhy_11n.nTL_SIG = 4;   // Non-HT SIGNAL field duration 4  
	phy->PHY_TYPE.ofdmPhy_11n.nTHT_SIG = 8;  // HT SIGNAL field duration 8 = 2TSYM 8 
	phy->PHY_TYPE.ofdmPhy_11n.nTHT_STF = 4;  // HT short training field duration  4 
	phy->PHY_TYPE.ofdmPhy_11n.nTHT_LTF1 = 8; // First HT long training field duration 4 in HT-mixed format, 8 in HTgreenfield format
	phy->PHY_TYPE.ofdmPhy_11n.nTHT_LTFs = 4; // Second, and subsequent, HT long training fields

	if(phy->nGuardInterval == 400)
		phy->PHY_TYPE.ofdmPhy_11n.nGI_TYPE = SHORT_GI;
	else
		phy->PHY_TYPE.ofdmPhy_11n.nGI_TYPE = LONG_GI;

	// Set the STBC coding
	phy->PHY_TYPE.ofdmPhy_11n.nSTBC = 0;
	// Check and Set NSS value, it should be minimum of transmit and received antenna count
	phy->NSS = min((phy->nNTX < phy->nNRX ? phy->nNTX: phy->nNRX),nNSS_MAX);
	
	// Set NSTS value 
	phy->NSTS = min(phy->NSS  + phy->PHY_TYPE.ofdmPhy_11n.nSTBC,nNSTS_MAX);

	// Set  HT_DLTFs  Table 20-12—Number of HT-DLTFs required for data space-time streams
	phy->N_HT_DLTF = phy->NSTS;
	if(phy->NSTS == 3) 
		phy->N_HT_DLTF = 4;

	// Set NESS   NESS + NSTS <= 4
	phy->NESS =  nNSTS_MAX - phy->NSTS;
	
	// Set HT_ELTFs Table 20-13—Number of HT-ELTFs required for extension spatial streams
	phy->N_HT_ELTF = phy->NESS;
	if(phy->NESS == 3)
		phy->N_HT_ELTF = 4;
	
	// Set total HT_LTF = HT_DLTF + HT_ELTF
	phy->N_HT_LTF = phy->N_HT_DLTF + phy->N_HT_ELTF;
	if(phy->N_HT_LTF > 5) // Max is 5
		phy->N_HT_LTF = 5;

	phy->dControlFrameDataRate = fn_NetSim_IEEE802_11_HTPhy_getCtrlFrameDataRate(phy);
	phy->dBroadcastFrameDataRate= fn_NetSim_IEEE802_11_HTPhy_getCtrlFrameDataRate(phy);
}

double get_11n_preamble_time(PIEEE802_11_PHY_VAR phy)
{
	double dTGF_HT_PREAMBLE;
	int nSignalExtension = 0;

	dTGF_HT_PREAMBLE = phy->PHY_TYPE.ofdmPhy_11n.nTHT_GF_STF \
		+ phy->PHY_TYPE.ofdmPhy_11n.nTHT_LTF1 \
		+ ((phy->N_HT_LTF - 1) * phy->PHY_TYPE.ofdmPhy_11n.nTHT_LTFs);

	return dTGF_HT_PREAMBLE + 
		phy->PHY_TYPE.ofdmPhy_11n.nTHT_SIG +
		nSignalExtension;
}
