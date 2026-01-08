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
#pragma endregion

#pragma region CBSSIZE

int LTENR_Mutiplexer_LDPC_SelectBaseGraph(double TBS, double R) 
{
	if (TBS <= 292 || (TBS <= 3824 && R <= 0.67) || (R <= 0.25)) return 2;
	return 1;
}

void LTENR_Multiplexer_ComputeCodeBlockSize(int LDPCG, UINT TBS, UINT* C, UINT* cbs, UINT* cbs_)
{
	UINT Kcb = 0;
	double Kb = 0, K = 0.0, Zc = 384;
	int L = 0;
	if (LDPCG == 1)
	{
		Kcb = 8448;
		Kb = 22;
	}
	else
	{
		Kcb = 3840;
		if (TBS > 640) Kb = 10;
		else if (TBS > 560) Kb = 9;
		else if (TBS > 192) Kb = 8;
		else Kb = 6;
	}

	if (TBS > Kcb)
	{
		L = 24;
		*C = ceil(TBS * 1.0 / (Kcb - L));
		TBS = TBS + (*C) * L;
	}
	else
		*C = 1;

	double K_ = (ceil(TBS / (8.0 * (*C)))) * 8.0;

	int LDPC_len = sizeof LDPCLiftingSizeTable / sizeof LDPCLiftingSizeTable[0];
	for (int i = 0; i < LDPC_len; i++)
	{
		if (Kb * (double)LDPCLiftingSizeTable[i].LiftingSize > K_)
		{
			Zc = LDPCLiftingSizeTable[i].LiftingSize;
			break;
		}
	}

	if (LDPCG == 1)
	{
		K = 22 * Zc;// Code Block Size
	}
	else {
		K = 10 * Zc;// Code Block Size
	}

	*cbs = K;
	*cbs_ = K_;
}

#pragma endregion
