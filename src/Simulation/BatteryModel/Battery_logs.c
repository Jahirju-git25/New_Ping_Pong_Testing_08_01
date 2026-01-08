/************************************************************************************
* Copyright (C) 2022																*
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
* Author:    Yugank Goyal				                                                    *
*										                                            *
* ----------------------------------------------------------------------------------*/
#define _BATTERY_MODEL_CODE_
#include "main.h"
#include "BatteryModel.h"
#include "NetSim_utility.h"

static FILE* fpBatterylog = NULL;

UINT DEVICE_ID_LIST[] = { 0 };//When { 0 } log is written for all ZigBee Devices. Specific ZigBee Device ID's can be defined
//for example {1,2,3} log will be written only for ZigBee Device ID's 1, 2 and 3 as transmitter/receiver

static bool validate_log(NETSIM_ID nDevID)
{
	int count = sizeof(DEVICE_ID_LIST) / sizeof(int);
	if (count == 1 && DEVICE_ID_LIST[0] == 0) return true;
	else
	{
		for (int i = 0; i < count; i++)
		{
			if (DEVICE_ID_LIST[i] == DEVICE_CONFIGID(nDevID))
				return true;
		}
	}
	return false;
}

void init_battery_log(ptrBATTERY battery)
{
	if (get_protocol_log_status("ENERGY_LOG"))
	{
		char s[BUFSIZ];
		sprintf(s, "%s\\%s", pszIOLogPath, "Energy_log.csv");
		fpBatterylog = fopen(s, "w");
		if (!fpBatterylog)
		{
			fnSystemError("Unable to open %s file", s);
			perror(s);
		}
		else
		{
			fprintf(fpBatterylog, "%s,%s,%s,%s,",
				"Current Time(ms)", "Mode Switch Time(ms)", "Previous Mode", "New Mode");

			fprintf(fpBatterylog, "%s,%s,%s,%s,",
				"Device Name", "Device Id", "Voltage(V)", "Mode Current(mA)");

			fprintf(fpBatterylog, "%s,%s,",
				"Initial Energy(mJ)", "Remaining Energy(mJ)");

			for (int i = 1;i < battery->modeCount;i++)
			{
				fprintf(fpBatterylog, "%s,Running Total %s,",
					battery->mode[i].heading, battery->mode[i].heading);
			}

			fprintf(fpBatterylog, "%s,%s,%s,%s,",
				"Harvested Energy(mJ)", "Running Total Harvested Energy(mJ)", "Consumed Energy(mJ)", "Running Total Consumed Energy(mJ)");

			fprintf(fpBatterylog, "\n");
			if (nDbgFlag) fflush(fpBatterylog);
		}
	}
}

void  Battery_RadioMeasurements_Log(ptrBATTERY battery, double harvestedEnergy, double ConsumedEnergy, int prev_mode, int curr_mode, double mode_change_time, ptrBATTERYMODE pm)
{
	if (!fpBatterylog)
		init_battery_log(battery);
	if (fpBatterylog == NULL || (!validate_log(battery->deviceId))) { return; }

	char old_mode[BUFSIZ],new_mode[BUFSIZ];
	switch (prev_mode)
	{
	case 1:sprintf(old_mode, "RX_ON_IDLE");
		break;
	case 2:sprintf(old_mode, "RX_ON_BUSY");
		break;
	case 3:sprintf(old_mode, "TRX_ON_BUSY");
		break;
	case 4:sprintf(old_mode, "SLEEP");
		break;
	default:
		sprintf(old_mode, "RX_OFF");
	}

	switch (curr_mode)
	{
	case 1:sprintf(new_mode, "RX_ON_IDLE");
		break;
	case 2:sprintf(new_mode, "RX_ON_BUSY");
		break;
	case 3:sprintf(new_mode, "TRX_ON_BUSY");
		break;
	case 4:sprintf(new_mode, "SLEEP");
		break;
	default:
		sprintf(new_mode, "RX_OFF");
	}

	fprintf(fpBatterylog, "%lf,%lf,%s,%s,%s,",
		ldEventTime / MILLISECOND, mode_change_time / MILLISECOND, old_mode, new_mode, DEVICE_NAME(battery->deviceId));

	fprintf(fpBatterylog, "%d,%lf,%lf,",
		DEVICE_CONFIGID(battery->deviceId), battery->voltage, pm->current);

	fprintf(fpBatterylog, "%lf,%lf,",
		battery->initialEnergy,
		battery->remainingEnergy);

	for (int i = 1;i < battery->modeCount;i++)
	{
		if(prev_mode == battery->mode[i].mode)
		{
			fprintf(fpBatterylog, "%lf,", ConsumedEnergy);
			switch (prev_mode)
			{
			case 1:fprintf(fpBatterylog, "%lf,", battery->SumIdleEnergy);
				break;
			case 2:fprintf(fpBatterylog, "%lf,", battery->SumReceiveEnergy);
				break;
			case 3:fprintf(fpBatterylog, "%lf,", battery->SumTransmitEnergy);
				break;
			case 4:fprintf(fpBatterylog, "%lf,", battery->SumSleepEnergy);
				break;
			default:fprintf(fpBatterylog, "%lf,", 0.0);
				break;
			}
		}
		else {
			fprintf(fpBatterylog, "%lf,", 0.0);
			switch (battery->mode[i].mode)
			{
			case 1:fprintf(fpBatterylog, "%lf,", battery->SumIdleEnergy);
				break;
			case 2:fprintf(fpBatterylog, "%lf,", battery->SumReceiveEnergy);
				break;
			case 3:fprintf(fpBatterylog, "%lf,", battery->SumTransmitEnergy);
				break;
			case 4:fprintf(fpBatterylog, "%lf,", battery->SumSleepEnergy);
				break;
			default:fprintf(fpBatterylog, "%lf,", 0.0);
				break;
			}
		}
	}

		fprintf(fpBatterylog, "%lf,%lf,%lf,%lf,",
			harvestedEnergy, battery->SumHarvestedEnergy,
			ConsumedEnergy, battery->SumConsumedEnergy);

	fprintf(fpBatterylog, "\n");
	if (nDbgFlag) fflush(fpBatterylog);
}

void close_battery_log()
{
	if (fpBatterylog)
		fclose(fpBatterylog);
}

bool get_protocol_log_status(char* logname)
{
	FILE* fp;
	char str[BUFSIZ];
	sprintf(str, "%s/%s", pszIOPath, "ProtocolLogsConfig.txt");
	fp = fopen(str, "r");
	if (!fp)
		return false;

	char data[BUFSIZ];
	while (fgets(data, BUFSIZ, fp))
	{
		lskip(data);
		sprintf(str, "%s=true", logname);
		if (!_strnicmp(data, str, strlen(str)))
		{
			fclose(fp);
			return true;
		}
	}
	fclose(fp);

	return false;
}
