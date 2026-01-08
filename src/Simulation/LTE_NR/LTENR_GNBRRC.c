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
* Author:    Kumar Gaurav	                                                        *
*										                                            *
* ----------------------------------------------------------------------------------*/

#pragma region HEADER_FILES
#include "stdafx.h"
#include "LTENR_GNBRRC.h"
#include "LTENR_PHY.h"
#include "LTENR_RLC.h"
#include "LTENR_PDCP.h"
#include "LTENR_MAC.h"
#include "LTENR_NAS.h"
#include "LTENR_EPSBearer.h"
#include "LTENR_Core.h"
#include "LTENR_GNB_CORE.h"
#include "LTENR_NSA.h"
#include "LTENR_SN_HandOver.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma endregion

#pragma region LOG
static FILE* fpHandoverLog = NULL;

// --- Python listener socket helpers ---
static SOCKET pySock = INVALID_SOCKET;
static bool py_wsa_started = false;

// Server/listener support so external Python client can connect to this simulator
static SOCKET pyListenSock = INVALID_SOCKET;
static HANDLE pyAcceptThread = NULL;
static volatile BOOL pyAcceptThreadRunning = FALSE;

static DWORD WINAPI py_accept_thread_func(LPVOID lpParam)
{
    char* port = (char*)lpParam; // allocated by caller
    struct addrinfo hints, *result = NULL, *rp = NULL;
    int iResult;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4 only for simplicity
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    iResult = getaddrinfo(NULL, port, &hints, &result);
    if (iResult != 0) {
        free(port);
        return 1;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        pyListenSock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (pyListenSock == INVALID_SOCKET)
            continue;
        BOOL opt = TRUE;
        setsockopt(pyListenSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
        if (bind(pyListenSock, rp->ai_addr, (int)rp->ai_addrlen) == 0)
            break;
        closesocket(pyListenSock);
        pyListenSock = INVALID_SOCKET;
    }
    freeaddrinfo(result);

    if (pyListenSock == INVALID_SOCKET) {
        free(port);
        return 1;
    }

    if (listen(pyListenSock, 1) == SOCKET_ERROR) {
        closesocket(pyListenSock);
        pyListenSock = INVALID_SOCKET;
        free(port);
        return 1;
    }

    pyAcceptThreadRunning = TRUE;

    while (pyAcceptThreadRunning) {
        struct sockaddr_in clientAddr;
        int addrlen = sizeof(clientAddr);
        SOCKET client = accept(pyListenSock, (struct sockaddr*)&clientAddr, &addrlen);
        if (client == INVALID_SOCKET) {
            // sleep briefly to allow shutdown
            Sleep(100);
            continue;
        }
        // close existing connection if any
        if (pySock != INVALID_SOCKET) {
            closesocket(pySock);
            pySock = INVALID_SOCKET;
        }
        pySock = client;
        // set send timeout
        int timeout = 100;
        setsockopt(pySock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));
        // keep the accepted socket until closed by remote; then accept next
        // we'll wait here until socket is closed
        char buf[1];
        // Blocking recv of zero bytes is not reliable; instead poll for closure
        while (pyAcceptThreadRunning) {
            int res = recv(pySock, buf, 0, 0);
            if (res == SOCKET_ERROR) {
                int err = WSAGetLastError();
                // if error indicates connection closed, break
                if (err == WSAECONNRESET || err == WSAENOTCONN || err == WSAESHUTDOWN) {
                    break;
                }
                // otherwise sleep and continue
                Sleep(200);
                continue;
            }
            // Sleep to avoid tight loop
            Sleep(200);
        }
        // close accepted client if still open
        if (pySock != INVALID_SOCKET) {
            closesocket(pySock);
            pySock = INVALID_SOCKET;
        }
    }

    if (pyListenSock != INVALID_SOCKET) {
        closesocket(pyListenSock);
        pyListenSock = INVALID_SOCKET;
    }
    free(port);
    return 0;
}

bool py_start_listener(const char* port, int send_timeout_ms)
{
    if (!py_wsa_started) {
        WORD wVersion = MAKEWORD(2, 2);
        WSADATA wsaData;
        if (WSAStartup(wVersion, &wsaData) != 0) {
            return false;
        }
        py_wsa_started = true;
    }

    if (pyAcceptThread != NULL)
        return true; // already running

    // duplicate port string for thread; thread will free it
    size_t plen = strlen(port) + 1;
    char* portdup = (char*)malloc(plen);
    if (!portdup)
        return false;
    strcpy_s(portdup, plen, port);

    pyAcceptThread = CreateThread(NULL, 0, py_accept_thread_func, portdup, 0, NULL);
    if (!pyAcceptThread) {
        free(portdup);
        return false;
    }
    return true;
}

void py_stop_listener()
{
    pyAcceptThreadRunning = FALSE;
    if (pyListenSock != INVALID_SOCKET) {
        closesocket(pyListenSock);
        pyListenSock = INVALID_SOCKET;
    }
    if (pyAcceptThread) {
        WaitForSingleObject(pyAcceptThread, 200);
        CloseHandle(pyAcceptThread);
        pyAcceptThread = NULL;
    }
}

void py_send_handover_event(NETSIM_ID ueID, double servingsnr, double targetsnr, NETSIM_ID servinggnb, NETSIM_ID targetgnb, const char* remark)
{
	if (pySock == INVALID_SOCKET)
		return;

	char buf[512];
	// Format: HANDOVER,UE,<ueID>,SERVING,<servingID>,TARGET,<targetID>,TIME,<time_ms>,SERVING_SINR,<serving>,TARGET_SINR,<target>,REMARK,<remark>\n
	double time_ms = ldEventTime / MILLISECOND;
	int len = snprintf(buf, sizeof(buf), "HANDOVER,UE,%s,SERVING,%s,TARGET,%s,TIME,%.2f,SERVING_SINR,%.2f,TARGET_SINR,%.2f,REMARK,%s\n",
		DEVICE_NAME(ueID), DEVICE_NAME(servinggnb), DEVICE_NAME(targetgnb), time_ms, servingsnr, targetsnr, remark ? remark : "");
	if (len <= 0) return;

	int sent = send(pySock, buf, len, 0);
	// ignore errors to avoid disturbing simulator
	(void)sent;
}
#pragma endregion

#pragma region MIB_GENERATION
static void start_mib_generation(NETSIM_ID gnbId,
	NETSIM_ID gnbInterface)
{

	ptrLTENR_GNBRRC rrc = LTENR_GNBRRC_GET(gnbId, gnbInterface);
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = rrc->mibInterval;
	pevent.nDeviceId = gnbId;
	pevent.nDeviceType = DEVICE_TYPE(gnbId);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = gnbInterface;
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(gnbId, gnbInterface);
	LTENR_SET_SUBEVENT(pd, &pevent, LTENR_SUBEVNET_GENERATE_MIB);
	fnpAddEvent(&pevent);
}

void fn_NetSim_LTENR_GNBRRC_GenerateMIB()
{
	ptrLTENR_GNBRRC rrc = LTENR_GNBRRC_GET(pstruEventDetails->nDeviceId, pstruEventDetails->nInterfaceId);
	pstruEventDetails->dEventTime += rrc->mibInterval;
	fnpAddEvent(pstruEventDetails);
	pstruEventDetails->dEventTime -= rrc->mibInterval;

	pstruEventDetails->pPacket = LTENR_PACKET_CREATE(pstruEventDetails->nDeviceId, 0, pstruEventDetails->dEventTime, LTENR_MSG_RRC_MIB);
	ptrLTENR_MSG msg = pstruEventDetails->pPacket->pstruMacData->Packet_MACProtocol;
	msg->logicalChannel = LTENR_LOGICALCHANNEL_BCCH;
	msg->physicalChannel = LTENR_PHYSICALCHANNEL_PBCH;
	msg->transportChannel = LTENR_TRANSPORTCHANNEL_BCH;
	msg->rlcMode = LTENR_RLCMODE_TM;
	msg->srb = LTENR_NA;


	if (!rrc->MIB) {
		rrc->MIB = calloc(1, sizeof * rrc->MIB);
	}

	rrc->MIB->dmrsTypeAPosition = dmrsTypeAPosition_pos2;
	rrc->MIB->cellBarred = cellBarred_notBarred;

	ptrLTENR_RRC_Hdr hdr = calloc(1, sizeof * hdr);
	hdr->msg = rrc->MIB;
	hdr->msgType = LTENR_MSG_RRC_MIB;
	hdr->SenderID = pstruEventDetails->nDeviceId;
	hdr->SenderIF = pstruEventDetails->nInterfaceId;
	fn_NetSIm_LTENR_RRC_ADD_HDR_INTO_PACKET(pstruEventDetails->pPacket, hdr, mibHdrID, LTENR_MSG_RRC_MIB);
	rrc->MIB = NULL;
	LTENR_CallRLCOut();
}

void fn_NetSim_LTENR_UERRC_MIB_RECV(NETSIM_ID ueID, NETSIM_ID ueIF, NETSIM_ID gnbID, NETSIM_ID gnbIF, ptrLTENR_RRC_MIBMSG MIB) {
	ptrLTENR_UERRC rrc = LTENR_UERRC_GET(ueID, ueIF);
	if (rrc->MIB)
	{
		if (rrc->ueRRCState == UERRC_IDLE || rrc->ueRRCState == UERRC_INACTIVE ||
			(rrc->ueRRCState == UERRC_CONNECTED && rrc->isT311Running))
		{
			if (rrc->MIB->cellBarred == cellBarred_barred)
			{
				if (rrc->MIB->intraFreqReselection == intraFreqReselection_notAllowed) {
					//Don't save this cell's MIB MSG
				}
				else {
					//Don't save this cell's MIB MSG
				}
			}
			else
			{
				//that condition mean UE is able to acquire the MIB
				free(rrc->MIB);
				rrc->MIB = LTENR_RRC_MIBMSG_COPY(MIB);
			}
		}
	}
	else
	{
		free(rrc->MIB);
		rrc->MIB = LTENR_RRC_MIBMSG_COPY(MIB);;
	}

	if (rrc->ueRRCState == UERRC_IDLE || rrc->ueRRCState == UERRC_INACTIVE) {
		ptr_LTENR_SINREPORT temp = SINR_REPORT_ALLOC();
		temp->cellID = gnbID;
		temp->sinr = LTENR_PHY_RRC_RSRP_SINR(gnbID, gnbIF, ueID, ueIF);
		ptr_LTENR_SINREPORT head = rrc->SINRReport;
		bool flag = false;
		while (head) {
			if (head->cellID == temp->cellID) {
				head->sinr = temp->sinr;
				flag = true;
				break;
			}
			head = LIST_NEXT(head);
		}
		if (!flag)
			LIST_ADD_LAST((void**)&rrc->SINRReport, temp);
	}
}

#pragma endregion

#pragma region CellSelection
bool fn_NetSim_LTENR_EvaluateCellForSelection(ptrLTENR_UERRC rrc) {
	double targetSINR = MININT;
	NETSIM_ID target = 0;
	ptr_LTENR_SINREPORT temp = rrc->SINRReport;
	ptrLTENR_PROTODATA rlte = LTENR_PROTODATA_GET(rrc->d, rrc->in);
	ptrLTENR_ASSOCIATIONINFO rinfo = rlte->associationInfo;
	return rrc->SIB1->cellAccessRelatedInfo->plmnIdentityInfo->cellIdentity == rinfo->d;

	/*while (temp) {
		if (temp->sinr > targetSINR) {
			targetSINR = temp->sinr;
			target = temp->cellID;
		}
		temp = LIST_NEXT(temp);
	}
	return rrc->SIB1->cellAccessRelatedInfo->plmnIdentityInfo->cellIdentity == target;*/
}

int fn_NetSim_LTENR_FIND_BEST_SN_CELL_FOR_UE(NETSIM_ID d, NETSIM_ID in) {
	double Curr_SINR = INT_MIN, Best_SINR = INT_MIN;
	int BEST_SN_ID = 0;
	ptrLTENR_PROTODATA data = LTENR_PROTODATA_GET(d, in);
	for (NETSIM_ID r = 0; r < NETWORK->nDeviceCount; r++)
	{
		if (d == r + 1)
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
			switch (pd->deviceType)
			{
			case LTENR_DEVICETYPE_GNB:
				Curr_SINR = LTENR_PHY_RRC_RSRP_SINR(r + 1, rin + 1, d, in);
				if (Best_SINR < Curr_SINR) {
					Best_SINR = Curr_SINR;
					BEST_SN_ID = r + 1;
				}
				break;
			}
		}
	}
	return BEST_SN_ID;
}
#pragma endregion

#pragma region RRC_SETUP_COMPLETE
void fn_NetSim_LTENR_RRC_SETUP_COMPLETE(NETSIM_ID UeID, NETSIM_ID ueIF, ptrLTENR_RRC_RRCSetupComplete rrcSetupCompletemsg) {
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = pstruEventDetails->dEventTime;
	pevent.nDeviceId = UeID;
	pevent.nDeviceType = DEVICE_TYPE(UeID);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = ueIF;
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(UeID, ueIF);
	LTENR_SET_SUBEVENT(pd, &pevent, LTENR_SUBEVENT_GENERATE_RRC_SETUP_COMPLETE);
	ptrLTENR_UERRC rrc = LTENR_UERRC_GET(UeID, ueIF);
	pevent.pPacket = LTENR_PACKET_CREATE(pevent.nDeviceId, rrc->SelectedCellID, pevent.dEventTime, LTENR_MSG_RRC_SETUP_COMPLETE);
	ptrLTENR_MSG msg = pevent.pPacket->pstruMacData->Packet_MACProtocol;
	msg->logicalChannel = LTENR_LOGICALCHANNEL_CCCH;
	msg->physicalChannel = LTENR_PHYSICALCHANNEL_PDCCH;
	msg->transportChannel = LTENR_TRANSPORTCHANNEL_DL_SCH;
	msg->rlcMode = LTENR_RLCMODE_TM;
	msg->srb = LTENR_SRB1;
	ptrLTENR_RRC_Hdr hdr = calloc(1, sizeof * hdr);
	hdr->msg = rrcSetupCompletemsg;
	hdr->msgType = LTENR_MSG_RRC_SETUP_COMPLETE;
	hdr->SenderID = UeID;
	hdr->SenderIF = ueIF;
	fn_NetSIm_LTENR_RRC_ADD_HDR_INTO_PACKET(pevent.pPacket, hdr, rrcSetupCompleteID, LTENR_MSG_RRC_SETUP_COMPLETE);
	fnpAddEvent(&pevent);
}

void fn_NetSim_LTENR_RRC_GENERATE_RRC_SETUP_COMPLETE() {
	LTENR_CallPDCPOut();
}

void fn_NetSim_LTENR_RRC_RRCSETUPCOMPLETE_RECV(NETSIM_ID gnbID, NETSIM_ID gnbIF, NETSIM_ID ueID, NETSIM_ID ueIF) {
	//AMF Selection
	ptrLTENR_UERRC rrc = LTENR_UERRC_GET(ueID, ueIF);
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(ueID, ueIF);
	ptrLTENR_PROTODATA data = LTENR_PROTODATA_GET(gnbID, gnbIF);

	if (!fn_NetSim_LTENR_IS_S1_INTERFACE_EXISTS(gnbID)) {
		rrc->SelectedAMFID = LTENR_AMF_GetNewAMF(ueID, ueIF);
		rrc->SelectedAMFID = fn_NetSim_Stack_GetDeviceId_asName(data->connectedAMFName);
		rrc->SelectedAMFIF = fn_NetSim_LTENR_CORE_INTERFACE(rrc->SelectedAMFID, nGC_INTERFACE_N1_N2);
		rrc->SelectedSMFID = fn_NetSim_Stack_GetDeviceId_asName(data->connectedSMFName);
		rrc->SelectedSMFIF = fn_NetSim_LTENR_CORE_INTERFACE(rrc->SelectedSMFID, nGC_INTERFACE_N11);
		NETSIM_ID gnbP2PIF = fn_NetSim_LTENR_CORE_INTERFACE(gnbID, nGC_INTERFACE_N1_N2);
		LTENR_AMF_AddgNB(rrc->SelectedAMFID, rrc->SelectedAMFIF, rrc->SelectedCellID, gnbP2PIF);

		ptrLTENR_CORE_HDR hdr = calloc(1, sizeof * hdr);
		hdr->AMFID = rrc->SelectedAMFID;
		hdr->AMFIF = rrc->SelectedAMFIF;
		hdr->gNBID = rrc->SelectedCellID;
		hdr->gnbUuIF = rrc->SelectedCellIF;
		hdr->gNBIF = gnbP2PIF;
		hdr->UEID = ueID;
		hdr->UEIF = ueIF;
		hdr->UPFID = fn_NetSim_Stack_GetDeviceId_asName(data->connectedUPFName);
		hdr->UPFIF = fn_NetSim_LTENR_CORE_INTERFACE(hdr->UPFID, nGC_INTERFACE_N4);
		hdr->SMFID = LTENR_SMF_GetNewSMF(ueID, ueIF);
		hdr->SMFID = fn_NetSim_Stack_GetDeviceId_asName(data->connectedSMFName);
		hdr->SMFIF = fn_NetSim_LTENR_CORE_INTERFACE(hdr->SMFID, nGC_INTERFACE_N11);
		hdr->SMFN4IF = fn_NetSim_LTENR_CORE_INTERFACE(hdr->SMFID, nGC_INTERFACE_N4);
		//INTIAL UE MSG
		fn_NetSim_LTENR_GNB_CORE_INITIAL_UE_MSG(hdr);
	}

}

#pragma endregion

#pragma region UE_MEASUREMENT_REPORT
void fn_NetSim_LTENR_RRC_UE_MEASUREMENT_REPORT_REQUEST(NETSIM_ID gnbId, NETSIM_ID gnbInterface) {
	ptrLTENR_GNBRRC rrc = LTENR_GNBRRC_GET(gnbId, gnbInterface);
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = rrc->ueMeasReportInterval;
	pevent.nDeviceId = gnbId;
	pevent.nDeviceType = DEVICE_TYPE(gnbId);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = gnbInterface;
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(gnbId, gnbInterface);
	LTENR_SET_SUBEVENT(pd, &pevent, LTENR_SUBEVENT_GENERATE_RRC_UE_MEASUREMENT_REPORT_REQUEST);
	fnpAddEvent(&pevent);
}

void fn_NetSim_LTENR_RRC_UE_Measurement_Report_Request_Handle() {
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	ptrLTENR_GNBRRC rrc = LTENR_GNBRRC_GET(d, in);
	ptrLTENR_UERRC ueRRC = NULL;

	pstruEventDetails->dEventTime += rrc->ueMeasReportInterval;
	fnpAddEvent(pstruEventDetails);
	pstruEventDetails->dEventTime -= rrc->ueMeasReportInterval;

	for (NETSIM_ID r = 0; r < NETWORK->nDeviceCount; r++)
	{
		for (NETSIM_ID rin = 0; rin < DEVICE(r + 1)->nNumOfInterface; rin++)
		{
			if (!isLTE_NRInterface(r + 1, rin + 1))
				continue;

			ptrLTENR_PROTODATA data = LTENR_PROTODATA_GET(r + 1, rin + 1);
			switch (data->deviceType)
			{
			case LTENR_DEVICETYPE_UE:
				ueRRC = LTENR_UERRC_GET(r + 1, rin + 1);
				if (ueRRC->ueRRCState == UERRC_CONNECTED &&
					ueRRC->SelectedCellID == d && ueRRC->SelectedCellIF == in) {
					fn_NetSim_LTENR_RRC_UE_Measurement_Report(d, in, r + 1, rin + 1);
				}
				break;
			default:
				break;
			}
		}
	}
}

void fn_NetSim_LTENR_RRC_UE_Measurement_Report(NETSIM_ID gnbID, NETSIM_ID gnbIF, NETSIM_ID UEID, NETSIM_ID ueIF) {
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = pstruEventDetails->dEventTime;
	pevent.nDeviceId = UEID;
	pevent.nDeviceType = DEVICE_TYPE(UEID);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = ueIF;
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(UEID, ueIF);
	ptrLTENR_UERRC ueRRC = LTENR_UERRC_GET(UEID, ueIF);
	LTENR_SET_SUBEVENT(pd, &pevent, LTENR_SUBEVENT_GENERATE_RRC_UE_MEASUREMENT_REPORT);
	pevent.pPacket = LTENR_PACKET_CREATE(pevent.nDeviceId, gnbID, pevent.dEventTime, LTENR_MSG_RRC_UE_MEASUREMENT_REPORT);
	ptrLTENR_MSG msg = pevent.pPacket->pstruMacData->Packet_MACProtocol;
	msg->logicalChannel = LTENR_LOGICALCHANNEL_DCCH;
	msg->physicalChannel = LTENR_PHYSICALCHANNEL_PUCCH;
	msg->transportChannel = LTENR_TRANSPORTCHANNEL_UL_SCH;
	msg->rlcMode = LTENR_RLCMODE_AM;
	msg->srb = LTENR_SRB1;
	//todo
	msg->logicalChannel = LTENR_LOGICALCHANNEL_CCCH;
	msg->physicalChannel = LTENR_PHYSICALCHANNEL_PDCCH;
	msg->transportChannel = LTENR_TRANSPORTCHANNEL_DL_SCH;
	msg->rlcMode = LTENR_RLCMODE_TM;
	ueRRC->ueMeasurementID = fnpAddEvent(&pevent);
}
void fn_NetSIM_LTENR_RRC_GENERATE_UE_MEASUREMENT_REPORT() {
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	ptrLTENR_UERRC ueRRC = LTENR_UERRC_GET(d, in);
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(d, in);
	ptrLTENR_GNBRRC gnbRRC = LTENR_GNBRRC_GET(ueRRC->SelectedCellID, ueRRC->SelectedCellIF);

	ptrLTENR_RRC_UE_MEASUREMENT_REPORT report = NULL;
	ptrLTENR_RRC_UE_MEASUREMENT_REPORT temp = NULL;
	ptrLTENR_GNBPHY phy = NULL;
	for (NETSIM_ID r = 0; r < NETWORK->nDeviceCount; r++)
	{
		for (NETSIM_ID rin = 0; rin < DEVICE(r + 1)->nNumOfInterface; rin++)
		{
			if (!isLTE_NRInterface(r + 1, rin + 1))
				continue;

			ptrLTENR_PROTODATA data = LTENR_PROTODATA_GET(r + 1, rin + 1);
			switch (data->deviceType)
			{
			case LTENR_DEVICETYPE_GNB:
				temp = MEASUREMENT_REPORT_ALLOC();
				temp->ueID = d;
				temp->cellID = r + 1;
				temp->cellIF = rin + 1;
				temp->rs_type = RS_TYPE_SSB;
				temp->reportAmount = ReportAmount_r1;
				temp->reportInteval = gnbRRC->ueMeasReportInterval;
				phy = LTENR_GNBPHY_GET(r + 1, rin + 1);
				temp->sinr = LTENR_PHY_RRC_RSRP_SINR(r + 1, rin + 1, d, in);
				LIST_ADD_LAST((void**)&report, temp);
				break;
			default:
				break;
			}
		}
	}
	ptrLTENR_RRC_Hdr hdr = calloc(1, sizeof * hdr);
	hdr->msg = report;
	hdr->msgType = LTENR_MSG_RRC_UE_MEASUREMENT_REPORT;
	hdr->SenderID = d;
	hdr->SenderIF = in;
	fn_NetSIm_LTENR_RRC_ADD_HDR_INTO_PACKET(pstruEventDetails->pPacket, hdr, ueMEASID, LTENR_MSG_RRC_UE_MEASUREMENT_REPORT);
	LTENR_CallPDCPOut();
}

void fn_NetSim_LTENR_RRC_UE_MEASUREMENT_RECV(NETSIM_ID gnbID, NETSIM_ID gnbIF, NETSIM_ID ueID, NETSIM_ID ueIF, ptrLTENR_RRC_UE_MEASUREMENT_REPORT report) {
	double servingSINR = 0;
	double servingRSRP = 0;
	double servingRSRQ = 0;
	double targetSINR = INT_MIN;
	NETSIM_ID target = 0;
	double serveingSNSINR = 0;
	double serveringSNRSRP = 0;
	double serveringSNRSRQ = 0;
	double targetSNSINR = INT_MIN;
	NETSIM_ID targetSN = 0;
	ptrLTENR_RRC_UE_MEASUREMENT_REPORT report1 = report;
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(ueID, ueIF);
	ptrLTENR_UERRC rrc = LTENR_UERRC_GET(ueID, ueIF);
	ptrLTENR_GNBRRC gnbrrc = LTENR_GNBRRC_GET(gnbID, gnbIF);
	double TriggerTime = 0;
	bool TTTflag = true;
SET_TTT_TRIGGER:
	while (report1)
	{
		if (pd->isDCEnable) {
			if (pd->MasterCellType == MMWAVE_CELL_TYPE) {
				if (fn_NetSim_isDeviceLTENR(ueID, ueIF)) {
					if (!fn_NetSim_isDeviceLTENR(report1->cellID, report1->cellIF)) {
						report1 = LIST_NEXT(report1);
						continue;
					}
				}
				else {
					report1 = LIST_NEXT(report1);
					continue;
				}
			}
			else {
				if (!fn_NetSim_isDeviceLTENR(ueID, ueIF)) {
					if (fn_NetSim_isDeviceLTENR(report1->cellID, report1->cellIF)) {
						report1 = LIST_NEXT(report1);
						continue;
					}
				}
				else {
					report1 = LIST_NEXT(report1);
					continue;
				}
			}
		}
		if (!TTTflag && report1->cellID != gnbID)
		{
			TriggerTime = fn_NetSim_LTENR_Conditional_HO_TTT_Matrix_Find(gnbrrc->Conditional_HO_TTT_Matrix,
				ueID, report1->cellID);
			if (report1->sinr - servingSINR > gnbrrc->HandoverMargin)
			{
				if (TriggerTime == -1 && gnbrrc->TTT > 0)
				{
					fn_NetSim_LTENR_Conditional_HO_TTT_Matrix_Set(gnbrrc->Conditional_HO_TTT_Matrix,
						ueID, report1->cellID, ldEventTime);
					write_handover_log(ueID, servingSINR, report1->sinr, gnbID, report1->cellID, "Handover offset conditions met");
				}
				else
				{
					if ((ldEventTime - TriggerTime) >= (gnbrrc->TTT * MILLISECOND))
					{
						targetSINR = report1->sinr;
						//target = report1->cellID;
					}
				}
			}
			else
			{
				if (TriggerTime != -1)
				{
					fn_NetSim_LTENR_Conditional_HO_TTT_Matrix_Set(gnbrrc->Conditional_HO_TTT_Matrix,
						ueID, report1->cellID, -1);
					write_handover_log(ueID, servingSINR, report1->sinr, gnbID, report1->cellID, "Handover not initiated. TTT condition not met");
				}
			}

		}

		if (report1->cellID == gnbID && TTTflag)
		{
			servingSINR = report1->sinr;
			servingRSRP = report1->rsrp;
			servingRSRQ = report1->rsrq;
			break;
		}
		report1 = LIST_NEXT(report1);
	}
	if (TTTflag)
	{
		report1 = report;
		TTTflag = false;
		goto SET_TTT_TRIGGER;
	}

	if (pd->isDCEnable && rrc->SNID == 0) {
		fn_NetSim_LTENR_SEC_CELL_SelectionAddition(ueID, ueIF);
	}

	if (pd->isDCEnable && rrc->SNID != 0) {
		fn_NetSim_DC_HO_SN_PROCESSIONG(ueID, ueIF, report);
	}

	if (servingSINR + gnbrrc->HandoverMargin < targetSINR && target > 0 && gnbID != target) {
		ptrLTENR_HANDOVER_Hdr msg = calloc(1, sizeof * msg);
		msg->HandoverStartTime = pstruEventDetails->dEventTime;
		write_handover_log(ueID, servingSINR, targetSINR, gnbID, target, "Handover Initiated");
		//call nas for handover request
		msg->serveringCellID = gnbID;
		msg->serveringCellIF = gnbIF;
		ptrLTENR_GNBMAC mac = LTENR_GNBMAC_GET(gnbID, gnbIF);
		msg->targetCellID = target;
		msg->targetCellIF = fn_NetSim_Get_LTENR_INTERFACE_ID_FROM_DEVICE_ID(target);
		msg->UEID = ueID;
		msg->UEIF = ueIF;
		if (!fn_NetSim_isDeviceLTENR(gnbID, gnbIF)) {
			msg->AMFID = mac->epcId;
			msg->AMFIF = mac->epcIf;
			if (fn_NetSim_LTENR_CORE_INTERFACE(gnbID, nGC_INTERFACE_XN)) {
				ptrLTENR_PROTODATA data = LTENR_PROTODATA_GET(ueID, ueIF);
				if (data->isDCEnable) {
					if (data->NSA_MODE == NSA_MODE_OPTION7 || data->NSA_MODE == NSA_MODE_OPTION7A
						|| data->NSA_MODE == NSA_MODE_OPTION7X) {
						msg->AMFID = rrc->SelectedAMFID;
						msg->AMFIF = rrc->SelectedAMFIF;
						ptrLTENR_PROTODATA trrc = LTENR_PROTODATA_GET(msg->targetCellID, msg->targetCellIF);
						msg->targetAMFID = fn_NetSim_Stack_GetDeviceId_asName(trrc->connectedAMFName);
						msg->targetAMFIF = fn_NetSim_LTENR_CORE_INTERFACE(msg->targetAMFID, nGC_INTERFACE_N1_N2);
						msg->associateSMFID = fn_NetSim_Stack_GetDeviceId_asName(trrc->connectedSMFName);
					}
				}
				LTENR_HANDOVER_REQUEST(msg);
			}
			else {
				fn_NetSim_LTENR_NAS_HANDOVER_REQUEST(msg);
			}
		}
		else {
			msg->AMFID = rrc->SelectedAMFID;
		 msg->AMFIF = rrc->SelectedAMFIF;
			ptrLTENR_PROTODATA trrc = LTENR_PROTODATA_GET(msg->targetCellID, msg->targetCellIF);
			msg->targetAMFID = fn_NetSim_Stack_GetDeviceId_asName(trrc->connectedAMFName);
			msg->targetAMFIF = fn_NetSim_LTENR_CORE_INTERFACE(msg->targetAMFID, nGC_INTERFACE_N1_N2);
			msg->associateSMFID = fn_NetSim_Stack_GetDeviceId_asName(trrc->connectedSMFName);

			LTENR_HANDOVER_REQUEST(msg);
		}
	}
}
#pragma endregion

#pragma region RRC_SETUP
/*
type 0: RRCSETUPREQUEST
type 1: RRCRESUMEREQUEST
type 2: RRCRESTABLISHMETREQUEST
*/
void fn_NetSim_LTENR_RRC_SETUP(NETSIM_ID gnbID, NETSIM_ID gnbIF, NETSIM_ID UEID, NETSIM_ID ueIF, int type) {
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = pstruEventDetails->dEventTime;
	pevent.nDeviceId = gnbID;
	pevent.nDeviceType = DEVICE_TYPE(gnbID);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = gnbIF;
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(gnbID, gnbIF);
	LTENR_SET_SUBEVENT(pd, &pevent, LTENR_SUBEVENT_GENERATE_RRC_SETUP);
	pevent.pPacket = LTENR_PACKET_CREATE(pstruEventDetails->nDeviceId, UEID, pstruEventDetails->dEventTime, LTENR_MSG_RRC_SETUP);
	ptrLTENR_MSG msg = pevent.pPacket->pstruMacData->Packet_MACProtocol;
	msg->logicalChannel = LTENR_LOGICALCHANNEL_CCCH;
	msg->physicalChannel = LTENR_PHYSICALCHANNEL_PDCCH;
	msg->transportChannel = LTENR_TRANSPORTCHANNEL_DL_SCH;
	msg->rlcMode = LTENR_RLCMODE_TM;
	msg->srb = LTENR_SRB0;
	ptrLTENR_RRC_RRCSetup rrcsetup = calloc(1, sizeof * rrcsetup);
	if (type == 0)
		rrcsetup->rrcResponsetype = RRCSETUP_RESPONSE_OF_RRCSETUPREQUEST;
	else if (type == 1)
		rrcsetup->rrcResponsetype = RRCSETUP_RESPONSE_OF_RRCRESUMEREQUEST;
	else if (type == 2)
		rrcsetup->rrcResponsetype = RRCSETUP_RESPONSE_OF_RRCREESTABLISHMETREQUEST;
	ptrLTENR_RRC_Hdr hdr = calloc(1, sizeof * hdr);
	hdr->msg = rrcsetup;
	hdr->msgType = LTENR_MSG_RRC_SETUP;
	hdr->SenderID = gnbID;
	hdr->SenderIF = gnbIF;

	fn_NetSIm_LTENR_RRC_ADD_HDR_INTO_PACKET(pevent.pPacket, hdr, rrcSetupID, LTENR_MSG_RRC_SETUP);
	fnpAddEvent(&pevent);
}

void fn_NetSIM_LTENR_RRC_GENERATE_RRCSETUP() {
	ptrLTENR_RRC_Hdr hdr = LTENR_RRC_HDR_GET(pstruEventDetails->pPacket, rrcSetupID);
	ptrLTENR_RRC_RRCSetup rrcsetup = hdr->msg;
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	NETSIM_ID r = pstruEventDetails->pPacket->nReceiverId;
	NETSIM_ID rin = LTENR_FIND_ASSOCIATEINTERFACE(d, in, r);
	rrcsetup->rrcTransactionIdentifier = calloc(1, sizeof * rrcsetup->rrcTransactionIdentifier);
	rrcsetup->rrcSetup = calloc(1, sizeof * rrcsetup->rrcSetup);
	rrcsetup->rrcTransactionIdentifier->RRCTransactionIdentifier = 1;
	ptrLTENR_ASSOCIATIONINFO info = LTENR_ASSOCIATEINFO_FIND(r, rin, 0, 0);
	if (info->d != d) {
		LTENR_ASSOCIATEINFO_ADD(d, in, r, rin);
		LTENR_ASSOCIATEINFO_REMOVE(info->d, info->in, r, rin);
	}
	if (rrcsetup->rrcResponsetype == RRCSETUP_RESPONSE_OF_RRCSETUPREQUEST) {
		LTENR_PHY_ASSOCIATION(d, in, r, rin, false);
		LTENR_PHY_ASSOCIATION(d, in, r, rin, true);
		LTENR_PDCP_ASSOCIATION(d, in, r, rin, true);
		LTENR_RLC_ASSOCIATION(d, in, r, rin, true);

		ptrLTENR_PROTODATA data = LTENR_PROTODATA_GET(r, rin);
		ptrLTENR_PDCPVAR pdcp = LTENR_PDCP_GET(d, in);
		rrcsetup->PDCPVAR = pdcp;
		rrcsetup->pdcpEntity = LTENR_PDCP_FindEntity(pdcp, d, in, r, rin, pdcp->pdcpEntity->rlcMode);
	}
	hdr->msg = rrcsetup;
	LTENR_CallPDCPOut();
}

void fn_NetSim_LTENR_RRC_SETUP_RECV(NETSIM_ID ueID, NETSIM_ID ueIF, NETSIM_ID gnbID, NETSIM_ID gnbIF, ptrLTENR_RRC_RRCSetup rrcSetup) {
	ptrLTENR_UERRC rrc = LTENR_UERRC_GET(ueID, ueIF);
	ptrLTENR_GNBMAC mac = LTENR_GNBMAC_GET(gnbID, gnbIF);
	rrc->SelectedCellID = gnbID;
	rrc->SelectedCellIF = gnbIF;
	rrc->ueRRCState = UERRC_CONNECTED;
	if (!fn_NetSim_isDeviceLTENR(gnbID, gnbIF))
		rrc->ueCMState = UE_CM_CONNECTED;
	rrc->pCell = gnbID;
	//stop all timer t300 t301 and t319 t302 t320
	LTENR_RRC_STOP_T300(rrc);
	LTENR_RRC_STOP_T311(rrc);

	ptrLTENR_RRC_RRCSetupComplete rrcSetupComplete = calloc(1, sizeof * rrcSetupComplete);
	rrcSetupComplete->rrcSetupCompleteIEs = calloc(1, sizeof * rrcSetupComplete->rrcSetupCompleteIEs);
	rrcSetupComplete->rrcSetupCompleteIEs->selectedPLMNIdentity = (UINT)NETSIM_RAND();
	rrcSetupComplete->rrcSetupCompleteIEs->amfIdentifier = mac->epcId;
	rrcSetupComplete->rrcSetupCompleteIEs->gaumiType = GaumiType_native;
	rrcSetupComplete->rrcTransactionIdentifier = calloc(1, sizeof * rrcSetupComplete->rrcTransactionIdentifier);
	rrcSetupComplete->rrcTransactionIdentifier->RRCTransactionIdentifier = 3;
	fn_NetSim_LTENR_RRC_SETUP_COMPLETE(ueID, ueIF, rrcSetupComplete);
}

#pragma endregion

#pragma region RRC_RECONFIGURATION
void fn_NetSim_LTENR_RRC_RECONFIGURATION(NETSIM_ID ueID, NETSIM_ID ueIF,
	NETSIM_ID gnbID, NETSIM_ID gnbIF) {
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = pstruEventDetails->dEventTime;
	pevent.nDeviceId = gnbID;
	pevent.nDeviceType = DEVICE_TYPE(gnbID);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = gnbIF;
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(gnbID, gnbIF);
	LTENR_SET_SUBEVENT(pd, &pevent, LTENR_SUBEVENT_GENERATE_RRC_RECONFIGURATION);
	pevent.pPacket = LTENR_PACKET_CREATE(pevent.nDeviceId, ueID, pevent.dEventTime, LTENR_MSG_RRC_RECONFIGURATION);
	ptrLTENR_MSG msg = pevent.pPacket->pstruMacData->Packet_MACProtocol;
	msg->logicalChannel = LTENR_LOGICALCHANNEL_CCCH;
	msg->physicalChannel = LTENR_PHYSICALCHANNEL_PDCCH;
	msg->transportChannel = LTENR_TRANSPORTCHANNEL_DL_SCH;
	msg->rlcMode = LTENR_RLCMODE_TM;
	msg->srb = LTENR_SRB1;
	fnpAddEvent(&pevent);
}

void  fn_NetSIM_LTENR_RRC_GENERATE_RECONFIGURATION() {
	ptrLTENR_RRC_RrcReconfiguration msg = calloc(1, sizeof * msg);
	msg->rrcTranactionIdentifier = calloc(1, sizeof * msg->rrcTranactionIdentifier);
	msg->rrcTranactionIdentifier->RRCTransactionIdentifier = 1;
	ptrLTENR_RRC_Hdr hdr = calloc(1, sizeof * hdr);
	hdr->msg = msg;
	hdr->msgType = LTENR_MSG_RRC_RECONFIGURATION;
	hdr->SenderID = pstruEventDetails->nDeviceId;
	hdr->SenderIF = pstruEventDetails->nInterfaceId;
	fn_NetSIm_LTENR_RRC_ADD_HDR_INTO_PACKET(pstruEventDetails->pPacket, hdr, rrcReconfigurationID, LTENR_MSG_RRC_RECONFIGURATION);
	LTENR_CallPDCPOut();
}

void fn_NetSim_LTENR_RRC_RECONFIGURATION_RECV(NETSIM_ID ueID, NETSIM_ID ueIF,
	NETSIM_ID gnbID, NETSIM_ID gnbIF, ptrLTENR_RRC_RRCReconfiguration msg) {
	ptrLTENR_UERRC rrc = LTENR_UERRC_GET(ueID, ueIF);
	if (rrc->SelectedCellID != gnbID && rrc->SelectedCellIF != gnbIF && rrc->ueRRCState == UERRC_CONNECTED) {
		rrc->SelectedCellID = gnbID;
		rrc->SelectedCellIF = gnbIF;
	}
}
#pragma endregion

#pragma region RRC_REESTABLISHMENT
void fn_NetSim_LTENR_RRC_REESTABLISHMENT(NETSIM_ID ueID, NETSIM_ID ueIF,
	NETSIM_ID gnbID, NETSIM_ID gnbIF) {
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = pstruEventDetails->dEventTime;
	pevent.nDeviceId = gnbID;
	pevent.nDeviceType = DEVICE_TYPE(gnbID);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = gnbIF;
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(gnbID, gnbIF);
	LTENR_SET_SUBEVENT(pd, &pevent, LTENR_SUBEVENT_GENERATE_RRC_REESTABLISHMENT);
	pevent.pPacket = LTENR_PACKET_CREATE(pevent.nDeviceId, ueID, pevent.dEventTime, LTENR_MSG_RRC_REESTABLISHMENT);
	ptrLTENR_MSG msg = pevent.pPacket->pstruMacData->Packet_MACProtocol;
	msg->logicalChannel = LTENR_LOGICALCHANNEL_CCCH;
	msg->physicalChannel = LTENR_PHYSICALCHANNEL_PDCCH;
	msg->transportChannel = LTENR_TRANSPORTCHANNEL_DL_SCH;
	msg->rlcMode = LTENR_RLCMODE_TM;
	msg->srb = LTENR_SRB1;
	fnpAddEvent(&pevent);
}

void  fn_NetSIM_LTENR_RRC_GENERATE__REESTABLISHMENT() {
	ptrLTENR_RRC_RRCReestablishment msg = calloc(1, sizeof * msg);
	msg->rrcTransactionIdentifier = calloc(1, sizeof * msg->rrcTransactionIdentifier);
	msg->rrcTransactionIdentifier->RRCTransactionIdentifier = 1;
	msg->nextHopChainingCount = 0;

	ptrLTENR_RRC_Hdr hdr = calloc(1, sizeof * hdr);
	hdr->msg = msg;
	hdr->msgType = LTENR_MSG_RRC_REESTABLISHMENT;
	hdr->SenderID = pstruEventDetails->nDeviceId;
	hdr->SenderIF = pstruEventDetails->nInterfaceId;
	fn_NetSIm_LTENR_RRC_ADD_HDR_INTO_PACKET(pstruEventDetails->pPacket, hdr, rrcReestablishmentID, LTENR_MSG_RRC_REESTABLISHMENT);
	LTENR_CallPDCPOut();
}

void fn_NetSim_LTENR_RRC_REESTABLISHMENT_RECV(NETSIM_ID ueID, NETSIM_ID ueIF,
	NETSIM_ID gnbID, NETSIM_ID gnbIF, ptrLTENR_RRC_RRCReestablishment msg) {
	ptrLTENR_UERRC rrc = LTENR_UERRC_GET(ueID, ueIF);
	if (rrc->SelectedCellID != gnbID && rrc->SelectedCellIF != gnbIF && rrc->ueRRCState == UERRC_CONNECTED) {
		fn_NetSim_LTENR_RRC_SETUP(gnbID, gnbIF, ueID, ueIF, 2);
	}
}
#pragma endregion

#pragma region RRC_REESTABLISHMENT_REQUEST
void fn_NetSim_LTENR_RRC_REESTABLISHMENT_REQUEST(NETSIM_ID ueID, NETSIM_ID ueIF,
	NETSIM_ID gnbID, NETSIM_ID gnbIF,
	ptrLTENR_RRC_RRCReestablishmentRequest RestablishmentRequestMsg) {
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = pstruEventDetails->dEventTime;
	pevent.nDeviceId = ueID;
	pevent.nDeviceType = DEVICE_TYPE(ueID);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = ueIF;
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(ueID, ueIF);
	LTENR_SET_SUBEVENT(pd, &pevent, LTENR_SUBEVENT_GENERATE_RRC_REESTABLISHMENT_REQUEST);
	pevent.pPacket = LTENR_PACKET_CREATE(pevent.nDeviceId, gnbID, pevent.dEventTime, LTENR_MSG_RRC_REESTABLISHMENT_REQUEST);
	ptrLTENR_MSG msg = pevent.pPacket->pstruMacData->Packet_MACProtocol;
	msg->logicalChannel = LTENR_LOGICALCHANNEL_CCCH;
	msg->physicalChannel = LTENR_PHYSICALCHANNEL_PDCCH;
	msg->transportChannel = LTENR_TRANSPORTCHANNEL_DL_SCH;
	msg->rlcMode = LTENR_RLCMODE_TM;

	ptrLTENR_RRC_Hdr hdr = calloc(1, sizeof * hdr);
	hdr->msg = RestablishmentRequestMsg;
	hdr->msgType = LTENR_MSG_RRC_REESTABLISHMENT_REQUEST;
	hdr->SenderID = gnbID;
	hdr->SenderIF = gnbIF;

	fn_NetSIm_LTENR_RRC_ADD_HDR_INTO_PACKET(pevent.pPacket, hdr, rrcReestablishmentRequestID, LTENR_MSG_RRC_REESTABLISHMENT_REQUEST);
	fnpAddEvent(&pevent);
}
#pragma endregion

#pragma region Paging & Connection Request
void fn_NetSim_LTENR_RRC_Connection_Request(NETSIM_ID ueID, NETSIM_ID ueIF) {
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = pstruEventDetails->dEventTime;
	pevent.nDeviceId = ueID;
	pevent.nDeviceType = DEVICE_TYPE(ueID);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = ueIF;
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(ueID, ueIF);
	LTENR_SET_SUBEVENT(pd, &pevent, LTENR_SUBEVENT_GENERATE_RRC_SETUP_REQUEST);
	fnpAddEvent(&pevent);
}


void fn_NetSim_LTENR_RRC_GENERATE_RRC_CONN_REQUEST() {
	ptrLTENR_UERRC rrc = LTENR_UERRC_GET(pstruEventDetails->nDeviceId, pstruEventDetails->nInterfaceId);

	if (rrc->ueRRCState == UERRC_CONNECTED) {
		print_ltenr_log("UE is already in connected mode");
		return;
	}

	if (!rrc->rrcSetupRequest)
		rrc->rrcSetupRequest = calloc(1, sizeof * rrc->rrcSetupRequest);
	rrc->rrcSetupRequest->ueIdentity = pstruEventDetails->nDeviceId;
	rrc->rrcSetupRequest->establishmentCause = EstablishmentCause_mo_data;

	pstruEventDetails->pPacket = LTENR_PACKET_CREATE(pstruEventDetails->nDeviceId, rrc->SelectedCellID, pstruEventDetails->dEventTime, LTENR_MSG_RRC_SETUP_REQUEST);
	ptrLTENR_MSG msg = pstruEventDetails->pPacket->pstruMacData->Packet_MACProtocol;
	msg->logicalChannel = LTENR_LOGICALCHANNEL_CCCH;
	msg->physicalChannel = LTENR_PHYSICALCHANNEL_PRACH;
	msg->transportChannel = LTENR_TRANSPORTCHANNEL_RACH;
	msg->rlcMode = LTENR_RLCMODE_TM;
	msg->srb = LTENR_SRB0;
	ptrLTENR_RRC_Hdr hdr = calloc(1, sizeof * hdr);
	hdr->msg = rrc->rrcSetupRequest;
	hdr->msgType = LTENR_MSG_RRC_SETUP_REQUEST;
	hdr->SenderID = pstruEventDetails->nDeviceId;
	hdr->SenderIF = pstruEventDetails->nInterfaceId;

	fn_NetSIm_LTENR_RRC_ADD_HDR_INTO_PACKET(pstruEventDetails->pPacket, hdr, rrcSetupRequestID, LTENR_MSG_RRC_SETUP_REQUEST);
	LTENR_RRC_Start_T300(rrc);
	LTENR_CallRLCOut();
}

void fn_NetSim_LTENR_RRC_SETUP_REQUEST_RECV(NETSIM_ID gnbID, NETSIM_ID gnbIF, NETSIM_ID ueID, NETSIM_ID ueIF, ptrLTENR_RRC_SETUP_REQUEST setupReq) {
	ptrLTENR_UERRC rrc = LTENR_UERRC_GET(ueID, ueIF);
	if (rrc->SelectedCellID == gnbID && rrc->SelectedCellIF == gnbIF) {
		fn_NetSim_LTENR_RRC_SETUP(gnbID, gnbIF, ueID, ueIF, 0);
	}
}

void fn_NetSim_LTENR_RRC_Paging(NETSIM_ID ueID, NETSIM_ID ueIF, NETSIM_ID gnbID, NETSIM_ID gnbIF) {
	fn_NetSim_LTENR_RRC_Connection_Request(ueID, ueIF);
}

#pragma endregion

#pragma region Request_for_SI
static void Request_for_System_Information(NETSIM_ID gnbID, NETSIM_ID gnbIF, NETSIM_ID ueID, NETSIM_ID ueIF) {
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = pstruEventDetails->dEventTime;
	pevent.nDeviceId = gnbID;
	pevent.nDeviceType = DEVICE_TYPE(gnbID);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = gnbIF;
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(gnbID, gnbIF);
	LTENR_SET_SUBEVENT(pd, &pevent, LTENR_SUBEVENT_GENERATE_SI);
	pevent.pPacket = LTENR_PACKET_CREATE(gnbID, ueID, pstruEventDetails->dEventTime, LTENR_MSG_RRC_SI);
	fnpAddEvent(&pevent);
}
void fn_NETSIM_LTENR_SUBEVENT_GENERATE_SI() {
	ptrLTENR_GNBRRC rrc = LTENR_GNBRRC_GET(pstruEventDetails->nDeviceId, pstruEventDetails->nInterfaceId);
	NETSIM_ID d = pstruEventDetails->nDeviceId;
	NETSIM_ID in = pstruEventDetails->nInterfaceId;
	NETSIM_ID r = pstruEventDetails->pPacket->nReceiverId;
	NETSIM_ID rin = fn_NetSim_LTENR_INTERFACE_ID_FROM_CONNECTED_DEVICE(d, in, r);
	ptrLTENR_PROTODATA data = LTENR_PROTODATA_GET(d, in);

	ptrLTENR_ASSOCIATIONINFO info = LTENR_ASSOCIATEINFO_FIND(r, rin, 0, 0);
	if (info->d != d) {
		LTENR_ASSOCIATEINFO_ADD(d, in, r, rin);
		fn_NetSim_Stack_RemoveDeviceFromlink(r, rin, DEVICE_PHYLAYER(info->d, info->in)->nLinkId);
		LTENR_ASSOCIATEINFO_REMOVE(info->d, info->in, r, rin);
		fn_NetSim_Stack_AddDeviceTolink(r, rin, DEVICE_PHYLAYER(d, in)->nLinkId);
	}
	if (!rrc->SI)
		rrc->SI = calloc(1, sizeof * rrc->SI);

	ptrLTENR_MSG msg = pstruEventDetails->pPacket->pstruMacData->Packet_MACProtocol;
	msg->logicalChannel = LTENR_LOGICALCHANNEL_CCCH;
	msg->physicalChannel = LTENR_PHYSICALCHANNEL_PBCH;
	msg->transportChannel = LTENR_TRANSPORTCHANNEL_BCH;
	msg->rlcMode = LTENR_RLCMODE_TM;
	msg->srb = LTENR_SRB0;
	ptrLTENR_RRC_Hdr hdr = calloc(1, sizeof * hdr);
	hdr->msg = rrc->SI;
	hdr->msgType = LTENR_MSG_RRC_SI;
	hdr->SenderID = d;
	hdr->SenderIF = in;
	LTENR_RLC_TMESTABLISHENTITYALL(d, in, r, rin);
	LTENR_RLC_TMESTABLISHENTITYALL(r, rin, d, in);
	LTENR_PHY_ASSOCIATION(d, in, r, rin, true);
	fn_NetSIm_LTENR_RRC_ADD_HDR_INTO_PACKET(pstruEventDetails->pPacket, hdr, siHdrID, LTENR_MSG_RRC_SI);
	LTENR_CallRLCOut();
}

void fn_NetSim_LTENR_RRC_SI_RECV(NETSIM_ID ueID, NETSIM_ID ueIF, NETSIM_ID gnbID, NETSIM_ID gnbIF, ptrLTENR_RRC_SIB1MSG SI) {
	ptrLTENR_UERRC rrc = LTENR_UERRC_GET(ueID, ueIF);
	rrc->SIB1 = NULL;
	if (rrc->ueRRCState == UERRC_CONNECTED) {
		fn_NetSim_LTENR_RRC_UE_MEASUREMENT_REPORT_REQUEST(gnbID, gnbIF);




	}

}
#pragma endregion

#pragma region SIB1_GENERATION
static void start_sib1_generation(NETSIM_ID gnbId,
	NETSIM_ID gnbInterface)
{
	ptrLTENR_GNBRRC rrc = LTENR_GNBRRC_GET(gnbId, gnbInterface);
	NetSim_EVENTDETAILS pevent;
	memset(&pevent, 0, sizeof pevent);
	pevent.dEventTime = rrc->sib1Interval;
	pevent.nDeviceId = gnbId;
	pevent.nDeviceType = DEVICE_TYPE(gnbId);
	pevent.nEventType = TIMER_EVENT;
	pevent.nInterfaceId = gnbInterface;
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(gnbId, gnbInterface);
	LTENR_SET_SUBEVENT(pd, &pevent, LTENR_SUBEVENT_GENERATE_SIB1);
	fnpAddEvent(&pevent);
}
void fn_NetSim_LTENR_GNBRRC_GenerateSIB1()
{
	ptrLTENR_GNBRRC rrc = LTENR_GNBRRC_GET(pstruEventDetails->nDeviceId, pstruEventDetails->nInterfaceId);
	pstruEventDetails->dEventTime += rrc->sib1Interval;
	fnpAddEvent(pstruEventDetails);
	pstruEventDetails->dEventTime -= rrc->sib1Interval;

	pstruEventDetails->pPacket = LTENR_PACKET_CREATE(pstruEventDetails->nDeviceId, 0, pstruEventDetails->dEventTime, LTENR_MSG_RRC_SIB1);
	ptrLTENR_MSG msg = pstruEventDetails->pPacket->pstruMacData->Packet_MACProtocol;
	msg->logicalChannel = LTENR_LOGICALCHANNEL_BCCH;
	msg->physicalChannel = LTENR_PHYSICALCHANNEL_PBCH;
	msg->transportChannel = LTENR_TRANSPORTCHANNEL_BCH;
	msg->rlcMode = LTENR_RLCMODE_TM;
	msg->srb = LTENR_NA;

	if (!rrc->SIB1)
		rrc->SIB1 = calloc(1, sizeof * rrc->SIB1);

	if (!rrc->SIB1->cellAccessRelatedInfo)
		rrc->SIB1->cellAccessRelatedInfo = calloc(1, sizeof * rrc->SIB1->cellAccessRelatedInfo);

	if (!rrc->SIB1->cellAccessRelatedInfo->plmnIdentityInfo)
		rrc->SIB1->cellAccessRelatedInfo->plmnIdentityInfo = calloc(1, sizeof * rrc->SIB1->cellAccessRelatedInfo->plmnIdentityInfo);

	if (!rrc->SIB1->cellSelectionInfo)
		rrc->SIB1->cellSelectionInfo = calloc(1, sizeof * rrc->SIB1->cellSelectionInfo);

	if (!rrc->SIB1->ueTimersandConstants)
		rrc->SIB1->ueTimersandConstants = calloc(1, sizeof * rrc->SIB1->ueTimersandConstants);

	if (!rrc->SIB1->cellEstFailureControl)
		rrc->SIB1->cellEstFailureControl = calloc(1, sizeof * rrc->SIB1->cellEstFailureControl);

	rrc->SIB1->cellSelectionInfo->q_RxLevMin = -70;//minimum value range is (-70,-22)
	rrc->SIB1->cellAccessRelatedInfo->plmnIdentityInfo->cellIdentity = pstruEventDetails->nDeviceId;
	rrc->SIB1->cellAccessRelatedInfo->cellReservedForOtherUse = CellReservedForOperatorUse_notReserved;
	rrc->SIB1->ueTimersandConstants->t300 = T300_ms100;
	rrc->SIB1->ueTimersandConstants->t301 = T301_ms100;
	rrc->SIB1->ueTimersandConstants->t310 = T310_ms100;
	rrc->SIB1->ueTimersandConstants->t311 = T311_ms1000;
	rrc->SIB1->ueTimersandConstants->t319 = T319_ms100;

	rrc->SIB1->cellEstFailureControl->connEstFailCount = ConnEstFailCount_n1;
	rrc->SIB1->cellEstFailureControl->connEstFailOffsetValidity = ConnEstFailOffsetValidity_s30;

	rrc->SIB1->useFullResumeHD = true;
	rrc->SIB1->imsEmergencySupport = true;
	rrc->SIB1->ecallOverIMSSupport = true;

	ptrLTENR_GNBPHY phy = NULL;
	phy = LTENR_GNBPHY_GET(pstruEventDetails->nDeviceId, pstruEventDetails->nInterfaceId);

	ptrLTENR_RRC_Hdr hdr = calloc(1, sizeof * hdr);
	hdr->msg = rrc->SIB1;
	hdr->msgType = LTENR_MSG_RRC_SIB1;
	hdr->SenderID = pstruEventDetails->nDeviceId;
	hdr->SenderIF = pstruEventDetails->nInterfaceId;

	fn_NetSIm_LTENR_RRC_ADD_HDR_INTO_PACKET(pstruEventDetails->pPacket, hdr, sib1HdrID, LTENR_MSG_RRC_SIB1);
	rrc->SIB1 = NULL;
	LTENR_CallRLCOut();
}

void fn_NetSim_LTENR_UERRC_SIB1_RECV(NETSIM_ID ueID, NETSIM_ID ueIF, NETSIM_ID gnbID, NETSIM_ID gnbIF, ptrLTENR_RRC_SIB1MSG SIB1) {
	ptrLTENR_UERRC rrc = LTENR_UERRC_GET(ueID, ueIF);
	rrc->SIB1 = SIB1;
	ptrLTENR_RRC_PLMNIdentityInfo plmn = NULL;
	if (SIB1->cellAccessRelatedInfo->plmnIdentityInfo && rrc->ueRRCState == UERRC_IDLE && rrc->MIB)
	{
		plmn = SIB1->cellAccessRelatedInfo->plmnIdentityInfo;
		if (fn_NetSim_LTENR_EvaluateCellForSelection(rrc) && SIB1->cellAccessRelatedInfo->plmnIdentityInfo->cellIdentity == gnbID)
		{
			Request_for_System_Information(gnbID, gnbIF, ueID, ueIF);
			rrc->SelectedCellID = gnbID;
			rrc->SelectedCellIF = gnbIF;
			ptr_LTENR_SINREPORT head_temp = rrc->SINRReport;
			while (head_temp) {
				SINR_REPORT_REMOVE(&head_temp, head_temp);
			}
			rrc->SINRReport = NULL;
		}
	}
	rrc->SIB1 = NULL;
}
#pragma endregion

#pragma region INIT
void fn_NetSim_LTENR_GNBRRC_Init(NETSIM_ID gnbId,
	NETSIM_ID gnbInterface)
{
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(gnbId, gnbInterface);
	ptrLTENR_GNBRRC rrc = calloc(1, sizeof * rrc);
	LTENR_GNBRRC_SET(gnbId, gnbInterface, rrc);
	//Read config parameter
	rrc->T300 = pd->T300 * MILLISECOND;
	rrc->ueMeasReportInterval = pd->ueMeasurementReportPeriod * MILLISECOND;
	rrc->mibInterval = pd->mibPeriod * MILLISECOND;
	rrc->sib1Interval = pd->sib1Period * MILLISECOND;
	rrc->InterruptionTime = pd->InteruptionTime_ms * MILLISECOND;
	rrc->d = gnbId;
	rrc->in = gnbInterface;
	rrc->HandoverMargin = pd->HandoverMargin;
	rrc->TTT = pd->TTT;
	rrc->Conditional_HO_TTT_Matrix = NULL;
	if (pd->series3GPP == 38) {
		ptrLTENR_Cell_List list = LTENR_Cell_List_ALLOC();
		list->d = gnbId;
		list->in = gnbInterface;
		LTENR_Cell_List_ADD(gnbDCList, list);
	}
	else {
		ptrLTENR_Cell_List list = LTENR_Cell_List_ALLOC();
		list->d = gnbId;
		list->in = gnbInterface;
		LTENR_Cell_List_ADD(enbDCList, list);
	}
	//Start the MIB generation
	start_mib_generation(gnbId, gnbInterface);
	start_sib1_generation(gnbId, gnbInterface);
	fn_NetSim_LTENR_RRC_UE_MEASUREMENT_REPORT_REQUEST(gnbId, gnbInterface);

	// initialize python listener client (best-effort)
	// try connect to localhost:12349 with 100ms send timeout
	py_start_listener("12349", 100);
}

void fn_NetSim_LTENR_UERRC_Init(NETSIM_ID ueId,
	NETSIM_ID ueInterface)
{
	ptrLTENR_UERRC rrc = calloc(1, sizeof * rrc);
	ptrLTENR_PROTODATA data = LTENR_PROTODATA_GET(ueId, ueInterface);
	rrc->ueRRCState = UERRC_IDLE;
	rrc->ueCMState = UE_CM_IDLE;
	rrc->d = ueId;
	rrc->in = ueInterface;
	LTENR_UERRC_SET(ueId, ueInterface, rrc);

}
#pragma endregion

#pragma  region HO_TTT_MATRIX
void fn_NetSim_LTENR_Conditional_HO_TTT_Matrix_Add(NETSIM_ID gnbId, NETSIM_ID gnbIf,
	NETSIM_ID ueId, NETSIM_ID ueIf)
{
	ptrLTENR_GNBRRC rrc = LTENR_GNBRRC_GET(gnbId, gnbIf);
	ptrLTENR_PROTODATA pd = LTENR_PROTODATA_GET(gnbId, gnbIf);
	ptrLTENR_Conditional_HO_TTT_Matrix temp = NULL;
	temp = HO_TTT_MATRIX_ALLOC();
	temp->ueID = ueId;
	temp->ueIF = ueIf;
	temp->gNB_HO_Trigger = NULL;

	ptrLTENR_gNB_HO_Trigger gnbList = NULL;
	ptrLTENR_Cell_List list = NULL;
	if (pd->series3GPP == 38)
		list = gnbDCList;
	else
		list = enbDCList;

	while (list) {
		if (list->d != gnbId)
		{
			gnbList = HO_TRIGGER_ALLOC();
			gnbList->gnbID = list->d;
			gnbList->gnbIF = list->in;
			gnbList->TriggerTime = TTT_NOT_SET;
			LIST_ADD_LAST((void**)&temp->gNB_HO_Trigger, gnbList);
		}
		list = LTENR_Cell_List_NEXT(list);
	}

	LIST_ADD_LAST((void**)&rrc->Conditional_HO_TTT_Matrix, temp);
}

void fn_NetSim_LTENR_Conditional_HO_TTT_Matrix_Remove(NETSIM_ID gnbId, NETSIM_ID gnbIf,
	NETSIM_ID ueId, NETSIM_ID ueIf)
{
	ptrLTENR_GNBRRC rrc = LTENR_GNBRRC_GET(gnbId, gnbIf);
	ptrLTENR_Conditional_HO_TTT_Matrix temp = rrc->Conditional_HO_TTT_Matrix;

	while (temp) {
		if (temp->ueID == ueId && temp->ueIF == ueIf)
		{
			ptrLTENR_gNB_HO_Trigger gnbList = temp->gNB_HO_Trigger;
			while (gnbList)
			{
				HO_TRIGGER_REMOVE(&gnbList, gnbList);
			}
			HO_TTT_MATRIX_REMOVE(&temp, temp);
			break;
		}
		temp = HO_TTT_MATRIX_NEXT(temp);
	}
	rrc->Conditional_HO_TTT_Matrix = temp;
}

double fn_NetSim_LTENR_Conditional_HO_TTT_Matrix_Find(void* list, NETSIM_ID ueID, NETSIM_ID targetID)
{
	ptrLTENR_Conditional_HO_TTT_Matrix temp = (ptrLTENR_Conditional_HO_TTT_Matrix)list;

	while (temp) {
		if (temp->ueID == ueID)
		{
			ptrLTENR_gNB_HO_Trigger gnbList = temp->gNB_HO_Trigger;
			while (gnbList)
			{
				if (gnbList->gnbID == targetID)
					return gnbList->TriggerTime;
				gnbList = HO_TRIGGER_REPORT_NEXT(gnbList);
			}
		}
		temp = HO_TTT_MATRIX_NEXT(temp);
	}
	//fnNetSimError("gNB/eNB ID: %d, not found in the list", targetID);
	return -1;
}

void fn_NetSim_LTENR_Conditional_HO_TTT_Matrix_Set(void* list, NETSIM_ID ueID, NETSIM_ID targetID, double value)
{
	ptrLTENR_Conditional_HO_TTT_Matrix temp = (ptrLTENR_Conditional_HO_TTT_Matrix)list;

	while (temp) {
		if (temp->ueID == ueID)
		{
			ptrLTENR_gNB_HO_Trigger gnbList = temp->gNB_HO_Trigger;
			while (gnbList)
			{
				if (gnbList->gnbID == targetID)
				{
					gnbList->TriggerTime = value;
					return;
				}
				gnbList = HO_TRIGGER_REPORT_NEXT(gnbList);
			}
		}
		temp = HO_TTT_MATRIX_NEXT(temp);
	}
	//fnNetSimError("gNB/eNB ID: %d, not found in the list", targetID);
}

void fn_NetSim_LTENR_Conditional_HO_TTT_Matrix_Free(NETSIM_ID gnbId, NETSIM_ID gnbIf)
{
	ptrLTENR_GNBRRC rrc = LTENR_GNBRRC_GET(gnbId, gnbIf);
	ptrLTENR_Conditional_HO_TTT_Matrix temp = rrc->Conditional_HO_TTT_Matrix;

	while (temp) {
		ptrLTENR_gNB_HO_Trigger gnbList = temp->gNB_HO_Trigger;
		while (gnbList)
		{
			HO_TRIGGER_REMOVE(&gnbList, gnbList);
		}
		HO_TTT_MATRIX_REMOVE(&temp, temp);
	}
	rrc->Conditional_HO_TTT_Matrix = temp;
}

#pragma endregion

#pragma region RRC_MSG_INIT
//create a new file for  rrc msg file
void LTENR_RRCMSG_INIT() {
	mibHdrID = LTENR_HDR_REGISTER("RRC_MIB", NULL, LTENR_RRC_MIB_COPY, LTENR_RRC_MIB_FREEHDR, LTENR_RRC_MIB_TRACE);
	sib1HdrID = LTENR_HDR_REGISTER("RRC_SIB1", NULL, LTENR_RRC_SIB1_COPY, LTENR_RRC_SIB1_FREEHDR, LTENR_RRC_SIB1_TRACE);
	siHdrID = LTENR_HDR_REGISTER("RRC_SI", NULL, NULL, NULL, LTENR_RRC_SI_TRACE);
	rrcSetupRequestID = LTENR_HDR_REGISTER("RRC_SETUP_REQUEST", NULL, LTENR_RRC_SETUP_REQUEST_COPY, LTENR_RRC_SETUP_REQUEST_FREEHDR, LTENR_RRC_SETUP_REQUEST_TRACE);
	rrcSetupID = LTENR_HDR_REGISTER("RRC_SETUP", NULL, LTENR_RRC_SETUP_COPY, LTENR_RRC_SETUP_FREEHDR, LTENR_RRC_SETUP_TRACE);
	rrcSetupCompleteID = LTENR_HDR_REGISTER("RRC_SETUP_COMPLETE", NULL, LTENR_RRC_SETUP_COMPLETE_COPY, LTENR_RRC_SETUP_COMPLETE_FREEHDR, LTENR_RRRC_SETUP_COMPLETE_TRACE);
	ueMEASID = LTENR_HDR_REGISTER("RRC_UEMEASUREMENT_REPORT", NULL, LTENR_RRC_UE_MEASUREMENT_REPORT_COPY, LTENR_RRC_UE_MEASUREMENT_REPORT_FREEHDR, NULL);
	rrcReestablishmentRequestID = LTENR_HDR_REGISTER("RRC_REESTABLISHMENT_REQUEST", NULL, NULL, LTENR_RRC_REESTABLISHMENT_REQUEST_FREEHDR, LTENR_RRRC_REESTABLISHMENT_REQUEST_TRACE);
	rrcReestablishmentID = LTENR_HDR_REGISTER("RRC_REESTABLISHMENT", NULL, NULL, NULL, NULL);
	rrcReconfigurationID = LTENR_HDR_REGISTER("RRC_RECONFIGURATION", NULL, LTENR_RRC_RECONFIGURATION_REQUEST_COPY, LTENR_RRC_RECONFIGURATION_REQUEST_FREEHDR, LTENR_RRRC_RECONFIGURATION_REQUEST_TRACE);
	nasHandoverRequest = LTENR_HDR_REGISTER("HANDOVER_REQUEST", NULL, NULL, LTENR_NAS_HANDOVER_REQUEST_FREEHDR, NULL);
	nasHandoverRequestAck = LTENR_HDR_REGISTER("HANDOVER_REQUEST_ACK", NULL, NULL, LTENR_NAS_HANDOVER_REUQEST_ACK_FREEHDR, NULL);
	nasHandoverTrigger = LTENR_HDR_REGISTER("HANDOVER_COMMAND", NULL, LTENR_NAS_HANDOVER_COMMAND_COPY, LTENR_NAS_HANDOVER_COMMAND_FREEHDR, NULL);
	nasHandoverComplete = LTENR_HDR_REGISTER("HANDOVER_COMPLETE", NULL, NULL, NULL, NULL);
	nasPathSwitch = LTENR_HDR_REGISTER("PATH_SWITCH", NULL, LTENR_NAS_PATH_SWITCH_COPY, LTENR_NAS_PATH_SWITCH_FREEHDR, NULL);
	naspathSwitchAck = LTENR_HDR_REGISTER("PATH_SWITCH_ACK", NULL, LTENR_NAS_PATH_SWITCH_ACK_COPY, LTENR_NAS_PATH_SWTICH_ACK_FREEHDR, NULL);
	nasUEContextRelease = LTENR_HDR_REGISTER("UE_CONTEXT_RELEASE", NULL, NULL, LTENR_NASUE_CONTEXT_RELEASE_FREEHDR, NULL);
	nasUEContextReleaseACK = LTENR_HDR_REGISTER("UE_CONTEXT_RELEASE_ACK", NULL, NULL, LTENR_NASUE_CONTEXT_RELEASE_ACK_FREEHDR, NULL);
}
#pragma endregion

#pragma region RRC_MIB_TRACE_COPY_FREE
void LTENRRC_MIB_TRACE(ptrLTENR_RRC_Hdr hdr, char* s)
{
	ptrLTENR_RRC_MIBMSG msg = hdr->msg;
	char info[BUFSIZ];

	sprintf(info, "Hdr Type = %s\n"
		"CellBarred Status = %s\nSubCarrierSpacingCommom=%s\n"
		"DmrsTypeAPosition=%s\n",
		"MIB", strLTENR_RRC_CellBarred[msg->cellBarred], strLTENR_RRC_subCarrierSpacingCommom[msg->subCarrierSpacingCommon],
		strLTENR_RRC_dmrsTypeAPosition[msg->dmrsTypeAPosition]);
	strcat(s, info);
}

ptrLTENR_RRC_Hdr LTENR_RRC_MIB_COPY(ptrLTENR_RRC_Hdr hdr)
{
	ptrLTENR_RRC_Hdr ret = calloc(1, sizeof * ret);
	memcpy(ret, hdr, sizeof * ret);
	ptrLTENR_RRC_MIBMSG msg = calloc(1, sizeof * msg);
	memcpy(msg, hdr->msg, sizeof * msg);
	ret->msg = msg;
	return ret;
}

ptrLTENR_RRC_MIBMSG LTENR_RRC_MIBMSG_COPY(ptrLTENR_RRC_MIBMSG mib)
{
	ptrLTENR_RRC_MIBMSG ret = calloc(1, sizeof * ret);
	memcpy(ret, mib, sizeof * ret);
	return ret;
}

void LTENR_RRC_MIB_FREEHDR(ptrLTENR_RRC_Hdr hdr) {
	ptrLTENR_RRC_MIBMSG msg = hdr->msg;
	free(msg);
	free(hdr);
}
#pragma endregion

#pragma region RRC_SIB1_TRACE_COPY_FREE
void LTENR_RRC_SIB1_TRACE(ptrLTENR_RRC_Hdr hdr, char* s)
{
	ptrLTENR_RRC_SIB1MSG msg = hdr->msg;
	char info[BUFSIZ];
	sprintf(info, "Hdr Type = %s\n T300 : %s\n T301 : %s\n T310 : %s\n T311 : %s\n T319 : %s\n"
		"connEstFailCount : %s\n ConnEstFailOffsetValidity : %s\n cellReservedForOtherUse : %s\n",
		"SIB1", strLTENR_RRC_T300[msg->ueTimersandConstants->t300],
		strLTENR_RRC_T301[msg->ueTimersandConstants->t301],
		strLTENR_RRC_T310[msg->ueTimersandConstants->t310],
		strLTENR_RRC_T311[msg->ueTimersandConstants->t311],
		strLTENR_RRC_T319[msg->ueTimersandConstants->t319],
		strLTENR_RRC_ConnEstFailCount[msg->cellEstFailureControl->connEstFailCount],
		strLTENR_RRC_ConnEstFailOffsetValidity[msg->cellEstFailureControl->connEstFailOffsetValidity],
		strLTENR_RRC_CellReservedForOperatorUse[msg->cellAccessRelatedInfo->cellReservedForOtherUse]);
	strcat(s, info);
}

ptrLTENR_RRC_Hdr LTENR_RRC_SIB1_COPY(ptrLTENR_RRC_Hdr hdr)
{
	ptrLTENR_RRC_Hdr ret = calloc(1, sizeof * ret);
	memcpy(ret, hdr, sizeof * ret);
	ptrLTENR_RRC_SIB1MSG msg = calloc(1, sizeof * msg);
	msg->cellAccessRelatedInfo = calloc(1, sizeof * msg->cellAccessRelatedInfo);
	msg->cellAccessRelatedInfo->plmnIdentityInfo = calloc(1, sizeof * msg->cellAccessRelatedInfo->plmnIdentityInfo);
	msg->cellSelectionInfo = calloc(1, sizeof * msg->cellSelectionInfo);
	msg->ueTimersandConstants = calloc(1, sizeof * msg->ueTimersandConstants);
	msg->cellEstFailureControl = calloc(1, sizeof * msg->cellEstFailureControl);

	ptrLTENR_RRC_SIB1MSG Ori_msg = hdr->msg;
	if (Ori_msg->cellAccessRelatedInfo->plmnIdentityInfo)
		memcpy(msg->cellAccessRelatedInfo->plmnIdentityInfo, Ori_msg->cellAccessRelatedInfo->plmnIdentityInfo, sizeof * msg->cellAccessRelatedInfo->plmnIdentityInfo);
	memcpy(msg->cellSelectionInfo, Ori_msg->cellSelectionInfo, sizeof * msg->cellSelectionInfo);
	memcpy(msg->ueTimersandConstants, Ori_msg->ueTimersandConstants, sizeof * msg->ueTimersandConstants);
	memcpy(msg->cellEstFailureControl, Ori_msg->cellEstFailureControl, sizeof * msg->cellEstFailureControl);
	msg->useFullResumeHD = Ori_msg->useFullResumeHD;
	msg->imsEmergencySupport = Ori_msg->imsEmergencySupport;
	msg->ecallOverIMSSupport = Ori_msg->ecallOverIMSSupport;
	ret->msg = msg;
	return ret;
}

void LTENR_RRC_SIB1_FREEHDR(ptrLTENR_RRC_Hdr hdr) {
	ptrLTENR_RRC_SIB1MSG msg = hdr->msg;
	free(msg->cellAccessRelatedInfo->plmnIdentityInfo);
	free(msg->cellAccessRelatedInfo);
	free(msg->cellEstFailureControl);
	free(msg->cellSelectionInfo);
	free(msg->ueTimersandConstants);
	free(msg);
	free(hdr);
}
#pragma endregion

#pragma region RRC_SI_TRACE_COPY_FREE
void LTENR_RRC_SI_TRACE(ptrLTENR_RRC_Hdr hdr, char* s) {
	char info[BUFSIZ];
	sprintf(info, "Hdr Type = %s\n",
		"SYSTEMINFORMATION");
	strcat(s, info);
}

void LTENR_RRC_SI_FREEHDR(ptrLTENR_RRC_Hdr hdr) {
	ptrLTENR_RRC_SIMSG msg = hdr->msg;
	free(msg);
	free(hdr);
}
#pragma endregion

#pragma region RRC_SETUP_REQUEST_TRACE_COPY_FREE
void LTENR_RRC_SETUP_REQUEST_TRACE(ptrLTENR_RRC_Hdr hdr, char* s) {
	ptrLTENR_RRC_SETUP_REQUEST msg = hdr->msg;
	char info[BUFSIZ];
	sprintf(info, "Hdr Type = %s\n UEIdentity = %d\n Establishment Cause = %s\n ",
		"RRC_SETUP_REQUEST", msg->ueIdentity, strLTENR_RRC_EstablishmentCause[msg->establishmentCause]);
	strcat(s, info);
}

ptrLTENR_RRC_Hdr LTENR_RRC_SETUP_REQUEST_COPY(ptrLTENR_RRC_Hdr hdr)
{
	ptrLTENR_RRC_Hdr ret = calloc(1, sizeof * ret);
	memcpy(ret, hdr, sizeof * ret);
	ptrLTENR_RRC_SETUP_REQUEST msg = calloc(1, sizeof * msg);
	memcpy(msg, hdr->msg, sizeof * msg);
	ret->msg = msg;
	return ret;
}

void LTENR_RRC_SETUP_REQUEST_FREEHDR(ptrLTENR_RRC_Hdr hdr) {
	ptrLTENR_RRC_SETUP_REQUEST msg = hdr->msg;
	free(msg);
	free(hdr);
}
#pragma endregion

#pragma region RRC_SETUP_TRACE_COPY_FREE
void LTENR_RRC_SETUP_TRACE(ptrLTENR_RRC_Hdr hdr, char* s) {
	ptrLTENR_RRC_RRCSetup msg = hdr->msg;
	char info[BUFSIZ];
	sprintf(info, "Hdr Type = %s\n RRCTransactionIdentifier = %d\n RRCResponsetype = %s\n"
		"PDCP Properties : UEID = %d and GNBID = %d\n DiscardDelayTimer = %lf\n T_Reordering = %lf\n",
		"RRC_SETUP", msg->rrcTransactionIdentifier->RRCTransactionIdentifier,
		strLTENR_RRC_RRCSetup_Response[msg->rrcResponsetype], msg->pdcpEntity->ueId, msg->pdcpEntity->enbId,
		msg->pdcpEntity->discardDelayTime, msg->pdcpEntity->t_Reordering);
	strcat(s, info);
}

ptrLTENR_RRC_Hdr LTENR_RRC_SETUP_COPY(ptrLTENR_RRC_Hdr hdr)
{
	ptrLTENR_RRC_Hdr ret = calloc(1, sizeof * ret);
	memcpy(ret, hdr, sizeof * ret);
	ptrLTENR_RRC_RRCSetup msg = calloc(1, sizeof * msg);
	msg->rrcTransactionIdentifier = calloc(1, sizeof * msg->rrcTransactionIdentifier);
	msg->rrcSetup = calloc(1, sizeof * msg->rrcSetup);
	ptrLTENR_RRC_RRCSetup ori_msg = hdr->msg;
	memcpy(msg->rrcTransactionIdentifier, ori_msg->rrcTransactionIdentifier, sizeof * msg->rrcTransactionIdentifier);
	msg->PDCPVAR = ori_msg->PDCPVAR;
	msg->pdcpEntity = ori_msg->pdcpEntity;
	memcpy(msg->PDCPVAR, ori_msg->PDCPVAR, sizeof * msg->PDCPVAR);
	memcpy(msg->pdcpEntity, ori_msg->pdcpEntity, sizeof * msg->pdcpEntity);
	ret->msg = msg;
	return ret;
}

void LTENR_RRC_SETUP_FREEHDR(ptrLTENR_RRC_Hdr hdr) {
	ptrLTENR_RRC_RRCSetup msg = hdr->msg;
	free(msg->rrcTransactionIdentifier);
	free(msg->rrcSetup);
	free(msg);
	free(hdr);
}
#pragma endregion

#pragma region UE_MEAS_REPORT_TRACE_COPY_TRACE
void LTENR_RRC_UE_MEASUREMENT_REPORT_TRACE(ptrLTENR_RRC_Hdr hdr, char* s) {
	ptrLTENR_RRC_UE_MEASUREMENT_REPORT msg = hdr->msg;
	char info[BUFSIZ];
	sprintf(info, "Hdr Type = %s\n UE_ID = %d\n"
		"CELL_ID = %d\n Report Amount = %s\n RS Type = %s\n Report Interval = %lf us\n",
		"RRC_UE_MEASUREMENT_REPORT", msg->ueID, msg->cellID, strLTENR_RRC_ReportAmount[msg->reportAmount],
		strLTENR_RRC_NR_RS_TYPE[msg->rs_type], msg->reportInteval);
	strcat(s, info);
}

void LTENR_RRC_UE_MEASUREMENT_REPORT_FREEHDR(ptrLTENR_RRC_Hdr hdr)
{
	ptrLTENR_RRC_UE_MEASUREMENT_REPORT msg = hdr->msg;
	while (msg) {
		MEASUREMENT_REPORT_REMOVE(&msg, msg);
	}
	free(hdr);
}

static ptrLTENR_RRC_UE_MEASUREMENT_REPORT copyReport(ptrLTENR_RRC_UE_MEASUREMENT_REPORT r)
{
	ptrLTENR_RRC_UE_MEASUREMENT_REPORT ret = MEASUREMENT_REPORT_ALLOC();
	_ele* temp = ret->ele;
	memcpy(ret, r, sizeof * ret);
	ret->ele = temp;
	return ret;
}

ptrLTENR_RRC_Hdr LTENR_RRC_UE_MEASUREMENT_REPORT_COPY(ptrLTENR_RRC_Hdr hdr)
{
	ptrLTENR_RRC_Hdr ret = calloc(1, sizeof * hdr);
	memcpy(ret, hdr, sizeof * ret);
	ret->msg = NULL;

	ptrLTENR_RRC_UE_MEASUREMENT_REPORT msg = hdr->msg;
	while (msg)
	{
		ptrLTENR_RRC_UE_MEASUREMENT_REPORT t = copyReport(msg);
		LIST_ADD_LAST(&ret->msg, t);
		MEASUREMENT_REPORT_NEXT(msg);
	}
	return ret;
}
#pragma endregion

#pragma region RRC_SETUP_COMPLETE_TRACE_COPY_TRACE
void LTENR_RRRC_SETUP_COMPLETE_TRACE(ptrLTENR_RRC_Hdr hdr, char* s) {
	ptrLTENR_RRC_RRCSetupComplete msg = hdr->msg;
	char info[BUFSIZ];
	sprintf(info, "Hdr Type = %s\n RRCTransactionIdentifier = %d\n"
		"SelectedPLMNIdentity = %d\n AMFIdentifier = %d\n Gaumi Type = %s\n",
		"RRC_SETUP_COMPLETE", msg->rrcTransactionIdentifier->RRCTransactionIdentifier,
		msg->rrcSetupCompleteIEs->selectedPLMNIdentity,
		msg->rrcSetupCompleteIEs->amfIdentifier,
		strLTENR_RRC_GaumiType[msg->rrcSetupCompleteIEs->gaumiType]);
	strcat(s, info);
}

ptrLTENR_RRC_Hdr LTENR_RRC_SETUP_COMPLETE_COPY(ptrLTENR_RRC_Hdr hdr) {
	ptrLTENR_RRC_Hdr ret = calloc(1, sizeof * ret);
	memcpy(ret, hdr, sizeof * ret);
	ptrLTENR_RRC_RRCSetupComplete msg = calloc(1, sizeof * msg);
	ptrLTENR_RRC_RRCSetupComplete ori_msg = hdr->msg;
	memcpy(msg, hdr->msg, sizeof * msg);
	msg->rrcTransactionIdentifier = calloc(1, sizeof * msg->rrcTransactionIdentifier);
	msg->rrcSetupCompleteIEs = calloc(1, sizeof * msg->rrcSetupCompleteIEs);
	memcpy(msg->rrcTransactionIdentifier, ori_msg->rrcTransactionIdentifier, sizeof * msg->rrcTransactionIdentifier);
	memcpy(msg->rrcSetupCompleteIEs, ori_msg->rrcSetupCompleteIEs, sizeof * msg->rrcSetupCompleteIEs);
	ret->msg = msg;
	return ret;
}

void LTENR_RRC_SETUP_COMPLETE_FREEHDR(ptrLTENR_RRC_Hdr hdr) {
	ptrLTENR_RRC_RRCSetupComplete msg = hdr->msg;
	free(msg->rrcSetupCompleteIEs);
	free(msg->rrcTransactionIdentifier);
	free(msg);
	free(hdr);
}
#pragma endregion

#pragma region RRC_REESTABLISHMENT_REQUEST_TRACE_COPY_FREE
void LTENR_RRRC_REESTABLISHMENT_REQUEST_TRACE(ptrLTENR_RRC_Hdr hdr, char* s) {
	ptrLTENR_RRC_RRCReestablishmentRequest msg = hdr->msg;
	char info[BUFSIZ];
	sprintf(info, "Hdr Type = %s\n UEIdentity = %d\n Establishment Cause = %s\n ",
		"RRC_REESTABLISHMENT_REQUEST", msg->ueIdentity, strLTENR_RRC_ReEstablishmentCause[msg->reRestablishmentCause]);
	strcat(s, info);
}

void LTENR_RRC_REESTABLISHMENT_REQUEST_FREEHDR(ptrLTENR_RRC_Hdr hdr) {
	ptrLTENR_RRC_RRCReestablishmentRequest msg = hdr->msg;
	free(msg);
	free(hdr);
}
#pragma endregion

#pragma region RRC_RECONFIG_REQUEST_TRACE_COPY_FREE
void LTENR_RRRC_RECONFIGURATION_REQUEST_TRACE(ptrLTENR_RRC_Hdr hdr, char* s) {
	ptrLTENR_RRC_RRCReconfiguration msg = hdr->msg;
	char info[BUFSIZ];
	sprintf(info, "Hdr Type = %s\n RRC_TRANSACTION_IDENTIFIER = %d\n ",
		"RRC_RECONFIGURATION", msg->rrcTranactionIdentifier->RRCTransactionIdentifier);
	strcat(s, info);
}

ptrLTENR_RRC_Hdr LTENR_RRC_RECONFIGURATION_REQUEST_COPY(ptrLTENR_RRC_Hdr hdr) {
	ptrLTENR_RRC_Hdr ret = calloc(1, sizeof * ret);
	memcpy(ret, hdr, sizeof * ret);
	ptrLTENR_RRC_RRCReconfiguration msg = calloc(1, sizeof * msg);
	ptrLTENR_RRC_RRCReconfiguration ori_msg = hdr->msg;
	memcpy(msg, hdr->msg, sizeof * msg);
	msg->rrcTranactionIdentifier = calloc(1, sizeof * msg->rrcTranactionIdentifier);
	memcpy(msg->rrcTranactionIdentifier, ori_msg->rrcTranactionIdentifier, sizeof * msg->rrcTranactionIdentifier);
	ret->msg = msg;
	return ret;
}

void LTENR_RRC_RECONFIGURATION_REQUEST_FREEHDR(ptrLTENR_RRC_Hdr hdr) {
	ptrLTENR_RRC_RRCReconfiguration msg = hdr->msg;
	free(msg);
	free(hdr);
}
#pragma endregion

#pragma region RRC_RECONFIG_TRACE_COPY_FREE
void LTENR_RRC_RECONFIGURATION_TRACE(ptrLTENR_RRC_Hdr hdr, char* s) {
	ptrLTENR_RRC_RRCReconfiguration msg = hdr->msg;
	char info[BUFSIZ];
	sprintf(info, "Hdr Type = %s\n RRC_TRANSACTION_IDENTIFIER = %d\n ",
		"RRC_RECONFIGURATION", msg->rrcTranactionIdentifier->RRCTransactionIdentifier);
	strcat(s, info);
}

void LTENR_RRC_RECONFIGURATION_FREEHDR(ptrLTENR_RRC_Hdr hdr) {
	ptrLTENR_RRC_RRCReconfiguration msg = hdr->msg;
	free(msg);
	free(hdr);
}
#pragma endregion

#pragma region HANDOVER_REQUEST_TRACE_COPY_FREE
void LTENR_NAS_HANDOVER_REQUEST_TRACE(ptrLTENR_HANDOVER_Hdr hdr, char* s) {
	ptrLTENR_NAS_HANDOVER_REQUEST msg = hdr->msg;
	char info[BUFSIZ];
	sprintf(info, "Hdr Type = %s\n Target_gNB = %d\n AMF_ID = %d\n CELL_Type = %s\n"
		"Last Visited Cell = %d\n HANDOVER_TYPE = %s\n",
		"HANDOVER_REQUEST", msg->targetID, msg->MME_UE_ID,
		strLTENR_NAS_CELL_TYPE[msg->LastVisitedCellType],
		msg->lastVisitedCellID, strLTENR_NAS_HANDOVER_TYPE[msg->type]);
	strcat(s, info);
}

ptrLTENR_HANDOVER_Hdr LTENR_NAS_HANDOVER_REQUEST_COPY(ptrLTENR_HANDOVER_Hdr hdr) {
	ptrLTENR_HANDOVER_Hdr ret = calloc(1, sizeof * ret);
	memcpy(ret, hdr, sizeof * ret);
	ptrLTENR_NAS_HANDOVER_REQUEST msg = calloc(1, sizeof * msg);
	memcpy(msg, hdr->msg, sizeof * msg);
	ret->msg = msg;
	return ret;
}

void LTENR_NAS_HANDOVER_REQUEST_FREEHDR(ptrLTENR_HANDOVER_Hdr hdr) {
	ptrLTENR_NAS_HANDOVER_REQUEST msg = hdr->msg;
	free(msg);
	free(hdr);
}
#pragma endregion

#pragma region HANDOVER_REQUEST_ACK_TRACE_COPY_FREE
void LTENR_NAS_HANDOVER_REUQEST_ACK_TRACE(ptrLTENR_HANDOVER_Hdr hdr, char* s) {
	ptrLTENR_NAS_HANDOVER_REQUEST_ACK msg = hdr->msg;
	char info[BUFSIZ];
	sprintf(info, "Hdr Type = %s\n AMF_ID = %d\n GNB_ID = %d\n OUTCOME = %s\n",
		"HANDOVER_REQUEST_ACK", msg->MME_UE_ID, msg->GNB_UE_ID,
		strLTENR_NAS_HANDOVER_OUTCOME[msg->outCome]);
	strcat(s, info);
}

ptrLTENR_HANDOVER_Hdr LTENR_NAS_HANDOVER_REQUEST_ACK_COPY(ptrLTENR_HANDOVER_Hdr hdr) {
	ptrLTENR_HANDOVER_Hdr ret = calloc(1, sizeof * ret);
	memcpy(ret, hdr, sizeof * ret);
	ptrLTENR_NAS_HANDOVER_REQUEST_ACK msg = calloc(1, sizeof * msg);
	memcpy(msg, hdr->msg, sizeof * msg);
	ret->msg = msg;
	return ret;
}

void LTENR_NAS_HANDOVER_REUQEST_ACK_FREEHDR(ptrLTENR_HANDOVER_Hdr hdr) {
	ptrLTENR_NAS_HANDOVER_REQUEST_ACK msg = hdr->msg;
	free(msg);
	free(hdr);
}
#pragma endregion

#pragma region HANDOVER_COMMAND_TRACE_COPY_FREE
void LTENR_NAS_HANDOVER_COMMAND_TRACE(ptrLTENR_HANDOVER_Hdr hdr, char* s) {
	ptrLTENR_NAS_HANDOVER_COMMAND msg = hdr->msg;
	char info[BUFSIZ];
	sprintf(info, "Hdr Type = %s\n AMF_ID = %d\n CELL_TYPE = %s\n",
		"HANDOVER_COMMAND", msg->MME_UE_ID,
		strLTENR_NAS_HANDOVER_TYPE[msg->type]);
	strcat(s, info);
}

ptrLTENR_HANDOVER_Hdr LTENR_NAS_HANDOVER_COMMAND_COPY(ptrLTENR_HANDOVER_Hdr hdr) {
	ptrLTENR_HANDOVER_Hdr ret = calloc(1, sizeof * ret);
	memcpy(ret, hdr, sizeof * ret);
	ptrLTENR_NAS_HANDOVER_COMMAND msg = calloc(1, sizeof * msg);
	memcpy(msg, hdr->msg, sizeof * msg);
	ret->msg = msg;
	return ret;
}

void LTENR_NAS_HANDOVER_COMMAND_FREEHDR(ptrLTENR_HANDOVER_Hdr hdr) {
	ptrLTENR_NAS_HANDOVER_COMMAND msg = hdr->msg;
	free(msg);
	free(hdr);
}
#pragma endregion

#pragma region PATH_SWTICH_TRACE_COPY_FREE
void LTENR_NAS_PATH_SWITCH_TRACE(ptrLTENR_HANDOVER_Hdr hdr, char* s) {
	ptrLTENR_NAS_PATH_SWITCH_REQUEST msg = hdr->msg;
	char info[BUFSIZ];
	sprintf(info, "Hdr Type = %s\n Target gNB ID = %d\n Associated AMF/EPC ID = %d\n",
		"PATH_SWITCH", msg->targetGNBID, msg->associatedEPCtoTarget);
	strcat(s, info);
}

ptrLTENR_HANDOVER_Hdr LTENR_NAS_PATH_SWITCH_COPY(ptrLTENR_HANDOVER_Hdr hdr) {
	ptrLTENR_HANDOVER_Hdr ret = calloc(1, sizeof * ret);
	memcpy(ret, hdr, sizeof * ret);
	ptrLTENR_NAS_PATH_SWITCH_REQUEST msg = calloc(1, sizeof * msg);
	memcpy(msg, hdr->msg, sizeof * msg);
	ret->msg = msg;
	return ret;
}

void LTENR_NAS_PATH_SWITCH_FREEHDR(ptrLTENR_HANDOVER_Hdr hdr) {
	ptrLTENR_NAS_PATH_SWITCH_REQUEST msg = hdr->msg;
	free(msg);
	free(hdr);
}
#pragma endregion

#pragma region PATH_SWTICH_ACK_TRACE_COPY_FREE
void LTENR_NAS_PATH_SWITCH_ACK_TRACE(ptrLTENR_HANDOVER_Hdr hdr, char* s) {
	ptrLTENR_NAS_PATH_SWITCH_REQUEST_ACK msg = hdr->msg;
	char info[BUFSIZ];
	sprintf(info, "Hdr Type = %s\n Target gNB ID = %d\n Associated AMF/EPC ID = %d\n",
		"PATH_SWITCH_ACK", msg->targetGNBID, msg->associatedEPCtoTarget);
	strcat(s, info);
}

ptrLTENR_HANDOVER_Hdr LTENR_NAS_PATH_SWITCH_ACK_COPY(ptrLTENR_HANDOVER_Hdr hdr) {
	ptrLTENR_HANDOVER_Hdr ret = calloc(1, sizeof * ret);
	memcpy(ret, hdr, sizeof * ret);
	ptrLTENR_NAS_PATH_SWITCH_REQUEST_ACK msg = calloc(1, sizeof * msg);
	memcpy(msg, hdr->msg, sizeof * msg);
	ret->msg = msg;
	return ret;
}

void LTENR_NAS_PATH_SWTICH_ACK_FREEHDR(ptrLTENR_HANDOVER_Hdr hdr) {
	ptrLTENR_NAS_PATH_SWITCH_REQUEST_ACK msg = hdr->msg;
	free(msg);
	free(hdr);
}
#pragma endregion

#pragma region UE_CONTEXT_RELEASE_TRACE_COPY_FREE
void LTENR_NAS_UE_CONTEXT_RELEASE_TRACE(ptrLTENR_HANDOVER_Hdr hdr, char* s) {
	ptrLTENR_NAS_UECONTEXTRELEASEREQUEST msg = hdr->msg;
	char info[BUFSIZ];
	sprintf(info, "Hdr Type = %s\n  gNB ID = %d\n Associated AMF/EPC ID = %d\n CAUSE = %s\n",
		"UE_CONTEXT_RELEASE", msg->gnb_UE_ID, msg->mme_UE_ID, strLTENR_NAS_HANDOVER_REQUEST_CAUSE[msg->cause]);
	strcat(s, info);
}

ptrLTENR_HANDOVER_Hdr LTENR_NAS_UE_CONTEXT_RELEASE_COPY(ptrLTENR_HANDOVER_Hdr hdr) {
	ptrLTENR_HANDOVER_Hdr ret = calloc(1, sizeof * ret);
	memcpy(ret, hdr, sizeof * ret);
	ptrLTENR_NAS_UECONTEXTRELEASEREQUEST msg = calloc(1, sizeof * msg);
	memcpy(msg, hdr->msg, sizeof * msg);
	ret->msg = msg;
	return ret;
}

void LTENR_NASUE_CONTEXT_RELEASE_FREEHDR(ptrLTENR_HANDOVER_Hdr hdr) {
	ptrLTENR_NAS_UECONTEXTRELEASEREQUEST msg = hdr->msg;
	free(msg);
	free(hdr);
}
#pragma endregion

#pragma region UE_CONTEXT_RELEASE_ACK_TRACE_COPY_FREE
void LTENR_NAS_UE_CONTEXT_RELEASE_ACK_TRACE(ptrLTENR_HANDOVER_Hdr hdr, char* s) {
	ptrLTENR_NAS_UECONTEXTRELEASECOMMAND msg = hdr->msg;
	char info[BUFSIZ];
	sprintf(info, "Hdr Type = %s\n  UE ID = %d\n OUTCOME = %s\n CAUSE = %s\n",
		"UE_CONTEXT_RELEASE_ACK", msg->ueID, strLTENR_NAS_HANDOVER_OUTCOME[msg->outCome]
		, strLTENR_NAS_HANDOVER_REQUEST_CAUSE[msg->cause]);
	strcat(s, info);
}

ptrLTENR_HANDOVER_Hdr LTENR_NAS_UE_CONTEXT_RELEASE_ACK_COPY(ptrLTENR_HANDOVER_Hdr hdr) {
	ptrLTENR_HANDOVER_Hdr ret = calloc(1, sizeof * ret);
	memcpy(ret, hdr, sizeof * ret);
	ptrLTENR_NAS_UECONTEXTRELEASECOMMAND msg = calloc(1, sizeof * msg);
	memcpy(msg, hdr->msg, sizeof * msg);
	ret->msg = msg;
	return ret;
}

void LTENR_NASUE_CONTEXT_RELEASE_ACK_FREEHDR(ptrLTENR_HANDOVER_Hdr hdr) {
	ptrLTENR_NAS_UECONTEXTRELEASECOMMAND msg = hdr->msg;
	free(msg);
	free(hdr);
}
#pragma endregion

#pragma region INIT
void fn_NetSim_LTENR_Init()
{
	RRC_TimerEvent_Init();
	LTENR_RRCMSG_INIT();
}
#pragma endregion

typedef enum enum_LTENR_RRCSetup_Response
{
	RRCSETUP_RESPONSE_OF_RRCSETUPREQUEST = 0,
	RRCSETUP_RESPONSE_OF_RRCRESUMEREQUEST,
	RRCSETUP_RESPONSE_OF_RRCREESTABLISHMETREQUEST
} LTENR_RRCSetup_Response;