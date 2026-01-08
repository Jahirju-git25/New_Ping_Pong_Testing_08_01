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
#include "LTENR_PHY.h"
#pragma endregion

#pragma region DEFINE
#define DELTAUP		0.2 //dB
#define MAXDELTA	0
#define MINDELTA	-20
#pragma endregion

#pragma region STRUCT
typedef struct stru_OLLA_Handler
{
	double deltaOLLA;
	double targerBLER;
	double deltaDown;
	double deltaUp;
}OLLAHANDLER, * ptrOLLAHANDLER;

typedef struct stru_OLLAContainer
{
	NETSIM_ID gNBId;
	NETSIM_ID gNBIf;
	NETSIM_ID ueId;
	NETSIM_ID ueIf;

	ptrOLLAHANDLER uplinkHandler[MAX_CA_COUNT][MAX_LAYER_COUNT];
	ptrOLLAHANDLER downlinkHandler[MAX_CA_COUNT][MAX_LAYER_COUNT];
}OLLACONTAINER, * ptrOLLACONTAINER;
#pragma endregion

#pragma region UTILITY
static ptrOLLACONTAINER olla_handler_init(NETSIM_ID gNBId, NETSIM_ID gNBIf,
	NETSIM_ID ueId, NETSIM_ID ueIf)
{
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(gNBId, gNBIf);

	ptrLTENR_ASSOCIATEDUEPHYINFO info = LTENR_ASSOCIATEDUEPHYINFO_GET(gNBId, gNBIf, ueId, ueIf);

	ptrOLLACONTAINER container = calloc(1, sizeof * container);
	container->gNBId = gNBId;
	container->ueId = ueId;
	container->gNBIf = gNBIf;
	container->ueIf = ueIf;
	info->OLLAContainer = container;

	for (int i = 0; i < MAX_CA_COUNT; i++)
	{
		for (int j = 0; j < MAX_LAYER_COUNT; j++)
		{
			container->uplinkHandler[i][j] = calloc(1, sizeof * container->uplinkHandler[i][j]);
			container->uplinkHandler[i][j]->targerBLER = pd->targetBLER;
			container->uplinkHandler[i][j]->deltaUp = DELTAUP;
			container->uplinkHandler[i][j]->deltaDown = (pd->targetBLER / (1 - pd->targetBLER)) * DELTAUP;

			container->downlinkHandler[i][j] = calloc(1, sizeof * container->downlinkHandler[i][j]);
			container->downlinkHandler[i][j]->targerBLER = pd->targetBLER;
			container->downlinkHandler[i][j]->deltaUp = DELTAUP;
			container->downlinkHandler[i][j]->deltaDown = (pd->targetBLER / (1 - pd->targetBLER)) * DELTAUP;
		}
	}
	return container;
}

static ptrOLLAHANDLER olla_handler_find(NETSIM_ID gNBId, NETSIM_ID gNBIf,
	NETSIM_ID ueId, NETSIM_ID ueIf,
	bool isUplink, UINT caId, UINT layerId)
{
//	assert(caId == 0);
//	assert(layerId == 1);
	ptrOLLACONTAINER container = LTENR_ASSOCIATEDUEPHYINFO_GET(gNBId, gNBIf, ueId, ueIf)->OLLAContainer;

	if (container == NULL) container = olla_handler_init(gNBId, gNBIf, ueId, ueIf);

	return isUplink ? container->uplinkHandler[caId][layerId - 1] : container->downlinkHandler[caId][layerId - 1];
}
#pragma endregion

#pragma region APIs
void LTENR_OLLA_MarkTBSError(NETSIM_ID gNBId, NETSIM_ID gNBIf,
	NETSIM_ID ueId, NETSIM_ID ueIf, bool isUplink,
	UINT caId, UINT layerId)
{
	ptrOLLAHANDLER handler = olla_handler_find(gNBId, gNBIf, ueId, ueIf, isUplink, caId, layerId);
	handler->deltaOLLA = handler->deltaOLLA + handler->deltaUp;
	if (handler->deltaOLLA < MINDELTA) handler->deltaOLLA = MINDELTA;
}

void LTENR_OLLA_MarkTBSSuccess(NETSIM_ID gNBId, NETSIM_ID gNBIf,
	NETSIM_ID ueId, NETSIM_ID ueIf, bool isUplink,
	UINT caId, UINT layerId)
{
	ptrOLLAHANDLER handler = olla_handler_find(gNBId, gNBIf, ueId, ueIf, isUplink, caId, layerId);
	handler->deltaOLLA = handler->deltaOLLA - handler->deltaDown;
	if (handler->deltaOLLA > MAXDELTA) handler->deltaOLLA = MAXDELTA;
}

double LTENR_OLLA_GetDeltaSINR(NETSIM_ID gNBId, NETSIM_ID gNBIf,
	NETSIM_ID ueId, NETSIM_ID ueIf, bool isUplink,
	UINT caId, UINT layerId)
{
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(gNBId, gNBIf);
	if (pd->isOLLAEnable)
	{
		ptrOLLAHANDLER handler = olla_handler_find(gNBId, gNBIf, ueId, ueIf, isUplink, caId, layerId);
		return handler->deltaOLLA;
	}
	else return 0.0;
}
#pragma endregion

#pragma region TEST
#define TEST_SINR		10
#define TEST_ALPHA		1.0
#define TEST_CBCOUNT	150
#define TEST_BLER		0.1
#define TEST_TBS_COUNT	500

static bool check_tbs_error(double bler)
{
	for (int i = 0; i < TEST_CBCOUNT; i++)
	{
		if (NETSIM_RAND_01() < bler) return true;
	}
	return false;
}

void test_olla(NETSIM_ID gNBId, NETSIM_ID gNBIf, NETSIM_ID ueId, NETSIM_ID ueIf)
{
	FILE* fp = fopen("test.csv", "w");
	if (fp == NULL) return;
	fprintf(fp, "SINR,DELTA,EFF,MCS_INDEX,BLER,isError,\n");
	double sinr = TEST_SINR;

	for (int i = 0; i < TEST_TBS_COUNT; i++)
	{
		double delta = LTENR_OLLA_GetDeltaSINR(gNBId, gNBIf, ueId, ueIf, false, 1, 1);
		double eb = DBM_TO_MW(sinr - delta);
		double eff = TEST_ALPHA * log2(1 + eb);
		ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(gNBId, gNBIf);
		LTENR_MCSINDEXTABLE mt = LTENR_GetMCSIndexTableFromSpectralEfficiency(phy->PDSCHConfig.mcsTable, eff);

		double bler = LTENR_GetBLER(phy->CSIReportConfig.tableId, mt.mcsIndex, 1, 8448, sinr);
		bool isError = check_tbs_error(bler);

		if (isError) LTENR_OLLA_MarkTBSError(gNBId, gNBIf, ueId, ueIf, false, 1, 1);
		else LTENR_OLLA_MarkTBSSuccess(gNBId, gNBIf, ueId, ueIf, false, 1, 1);

		fprintf(fp, "%lf,%lf,%lf,%d,%lf,%s,\n", sinr, delta, eff, mt.mcsIndex, bler,
			isError ? "Error" : "Success");
	}

	fclose(fp); 
	system("test.csv");
}
#pragma endregion
