/************************************************************************************
* NetSim 5G NR – Ping Pong Runtime Logger
* Uses existing runtime socket (NO new listener)
************************************************************************************/

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "stdafx.h"
// #include "LTENR_GNBRRC.h" <-- Removed problematic include
// #include "LTENR_NAS.h" <-- Commented out to fix E1696 error
#include "../LTE_NR/LTENR_NAS.h"
#include <WinSock2.h>

/* ---------------- EXTERNAL RUNTIME SOCKET ---------------- */
/* This socket is ALREADY created by waiting_struct_listener */
extern SOCKET clientSocket;

/* ---------------- CONFIG ---------------- */
#define MAX_UE 2048
#define PINGPONG_WINDOW 2.0   /* seconds */

/* ---------------- STATE ---------------- */
static int lastServingCell[MAX_UE];
static double lastHOTime[MAX_UE];
static int totalHOCount = 0;
static int pingPongCount = 0;

/* ---------------- INIT ---------------- */
void init_pingpong_tracker()
{
	memset(lastServingCell, 0, sizeof(lastServingCell));
	memset(lastHOTime, 0, sizeof(lastHOTime));
	totalHOCount = 0;
	pingPongCount = 0;
}

/* ---------------- SEND TEXT EVENT ---------------- */
static void send_pingpong_text(
	int ue,
	int fromCell,
	int toCell,
	double dt,
	double simTime)
{
	if (clientSocket == INVALID_SOCKET)
		return;

	char buffer[256];

	sprintf(buffer,
		"PINGPONG,TIME,%.3f,UE,%d,FROM,%d,TO,%d,DT,%.3f,TOTAL_HO,%d,TOTAL_PP,%d\n",
		simTime,
		ue,
		fromCell,
		toCell,
		dt,
		totalHOCount,
		pingPongCount);

	send(clientSocket, buffer, (int)strlen(buffer), 0);
}

/* ---------------- MAIN ENTRY (CALL THIS AFTER HO COMPLETE) ---------------- */
void LTENR_Check_PingPong(ptrLTENR_HANDOVER_Hdr hdr)
{
	if (!hdr)
		return;

	int ue = hdr->UEID;
	int fromCell = hdr->serveringCellID;
	int toCell = hdr->targetCellID;
	double now = pstruEventDetails->dEventTime;

	if (ue <= 0 || ue >= MAX_UE)
		return;

	totalHOCount++;

	/* First HO */
	if (lastServingCell[ue] == 0) {
		lastServingCell[ue] = toCell;
		lastHOTime[ue] = now;
		return;
	}

	double dt = now - lastHOTime[ue];

	/* Ping-pong condition */
	if (lastServingCell[ue] == toCell && dt <= PINGPONG_WINDOW) {
		pingPongCount++;

		send_pingpong_text(
			ue,
			fromCell,
			toCell,
			dt,
			now
		);
	}

	lastServingCell[ue] = toCell;
	lastHOTime[ue] = now;
}
