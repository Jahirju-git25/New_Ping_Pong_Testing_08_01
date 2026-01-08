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
#include "LTENR_AntennaModel.h"
#include "LTENR_PHY.h"
#pragma endregion
#define LTENR_TX_ANTENNA_COUNT_DEFAULT		1
#define LTENR_RX_ANTENNA_COUNT_DEFAULT		1
#define LTENR_ELEMENT_GAIN_DEFAULT 0
#define LTENR_BORESIGHT_ANGLE_DEFAULT 60
#define LTENR_BEAM_WIDTH_DEFAULT 65
#define LTENR_FRONT_TO_BACK_RATIO_DEFAULT 30
#define LTENR_SECTOR_ID_DEFAULT 1

#define ANGLE_TO_RADIANS(angle) (angle * M_PI / 180.0)

void LTENR_CONFIGURE_ANTENNA(NETSIM_ID d, NETSIM_ID in, ptrLTENR_ANTENNA antenna, void* xmlNetSimNode)
{
	void* xmlChild = fn_NetSim_xmlGetFirstChildElement(xmlNetSimNode, "ANTENNA");
	getXmlVar(&antenna->txAntennaCount, TX_ANTENNA_COUNT, xmlChild, 1, _UINT, LTENR);
	getXmlVar(&antenna->rxAntennaCount, RX_ANTENNA_COUNT, xmlChild, 1, _UINT, LTENR);
	if (isGNB(d, in))
	{
		antenna->antennaType = ConfigReadEnum(xmlChild, "ANTENNA_TYPE", LTENR_ANTENNA_TYPE, true);
		switch (antenna->antennaType)
		{
		case LTENR_OMNIDIRECTIONAL:
			break; //Nothing to configure
		case LTENR_SECTOR:
			antenna->antennaModel = ConfigReadEnum(xmlChild, "ANTENNA_MODEL", LTENR_ANTENNA_MODEL, true);
			switch (antenna->antennaModel)
			{
			case LTENR_2D_PASSIVE_ANTENNA:
				antenna->antennaModel = ConfigReadEnum(xmlChild, "ANTENNA_MODEL", LTENR_ANTENNA_MODEL, true);
				getXmlVar(&antenna->boresightAngle, BORESIGHT_ANGLE, xmlChild, 1, _DOUBLE, LTENR);
				getXmlVar(&antenna->elementGain, ELEMENT_GAIN, xmlChild, 1, _DOUBLE, LTENR);
				getXmlVar(&antenna->beamwidth, BEAM_WIDTH, xmlChild, 1, _DOUBLE, LTENR);
				getXmlVar(&antenna->frontToBackRatio, FRONT_TO_BACK_RATIO, xmlChild, 1, _DOUBLE, LTENR);
				break;
			default:
				fnNetSimError("Unknown Antenna Model for device %d interface %d.\n", d, in);
				break;
			}
			break;
		default:
			fnNetSimError("Unknown Antenna Type for device %d interface %d.\n", d, in);
			break;
		}
	}

}

void LTENR_ANTENNA_SET_LAYER_COUNT(ptrLTENR_ANTENNA gnbAntenna, ptrLTENR_ANTENNA ueAntenna)
{
	if (gnbAntenna->txAntennaCount < ueAntenna->rxAntennaCount ||
		gnbAntenna->rxAntennaCount < ueAntenna->txAntennaCount)
	{
		fnNetSimError("Warning: gNB/eNB antenna count is less than the UE antenna count.\n"
					  "This configuration is not typical. Cross-check Antenna configuration in both gNB/eNB and UE.\n");
	}
	ueAntenna->downlinkLayerCount = min(gnbAntenna->txAntennaCount, ueAntenna->rxAntennaCount);
	ueAntenna->uplinkLayerCount = min(gnbAntenna->rxAntennaCount, ueAntenna->txAntennaCount);
}

UINT LTENR_ANTENNA_GET_LAYER_COUNT(ptrLTENR_ANTENNA ueAntenna, bool isUplink)
{
	if (isUplink) return ueAntenna->uplinkLayerCount;
	else return ueAntenna->downlinkLayerCount;
}

UINT LTENR_ANTENNA_GET_LAYER_COUNT_FOR_NONASSOCIATED_UE(ptrLTENR_ANTENNA gnBAntenna, ptrLTENR_ANTENNA ueAntenna, bool isUplink)
{
	if (isUplink)
		return min(gnBAntenna->rxAntennaCount, ueAntenna->txAntennaCount);
	else
		return min(gnBAntenna->txAntennaCount, ueAntenna->rxAntennaCount);
}

_declspec(dllexport) double LTENR_PHY_GetAntennaGain(ptrLTENR_PROPAGATIONINFO info, ptrLTENR_ANTENNA antenna)
{

	double gNBX = info->gnbPos.X;
	double gNBY = info->gnbPos.Y;

	double ueX = info->uePos.X;
	double ueY = info->uePos.Y;

	double dx = ueX - gNBX;
	double dy = ueY - gNBY;
	double angle = atan2(dy, dx) * 180 / M_PI;
	double antennaGain = 0;

	// Ensure the angle is positive
	if (angle < 0) {
		angle += 360;
	}

	double phi = ANGLE_TO_RADIANS(angle) - ANGLE_TO_RADIANS(antenna->boresightAngle);
	while (phi < -M_PI)
	{
		phi += M_PI + M_PI;
	}
	while (phi > M_PI)
	{
		phi -= M_PI + M_PI;
	}

	//Horizontal Radiation Pattern
	double Ah = -min(12 * pow(phi / ANGLE_TO_RADIANS(antenna->beamwidth), 2), antenna->frontToBackRatio);
	antennaGain = antenna->elementGain - min(-Ah, antenna->frontToBackRatio);

	return antennaGain;
}
