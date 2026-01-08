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
* Author:    Shashi kant suman														*
* ----------------------------------------------------------------------------------*/

#include "main.h"
#include "stdafx.h"
#include "LTENR_PHY.h"
#include "LTENR_PropagationModel.h"
#include "MobilityInterface.h"
#include "NetSim_utility.h"

#define LTENR_MATLAB_MODEL_DEFAULT							_strdup("NONE")

//Rain
#define MATLAB_LOSS_RAIN_RAINRATE_DEFAULT				16
#define MATLAB_LOSS_RAIN_TILTANGLE_DEFAULT				0
#define MATLAB_LOSS_RAIN_ELEVATIONANGLE_DEFAULT			0
#define MATLAB_LOSS_RAIN_EXCEEDANCEPERCENTAGEOFRAINFALL_DEFAULT		0

//Fog
#define MATLAB_LOSS_FOG_TEMPERATURE_DEFAULT				15
#define MATLAB_LOSS_FOG_WATERDENSITY_DEFAULT			7.5

//GAS
#define MATLAB_LOSS_GAS_TEMPERATURE_DEFAULT				15
#define MATLAB_LOSS_GAS_WATERDENSITY_DEFAULT			7.5
#define MATLAB_LOSS_GAS_AIRPRESSURE_DEFAULT				10300

void netsim_matlab_configure_loss_model(ptrLTENR_PROPAGATIONCONFIG config, void* xmlNetSimNode)
{
	char* szVal;
	getXmlVar(&szVal, MATLAB_MODEL, xmlNetSimNode, 1, _STRING, LTENR);
	config->matlabLossModel = LTENR_ConvertStrToEnum(MATLAB_LOSSMODEL, szVal);
	free(szVal);

	switch(config->matlabLossModel)
	{
	case MATLAB_LOSSMODEL_NONE:
		//No configuration reqd
		break;
	case MATLAB_LOSSMODEL_RAIN:
		getXmlVar(&config->rainRate, RAINRATE, xmlNetSimNode, 1, _DOUBLE, MATLAB_LOSS_RAIN);
		getXmlVar(&config->tiltAngle, TILTANGLE, xmlNetSimNode, 1, _DOUBLE, MATLAB_LOSS_RAIN);
		getXmlVar(&config->elevationAngle, ELEVATIONANGLE, xmlNetSimNode, 1, _DOUBLE, MATLAB_LOSS_RAIN);
		getXmlVar(&config->exceedancePercentageOfRainfall, EXCEEDANCEPERCENTAGEOFRAINFALL, xmlNetSimNode, 1, _DOUBLE, MATLAB_LOSS_RAIN);
		break;
	case MATLAB_LOSSMODEL_FOG:
		getXmlVar(&config->temperature, TEMPERATURE, xmlNetSimNode, 1, _DOUBLE, MATLAB_LOSS_FOG);
		getXmlVar(&config->waterDensity, WATERDENSITY, xmlNetSimNode, 1, _DOUBLE, MATLAB_LOSS_FOG);
		break;
	case MATLAB_LOSSMODEL_GAS:
		getXmlVar(&config->temperature, TEMPERATURE, xmlNetSimNode, 1, _DOUBLE, MATLAB_LOSS_GAS);
		getXmlVar(&config->airPressure, AIRPRESSURE, xmlNetSimNode, 1, _DOUBLE, MATLAB_LOSS_GAS);
		getXmlVar(&config->waterDensity, WATERDENSITY, xmlNetSimNode, 1, _DOUBLE, MATLAB_LOSS_GAS);
		break;
	default:
		fnNetSimError("Unknown MATLAB loss model - %s\n", strMATLAB_LOSSMODEL[config->matlabLossModel]);
		break;
	}
}

static double netsim_matlab_get_fspl(ptrLTENR_PROPAGATIONINFO info)
{
	netsim_matlab_send_ascii_command("pathloss=netsim_matlab_free_space_pathloss(%lf,%lf,%lf,%lf,%lf,%lf,%lf)",
		info->centralFrequency_MHz * MHZ,
		info->gnbPos.X, info->gnbPos.Y, info->gnbPos.Z,
		info->uePos.X, info->uePos.Y, info->uePos.Z);
	char s[BUFSIZ];
	netsim_matlab_get_value(s, BUFSIZ, "pathloss", "double");
	return atof(s);
}

static double netsim_matlab_get_rainLoss(ptrLTENR_PROPAGATIONINFO info)
{
	netsim_matlab_send_ascii_command("loss=rainpl(%lf,%lf,%lf,%lf,%lf,%lf)",
		info->dist3D,
		info->centralFrequency_MHz * MHZ,
		info->propagationConfig->rainRate,
		info->propagationConfig->elevationAngle,
		info->propagationConfig->tiltAngle,
		info->propagationConfig->exceedancePercentageOfRainfall);
	char s[BUFSIZ];
	netsim_matlab_get_value(s, BUFSIZ, "loss", "double");
	return atof(s);
}

static double netsim_matlab_get_gasLoss(ptrLTENR_PROPAGATIONINFO info)
{
	if (info->centralFrequency_MHz * MHZ < 1 * GHZ)
	{
		fnNetSimError("MATLAB GAS loss model is only avaiable for frequency above than 1GHz\n");
		return 0;
	}

	netsim_matlab_send_ascii_command("loss=gaspl(%lf,%lf,%lf,%lf,%lf)",
									 info->dist3D,
									 info->centralFrequency_MHz * MHZ,
									 info->propagationConfig->temperature,
									 info->propagationConfig->airPressure,
									 info->propagationConfig->waterDensity);
	char s[BUFSIZ];
	netsim_matlab_get_value(s, BUFSIZ, "loss", "double");
	return atof(s);
}

static double netsim_matlab_get_fogLoss(ptrLTENR_PROPAGATIONINFO info)
{
	if (info->centralFrequency_MHz * MHZ < 10 * GHZ)
	{
		fnNetSimError("MATLAB Fog loss model is only avaiable for frequency above than 10GHz\n");
		return 0;
	}

	netsim_matlab_send_ascii_command("loss=fogpl(%lf,%lf,%lf,%lf)",
		info->dist3D,
		info->centralFrequency_MHz * MHZ,
		info->propagationConfig->temperature,
		info->propagationConfig->waterDensity);
	char s[BUFSIZ];
	netsim_matlab_get_value(s, BUFSIZ, "loss", "double");
	return atof(s);
}

double netsim_matlab_calculate_loss(ptrLTENR_PROPAGATIONINFO info)
{
	switch (info->propagationConfig->matlabLossModel)
	{
	case MATLAB_LOSSMODEL_NONE:
		return 0;
	case MATLAB_LOSSMODEL_RAIN:
		return netsim_matlab_get_rainLoss(info);
	case MATLAB_LOSSMODEL_FOG:
		return netsim_matlab_get_fogLoss(info);
	case MATLAB_LOSSMODEL_GAS:
		return netsim_matlab_get_gasLoss(info);
	default:
		fnNetSimError("Unknown MATLAB loss model - %s\n", strMATLAB_LOSSMODEL[info->propagationConfig->matlabLossModel]);
		break;
	}
	return 0;
}
