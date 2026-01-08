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

#pragma region HEADER_FILES
#include "stdafx.h"
#include "stdio.h"
#include "stdlib.h"
#include "LTENR_MAC.h"
#include "LTENR_PHY.h"
#include "..\NetSim_Python_Interfacing\Waiting_Struct_Listener_v2.h"
#include "LTENR_PDCP.h"
#include "LTENR_NetworkSlicing.h"
#pragma endregion

#pragma comment(lib, "NetSim_Python_Interfacing.lib")

#pragma region FUNCTION_PROTOTYPE
//PRB
static void LTENR_form_prb_list(NETSIM_ID d, NETSIM_ID in, int CA_ID);

//Active UE per CA
static void LTENR_form_active_ue_list(NETSIM_ID gnbId, NETSIM_ID gnbIf, int CA_ID);

//Frame
static void LTENR_addStartFrameEvent(NETSIM_ID gnbId, NETSIM_ID gnbIf, double time, int CA_ID);
void LTENR_handleStartFrameEvent();

//SubFrame
static void LTENR_addStartSubFrameEvent(NETSIM_ID gnbId, NETSIM_ID gnbIf, double time, int CA_ID);
void LTENR_handleStartSubFrameEvent();

//Slot
static void LTENR_addStartSlotEvent(NETSIM_ID gnbId, NETSIM_ID gnbIf, double time, int CA_ID);
void LTENR_handleStartSlotEvent();

//SendReceive
void LTENR_handleSendReceive();

//Association
void LTENR_PHY_ASSOCIATION(NETSIM_ID gnbId, NETSIM_ID gnbIf,
						   NETSIM_ID ueId, NETSIM_ID ueIf,
						   bool isAssociated);

//AMC
static void LTENR_PHY_initAMCInfo(ptrLTENR_GNBPHY phy, ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo);
static void LTENR_PHY_setAMCInfo(ptrLTENR_GNBPHY phy, ptrLTENR_ASSOCIATEDUEPHYINFO info, int CA_ID);

//BWP
void LTENR_ACTIVEUEINFO_ADD(NETSIM_ID ueId, NETSIM_ID ueIf, NETSIM_ID gnbId, NETSIM_ID gnbIf, int CA_ID);
void LTENR_ACTIVEUEINFO_REMOVE(NETSIM_ID ueId, NETSIM_ID ueIf, NETSIM_ID gnbId, NETSIM_ID gnbIf, int CA_ID);
//ptrLTENR_CA_UE_LIST LTENR_ACTIVEUEINFO_FIND(NETSIM_ID ueId, NETSIM_ID ueIf, NETSIM_ID gnbId, NETSIM_ID gnbIf, int CA_ID);
#pragma endregion

#pragma region OLLA_LOG
static FILE* fpOLLALog = NULL;
static void OpenOLLALog()
{
	if (get_protocol_log_status("LTENR_OLLA_LOG"))
	{
		char s[BUFSIZ];
		sprintf(s, "%s\\%s", pszIOLogPath, "LTENR_OLLA_Log.csv");
		fpOLLALog = fopen(s, "w");
		if (!fpOLLALog)
		{
			fnSystemError("Unable to open %s file", s);
			perror(s);
		}
		else
		{
			fprintf(fpOLLALog, "%s,%s,%s,%s,%s,",
				"Time(ms)", "eNB/gNB Name", "UE Name", "Carrier ID", "Layer ID");

			fprintf(fpOLLALog, "%s,%s,%s,%s,%s,",
				"Frame Id", "Sub-Frame Id", "Slot Id", "Channel", "Phy SINR(dB)");

			fprintf(fpOLLALog, "%s,%s,%s,%s,",
				"SINR Delta(dB)", "Virtual SINR(dB)", "CQI without OLLA", "CQI with OLLA");

			fprintf(fpOLLALog, "\n");
			fflush(fpOLLALog);
		}
	}
}

static void CloseOLLALog()
{
	if (fpOLLALog) { fclose(fpOLLALog); }
}

void LTENR_OLLA_Log_Init()
{
	OpenOLLALog();
}

void LTENR_OLLA_Log_Finish()
{
	CloseOLLALog();
}

static void WriteOLLALog(char* format, ...)
{
	if (fpOLLALog)
	{
		va_list l;
		va_start(l, format);
		vfprintf(fpOLLALog, format, l);
		if (nDbgFlag) fflush(fpOLLALog);
	}
}
#pragma endregion //OLLA_LOG

static double LTENR_PHY_GetDownlinkBeamFormingValue(ptrLTENR_PROPAGATIONINFO info, int layerId)
{
	if (info->propagationConfig->fastFadingModel == LTENR_FASTFADING_MODEL_NO_FADING_MIMO_ARRAY_GAIN)
	{
		UINT MaxAntenaCount = max(info->downlink.rxAntennaCount, info->downlink.txAntennaCount);
		info->downlink.ArrayGain = 10.0 * log10(MaxAntenaCount);
		return info->downlink.ArrayGain;
	}
	if (info->propagationConfig->fastFadingModel == LTENR_FASTFADING_MODEL_AWGN_WITH_RAYLEIGH_FADING)
	{
		return LTENR_BeamForming_Downlink_GetValue(info, layerId);
	}
	else
		return 0;
}


#pragma region PHY_INIT
void fn_NetSim_LTENR_PHY_Init()
{
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENT_PHY_HANDLE_SEND_RECEIVE, "LTENR_HANDLE_SEND_RECEIVE", LTENR_handleSendReceive);
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENT_PHY_STARTFRAME, "LTENR_STARTFRAME", LTENR_handleStartFrameEvent);
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENT_PHY_STARTSUBFRAME, "LTENR_STARTSUBFRAME", LTENR_handleStartSubFrameEvent);
	LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENT_PHY_STARTSLOT, "LTENR_STARTSLOT", LTENR_handleStartSlotEvent);

	fn_NetSim_LTENR_RegisterCallBackForAssociation(LTENR_PHY_ASSOCIATION);
}

void fn_NetSim_LTENR_UEPHY_Init(NETSIM_ID ueId, NETSIM_ID ueIf)
{
}

void fn_NetSim_LTENR_GNBPHY_Init(NETSIM_ID gnbId, NETSIM_ID gnbIf)
{
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);
	UINT i = 0;
	for (i = 0; i < phy->ca_count; i++) {
		phy->spectrumConfig->Current_CA_ID = i;

		LTENR_form_prb_list(gnbId, gnbIf, i);

		if(phy->spectrumConfig->CA[i]->isBWPEnabled)
			LTENR_form_active_ue_list(gnbId, gnbIf, i);

		LTENR_addStartFrameEvent(gnbId, gnbIf, 0, i);
	}
	//Beamforming
	LTENR_BeamForming_InitCoherenceTimer(phy);
}

ptrLTENR_GNBPHY LTENR_GNBPHY_NEW(NETSIM_ID gnbId, NETSIM_ID gnbIf)
{
	ptrLTENR_GNBPHY phy = calloc(1, sizeof * phy);
	phy->spectrumConfig = calloc(1, sizeof * phy->spectrumConfig);
	phy->propagationConfig = calloc(1, sizeof * phy->propagationConfig);
	phy->antenna = calloc(1, sizeof * phy->antenna);
	LTENR_GNBPHY_SET(gnbId, gnbIf, phy);
	phy->gnbId = gnbId;
	phy->gnbIf = gnbIf;
	return phy;
}

ptrLTENR_UEPHY LTENR_UEPHY_NEW(NETSIM_ID ueId, NETSIM_ID ueIf)
{
	ptrLTENR_UEPHY phy = calloc(1, sizeof * phy);
	phy->propagationConfig = calloc(1, sizeof * phy->propagationConfig);
	phy->antenna = calloc(1, sizeof * phy->antenna);
	LTENR_UEPHY_SET(ueId, ueIf, phy);
	phy->ueId = ueId;
	phy->ueIf = ueIf;
	return phy;
}
#pragma endregion

#pragma region PRB
static void LTENR_form_prb_list(NETSIM_ID d, NETSIM_ID in, int CA_ID)
{
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(d, in);
	ptrLTENR_SPECTRUMCONFIG sc = phy->spectrumConfig;
	ptrLTENR_CA ca = sc->CA[CA_ID];
	UINT i;
	print_ltenr_log("Forming PRB list for gNB %d:%d -- \n", d, in);
	double prbBandwidth_kHz = ca->prbBandwidth_kHz;
	double guard_kHz = ca->guardBand_kHz;
	print_ltenr_log("CA_ID = %d\n", CA_ID);
	print_ltenr_log("\tF_Low_MHz = %d\n", ca->Flow_MHZ);
	print_ltenr_log("\tF_High_MHz = %d\n", ca->Fhigh_MHz);
	print_ltenr_log("\tChannel_Bandwidth_MHz = %lf\n", ca->channelBandwidth_mHz);
	print_ltenr_log("\tPRB_Bandwidth_kHz = %lf\n", prbBandwidth_kHz);
	print_ltenr_log("\tGuard_Bandwidth_kHz = %lf\n", guard_kHz);
	print_ltenr_log("\tPRB_ID\tF_Low\tF_High\tF_center\n");

	phy->prbList = calloc(ca->PRBCount, sizeof * phy->prbList);
	for (i = 0; i < ca->PRBCount; i++)
	{
		phy->prbList[i].prbId = i + 1;
		phy->prbList[i].lFrequency_MHz = ca->Flow_MHZ + guard_kHz * 0.001 + prbBandwidth_kHz * i * 0.001;
		phy->prbList[i].uFrequency_MHz = phy->prbList[i].lFrequency_MHz + prbBandwidth_kHz * 0.001;
		phy->prbList[i].centralFrequency_MHz = (phy->prbList[i].lFrequency_MHz + phy->prbList[i].uFrequency_MHz) / 2.0;
		phy->prbList[i].prbBandwidth_MHz = prbBandwidth_kHz * 0.001;
		print_ltenr_log("\t%3d\t%lf\t%lf\t%lf\n",
						i + 1,
						phy->prbList[i].lFrequency_MHz,
						phy->prbList[i].uFrequency_MHz,
						phy->prbList[i].centralFrequency_MHz);
	}
	print_ltenr_log("\n\n");
}
#pragma endregion

#pragma region FRAME
static void LTENR_addStartFrameEvent(NETSIM_ID gnbId, NETSIM_ID gnbIf, double time, int CA_ID)
{
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = time;
	pevent.nDeviceId = gnbId;
	pevent.nDeviceType = DEVICE_TYPE(gnbId);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = gnbIf;
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(gnbId, gnbIf);
	LTENR_SET_SUBEVENT(pd, &pevent, LTENR_SUBEVENT_PHY_STARTFRAME);
#pragma warning (disable : 4312)
	pevent.szOtherDetails = (void*)CA_ID;
#pragma warning (default : 4312)
	UINT id = fnpAddEvent(&pevent);
}

void LTENR_addSendReceive(double time) {
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = time;

	pevent.nEventType = TIMER_EVENT;
	pevent.nProtocolId = MAC_PROTOCOL_LTE_NR;
	pevent.nSubEventType = LTENR_SUBEVENT_PHY_HANDLE_SEND_RECEIVE;
	UINT id = fnpAddEvent(&pevent);
}

static void LTENR_resetFrame(ptrLTENR_GNBPHY phy, int CA_ID)
{
	ptrLTENR_FRAMEINFO info = phy->frameInfo[CA_ID];
	ptrLTENR_SPECTRUMCONFIG sc = phy->spectrumConfig;

	info->frameId++;
	info->frameStartTime = pstruEventDetails->dEventTime;
	info->frameEndTime = pstruEventDetails->dEventTime + sc->frameDuration;

	//reset slot
	info->slotEndTime = 0;
	info->slotId = 0;
	info->slotStartTime = 0;

	//reset subframe
	info->subFrameEndTime = 0;
	info->subFrameId = 0;
	info->subFrameStartTime = 0;
}

int frame_count = 1;

void LTENR_handleSendReceive()
{
	//fprintf(stderr, "\nStarting frame number %d\n", frame_count);
	//fprintf(stderr, "================================================\n", frame_count);

	double sum_throughput = 0.0;
	int p = 0;

	double* throughputs;
	throughputs = malloc(num_UEs * sizeof(double));
	for (int mp = 0; mp < num_UEs; mp++)
		throughputs[mp] = 0.0;

	double* delta_P;
	delta_P = malloc(num_gNBs * sizeof(double));
	for (int mp = 0; mp < num_gNBs; mp++)
		delta_P[mp] = 0.0;

	double* sinr_list;
	sinr_list = malloc(num_UEs * sizeof(double));
	for (int mp = 0; mp < num_UEs; mp++)
		sinr_list[mp] = 0.0;

	double* interference_list;
	interference_list = malloc(num_UEs * sizeof(double));
	for (int mp = 0; mp < num_UEs; mp++)
		interference_list[mp] = 0.0;

	double* MCS_list;
	MCS_list = malloc(num_UEs * sizeof(double));
	for (int mp = 0; mp < num_UEs; mp++)
		MCS_list[mp] = 0.0;

	//gNB and associated UE list
	double frameDuration = 0;
	ptrLTENR_Cell_List list = NULL;
	ptrLTENR_SPECTRUMCONFIG sc;
	ptrLTENR_PDCPVAR pdcp;
	double pdsch_throughput;
	int layerCount;
	double sinr;


	list = gnbDCList;////gNBlist	
	while (list)
	{
		ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(list->d, list->in);
		ptrLTENR_ASSOCIATEDUEPHYINFO info = phy->associatedUEPhyInfo;
		while (info)
		{
			if (info->isAssociated)
			{
				ptrLTENR_PROPAGATIONINFO pinfo;
				for (UINT CAid = 0; CAid < phy->ca_count; CAid++)
				{
					layerCount = info->propagationInfo[CAid]->downlink.layerCount;

					for (UINT i = 0; i < layerCount; i++)
					{
						sc = phy->spectrumConfig;
						pdcp = LTENR_PDCP_GET(info->ueId, info->ueIf);
						frameDuration = sc->frameDuration;
						sinr = info->propagationInfo[CAid]->downlink.SINR_db[i];

						pdsch_throughput = (pdcp->TotalPDSCHBytes * 8) / (3 * sc->frameDuration);

						valuesToSend_Python->throughputs_NETSIM[p] = pdsch_throughput;
						valuesToSend_Python->SINRlist[p] = sinr;
						sinr_list[p] = sinr;
						throughputs[p] = pdsch_throughput;

						interference_list[p] = info->propagationInfo[CAid]->downlink.InterferencePower_dbm[i];
						//MCS_list[p] = info->downlinkAMCInfo[CAid][i]->mcsTable.mcsIndex;
						p++;
						sum_throughput += pdsch_throughput;

						pdcp->TotalPDSCHBytes = 0;
					}
				}
			}
			valuesToSend_Python->reward = sum_throughput;
			LTENR_ASSOCIATEDUEPHYINFO_NEXT(info);
		}
		list = LTENR_Cell_List_NEXT(list);
	}


	//LTENR_Python_Log("%lf,Before,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf", pstruEventDetails->dEventTime, throughputs[0], sinr_list[0], throughputs[1], sinr_list[1], throughputs[2], sinr_list[2], throughputs[3], sinr_list[3], throughputs[4], sinr_list[4], throughputs[5], sinr_list[5], valuesToSend_Python->reward, MCS_list[0], MCS_list[1], MCS_list[2], MCS_list[3], MCS_list[4], MCS_list[5], interference_list[0], interference_list[1], interference_list[2], interference_list[3], interference_list[4], interference_list[5]);


	handle_Send_Receive(valuesToSend_Python, receivedValue_From_Python);
	handle_Send_Receive(valuesToSend_Python, receivedValue_From_Python);

	list = gnbDCList;////gNBlist	

	int flag = true;

	p = 0;
	while (list)
	{
		ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(list->d, list->in);
		ptrLTENR_ASSOCIATEDUEPHYINFO info = phy->associatedUEPhyInfo;
		while (info)
		{
			for (UINT CAid = 0; CAid < phy->ca_count; CAid++)
			{
				delta_P[p] = receivedValue_From_Python->gNBlist[p] - info->propagationInfo[CAid]->downlink.txPower_dbm;
				info->propagationInfo[CAid]->downlink.txPower_dbm = receivedValue_From_Python->gNBlist[p];
			}
			LTENR_ASSOCIATEDUEPHYINFO_NEXT(info);
		}


		list = LTENR_Cell_List_NEXT(list);
		p++;
		fprintf(stderr, "\n");
	}

	list = gnbDCList;////gNBlist	
	//update rx powers for all gNB-UE pairs
	//while (list)
	//{
	//	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(list->d, list->in);
	//	ptrLTENR_ASSOCIATEDUEPHYINFO info = phy->associatedUEPhyInfo;
	//	while (info)
	//	{
	//		{
	//			for (UINT CAid = 0; CAid < phy->ca_count; CAid++)
	//			{
	//				layerCount = info->propagationInfo[CAid]->downlink.layerCount;

	//				for (UINT i = 0; i < layerCount; i++)
	//				{
	//					LTENR_PHY_GetDownlinkSpectralEfficiency(info->propagationInfo[CAid], i, CAid);
	//				}
	//			}
	//		}
	//		LTENR_ASSOCIATEDUEPHYINFO_NEXT(info);
	//	}
	//	list = LTENR_Cell_List_NEXT(list);
	//}	

	//int counter = 0;
	//list = gnbDCList;////gNBlist	
	////recompute interference using updated rx powers for all associated gNB-UE pairs
	//while (list)
	//{
	//	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(list->d, list->in);
	//	ptrLTENR_ASSOCIATEDUEPHYINFO info = phy->associatedUEPhyInfo;
	//	while (info)
	//	{
	//		if (info->isAssociated)
	//		{
	//			for (UINT CAid = 0; CAid < phy->ca_count; CAid++)
	//			{
	//				layerCount = info->propagationInfo[CAid]->downlink.layerCount;

	//				for (UINT i = 0; i < layerCount; i++)
	//				{
	//					//LTENR_PHY_setAMCInfo(phy, info, CAid);
	//					LTENR_PHY_GetDownlinkSpectralEfficiency(info->propagationInfo[CAid], i, CAid);
	//					sinr_list[counter] = info->propagationInfo[CAid]->downlink.SINR_db[i];
	//					MCS_list[counter] = LTENR_PHY_GetDownlinkBeamFormingValue(info->propagationInfo[CAid], i);
	//					//MCS_list[p] = info->downlinkAMCInfo[CAid][i]->mcsTable.mcsIndex;
	//					interference_list[counter] = info->propagationInfo[CAid]->downlink.InterferencePower_dbm[i];
	//					counter++;
	//				}
	//			}
	//		}
	//		LTENR_ASSOCIATEDUEPHYINFO_NEXT(info);
	//	}
	//	list = LTENR_Cell_List_NEXT(list);
	//}

	//LTENR_Python_Log("%lf,After,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf", pstruEventDetails->dEventTime, throughputs[0], sinr_list[0], throughputs[1], sinr_list[1], throughputs[2], sinr_list[2], throughputs[3], sinr_list[3], throughputs[4], sinr_list[4], throughputs[5], sinr_list[5], valuesToSend_Python->reward, MCS_list[0], MCS_list[1], MCS_list[2], MCS_list[3], MCS_list[4], MCS_list[5], interference_list[0], interference_list[1], interference_list[2], interference_list[3], interference_list[4], interference_list[5]);

	//LTENR_addSendReceive(pstruEventDetails->dEventTime + (frameDuration*3));


	free(throughputs);
	free(delta_P);
	free(sinr_list);
	free(interference_list);
	free(MCS_list);

	//fprintf(stderr, "\n================================================\n", frame_count);
	//fprintf(stderr, "\Ending frame number %d\n", frame_count);

	//frame_count++;
}
void LTENR_handleStartFrameEvent()
{
	NETSIM_ID gnbId = pstruEventDetails->nDeviceId;
	NETSIM_ID gnbIf = pstruEventDetails->nInterfaceId;
#pragma warning (disable : 4047)
	int CA_ID = pstruEventDetails->szOtherDetails;
#pragma warning (default : 4047)
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);
	
	LTENR_resetFrame(phy, CA_ID);
	print_ltenr_log("Starting new frame for gNB %d:%d\n", gnbId, gnbIf);
	print_ltenr_log("CA_ID for Frame = %d\n", CA_ID);
	print_ltenr_log("\tFrame Id = %d\n", phy->frameInfo[CA_ID]->frameId);
	print_ltenr_log("\tFrame start time (탎) = %lf\n", phy->frameInfo[CA_ID]->frameStartTime);
	print_ltenr_log("\tFrame end time (탎) = %lf\n", phy->frameInfo[CA_ID]->frameEndTime);

	LTENR_addStartFrameEvent(gnbId, gnbIf,
							 phy->frameInfo[CA_ID]->frameEndTime, CA_ID);

	LTENR_addStartSubFrameEvent(gnbId, gnbIf,
								phy->frameInfo[CA_ID]->frameStartTime, CA_ID);
}
#pragma endregion

#pragma region SUBFRAME
static void LTENR_addStartSubFrameEvent(NETSIM_ID gnbId, NETSIM_ID gnbIf, double time, int CA_ID)
{
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = time;
	pevent.nDeviceId = gnbId;
	pevent.nDeviceType = DEVICE_TYPE(gnbId);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = gnbIf;
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(gnbId, gnbIf);
	LTENR_SET_SUBEVENT(pd, &pevent, LTENR_SUBEVENT_PHY_STARTSUBFRAME);
#pragma warning (disable : 4312)
	pevent.szOtherDetails = (void*)CA_ID;
#pragma warning (default : 4312)
	fnpAddEvent(&pevent);
}

static void LTENR_resetSubFrame(ptrLTENR_GNBPHY phy, int CA_ID)
{
	ptrLTENR_FRAMEINFO info = phy->frameInfo[CA_ID];
	ptrLTENR_SPECTRUMCONFIG sc = phy->spectrumConfig;

	//reset slot
	info->slotEndTime = 0;
	info->slotId = 0;
	info->slotStartTime = 0;

	//reset subframe
	info->subFrameStartTime = pstruEventDetails->dEventTime;
	info->subFrameEndTime = pstruEventDetails->dEventTime + sc->subFrameDuration;
	info->subFrameId++;
	
}

void LTENR_handleStartSubFrameEvent()
{
	NETSIM_ID gnbId = pstruEventDetails->nDeviceId;
	NETSIM_ID gnbIf = pstruEventDetails->nInterfaceId;
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);
#pragma warning (disable : 4047)
	int CA_ID = pstruEventDetails->szOtherDetails;
#pragma warning (default : 4047)
	
	LTENR_resetSubFrame(phy,CA_ID);
	print_ltenr_log("Starting new sub frame for gNB %d:%d\n", gnbId, gnbIf);
	print_ltenr_log("CA_ID for SubFrame = %d\n", CA_ID);
	print_ltenr_log("\tFrame Id = %d\n", phy->frameInfo[CA_ID]->frameId);
	print_ltenr_log("\tSubFrame Id = %d\n", phy->frameInfo[CA_ID]->subFrameId);
	print_ltenr_log("\tSubFrame start time (탎) = %lf\n", phy->frameInfo[CA_ID]->subFrameStartTime);
	print_ltenr_log("\tSubFrame end time (탎) = %lf\n", phy->frameInfo[CA_ID]->subFrameEndTime);

	if (phy->frameInfo[CA_ID]->subFrameId != phy->spectrumConfig->subFramePerFrame)
		LTENR_addStartSubFrameEvent(gnbId, gnbIf,
									phy->frameInfo[CA_ID]->subFrameEndTime, CA_ID);

	LTENR_addStartSlotEvent(gnbId, gnbIf,
								phy->frameInfo[CA_ID]->subFrameStartTime, CA_ID);
}
#pragma endregion

#pragma region SLOT
static void LTENR_addStartSlotEvent(NETSIM_ID gnbId, NETSIM_ID gnbIf, double time, int CA_ID)
{
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = time;
	pevent.nDeviceId = gnbId;
	pevent.nDeviceType = DEVICE_TYPE(gnbId);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = gnbIf;
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(gnbId, gnbIf);
	LTENR_SET_SUBEVENT(pd, &pevent, LTENR_SUBEVENT_PHY_STARTSLOT);
#pragma warning (disable : 4312)
	pevent.szOtherDetails = (PVOID)CA_ID;
#pragma warning (default : 4312)	
	fnpAddEvent(&pevent);
}

static void LTENR_resetSlot(ptrLTENR_GNBPHY phy, int CA_ID)
{
	ptrLTENR_FRAMEINFO info = phy->frameInfo[CA_ID];
	ptrLTENR_CA ca = phy->spectrumConfig->CA[CA_ID];

	//reset slot
	info->slotId++;
	info->slotStartTime = pstruEventDetails->dEventTime;
	info->slotEndTime = pstruEventDetails->dEventTime + ca->slotDuration_us;

	if (ca->configSlotType == SLOT_MIXED)
	{
		if (ca->totalSlotCount && (ca->dlSlotCount*1.0) / ca->totalSlotCount < ca->dlSlotRatio)
		{
			ca->dlSlotCount++;
			info->slotType = SLOT_DOWNLINK;
		}
		else
			info->slotType = SLOT_UPLINK;

		ca->totalSlotCount++;
	}
	else
		info->slotType = ca->configSlotType;
}

void LTENR_handleStartSlotEvent()
{
	NETSIM_ID gnbId = pstruEventDetails->nDeviceId;
	NETSIM_ID gnbIf = pstruEventDetails->nInterfaceId;
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);

#pragma warning (disable : 4047)
	int CA_ID = pstruEventDetails->szOtherDetails;
#pragma warning (default : 4047)

	ptrLTENR_CA ca = phy->spectrumConfig->CA[CA_ID];

	LTENR_resetSlot(phy,CA_ID);
	print_ltenr_log("Starting new slot for gNB %d:%d\n", gnbId, gnbIf);
	print_ltenr_log("CA_ID for Slot = %d\n", CA_ID);
	print_ltenr_log("\tFrame Id = %d\n", phy->frameInfo[CA_ID]->frameId);
	print_ltenr_log("\tSubFrame Id = %d\n", phy->frameInfo[CA_ID]->subFrameId);
	print_ltenr_log("\tSlot Id = %d\n", phy->frameInfo[CA_ID]->slotId);
	print_ltenr_log("\tSlot start time (탎) = %lf\n", phy->frameInfo[CA_ID]->slotStartTime);
	print_ltenr_log("\tslot end time (탎) = %lf\n", phy->frameInfo[CA_ID]->slotEndTime);
	print_ltenr_log("\tSlot type = %s\n", strLTENR_SLOTTYPE[phy->frameInfo[CA_ID]->slotType]);

	phy->currentFrameInfo = phy->frameInfo[CA_ID];
	phy->currentFrameInfo->Current_CA_ID = CA_ID;
	if (phy->frameInfo[CA_ID]->slotId != ca->slotPerSubframe)
		LTENR_addStartSlotEvent(gnbId, gnbIf,
								phy->frameInfo[CA_ID]->slotEndTime, CA_ID);

	ptrLTENR_ASSOCIATEDUEPHYINFO info = phy->associatedUEPhyInfo;
	while (info)
	{
		if (info->isAssociated)
		{
			for (NETSIM_ID i = 0; i < phy->ca_count; i++)
				LTENR_PHY_setAMCInfo(phy, info, i);

		}
		info = LTENR_ASSOCIATEDUEPHYINFO_NEXT(info);

	}

	ptrLTENR_GNBMAC mac = LTENR_GNBMAC_GET(gnbId, gnbIf);
	ptrLTENR_SCHEDULERINFO schedulerInfo = mac->schedulerInfo[CA_ID];
	LTENR_NotifyMACForStartingSlot();
	phy->frameInfo[CA_ID]->prevSlotType = phy->frameInfo[CA_ID]->slotType;
}
#pragma endregion

#pragma region PHY_PROPAGATION_INTERFACE
static ptrLTENR_PROPAGATIONINFO LTENR_PHY_initPropagationInfo(ptrLTENR_GNBPHY phy,
															  NETSIM_ID ueId, NETSIM_ID ueIf,
															  int CA_ID)
{
	ptrLTENR_UEPHY uePhy = LTENR_UEPHY_GET(ueId, ueIf);
	ptrLTENR_PROPAGATIONINFO info = calloc(1, sizeof * info);

	ptrLTENR_CA ca = phy->spectrumConfig->CA[CA_ID];

	info->gnbId = phy->gnbId;
	info->gnbIf = phy->gnbIf;
	memcpy(&info->gnbPos, DEVICE_POSITION(phy->gnbId), sizeof info->gnbPos);
	memcpy(&info->uePos, DEVICE_POSITION(ueId), sizeof info->uePos);
	info->ueId = ueId;
	info->ueIf = ueIf;
	info->bandwidth_MHz = ca->channelBandwidth_mHz;
	info->centralFrequency_MHz = (ca->Fhigh_MHz + ca->Flow_MHZ) / 2.0;
	UINT ulLayerCount = LTENR_PHY_GET_ULLAYER_COUNT_FOR_NONASSOCIATED(phy, uePhy);
	UINT dlLayerCount = LTENR_PHY_GET_DLLAYER_COUNT_FOR_NONASSOCIATED(phy, uePhy);
	info->downlink.txPower_dbm = MW_TO_DBM(DBM_TO_MW(phy->propagationConfig->txPower_dbm) / dlLayerCount);
	info->uplink.txPower_dbm = MW_TO_DBM(DBM_TO_MW(uePhy->propagationConfig->txPower_dbm) / ulLayerCount);
	info->downlink.totaltxpower_dbm = phy->propagationConfig->txPower_dbm;
	info->uplink.totaltxpower_dbm = uePhy->propagationConfig->txPower_dbm;

	info->propagationConfig = phy->propagationConfig;
	info->propagationConfig->UE_height = uePhy->propagationConfig->UE_height;

	info->downlink.EB_by_N0 = calloc(dlLayerCount, sizeof * info->downlink.EB_by_N0);
	info->downlink.SNR_db = calloc(dlLayerCount, sizeof * info->downlink.SNR_db);
	info->downlink.SINR_db = calloc(dlLayerCount, sizeof * info->downlink.SINR_db);
	info->downlink.InterferencePower_dbm = calloc(dlLayerCount, sizeof * info->downlink.InterferencePower_dbm);
	info->downlink.spectralEfficiency = calloc(dlLayerCount, sizeof * info->downlink.spectralEfficiency);
	info->downlink.rxPower_dbm = calloc(dlLayerCount, sizeof * info->downlink.rxPower_dbm);
	info->downlink.layerCount = dlLayerCount;
	info->downlink.txAntennaCount = phy->antenna->txAntennaCount;
	info->downlink.rxAntennaCount = uePhy->antenna->rxAntennaCount;

	info->uplink.EB_by_N0 = calloc(ulLayerCount, sizeof * info->uplink.EB_by_N0);
	info->uplink.SNR_db = calloc(ulLayerCount, sizeof * info->uplink.SNR_db);
	info->uplink.SINR_db = calloc(ulLayerCount, sizeof * info->uplink.SINR_db);
	info->uplink.InterferencePower_dbm = calloc(ulLayerCount, sizeof * info->uplink.InterferencePower_dbm);
	info->uplink.spectralEfficiency = calloc(ulLayerCount, sizeof * info->uplink.spectralEfficiency);
	info->uplink.rxPower_dbm = calloc(ulLayerCount, sizeof * info->uplink.rxPower_dbm);
	info->uplink.layerCount = ulLayerCount;
	info->uplink.txAntennaCount = uePhy->antenna->txAntennaCount;
	info->uplink.rxAntennaCount = phy->antenna->rxAntennaCount;
	if(info->propagationConfig->fastFadingModel == LTENR_FASTFADING_MODEL_AWGN_WITH_RAYLEIGH_FADING)
		LTENR_BeamForming_Init(info);
	return info;
}

static void LTENR_PHY_updatePropagationInfo(ptrLTENR_PROPAGATIONINFO info)
{
	memcpy(&info->gnbPos, DEVICE_POSITION(info->gnbId), sizeof info->gnbPos);
	memcpy(&info->uePos, DEVICE_POSITION(info->ueId), sizeof info->uePos);
}

#define BOLTZMANN 1.38064852e-23 //m2kgs-2K-1
#define TEMPERATURE 300 //kelvin
static double LTENR_PHY_calculateThermalNoise(double bandwidth)
{
	double noise = BOLTZMANN * TEMPERATURE * bandwidth * 1000000; //in W
	noise *= 1000; // in mW
	double noise_dbm = MW_TO_DBM(noise);
	return noise_dbm;
}

//static double LTENR_PHY_GetDownlinkBeamFormingValue(ptrLTENR_PROPAGATIONINFO info, int layerId)
//{
//	if (info->propagationConfig->fastFadingModel == LTENR_FASTFADING_MODEL_NO_FADING_MIMO_ARRAY_GAIN)
//	{
//		UINT MaxAntenaCount = max(info->downlink.rxAntennaCount, info->downlink.txAntennaCount);
//		info->downlink.ArrayGain = 10.0 * log10(MaxAntenaCount);
//		return info->downlink.ArrayGain;
//	}
//	if(info->propagationConfig->fastFadingModel == LTENR_FASTFADING_MODEL_AWGN_WITH_RAYLEIGH_FADING)
//	{
//		return LTENR_BeamForming_Downlink_GetValue(info, layerId);
//	}
//	else
//		return 0;
//}
static double LTENR_PHY_GetUplinkBeamFormingValue(ptrLTENR_PROPAGATIONINFO info, int layerId)
{
	if (info->propagationConfig->fastFadingModel == LTENR_FASTFADING_MODEL_NO_FADING_MIMO_ARRAY_GAIN) {
		UINT MaxAntenaCount = max(info->uplink.rxAntennaCount, info->uplink.txAntennaCount);
		info->uplink.ArrayGain = 10.0 * log10(MaxAntenaCount);
		return info->uplink.ArrayGain;
	}
	if (info->propagationConfig->fastFadingModel == LTENR_FASTFADING_MODEL_AWGN_WITH_RAYLEIGH_FADING) {
		return LTENR_BeamForming_Uplink_GetValue(info, layerId);
	}
	else
		return 0;
}

static double LTENR_PHY_GetDownlinkAnalogBeamFormingValue(ptrLTENR_PROPAGATIONINFO info)
{
	info->downlink.SSB_AnalogBeamformingGain_dB = 10.0 * log10(info->downlink.txAntennaCount * info->downlink.rxAntennaCount);
	return info->downlink.SSB_AnalogBeamformingGain_dB;
}

static void LTENR_PHY_calculateSNR(double rxPower_dbm, double thermalNoise_dbm,
								   double* ebByN0,double* snrDB)
{
	double p = DBM_TO_MW(rxPower_dbm);
	double n = DBM_TO_MW(thermalNoise_dbm);
	*ebByN0 = p / n;
	*snrDB = MW_TO_DBM(*ebByN0);
}

static void LTENR_PHY_calculateSINR(double rxPower_dbm, double thermalNoise_dbm, double InterferencePower_dbm,
	double* sinrDB, double* ebByN0)
{
	double p = DBM_TO_MW(rxPower_dbm);
	double n = DBM_TO_MW(thermalNoise_dbm);
	double I = DBM_TO_MW(InterferencePower_dbm);
	*ebByN0 = p / (n+I);
	*sinrDB = MW_TO_DBM(*ebByN0);
}
double LTENR_PHY_GetDownlinkSpectralEfficiency(ptrLTENR_PROPAGATIONINFO info, int layerId,int CA_ID)
{
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(info->gnbId, info->gnbIf);
	ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo = LTENR_ASSOCIATEDUEPHYINFO_GET(info->gnbId, info->gnbIf, info->ueId, info->ueIf);
	double BeamFormingGain_dbm = assocInfo->isAssociated ? LTENR_PHY_GetDownlinkBeamFormingValue(info, layerId) : 0.0;
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(info->gnbId, info->gnbIf);
	info->downlink.antennaGain = LTENR_PHY_GetAntennaGain(info, phy->antenna);
	info->downlink.rxPower_dbm[layerId] = info->downlink.txPower_dbm - info->dTotalLoss + info->downlink.antennaGain;

	info->downlink.thermalNoise = LTENR_PHY_calculateThermalNoise(info->bandwidth_MHz);
	LTENR_Calculate_Interference(info, info->downlink.rxPower_dbm[layerId], &info->downlink.InterferencePower_dbm[layerId],layerId, CA_ID, true);
	info->downlink.rxPower_dbm[layerId] = info->downlink.txPower_dbm - info->dTotalLoss + BeamFormingGain_dbm + info->downlink.antennaGain;
	LTENR_PHY_calculateSNR(info->downlink.rxPower_dbm[layerId],
						   info->downlink.thermalNoise,
						   &info->downlink.EB_by_N0[layerId],
						   &info->downlink.SNR_db[layerId]);
	if (info->propagationConfig->DownlinkInterferenceModel == LTENR_INTERFERENCE_OVER_THERMAL_INTERFERENCE)
	{
		ptrLTENR_PROPAGATIONCONFIG propagation = info->propagationConfig;
		info->downlink.SINR_db[layerId] = info->downlink.SNR_db[layerId] - propagation->Downlink_IoT_dB;
		if (propagation->Downlink_IoT_dB < 0)
		{
			fnNetSimError("Uplink Interference over thermal is less than 0. Value is %lf. Resetting it to 0.\n",
				propagation->Downlink_IoT_dB);
			propagation->Downlink_IoT_dB = 0;
		}
		if (propagation->Downlink_IoT_dB == 0)
			info->downlink.InterferencePower_dbm[layerId] = NEGATIVE_DBM;
		else
			info->downlink.InterferencePower_dbm[layerId] = 10 * log10(DBM_TO_MW(info->downlink.thermalNoise) * (pow(10, propagation->Downlink_IoT_dB) - 1));
		info->downlink.EB_by_N0[layerId] = DBM_TO_MW(info->downlink.SINR_db[layerId]);
	}
	else
	{
	LTENR_PHY_calculateSINR(info->downlink.rxPower_dbm[layerId],
		info->downlink.thermalNoise,
		info->downlink.InterferencePower_dbm[layerId],
		&info->downlink.SINR_db[layerId], &info->downlink.EB_by_N0[layerId]);
	}
	info->downlink.spectralEfficiency[layerId] = pd->alpha*
													log2(1 + info->downlink.EB_by_N0[layerId]);
	return info->downlink.spectralEfficiency[layerId];
}

double LTENR_PHY_GetUplinkSpectralEfficiency(ptrLTENR_PROPAGATIONINFO info, int layerId, int CA_ID)
{
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(info->gnbId, info->gnbIf);
	ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo = LTENR_ASSOCIATEDUEPHYINFO_GET(info->gnbId, info->gnbIf, info->ueId, info->ueIf);
	double BeamFormingGain_dbm = assocInfo->isAssociated ? LTENR_PHY_GetUplinkBeamFormingValue(info, layerId) : 0.0;
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(info->gnbId, info->gnbIf);
	info->uplink.antennaGain = LTENR_PHY_GetAntennaGain(info, phy->antenna);
	info->uplink.rxPower_dbm[layerId] = info->uplink.txPower_dbm - info->dTotalLoss + info->uplink.antennaGain;
	info->uplink.thermalNoise = LTENR_PHY_calculateThermalNoise(info->bandwidth_MHz);
	LTENR_Calculate_Interference(info, info->uplink.rxPower_dbm[layerId], &info->uplink.InterferencePower_dbm[layerId], layerId, CA_ID, false);
	info->uplink.rxPower_dbm[layerId] = info->uplink.txPower_dbm - info->dTotalLoss + BeamFormingGain_dbm + info->uplink.antennaGain;
	LTENR_PHY_calculateSNR(info->uplink.rxPower_dbm[layerId],
						   info->uplink.thermalNoise,
						   &info->uplink.EB_by_N0[layerId],
						   &info->uplink.SNR_db[layerId]);
	ptrLTENR_PROPAGATIONCONFIG propagation = info->propagationConfig;
	info->uplink.SINR_db[layerId] = info->uplink.SNR_db[layerId] - propagation->Uplink_IoT_dB;
	if (propagation->Uplink_IoT_dB < 0)
	{
		fnNetSimError("Uplink Interference over thermal is less than 0. Value is %lf. Resetting it to 0.\n",
			propagation->Uplink_IoT_dB);
		propagation->Uplink_IoT_dB = 0;
			}
	if (propagation->Uplink_IoT_dB == 0)
		info->uplink.InterferencePower_dbm[layerId] = NEGATIVE_DBM;
	else
		info->uplink.InterferencePower_dbm[layerId] = 10 * log10(DBM_TO_MW(info->uplink.thermalNoise) * (pow(10, propagation->Uplink_IoT_dB) - 1));
	info->uplink.EB_by_N0[layerId] = DBM_TO_MW(info->uplink.SINR_db[layerId]);
	info->uplink.spectralEfficiency[layerId] = pd->alpha * 
												log2(1 + info->uplink.EB_by_N0[layerId]);
	return info->uplink.spectralEfficiency[layerId];
}

static void LTENR_PHY_calculateSpectralEfficiency(ptrLTENR_GNBPHY phy,
												  NETSIM_ID ueId, NETSIM_ID ueIf,
												  int CA_ID)
{
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(phy->gnbId, phy->gnbIf);
	ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo = LTENR_ASSOCIATEDUEPHYINFO_GET(phy->gnbId, phy->gnbIf, ueId, ueIf);
	if (!assocInfo->propagationInfo[CA_ID])
		assocInfo->propagationInfo[CA_ID] = LTENR_PHY_initPropagationInfo(phy, ueId, ueIf, CA_ID);
	else
		LTENR_PHY_updatePropagationInfo(assocInfo->propagationInfo[CA_ID]);

	ptrLTENR_PROPAGATIONINFO info = assocInfo->propagationInfo[CA_ID];
	if (!info)
	{
		fnNetSimError("Propagation info is NULL for CA_ID = %d for gNB %d-%d --> UE %d-%d.\n",
					  CA_ID,
					  phy->gnbId, phy->gnbIf,
					  assocInfo->ueId, assocInfo->ueIf);
		return;
	}

	//Call propagation model
	print_ltenr_log("\n\tCarrier Id = %d\n", CA_ID);
	LTENR_Propagation_TotalLoss(info);

	if (DEVICE_MACPROTOCOL(info->gnbId, info->gnbIf) == MAC_PROTOCOL_LTE_NR)
	{
		info->downlink.antennaGain = LTENR_PHY_GetAntennaGain(info, phy->antenna);
		info->downlink.SSB_rxPower_dBm = info->downlink.totaltxpower_dbm - info->dTotalLoss + LTENR_PHY_GetDownlinkAnalogBeamFormingValue(info)
			+ info->downlink.antennaGain;
	}
	else if (DEVICE_MACPROTOCOL(info->gnbId, info->gnbIf) == MAC_PROTOCOL_LTE)
		info->downlink.SSB_rxPower_dBm = info->downlink.totaltxpower_dbm - info->dTotalLoss;
	else
		fnNetSimError("Unknown MAC protocol for gNB/eNB %d-%d", info->gnbId, info->gnbIf); 
	
	info->downlink.thermalNoise = LTENR_PHY_calculateThermalNoise(info->bandwidth_MHz);
	LTENR_PHY_calculateSNR(info->downlink.SSB_rxPower_dBm,
		info->downlink.thermalNoise,
		&info->downlink.SSB_EB_by_N0,
		&info->downlink.SSB_SNR_dB);
	
	for (UINT i = 0; i < info->downlink.layerCount; i++)
	{
		double BeamFormingGain_dbm = assocInfo->isAssociated ? LTENR_PHY_GetDownlinkBeamFormingValue(info, i) : 0.0;
		info->downlink.rxPower_dbm[i] = info->downlink.txPower_dbm - info->dTotalLoss + info->downlink.antennaGain;
		info->downlink.thermalNoise = LTENR_PHY_calculateThermalNoise(info->bandwidth_MHz);
		LTENR_Calculate_Interference(info, info->downlink.rxPower_dbm[i], &info->downlink.InterferencePower_dbm[i], i, CA_ID, true);
		info->downlink.rxPower_dbm[i] = info->downlink.txPower_dbm - info->dTotalLoss + BeamFormingGain_dbm + info->downlink.antennaGain;
		LTENR_PHY_calculateSNR(info->downlink.rxPower_dbm[i],
							   info->downlink.thermalNoise,
							   &info->downlink.EB_by_N0[i],
							   &info->downlink.SNR_db[i]);
		LTENR_PHY_calculateSINR(info->downlink.rxPower_dbm[i],
			info->downlink.thermalNoise,
			info->downlink.InterferencePower_dbm[i],
			&info->downlink.SINR_db[i], &info->downlink.EB_by_N0[i]);
		info->downlink.spectralEfficiency[i] = pd->alpha * 
												log2(1 + info->downlink.EB_by_N0[i]);

		print_ltenr_log("\tDownlink for Layer %d\n", i + 1);
		print_ltenr_log("\t\t Thermal Noise = %lf dBm\n", info->downlink.thermalNoise);
		print_ltenr_log("\t\t Interference = %lf dBm\n", info->downlink.InterferencePower_dbm[i]);
		print_ltenr_log("\t\t Signal to Noise Ratio (SNR) = %lf dB\n", info->downlink.SNR_db[i]);
		print_ltenr_log("\t\t Signal to Interference & Noise Ratio (SINR) = %lf dB\n", info->downlink.SINR_db[i]);
		print_ltenr_log("\t\t Spectral Efficiency = %lf dB\n", info->downlink.spectralEfficiency[i]);
		print_ltenr_log("\n");
	}

	for (UINT i = 0; i < info->uplink.layerCount; i++)
	{
		double BeamFormingGain_dbm = assocInfo->isAssociated ? LTENR_PHY_GetUplinkBeamFormingValue(info, i) : 0.0;
		info->uplink.rxPower_dbm[i] = info->uplink.txPower_dbm - info->dTotalLoss;
		info->uplink.thermalNoise = LTENR_PHY_calculateThermalNoise(info->bandwidth_MHz);
		LTENR_Calculate_Interference(info, info->uplink.rxPower_dbm[i], &info->uplink.InterferencePower_dbm[i], i, CA_ID, false);
		info->uplink.rxPower_dbm[i] = info->uplink.txPower_dbm - info->dTotalLoss + BeamFormingGain_dbm + info->uplink.antennaGain;
		LTENR_PHY_calculateSNR(info->uplink.rxPower_dbm[i],
			info->uplink.thermalNoise,
			&info->uplink.EB_by_N0[i],
			&info->uplink.SNR_db[i]);

		ptrLTENR_PROPAGATIONCONFIG propagation = info->propagationConfig;
		info->uplink.SINR_db[i] = info->uplink.SNR_db[i] - propagation->Uplink_IoT_dB;
		if (propagation->Uplink_IoT_dB < 0)
		{
			fnNetSimError("Uplink Interference over thermal is less than 0. Value is %lf. Resetting it to 0.\n",
				propagation->Uplink_IoT_dB);
			propagation->Uplink_IoT_dB = 0;
		}
		if (propagation->Uplink_IoT_dB == 0)
			info->uplink.InterferencePower_dbm[i] = NEGATIVE_DBM;
		else
			info->uplink.InterferencePower_dbm[i] = 10 * log10(DBM_TO_MW(info->uplink.thermalNoise) * (pow(10, propagation->Uplink_IoT_dB) - 1));
		info->uplink.EB_by_N0[i] = DBM_TO_MW(info->uplink.SINR_db[i]);
		info->uplink.spectralEfficiency[i] = pd->alpha *
			log2(1 + info->uplink.EB_by_N0[i]);

		print_ltenr_log("\tUPlink for Layer %d\n", i + 1);
		print_ltenr_log("\t\t Thermal Noise = %lf dBm\n", info->uplink.thermalNoise);
		print_ltenr_log("\t\t Interference = %lf dBm\n", info->uplink.InterferencePower_dbm[i]);
		print_ltenr_log("\t\t Signal to Noise Ratio (SNR) = %lf dB\n", info->uplink.SNR_db[i]);
		print_ltenr_log("\t\t Signal to Interference & Noise Ratio (SINR) = %lf dB\n", info->uplink.SINR_db[i]);
		print_ltenr_log("\t\t Spectral Efficiency = %lf dB\n", info->uplink.spectralEfficiency[i]);
		print_ltenr_log("\n");
	}
}

double LTENR_PHY_Send_SINR_to_RRC(ptrLTENR_GNBPHY phy)
{
	UINT c = 0;
	double sinr = 0;
	for (UINT i = 0; i < phy->ca_count; i++) 
	{
		ptrLTENR_PROPAGATIONINFO info = phy->associatedUEPhyInfo->propagationInfo[i];
		for (UINT j = 0; j < info->downlink.layerCount; j++)
		{
			c++;
			sinr += info->downlink.SNR_db[j];
		}
	}
	return sinr/c;
}

double LTENR_PHY_Send_RSRP_to_RRC(ptrLTENR_GNBPHY phy) {
	UINT c = 0;
	double rsrp = 0;
	for (UINT i = 0; i < phy->ca_count; i++)
	{
		ptrLTENR_PROPAGATIONINFO info = phy->associatedUEPhyInfo->propagationInfo[i];
		for (UINT j = 0; j < info->downlink.layerCount; j++)
		{
			c++;
			rsrp += info->downlink.rxPower_dbm[j];
		}
	}
	return rsrp / c;
}

static void setAMCInfo(ptrLTENR_GNBPHY phy, NETSIM_ID ueId, NETSIM_ID ueIf, ptrLTENR_AMCINFO amcInfo, bool isUplink, int caId, int layerId)
{
	ptrLTENR_ASSOCIATEDUEPHYINFO info = LTENR_ASSOCIATEDUEPHYINFO_GET(phy->gnbId, phy->gnbIf, ueId, ueIf);

	WriteOLLALog("%.2lf,%s,%s,%d,%d,",
		ldEventTime / MILLISECOND, DEVICE_NAME(phy->gnbId), DEVICE_NAME(ueId), caId + 1, layerId);

	WriteOLLALog("%d,%d,%d,%s,",
		phy->currentFrameInfo->frameId, phy->currentFrameInfo->subFrameId, phy->currentFrameInfo->slotId, isUplink ? "PUSCH" : "PDSCH");

	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(phy->gnbId, phy->gnbIf);
	LTENR_CQITable ot = LTENR_GetCQITableFromSpectralEfficiency(phy->CSIReportConfig.cqiTable,
		amcInfo->SpectralEfficiency);
	double delta = LTENR_OLLA_GetDeltaSINR(phy->gnbId, phy->gnbIf, ueId, ueIf, isUplink, caId, layerId);
	double eff = amcInfo->SpectralEfficiency;
	eff /= pd->alpha;
	double eb = pow(2, eff) - 1;
	double sinr = MW_TO_DBM(eb) - delta;
	eff = pd->alpha * log2(1 + DBM_TO_MW(sinr));
	amcInfo->cqiTable = LTENR_GetCQITableFromSpectralEfficiency(phy->CSIReportConfig.cqiTable,
		eff);
	WriteOLLALog("%lf,%lf,%lf,%d,%d,\n", 
		isUplink ? info->propagationInfo[caId]->uplink.SINR_db[layerId - 1] : info->propagationInfo[caId]->downlink.SINR_db[layerId - 1],
		delta, sinr, ot.CQIIndex, amcInfo->cqiTable.CQIIndex);

	if (amcInfo->cqiTable.CQIIndex != 0)
	{
		if (isUplink)
			amcInfo->mcsTable = LTENR_GetMCSIndexTableFromSpectralEfficiency(phy->PUSCHConfig.mcsTable,
				amcInfo->cqiTable.efficiency);
		else
			amcInfo->mcsTable = LTENR_GetMCSIndexTableFromSpectralEfficiency(phy->PDSCHConfig.mcsTable,
				amcInfo->cqiTable.efficiency);
	}
	else
	{
		if (isUplink)
			amcInfo->mcsTable = ((ptrLTENR_MCSINDEXTABLE)phy->PUSCHConfig.mcsTable)[0];
		else
			amcInfo->mcsTable = ((ptrLTENR_MCSINDEXTABLE)phy->PDSCHConfig.mcsTable)[0];
	}

	
	print_ltenr_log("\t\tSpectral Efficiency = %lf\n", amcInfo->SpectralEfficiency);
	print_ltenr_log("\t\tCQI Table\n");
	print_ltenr_log("\t\t\t%d\t%s\t%d\t%lf\n",
					amcInfo->cqiTable.CQIIndex,
					strPHY_MODULATION[amcInfo->cqiTable.modulation],
					amcInfo->cqiTable.codeRate,
					amcInfo->cqiTable.efficiency);
	print_ltenr_log("\t\tMCS Table\n");
	print_ltenr_log("\t\t\t%d\t%s\t%d\t%lf\t%lf\n",
					amcInfo->mcsTable.mcsIndex,
					strPHY_MODULATION[amcInfo->mcsTable.modulation],
					amcInfo->mcsTable.modulationOrder,
					amcInfo->mcsTable.codeRate,
					amcInfo->mcsTable.spectralEfficiency);
}

static void LTENR_PHY_setAMCInfo(ptrLTENR_GNBPHY phy, ptrLTENR_ASSOCIATEDUEPHYINFO info, int CA_ID)
{
	UINT layerCount;
	ptrLTENR_UEPHY uePhy = LTENR_UEPHY_GET(info->ueId, info->ueIf);

	//Downlink
	layerCount = LTENR_PHY_GET_DLLAYER_COUNT(uePhy);
	for (UINT i = 0; i < layerCount; i++)
	{
		print_ltenr_log("\tAMC info between gNB %d:%d and UE %d:%d, Carrier Id = %d, Layer Id = %d for downlink-\n",
						phy->gnbId, phy->gnbIf,
						info->ueId, info->ueIf,
						CA_ID, i);
		info->downlinkAMCInfo[CA_ID][i]->SpectralEfficiency = LTENR_PHY_GetDownlinkSpectralEfficiency(info->propagationInfo[CA_ID], i, CA_ID);
		setAMCInfo(phy, info->ueId, info->ueIf, info->downlinkAMCInfo[CA_ID][i], false, CA_ID, i + 1);
	}
	LTENR_RadioMeasurements_PDSCH_Log(phy, CA_ID, info);

	//Uplink
	layerCount = LTENR_PHY_GET_ULLAYER_COUNT(uePhy);
	for (UINT i = 0; i < layerCount; i++)
	{
		print_ltenr_log("\tAMC info between gNB %d:%d and UE %d:%d, Carrier Id = %d, Layer Id = %d for uplink-\n",
						phy->gnbId, phy->gnbIf,
						info->ueId, info->ueIf,
						CA_ID, i);
		info->uplinkAMCInfo[CA_ID][i]->SpectralEfficiency = LTENR_PHY_GetUplinkSpectralEfficiency(info->propagationInfo[CA_ID], i, CA_ID);
		setAMCInfo(phy, info->ueId, info->ueIf, info->uplinkAMCInfo[CA_ID][i], true, CA_ID, i + 1);
	}
	LTENR_RadioMeasurements_PUSCH_Log(phy, CA_ID, info);
}
#pragma endregion

#pragma region PHY_AMCINFO
static void LTENR_PHY_initAMCInfo(ptrLTENR_GNBPHY phy, ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo)
{
	NETSIM_ID i = 0;
	for (i = 0; i < phy->ca_count; i++)
	{
		if (!assocInfo->downlinkAMCInfo[i])
		{
			ptrLTENR_UEPHY uePhy = LTENR_UEPHY_GET(assocInfo->ueId, assocInfo->ueIf);
			UINT layerCount = LTENR_PHY_GET_DLLAYER_COUNT(uePhy);
			assocInfo->downlinkAMCInfo[i] = calloc(layerCount, sizeof * assocInfo->downlinkAMCInfo[i]);
			for (UINT j = 0; j < layerCount; j++)
				assocInfo->downlinkAMCInfo[i][j] = calloc(1, sizeof * assocInfo->downlinkAMCInfo[i][j]);
		}
		if (!assocInfo->uplinkAMCInfo[i])
		{
			ptrLTENR_UEPHY uePhy = LTENR_UEPHY_GET(assocInfo->ueId, assocInfo->ueIf);
			UINT layerCount = LTENR_PHY_GET_ULLAYER_COUNT(uePhy);
			assocInfo->uplinkAMCInfo[i] = calloc(layerCount, sizeof * assocInfo->uplinkAMCInfo[i]);
			for (UINT j = 0; j < layerCount; j++)
				assocInfo->uplinkAMCInfo[i][j] = calloc(1, sizeof * assocInfo->uplinkAMCInfo[i][j]);
		}

		LTENR_PHY_calculateSpectralEfficiency(phy, assocInfo->ueId, assocInfo->ueIf, i);
		LTENR_PHY_setAMCInfo(phy, assocInfo, i);
		LTENR_RadioMeasurements_PBSCH_Log(phy, i, assocInfo);
	}
}
#pragma endregion

#pragma region RSRP_SINR_FOR_RRC
double LTENR_PHY_RRC_RSRP_SINR(NETSIM_ID gnbId, NETSIM_ID gnbIf,
							 NETSIM_ID ueId, NETSIM_ID ueIf) {
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);

	ptrLTENR_ASSOCIATEDUEPHYINFO info = LTENR_ASSOCIATEDUEPHYINFO_GET(gnbId, gnbIf, ueId, ueIf);
	
	double Avg_SSB_SNR = 0;
	UINT c = 0;
	for (NETSIM_ID i = 0; i < phy->ca_count; i++)
	{
		if (info->propagationInfo[i] == NULL)
			LTENR_PHY_calculateSpectralEfficiency(phy, ueId, ueIf, i);
		if (info->propagationInfo[i] != NULL)
			Avg_SSB_SNR += DBM_TO_MW(info->propagationInfo[i]->downlink.SSB_SNR_dB);
		c++;
		LTENR_RadioMeasurements_PBSCH_Log(phy, i, info);
	}
	return MW_TO_DBM(Avg_SSB_SNR / c);
}
#pragma endregion

#pragma region PHY_ASSOCIATION
static ptrLTENR_ASSOCIATEDUEPHYINFO LTENR_ASSOCIATEDUEPHYINFO_FIND(ptrLTENR_GNBPHY phy,
															  NETSIM_ID ueId, NETSIM_ID ueIf)
{
	ptrLTENR_ASSOCIATEDUEPHYINFO info = phy->associatedUEPhyInfo;
	while (info)
	{
		if (info->ueId == ueId && info->ueIf == ueIf)
			return info;
		LTENR_ASSOCIATEDUEPHYINFO_NEXT(info);
	}
	return NULL;
}

ptrLTENR_ASSOCIATEDUEPHYINFO LTENR_ASSOCIATEDUEPHYINFO_GET(NETSIM_ID gnbId, NETSIM_ID gnbIf,
									  NETSIM_ID ueId, NETSIM_ID ueIf)
{
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);
	ptrLTENR_ASSOCIATEDUEPHYINFO info = LTENR_ASSOCIATEDUEPHYINFO_FIND(phy, ueId, ueIf);
	if (info) return info;

	info = LTENR_ASSOCIATEDUEPHYINFO_ALLOC();
	LTENR_ASSOCIATEDUEPHYINFO_ADD(phy, info);
	info->ueId = ueId;
	info->ueIf = ueIf;

	return info;
}

static void LTENR_PHY_associateUE(NETSIM_ID gnbId, NETSIM_ID gnbIf,
								  NETSIM_ID ueId, NETSIM_ID ueIf)
{
	print_ltenr_log("PHY-- UE %d:%d is associated with gNB %d:%d\n",
					ueId, ueIf, gnbId, gnbIf);

	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);
	ptrLTENR_ASSOCIATEDUEPHYINFO info = LTENR_ASSOCIATEDUEPHYINFO_GET(gnbId, gnbIf, ueId, ueIf);

	info->isAssociated = true;

	ptrLTENR_UEPHY uePhy = LTENR_UEPHY_GET(ueId, ueIf);
	uePhy->gnBId = gnbId;
	uePhy->gnbIf = gnbIf;

	fn_NetSim_LTENR_Conditional_HO_TTT_Matrix_Add(gnbId, gnbIf, ueId, ueIf);

	LTENR_ANTENNA_SET_LAYER_COUNT(phy->antenna, uePhy->antenna);

	LTENR_PHY_initAMCInfo(phy, info);

	HARQAssociationHandler(gnbId, gnbIf, ueId, ueIf, true);

	ptrLTENR_GNBMAC mac = LTENR_GNBMAC_GET(gnbId, gnbIf);
	if (fn_NetSim_LTENR_IS_S1_INTERFACE_EXISTS(gnbId)) {
		if (mac->epcId != 0) {
			LTENR_EPC_ASSOCIATION(mac->epcId, mac->epcIf,
				gnbId, gnbIf,
				ueId, ueIf);
		}
	}
}

static void LTENR_PHY_deassociateUE(NETSIM_ID gnbId, NETSIM_ID gnbIf,
								  NETSIM_ID ueId, NETSIM_ID ueIf)
{
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);
	ptrLTENR_ASSOCIATEDUEPHYINFO info = phy->associatedUEPhyInfo;
	while (info) 
	{
		if (info->ueId == ueId && info->ueIf == ueIf) 
		{
			info->isAssociated = false;
			HARQAssociationHandler(gnbId, gnbIf, ueId, ueIf, false);
			for (UINT i = 0; i < MAX_CA_COUNT; i++)
			{
				free(info->downlinkAMCInfo[i]);
				info->downlinkAMCInfo[i] = NULL;
				free(info->uplinkAMCInfo[i]);
				info->uplinkAMCInfo[i] = NULL;
			}
			break;
		}
		LTENR_ASSOCIATEDUEPHYINFO_NEXT(info);
	}
	fn_NetSim_LTENR_Conditional_HO_TTT_Matrix_Remove(gnbId, gnbIf, ueId, ueIf);
}

void LTENR_PHY_ASSOCIATION(NETSIM_ID gnbId, NETSIM_ID gnbIf,
						   NETSIM_ID ueId, NETSIM_ID ueIf,
						   bool isAssociated)
{
	if (isAssociated)
		LTENR_PHY_associateUE(gnbId, gnbIf, ueId, ueIf);
	else
		LTENR_PHY_deassociateUE(gnbId, gnbIf, ueId, ueIf);
}
#pragma endregion

#pragma region PHY_API
double LTENR_PHY_GetSlotEndTime(NETSIM_ID d, NETSIM_ID in)
{
	if (isGNB(d, in))
	{
		ptrLTENR_FRAMEINFO fi = ((ptrLTENR_GNBPHY)LTENR_GNBPHY_GET(d, in))->currentFrameInfo;
		return fi->slotEndTime;
	}
	else
	{
		ptrLTENR_UEPHY phy = LTENR_UEPHY_GET(d, in);
		return LTENR_PHY_GetSlotEndTime(phy->gnBId, phy->gnbIf);
	}
}

static void fn_NetSim_UPDATE_SPECTRAL_EFFICIENCY_INFO(NETSIM_ID gnbID, NETSIM_ID gnbIF,
											   NETSIM_ID ueID, NETSIM_ID ueIF) 
{
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbID, gnbIF);
	ptrLTENR_ASSOCIATEDUEPHYINFO info = LTENR_ASSOCIATEDUEPHYINFO_GET(gnbID, gnbIF, ueID, ueIF);
	for (NETSIM_ID i = 0; i < phy->ca_count; i++)
	{
		if (info->propagationInfo[i])
		{
			info->propagationInfo[i]->uePos.X = DEVICE_POSITION(ueID)->X;
			info->propagationInfo[i]->uePos.Y = DEVICE_POSITION(ueID)->Y;
			info->propagationInfo[i]->uePos.Z = DEVICE_POSITION(ueID)->Z;
		}
		LTENR_PHY_calculateSpectralEfficiency(phy, ueID, ueIF, i);
	}
}

#pragma endregion

void fn_NetSim_PHY_MOBILITY_HANDLE(NETSIM_ID d)
{
	for (NETSIM_ID in = 1; in <= DEVICE(d)->nNumOfInterface; in++)
	{
		if (!isLTENR_RANInterface(d, in)) continue;

		if (isGNB(d, in))
		{
			ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(d, in);
			ptrLTENR_ASSOCIATEDUEPHYINFO info = phy->associatedUEPhyInfo;
			while (info)
			{
				fn_NetSim_UPDATE_SPECTRAL_EFFICIENCY_INFO(d, in, info->ueId, info->ueIf);
				LTENR_ASSOCIATEDUEPHYINFO_NEXT(info);
			}
		}
		else if (isUE(d, in))
		{
			for (NETSIM_ID i = 1; i <= NETWORK->nDeviceCount; i++)
			{
				if (d == i) continue;
				for (NETSIM_ID j = 1; j <= DEVICE(i)->nNumOfInterface; j++)
				{
					if (!isLTENR_RANInterface(i, j)) continue;

					if (isGNB(i, j))
					{
						ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(i, j);
						ptrLTENR_ASSOCIATEDUEPHYINFO info = LTENR_ASSOCIATEDUEPHYINFO_GET(i, j, d, in);
						if (info) fn_NetSim_UPDATE_SPECTRAL_EFFICIENCY_INFO(i, j, d, in);
					}
				}
			}
		}
	}
}

#pragma region ACTIVE_UE_LIST

#pragma region INITIAL_UE_LIST_ADD
 void LTENR_form_active_ue_list(NETSIM_ID gnbId, NETSIM_ID gnbIf, int CA_ID)
{
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);

	ptrLTENR_SPECTRUMCONFIG sc = phy->spectrumConfig;
	ptrLTENR_CA ca = sc->CA[CA_ID];
	
	for (NETSIM_ID i = 1; i <= NETWORK->nDeviceCount; i++)
	{
		for (NETSIM_ID in = 1; in <= DEVICE(i)->nNumOfInterface; in++)
		{
			if (!isLTE_NRInterface(i, in))
				continue;

			if (isUE(i, in)) {
				if (LTENR_ISASSOCIATED(gnbId, gnbIf, i, in)) {

					ptrLTENR_UEPHY ue_phy = LTENR_UEPHY_GET(i, in);

					if (ue_phy->first_active_bwp_id == ca->bwp_id) {

						ptrLTENR_CA_UE_LIST active_ue = calloc(1, sizeof * active_ue);
						active_ue->ue_id = i;
						active_ue->ue_in = in;

						if (ca->ue_list) {
							ptrLTENR_CA_UE_LIST ue = ca->ue_list;
							while (ue->next) ue = ue->next;             
							ue->next = active_ue;
						}
						else
							ca->ue_list = active_ue;
					}
				}
			}
		}								
	}
}
#pragma endregion

#pragma region ACTIVEUE_INFO_ADD
 void LTENR_ACTIVEUEINFO_ADD(NETSIM_ID ueId, NETSIM_ID ueIf, NETSIM_ID gnbId, NETSIM_ID gnbIf, int CA_ID)
 {
	 // to get ca
	 ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);
	 ptrLTENR_SPECTRUMCONFIG sc = phy->spectrumConfig;
	 ptrLTENR_CA ca = sc->CA[CA_ID];
	 
	 // to store new UE id 
	ptrLTENR_CA_UE_LIST ue_info = calloc(1, sizeof * ue_info);
	ue_info->ue_id = ueId;
	ue_info->ue_in = ueIf;

	// to add new ue to ue list
	if (ca->ue_list) {
		ptrLTENR_CA_UE_LIST ue = ca->ue_list;
		while (ue->next) ue = ue->next;
		ue->next = ue_info;
	}
	else
		ca->ue_list = ue_info;
 }
#pragma endregion

#pragma region ACTIVEUE_INFO_REMOVE
 void LTENR_ACTIVEUEINFO_REMOVE(NETSIM_ID ueId, NETSIM_ID ueIf, NETSIM_ID gnbId, NETSIM_ID gnbIf, int CA_ID){ 
	 
	 // for ca
	 ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);//to get ca
	 ptrLTENR_SPECTRUMCONFIG sc = phy->spectrumConfig;
	 ptrLTENR_CA ca = sc->CA[CA_ID];

	 // to get ue list
	 ptrLTENR_UEPHY ue_phy = LTENR_UEPHY_GET(ueId, ueIf); 
	 ptrLTENR_CA_UE_LIST ca_ue_info = ca->ue_list;
	 ptrLTENR_CA_UE_LIST p = NULL;

	 while (ca_ue_info) {
		 if (ca_ue_info->ue_id == ueId &&
			 ca_ue_info->ue_in == ueIf)
		 {
			 if (!p)
			 {
				 ca->ue_list = ca_ue_info->next;
				 free(ca_ue_info);
				 break;
			 }
			 else
			 {
				 p->next = ca_ue_info->next;
				 free(ca_ue_info);
				 break;
			 }
		 }
		 p = ca_ue_info;
		 ca_ue_info = ca_ue_info->next;
	 }
 }
#pragma endregion

#pragma region ACTIVEUE_INFO_FIND
 //ptrLTENR_CA_UE_LIST LTENR_ACTIVEUEINFO_FIND(NETSIM_ID ueId, NETSIM_ID ueIf, NETSIM_ID gnbId, NETSIM_ID gnbIf, int CA_ID) {

	// ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);//to get ca
	// ptrLTENR_SPECTRUMCONFIG sc = phy->spectrumConfig;
	// ptrLTENR_CA ca = sc->CA[CA_ID];

	// ptrLTENR_UEPHY ue_phy = LTENR_UEPHY_GET(ueId, ueIf); // to get ue->list
	// ptrLTENR_CA_UE_LIST info = ca->ue_list;
	// ptrLTENR_CA_UE_LIST p = NULL;

	//if (!ueId)
	//	return info;

	//while (info)
	//{
	//	if (info->ue_id == ueId)
	//	{
	//		return info;
	//		if (!ueIf)
	//			return info;
	//		else if (info->ue_in == ueIf)
	//			return info;
	//	}
	//	info = info->next;
	//}
	//return NULL;
 //}
#pragma endregion

#pragma endregion

 ptrLTENR_BwpSwitch LTENR_BWP_Switch(ptrLTENR_SCHEDULERINFO schedulerInfo, ptrLTENR_UESCHEDULERINFO curr, UINT PRB_needed) {

	 ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(schedulerInfo->gnbId, schedulerInfo->gnbIf);
	 UINT CA_ID = phy->currentFrameInfo->Current_CA_ID;
	 ptrLTENR_CA ca_old = phy->spectrumConfig->CA[CA_ID];
	 UINT max = 0;
	 UINT ca_max = 0;
	 
	 ptrLTENR_BwpSwitch bwp_switch = calloc(1, sizeof * bwp_switch);;

	 if (ca_old->isBWPEnabled) {
		 for (UINT i = 0; i < phy->ca_count; i++) {
			 if (CA_ID <= i) {
				 //switch for ca id higher 
				 ptrLTENR_CA ca = phy->spectrumConfig->CA[i];
				 ptrLTENR_SPECTRUMCONFIG sc = phy->spectrumConfig;
				
				 bwp_switch->ca_new = true;

				 if (ca->PRBCount >= PRB_needed) {
					 bwp_switch->bwp_active = true;
					 LTENR_ACTIVEUEINFO_REMOVE(curr->ueId, curr->ueIf, schedulerInfo->gnbId, schedulerInfo->gnbIf, CA_ID);
					 LTENR_ACTIVEUEINFO_ADD(curr->ueId, curr->ueIf, schedulerInfo->gnbId, schedulerInfo->gnbIf, i);
					 return bwp_switch;
				 }
				 else
				 {
					 if (ca->PRBCount > max)
					 {
						 max = ca->PRBCount;
						 ca_max = i;
					 }
				 }
			 }
			 else if (CA_ID > i) {
				 // old greater than new
				 ptrLTENR_CA ca = phy->spectrumConfig->CA[i];
				 ptrLTENR_SPECTRUMCONFIG sc = phy->spectrumConfig;

				 bwp_switch->ca_new = false;

				 if (ca->PRBCount >= PRB_needed) {
					 bwp_switch->bwp_active = true;
					 bwp_switch->prb_count = ca->PRBCount;
					 LTENR_ACTIVEUEINFO_REMOVE(curr->ueId, curr->ueIf, schedulerInfo->gnbId, schedulerInfo->gnbIf, CA_ID);
					 LTENR_ACTIVEUEINFO_ADD(curr->ueId, curr->ueIf, schedulerInfo->gnbId, schedulerInfo->gnbIf, i);
					 return bwp_switch;
				 }
				 else
				 {
					 if (ca->PRBCount > max)
					 {
						 max = ca->PRBCount;
						 ca_max = i;
					 }
				 }

			 }
		 }

		 //after for	 
		 if (ca_max != CA_ID)
		 {
			 bwp_switch->bwp_active = true;
			 LTENR_ACTIVEUEINFO_REMOVE(curr->ueId, curr->ueIf, schedulerInfo->gnbId, schedulerInfo->gnbIf, CA_ID);
			 LTENR_ACTIVEUEINFO_ADD(curr->ueId, curr->ueIf, schedulerInfo->gnbId, schedulerInfo->gnbIf, ca_max);
			 return bwp_switch;
		 }
		 else
		 {
			 bwp_switch->bwp_active = false;
			 bwp_switch->prb_count = 0;
			 return bwp_switch;
		 }

	 }
	 else if (!ca_old->isBWPEnabled)
	 {
		 bwp_switch->bwp_active = false;
		 bwp_switch->prb_count = 0;
		 return bwp_switch;
	 }
	 return NULL;
 }

#pragma region PHYOUT
 static void LTENR_PhyOut_HandleBroadCast()
 {
	 NetSim_PACKET* packet = pstruEventDetails->pPacket;
	 NETSIM_ID d = pstruEventDetails->nDeviceId;
	 NETSIM_ID in = pstruEventDetails->nInterfaceId;
	 NETSIM_ID r = packet->nReceiverId;

	 for (r = 0; r < NETWORK->nDeviceCount; r++)
	 {
		 if (d == r + 1)
			 continue;

		 for (NETSIM_ID rin = 0; rin < DEVICE(r + 1)->nNumOfInterface; rin++)
		 {
			 if (!isLTE_NRInterface(r + 1, rin + 1))
				 continue;

			 ptrLTENR_PROTODATA data = LTENR_PROTODATA_GET(r + 1, rin + 1);
			 if (data->isDCEnable) {
				 if (data->MasterCellType == MMWAVE_CELL_TYPE) {
					 if (fn_NetSim_isDeviceLTENR(r + 1, rin + 1)) {
						 if (!fn_NetSim_isDeviceLTENR(d, in))
							 continue;
					 }
					 else {
						 continue;
					 }
				 }
				 else {
					 if (!fn_NetSim_isDeviceLTENR(r + 1, rin + 1)) {
						 if (fn_NetSim_isDeviceLTENR(d, in))
							 continue;
					 }
					 else {
						 continue;
					 }
				 }
			 }
			 switch (data->deviceType)
			 {
			 case LTENR_DEVICETYPE_UE:
			 {
				 double endTime = LTENR_PHY_GetSlotEndTime(d, in);
				 NetSim_PACKET* p = fn_NetSim_Packet_CopyPacket(packet);

				 p->pstruPhyData->dArrivalTime = pstruEventDetails->dEventTime;
				 p->pstruPhyData->dEndTime = endTime - 1;
				 p->pstruPhyData->dPacketSize = p->pstruMacData->dPacketSize;
				 p->pstruPhyData->dPayload = p->pstruMacData->dPacketSize;
				 p->pstruPhyData->dStartTime = pstruEventDetails->dEventTime;
				 p->pstruMacData->Packet_MACProtocol = NULL;
				 p->pstruMacData->Packet_MACProtocol = LTENR_MSG_COPY(packet);

				 pstruEventDetails->pPacket = p;

				 pstruEventDetails->dEventTime = endTime - 1;
				 pstruEventDetails->nEventType = PHYSICAL_IN_EVENT;
				 pstruEventDetails->nDeviceId = r + 1;
				 pstruEventDetails->nDeviceType = DEVICE_TYPE(r + 1);
				 pstruEventDetails->nInterfaceId = rin + 1;
				 pstruEventDetails->pPacket->nTransmitterId = d;
				 pstruEventDetails->pPacket->nReceiverId = r + 1;

				 fnpAddEvent(pstruEventDetails);
			 }
			 break;
			 default:
				 break;
			 }
		 }
	 }

	 fn_NetSim_Packet_FreePacket(packet);
	 pstruEventDetails->pPacket = NULL;
 }

 static void LTENR_PhyOut_HandleUnicast()
 {
	 NetSim_PACKET* packet = pstruEventDetails->pPacket;
	 NETSIM_ID d = pstruEventDetails->nDeviceId;
	 NETSIM_ID in = pstruEventDetails->nInterfaceId;
	 NETSIM_ID r = packet->nReceiverId;
	 NETSIM_ID rin = LTENR_FIND_ASSOCIATEINTERFACE(d, in, r);

	 if (rin == 0)
	 {
		 fn_NetSim_Packet_FreePacket(packet);
		 return;
	 }

	 double endTime = LTENR_PHY_GetSlotEndTime(d, in);

	 packet->pstruPhyData->dArrivalTime = pstruEventDetails->dEventTime;
	 packet->pstruPhyData->dEndTime = endTime - 1;
	 packet->pstruPhyData->dPacketSize = packet->pstruMacData->dPacketSize;
	 packet->pstruPhyData->dPayload = packet->pstruMacData->dPacketSize;
	 packet->pstruPhyData->dStartTime = pstruEventDetails->dEventTime;

	 pstruEventDetails->dEventTime = endTime - 1;
	 pstruEventDetails->nEventType = PHYSICAL_IN_EVENT;
	 pstruEventDetails->nDeviceId = r;
	 pstruEventDetails->nDeviceType = DEVICE_TYPE(r);
	 pstruEventDetails->nInterfaceId = rin;
	 fnpAddEvent(pstruEventDetails);
 }

 void fn_NetSim_LTENR_HandlePhyOut()
 {
	 NetSim_PACKET* packet = pstruEventDetails->pPacket;
	 NETSIM_ID r = packet->nReceiverId;

	 if (r == 0) LTENR_PhyOut_HandleBroadCast();
	 else LTENR_PhyOut_HandleUnicast();
 }
#pragma endregion
