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
#include "LTENR_Spectrum.h"
#include "LTENR_AMCTable.h"
#include "LTENR_MAC.h"
#include "LTENR_PHY.h"
#include "LTENR_Multiplexing.h"
#include "LTENR_Spectrum.h"
#include "LTENR_HARQ.h"
#include <stdarg.h>
#pragma endregion

#pragma region HARQ_LOG
static FILE* fpLog = NULL;
static void OpenLog()
{
	if (get_protocol_log_status("LTENR_CODEBLOCK_LOG"))
	{
	char s[BUFSIZ];
	sprintf(s, "%s\\%s", pszIOLogPath, "LTENR_CodeBlock_Log.csv");
	fpLog = fopen(s, "w");
	if (!fpLog)
	{
		fnSystemError("Unable to open %s file", s);
		perror(s);
	}
	else
	{
		fprintf(fpLog, "%s,", "Time (ms)");

		fprintf(fpLog, "%s,%s,%s,%s,%s,",
			"gNB/eNB ID", "gNB/eNB I/F", "UE ID", "UE I/F", "Channel");

		fprintf(fpLog, "%s,%s,%s,%s,%s,%s,",
			"CC ID", "Frame ID", "SubFrame ID", "Slot ID", "Layer ID", "Process ID");

		fprintf(fpLog, "%s,", "Remarks");

		fprintf(fpLog, "%s,%s,%s,%s,%s,%s,%s,",
			"TBS", "Modulation", "CodeRate", "CBS", "CBS_", "SINR(Combined)", "BLER");

		fprintf(fpLog, "%s,%s,%s,%s,%s,",
			"CBG ID", "CB ID", "NDI", "TransmissionNumber", "Status");

		fprintf(fpLog, "\n");
		fflush(fpLog);
	}
	}
}

static void CloseLog()
{
	if (fpLog) { fclose(fpLog); }
}

static void WriteTextLog(ptrLTENR_HARQ_ENTITY entity, char* format, ...)
{
	if (fpLog == NULL) return;

	fprintf(fpLog, "%0.3lf,%d,%d,%d,%d,%s,",
		ldEventTime / MILLISECOND,
		entity->gNBId, entity->gNBIf,
		entity->ueId, entity->ueIf,
		entity->isUpinkEntity ? "PUSCH" : "PDSCH");

	ptrLTENR_FRAMEINFO frame = LTENR_GNBPHY_GET(entity->gNBId, entity->gNBIf)->currentFrameInfo;
	fprintf(fpLog, "%d,%d,%d,%d,%d,%d,",
		frame->Current_CA_ID + 1, frame->frameId, frame->subFrameId,
		frame->slotId, 0, entity->currentProcessId);

	va_list list;
	va_start(list, format);
	vfprintf(fpLog, format, list);
	va_end(list);

	fprintf(fpLog, ",\n");

	if (nDbgFlag) fflush(fpLog);
}

static void WriteTextLogWithId(NETSIM_ID gnbId, NETSIM_ID gnbIf, NETSIM_ID ueId, NETSIM_ID ueIf, char* format, ...)
{
	if (fpLog == NULL) return;

	ptrLTENR_FRAMEINFO frame = LTENR_GNBPHY_GET(gnbId, gnbIf)->currentFrameInfo;

	fprintf(fpLog, "%0.3lf,%d,%d,%d,%d,%s,",
		ldEventTime / MILLISECOND,
		gnbId, gnbIf, ueId, ueIf, "");

	
	fprintf(fpLog, "%d,%d,%d,%d,%d,%s,",
		frame->Current_CA_ID + 1, frame->frameId, frame->subFrameId,
		frame->slotId, 0, "N/A");

	va_list list;
	va_start(list, format);
	vfprintf(fpLog, format, list);
	va_end(list);

	fprintf(fpLog, ",\n");

	if (nDbgFlag) fflush(fpLog);
}

static void WriteCBLog(NETSIM_ID gNBId, NETSIM_ID gNBIf, NETSIM_ID ueId, NETSIM_ID ueIf,
	ptrLTENR_HARQ_PROCESS process,
	char* remarks, int layerId, int cbgId, int cbid,PACKET_STATUS status)
{
	if (fpLog == NULL) return;

	fprintf(fpLog, "%0.3lf,", ldEventTime / MILLISECOND);

	fprintf(fpLog, "%d,%d,%d,%d,%s,",
		gNBId, gNBIf, ueId, ueIf, process->isUPLinkProcess ? "PUSCH" : "PDSCH");

	fprintf(fpLog, "%d,%d,%d,%d,%d,%d,",
		process->caId + 1, process->frameId, process->subFrameId, process->slotId, layerId, process->processId + 1);

	fprintf(fpLog, "%s,", remarks);

	fprintf(fpLog, "%d,%s,%lf,%d,%d,%lf,%lf,",
		process->amcInfo[layerId - 1]->tbsSize,
		strPHY_MODULATION[process->amcInfo[layerId - 1]->mcsTable.modulation],
		process->amcInfo[layerId - 1]->mcsTable.codeRate,
		process->amcInfo[layerId - 1]->CBS,
		process->amcInfo[layerId - 1]->CBS_,
		process->transmissionBuffer[layerId - 1].sinr,
		process->transmissionBuffer[layerId - 1].bler);

	fprintf(fpLog, "%d,%d,%s,%d,%s,",
		cbgId, cbid,
		process->NDI ? "True" : "False",
		process->transmissionNumber,
		status == PacketStatus_Error ? "Error" : "Success");

	fprintf(fpLog, "\n");
	fflush(fpLog);
}

static void WriteCBGLog(NETSIM_ID gNBId, NETSIM_ID gNBIf, NETSIM_ID ueId, NETSIM_ID ueIf,
	ptrLTENR_HARQ_PROCESS process)
{
	if (fpLog == NULL) return;

	char remark[BUFSIZ] = "";
	for (UINT l = 0; l < process->layerCount; l++)
	{
		for (UINT i = 0; i < process->transmissionBuffer->CBGCount; i++)
		{
			ptrLTENR_CBG cbg = &process->transmissionBuffer[l].CBGList[i];
			remark[0] = 0;
			for (UINT c = 0; c < cbg->CBCount; c++)
			{
				WriteCBLog(gNBId, gNBIf, ueId, ueIf,
					process, remark,
					l + 1, i + 1, c + 1, cbg->CBErrorStatus[c]);
			}
		}
	}
}
#pragma endregion //HARQ_LOG

#pragma region ERROR
static PACKET_STATUS DecideCBError(double bler)
{
	double r = NETSIM_RAND_01();
	if (r <= bler) return PacketStatus_Error;
	else return PacketStatus_NoError;
}

static void DecideCBGError(ptrLTENR_HARQ_PROCESS process)
{
	for (UINT l = 0; l < process->layerCount; l++)
	{
		process->transmissionBuffer[l].isErrored = PacketStatus_NoError;
		for (UINT i = 0; i < process->transmissionBuffer[l].CBGCount; i++)
		{
			ptrLTENR_CBG cbg = &process->transmissionBuffer[l].CBGList[i];
			cbg->isErrored = PacketStatus_NoError;
			for (UINT c = 0; c < cbg->CBCount; c++)
			{
				if (cbg->CBErrorStatus[c] == PacketStatus_Error || process->transmissionNumber == 0)
					cbg->CBErrorStatus[c] = DecideCBError(process->transmissionBuffer[l].bler);
				if (cbg->isErrored == PacketStatus_NoError)
					cbg->isErrored = cbg->CBErrorStatus[c];
			}
			if (cbg->isErrored == PacketStatus_Error) process->transmissionBuffer[l].isErrored = PacketStatus_Error;
		}
	}
}
#pragma endregion //ERROR

#pragma region PACKETBUFFER
static void EmptyPacketBuffer(ptrLTENR_HARQ_PROCESS process)
{
	while (process->packetBuffer.head)
	{
		NetSim_PACKET* t = process->packetBuffer.head;
		process->packetBuffer.head = t->pstruNextPacket;
		t->pstruNextPacket = NULL;
		t->nPacketStatus = PacketStatus_Error;
		t->pstruPhyData->dEndTime = ldEventTime;
		fn_NetSim_WritePacketTrace(t);
		fn_NetSim_Packet_FreePacket(t);
	}
	process->packetBuffer.tail = NULL;
}

static void AddPacketToPacketBuffer(ptrLTENR_HARQ_PROCESS process, NetSim_PACKET* packet)
{
	if (process->packetBuffer.head)
	{
		process->packetBuffer.tail->pstruNextPacket = packet;
		process->packetBuffer.tail = packet;
	}
	else
	{
		process->packetBuffer.head = packet;
		process->packetBuffer.tail = packet;
	}
}
#pragma endregion //PACKETBUFFER

#pragma region CBG
static void ResetCBG(ptrLTENR_HARQ_PROCESS process)
{
	for (UINT l = 0; l < process->layerCount; l++)
	{
		process->transmissionBuffer[l].bitsPerPRB = 0;
		process->transmissionBuffer[l].bler = 0;
		process->transmissionBuffer[l].CBGCount = 0;
		process->transmissionBuffer[l].isErrored = PacketStatus_NoError;
		process->transmissionBuffer[l].isNonEmpty = false;
		process->transmissionBuffer[l].PendingSize = 0;
		process->transmissionBuffer[l].sinr = 0;
		process->transmissionBuffer[l].Totalsize = 0;
		for (int c = 0; c < MAX_CBGCOUNT_PER_TB; c++)
		{
			process->transmissionBuffer[l].CBGList[c].CBCount = 0;
			if (process->transmissionBuffer[l].CBGList[c].CBErrorStatus) free(process->transmissionBuffer[l].CBGList[c].CBErrorStatus);
			process->transmissionBuffer[l].CBGList[c].CBErrorStatus = NULL;
			process->transmissionBuffer[l].CBGList[c].isErrored = PacketStatus_NoError;
		}
	}
	process->NDI = true;
	process->PRBReqdForRetx = 0;
	process->transmissionNumber = 0;
}

static void CalculatePendingData(ptrLTENR_HARQ_PROCESS process)
{
	UINT prbreqd = 0;
	for (UINT l = 0; l < process->layerCount; l++)
	{
		process->transmissionBuffer[l].PendingSize = 0;
		for (UINT i = 0; i < process->transmissionBuffer[l].CBGCount; i++)
		{
			if (process->transmissionBuffer[l].CBGList[i].isErrored == PacketStatus_Error)
			{
				process->transmissionBuffer[l].PendingSize += process->transmissionBuffer[l].CBGList[i].CBCount * process->amcInfo[l]->CBS_;
			}
		}
		prbreqd = max(prbreqd, process->transmissionBuffer[l].PendingSize / process->transmissionBuffer[l].bitsPerPRB);
	}
	process->PRBReqdForRetx = prbreqd;
}
#pragma endregion //CBG

#pragma region FREE
static void FreeHARQProcess(ptrLTENR_HARQ_PROCESS process)
{
	EmptyPacketBuffer(process);
}

static void FreeHARQEntity(ptrLTENR_HARQ_ENTITY entity)
{
	for (int i = 0; i < MAX_HARQ_PROCESS; i++)
	{
		FreeHARQProcess(&entity->harqProcess[i]);
	}
}

static void FreeHARQ(NETSIM_ID gnbId, NETSIM_ID gnbIf, NETSIM_ID ueId, NETSIM_ID ueIf)
{
	ptrLTENR_ASSOCIATEDUEPHYINFO info = LTENR_ASSOCIATEDUEPHYINFO_GET(gnbId, gnbIf, ueId, ueIf);
	for (int c = 0; c < MAX_CA_COUNT; c++)
	{
		FreeHARQEntity(&info->harq->downlinkHARQEntity[c]);
		FreeHARQEntity(&info->harq->uplinkHARQEntity[c]);
	}
	free(info->harq);
	info->harq = NULL;
}
#pragma endregion //FREE

#pragma region INIT
static void InitHARQ(NETSIM_ID gnbId, NETSIM_ID gnbIf, NETSIM_ID ueId, NETSIM_ID ueIf)
{
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(gnbId, gnbIf);
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);
	ptrLTENR_ASSOCIATEDUEPHYINFO info = LTENR_ASSOCIATEDUEPHYINFO_GET(gnbId, gnbIf, ueId, ueIf);
	info->harq = calloc(1, sizeof * info->harq);
	for (int i = 0; i < MAX_CA_COUNT; i++)
	{
		info->harq->downlinkHARQEntity[i].maxProcessCount = pd->HARQ_ProcessCount;
		info->harq->downlinkHARQEntity[i].retryLimit = pd->isHARQEnable ? pd->HARQ_RetryLimit : 0;
		info->harq->downlinkHARQEntity[i].maxCBGPerTB = pd->maxCBGPerTB;
		info->harq->downlinkHARQEntity[i].BlerTableId = phy->CSIReportConfig.tableId;
		info->harq->downlinkHARQEntity[i].isUpinkEntity = false;
		info->harq->downlinkHARQEntity[i].gNBId = gnbId;
		info->harq->downlinkHARQEntity[i].gNBIf = gnbIf;
		info->harq->downlinkHARQEntity[i].ueId = ueId;
		info->harq->downlinkHARQEntity[i].ueIf = ueIf;

		info->harq->uplinkHARQEntity[i].maxProcessCount = pd->HARQ_ProcessCount;
		info->harq->uplinkHARQEntity[i].retryLimit = pd->isHARQEnable ? pd->HARQ_RetryLimit : 0;
		info->harq->uplinkHARQEntity[i].maxCBGPerTB = pd->maxCBGPerTB;
		info->harq->uplinkHARQEntity[i].BlerTableId = phy->CSIReportConfig.tableId;
		info->harq->uplinkHARQEntity[i].isUpinkEntity = true;
		info->harq->uplinkHARQEntity[i].gNBId = gnbId;
		info->harq->uplinkHARQEntity[i].gNBIf = gnbIf;
		info->harq->uplinkHARQEntity[i].ueId = ueId;
		info->harq->uplinkHARQEntity[i].ueIf = ueIf;
	}
}

void HARQAssociationHandler(NETSIM_ID gnbId, NETSIM_ID gnbIf, NETSIM_ID ueId, NETSIM_ID ueIf, bool isAssociated)
{
	if (isAssociated)
	{
		WriteTextLogWithId(gnbId, gnbIf, ueId, ueIf, "HARQ entity created");
		InitHARQ(gnbId, gnbIf, ueId, ueIf);
	}
	else
	{
		WriteTextLogWithId(gnbId, gnbIf, ueId, ueIf, "HARQ entity destroyed");
		FreeHARQ(gnbId, gnbIf, ueId, ueIf);
	}
}

void fn_NetSim_LTENR_HARQ_Init()
{
	OpenLog();
	fn_NetSim_LTENR_RegisterCallBackForAssociation(HARQAssociationHandler);
}
#pragma endregion //INIT

#pragma region FINISH
void fn_NetSim_LTENR_HARQ_Finish()
{
	CloseLog();
}
#pragma endregion //FINISH

#pragma region SLOT_START
UINT fn_NetSim_LTENR_HARQ_GetPRBReqdForRetx(ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo, bool isUplink, int caId)
{
	ptrLTENR_HARQ_ENTITY entity = isUplink ? &assocInfo->harq->uplinkHARQEntity[caId] : &assocInfo->harq->downlinkHARQEntity[caId];
	ptrLTENR_HARQ_PROCESS process = &entity->harqProcess[entity->currentProcessId - 1];
	return process->PRBReqdForRetx;
}

static void IncreaseProcessNumber(ptrLTENR_HARQ_ENTITY entity)
{
	entity->currentProcessId++;
	if (entity->currentProcessId > entity->maxProcessCount)
		entity->currentProcessId = 1;
}

static void EmptyProcess(ptrLTENR_HARQ_PROCESS process)
{
	EmptyPacketBuffer(process);
	ResetCBG(process);
}

static void SetNDI(ptrLTENR_HARQ_ENTITY entity)
{
	ptrLTENR_HARQ_PROCESS process = &entity->harqProcess[entity->currentProcessId - 1];
	if (process->packetBuffer.head == NULL)
	{
		process->NDI = true;
		process->transmissionNumber = 0;
	}
	else
	{
		process->transmissionNumber++;
		if (process->transmissionNumber >= entity->retryLimit)
		{
			WriteTextLog(entity, "All transmission attempts failed for current TB. Dropping TB");
			EmptyProcess(process);
			process->NDI = true;
			process->transmissionNumber = 0;
		}
		else
		{
			process->NDI = false;
			CalculatePendingData(process);
			WriteTextLog(entity, "PRBs required for retransmission is %d", process->PRBReqdForRetx);
		}
	}
}

bool fn_NetSim_LTENR_HARQ_GetNDIFlag(ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo, bool isUplink, int caId)
{
	ptrLTENR_HARQ_ENTITY entity = isUplink ? &assocInfo->harq->uplinkHARQEntity[caId] : &assocInfo->harq->downlinkHARQEntity[caId];
	return entity->harqProcess[entity->currentProcessId - 1].NDI;
}

void fn_NetSim_LTENR_HARQ_HandleSlotStart(NETSIM_ID gnbId,NETSIM_ID gnbIf, int caId)
{
	ptrLTENR_GNBMAC mac = LTENR_GNBMAC_GET(gnbId, gnbIf);
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);
	ptrLTENR_FRAMEINFO finfo = phy->currentFrameInfo;

	ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo = phy->associatedUEPhyInfo;
	while (assocInfo)
	{
		if (assocInfo->isAssociated)
		{
			ptrLTENR_HARQ_ENTITY entity = finfo->slotType == SLOT_DOWNLINK ? &assocInfo->harq->downlinkHARQEntity[caId] : &assocInfo->harq->uplinkHARQEntity[caId];
			IncreaseProcessNumber(entity);
			SetNDI(entity);
			WriteTextLog(entity,
				"\"Process number = %d, NDI = %s, Transmission number = %d\"",
				entity->currentProcessId,
				entity->harqProcess[entity->currentProcessId - 1].NDI ? "True" : "False",
				entity->harqProcess[entity->currentProcessId - 1].transmissionNumber);
		}
		LTENR_ASSOCIATEDUEPHYINFO_NEXT(assocInfo);
	}
}

static void StoreAMCInfo(ptrLTENR_HARQ_ENTITY entity, ptrLTENR_HARQ_PROCESS process, ptrLTENR_UESCHEDULERINFO si, ptrLTENR_AMCINFO* amcInfo, UINT layerCount, double* sinr, UINT BlerTableId)
{
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(entity->gNBId, entity->gNBIf);
	if (process->NDI == false)
	{
		//No need to store the AMC. Transmit using old AMC
		//Update the sinr and bler
		for (UINT i = 0; i < layerCount; i++)
		{
			process->transmissionBuffer[i].sinr = MW_TO_DBM(DBM_TO_MW(process->transmissionBuffer[i].sinr) +
				DBM_TO_MW(sinr[i]));
			if(pd->blerModel == LTENR_BLER_ENABLE)
				process->transmissionBuffer[i].bler = LTENR_GetBLER(BlerTableId, amcInfo[i]->mcsTable.mcsIndex, amcInfo[i]->BG, amcInfo[i]->CBS_, process->transmissionBuffer[i].sinr);
			else process->transmissionBuffer[i].bler = 0.0;
			//process->transmissionBuffer[i].bler = 1.0;
		}
	}
	else
	{
		if (amcInfo == NULL || si == NULL) return;

		process->amcInfo = amcInfo;
		process->layerCount = layerCount;

		for (UINT i = 0; i < layerCount; i++)
		{
			if (amcInfo[i] == NULL) continue;

			double R = amcInfo[i]->mcsTable.codeRate / 1024.0;
			si->TBSLayer[i] = (si->bitsPerPRBLayer[i] * si->allocatedPRBCount);
			amcInfo[i]->tbsSize = si->TBSLayer[i];
			process->transmissionBuffer[i].bitsPerPRB = si->bitsPerPRBLayer[i];
			process->transmissionBuffer[i].sinr = sinr[i];
			amcInfo[i]->BG = LTENR_Mutiplexer_LDPC_SelectBaseGraph(si->TBSLayer[i], R);
			LTENR_Multiplexer_ComputeCodeBlockSize(amcInfo[i]->BG, si->TBSLayer[i], &amcInfo[i]->nCodeBlock, &amcInfo[i]->CBS, &amcInfo[i]->CBS_);
			if (pd->blerModel == LTENR_BLER_ENABLE)
				process->transmissionBuffer[i].bler = LTENR_GetBLER(BlerTableId, amcInfo[i]->mcsTable.mcsIndex, amcInfo[i]->BG, amcInfo[i]->CBS, sinr[i]);
			else process->transmissionBuffer[i].bler = 0.0;
			/*if (process->transmissionBuffer[i].bler <= 1.0)
			{
				fprintf(stderr, "\ngnb %d:%d-->ue %d:%d %s\n",
					entity->gNBId, entity->gNBIf, entity->ueId, entity->ueIf,
					entity->isUpinkEntity ? "Uplink" : "Downlink");
				fprintf(stderr, "\nTable_%d_MCS_%d_BG_%d_CBS_%d_SINR_%lf=%lf\n",
					BlerTableId, amcInfo[i]->mcsTable.mcsIndex, amcInfo[i]->BG,
					amcInfo[i]->CBS, sinr[i], process->transmissionBuffer[i].bler);
			}*/
			//process->transmissionBuffer[i].bler = 1.0;
			WriteTextLog(entity, "\"Layer = %d, TBS=%d, CBS_=%d, Count=%d\"", i + 1, si->TBSLayer[i], amcInfo[i]->CBS_, amcInfo[i]->nCodeBlock);
		}
	}
}

static void AllocateCBG(ptrLTENR_HARQ_PROCESS process, UINT maxCBGCount)
{
	if (process->NDI == false) return; //No need to form new CBG. Transmit error CBG

	for (UINT i = 0; i < process->layerCount; i++)
	{
		ptrLTENR_HARQ_BUFFER buff = &process->transmissionBuffer[i];
		buff->CBGCount = min(process->amcInfo[i]->nCodeBlock, maxCBGCount);
		int x = process->amcInfo[i]->nCodeBlock / buff->CBGCount;
		int r = process->amcInfo[i]->nCodeBlock % buff->CBGCount;
		for (UINT c = 0; c < buff->CBGCount; c++)
		{
			buff->CBGList[c].CBCount = x + (r <= 0 ? 0 : 1);
			r--;
			if(buff->CBGList[c].CBErrorStatus)
				free(buff->CBGList[c].CBErrorStatus);
			if (buff->CBGList[c].CBCount != 0)
				buff->CBGList[c].CBErrorStatus = calloc(buff->CBGList[c].CBCount, sizeof * buff->CBGList[c].CBErrorStatus);
			else buff->CBGList[c].CBErrorStatus = NULL;
			buff->CBGList[c].isErrored = PacketStatus_NoError;
		}
	}
}

void fn_NetSim_LTENR_HARQ_AllocateCBG(NETSIM_ID gnbId, NETSIM_ID gnbIf, int caId)
{
	ptrLTENR_GNBMAC mac = LTENR_GNBMAC_GET(gnbId, gnbIf);
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);
	ptrLTENR_FRAMEINFO frame = phy->currentFrameInfo;
	ptrLTENR_SCHEDULERINFO si = mac->schedulerInfo[caId];

	ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo = phy->associatedUEPhyInfo;
	while (assocInfo)
	{
		if (assocInfo->isAssociated)
		{
			ptrLTENR_HARQ_ENTITY entity = si->slotType == SLOT_DOWNLINK ? &assocInfo->harq->downlinkHARQEntity[caId] : &assocInfo->harq->uplinkHARQEntity[caId];
			ptrLTENR_HARQ_PROCESS process = &entity->harqProcess[entity->currentProcessId - 1];
			process->isUPLinkProcess = si->slotType == SLOT_UPLINK;
			process->caId = caId;
			process->slotId = frame->slotId;
			process->frameId = frame->frameId;
			process->subFrameId = frame->subFrameId;
			ptrLTENR_UESCHEDULERINFO sinfo = LTENR_MACScheduler_FindInfoForUE(si, assocInfo->ueId, assocInfo->ueIf, si->slotType == SLOT_UPLINK);
			ptrLTENR_AMCINFO* amc = si->slotType == SLOT_DOWNLINK ? assocInfo->downlinkAMCInfo[caId] : assocInfo->uplinkAMCInfo[caId];
			ptrLTENR_UEPHY uePhy = LTENR_UEPHY_GET(assocInfo->ueId, assocInfo->ueIf);
			UINT layerCount = si->slotType == SLOT_DOWNLINK ? LTENR_PHY_GET_DLLAYER_COUNT(uePhy) : LTENR_PHY_GET_ULLAYER_COUNT(uePhy);
			double* sinr = si->slotType == SLOT_DOWNLINK ? assocInfo->propagationInfo[caId]->downlink.SINR_db : assocInfo->propagationInfo[caId]->uplink.SINR_db;
			
			if (sinfo->NDI) WriteTextLog(entity, "Allocated PRBs for new data = %d", sinfo->allocatedPRBCount);
			else WriteTextLog(entity, "Allocated PRBs for retransmission = %d", sinfo->PRBReqdForHARQRetransmission);
			if (sinfo->allocatedPRBCount != 0 || process->NDI == false)
			{
				StoreAMCInfo(entity, process, sinfo, amc, layerCount, sinr, entity->BlerTableId);
				AllocateCBG(process, entity->maxCBGPerTB);
			}
			else
			{
				ResetCBG(process);
			}
		}
		LTENR_ASSOCIATEDUEPHYINFO_NEXT(assocInfo);
	}
}
#pragma endregion //SLOT_START

#pragma region MAC_OUT
void fn_NetSim_LTENR_HARQ_StoreDLPacket()
{
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	NETSIM_ID gnbId = pstruEventDetails->nDeviceId;
	NETSIM_ID gnbIf = pstruEventDetails->nInterfaceId;
	NETSIM_ID ueId = packet->nReceiverId;
	NETSIM_ID ueIf = LTENR_FIND_ASSOCIATEINTERFACE(gnbId, gnbIf, ueId);
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);
	ptrLTENR_FRAMEINFO fInfo = phy->currentFrameInfo;
	ptrLTENR_ASSOCIATEDUEPHYINFO info = LTENR_ASSOCIATEDUEPHYINFO_GET(gnbId, gnbIf, ueId, ueIf);
	if (info == NULL || info->isAssociated == false)
	{
		fnNetSimError("UE %d:%d is not connected to gNB/eNB %d:%d\n", ueId, ueIf, gnbId, gnbIf);
		fn_NetSim_Packet_FreePacket(packet);
		return;
	}

	int caId = LTENR_GNBPHY_GETCURRENTCAID(gnbId, gnbIf);
	ptrLTENR_HARQ_ENTITY entity = &info->harq->downlinkHARQEntity[caId];

	ptrLTENR_HARQ_PROCESS process = &entity->harqProcess[entity->currentProcessId - 1];
	if (process->packetBuffer.head == NULL)
	{
		process->packetBuffer.head = packet;
		process->packetBuffer.tail = packet;
	}
	else
	{
		process->packetBuffer.tail->pstruNextPacket = packet;
		process->packetBuffer.tail = packet;
	}
}

void fn_NetSim_LTENR_HARQ_StoreULPacket()
{
	NetSim_PACKET* packet = pstruEventDetails->pPacket;
	NETSIM_ID ueId = pstruEventDetails->nDeviceId;
	NETSIM_ID ueIf = pstruEventDetails->nInterfaceId;
	ptrLTENR_UEPHY uphy = LTENR_UEPHY_GET(ueId, ueIf);
	NETSIM_ID gnbId = uphy->gnBId;
	NETSIM_ID gnbIf = uphy->gnbIf;
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);
	ptrLTENR_FRAMEINFO fInfo = phy->currentFrameInfo;
	ptrLTENR_ASSOCIATEDUEPHYINFO info = LTENR_ASSOCIATEDUEPHYINFO_GET(gnbId, gnbIf, ueId, ueIf);
	if (info == NULL || info->isAssociated == false)
	{
		fnNetSimError("UE %d:%d is not connected to gNB/eNB %d:%d\n", ueId, ueIf, gnbId, gnbIf);
		fn_NetSim_Packet_FreePacket(packet);
		return;
	}

	int caId = LTENR_GNBPHY_GETCURRENTCAID(gnbId, gnbIf);
	ptrLTENR_HARQ_ENTITY entity = &info->harq->uplinkHARQEntity[caId];

	ptrLTENR_HARQ_PROCESS process = &entity->harqProcess[entity->currentProcessId - 1];
	if (process->packetBuffer.head == NULL)
	{
		process->packetBuffer.head = packet;
		process->packetBuffer.tail = packet;
	}
	else
	{
		process->packetBuffer.tail->pstruNextPacket = packet;
		process->packetBuffer.tail = packet;
	}
}

static void AddMACINEvent(NETSIM_ID gnbId, NETSIM_ID gnbIf, ptrLTENR_ASSOCIATEDUEPHYINFO info, bool isUplink, NetSim_PACKET* packet)
{
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gnbId, gnbIf);
	
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = phy->currentFrameInfo->slotEndTime;
	pevent.dPacketSize = packet->pstruMacData->dPacketSize;
	if (packet->pstruAppData)
	{
		pevent.nApplicationId = packet->pstruAppData->nApplicationId;
		pevent.nSegmentId = packet->pstruAppData->nSegmentId;
	}
	if (isUplink)
	{
		pevent.nDeviceId = gnbId;
		pevent.nInterfaceId = gnbIf;
		pevent.nDeviceType = DEVICE_TYPE(gnbId);
	}
	else
	{
		pevent.nDeviceId = info->ueId;
		pevent.nInterfaceId = info->ueIf;
		pevent.nDeviceType = DEVICE_TYPE(info->ueId);
	}

	pevent.nEventType = MAC_IN_EVENT;
	pevent.nPacketId = packet->nPacketId;
	pevent.nProtocolId = LTENR_PROTODATA_GET(gnbId, gnbIf)->macProtocol;
	pevent.pPacket = packet;
	fnpAddEvent(&pevent);
}

static void SendToReceiver(NETSIM_ID gnbId, NETSIM_ID gnbIf, ptrLTENR_ASSOCIATEDUEPHYINFO info, ptrLTENR_HARQ_PROCESS process)
{
	bool isNonError = true;
	for (UINT l = 0; l < process->layerCount; l++)
	{
		if (process->transmissionBuffer[l].isErrored == PacketStatus_Error)
		{
			isNonError = false;
			break;
		}
	}

	if (isNonError == false) return; //TB is error

	NetSim_PACKET* packet = process->packetBuffer.head;
	if (packet) AddMACINEvent(gnbId, gnbIf, info, process->isUPLinkProcess, packet);
	process->packetBuffer.tail = NULL;
	process->packetBuffer.head = NULL;
}

static void ReportToOLLA(NETSIM_ID gnbId, NETSIM_ID gnbIf, ptrLTENR_ASSOCIATEDUEPHYINFO info, int caId, ptrLTENR_HARQ_PROCESS process)
{
	if (process->transmissionNumber != 0) return;

	for (UINT l = 0; l < process->layerCount; l++)
	{
		if (process->transmissionBuffer[l].isErrored)
			LTENR_OLLA_MarkTBSError(gnbId, gnbIf, info->ueId, info->ueIf, process->isUPLinkProcess, caId, l + 1);
		else LTENR_OLLA_MarkTBSSuccess(gnbId, gnbIf, info->ueId, info->ueIf, process->isUPLinkProcess, caId, l + 1);
	}
}

void fn_NetSim_LTENR_HARQ_TransmitDownLinkCBG(NETSIM_ID gnbId, NETSIM_ID gnbIf, ptrLTENR_ASSOCIATEDUEPHYINFO info, int caId)
{
	ptrLTENR_HARQ_ENTITY entity = &info->harq->downlinkHARQEntity[caId];
	ptrLTENR_HARQ_PROCESS process = &entity->harqProcess[entity->currentProcessId - 1];
	DecideCBGError(process);
	ReportToOLLA(gnbId, gnbIf, info, caId, process);
	WriteCBGLog(gnbId, gnbIf, info->ueId, info->ueIf, process);
	SendToReceiver(gnbId, gnbIf, info, process);
}

void fn_NetSim_LTENR_HARQ_TransmitUpLinkCBG(NETSIM_ID gnbId, NETSIM_ID gnbIf, ptrLTENR_ASSOCIATEDUEPHYINFO info, int caId)
{
	ptrLTENR_HARQ_ENTITY entity = &info->harq->uplinkHARQEntity[caId];
	ptrLTENR_HARQ_PROCESS process = &entity->harqProcess[entity->currentProcessId - 1];
	DecideCBGError(process);
	ReportToOLLA(gnbId, gnbIf, info, caId, process);
	WriteCBGLog(gnbId, gnbIf, info->ueId, info->ueIf, process);
	SendToReceiver(gnbId, gnbIf, info, process);
}

void fn_NetSim_LTENR_HARQ_Transmit(NETSIM_ID gNBId, NETSIM_ID gNBIf, int caId)
{
	ptrLTENR_GNBMAC mac = LTENR_GNBMAC_GET(gNBId, gNBIf);
	ptrLTENR_SCHEDULERINFO si = mac->schedulerInfo[caId];
	ptrLTENR_ASSOCIATEDUEPHYINFO info = ((ptrLTENR_GNBPHY)LTENR_GNBPHY_GET(gNBId, gNBIf))->associatedUEPhyInfo;
	while (info)
	{
		if (info->isAssociated)
		{
			if(si->slotType == SLOT_DOWNLINK)
				fn_NetSim_LTENR_HARQ_TransmitDownLinkCBG(gNBId, gNBIf, info, caId);
			else
				fn_NetSim_LTENR_HARQ_TransmitUpLinkCBG(gNBId, gNBIf, info, caId);
		}
		LTENR_ASSOCIATEDUEPHYINFO_NEXT(info);
	}
}
#pragma endregion //MAC_OUT
