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

#include "main.h"
#include "Mobility.h"
#include "Animation.h"

static FILE* Mobilitylog = NULL;

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

void init_mobility_log()
{
	if (get_protocol_log_status("MOBILITY_LOG"))
	{
		char s[BUFSIZ];
		sprintf(s, "%s\\%s", pszIOLogPath, "Mobility_log.csv");
		Mobilitylog = fopen(s, "w");
		if (!Mobilitylog)
		{
			fnSystemError("Unable to open %s file", s);
			perror(s);
		}
		else
		{
			fprintf(Mobilitylog, "%s,%s,%s,%s,%s,%s,",
				"Time(ms)", "Device Name", "Device Id", "Position X(m)", "Position Y(m)", "Position Z(m)");

		}
		fprintf(Mobilitylog, "\n");
		if (nDbgFlag) fflush(Mobilitylog);
	}
}

void close_mobility_logs()
{
	if (Mobilitylog)
		fclose(Mobilitylog);
}
void log_mobility(MOVENODE mob_log)
{
	if (Mobilitylog == NULL || !validate_log(mob_log.d))
	{
		return;
	}

	fprintf(Mobilitylog, "%lf,%s,%d,%lf,%lf,%lf,", mob_log.time / MILLISECOND, DEVICE_NAME(fn_NetSim_GetDeviceIdByConfigId(mob_log.d)),
		mob_log.d, mob_log.x, mob_log.y, mob_log.z);
	fprintf(Mobilitylog, "\n");

	if (nDbgFlag) fflush(Mobilitylog);
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
		if (!strnicmp(data, str, strlen(str)))
		{
			fclose(fp);
			return true;
		}
	}
	fclose(fp);

	return false;
}



