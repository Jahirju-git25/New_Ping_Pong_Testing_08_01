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
* Author:    Kumar Gaurav		                                                    *
*										                                            *
* ----------------------------------------------------------------------------------*/
#pragma region HEADER_FILES
#include "stdafx.h"
#include "LTENR_MAC.h"
#include "LTE_NR.h"
#include "LTENR_NetworkSlicing.h"
#include "LTENR_PDCP.h"
#pragma endregion

#pragma region LOG
static FILE* fpPRBLog = NULL;
static FILE* fpSLICELog = NULL;
void fn_NetSim_LTE_MAC_PRBLOG()
{
	if (get_protocol_log_status("LTENR_Radio_Resource_Allocation"))
	{
	char str[BUFSIZ];

	sprintf(str, "%s\\%s", pszIOLogPath, "LTENR_Radio_Resource_Allocation.csv");
	fpPRBLog = fopen(str, "w");
	if (!fpPRBLog)
	{
		perror(str);
		fnSystemError("Unable to open LTENR_Radio_Resource_Allocation.csv file");
	}
	else
	{
		fprintf(fpPRBLog, "%s,%s,%s,%s,",
			"gNB ID", "CC ID", "Slot ID", "Slot");

		fprintf(fpPRBLog, "%s,%s,%s,%s,%s,",
			"Total PRBs", "Control PRBs", "Slot Start Time(ms)", "Slot End Time(ms)", "UE ID");

		fprintf(fpPRBLog, "%s,%s,%s,%s,%s,%s,",
			"BitsPerPRB", "BufferFill(B)", "Allocated PRBs", "Rank", "EWMA MAC Throughput (Mbps)", "Index Bias");

		if (nws->isSlicing)
		{
			fprintf(fpPRBLog, "%s,%s,%s,",
				"Slice ID", "Slice Type", "Slice Differentiator");
		}

		fprintf(fpPRBLog, "\n");

		//fprintf(fpPRBLog, "var2 is past throughput performance for proportional fair scheduling algorithm\n");
		if (nDbgFlag) fflush(fpPRBLog);
	}
	} //LTENR_PRB_LOG
}

void close_ltenr_PRB_log()
{
	if (fpPRBLog)
		fclose(fpPRBLog);
}

static void write_prb_log(ptrLTENR_SCHEDULERINFO schedulerInfo, ptrLTENR_UESCHEDULERINFO list, UINT sliceId)
{
	if (fpPRBLog == NULL) return;

	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(schedulerInfo->gnbId, schedulerInfo->gnbIf);
	int CA_ID = phy->currentFrameInfo->Current_CA_ID;
	LTENR_SLOTTYPE slotType = phy->frameInfo[CA_ID]->slotType;
	double control_prb = (slotType == SLOT_UPLINK ? (ceil(schedulerInfo->OH_UL * schedulerInfo->PRBCount)) :
						 (ceil(schedulerInfo->OH_DL * schedulerInfo->PRBCount)));
	while (list)
	{
		if(list->bufferSize<=0)
			goto NEXTUE;
		if (nws->rsrcSharingTechnique== LTENR_RESOURCE_SHARING_STATIC && nws->ueSliceId[list->ueId] != sliceId)
			goto NEXTUE;
		fprintf(fpPRBLog, "%d, %d, %d,%s,",
			schedulerInfo->gnbId, CA_ID + 1, phy->currentFrameInfo->slotId,
			slotType == SLOT_UPLINK ? "Uplink" : "Downlink");

		fprintf(fpPRBLog, "%u, %lf, %lf, %lf,%d,", schedulerInfo->PRBCount,
			control_prb, phy->currentFrameInfo->slotStartTime / MILLISECOND,
			phy->currentFrameInfo->slotEndTime / MILLISECOND, list->ueId);

		fprintf(fpPRBLog, "%llu,%u,%u,%lf,%lf,",
			list->bitsPerPRB, list->bufferSize,
			list->allocatedPRBCount,
			list->rank, list->var2);
		int slID = (nws->rsrcSharingTechnique == LTENR_RESOURCE_SHARING_DYNAMIC)?nws->ueSliceId[list->ueId]: sliceId;
		if (slotType == SLOT_UPLINK)
			fprintf(fpPRBLog, "%lf,", nws->Info[slID].uplinkBandwidth.nu);
		else
			fprintf(fpPRBLog, "%lf,", nws->Info[slID].downlinkBandwidth.nu);

		if (nws->isSlicing)
		{
		fprintf(fpPRBLog, "%d,%s,%d,",
			slID,
			strSLICE_SERVICE_TYPE[nws->Info[slID].sliceServiceType],
			nws->Info[slID].sliceDifferentiator);
		}

		fprintf(fpPRBLog, "\n");
		NEXTUE:
		list = list->next;
	}

	if (nDbgFlag) fflush(fpPRBLog);
}
#pragma endregion


#pragma region SORTING_OF_RANK
static ptrLTENR_UESCHEDULERINFO LTENR_PRB_SortingofRank(ptrLTENR_UESCHEDULERINFO list)
{
	ptrLTENR_UESCHEDULERINFO head = NULL;
	ptrLTENR_UESCHEDULERINFO tail = NULL;
	ptrLTENR_UESCHEDULERINFO t = NULL;
	while (list)
	{
		if (!head)
		{
			//first member
			head = list;
			tail = list;
			list = list->next;
			head->next = NULL;
			continue;
		}

		if (list->rank >= head->rank)
		{
			// Rank is less and head rank. So place at head
			t = list->next;
			list->next = head;
			head = list;
			list = t;
			continue;
		}

		if (list->rank <= tail->rank)
		{
			// Rank is more than tail rank, so place as tail
			t = list->next;
			tail->next = list;
			tail = list;
			list = t;
			tail->next = NULL;
			continue;
		}

		//Middle
		t = list->next;
		ptrLTENR_UESCHEDULERINFO info = head;
		while (info)
		{
			if (info->next->rank < list->rank)
			{
				list->next = info->next;
				info->next = list;
				break;
			}
			info = info->next;
		}
		list = t;
	}
	return head;
}
#pragma endregion

#pragma region SET_RANK
static void LTENR_PRB_SetRank(ptrLTENR_UESCHEDULERINFO list, ptrLTENR_SCHEDULERINFO schedulerInfo) {
	double sigmaofBitperPRB = 0;
	ptrLTENR_UESCHEDULERINFO current = list;
	if (schedulerInfo->schedulingType == LTENR_MAC_SCHEDULING_ROUNDROBIN) {//RR
		while (list) {
			list->rank = 1.0;
			list->initRank = 1.0;
			list = list->next;
		}
	}

	else if (schedulerInfo->schedulingType == LTENR_MAC_SCHEDULING_PROPORTIONALFAIR) {//fair proportional
		while (list) 
		{
			list->rank = 1;
			list->initRank = 1;
			list->var2 = 1;
			list = list->next;
		}
	}
	else if (schedulerInfo->schedulingType == LTENR_MAC_SCHEDULING_FAIR_SCHEDULING) {//fair scheduling
		double product = 1;
		while (current) {
			sigmaofBitperPRB += current->bitsPerPRB;
			current = current->next;
		}
		current = list;
		while (current) {
			if (sigmaofBitperPRB) {
				current->rank = (double)(current->bitsPerPRB / sigmaofBitperPRB);
				product *= current->rank;
				current->initRank = current->rank;
			}
			current = current->next;
		}
		//for FAIR
		current = list;
		while (current) {
			if (current->rank) {
				current->rank = product / current->rank;
				current->initRank = current->rank;
			}
			current = current->next;
		}
	}
}
#pragma endregion

#pragma region UPDATE_AND_RESET_RANK
static void LTENR_PRB_ResetAllocationPRBCount(ptrLTENR_UESCHEDULERINFO list) {
	NETSIM_ID gnbId = pstruEventDetails->nDeviceId;
	NETSIM_ID gnbIf = pstruEventDetails->nInterfaceId;
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);
	int CA_ID = phy->currentFrameInfo->Current_CA_ID;
	ptrLTENR_CA ca = phy->spectrumConfig->CA[CA_ID];

	while (list) {
		double rate = ((list->allocatedPRBCount * list->bitsPerPRB) / ca->slotDuration_us);
		list->averageThroughput = list->averageThroughput + nws->EWMA_Learning_Rate*
			(rate - list->averageThroughput);
		list->allocatedPRBCount = 0;
		list = list->next;
	}
}

static void LTENR_PRB_UpdateRank_RoundRobin(ptrLTENR_UESCHEDULERINFO list) {
	bool isanyzero = false;
	ptrLTENR_UESCHEDULERINFO info = list;
	while (list) {
		list->rank = (list->rank - (list->allocatedPRBCount / initTotalPRBAvailable)); //+ list->initRank;
		if (list->rank <= 0)
			isanyzero = true;
		list = list->next;
	}
	if (isanyzero)
	{
		list = info;
		while (list)
		{
			list->rank += 1;
			list = list->next;
		}
	}
}

static void LTENR_PRB_UpdateIndexBias_ProportionalFair(ptrLTENR_UESCHEDULERINFO list)
{
	NETSIM_ID gnbId = pstruEventDetails->nDeviceId;
	NETSIM_ID gnbIf = pstruEventDetails->nInterfaceId;
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);

#pragma warning (disable : 4047)
	int CA_ID = pstruEventDetails->szOtherDetails;
#pragma warning (default : 4047)

	LTENR_SLOTTYPE slotType = phy->frameInfo[CA_ID]->slotType;
	for (int sliceId = 0; sliceId <= nws->sliceCount; sliceId++)
	{
		if (nws->Info[sliceId].sliceServiceType == SLICE_SERVICE_TYPE_NONE)
		{
			if(slotType == SLOT_DOWNLINK)
				nws->Info[sliceId].downlinkBandwidth.nu = 0;
			else if(slotType == SLOT_UPLINK)
				nws->Info[sliceId].uplinkBandwidth.nu = 0;
			else
				fnNetSimError("Unknown slot type %s in %s\n",
					strLTENR_SLOTTYPE[slotType],
					__FUNCTION__);
		}
		else
		{//check downlink/uplink
			if (slotType == SLOT_DOWNLINK)
			{
				nws->Info[sliceId].downlinkBandwidth.nu = nws->Info[sliceId].downlinkBandwidth.nu + nws->LagrangeMultiplierLearningRate *
					(nws->Info[sliceId].downlinkBandwidth.aggregateRateGuarantee_mbps - nws->Info[sliceId].downlinkBandwidth.sumAvgThroughput);
				nws->Info[sliceId].downlinkBandwidth.nu = (nws->Info[sliceId].downlinkBandwidth.nu < 0) ? 
					0 : (nws->Info[sliceId].downlinkBandwidth.nu > 10) ? 
					10 : nws->Info[sliceId].downlinkBandwidth.nu;
			}
			else if(slotType == SLOT_UPLINK)
			{
				nws->Info[sliceId].uplinkBandwidth.nu = nws->Info[sliceId].uplinkBandwidth.nu + nws->LagrangeMultiplierLearningRate *
					(nws->Info[sliceId].uplinkBandwidth.aggregateRateGuarantee_mbps - nws->Info[sliceId].uplinkBandwidth.sumAvgThroughput);
				nws->Info[sliceId].uplinkBandwidth.nu = (nws->Info[sliceId].uplinkBandwidth.nu < 0) ?
					0 : (nws->Info[sliceId].uplinkBandwidth.nu > 10) ?
					10 : nws->Info[sliceId].uplinkBandwidth.nu;
			}
			else
			{
				fnNetSimError("Unknown slot type %s in %s\n",
					strLTENR_SLOTTYPE[slotType],
					__FUNCTION__);
			}
		}
		
	}
}

static void LTENR_PRB_UpdateSumAvgThroughput_ProportionalFair(ptrLTENR_UESCHEDULERINFO list)
{
	NETSIM_ID gnbId = pstruEventDetails->nDeviceId;
	NETSIM_ID gnbIf = pstruEventDetails->nInterfaceId;
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);

#pragma warning (disable : 4047)
	int CA_ID = pstruEventDetails->szOtherDetails;
#pragma warning (default : 4047)

	LTENR_SLOTTYPE slotType = phy->frameInfo[CA_ID]->slotType;

	for(int sliceId =0; sliceId <=nws->sliceCount; sliceId++)
	{		
		ptrLTENR_UESCHEDULERINFO temp = list;
		if (slotType == SLOT_DOWNLINK)
			nws->Info[sliceId].downlinkBandwidth.sumAvgThroughput = 0;
		else if(slotType == SLOT_UPLINK)
			nws->Info[sliceId].uplinkBandwidth.sumAvgThroughput = 0;
		else
			fnNetSimError("Unknown slot type %s in %s\n",
				strLTENR_SLOTTYPE[slotType],
				__FUNCTION__);
		while (temp)
		{
			if (nws->ueSliceId[temp->ueId] == sliceId)
			{
				if (slotType == SLOT_DOWNLINK)
					nws->Info[sliceId].downlinkBandwidth.sumAvgThroughput += temp->averageThroughput;///slottime(microsecond));
				else if(slotType == SLOT_UPLINK)
					nws->Info[sliceId].uplinkBandwidth.sumAvgThroughput += temp->averageThroughput;
				else
					fnNetSimError("Unknown slot type %s in %s\n",
						strLTENR_SLOTTYPE[slotType],
						__FUNCTION__);
			}
			temp = temp->next;
		}
	}
}

static void LTENR_PRB_UpdateRank_ProportionalFair(ptrLTENR_UESCHEDULERINFO list, double alpha)
{
	if (list->bufferSize > 0)
	{
		LTENR_PRB_UpdateSumAvgThroughput_ProportionalFair(list);
		LTENR_PRB_UpdateIndexBias_ProportionalFair(list);
	}
	while (list)
	{
		// Tj(t) = (1-1/alpha)*Tj(t-1) + (1/alpha)*Tj^(t)
		// past throughput performance at t = (1-1/alpha)past_throughput_performance_at_(t-1) + (1/alpha)*(allocatedPRB * bits_per_prb) 
		//list->var2 = (1 - 1.0 / alpha) * list->var2 + (1.0 / alpha) * ((double)(list->allocatedPRBCount * list->bitsPerPRB));
		list->var2 = list->averageThroughput;
		if (list->var2 == 0) list->var2 = 1;
		list = list->next;
	}
}

static void LTENR_PRB_InitRank_ProportionalFair(ptrLTENR_UESCHEDULERINFO list)
{
	NETSIM_ID gnbId = pstruEventDetails->nDeviceId;
	NETSIM_ID gnbIf = pstruEventDetails->nInterfaceId;
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);

#pragma warning (disable : 4047)
	int CA_ID = pstruEventDetails->szOtherDetails;
#pragma warning (default : 4047)

	LTENR_SLOTTYPE slotType = phy->frameInfo[CA_ID]->slotType;

	while (list)
	{
		// Compute R_i(k,t) = S(M_i,k(t),1)/tau = bits_per_prb/TTI = bits_per_prb assuming TTI = 1
		// rank = R_j(k,t)/Tj(k,t) = bits_per_prb/past_throughput_performance.
		if (list->var2 == 0) 
			list->var2 = 1; // past throughput performance become 0 due to handover. Resetting back to 1
		if (slotType == SLOT_DOWNLINK)
			list->rank = list->bitsPerPRB *((1.0 / list->var2) + nws->Info[nws->ueSliceId[list->ueId]].downlinkBandwidth.nu); 
		else if(slotType == SLOT_UPLINK)
			list->rank = list->bitsPerPRB *((1.0 / list->var2) + nws->Info[nws->ueSliceId[list->ueId]].uplinkBandwidth.nu);
		else
			fnNetSimError("Unknown slot type %s in %s\n",
				strLTENR_SLOTTYPE[slotType],
				__FUNCTION__);
		list = list->next;
	}
}

static void LTENR_PRB_InitRank_MaxThroughput(ptrLTENR_UESCHEDULERINFO list)
{
	ptrLTENR_UESCHEDULERINFO info = list;
	while (info)
	{
		info->rank = info->bitsPerPRB;
		info = info->next;
	}
}

static void LTENR_PRB_UpdateRank_FAIR(ptrLTENR_UESCHEDULERINFO list) {
	double min_rank = INT_MAX;
	double product = 1;
	bool isnegative = false;
	ptrLTENR_UESCHEDULERINFO current = list;
	while (current) {
		if (current->initRank < min_rank)
			min_rank = current->initRank;
		product *= current->rank;
		current = current->next;
	}
	current = list;
	while (current) {
		current->rank = current->rank - (min_rank * (current->allocatedPRBCount / initTotalPRBAvailable));
		if (current->rank <= 0)
			isnegative = true;
		current = current->next;
	}
	current = list;
	if (isnegative) {
		while (current) {
			current->rank = current->rank + current->initRank;
			current = current->next;
		}
	}
}
#pragma endregion

#pragma region ALLOCATE_RESOURCE
static void LTENR_PRB_AllocatePRB(ptrLTENR_SCHEDULERINFO schedulerInfo, ptrLTENR_UESCHEDULERINFO* curr, UINT totalPRBAvailable, UINT sliceId)
{
	ptrLTENR_UESCHEDULERINFO current = *curr;
	if (schedulerInfo->isUERRCSetUpCompleted == false) return;

	if (!schedulerInfo->isPRBRankInit)
	{
		//first time to set rank
		LTENR_PRB_SetRank(schedulerInfo->downlinkInfo, schedulerInfo);
		LTENR_PRB_SetRank(schedulerInfo->uplinkInfo, schedulerInfo);
		schedulerInfo->isPRBRankInit = TRUE;
	}

	if (schedulerInfo->schedulingType == LTENR_MAC_SCHEDULING_PROPORTIONALFAIR)
		LTENR_PRB_InitRank_ProportionalFair(current);
	else if (schedulerInfo->schedulingType == LTENR_MAC_SCHEDULING_MAXTHROUGHTPUT)
		LTENR_PRB_InitRank_MaxThroughput(current);

	current = *curr;
	UINT totalPRB = totalPRBAvailable;
	current = LTENR_PRB_SortingofRank(current);
	ptrLTENR_UESCHEDULERINFO current1 = current;
	*curr = current;

	while (current && totalPRBAvailable > 0)
	{
		if (nws->rsrcSharingTechnique == LTENR_RESOURCE_SHARING_STATIC && 
			nws->ueSliceId[current->ueId] != sliceId)
			goto NEXTUE;
		if (current->bitsPerPRB > 0)
		{
			UINT PRBallocation = (UINT)ceil(current->bufferSize * 8.0 / (current->bitsPerPRB));

			if (PRBallocation < totalPRBAvailable) {
				current->allocatedPRBCount += PRBallocation;
				totalPRBAvailable -= current->allocatedPRBCount;
				ptrLTENR_PDCPVAR pdcpUE = LTENR_PDCP_GET(current->ueId, current->ueIf);
				pdcpUE->TotalPDSCHBytes += (current->allocatedPRBCount * current->bitsPerPRB) / 8;
			}
			else
			{
				ptrLTENR_BwpSwitch bwp_switch = LTENR_BWP_Switch(schedulerInfo, current1, PRBallocation);
										
				if (bwp_switch->bwp_active) {
						if (PRBallocation > bwp_switch->prb_count) {
							current->allocatedPRBCount = bwp_switch->prb_count;
							totalPRBAvailable = 0;
						}
						else if (PRBallocation <= bwp_switch->prb_count) {
							current->allocatedPRBCount = PRBallocation;
							totalPRBAvailable = bwp_switch->prb_count - PRBallocation;
						}
					}
				else if (!bwp_switch->bwp_active) 
				{
					current->allocatedPRBCount = totalPRBAvailable;
					totalPRBAvailable = 0;
					ptrLTENR_PDCPVAR pdcpUE = LTENR_PDCP_GET(current->ueId, current->ueIf);
					pdcpUE->TotalPDSCHBytes += (current->allocatedPRBCount * current->bitsPerPRB) / 8;
				}
				free(bwp_switch);
			}
		}
		NEXTUE:
		current = current->next;
	}
	if (schedulerInfo->schedulingType == LTENR_MAC_SCHEDULING_FAIR_SCHEDULING)
		LTENR_PRB_UpdateRank_FAIR(current1);
	else if (schedulerInfo->schedulingType == LTENR_MAC_SCHEDULING_ROUNDROBIN)
		LTENR_PRB_UpdateRank_RoundRobin(current1);
	else if (schedulerInfo->schedulingType == LTENR_MAC_SCHEDULING_PROPORTIONALFAIR)
		LTENR_PRB_UpdateRank_ProportionalFair(current1, schedulerInfo->alpha);

	write_prb_log(schedulerInfo, current1, sliceId);
}

static int sumofAllocation(ptrLTENR_UESCHEDULERINFO list) 
{
	int totalPRBAllocated = 0;
	while (list) {
		totalPRBAllocated += list->allocatedPRBCount;
		list = list->next;
	}
	return totalPRBAllocated;
}
#pragma endregion

#pragma region RESOURCE_SCHEDULER
void LTENR_PRB_Scheduler(ptrLTENR_SCHEDULERINFO schedulerInfo) {
	NETSIM_ID gnbId = pstruEventDetails->nDeviceId;
	NETSIM_ID gnbIf = pstruEventDetails->nInterfaceId;
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);
	int CA_ID = phy->currentFrameInfo->Current_CA_ID;
	LTENR_SLOTTYPE slotType = phy->frameInfo[CA_ID]->slotType;
	UINT totalPRBAvailable = schedulerInfo->PRBCount;
	initTotalPRBAvailable = totalPRBAvailable;

	if (slotType == SLOT_UPLINK) {
		if (!schedulerInfo->uplinkInfo) {
			return;
		}
		totalPRBAvailable -= (UINT)ceil(totalPRBAvailable * schedulerInfo->OH_UL);//allocate PRB for overhead 
		LTENR_PRB_ResetAllocationPRBCount(schedulerInfo->uplinkInfo);
		LTENR_PRB_ResetAllocationPRBCount(schedulerInfo->downlinkInfo);
		//Subtract prb reqd for retransmission
		totalPRBAvailable -= schedulerInfo->TotalPRBReqdForHARQRetransmission;

		//Network Slicing
		//slice id, slice type, slice differentiator to PRB log if slicing is enabled
		ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(gnbId, gnbIf);
		if (nws->isSlicing && nws->rsrcSharingTechnique == LTENR_RESOURCE_SHARING_STATIC)
		{
			double PRB_fraction = (double)totalPRBAvailable / 100;
			UINT PRBforSlice = totalPRBAvailable;
			for (int i = 0; i <= nws->sliceCount; i++)
			{
				PRBforSlice = (UINT)(PRB_fraction * nws->Info[i].uplinkBandwidth.resourceAllocationPercentage);
				LTENR_PRB_AllocatePRB(schedulerInfo, &schedulerInfo->uplinkInfo, PRBforSlice, i);
			}
		}
		else
			LTENR_PRB_AllocatePRB(schedulerInfo, &schedulerInfo->uplinkInfo, totalPRBAvailable, 0);
	}

	else if (schedulerInfo->slotType == SLOT_DOWNLINK) {
		if (!schedulerInfo->downlinkInfo) {
			return;
		}

		totalPRBAvailable -= (UINT)ceil(totalPRBAvailable * schedulerInfo->OH_DL);//allocate PRB for overhead 
		LTENR_PRB_ResetAllocationPRBCount(schedulerInfo->downlinkInfo);
		LTENR_PRB_ResetAllocationPRBCount(schedulerInfo->uplinkInfo);
		//Subtract prb reqd for retransmission
		totalPRBAvailable -= schedulerInfo->TotalPRBReqdForHARQRetransmission;

		//Network Slicing
		ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(gnbId, gnbIf);
		if (nws->isSlicing && nws->rsrcSharingTechnique == LTENR_RESOURCE_SHARING_STATIC)
		{
			double PRB_fraction = (double)totalPRBAvailable / 100;
			UINT PRBforSlice = totalPRBAvailable;
			for (int i = 0; i <= nws->sliceCount; i++)
			{
				PRBforSlice = (UINT)(PRB_fraction * nws->Info[i].downlinkBandwidth.resourceAllocationPercentage);
				LTENR_PRB_AllocatePRB(schedulerInfo, &schedulerInfo->downlinkInfo, PRBforSlice, i);
			}
		}
		else
			LTENR_PRB_AllocatePRB(schedulerInfo, &schedulerInfo->downlinkInfo, totalPRBAvailable, 0);
	}

	else 
	{
		fnNetSimError("Unknown slot type %s in %s\n",
					  strLTENR_SLOTTYPE[schedulerInfo->slotType],
					  __FUNCTION__);
	}
}
#pragma endregion
