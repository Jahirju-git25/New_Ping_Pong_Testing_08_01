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
#ifndef _NETSIM_IEEE802_11_PHY_H_
#define _NETSIM_IEEE802_11_PHY_H_
#ifdef  __cplusplus
extern "C" {
#endif

#include "IEEE802_11.h"
#include "ErrorModel.h"

	//Default Battery config parameter
#define BATTERY_RECHARGING_CURRENT_MA_DEFAULT	0
#define BATTERY_VOLTAGE_V_DEFAULT				0
#define BATTERY_INITIAL_ENERGY_DEFAULT			0
#define BATTERY_TRANSMITTING_CURRENT_MA_DEFAULT	0
#define BATTERY_RECEIVING_CURRENT_MA_DEFAULT	0
#define BATTERY_IDLEMODE_CURRENT_MA_DEFAULT		0
#define BATTERY_SLEEPMODE_CURRENT_MA_DEFAULT	0

#define CSRANGEDIFF -3 //db

#include "IEEE802_11_HTPhy.h"

	typedef enum
	{
		FHSS_2_4_GHz = 01,
		DSSS_2_4_GHz = 02,
		IR_Baseband = 03,
		OFDM = 04,
		DSSS = 05,
		ERP = 06,
		HT = 07,
		VHT = 8,
	}IEEE802_11_PHY_TYPE;

	static const char* strIEEE802_11_ANTENNA_TYPE[] =
	{ "Omnidirectional Antenna", "Sector Antenna", };

	/// Enumeration to represent the DSSS rate	
	typedef enum enum_802_11_DSSS_PLCP_SIGNAL_Field	
	{
		Rate_1Mbps = 0x0A,
		Rate_2Mbps = 0x14,
		Rate_5dot5Mbps = 0x37,
		Rate_11Mbps = 0x6E,
	}IEEE802_11_DSSS_PLCP_SIGNAL;

	typedef struct stru_IEEE802_11_DSSS_Phy
	{
		double dDataRate;
		PHY_MODULATION modulation;
		double dCodeRate;
		IEEE802_11_DSSS_PLCP_SIGNAL dsssrate;
		double dProcessingGain;
	}IEEE802_11_DSSS_PHY,*PIEEE802_11_DSSS_PHY;

	typedef struct stru_IEEE802_11_OFDM_Phy
	{
		double dDataRate;
		double dCodingRate;
		PHY_MODULATION modulation;
		int nNBPSC;
		int nNCBPS;
		int nNDBPS;
	}IEEE802_11_OFDM_PHY,*PIEEE802_11_OFDM_PHY;

	/**
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	********** PHY layer ************
	Page-1514 IEEE Std 802.11 2012 Table 16-2 DS PHY characteristics
	Page-1623 IEEE Std 802.11-2012 Table 18-17—OFDM PHY characteristics
	Page-361 6.5.4.2 Semantics of the service primitive
	PLME-CHARACTERISTICS
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	*/
	typedef struct stru_802_11_PHY_PLME_characteristics
	{
		UINT aSlotTime;
		UINT aSIFSTime;
		UINT aCCATime;
		UINT aPHY_RX_START_Delay;
		UINT aRxTxTurnaroundTime;
		UINT aTxPLCPDelay;
		UINT aRxPLCPDelay;
		UINT aRxTxSwitchTime;
		UINT aTxRampOnTime;
		UINT aTxRampOffTime;
		UINT aTxRFDelay;
		UINT aRxRFDelay;
		UINT aAirPropagationTime;
		UINT aMACProcessingDelay;
		UINT aPreambleLength;
		UINT aPLCPHeaderLength;
		UINT aMPDUDurationFactor;
		UINT aMPDUMaxLength;
		UINT aCWmin;
		UINT aCWmax;
	}PLME_CHARACTERISTICS,*PPLME_CHARACTERISTICS;
#define getSlotTime(phy) (phy->plmeCharacteristics.aSlotTime)
#define getSIFSTime(phy) (phy->plmeCharacteristics.aSIFSTime)

	typedef struct stru_802_11_phy_radio
	{
		PHY_TX_STATUS radioState;
		NETSIM_ID peerId;
		UINT64 transmissionId;
		UINT64 eventId;
	}IEEE802_11_RADIO, *ptrIEEE802_11_RADIO;

	typedef struct stru_802_11_Phy_Var
	{
		//Config variable
		UINT nTxPower_mw;
		IEEE802_11_PROTOCOL PhyProtocol;
		IEEE802_11_PHY_TYPE PhyType;
		UINT16 usnChannelNumber;
		double dFrequency;
		double dFrequencyBand;
		double dChannelBandwidth;
		double dAntennaHeight;
		double dAntennaGain;

		//For HT/VHT config variable
		UINT nNTX;
		UINT nNRX;
		UINT nGuardInterval;

		// Scetor Antenna Variables 
		ANTENNA_TYPE antennaType;
		double boresightAngle;
		double elementGain;
		double beamwidth;
		double frontToBackRatio;

		PLME_CHARACTERISTICS plmeCharacteristics;

		//Rate adaptation
		void* rateAdaptationData;

		//Data Rate
		double dControlFrameDataRate;
		double dBroadcastFrameDataRate;

		//CCA
		IEEE802_11_CCAMODE ccaMode;
		double dEDThreshold;

		//Radio
		IEEE802_11_RADIO radio;

		//IFS
		UINT RIFS;
		UINT SIFS;
		UINT PIFS;
		UINT DIFS;
		UINT AIFS;
		UINT EIFS;

		//Power
		double dCurrentRxSensitivity_dbm;

		//Phy type
		union{
			IEEE802_11_DSSS_PHY dsssPhy;
			IEEE802_11_OFDM_PHY ofdmPhy;
			IEEE802_11_OFDM_MIMO ofdmPhy_11n;
			IEEE802_11_OFDM_MIMO ofdmPhy_11ac;
		}PHY_TYPE;

		//MIMO
		double codingRate;
		unsigned int NSS; // Number of spatial streams
		unsigned int NSTS; // Number of space-time streams 
		unsigned int N_HT_DLTF; // Number of Data HT Long Training fields
		unsigned int NESS; // Number of extension spatial streams
		unsigned short int nN_HT_DLTF;	// Number of Data HT Long Training fields
		unsigned int N_HT_ELTF; // Number of Extension HT Long Training fields
		unsigned int N_HT_LTF; // Number of HT Long Training fields (see 20.3.9.4.6)
		unsigned int MCS;
		unsigned int NES; // Number of BCC encoders for the Data field

		PACKET_STATUS firstpacketstatus;

		double d0;
		double pld0;

		//Battery
		bool isDeviceOn;
		void* battery;

		//BER-Model
		BER_MODEL_TYPE Ber_Model;

		//Fixed MCS
		bool Fixed_MCS;
		int Fixed_Data_Rate_MCS;

		ptrpropagation_info_for_list propagation_info_list;

		UINT64 transmissionId;
	}IEEE802_PHY_VAR,*PIEEE802_11_PHY_VAR;

#define IEEE802_11_CURR_PHY IEEE802_11_PHY(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId)

	//Propagation macro
	double GET_RX_POWER_dbm(NETSIM_ID tx, NETSIM_ID txi, NETSIM_ID rx, NETSIM_ID rxi, double time);
#define GET_RX_POWER_mw(tx, txi, rx, rxi, time) (DBM_TO_MW(GET_RX_POWER_dbm(tx, txi, rx, rxi, time)))

	//Function Prototype
	int fn_NetSim_IEEE802_11_PHY_Init(NETSIM_ID nDeviceId,NETSIM_ID nInterfaceId);
	bool isMediumIdle(NETSIM_ID, NETSIM_ID);
	void fn_NetSim_IEEE802_11_Add_Phy_Header(NetSim_PACKET* packet, UINT64* transmissionId);
	double get_preamble_time(PIEEE802_11_PHY_VAR phy);
	int fn_NetSim_IEEE802_11_PhyOut();
	int fn_NetSim_IEEE802_11_PhyIn();
	bool fn_NetSim_IEEE802_11_TransmitFrame(NetSim_PACKET* pstruPacket, NETSIM_ID nDevId, NETSIM_ID nInterfaceId);
	bool fn_NetSim_IEEE802_11_TransmitBroadcastFrame(NetSim_PACKET* pstruPacket, NETSIM_ID nDevId, NETSIM_ID nInterfaceId);
	int fn_NetSim_IEEE802_11_SetDataRate(NETSIM_ID txId, NETSIM_ID txIf,
										 NETSIM_ID rxId, NETSIM_ID rxIf,
										 NetSim_PACKET* packet, double time);
	void free_ieee802_11_phy_header(NetSim_PACKET* packet);
	void copy_ieee802_11_phy_header(NetSim_PACKET* d, NetSim_PACKET* s);
	int fn_NetSim_IEEE802_11_PropagationModel(NETSIM_ID nodeId, double time);
	void fn_Update_TxInfo_TxRxPos(NETSIM_ID devId);
	double fn_NetSim_IEEE802_11_CalculateTransmissionTime(double size, NETSIM_ID nDevId, NETSIM_ID nInterfaceId);
	double fn_NetSim_IEEE802_11_HTPhy_getCtrlFrameDataRate(PIEEE802_11_PHY_VAR pstruPhy);
	double fn_NetSim_IEEE802_11_GetMinRxSensitivity(NETSIM_ID txId, NETSIM_ID txIf);

	//Radio
	bool is_radio_idle(PIEEE802_11_PHY_VAR phy);
	bool set_radio_state(NETSIM_ID d,
						 NETSIM_ID in,
						 PHY_TX_STATUS state,
						 NETSIM_ID peerId,
						 UINT64 transmissionId);
	PHY_TX_STATUS get_radio_state(PIEEE802_11_PHY_VAR phy);
	void set_mac_state_after_txend(PIEEE802_11_MAC_VAR mac);
	bool CheckFrequencyInterfrence(double dFrequency1,double dFrequency2,double bandwidth);

	//IEEE802.11b
	void fn_NetSim_IEEE802_11_DSSPhy_SetEDThreshold(PIEEE802_11_PHY_VAR phy);
	int fn_NetSim_IEEE802_11_DSSPhy_UpdateParameter();
	int fn_NetSim_IEEE802_11_DSSSPhy_DataRate(NETSIM_ID nDeviceId, NETSIM_ID nInterfaceId, NETSIM_ID nReceiverId,NetSim_PACKET* packet,double time);
	unsigned int get_dsss_phy_max_index();
	unsigned int get_dsss_phy_min_index();
	void get_dsss_phy_all_rate(double* rate, UINT* len);

	//IEEE802.11a/g/p
	void fn_NetSim_IEEE802_11_OFDMPhy_SetEDThreshold(PIEEE802_11_PHY_VAR phy);
	int fn_NetSim_IEEE802_11_OFDMPhy_UpdateParameter();
	int fn_NetSim_IEEE802_11_OFDMPhy_DataRate(NETSIM_ID nDeviceId, NETSIM_ID nInterfaceId, NETSIM_ID nReceiverId,NetSim_PACKET* packet,double time);
	unsigned int get_ofdm_phy_max_index();
	unsigned int get_ofdm_phy_min_index();
	void get_ofdm_phy_all_rate(int bandwidth_MHz, double* rate, UINT* len);

	//IEEE802.11n
	void fn_NetSim_IEEE802_11_HTPhy_SetEDThreshold(PIEEE802_11_PHY_VAR phy);
	int fn_NetSim_IEEE802_11_HTPhy_UpdateParameter();
	void fn_NetSim_IEEE802_11n_OFDM_MIMO_init(NETSIM_ID nDeviceId,NETSIM_ID nInterfaceId);
	double get_11n_preamble_time(PIEEE802_11_PHY_VAR phy);
	double fn_NetSim_IEEE802_11n_TxTimeCalculation(NetSim_PACKET *pstruPacket, NETSIM_ID nDevId, NETSIM_ID nInterfaceId);
	int fn_NetSim_IEEE802_11_HTPhy_DataRate(NETSIM_ID nDeviceId, NETSIM_ID nInterfaceId, NETSIM_ID nReceiverId,NetSim_PACKET* packet,double time);
	unsigned int get_ht_phy_max_index(IEEE802_11_PROTOCOL protocol,UINT);
	unsigned int get_ht_phy_min_index(IEEE802_11_PROTOCOL protocol,UINT);

	//IEEE802.11ac
	void fn_NetSim_IEEE802_11ac_OFDM_MIMO_init(NETSIM_ID nDeviceId,NETSIM_ID nInterfaceId);
	double get_11ac_preamble_time(PIEEE802_11_PHY_VAR phy);
	double fn_NetSim_IEEE802_11ac_TxTimeCalculation(NetSim_PACKET *pstruPacket, NETSIM_ID nDevId, NETSIM_ID nInterfaceId);
	struct stru_802_11_Phy_Parameters_HT* get_phy_parameter_HT(double dChannelBandwidth,UINT NSS);

	//IEEE1609 Interface
	PIEEE802_11_PHY_VAR IEEE802_11_PHY(NETSIM_ID ndeviceId, NETSIM_ID nInterfaceId);
	void SET_IEEE802_11_PHY(NETSIM_ID ndeviceId, NETSIM_ID nInterfaceId, PIEEE802_11_PHY_VAR phy);

	//IEEE802_11 Radio Measurements
	void IEEE_802_11RadioMeasurements_Init();
	void IEEE_802_11RadioMeasurements_Finish();
	void IEEE_802_11RadioMeasurements_Log(NetSim_PACKET* packet, NETSIM_ID txid, NETSIM_ID txif, NETSIM_ID rxid, NETSIM_ID rxif);
#ifdef  __cplusplus
}
#endif
#endif //_NETSIM_IEEE802_11_PHY_H_