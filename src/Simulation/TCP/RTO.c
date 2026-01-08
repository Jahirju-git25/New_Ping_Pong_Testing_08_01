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
#include "TCP.h"
#include "TCP_Enum.h"
#include "TCP_Header.h"

//RFC 6298
#define alpha	(1/8.0)
#define beta	(1/4.0)
#define G		(0.5*SECOND) /* Min RTO value is set to 0.5 s.
							  * The RFC states this should be 1 s
							  *	Linux seems to be currently using 200 ms.
							  */
#define K		(4)

double calculate_RTO(double R,
					 double* srtt,
					 double* rtt_var)
{
	double rto;
	if (!R) //2.1
		rto = 1 * SECOND;
	else if (!*srtt) //2.2
	{
		*srtt = R;
		*rtt_var = R / 2.0;
		rto = *srtt + max(G, K*(*rtt_var));
	}
	else //2.3
	{
		*rtt_var = (1 - beta) * (*rtt_var) + beta *  fabs(*srtt - R);
		*srtt = (1 - alpha) * (*srtt) + alpha * R;
		rto = *srtt + max(G, K*(*rtt_var));
	}
	return min(max(rto, G), (60 * SECOND)); //2.4 and 2.5
}

static void Backoff_RTO(double* rto)
{
	*rto = min(max((*rto*2), G), (60 * SECOND));
	print_tcp_log("New RTO = %0.2lf", *rto);
}

void add_timeout_event(PNETSIM_SOCKET s,
					   NetSim_PACKET* packet)
{
	NetSim_PACKET* p = fn_NetSim_Packet_CopyPacket(packet);
	add_packet_to_queue(&s->tcb->retransmissionQueue, p, pstruEventDetails->dEventTime);
	if (s->tcb->isRTOTimerRunning) return;
	s->tcb->isRTOTimerRunning = true;
	NetSim_EVENTDETAILS pevent;
	memcpy(&pevent, pstruEventDetails, sizeof pevent);
	pevent.dEventTime += TCP_RTO(s->tcb);
	pevent.dPacketSize = packet->pstruTransportData->dPacketSize;
	pevent.nEventType = TIMER_EVENT;
	pevent.nPacketId = packet->nPacketId;
	if (packet->pstruAppData)
	{
		pevent.nApplicationId = packet->pstruAppData->nApplicationId;
		pevent.nSegmentId = packet->pstruAppData->nSegmentId;
	}
	else
		pevent.nSegmentId = 0;
	pevent.nProtocolId = TX_PROTOCOL_TCP;
	pevent.pPacket = fn_NetSim_Packet_CopyPacket(p);
	pevent.szOtherDetails = NULL;
	pevent.nSubEventType = TCP_RTO_TIMEOUT;
	s->tcb->RTOEventId = fnpAddEvent(&pevent);
	s->tcb->eventPacketptr = pevent.pPacket;
	print_tcp_log("Adding RTO Timer at %0.1lf", pevent.dEventTime);
}

static void handle_rto_timer_for_ctrl(PNETSIM_SOCKET s)
{
	if (isSynbitSet(pstruEventDetails->pPacket))
		resend_syn(s);
	else
		fnNetSimError("Unknown packet %d arrives at %s\n",
					  pstruEventDetails->pPacket->nControlDataType,
					  __FUNCTION__);
}
void handle_rto_timer()
{
	PNETSIM_SOCKET s = find_socket_at_source(pstruEventDetails->pPacket);
	if (!s)
	{
		//Socket is already close. Ignore this event
		fn_NetSim_Packet_FreePacket(pstruEventDetails->pPacket);
		return;
	}
	if (!s->tcb || s->tcb->isOtherTimerCancel)
	{
		fn_NetSim_Packet_FreePacket(pstruEventDetails->pPacket);
		return;
	}

	s->tcb->isRTOTimerRunning = false;
	bool isPresent = isAnySegmentInQueue(&s->tcb->retransmissionQueue);
	
	if (isPresent)
	{
		NetSim_PACKET* segment = get_earliest_copy_segment_from_queue(&s->tcb->retransmissionQueue);
		
		print_tcp_log("\nDevice %d, Time %0.2lf: RTO Time expire.",
					  pstruEventDetails->nDeviceId,
					  pstruEventDetails->dEventTime);
		
		s->tcpMetrics->timesRTOExpired++;

		Backoff_RTO(&TCP_RTO(s->tcb));

		if (isTCPControl(segment))
		{
			handle_rto_timer_for_ctrl(s);
		}
		else
		{
			//Call congestion algorithm
			s->tcb->rto_expired(s);

			resend_segment(s, segment);
		}

		fn_NetSim_Packet_FreePacket(segment);
	}
	else
	{
		//Ignore this event
	}
	fn_NetSim_Packet_FreePacket(pstruEventDetails->pPacket);
}

void restart_rto_timer(PNETSIM_SOCKET s)
{
	fnDeleteEvent(s->tcb->RTOEventId);
	fn_NetSim_Packet_FreePacket(s->tcb->eventPacketptr);
	s->tcb->eventPacketptr = NULL;
	s->tcb->isRTOTimerRunning = false;

	bool isPresent = isAnySegmentInQueue(&s->tcb->retransmissionQueue);
	if (isPresent)
	{
		s->tcb->isRTOTimerRunning = true;
		NetSim_PACKET* segment = get_earliest_copy_segment_from_queue(&s->tcb->retransmissionQueue);
		NetSim_EVENTDETAILS pevent;
		memcpy(&pevent, pstruEventDetails, sizeof pevent);
		pevent.dEventTime += TCP_RTO(s->tcb);
		pevent.dPacketSize = segment->pstruTransportData->dPacketSize;
		pevent.nEventType = TIMER_EVENT;
		pevent.nPacketId = segment->nPacketId;
		if (segment->pstruAppData)
		{
			pevent.nApplicationId = segment->pstruAppData->nApplicationId;
			pevent.nSegmentId = segment->pstruAppData->nSegmentId;
		}
		else
			pevent.nSegmentId = 0;
		pevent.nProtocolId = TX_PROTOCOL_TCP;
		pevent.pPacket = segment;
		pevent.szOtherDetails = NULL;
		pevent.nSubEventType = TCP_RTO_TIMEOUT;
		s->tcb->RTOEventId = fnpAddEvent(&pevent);
		s->tcb->eventPacketptr = segment;
		print_tcp_log("Adding RTO Timer at %0.1lf", pevent.dEventTime);
	}
}
