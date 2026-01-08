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

typedef struct stru_propagation_info_for_list
{
	NETSIM_ID TX_ID;
	NETSIM_ID RX_ID;
	PPROPAGATION_INFO Propagation_Info;
	_ptr_ele ele;
}propagation_info_for_list, * ptrpropagation_info_for_list;
ptrpropagation_info_for_list propagation_info_list;

#define PPROPAGATION_INFO_ALLOC() (struct stru_propagation_info_for_list*)list_alloc(sizeof(struct stru_propagation_info_for_list),offsetof(struct stru_propagation_info_for_list,ele))
#define PPROPAGATION_INFO_ADD(info,e)		(LIST_ADD_LAST(&(info),(e)))
#define PPROPAGATION_INFO_NEXT(entity)	(LIST_NEXT(entity))
#define PPROPAGATION_INFO_REMOVE(ls, mem) (LIST_FREE((void**)(ls),(mem)))

void ALOHA_gettxinfo(NETSIM_ID nTxId,
					NETSIM_ID nTxInterface,
					NETSIM_ID nRxId,
					NETSIM_ID nRxInterface,
					PTX_INFO Txinfo)
{
	PALOHA_PHY_VAR txphy = ALOHA_PHY(nTxId, nTxInterface);
	PALOHA_PHY_VAR rxphy = ALOHA_PHY(nRxId, nRxInterface);

	Txinfo->dCentralFrequency = txphy->frequency;
	Txinfo->dRxAntennaHeight = rxphy->dAntennaHeight;
	Txinfo->dRxGain = rxphy->dAntennaGain;
	Txinfo->dTxAntennaHeight = txphy->dAntennaHeight;
	Txinfo->dTxGain = txphy->dAntennaGain;
	Txinfo->dTxPower_mw = (double)txphy->tx_power;
	Txinfo->dTxPower_dbm = MW_TO_DBM(Txinfo->dTxPower_mw);
	Txinfo->dTx_Rx_Distance = DEVICE_DISTANCE(nTxId, nRxId);
}

static void fill_propagation_from_link(NETSIM_ID d, NETSIM_ID in, PPROPAGATION prop)
{
	memset(prop, 0, sizeof * prop);
	NetSim_LINKS* link = DEVICE_PHYLAYER(d, in)->pstruNetSimLinks;
	memcpy(prop, link->puniMedProp.pstruWirelessLink.propagation, sizeof * prop);
	prop->pathlossVar.d0 = 1;
}

bool CheckFrequencyInterfrence(double dFrequency1, double dFrequency2, double bandwidth)
{
	if (dFrequency1 > dFrequency2)
	{
		if ((dFrequency1 - dFrequency2) >= bandwidth)
			return false; // no interference
		else
			return true; // interference
	}
	else
	{
		if ((dFrequency2 - dFrequency1) >= bandwidth)
			return false; // no interference
		else
			return true; // interference
	}
}

bool check_interference(NETSIM_ID t, NETSIM_ID ti, NETSIM_ID r, NETSIM_ID ri)
{
	PALOHA_PHY_VAR tp = ALOHA_PHY(t, ti);
	PALOHA_PHY_VAR rp = ALOHA_PHY(r, ri);
	return CheckFrequencyInterfrence(tp->frequency, rp->frequency, tp->bandwidth);
}

ptrpropagation_info_for_list fn_NetSim_Aloha_InitPropagationModel(NETSIM_ID tx, NETSIM_ID rx)
{	
	ptrpropagation_info_for_list entry = PPROPAGATION_INFO_ALLOC();
	entry->TX_ID = tx;
	entry->RX_ID = rx;
	TX_INFO txInfo;
	ALOHA_gettxinfo(tx, 1, rx, 1, &txInfo);
	PROPAGATION prop;
	fill_propagation_from_link(tx, 1, &prop);
	entry->Propagation_Info = propagation_create_propagation_info(tx, 1, rx, 1, NULL, &txInfo, &prop);
	return entry;
}

PPROPAGATION_INFO find_propagation_info(NETSIM_ID tx, NETSIM_ID rx)
{
	ptrpropagation_info_for_list data = propagation_info_list;
	while (data) 
	{
		if (data->TX_ID == tx && data->RX_ID == rx)
		{
			data->Propagation_Info->txInfo.dTx_Rx_Distance = DEVICE_DISTANCE(tx, rx);
			return data->Propagation_Info;
		}
		else {
			data = PPROPAGATION_INFO_NEXT(data);
		}
	}
	return 0;
}

double get_received_power(NETSIM_ID TX,NETSIM_ID RX, double time)
{
	return _propagation_get_received_power_dbm(find_propagation_info(TX, RX), time);
}

double get_rx_power_callbackhandler(NETSIM_ID tx, NETSIM_ID txi,
									NETSIM_ID rx, NETSIM_ID rxi,
									double time)
{
	return DBM_TO_MW(_propagation_get_received_power_dbm(find_propagation_info(tx, rx), time));
}

PPROPAGATION_INFO get_aloha_propagation_info(NETSIM_ID TX, NETSIM_ID RX)
{
	return find_propagation_info(TX, RX);
}

int fn_NetSim_Aloha_FreePropagationInfo()
{
	NETSIM_ID t, ti;

	for (t = 0; t < NETWORK->nDeviceCount; t++)
	{
		for (ti = 0; ti < DEVICE(t + 1)->nNumOfInterface; ti++)
		{
			ptrpropagation_info_for_list data = propagation_info_list;
			while (data)
			{
				ptrpropagation_info_for_list temp = PPROPAGATION_INFO_NEXT(data);
				PPROPAGATION_INFO_REMOVE(&propagation_info_list, data);
				data = temp;
			}
		}
	}
	return 0;
}

static void calculate_received_power(PPROPAGATION_INFO info)
{
	info->txInfo.Tx_X = DEVICE_POSITION(info->nTxId)->X;
	info->txInfo.Tx_Y = DEVICE_POSITION(info->nTxId)->Y;
	info->txInfo.Tx_Z = DEVICE_POSITION(info->nTxId)->Z;

	info->txInfo.Rx_X = DEVICE_POSITION(info->nRxId)->X;
	info->txInfo.Rx_Y = DEVICE_POSITION(info->nRxId)->Y;
	info->txInfo.Rx_Z = DEVICE_POSITION(info->nRxId)->Z;

	_propagation_calculate_received_power(info, pstruEventDetails->dEventTime);
}

int fn_NetSim_Aloha_CalulateReceivedPower()
{
	NETSIM_ID i,j;
	for(i=0;i<NETWORK->nDeviceCount;i++)
	{
		for (j = 0; j < NETWORK->nDeviceCount; j++)
		{
			ptrpropagation_info_for_list entry;
			entry = fn_NetSim_Aloha_InitPropagationModel(i + 1, j + 1);
			PPROPAGATION_INFO_ADD(propagation_info_list, entry);
			calculate_received_power(entry->Propagation_Info);
		}
	}
	return 0;
}

int fn_NetSim_Aloha_handleMobility(NETSIM_ID d)
{
	for (NETSIM_ID i = 0; i < NETWORK->nDeviceCount; i++)
	{
		if (i + 1 == d) { continue; }
		calculate_received_power(find_propagation_info(d, i + 1));
		calculate_received_power(find_propagation_info(i + 1, d));
	}
	return 0;
}