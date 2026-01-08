#include "main.h"
#include "Application.h"
#include "NetSim_utility.h"

static FILE* fpApp = NULL;

void init_application_log()
{
	if (get_protocol_log_status("APPLICATION_PACKET_LOG"))
	{
		char s[BUFSIZ];
		sprintf(s, "%s\\%s", pszIOLogPath, "Application_Packet_Log.csv");
		fpApp = fopen(s, "w");
		if (!fpApp)
		{
			fnSystemError("Unable to open %s file", s);
			perror(s);
		}
		else
		{
			fprintf(fpApp, "%s,%s,%s,%s,%s,",
				"Application Name", "Application ID", "Source", "Destination", "Packet ID");

			fprintf(fpApp, "%s,%s,%s,%s,",
				"Segment ID", "Packet or Segment Start Time(ms)", "Packet or Segment End Time(ms)", "Packet or Segment size(Bytes)");

			fprintf(fpApp, "%s,%s,%s,",
				"Latency(Microseconds)", "Jitter(Microseconds)", "Throughput(Mbps)");

			fprintf(fpApp, "\n");
		}
	}
	
}

void print_application_log(ptrAPPLICATION_INFO pstruappinfo, NetSim_PACKET* packet)
{
	if (fpApp == NULL)
		return;
	double latency, jitter;
	int TwoWayApplicationflag = 0, linkType = 0; // linkType 1 for UL and 2 for DL
	NETSIM_ID nDestinationId = get_first_dest_from_packet(packet);
	latency = packet->pstruAppData->dEndTime - packet->pstruAppData->dStartTime;
	char Appname[BUFSIZ];
	strcpy(Appname, pstruappinfo->name);
	
	if (nDestinationId == 0) // Check for Broadcast Applications
		return;
	if (pstruappinfo->nAppType == TRAFFIC_INTERACTIVE_GAMING || pstruappinfo->nAppType == TRAFFIC_EMAIL || pstruappinfo->nAppType == TRAFFIC_HTTP)
	{
		TwoWayApplicationflag = 1;
		if (get_first_dest_from_packet(packet) == pstruappinfo->sourceList[0])
		{
			linkType = 1; // UL
			strcat(Appname, "_UL");
		}
		else
		{
			linkType = 2; // DL
			strcat(Appname, "_DL");
		}
	}
	
	if(TwoWayApplicationflag == 0 || (TwoWayApplicationflag == 1 && linkType == 1))
	{
		if (pstruappinfo->uplink.prevPacketLatency == -1 || packet->nPacketId == pstruappinfo->uplink.firstPacketReceived)
		{
			pstruappinfo->uplink.firstPacketReceived = packet->nPacketId;
			if (packet->pstruAppData->nSegmentId == 0 || packet->pstruAppData->nSegmentId == 1)
				jitter = 0;
			else
				jitter = fabs(latency - pstruappinfo->uplink.prevPacketLatency);
		}
		else
		{
			jitter = fabs(latency - pstruappinfo->uplink.prevPacketLatency);
		}
		if (packet->pstruAppData->nSegmentId == 0 || packet->pstruAppData->nPacketFragment == Segment_LastFragment)
			pstruappinfo->uplink.prevStartTime = packet->pstruAppData->dStartTime;
		pstruappinfo->uplink.prevEndTime = packet->pstruAppData->dEndTime;
		pstruappinfo->uplink.prevPacketLatency = latency;
	}

	else if (TwoWayApplicationflag == 1 && linkType == 2)
	{
		if (pstruappinfo->downlink.prevPacketLatency == -1 || packet->nPacketId == pstruappinfo->downlink.firstPacketReceived)
		{
			pstruappinfo->downlink.firstPacketReceived = packet->nPacketId;
			if (packet->pstruAppData->nSegmentId == 0 || packet->pstruAppData->nSegmentId == 1)
				jitter = 0;
			else
				jitter = fabs(latency - pstruappinfo->downlink.prevPacketLatency);
		}
		else
		{
			jitter = fabs(latency - pstruappinfo->downlink.prevPacketLatency);

		}
		if (packet->pstruAppData->nSegmentId == 0 || packet->pstruAppData->nPacketFragment == Segment_LastFragment)
			pstruappinfo->downlink.prevStartTime = packet->pstruAppData->dStartTime;
		pstruappinfo->downlink.prevEndTime = packet->pstruAppData->dEndTime;
		pstruappinfo->downlink.prevPacketLatency = latency;

	}

	else
	{
		fprintf(stderr, "Unknown Application and Link Type");
	}
	
		fprintf(fpApp, "%s,%d,%s,",
			Appname, pstruappinfo->nConfigId, DEVICE_NAME(packet->nSourceId));

		fprintf(fpApp, "%s,%lld,%d,",
			DEVICE_NAME(nDestinationId), packet->nPacketId, packet->pstruAppData->nSegmentId);

		fprintf(fpApp, "%lf,%lf,%lf,",
			(packet->pstruAppData->dStartTime / MILLISECOND), (packet->pstruAppData->dEndTime / MILLISECOND), packet->pstruAppData->dPacketSize);

		fprintf(fpApp, "%lf,%lf,-1,",
			 latency, jitter); // -1 is logged for throughput calculations

		fprintf(fpApp, "\n");
}

void close_application_log()
{
	if (fpApp)
		fclose(fpApp);
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