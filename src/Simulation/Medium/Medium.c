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
#define _NETSIM_MDEIUM_CODE_
#include "main.h"
#include "Medium.h"
#include "ErrorModel.h"
#include "PropagationModel.h"
#pragma comment(lib,"NetworkStack.lib")
#pragma comment(lib,"Metrics.lib")
#pragma comment(lib,"PropagationModel.lib")
//#define _MEDIUM_LOG_

typedef struct stru_Packet_info
{
	NetSim_PACKET* packet;
	NETSIM_ID txId;
	NETSIM_ID txIf;
	NETSIM_ID rxId;
	NETSIM_ID rxIf;
	double startTime_us;
	double endTime_us;
	PHY_MODULATION modulation;
	double codeRate;
	double dDataRate_mbps;
	double dFrequency_MHz;
	double dBandwidth_MHz;

	double dMaxInterference_dbm;
	double dRXPower_dbm;
	struct stru_Packet_info* next;
}PACKETINFO, *ptrPACKETINFO;

typedef struct stru_Device
{
	NETSIM_ID deviceId;
	NETSIM_ID interfaceId;
	double frequency_MHz;
	double bandwidth_MHz;
	double dRxSensitivity_dBm;
	double dEDThreshold_dBm;
	PHY_MODULATION modulation;
	double dCodeRate;
	double dDataRate_mbps;

	//SNR-PEP-Table Function
	int MCS;
	string StandardType;
	string Ber_Model;

	double dCummulativeReceivedPower_mw;
	double dCummulativeReceivedPower_dbm;

	void(*medium_change_callback)(NETSIM_ID, NETSIM_ID, bool, NetSim_PACKET*);
	bool(*isRadioIdle)(NETSIM_ID, NETSIM_ID);
	void* (*propagationinfo_find)(NETSIM_ID, NETSIM_ID, NETSIM_ID, NETSIM_ID);
	bool(*isTranmitterBusy)(NETSIM_ID, NETSIM_ID);
	void(*packetSentNotify)(NETSIM_ID, NETSIM_ID, NetSim_PACKET*);
}DEVICE_IN_MEDIUM, *ptrDEVICE_IN_MEDIUM;

typedef struct stru_medium
{
	ptrDEVICE_IN_MEDIUM** devices;
	ptrPACKETINFO transmissionPacket;
}MEDIUM,*ptrMEDIUM;
static MEDIUM medium;
static ptrDEVICE_IN_MEDIUM medium_find_device(NETSIM_ID devId,
	NETSIM_ID devIf)
{
	if (!devId || !devIf)
		fnNetSimError("Device id = %d, device interface = %d is not a valid input for %s,",
			devId,
			devIf,
			__FUNCTION__);
	if (medium.devices &&
		medium.devices[devId - 1])
		return medium.devices[devId - 1][devIf - 1];
	return NULL;
}


static FILE* fplog = NULL;
static void write_log(char* format,...)
{
#ifdef _MEDIUM_LOG_
	if (fplog == NULL)
	{
		char str[BUFSIZ];
		sprintf(str, "%s\\%s", pszIOLogPath, "medium.log");
		fplog = fopen(str, "w");
	}
	va_list ls;
	va_start(ls, format);
	vfprintf(fplog, format, ls);
	fflush(fplog);
#else
	format;
#endif
}

static double GetRXPowerdBm(NETSIM_ID t, NETSIM_ID ti, NETSIM_ID r, NETSIM_ID ri, double time)
{
	ptrDEVICE_IN_MEDIUM dm = medium_find_device(t, ti);
	if (!dm)
	{
		fnNetSimError("Device %d, interface %d is not added to medium before transmission.",
			t, ti);
		return NEGATIVE_DBM;
	}

	PPROPAGATION_INFO info = dm->propagationinfo_find(t, ti, r, ri);
	if (info == NULL)
	{
		fnNetSimErrorandStop("Propagation info is NULL between %d:%d-%d:%d\n", t, ti, r, ri);
		return NEGATIVE_DBM;
	}
	return _propagation_get_received_power_dbm(info, time);
}

_declspec(dllexport) void medium_add_device(NETSIM_ID d,
	NETSIM_ID ifid,
	double dFrequency_MHz,
	double dBandwidth_MHz,
	double dRxSensitivity_dBm,
	double dEdThreshold_dBm,
	void(*medium_change_callback)(NETSIM_ID, NETSIM_ID, bool,NetSim_PACKET*),
	bool(*isRadioIdle)(NETSIM_ID, NETSIM_ID),
	bool(*isTransmitterBusy)(NETSIM_ID, NETSIM_ID),
	void*(*propagationinfo_find)(NETSIM_ID, NETSIM_ID, NETSIM_ID, NETSIM_ID),
	void(*packetsentnotify)(NETSIM_ID,NETSIM_ID,NetSim_PACKET*))
{
	ptrDEVICE_IN_MEDIUM dm = calloc(1, sizeof * dm);
	dm->deviceId = d;
	dm->interfaceId = ifid;
	dm->dEDThreshold_dBm = dEdThreshold_dBm;
	dm->dRxSensitivity_dBm = dRxSensitivity_dBm;
	dm->frequency_MHz = dFrequency_MHz;
	dm->bandwidth_MHz = dBandwidth_MHz;
	dm->dCummulativeReceivedPower_dbm = NEGATIVE_INFINITY;
	dm->medium_change_callback = medium_change_callback;
	dm->isRadioIdle = isRadioIdle;
	dm->isTranmitterBusy = isTransmitterBusy;
	dm->propagationinfo_find = propagationinfo_find;
	dm->packetSentNotify = packetsentnotify;

	if (!medium.devices)
		medium.devices = calloc(NETWORK->nDeviceCount, sizeof * medium.devices);
	if (!medium.devices[d - 1])
		medium.devices[d - 1] = calloc(DEVICE(d)->nNumOfInterface, sizeof * medium.devices[d - 1]);
	medium.devices[d - 1][ifid - 1] = dm;
}

_declspec(dllexport) void medium_update_frequency(NETSIM_ID d, NETSIM_ID in, double f_MHz)
{
	ptrDEVICE_IN_MEDIUM dm = medium_find_device(d, in);
	if (!dm)
	{
		fnNetSimError("%s is called without adding device %d:%d to medium. use medium_add_device to add the device.\n",
			__FUNCTION__, d, in);
		return;
	}
	dm->frequency_MHz = f_MHz;
}

_declspec(dllexport) void medium_update_bandwidth(NETSIM_ID d, NETSIM_ID in, double bw_MHz)
{
	ptrDEVICE_IN_MEDIUM dm = medium_find_device(d, in);
	if (!dm)
	{
		fnNetSimError("%s is called without adding device %d:%d to medium. use medium_add_device to add the device.\n",
			__FUNCTION__, d, in);
		return;
	}
	dm->bandwidth_MHz = bw_MHz;
}

_declspec(dllexport) void medium_update_rxsensitivity(NETSIM_ID d, NETSIM_ID in, double p_dbm)
{
	ptrDEVICE_IN_MEDIUM dm = medium_find_device(d, in);
	if (!dm)
	{
		fnNetSimError("%s is called without adding device %d:%d to medium. use medium_add_device to add the device.\n",
			__FUNCTION__, d, in);
		return;
	}
	dm->dRxSensitivity_dBm = p_dbm;
}

_declspec(dllexport) void medium_update_edthershold(NETSIM_ID d, NETSIM_ID in, double p_dbm)
{
	ptrDEVICE_IN_MEDIUM dm = medium_find_device(d, in);
	if (!dm)
	{
		fnNetSimError("%s is called without adding device %d:%d to medium. use medium_add_device to add the device.\n",
			__FUNCTION__, d, in);
		return;
	}
	dm->dEDThreshold_dBm = p_dbm;
}

_declspec(dllexport) void medium_update_modulation(NETSIM_ID d, NETSIM_ID in, PHY_MODULATION m, double coderate)
{
	ptrDEVICE_IN_MEDIUM dm = medium_find_device(d, in);
	if (!dm)
	{
		fnNetSimError("%s is called without adding device %d:%d to medium. use medium_add_device to add the device.\n",
			__FUNCTION__, d, in);
		return;
	}
	dm->modulation = m;
	dm->dCodeRate = coderate;
}

_declspec(dllexport) void medium_update_datarate(NETSIM_ID d, NETSIM_ID in, double r_mbps)
{
	ptrDEVICE_IN_MEDIUM dm = medium_find_device(d, in);
	if (!dm)
	{
		fnNetSimError("%s is called without adding device %d:%d to medium. use medium_add_device to add the device.\n",
			__FUNCTION__, d, in);
		return;
	}
	dm->dDataRate_mbps = r_mbps;
}

_declspec(dllexport) void medium_update_MCS(NETSIM_ID d, NETSIM_ID in, int MCS, string Standard, string BER_Model)
{
	ptrDEVICE_IN_MEDIUM dm = medium_find_device(d, in);
	if (!dm)
	{
		fnNetSimError("%s is called without adding device %d:%d to medium. use medium_add_device to add the device.\n",
			__FUNCTION__, d, in);
		return;
	}
	dm->MCS = MCS;
	dm->StandardType = Standard;
	dm->Ber_Model = BER_Model;
}

static void medium_remove_device(NETSIM_ID d,
								 NETSIM_ID ifId)
{
	ptrDEVICE_IN_MEDIUM dm = medium_find_device(d, ifId);
	if (dm)
	{
		free(dm);
		medium.devices[d - 1][ifId - 1] = NULL;
	}
}

static ptrPACKETINFO packetInfo_add(NetSim_PACKET* packet,
	NETSIM_ID txId,
	NETSIM_ID txIf,
	NETSIM_ID rxId,
	NETSIM_ID rxIf)
{
	ptrDEVICE_IN_MEDIUM dm = medium_find_device(txId, txIf);

	ptrPACKETINFO p = calloc(1, sizeof* p);
	p->packet = packet;
	p->rxId = rxId;
	p->rxIf = rxIf;
	p->startTime_us = packet->pstruPhyData->dArrivalTime;
	p->endTime_us = packet->pstruPhyData->dEndTime;
	p->txId = txId;
	p->txIf = txIf;

	p->codeRate = dm->dCodeRate;
	p->dBandwidth_MHz = dm->bandwidth_MHz;
	p->dDataRate_mbps = dm->dDataRate_mbps;
	p->dFrequency_MHz = dm->frequency_MHz;
	p->modulation = dm->modulation;
	p->dRXPower_dbm = GetRXPowerdBm(txId, txIf, rxId, rxIf, p->startTime_us);
	p->dMaxInterference_dbm = NEGATIVE_DBM;

	if (medium.transmissionPacket)
	{
		ptrPACKETINFO t = medium.transmissionPacket;
		while (t->next)
			t = t->next;
		t->next = p;
	}
	else
	{
		medium.transmissionPacket = p;
	}
	write_log("New Packet %d,TX=%d-%d,RX=%d-%d,RxPower=%lf,\n",
		p->packet->nPacketId == 0 ? p->packet->nControlDataType * -1 : p->packet->nPacketId,
		txId, txIf, rxId, rxIf,
		p->dRXPower_dbm);
	return p;
}

static ptrPACKETINFO  packetInfo_find(NetSim_PACKET* packet,
	NETSIM_ID txId,
	NETSIM_ID txIf,
	NETSIM_ID rxId,
	NETSIM_ID rxIf)

{
	ptrPACKETINFO i = medium.transmissionPacket;
	while (i)
	{
		if (txId == i->txId && txIf == i->txIf && rxId == i->rxId && rxIf == i->rxIf && packet == i->packet)
			return i;
        i = i->next;
	}
	return NULL;
}


static ptrPACKETINFO packetInfo_remove(NetSim_PACKET* packet)
{
	ptrPACKETINFO p = medium.transmissionPacket;
	ptrPACKETINFO pr = NULL;
	while (p)
	{
		if (p->packet == packet)
		{
			if (pr)
			{
				pr->next = p->next;
				break;
			}
			else
			{
				medium.transmissionPacket = p->next;
				break;
			}
		}
		pr = p;
		p = p->next;
	}
	p->next = NULL;
	write_log("remove Packet %d,TX=%d-%d,RX=%d-%d,RxPower=%lf,\n",
		p->packet->nPacketId == 0 ? p->packet->nControlDataType * -1 : p->packet->nPacketId,
		p->txId, p->txIf, p->rxId, p->rxIf,
		p->dRXPower_dbm);
	return p;
}

static void packetInfo_free(ptrPACKETINFO info)
{
	free(info);
}

#define BOLTZMANN 1.38064852e-23 //m2kgs-2K-1
#define TEMPERATURE 300 //kelvin
static double compute_sinr(double p, double i, double bw)
{
	double noise = BOLTZMANN * TEMPERATURE * bw * 1000000; //in W
	double Pmw = DBM_TO_MW(p);
	double imw = DBM_TO_MW(i);
	noise *= 1000; // in mW
	return MW_TO_DBM(Pmw / (noise + imw));
}

static double GetRXPowerMW(NETSIM_ID t, NETSIM_ID ti, NETSIM_ID r, NETSIM_ID ri, double time)
{
	return DBM_TO_MW(GetRXPowerdBm(t, ti, r, ri, time));
}

static void medium_update_interference(ptrPACKETINFO info)
{
	info;
	ptrPACKETINFO i = medium.transmissionPacket;
	while (i)
	{
		/*if (i->txId == info->txId &&
			i->txIf == info->txIf &&
			i != info)
		{
			i = i->next;
			continue;
		}*/
		ptrDEVICE_IN_MEDIUM dm = medium_find_device(i->rxId, i->rxIf);
		
		double pdbm = GetRXPowerdBm(i->txId, i->txIf,
			i->rxId, i->rxIf,
			i->startTime_us);
		double p = DBM_TO_MW(pdbm);

		double interference = dm->dCummulativeReceivedPower_mw - p;
		double interferencedbm = MW_TO_DBM(interference);

		i->dMaxInterference_dbm = max(i->dMaxInterference_dbm, interferencedbm);

		//if (dm->isTranmitterBusy(dm->deviceId, dm->interfaceId)) // Transmitter refers to transceiver.
		//{
		//	i->packet->nPacketStatus = PacketStatus_Collided; // Also receiving when transmitting 
		//	write_log("Packet %d collided due to %d-%d busy,\n",
		//		i->packet->nPacketId == 0 ? i->packet->nControlDataType * -1 : i->packet->nPacketId,
		//		dm->deviceId, dm->interfaceId);
		//}

		i = i->next;
	}
}

static bool isAnypacketIsThereForThisTransmitter(ptrPACKETINFO info)
{
	ptrPACKETINFO i = medium.transmissionPacket;
	while (i)
	{
		if (i != info)
		{
			if (i->txId == info->txId &&
				i->txIf == info->txIf)
				return true;
		}
		i = i->next;
	}
	return false;
}

static bool CheckFrequencyInterfrence(double dFrequency1, double dFrequency2, double bandwidth)
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

static void update_power_due_to_transmission(ptrPACKETINFO info)
{	
	ptrDEVICE_IN_MEDIUM txdm = medium_find_device(info->txId, info->txIf);
	if (!txdm)
	{
		fnNetSimError("Device %d, interface %d is not added to medium before transmission.",
					  info->txId, info->txIf);
		return;
	}

	NETSIM_ID nLoop, nLoopInterface;
	for (nLoop = 1; nLoop <= NETWORK->nDeviceCount; nLoop++)
	{
		for (nLoopInterface = 1; nLoopInterface <= DEVICE(nLoop)->nNumOfInterface; nLoopInterface++)
		{
			ptrDEVICE_IN_MEDIUM dm = medium_find_device(nLoop, nLoopInterface);
			if (!dm)
				continue;

			if (!CheckFrequencyInterfrence(txdm->frequency_MHz, dm->frequency_MHz, txdm->bandwidth_MHz))
				continue; //Different band


			double p = GetRXPowerMW(txdm->deviceId, txdm->interfaceId,
				dm->deviceId, dm->interfaceId,
				info->startTime_us);

			dm->dCummulativeReceivedPower_mw += p;

			dm->dCummulativeReceivedPower_dbm = MW_TO_DBM(dm->dCummulativeReceivedPower_mw);

			write_log("\tTotal recv power of %d-%d = %lf\n", dm->deviceId, dm->interfaceId,
				dm->dCummulativeReceivedPower_dbm);

			if (nLoop == info->rxId &&
				nLoopInterface == info->rxIf) continue; // Ignore receiver
			if (nLoop == info->txId &&
				nLoopInterface == info->txIf) continue; // Ignore transmitter

			if (dm->dCummulativeReceivedPower_dbm > dm->dEDThreshold_dBm &&
					dm->medium_change_callback)
					dm->medium_change_callback(dm->deviceId, dm->interfaceId, true, info->packet);
			double pdbm = GetRXPowerdBm(info->txId, info->txIf, dm->deviceId, dm->interfaceId, info->startTime_us);
			if (pdbm > dm->dEDThreshold_dBm && dm->packetSentNotify)
				dm->packetSentNotify(dm->deviceId, dm->interfaceId, info->packet);
		}
	}
}

static void update_power_due_to_transmission_stop(ptrPACKETINFO info)
{
	ptrDEVICE_IN_MEDIUM txdm = medium_find_device(info->txId, info->txIf);
	if (!txdm)
	{
		fnNetSimError("Device %d, interface %d is not added to medium before transmission.",
					  info->txId, info->txIf);
		return;
	}

	NETSIM_ID nLoop, nLoopInterface;
	for (nLoop = 1; nLoop <= NETWORK->nDeviceCount; nLoop++)
	{
		for (nLoopInterface = 1; nLoopInterface <= DEVICE(nLoop)->nNumOfInterface; nLoopInterface++)
		{
			if (nLoop == info->txId &&
				nLoopInterface == info->txIf)
			{
				if(txdm->isRadioIdle(txdm->deviceId, txdm->interfaceId) &&
				   txdm->medium_change_callback)
					txdm->medium_change_callback(txdm->deviceId, txdm->interfaceId, false, NULL);
				//continue;
			}

			ptrDEVICE_IN_MEDIUM dm = medium_find_device(nLoop, nLoopInterface);
			if (!dm)
				continue;

			if (!CheckFrequencyInterfrence(txdm->frequency_MHz, dm->frequency_MHz, txdm->bandwidth_MHz))
				continue; //Different band


			dm->dCummulativeReceivedPower_mw -= GetRXPowerMW(txdm->deviceId, txdm->interfaceId,
																  dm->deviceId, dm->interfaceId,
																  info->startTime_us);

			dm->dCummulativeReceivedPower_dbm = MW_TO_DBM(dm->dCummulativeReceivedPower_mw);

			if (nLoop == info->rxId &&
				nLoopInterface == info->rxIf) continue; // Ignore receiver
			if (nLoop == info->txId &&
				nLoopInterface == info->txIf) continue; // Ignore transmitter

			if (dm->dCummulativeReceivedPower_dbm < dm->dEDThreshold_dBm &&
				dm->medium_change_callback)
				dm->medium_change_callback(dm->deviceId, dm->interfaceId, false, NULL);
		}
	}
}

static void medium_mark_packet_error(ptrPACKETINFO info)
{
	ptrDEVICE_IN_MEDIUM dm = medium_find_device(info->txId, info->txIf);
	double i = info->dMaxInterference_dbm;
	double p = info->dRXPower_dbm;
	double w = info->dBandwidth_MHz;
	double ber;
	double f = _propagation_calculate_fadingloss(dm->propagationinfo_find(
		info->txId, info->txIf,
		info->rxId, info->rxIf));
	p -= f;

	//BER Model Selection
	if (strcmp(dm->Ber_Model, "SINR_BER_PER_TABLE") == 0)
		ber = calculate_BER_Using_Table(info->modulation, p, i, w, dm->MCS, dm->StandardType);
	else
		ber = calculate_FEC_BER(info->modulation, info->codeRate, p, i, w, info->dDataRate_mbps);

	PACKET_STATUS status = fn_NetSim_Packet_DecideError(ber, info->packet->pstruPhyData->dPacketSize);
	if (status == PacketStatus_Error && info->packet->nPacketStatus == PacketStatus_NoError)
	{
		if (i >= dm->dRxSensitivity_dBm) info->packet->nPacketStatus = PacketStatus_Collided;
		else info->packet->nPacketStatus = PacketStatus_Error;
		write_log("Packet %d errored, BER=%lf, Interference=%lf, RXPower=%lf,\n",
			info->packet->nPacketId == 0 ? info->packet->nControlDataType * -1 : info->packet->nPacketId,
			ber, i, p);
	}
}

_declspec(dllexport) void medium_notify_packet_send(NetSim_PACKET* packet,
													NETSIM_ID txId,
													NETSIM_ID txIf,
													NETSIM_ID rxId,
													NETSIM_ID rxIf)
{
	ptrPACKETINFO info = packetInfo_add(packet, txId, txIf, rxId, rxIf);
	if (isAnypacketIsThereForThisTransmitter(info))
		return;
	update_power_due_to_transmission(info);
	medium_update_interference(info);
}

_declspec(dllexport) void medium_notify_packet_received(NetSim_PACKET* packet)
{
	ptrPACKETINFO info = packetInfo_remove(packet);
	medium_mark_packet_error(info);
	if (!isAnypacketIsThereForThisTransmitter(info))
		update_power_due_to_transmission_stop(info);
	packetInfo_free(info);
}

_declspec(dllexport) bool medium_isIdle(NETSIM_ID d,
										NETSIM_ID in)
{
	ptrDEVICE_IN_MEDIUM dm = medium_find_device(d, in);
	return (dm->dCummulativeReceivedPower_dbm < dm->dEDThreshold_dBm);
}

_declspec(dllexport) double medium_get_interference(NetSim_PACKET* packet,
	NETSIM_ID txId,
	NETSIM_ID txIf,
	NETSIM_ID rxId,
	NETSIM_ID rxIf)
{
	ptrPACKETINFO info = packetInfo_find(packet, txId, txIf, rxId, rxIf);
	if (info)
		return (info->dMaxInterference_dbm);
	else
		return NEGATIVE_DBM;
}

