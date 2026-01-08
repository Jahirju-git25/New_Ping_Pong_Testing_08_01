/************************************************************************************
 * Copyright (C) 2023                                                               *
 * TETCOS, Bangalore. India                                                         *
 *                                                                                  *
 * Tetcos owns the intellectual property rights in the Product and its content.     *
 * The copying, redistribution, reselling or publication of any or all of the       *
 * Product or its content without express prior written consent of Tetcos is        *
 * prohibited. Ownership and / or any other right relating to the software and all  *
 * intellectual property rights therein shall remain at all times with Tetcos.      *
 *                                                                                  *
 * Author:    Shashi Kant Suman                                                     *
 *                                                                                  *
 * ---------------------------------------------------------------------------------*/
#include "main.h"
#include "Aloha.h"
#include "../UWAN/UWAN.h"
#include "AdvancedPlots.h"
#include "../BatteryModel/BatteryModel.h"
#pragma comment(lib,"AdvancedPlots.lib")

void send_to_phy();
void wait_for_random_time();
int fn_NetSim_Aloha_handleMobility(NETSIM_ID d);

PHY_MODULATION getModulationFromString(char* val)
{
	if(!_stricmp(val,"QPSK"))
		return Modulation_QPSK;
	else if(!_stricmp(val,"BPSK"))
		return Modulation_BPSK;
	else if (!_stricmp(val, "FSK"))
		return Modulation_FSK;
	else if(!_stricmp(val,"16QAM"))
		return Modulation_16_QAM;
	else if(!_stricmp(val,"64QAM"))
		return Modulation_64_QAM;
	else if (!_stricmp(val, "256QAM"))
		return Modulation_256_QAM;
	else
	{
		fnNetSimError("Unknown modulation %s. Assigning QPSK\n",val);
		return Modulation_QPSK;
	}
}

_declspec(dllexport) int fn_NetSim_Aloha_Configure(void** var)
{
	void* xmlNetSimNode;
	NETSIM_ID nDeviceId;
	NETSIM_ID nInterfaceId;
	LAYER_TYPE nLayerType;
	xmlNetSimNode = var[2];
	nDeviceId = *((NETSIM_ID*)var[3]);
	nInterfaceId = *((NETSIM_ID*)var[4]);
	nLayerType = *((LAYER_TYPE*)var[5]);
	switch(nLayerType)
	{	
	case PHYSICAL_LAYER:
		{
			PALOHA_PHY_VAR phy = ALOHA_PHY(nDeviceId,nInterfaceId);
			char* szVal;

			if(!phy)
			{
				phy = (PALOHA_PHY_VAR)calloc(1,sizeof *phy);
				DEVICE_PHYVAR(nDeviceId,nInterfaceId) = phy;
			}

			phy->frequency = fn_NetSim_Config_read_Frequency(xmlNetSimNode, "FREQUENCY",
															 ALOHA_FREQUENCY_DEFAULT, "MHZ");

			phy->bandwidth = fn_NetSim_Config_read_Frequency(xmlNetSimNode, "BANDWIDTH",
															 ALOHA_BANDWIDTH_DEFAULT, "MHz");

			phy->tx_power = fn_NetSim_Config_read_txPower(xmlNetSimNode, "TX_POWER",
														  ALOHA_TX_POWER_DEFAULT, "MW");

			getXmlVar(&phy->rx_sensitivity,RECEIVER_SENSITIVITY_DBM,xmlNetSimNode,1, _DOUBLE,ALOHA);
			getXmlVar(&szVal,MODULATION,xmlNetSimNode,1,_STRING,ALOHA);
			phy->modulation = getModulationFromString(szVal);
			free(szVal);
			phy->data_rate = fn_NetSim_Config_read_dataRate(xmlNetSimNode, "DATA_RATE",
															ALOHA_DATA_RATE_DEFAULT, "MBPS");

			void* pXmlChild = fn_NetSim_xmlGetChildElement(xmlNetSimNode, "POWER", 0);
			if (pXmlChild)
			{
				fn_NetSim_Configure_UWAN_POWER(xmlNetSimNode, nDeviceId,nInterfaceId);
				pXmlChild = NULL;
			}
		}
		break;
	case MAC_LAYER:
		{
			PALOHA_MAC_VAR mac = ALOHA_MAC(nDeviceId,nInterfaceId);
			char* szVal;

			if(!mac)
			{
				mac = (PALOHA_MAC_VAR)calloc(1,sizeof *mac);
				DEVICE_MACVAR(nDeviceId,nInterfaceId) = mac;
			}

			//Get the MAC address
			szVal = fn_NetSim_xmlConfig_GetVal(xmlNetSimNode,"MAC_ADDRESS",1);
			if(szVal)
			{
				NETWORK->ppstruDeviceList[nDeviceId-1]->ppstruInterfaceList[nInterfaceId-1]->pstruMACLayer->szMacAddress = STR_TO_MAC(szVal);
				free(szVal);
			}

			getXmlVar(&mac->slotlength,SLOT_LENGTH,xmlNetSimNode,0, _DOUBLE,ALOHA);

			getXmlVar(&mac->retryLimit, RETRY_LIMIT, xmlNetSimNode, 1, _UINT, ALOHA);

			getXmlVar(&mac->isMACBuffer, IS_MAC_BUFFER, xmlNetSimNode, 1, _BOOL, ALOHA);
		}
		break;
	default:
		fnNetSimError("%d layer is not implemented for Aloha protocol\n",nLayerType);
		break;
	}
	return 0;
}

_declspec (dllexport) int fn_NetSim_Aloha_Init()
{
	NETSIM_ID i;
	init_UWAN_log();
	init_linkpacketlog();
	fn_NetSim_Aloha_CalulateReceivedPower();
	fnMobilityRegisterCallBackFunction(fn_NetSim_Aloha_handleMobility);
	for (i = 0; i < NETWORK->nDeviceCount; i++)
	{
		PALOHA_MAC_VAR mac = ALOHA_MAC(i + 1, 1);
		PALOHA_PHY_VAR phy = ALOHA_PHY(i + 1, 1);
		if (mac->slotlength)
		{
			mac->isSlotted = true;
			//double t = get_slot_length(phy);
			//mac->slotlength = max(t, mac->slotlength);
		}
		else
		{
			mac->slotlength = get_slot_length(phy);
		}


		phy->transmitter_status = RX_ON_IDLE;
		if(phy->battery)
			battery_set_mode(phy->battery, RX_ON_IDLE, ldEventTime);
		phy->dInterferencePower = MW_TO_DBM(0);
	}
	return 0;
}

_declspec (dllexport) int fn_NetSim_Aloha_Run()
{
	switch(pstruEventDetails->nEventType)
	{
	case MAC_OUT_EVENT:
		fn_NetSim_Aloha_Handle_MacOut();
		break;
	case MAC_IN_EVENT:
		pstruEventDetails->nEventType = NETWORK_IN_EVENT;
		fnpAddEvent(pstruEventDetails);
		break;
	case PHYSICAL_OUT_EVENT:
		fn_NetSim_Aloha_PhyOut();
		break;
	case PHYSICAL_IN_EVENT:
		fn_NetSim_Aloha_PhyIn();
		break;
	default:
		fnNetSimError("Unknown event type %d for aloha protocol in %s",pstruEventDetails->nEventType,__FUNCTION__);
		break;
	}
	return 0;
}

_declspec(dllexport) int fn_NetSim_Aloha_Finish()
{
	close_UWAN_log();
	LinkPacketLog_close();
	fn_NetSim_Aloha_FreePropagationInfo();
	return 0;
}

#include "Aloha_enum.h"
_declspec (dllexport) char* fn_NetSim_Aloha_Trace(int nSubEvent)
{
	return GetStringALOHA_Subevent(nSubEvent);
}
_declspec(dllexport) int fn_NetSim_Aloha_FreePacket(NetSim_PACKET* pstruPacket)
{
	pstruPacket;
	return 0;
}

/**
	This function is called by NetworkStack.dll, to copy the aloha protocol
	details from source packet to destination.
*/
_declspec(dllexport) int fn_NetSim_Aloha_CopyPacket(NetSim_PACKET* pstruDestPacket,NetSim_PACKET* pstruSrcPacket)
{
	pstruDestPacket;
	pstruSrcPacket;
	return 0;
}

/**
This function write the Metrics 	
*/
_declspec(dllexport) int fn_NetSim_Aloha_Metrics(PMETRICSWRITER metricsWriter)
{
	metricsWriter;
	battery_metrics(metricsWriter);
	return 0;
}

/**
This function will return the string to write packet trace heading.
*/
_declspec(dllexport) char* fn_NetSim_Aloha_ConfigPacketTrace()
{
	return "";
}

/**
 This function will return the string to write packet trace.																									
*/
_declspec(dllexport) char* fn_NetSim_Aloha_WritePacketTrace(NetSim_PACKET* pstruPacket, char** ppszTrace)
{
	ppszTrace;
	pstruPacket;
	return "";
}

int fn_NetSim_Configure_UWAN_POWER(void* xmlNetSimNode, int nDeviceId, int nInterfaceId)
{
	char* szVal;
	void* pXmlChild;
	pXmlChild = fn_NetSim_xmlGetChildElement(xmlNetSimNode, "POWER", 0);
	szVal = fn_NetSim_xmlConfig_GetVal(pXmlChild, "SOURCE", 1);
	if (szVal)
	{
		if (_stricmp(szVal, "Battery"))
		{
			free(szVal);
			return -1;
		}
		free(szVal);
	}
	PALOHA_PHY_VAR phy = ALOHA_PHY(nDeviceId, nInterfaceId);
	double dRechargingCurrent = 0;
	double dVoltage = 0;
	double dInitialEnergy = 0;
	double dCurrent;
	getXmlVar(&dRechargingCurrent, RECHARGING_CURRENT_MA, pXmlChild, 0, _DOUBLE, BATTERY);
	getXmlVar(&dVoltage, VOLTAGE_V, pXmlChild, 0, _DOUBLE, BATTERY);
	getXmlVar(&dInitialEnergy, INITIAL_ENERGY, pXmlChild, 0, _DOUBLE, BATTERY);
	dInitialEnergy *= dVoltage * 3600;
	phy->battery = battery_init_new(nDeviceId,
		0,
		dInitialEnergy,
		dVoltage,
		dRechargingCurrent);

	getXmlVar(&dCurrent, TRANSMITTING_CURRENT_MA, pXmlChild, 0, _DOUBLE, BATTERY);
	battery_add_new_mode(phy->battery, TRX_ON_BUSY, dCurrent, "Transmitting energy(mJ)");

	getXmlVar(&dCurrent, RECEIVING_CURRENT_MA, pXmlChild, 0, _DOUBLE, BATTERY);
	battery_add_new_mode(phy->battery, RX_ON_BUSY, dCurrent, "Receiving energy(mJ)");

	getXmlVar(&dCurrent, IDLEMODE_CURRENT_MA, pXmlChild, 0, _DOUBLE, BATTERY);
	battery_add_new_mode(phy->battery, RX_ON_IDLE, dCurrent, "Idle energy(mJ)");

	return 1;
}
static bool isChangeRadioIsPermitted(PALOHA_PHY_VAR phy, PHY_TX_STATUS newState)
{
	PHY_TX_STATUS oldState = phy->transmitter_status;
	if (oldState == RX_ON_IDLE)
		return true;
	if (oldState == RX_ON_BUSY && newState != RX_ON_IDLE)
		return false;
	if (oldState == TRX_ON_BUSY && newState != RX_ON_IDLE)
		return false;
	return true;
}

bool set_radio_state(NETSIM_ID DeviceId, NETSIM_ID InterfaceId, PHY_TX_STATUS state)
{
	PALOHA_PHY_VAR phy = ALOHA_PHY(DeviceId, InterfaceId);
	if (phy == NULL) { return false; }
	ptrBATTERY battery = phy->battery;
	
	if (phy->transmitter_status == RX_OFF)
		return false;
	
	if (!isChangeRadioIsPermitted(phy, state))
		return false;
	bool isChange = true;
	if (battery)
	{
		isChange = battery_set_mode(battery, state, ldEventTime);
	}
	if (isChange)
	{
		phy->transmitter_status = state;
	}
	else
	{
		phy->transmitter_status = RX_OFF;
		battery_set_mode(battery, state, ldEventTime);
	}
	return isChange;
}