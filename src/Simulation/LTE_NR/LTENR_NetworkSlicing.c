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
#include "main.h"
#include "LTE_NR.h"
#include "NetSim_utility.h"
#include "LTENR_NetworkSlicing.h"
#pragma endregion

#pragma region BUILDER
char* sliceToString(_In_ ptrNetworkSliceInfo info, _Inout_ char* buf)
{
    sprintf(buf, "SliceInfo{ ""mSliceServiceType=%s"
        ", mSliceDifferentiator=%d }",
        strSLICE_SERVICE_TYPE[info->sliceServiceType], info->sliceDifferentiator);
    return buf;
}

void setSliceServiceType(_In_ ptrNetworkSliceInfo sliceInfo, SLICE_SERVICE_TYPE type) 
{
    sliceInfo->sliceServiceType = type;
}

void setSliceDifferentiator(_In_ ptrNetworkSliceInfo sliceInfo, _In_range_(MIN_SLICE_DIFFERENTIATOR,MAX_SLICE_DIFFERENTIATOR) int sliceDifferentiator)
{
    if (sliceDifferentiator < MIN_SLICE_DIFFERENTIATOR
        || sliceDifferentiator > MAX_SLICE_DIFFERENTIATOR)
    {
        fnNetSimError("The slice differentiator value is out of range");
        return;
    }
    sliceInfo->sliceDifferentiator = sliceDifferentiator;
}

_Check_return_ _Ret_notnull_ ptrNetworkSliceInfo sliceBuilder()
{
    ptrNetworkSliceInfo info = calloc(1,sizeof * info);
    info->sliceDifferentiator = SLICE_DIFFERENTIATOR_NO_SLICE;
    info->sliceServiceType = SLICE_SERVICE_TYPE_NONE;
    return info;
}

int findIndex(const char* arr[], int size, const char* target) {
	for (int i = 0; i < size; i++) {
		if (strcmp(target, arr[i]) == 0) {
			return i;
		}
	}
	return -1; // Indicates that the string was not found in the array
}

void LTENR_ConfigureNETWORK_SLICINGConfig() //auto assign slice differentiator
{
	FILE* fp;
	char* temp;
	char data[BUFSIZ], input[BUFSIZ];
	int line = 0;
	int slCount = 0;
	int slIndex = 0;
	int *sdCount;
	NETSIM_ID d, slid;
	
	sdCount = calloc(SLICE_SERVICE_TYPE_UNKNOWN, sizeof * sdCount);
	sprintf(input, "%s%s%s", pszIOPath, pathSeperator, "ConfigSupport/networkslicing.csv"/*nws->slicingFileName*/);

	fp = fopen(input, "r");
	if (fp == NULL)
	{
		fnSystemError("Unable to open %s file", input);
		perror(input);
		return;
	}
	else
	{
		while (fgets(data, BUFSIZ, fp) != NULL)
		{
			line++;
			temp = data;
			temp = lskip(temp);

			if (*temp == '\n' || *temp == '#' || *temp == 0 || *temp == ',')
				continue;

			_strupr(temp);

			size_t len = strlen(temp);
			while (len > 0 && ((temp[len - 1] == '\n') || (temp[len - 1] == ','))) temp[--len] = '\0';
			if (*temp == 'R')
			{
				char* f = strtok(temp, ",");
				if (!_stricmp(f, "resource_sharing_technique"))
				{
					char* l = strtok(NULL, ",\n");
					int size = sizeof(strLTENR_RESOURCE_SHARING_TECHNIQUE) / sizeof(strLTENR_RESOURCE_SHARING_TECHNIQUE[0]);
					int i = findIndex(strLTENR_RESOURCE_SHARING_TECHNIQUE, size, l);
					if (i >= 0)
						nws->rsrcSharingTechnique = i;
					else
					{
						fprintf(stderr, "Invalid resource sharing technique found in line %d.\n\"%s\" is not a valid Resource Sharing Technique. Please edit the file and re-run simulation. \n"
							"Press any key to skip this line and continue simulation.\n", line, temp);
						_getch();
					}
				}
				else
				{
					goto INVALIDINPUT;
				}
			}
			else if (*temp == 'N')
			{
				char* f = strtok(temp, ",");
				if (!_stricmp(f, "number_of_slices"))
				{
					char* l = strtok(NULL, ",\n");
					nws->sliceCount = atoi(l);
				}
				else
				{
					goto INVALIDINPUT;
				}
			}
			else if (*temp == 'S')
			{
				char* f = strtok(temp, ",");
				if (!_stricmp(f, "slice_id"))
				{
					slCount++;
					char* l = strtok(NULL, ",\n");
					nws->Info[slCount - 1].sliceId = atoi(l);
				}
				else if (!_stricmp(f, "slice_type"))
				{
					char* l = strtok(NULL, ",\n");
					int size = sizeof(strSLICE_SERVICE_TYPE) / sizeof(strSLICE_SERVICE_TYPE[0]);
					slIndex = findIndex(strSLICE_SERVICE_TYPE, size, l);
					if (slIndex >= 0)
					{
						//nws->Info[slCount - 1].sliceServiceType = slIndex;
						setSliceServiceType(&nws->Info[slCount - 1], slIndex);
						sdCount[slIndex]++;
					}
					else
					{
						fprintf(stderr, "Invalid slice type found in line %d.\n\"%s\" is not a valid slice type. Please edit the file and re-run simulation. \n"
							"Press any key to skip this line and continue simulation.\n", line, temp);
						_getch();
					}
				}
				else if (!_stricmp(f, "slice_differentiator"))
				{
					char* l = strtok(NULL, ",\n");
					//nws->Info[slCount - 1].sliceDifferentiator = atoi(l);
					//setSliceDifferentiator(&nws->Info[slCount - 1], atoi(l)); //manual from file
					setSliceDifferentiator(&nws->Info[slCount - 1], sdCount[slIndex]); //auto assignment
				}
				else
				{
					goto INVALIDINPUT;
				}
			}
			else if (*temp == 'P')
			{
				char* f = strtok(temp, ",");
				if (!_stricmp(f, "percentage_of_resources_allocated"))
				{
					char* l = strtok(NULL, ",\n");
					nws->Info[slCount - 1].downlinkBandwidth.resourceAllocationPercentage = atof(l);
					nws->Info[slCount - 1].uplinkBandwidth.resourceAllocationPercentage = atof(l);
				}
				else
				{
					goto INVALIDINPUT;
				}
			}
			else if (*temp == 'M')
			{
				char* f = strtok(temp, ",");
				if (!_stricmp(f, "min_aggr_throughput"))
				{
					char* l = strtok(NULL, ",\n");
					nws->Info[slCount - 1].downlinkBandwidth.minGuaranteedBitRate_mbps = atof(l);
					nws->Info[slCount - 1].uplinkBandwidth.minGuaranteedBitRate_mbps = atof(l);
				}
				else
				{
					goto INVALIDINPUT;
				}
			}
			else if (*temp == 'A')
			{
				char* f = strtok(temp, ",");
				if (!_stricmp(f, "aggregate_rate_guarantee_dl"))
				{
					char* l = strtok(NULL, ",\n");
					nws->Info[slCount - 1].downlinkBandwidth.aggregateRateGuarantee_mbps = atof(l);
				}
				else if (!_stricmp(f, "aggregate_rate_guarantee_ul"))
				{
					char* l = strtok(NULL, ",\n");
					nws->Info[slCount - 1].uplinkBandwidth.aggregateRateGuarantee_mbps = atof(l);
				}
				else
				{
					goto INVALIDINPUT;
				}
			}
			else if (sscanf(data, "%d,%d\"",
				&d, &slid) == 2)
			{
				NETSIM_ID ueId = fn_NetSim_GetDeviceIdByConfigId(d);
				if (ueId > 0)
					nws->ueSliceId[ueId] = slid;
				else
					fprintf(stderr, "In networkslicing.csv, the device id is invalid, at line no %d\n", line);

			}
			else
			{
			INVALIDINPUT:
				fprintf(stderr, "Invalid input found in line %d.\n\"%s\"\n"
					"Press any key to skip this line and continue simulation.\n", line, temp);
				_getch();
			}
		}
	}
	if (fp)
		fclose(fp);
}
#pragma endregion
