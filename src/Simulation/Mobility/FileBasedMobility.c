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
#include "main.h"
#include "Mobility.h"
#include "NetSim_utility.h"

char* mobilityfile;
typedef struct stru_filebasedmobilityinfo
{
	NETSIM_ID d;
	double time;
	double x;
	double y;
	double z;
	struct stru_filebasedmobilityinfo* next;
}INFO, * ptrINFO;
ptrINFO* mobilityInfo;
ptrINFO* lastInfo;

static void add_mobility_info(NETSIM_ID d,
	double time,
	double x,
	double y,
	double z)
{
	if (!mobilityInfo)
	{
		mobilityInfo = calloc(NETWORK->nDeviceCount, sizeof * mobilityInfo);
		lastInfo = calloc(NETWORK->nDeviceCount, sizeof * lastInfo);
	}
	ptrINFO info = mobilityInfo[d - 1];

	ptrINFO t = calloc(1, sizeof * t);
	t->d = d;
	t->time = time;
	t->x = x;
	t->y = y;
	t->z = z;
	if (info)
	{
		lastInfo[d - 1]->next = t;
		lastInfo[d - 1] = t;
	}
	else
	{
		mobilityInfo[d - 1] = t;
		lastInfo[d - 1] = t;
	}
	fprintf(stderr, "%lf,%d,%lf,%lf,%lf\n",
		time / SECOND, d, x, y, z);
}

static void add_mobility_event()
{
	NETSIM_ID d;
	if (!mobilityInfo)
		return;
	for (d = 0; d < NETWORK->nDeviceCount; d++)
	{
		if (!mobilityInfo[d])
			continue;
		NetSim_MOBILITY* mob = DEVICE_MOBILITY(d + 1);
		mob->pstruCurrentPosition = calloc(1, sizeof * mob->pstruCurrentPosition);
		mob->pstruNextPosition = calloc(1, sizeof * mob->pstruNextPosition);
		ptrINFO i = mobilityInfo[d];
		pstruEventDetails->dEventTime = i->time;
		pstruEventDetails->dPacketSize = 0;
		pstruEventDetails->nApplicationId = 0;
		pstruEventDetails->nDeviceId = d + 1;
		pstruEventDetails->nDeviceType = NETWORK->ppstruDeviceList[d]->nDeviceType;
		pstruEventDetails->nEventType = TIMER_EVENT;
		pstruEventDetails->nInterfaceId = 0;
		pstruEventDetails->nPacketId = 0;
		pstruEventDetails->nProtocolId = PROTOCOL_MOBILITY;
		pstruEventDetails->nSubEventType = 0;
		pstruEventDetails->pPacket = NULL;
		fnpAddEvent(pstruEventDetails);
	}
}

void process_filebased_mobility_event()
{
	if (!mobilityInfo)
		return;
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	ptrINFO i = mobilityInfo[d - 1];
	if (!i)
		return;
	NetSim_MOBILITY* mob = DEVICE_MOBILITY(d);
	NetSim_COORDINATES* c = mob->pstruCurrentPosition;
	NetSim_COORDINATES* n = mob->pstruNextPosition;
	NetSim_COORDINATES* dc = DEVICE(d)->pstruDevicePosition;
	dc->X = i->x;
	dc->Y = i->y;
	dc->Z = i->z;
	c->X = i->x;
	c->Y = i->y;
	c->Z = i->z;
	if (i->next)
	{
		n->X = i->next->x;
		n->Y = i->next->y;
		n->Z = i->next->z;
	}
	mobilityInfo[d - 1] = i->next;
	free(i);
	mobility_pass_position_to_animation(pstruEventDetails->nDeviceId,
		pstruEventDetails->dEventTime,
		DEVICE_POSITION(pstruEventDetails->nDeviceId));
	if (mobilityInfo[d - 1])
	{
		pstruEventDetails->dEventTime = mobilityInfo[d - 1]->time;
		fnpAddEvent(pstruEventDetails);
	}
}

/** This function is to free the file pointers */
int FileBasedMobilityPointersFree()
{
	NETSIM_ID d;
	if (mobilityInfo)
	{
		for (d = 0; d < NETWORK->nDeviceCount; d++)
		{
			ptrINFO i = mobilityInfo[d];
			while (i)
			{
				mobilityInfo[d] = i->next;
				free(i);
				i = mobilityInfo[d];
			}
		}
		free(mobilityInfo);
	}
	return 0;
}

/** This function is to open the file, to define position pointers and to set the initial positions for all the nodes */
int FileBasedMobilityReadingFile()
{
	int lineno = 0;
	double time, x, y, z;
	NETSIM_ID d;
	FILE* fp;
	char str[BUFSIZ];
	bool istext = false;

	if (!mobilityInfo)
	{
		fprintf(stderr, "Reading File Based Mobility Input:\n");
		sprintf(str, "%s/%s", pszIOPath, mobilityfile);
		fp = fopen(str, "r");
		if (!fp)
			istext = true;

		if (istext)
		{
		sprintf(str, "%s/%s", pszIOPath, "mobility.txt");
		fp = fopen(str, "r");
		if (!fp)
		{
				fnSystemError("Unable to open mobility.csv/mobility.txt file.\n", str);
			perror(str);
			return -1;
		}
	}
	}
	else
	{
		return -1;
	}

	char data[BUFSIZ];
	bool fileEmpty = true;
	while (fgets(data, BUFSIZ, fp))
	{
		lineno++;
		lskip(data);
		if (*data == '#' || *data == 0)
			continue;
		fileEmpty = false;
		if (istext)
		{
		if (*data != '$')
		{
			fprintf(stderr, "In mobility.txt, invalid data at line no %d\n", lineno);
			continue;
		}
		if (*(data + 1) == 'n')
			continue;
		if (*(data + 1) != 't')
		{
			fprintf(stderr, "In mobility.txt, invalid data at line no %d\n", lineno);
			continue;
		}

		if (sscanf(data, "$time %lf \"$node_(%d) %lf %lf %lf\"",
				&time, &d, &x, &y, &z) == 5)
			{
		d = fn_NetSim_GetDeviceIdByConfigId(d + 1);
				if (d > 0 && DEVICE_MOBILITY(d)->nMobilityType == MobilityModel_FILEBASEDMOBILITY)
					add_mobility_info(d, time * SECOND, x, y, z);
				else
					fprintf(stderr, "In mobility.txt, either the device id is invalid or file based mobility not enabled, at line no %d\n", lineno);

			}
			else
			{
				fprintf(stderr, "In mobility.txt, invalid data at line no %d\n", lineno);
				continue;
			}
		}
		else
		{
			if (sscanf(data, "%lf,%d,%lf,%lf,%lf\"",
				&time, &d, &x, &y, &z) == 5)
			{
				d = fn_NetSim_GetDeviceIdByConfigId(d);
				if (d > 0 && DEVICE_MOBILITY(d)->nMobilityType == MobilityModel_FILEBASEDMOBILITY)
					add_mobility_info(d, time * SECOND, x, y, z);
				else
					fprintf(stderr, "In mobility.csv, either the device id is invalid or file based mobility not enabled, at line no %d\n", lineno);

			}
			else
			{
				fprintf(stderr, "In mobility.csv, invalid data at line no %d\n", lineno);
				continue;
			}
		}
	}
	fclose(fp);

	if (fileEmpty)
		fnSystemError("Empty mobility input file detected\n");

	add_mobility_event();
	return 0;
}
