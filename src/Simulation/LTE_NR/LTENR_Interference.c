#pragma region HEADER_FILES
#include "stdafx.h"
#include "LTENR_PHY.h"
#pragma endregion

#pragma region INTERFERENCE_PARAMETER

extern char* wynerFile;
typedef struct stru_GRADED_DISTANCE_BASED_WYNER_INTERFERENCE {
	NETSIM_ID CellId;
	NETSIM_ID PairId;//PairId
	double DDRth;
	int BinCount;//BinCount
	double* Relative_Intereference_dB;
	struct stru_GRADED_DISTANCE_BASED_WYNER_INTERFERENCE* next;
}GRADED_DISTANCE_BASED_WYNER_INTERFERENCE, * ptrGRADED_DISTANCE_BASED_WYNER_INTERFERENCE;
ptrGRADED_DISTANCE_BASED_WYNER_INTERFERENCE GradedDistanceBasedWynerInterferenceheadDownlink;
ptrGRADED_DISTANCE_BASED_WYNER_INTERFERENCE GradedDistanceBasedWynerInterferenceheadUplink;
#pragma endregion

//Interference Calc
static double fn_GetEdgeInterference(NETSIM_ID gNBiID, NETSIM_ID gNBjID, double dUEgnbi, double dUEgnbj,bool isDownlink) {
	ptrGRADED_DISTANCE_BASED_WYNER_INTERFERENCE temp = NULL;
	double interference_db = -NEGATIVE_DBM;
	if(isDownlink)
		temp = GradedDistanceBasedWynerInterferenceheadDownlink;
	if(!isDownlink)
		temp = GradedDistanceBasedWynerInterferenceheadUplink;

	if (!temp) return interference_db;
	else {
		while (temp) {
			if ((temp->CellId == gNBiID && temp->PairId == gNBjID) ||
				(temp->CellId == gNBjID && temp->PairId == gNBiID)) {
				double ratio = (dUEgnbj - dUEgnbi) / dUEgnbj;
				if (ratio < 0 && ratio>(-temp->DDRth)) return temp->Relative_Intereference_dB[0];
				if (ratio >= 0 && ratio <= temp->DDRth) {
					double Band = temp->DDRth / temp->BinCount;
					int i = 1;
					while (i <= temp->BinCount+1) {
						double bi = Band * ((double)i);
						if (ratio < bi) {
							return(temp->Relative_Intereference_dB[i]);
						}
						i++;
					}
				}
			}
			temp = temp->next;
		}
	}
	return interference_db;
}

//Graded Distance Based Wyner Model
static void LTENR_fn_Graded_Distance_Based_Wyner_Model_Interference(ptrLTENR_PROPAGATIONINFO info, double rxPower_dbm,
	double* Interference,int CA_ID, bool isDownlink) {

	double Interference_linear = 0;
	double In_db = INFINITY;//Relative Interference
	ptrLTENR_PROTODATA data = LTENR_PROTODATA_GET(info->ueId, info->ueIf);
	ptrLTENR_UEPHY uePhy = LTENR_UEPHY_GET(info->ueId, info->ueIf);
	for (NETSIM_ID r = 0; r < NETWORK->nDeviceCount; r++)
	{
		if (info->ueId == r + 1)
			continue;

		for (NETSIM_ID rin = 0; rin < DEVICE(r + 1)->nNumOfInterface; rin++)
		{
			if (!isLTE_NRInterface(r + 1, rin + 1))
				continue;
			
			ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(r + 1, rin + 1);
			ptrLTENR_GNBPHY phy = NULL;
			if (pd->deviceType == LTENR_DEVICETYPE_GNB) {
				phy = LTENR_GNBPHY_GET(r + 1, rin + 1);
				ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo = LTENR_ASSOCIATEDUEPHYINFO_GET(phy->gnbId, phy->gnbIf, info->ueId, info->ueIf);
				ptrLTENR_PROPAGATIONINFO pinfo = assocInfo->propagationInfo[CA_ID];
				
				if (pinfo && info->gnbId != r + 1 && pinfo->centralFrequency_MHz == info->centralFrequency_MHz) {
					In_db = fn_GetEdgeInterference(info->gnbId, r + 1, info->dist2D, pinfo->dist2D, isDownlink);
					Interference_linear += DBM_TO_MW((rxPower_dbm - In_db));
					//We convert relative interference In into absolute interference(dB)
					//We than convert dB scale to Linear scale.
				}
			}
		}
	}
	*Interference = MW_TO_DBM(Interference_linear);
	return;
}

//ConfigInit
void LTENR_fn_InterferenceInit() {
	FILE* fp;
	char* temp;
	char data[BUFSIZ], input[BUFSIZ];
	GradedDistanceBasedWynerInterferenceheadDownlink = (ptrGRADED_DISTANCE_BASED_WYNER_INTERFERENCE)malloc(sizeof(GRADED_DISTANCE_BASED_WYNER_INTERFERENCE));
	ptrGRADED_DISTANCE_BASED_WYNER_INTERFERENCE t = GradedDistanceBasedWynerInterferenceheadDownlink;
	int line = 0;
	sprintf(input, "%s%s%s", pszIOPath,pathSeperator, wynerFile);

	fp = fopen(input, "r");
	if (fp == NULL)
	{
		return;
	}
	else
	{
		
		while (fgets(data, BUFSIZ, fp))
		{
			line++;
			temp = data;
			if (*temp == '\n')
				continue;
			if (*temp == '#' || *temp == 0)
				continue;
			bool dev1 = false, dev2 = false;
			double in;
			char* newline = strchr(temp, '\n'); if (newline) *newline = 0;
			char* token = strtok(temp, ",");
			

			if (!t) break;
			if (token != NULL) 
			{
				t->CellId = fn_NetSim_Stack_GetDeviceId_asName(token);
				if (!t->CellId)
				{
					fprintf(stderr, "Invalid Input detected in line %d.\n\"%s\" is not a valid Device Name. Please edit the file and re-run simulation. \n"
						"Press any key to skip this line and continue simulation.\n", line, token);
					_getch();
				}
				else
				{
					for (NETSIM_ID In = 1; In <= DEVICE(t->CellId)->nNumOfInterface; In++)
					{
						if (!isLTENR_RANInterface(t->CellId, In)) continue;
						if (isGNB(t->CellId, In))
						{
							dev1 = true;
							break;
						}
					}
					if (!dev1)
					{
						fprintf(stderr, "Invalid Input detected in line %d.\n\"%s\" is not a valid gNB Device. Please edit the file and re-run simulation. \n"
							"Press any key to skip this line and continue simulation.\n", line, token);
						_getch();
					}
					else
					{
						if (token == NULL)
							continue;
						else
						{
							token = strtok(NULL, ",");
							t->PairId = fn_NetSim_Stack_GetDeviceId_asName(token);

							if (!t->PairId)
							{
								fprintf(stderr, "Invalid Input detected in line %d.\n\"%s\" is not a valid Device Name. Please edit the file and re-run simulation. \n"
									"Press any key to skip this line and continue simulation.\n", line, token);
								_getch();
							}
							else
							{
								for (NETSIM_ID In = 1; In <= DEVICE(t->PairId)->nNumOfInterface; In++)
								{
									if (!isLTENR_RANInterface(t->PairId, In)) continue;
									if (isGNB(t->PairId, In))
									{
										dev2 = true;
										break;
									}
								}
								if (!dev2)
								{
									fprintf(stderr, "Invalid Input detected in line %d.\n\"%s\" is not a valid LTE NR Device. Please edit the file and re-run simulation. \n"
										"Press any key to skip this line and continue simulation.\n", line, token);
									_getch();
								}
							}
						}
					}
				}
			}

			token = strtok(NULL, ",");
			if (token != NULL) {
				t->DDRth = strtod(token, NULL);
			}

			token = strtok(NULL, ",");
			int bincount = 4, k = 1;
			if (token != NULL) {
				t->BinCount = atoi(token);
				bincount = t->BinCount;
			}

			t->Relative_Intereference_dB = calloc((bincount + 2), sizeof(t->Relative_Intereference_dB));//Relative Interference 

			token = strtok(NULL, ",");
			if (token != NULL) {
				in = strtod(token, NULL);
				if (in >= -3 && in <= 0) t->Relative_Intereference_dB[0] = in;
				else
					printf("\nIncorrect value of Relative Interference"
						"has been found in line: %d, for Relative Interference -1 Enter value in range of 0 to 10", line);
			}
			while (k < bincount + 2) {

				token = strtok(NULL, ",");
				if (token != NULL) {
					in = strtod(token, NULL);
					if (t->Relative_Intereference_dB[k - 1] <= in && 0 <= in)
						t->Relative_Intereference_dB[k] = in;
					else
						printf("\nIncorrect value of Relative Interference has been found in line: %d, for In %d", line, k - 1);
				}
				k++;
			}
			t->next = (ptrGRADED_DISTANCE_BASED_WYNER_INTERFERENCE)malloc(1*sizeof(GRADED_DISTANCE_BASED_WYNER_INTERFERENCE));
			t = t->next;

		}

	}
	if (fp) fclose(fp);
}

//Geometric Edge Interference
void LTENR_fn_ExactGeometricModelInterference(ptrLTENR_PROPAGATIONINFO info, double rxPower_dbm,
	double* Interference,int layerId, int CA_ID, bool isDownlink) {

	double Interference_linear = 0;

	ptrLTENR_PROTODATA data = LTENR_PROTODATA_GET(info->ueId, info->ueIf);
	if(isDownlink)
	for (NETSIM_ID r = 0; r < NETWORK->nDeviceCount; r++)
	{
		if (info->ueId == r + 1)
			continue;

		for (NETSIM_ID rin = 0; rin < DEVICE(r + 1)->nNumOfInterface; rin++)
		{
			if (!isLTE_NRInterface(r + 1, rin + 1))
				continue;

			if (data->isDCEnable) {
				if (data->SecCellType == MMWAVE_CELL_TYPE) {
					if (!fn_NetSim_isDeviceLTENR(r + 1, rin + 1))
						continue;
				}
				else {
					if (fn_NetSim_isDeviceLTENR(r + 1, rin + 1))
						continue;
				}
			}
			ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(r + 1, rin + 1);
			ptrLTENR_GNBPHY phy = NULL;
			if (pd->deviceType == LTENR_DEVICETYPE_GNB) {
				phy = LTENR_GNBPHY_GET(r + 1, rin + 1);
				ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo = LTENR_ASSOCIATEDUEPHYINFO_GET(phy->gnbId, phy->gnbIf, info->ueId, info->ueIf);
				ptrLTENR_PROPAGATIONINFO pinfo = assocInfo->propagationInfo[CA_ID];
				if (pinfo && info->gnbId != r + 1 && pinfo->centralFrequency_MHz == info->centralFrequency_MHz) {
					if (isDownlink) Interference_linear += DBM_TO_MW(pinfo->downlink.rxPower_dbm[layerId]);
					else Interference_linear += DBM_TO_MW(pinfo->uplink.rxPower_dbm[layerId]);
				}
			}
		}
	}

	*Interference = MW_TO_DBM(Interference_linear);
	return;
}


void LTENR_Calculate_Interference(ptrLTENR_PROPAGATIONINFO info, double rxPower_dbm,
	double* Interference,int layerId, int CA_ID, bool isDownlink) {
	ptrLTENR_GNBPHY phy = LTENR_GNBPHY_GET(info->gnbId, info->gnbIf);

	if (phy->propagationConfig->DownlinkInterferenceModel == LTENR_EXACT_GEOMETRIC_MODEL_INTERFERENCE)
		LTENR_fn_ExactGeometricModelInterference(info, rxPower_dbm, Interference, layerId, CA_ID, isDownlink);
	
	else if (phy->propagationConfig->DownlinkInterferenceModel == LTENR_GRADED_DISTANCE_BASED_WYNER_MODEL_INTERFERENCE)
		LTENR_fn_Graded_Distance_Based_Wyner_Model_Interference(info, rxPower_dbm, Interference, CA_ID, isDownlink);//Cell Edge Interference
	else
		*Interference = NEGATIVE_DBM;
	
}

void free_interference() {
	if (GradedDistanceBasedWynerInterferenceheadDownlink) {
		while (GradedDistanceBasedWynerInterferenceheadDownlink!=NULL) {
			ptrGRADED_DISTANCE_BASED_WYNER_INTERFERENCE temp = GradedDistanceBasedWynerInterferenceheadDownlink;
			GradedDistanceBasedWynerInterferenceheadDownlink = GradedDistanceBasedWynerInterferenceheadDownlink->next;
			free(temp->Relative_Intereference_dB);
			temp->Relative_Intereference_dB = NULL;
			free(temp);
			temp = NULL;
		}
		free(wynerFile);
	}
	return;
}
