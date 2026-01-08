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
#include "stdafx.h"
#include "LTENR_RLC.h"
#pragma endregion

ptrLTENR_RLCBUFFER LTENR_RLCBUFFER_ALLOC()
{
	ptrLTENR_RLCBUFFER buffer = calloc(1, sizeof * buffer);
	return buffer;
}

void LTENR_RLCBUFFER_FREE(ptrLTENR_RLCBUFFER buffer)
{
	free(buffer);
}

void LTENR_RLCBUFFER_ADD_DATA(ptrLTENR_RLCBUFFER buffer, ptrLTENR_RLCBUFFER_DATA data)
{
	if (buffer->head)
	{
		buffer->tail->next = data;
		data->prev = buffer->tail;
		data->next = NULL;
		buffer->tail = data;
	}
	else
	{
		buffer->head = data;
		buffer->tail = data;
		data->prev = NULL;
		data->next = NULL;
	}
}

void LTENR_RLCBUFFER_REMOVE_DATA(ptrLTENR_RLCBUFFER buffer, ptrLTENR_RLCBUFFER_DATA data)
{
	if (buffer->head == data)
	{
		buffer->head = buffer->head->next;
		data->next = NULL;
		data->prev = NULL;
		if (buffer->head)
		{
			buffer->head->prev = NULL;
		}
		else
		{
			buffer->tail = NULL;
		}
	}
	else
	{
		data->prev->next = data->next;
		if (data->next)
		{
			data->next->prev = data->prev;
		}
		else
		{
			buffer->tail = data->prev;
		}
		data->next = NULL;
		data->prev = NULL;
	}
}

ptrLTENR_RLCBUFFER_DATA LTENR_RLCBUFFER_GETHEAD(ptrLTENR_RLCBUFFER buffer)
{
	ptrLTENR_RLCBUFFER_DATA data = buffer->head;
	LTENR_RLCBUFFER_REMOVE_DATA(buffer, data);
	return data;
}

void LTENR_RLCBuffer_AddPacket(ptrLTENR_RLCBUFFER buffer, NetSim_PACKET* packet,
	UINT sn, UINT16 so, LTENR_LOGICALCHANNEL channel)
{
	ptrLTENR_RLCBUFFER_DATA b = calloc(1, sizeof * b);
	b->packet = packet;
	b->SN = sn;
	b->channel = channel;
	b->SO = so;
	LTENR_RLCBUFFER_ADD_DATA(buffer, b);
}

void LTENR_RLCBuffer_UpdateBuffer(ptrLTENR_RLCBUFFER buffer)
{
	ptrLTENR_RLCBUFFER_DATA b = buffer->head;
	ptrLTENR_RLCBUFFER_DATA p = NULL;
	while (b)
	{
		if (b->isMarkedForRemoval)
		{
			p = b->next;
			LTENR_RLCBUFFER_REMOVE_DATA(buffer, b);
			b = p;
		}
		else
		{
			break;
		}
	}
}

ptrLTENR_RLCBUFFER LTENR_RLCBUFFER_FindAndRemoveAllBySN(ptrLTENR_RLCBUFFER buffer, UINT sn)
{
	ptrLTENR_RLCBUFFER_DATA buf = buffer->head;
	ptrLTENR_RLCBUFFER_DATA p = NULL;
	ptrLTENR_RLCBUFFER ret = calloc(1, sizeof * ret);
	while (buf)
	{
		if (buf->SN == sn)
		{
			p = buf->next;
			LTENR_RLCBUFFER_REMOVE_DATA(buffer, buf);
			LTENR_RLCBUFFER_ADD_DATA(ret, buf);
			buf = p;
		}
		else
		{
			buf = buf->next;
		}
	}
	return ret;
}

ptrLTENR_RLCBUFFER_DATA LTENR_RLCBUFFER_FindAndRemoveBySNSO(ptrLTENR_RLCBUFFER buffer, UINT sn, UINT16 so)
{
	ptrLTENR_RLCBUFFER_DATA buf = buffer->head;
	while (buf)
	{
		if (buf->SN == sn && buf->SO == so)
		{
			LTENR_RLCBUFFER_REMOVE_DATA(buffer, buf);
			return buf;
		}
		LTENR_RLCBUFFER_NEXT(buf);
	}
	return NULL;
}
#pragma endregion