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
#include "IEEE802_11_Phy.h"

int fn_NetSim_IEEE802_11_PropagationModelCallback(NETSIM_ID nodeId)
{
	fn_Update_TxInfo_TxRxPos(nodeId);
	return fn_NetSim_IEEE802_11_PropagationModel(nodeId, ldEventTime);
}

void IEEE802_11_gettxinfo(NETSIM_ID nTxId,
	NETSIM_ID nTxInterface,
	NETSIM_ID nRxId,
	NETSIM_ID nRxInterface,
	PTX_INFO Txinfo)
{
	PIEEE802_11_PHY_VAR txphy = IEEE802_11_PHY(nTxId, nTxInterface);
	PIEEE802_11_PHY_VAR rxphy = IEEE802_11_PHY(nRxId, nRxInterface);
	Txinfo->dCentralFrequency = txphy->dFrequency;
	Txinfo->dRxAntennaHeight = rxphy->dAntennaHeight;
	Txinfo->dTxAntennaHeight = txphy->dAntennaHeight;
	Txinfo->Tx_AntennaType = txphy->antennaType;
	Txinfo->Rx_AntennaType = rxphy->antennaType;
	Txinfo->dTxGain = 0.0;
	Txinfo->dRxGain = 0.0;
	Txinfo->Tx_X = DEVICE_POSITION(nTxId)->X;
	Txinfo->Tx_Y = DEVICE_POSITION(nTxId)->Y;
	Txinfo->Rx_X = DEVICE_POSITION(nRxId)->X;
	Txinfo->Rx_Y = DEVICE_POSITION(nRxId)->Y;
	if (txphy->antennaType == OMNIDIRECTIONAL_ANTENNA)
	{
	Txinfo->dTxGain = txphy->dAntennaGain;
	}
	else
	{
		Txinfo->dTx_Beamwidth = txphy->beamwidth;
		Txinfo->dTx_BoresightAngle = txphy->boresightAngle;
		Txinfo->dTx_FrontToBackRatio = txphy->frontToBackRatio;
		Txinfo->dTx_ElementGain = txphy->elementGain;
	}

	if(rxphy->antennaType == OMNIDIRECTIONAL_ANTENNA)
	{
		Txinfo->dRxGain = rxphy->dAntennaGain;
	}
	else
	{
		Txinfo->dRx_Beamwidth = rxphy->beamwidth;
		Txinfo->dRx_BoresightAngle = rxphy->boresightAngle;
		Txinfo->dRx_FrontToBackRatio = rxphy->frontToBackRatio;
		Txinfo->dRx_ElementGain = rxphy->elementGain;
	}
	
	Txinfo->dTxPower_mw = (double)txphy->nTxPower_mw;
	Txinfo->dTxPower_dbm = MW_TO_DBM(Txinfo->dTxPower_mw);
	Txinfo->dTx_Rx_Distance = DEVICE_DISTANCE(nTxId, nRxId);
	Txinfo->d0 = txphy->d0;
}

static void fill_propagation_from_link(NETSIM_ID d, NETSIM_ID in, PPROPAGATION prop, PTX_INFO Txinfo)
{
	memset(prop, 0, sizeof * prop);
	NetSim_LINKS* link = DEVICE_PHYLAYER(d, in)->pstruNetSimLinks;
	memcpy(prop, link->puniMedProp.pstruWirelessLink.propagation, sizeof * prop);
	prop->pathlossVar.d0 = Txinfo->d0;
}

void fn_NetSim_IEEE8052_11_InitPropagationModel(NETSIM_ID tx, NETSIM_ID txi, NETSIM_ID rx, NETSIM_ID rxi)
{
	PIEEE802_11_PHY_VAR txphy = IEEE802_11_PHY(tx, txi);
	ptrpropagation_info_for_list entry = PPROPAGATION_INFO_ALLOC();
	entry->TX_ID = tx;
	entry->RX_ID = rx;
	entry->TX_IF_ID = txi;
	entry->RX_IF_ID = rxi;
	TX_INFO txInfo;
	IEEE802_11_gettxinfo(tx, txi, rx, rxi, &txInfo);
	PROPAGATION prop;
	fill_propagation_from_link(tx, txi, &prop, &txInfo);
	entry->Propagation_Info = propagation_create_propagation_info(tx, txi, rx, rxi, NULL, &txInfo, &prop);
	PPROPAGATION_INFO_ADD(txphy->propagation_info_list, entry);
}

_declspec(dllexport) PPROPAGATION_INFO find_propagation_info(NETSIM_ID t, NETSIM_ID ti, NETSIM_ID r, NETSIM_ID ri)
{
	PIEEE802_11_PHY_VAR txphy = IEEE802_11_PHY(t, ti);
	ptrpropagation_info_for_list data = txphy->propagation_info_list;
	while (data)
	{
		if (data->TX_ID == t && data->RX_ID == r && data->TX_IF_ID == ti && data->RX_IF_ID == ri)
		{
			data->Propagation_Info->txInfo.dTx_Rx_Distance = DEVICE_DISTANCE(t, r);
			return data->Propagation_Info;
		}
		else {
			data = PPROPAGATION_INFO_NEXT(data);
		}
	}
	return NULL;
}

int fn_NetSim_IEEE802_11_PropagationInit()
{
	NETSIM_ID t, ti, r, ri;

	for (t = 0; t < NETWORK->nDeviceCount; t++)
	{
		for (ti = 0; ti < DEVICE(t + 1)->nNumOfInterface; ti++)
		{
			if (!isIEEE802_11_Configure(t + 1, ti + 1))
				continue;
			for (r = 0; r < NETWORK->nDeviceCount; r++)
			{
				for (ri = 0; ri < DEVICE(r + 1)->nNumOfInterface; ri++)
				{
					if (!isIEEE802_11_Configure(r + 1, ri + 1))
						continue;
					fn_NetSim_IEEE8052_11_InitPropagationModel(t + 1, ti + 1, r + 1, ri + 1);
				}
			}
		}
	}
	return 0;
}

int fn_NetSim_IEEE802_11_FreePropagationInfo()
{
	NETSIM_ID t, ti;

	for (t = 0; t < NETWORK->nDeviceCount; t++)
	{
		for (ti = 0; ti < DEVICE(t + 1)->nNumOfInterface; ti++)
		{
			if (!isIEEE802_11_Configure(t + 1, ti + 1))
				continue;
			PIEEE802_11_PHY_VAR txphy = IEEE802_11_PHY(t + 1, ti + 1);
			ptrpropagation_info_for_list data = txphy->propagation_info_list;
			while (data)
			{
				ptrpropagation_info_for_list temp = PPROPAGATION_INFO_NEXT(data);
				PPROPAGATION_INFO_REMOVE(txphy->propagation_info_list, data);
				data = temp;
			}
		}
	}
	return 0;
}

/**
This function used to calculate the received from any wireless node other wireless
nodes in the network. It check is there any interference between the adjacent channel.
If any interference then it consider that effect to calculate the received power.
*/
int fn_NetSim_IEEE802_11_PropagationModel(NETSIM_ID nodeId,double time)
{
	NETSIM_ID in, c, ci;

	for (in = 0; in < DEVICE(nodeId)->nNumOfInterface; in++)
	{
		if (!isIEEE802_11_Configure(nodeId, in + 1))
			continue;

		if (isVirtualInterface(nodeId, in + 1)) continue;

		for (c = 0; c < NETWORK->nDeviceCount; c++)
		{
			for (ci = 0; ci < DEVICE(c + 1)->nNumOfInterface; ci++)
			{
				if (!isIEEE802_11_Configure(c + 1, ci + 1))
					continue;

				if (isVirtualInterface(c + 1, ci + 1)) continue;

				//Distance will change between Tx and Rx due to mobility.
				// So update the power from both side
				
				
				PPROPAGATION_INFO Txinfo = find_propagation_info(nodeId, in + 1, c + 1, ci + 1);

				Txinfo->txInfo.Tx_X = DEVICE_POSITION(Txinfo->nTxId)->X;
				Txinfo->txInfo.Tx_Y = DEVICE_POSITION(Txinfo->nTxId)->Y;
				Txinfo->txInfo.Tx_Z = DEVICE_POSITION(Txinfo->nTxId)->Z;

				Txinfo->txInfo.Rx_X = DEVICE_POSITION(Txinfo->nRxId)->X;
				Txinfo->txInfo.Rx_Y = DEVICE_POSITION(Txinfo->nRxId)->Y;
				Txinfo->txInfo.Rx_Z = DEVICE_POSITION(Txinfo->nRxId)->Z;

				PPROPAGATION_INFO Rxinfo = find_propagation_info(c + 1, ci + 1, nodeId, in + 1);

				Rxinfo->txInfo.Tx_X = DEVICE_POSITION(Rxinfo->nTxId)->X;
				Rxinfo->txInfo.Tx_Y = DEVICE_POSITION(Rxinfo->nTxId)->Y;
				Rxinfo->txInfo.Tx_Z = DEVICE_POSITION(Rxinfo->nTxId)->Z;

				Rxinfo->txInfo.Rx_X = DEVICE_POSITION(Rxinfo->nRxId)->X;
				Rxinfo->txInfo.Rx_Y = DEVICE_POSITION(Rxinfo->nRxId)->Y;
				Rxinfo->txInfo.Rx_Z = DEVICE_POSITION(Rxinfo->nRxId)->Z;

				_propagation_calculate_received_power(Txinfo, time);
				_propagation_calculate_received_power(Rxinfo, time);

			}
		}
	}
	return 0;
}

double GET_RX_POWER_dbm(NETSIM_ID tx, NETSIM_ID txi, NETSIM_ID rx, NETSIM_ID rxi, double time)
{
	PPROPAGATION_INFO info = find_propagation_info(tx, txi, rx, rxi);
	if (info == NULL)
	{
		fnNetSimErrorandStop("Propagation info is NULL between %d:%d-%d:%d\n", tx, txi, rx, rxi);
		return NEGATIVE_DBM;
	}
	return _propagation_get_received_power_dbm(info, time);
}

void fn_Update_TxInfo_TxRxPos(NETSIM_ID devId)
{
	NETSIM_ID d,in, c, ci;
	double time = ldEventTime;
	for (d = 0; d < NETWORK->nDeviceCount; d++)
	{
		for (in = 0; in < DEVICE(d+1)->nNumOfInterface; in++)
		{
			if (!isIEEE802_11_Configure(d+1, in + 1))
				continue;

			if (isVirtualInterface(d+1, in + 1)) continue;

			for (c = 0; c < NETWORK->nDeviceCount; c++)
			{
				for (ci = 0; ci < DEVICE(c + 1)->nNumOfInterface; ci++)
				{
					if (!isIEEE802_11_Configure(c + 1, ci + 1))
						continue;

					if (isVirtualInterface(c + 1, ci + 1)) continue;

					if (d+1 == devId)
					{
						PPROPAGATION_INFO pinfo = find_propagation_info(d + 1, in + 1, c + 1, ci + 1);
						pinfo->txInfo.Tx_X = DEVICE_POSITION(d + 1)->X;
						pinfo->txInfo.Tx_Y = DEVICE_POSITION(d + 1)->Y;
					}

					if (c + 1 == devId)
					{
						PPROPAGATION_INFO pinfo = find_propagation_info(d + 1, in + 1, c + 1, ci + 1);
						pinfo->txInfo.Rx_X = DEVICE_POSITION(c + 1)->X;
						pinfo->txInfo.Rx_Y = DEVICE_POSITION(c + 1)->Y;
					}

				}
			}
		}
	}
}