#pragma once
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
#ifndef _NETSIM_LTENR_MULTIPLEXING_H_
#define _NETSIM_LTENR_MULTIPLEXING_H_
#ifdef  __cplusplus
extern "C" {
#endif

#include "main.h"

#pragma region GLOBALVARIABLE_CONSTANT
#define CRC_SIZE			3 //bytes
#define LTENR_MAC_HDR_LEN	0 //bytes
	UINT MACHDRID;
#pragma endregion

#pragma region LIFTING_SIZE_TABLE
	typedef struct stru_LIFTING_SIZE_TABLE {
		int LiftingSize;
		int Index;
	}LIFTING_SIZE_TABLE, * ptrLIFTING_SIZE_TABLE;
	/*
	* Lookup table for minimum Z value for which Zc*kb>=K_
	*/
	static LIFTING_SIZE_TABLE LDPCLiftingSizeTable[] =
	{
		{2, 0},
		{3, 1},
		{4, 0},
		{5, 2},
		{6, 1},
		{7, 3},
		{8, 0},
		{9, 4},
		{10, 2},
		{11, 5},
		{12, 1},
		{13, 6},
		{14, 3},
		{15, 7},
		{16, 0},
		{18, 4},
		{20, 2},
		{22, 5},
		{24, 1},
		{26, 6},
		{28, 3},
		{30, 7},
		{32, 0},
		{36, 4},
		{40, 2},
		{44, 5},
		{48, 1},
		{52, 6},
		{56, 3},
		{60, 7},
		{64, 0},
		{72, 4},
		{80, 2},
		{88, 5},
		{96, 1},
		{104, 6},
		{112, 3},
		{120, 7},
		{128, 0},
		{144, 4},
		{160 ,2},
		{176 ,5},
		{192 ,1},
		{208 ,6},
		{224 ,3},
		{240 ,7},
		{256, 0},
		{288, 4},
		{320 ,2},
		{352 ,5},
		{384, 1},
	};

#pragma endregion

#ifdef  __cplusplus
}
#endif
#endif /* _NETSIM_LTENR_MULTIPLEXING_H_ */