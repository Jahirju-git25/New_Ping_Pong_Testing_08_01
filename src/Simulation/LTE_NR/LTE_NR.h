#pragma once
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
#ifndef _NETSIM_LTE_NR_H_
#define _NETSIM_LTE_NR_H_

#pragma region HEADER_FILES_AND_WARNING_REMOVAL
#include "List.h"
#pragma warning ( disable : 4090 )
#pragma warning ( disable : 4100 )
#pragma warning ( disable : 4189 )
#pragma warning ( disable : 4244 )
#pragma endregion

#ifdef  __cplusplus
extern "C" {
#endif

#pragma region LOG_MACRO
//#define LTENR_LOG
//#ifdef  LTENR_LOG
//#define LTENR_LOG_DEV
//#define LTENR_PROPAGATION_LOG
//#define LTENR_PDCP_LOG
//#define LTENR_RLC_LOG
//#ifdef LTENR_RLC_LOG
//#define LTENR_RLC_BUFFERSTATUSREPORTING_LOG
//#define LTENR_RLC_TRANSMISSIONSTATUSNOTIFICATION_LOG
//#endif
//#endif //  LTENR_LOG
#pragma endregion
#define ITERATION_START 210000

	int num_gNBs;
	int num_UEs;
	int last_gNB;
#pragma region TYPEDEFS
	// Typedef's that are referenced in multiple headers file
	typedef struct stru_LTENR_AssociatedUEPhyInfo LTENR_ASSOCIATEDUEPHYINFO, * ptrLTENR_ASSOCIATEDUEPHYINFO;
	typedef struct stru_LTENR_GNBPHY LTENR_GNBPHY, * ptrLTENR_GNBPHY;
#pragma endregion //TYPEDEFS

#pragma region DEVICE_TYPE
	static bool isFastProcessing = true;
	/**
	* @enum enum_LTE_NR_DEVICE_TYPE
	* @brief Types of DEVICE
	*/
	typedef enum enum_LTE_NR_DEVICE_TYPE
	{
		LTENR_DEVICETYPE_UE = 0,
		LTENR_DEVICETYPE_GNB = 1,
		LTENR_DEVICETYPE_ENB = 1,
		LTENR_DEVICETYPE_EPC = 2,
		LTENR_DEVICETYPE_AMF = 3,
		LTENR_DEVICETYPE_SMF = 4,
		LTENR_DEVICETYPE_UPF = 5,
		LTENR_DEVICETYPE_LAST, //Keep me at last
	}LTENR_DEVICETYPE;
	static const char* strLTENR_DEVICETYPE[] =
	{ "UE","gNB","EPC","AMF","SMF","UPF","Unknown" };
#pragma endregion

#pragma region LTENR_LogicalChannel
	/**
	* @enum enum_LTENR_LogicalChannel
	* @brief Types of Logical Channel
	*/
	typedef enum enum_LTENR_LogicalChannel
	{
		LTENR_LOGICALCHANNEL_NONE,
		LTENR_LOGICALCHANNEL_PCCH, // Paging Control Channel
		LTENR_LOGICALCHANNEL_BCCH, // Broadcast Control Channel
		LTENR_LOGICALCHANNEL_CCCH, // Common Control Channel
		LTENR_LOGICALCHANNEL_DCCH, // Dedicated Control Channel 
		LTENR_LOGICALCHANNEL_DTCH, // Dedicated Traffic Channel
		LTENR_LOGICALCHANNEL_LAST, // Keep me at last
	}LTENR_LOGICALCHANNEL;
	static const char* strLTENR_LOGICALCHANNEL[] =
	{ "NONE","PCCH","BCCH","CCCH","DCCH","DTCH","Unknown" };
#pragma endregion

#pragma region LTENR_SRB
	/**
	* @enum enum_LTENR_SRB
	* @brief Types of SRB
	*/
	typedef enum enum_LTENR_SRB
	{
		LTENR_SRB0,
		LTENR_SRB1,
		LTENR_SRB2,
		LTENR_NA,
		LTENR_LAST,
	}LTENR_SRB;
	static const char* strLTENR_SRB[] =
	{ "SRB0","SRB1","SRB2","N/A","Unknown" };
#pragma endregion

#pragma region LTENR_TransportChannel
	/**
	* @enum enum_LTENR_TransportChannel
	* @brief Types of Transport Channel
	*/
	typedef enum enum_LTENR_TransportChannel
	{
		LTENR_TRANSPORTCHANNEL_NONE,
		LTENR_TRANSPORTCHANNEL_BCH, // Broadcast Channel
		LTENR_TRANSPORTCHANNEL_PCH, // Paging Channel
		LTENR_TRANSPORTCHANNEL_DL_SCH, // Downlink Shared Channel
		LTENR_TRANSPORTCHANNEL_UL_SCH, // Uplink Shared Channel
		LTENR_TRANSPORTCHANNEL_RACH, // Random Access Channel
		LTENR_TRANSPORTCHANNEL_LAST, // Keep me at last
	}LTENR_TRANSPORTCHANNEL;
	static const char* strLTENR_TRANSPORTCHANNEL[] =
	{ "NONE","BCH","PCH","DL-SCH","UL-SCH","RACH","Unknown" };
#pragma endregion

#pragma region LTENR_PhysicalChannel
	/**
	* @enum enum_LTENR_PhysicalChannel
	* @brief Types of Physical Channel
	*/
	typedef enum enum_LTENR_PhysicalChannel
	{
		LTENR_PHYSICALCHANNEL_NONE,
		LTENR_PHYSICALCHANNEL_PBCH,		// Physical broadcast channel
		LTENR_PHYSICALCHANNEL_PDCCH,	// Physical downlink control channel
		LTENR_PHYSICALCHANNEL_PDSCH,	// Physical downlink shared channel
		LTENR_PHYSICALCHANNEL_PRACH,	// Physical random access channel
		LTENR_PHYSICALCHANNEL_PUSCH,	// physical uplink shared channel
		LTENR_PHYSICALCHANNEL_PUCCH,	// Physical uplink control channel
		LTENR_PHYSICALCHANNEL_LAST,		// Keep me at last
	}LTENR_PHYSICALCHANNEL;
	static const char* strLTENR_PHYSICALCHANNEL[] =
	{ "NONE","PBCH","PDCCH","PDSCH","PRACH","PUSCH","PUCCH","Unknown" };
#pragma endregion

#pragma region LTENR_RLCMode
	/**
	* @enum enum_LTENR_RLCMode
	* @brief Types of RLC Mode
	*/
	typedef enum enum_LTENR_RLCMode
	{
		LTENR_RLCMODE_TM, // Transparent Mode
		LTENR_RLCMODE_UM, // Unacknowledged mode
		LTENR_RLCMODE_AM, // Acknowledged mode
	}LTENR_RLCMODE;
	static const char* strLTENR_RLCMODE[] =
	{ "Transparent Mode","Unacknowledged Mode", "Acknowledged Mode" };
#pragma endregion 

#pragma region LTENR_INTERFACE_TYPE
	/**
	* @enum enum_LTENR_INTERFACE_TYPE
	* @brief Type of Interfaces 
	*/
	typedef enum enum_LTENR_INTERFACE_TYPE {
		nGC_INTERFACE_NULL,
		nGC_INTERFACE_LTE,//4G interface
		nGC_INTERFACE_RAN,//(R)AN--Uu--UE
		nGC_INTERFACE_N1_N2,//UE--N1--AMF //(R)AN--N2--AMF
		nGC_INTERFACE_N3,//UPF--N3--(R)AN
		nGC_INTERFACE_N4,//UPF--N4--SMF
		nGC_INTERFACE_N11,//AMF--N11--SMF
		nGC_INTERFACE_N6,//UPF--N6--DN
		nGC_INTERFACE_XN,//GNB--Xn--GNB
		nGC_INTERFACE_LTE_S1,
		nGC_INTERFACE_LAST,
	}LTENR_INTERFACE_TYPE;
	static const char* strLTENR_DEVICEINTERFACETYPE[] =
	{ "Invalid", "LTE","5G_RAN", "5G_N1_N2","5G_N3","5G_N4","5G_N11","5G_N6", "5G_XN", "LTE_S1","Unknown" };
#pragma endregion

#pragma region LTE_NR_SUBLAYER
	/**
	* @enum enum_LTE_NR_SUBLAYER
	* @brief Type to Sublayers
	*/
	typedef enum enum_LTE_NR_SUBLAYER
	{
		LTENR_SUBLAYER_NGAP,
		LTENR_SUBLAYER_GTP_U,
		LTENR_SUBLAYER_NAS_MM,
		LTENR_SUBLAYER_NAS_SM,
		LTENR_SUBLAYER_PFCP,
		LTENR_SUBLAYER_EPC,
		LTENR_SUBLAYER_NAS,
		LTENR_SUBLAYER_SDAP,
		LTENR_SUBLAYER_RRC,
		LTENR_SUBLAYER_PDCP,
		LTENR_SUBLAYER_RLC,
		LTENR_SUBLAYER_MACSCHEDULER,
		LTENR_SUBLAYER_MAC,
		LTENR_SUBLAYER_PHY,
		LTENR_SUBLAYER_LAST, // Keep me at last
	}LTENR_SUBLAYER;
#pragma endregion

#pragma region FUN_DEF_OUT_AND_IN
	//EPC
	void LTENR_EPC_NetworkOut();

	//Core
	void fn_NetSim_LTENR_AMF_PACKET_HANDLER();
	void fn_NetSim_LTENR_SMF_UPF_PACKET_HANDLER();
	void fn_NetSim_LTENR_GNB_CORE_PACKET_HANDLER();

	//NAS
	void fn_NetSim_NAS_IN();
	void fn_NetSim_LTENR_Handover_RECV();

	//PDCP 
	void fn_NetSim_LTENR_PDCP_TransmitPDCPSDU();
	void fn_NetSim_LTENR_PDCP_ReceivePDCPSDU();

	//RRC
	void fn_NetSim_LTENR_RRC_MacOut();
	void fn_NetSim_LTENR_RRC_MacIn();

	//RLC
	void fn_NetSim_LTENR_RLC_HandlePDUFromUpperLayer();
	void fn_NetSim_LTENR_RLC_HandleMACIN();

	//SDAP
	void fn_NetSim_LTENR_SDAP_HandleMacOut();
	void fn_NetSim_LTENR_SDAP_HandleMacIn();

	//MAC
	void macOut();
	void macIn();

	//HARQ
	void fn_NetSim_LTENR_HARQ_Init();
	void HARQAssociationHandler(NETSIM_ID gnbId, NETSIM_ID gnbIf, NETSIM_ID ueId, NETSIM_ID ueIf, bool isAssociated);
	void fn_NetSim_LTENR_HARQ_Finish();
	void fn_NetSim_LTENR_HARQ_StoreDLPacket();
	void fn_NetSim_LTENR_HARQ_StoreULPacket();
	void fn_NetSim_LTENR_HARQ_HandleSlotStart(NETSIM_ID gnbId, NETSIM_ID gnbIf, int caId);
	void fn_NetSim_LTENR_HARQ_TransmitDownLinkCBG(NETSIM_ID gnbId, NETSIM_ID gnbIf, ptrLTENR_ASSOCIATEDUEPHYINFO info, int caId);
	void fn_NetSim_LTENR_HARQ_TransmitUpLinkCBG(NETSIM_ID gnbId, NETSIM_ID gnbIf, ptrLTENR_ASSOCIATEDUEPHYINFO info, int caId);
	void fn_NetSim_LTENR_HARQ_AllocateCBG(NETSIM_ID gnbId, NETSIM_ID gnbIf, int caId);
	void fn_NetSim_LTENR_HARQ_Transmit(NETSIM_ID gNBId, NETSIM_ID gNBIf, int caId);
	bool fn_NetSim_LTENR_HARQ_GetNDIFlag(ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo, bool isUplink, int caId);
	UINT fn_NetSim_LTENR_HARQ_GetPRBReqdForRetx(ptrLTENR_ASSOCIATEDUEPHYINFO assocInfo, bool isUplink, int caId);
#pragma endregion

#pragma region OUT_AND_IN_MACROS
#ifdef OUT
#undef OUT
#define OUT 0
#endif
#ifdef IN
#undef IN
#define IN 1
#endif
#pragma endregion

#pragma region LTENR_SublayerInfo
	typedef struct stru_LTENR_SublayerInfo
	{
		LTENR_SUBLAYER subLayer;
		char name[50];
		void(*fnOut)(); // Call back function for out event
		void(*fnIn)(); // Call back function for in event
	}LTENR_SUBLAYERINFO, *ptrLTENR_SUBLAYERINFO;
	static LTENR_SUBLAYERINFO subLayerInfo[LTENR_SUBLAYER_LAST] =
	{
		{LTENR_SUBLAYER_NGAP,"NGAP",NULL,NULL},
		{LTENR_SUBLAYER_GTP_U,"GTP-U",NULL,NULL},
		{LTENR_SUBLAYER_NAS_MM,"NAS-MM",NULL,NULL},
		{LTENR_SUBLAYER_NAS_SM,"NAS-SM",NULL,NULL},
		{LTENR_SUBLAYER_PFCP,"PFCP",NULL,NULL},
		{LTENR_SUBLAYER_EPC,"EPC",LTENR_EPC_NetworkOut,NULL},
		{LTENR_SUBLAYER_NAS,"NAS",NULL,fn_NetSim_NAS_IN},
		{LTENR_SUBLAYER_SDAP,"SDAP",fn_NetSim_LTENR_SDAP_HandleMacOut,fn_NetSim_LTENR_SDAP_HandleMacIn},
		{LTENR_SUBLAYER_RRC,"RRC",fn_NetSim_LTENR_RRC_MacOut, fn_NetSim_LTENR_RRC_MacIn},
		{LTENR_SUBLAYER_PDCP,"PDCP",fn_NetSim_LTENR_PDCP_TransmitPDCPSDU,fn_NetSim_LTENR_PDCP_ReceivePDCPSDU},
		{LTENR_SUBLAYER_RLC,"RLC",fn_NetSim_LTENR_RLC_HandlePDUFromUpperLayer,fn_NetSim_LTENR_RLC_HandleMACIN},
		{LTENR_SUBLAYER_MACSCHEDULER,"MACSCHEDULER",NULL,NULL},
		{LTENR_SUBLAYER_MAC,"MAC",macOut,macIn},
		{LTENR_SUBLAYER_PHY,"PHY",NULL,NULL},
	};
	void fnCallSublayer(LTENR_SUBLAYER layer, int c);
#define LTENR_CallEPCOut()			(fnCallSublayer(LTENR_SUBLAYER_EPC,OUT))
#define LTENR_CallNASOut()			(fnCallSublayer(LTENR_SUBLAYER_NAS,OUT))
#define LTENR_CallNASIn()			(fnCallSublayer(LTENR_SUBLAYER_NAS,IN))
#define LTENR_CallSDAPOut()			(fnCallSublayer(LTENR_SUBLAYER_SDAP,OUT))
#define LTENR_CallSDAPIn()			(fnCallSublayer(LTENR_SUBLAYER_SDAP,IN))
#define LTENR_CallRRCOut()			(fnCallSublayer(LTENR_SUBLAYER_RRC,OUT))
#define LTENR_CallRRCIn()			(fnCallSublayer(LTENR_SUBLAYER_RRC,IN))
#define LTENR_CallPDCPOut()			(fnCallSublayer(LTENR_SUBLAYER_PDCP,OUT))
#define LTENR_CallPDCPIN()			(fnCallSublayer(LTENR_SUBLAYER_PDCP,IN))
#define LTENR_CallRLCOut()			(fnCallSublayer(LTENR_SUBLAYER_RLC,OUT))
#define LTENR_CallRLCIn()			(fnCallSublayer(LTENR_SUBLAYER_RLC,IN))
#define LTENR_CallMACSCHEDULEROut()	(fnCallSublayer(LTENR_SUBLAYER_MACSCHEDULER,OUT))
#define LTENR_CallMACSCHEDULERIn()	(fnCallSublayer(LTENR_SUBLAYER_MACSCHEDULER,IN))
#define LTENR_CallMACOut()			(fnCallSublayer(LTENR_SUBLAYER_MAC,OUT))
#define LTENR_CallMACIn()			(fnCallSublayer(LTENR_SUBLAYER_MAC,IN))
#pragma endregion

#pragma region ASSOCIATION_STRUCT_AND_APIs
	typedef struct stru_LTENR_ASSOCIATIONINFO
	{
		NETSIM_ID d;
		NETSIM_ID in;
		struct stru_LTENR_ASSOCIATIONINFO* next;
	}LTENR_ASSOCIATIONINFO, *ptrLTENR_ASSOCIATIONINFO;
	void LTENR_ASSOCIATEINFO_ADD(NETSIM_ID d,
								 NETSIM_ID in,
								 NETSIM_ID rd,
								 NETSIM_ID rin);
	void LTENR_ASSOCIATEINFO_REMOVE(NETSIM_ID d,
									NETSIM_ID in,
									NETSIM_ID rd,
									NETSIM_ID rin);
	ptrLTENR_ASSOCIATIONINFO LTENR_ASSOCIATEINFO_FIND(NETSIM_ID d,
													  NETSIM_ID in,
													  NETSIM_ID rd /*OPTIONAL, Return first association*/,
													  NETSIM_ID rin /*OPTIONAL, Return first association for rd*/);
#define LTENR_ISASSOCIATED(d,in,rd,rin) (LTENR_ASSOCIATEINFO_FIND(d,in,rd,rin)!=NULL)
	NETSIM_ID LTENR_FIND_ASSOCIATEINTERFACE(NETSIM_ID d, NETSIM_ID i, NETSIM_ID rd);
#define LTENR_FIND_ASSOCIATEDGNB(ueId,ueIn) (isUE(ueId,ueIn)?LTENR_ASSOCIATEINFO_FIND(ueId,ueIn,0,0)->d:0)
	void fn_NetSim_LTENR_InitAssociation();
	void fn_NetSim_LTENR_RegisterCallBackForAssociation(void(*fnCallBack)(NETSIM_ID gnbId,
																		  NETSIM_ID gnbIf,
																		  NETSIM_ID ueId,
																		  NETSIM_ID ueIf,
																		  bool isAssociated));
#pragma endregion

#pragma region NGAP_PROTODATA
	typedef struct stru_LTE_NR_NGAP_DATA {
		NETSIM_ID d;
		NETSIM_ID in;
		NETSIM_IPAddress destIPaddr;
		NETSIM_ID destid;
		LTENR_INTERFACE_TYPE IFType;
		_ptr_ele ele;
	}LTE_NR_NGAP_DATA, *ptrLTE_NR_NGAP_DATA;
#define LTENR_NGAP_DATA_ALLOC()			(list_alloc(sizeof(LTE_NR_NGAP_DATA),offsetof(LTE_NR_NGAP_DATA,ele)))
#define LTENR_NGAP_DATA_NEXT(info)			(LIST_NEXT(info))
#define LTENR_NGAP_DATA_ADD(info,e)		(LIST_ADD_LAST(&(info),e))
#define LTENR_NGAP_DATA_REMOVE(info,e)		(LIST_REMOVE(&(info),e))
#pragma endregion

#pragma region PFCP_PROTODATA
	typedef struct stru_LTE_NR_PFCP_DATA {
		NETSIM_ID d;
		NETSIM_ID in;
		NETSIM_IPAddress destIPaddr;
		NETSIM_ID destid;
		LTENR_INTERFACE_TYPE IFType;
		_ptr_ele ele;
	}LTE_NR_PFCP_DATA, * ptrLTE_NR_PFCP_DATA;
#define LTENR_PFCP_DATA_ALLOC()			(list_alloc(sizeof(LTE_NR_PFCP_DATA),offsetof(LTE_NR_PFCP_DATA,ele)))
#define LTENR_PFCP_DATA_NEXT(info)			(LIST_NEXT(info))
#define LTENR_PFCP_DATA_ADD(info,e)		(LIST_ADD_LAST(&(info),e))
#define LTENR_PFCP_DATA_REMOVE(info,e)		(LIST_REMOVE(&(info),e))
#pragma endregion

#pragma region GTPU_PROTODATA
	typedef struct stru_LTE_NR_GTPU_DATA {
		NETSIM_ID d;
		NETSIM_ID in;
		NETSIM_IPAddress destIPaddr;
		NETSIM_ID destid;
		LTENR_INTERFACE_TYPE IFType;
		_ptr_ele ele;
	}LTE_NR_GTPU_DATA, * ptrLTE_NR_GTPU_DATA;
#define LTENR_GTPU_DATA_ALLOC()			(list_alloc(sizeof(LTE_NR_GTPU_DATA),offsetof(LTE_NR_GTPU_DATA,ele)))
#define LTENR_GTPU_DATA_NEXT(info)			(LIST_NEXT(info))
#define LTENR_GTPU_DATA_ADD(info,e)		(LIST_ADD_LAST(&(info),e))
#define LTENR_GTPU_DATA_REMOVE(info,e)		(LIST_REMOVE(&(info),e))
#pragma endregion

#pragma region GTPC_PROTODATA
	typedef struct stru_LTE_NR_N11STACK_DATA {
		NETSIM_ID d;
		NETSIM_ID in;
		NETSIM_IPAddress destIPaddr;
		NETSIM_ID destid;
		LTENR_INTERFACE_TYPE IFType;
		_ptr_ele ele;
	}LTE_NR_N11STACK_DATA, * ptrLTE_NR_N11STACK_DATA;
#define LTENR_N11STACK_DATA_ALLOC()			(list_alloc(sizeof(LTE_NR_N11STACK_DATA),offsetof(LTE_NR_N11STACK_DATA,ele)))
#define LTENR_N11STACK_DATA_NEXT(info)			(LIST_NEXT(info))
#define LTENR_N11STACK_DATA_ADD(info,e)		(LIST_ADD_LAST(&(info),e))
#define LTENR_N11STACK_DATA_REMOVE(info,e)		(LIST_REMOVE(&(info),e))
#pragma endregion

#pragma region NSA
	typedef enum enum_LTENR_CELL_TYPES 
	{
		CELL_TYPE_NONE,
		MMWAVE_CELL_TYPE,
		NON_MMWAVE_CELL_TYPE,
	}LTENR_CELL_TYPES;

	typedef enum enum_LTENR_NSA_MODES {
		NSA_MODE_OPTION3,
		NSA_MODE_OPTION3A,
		NSA_MODE_OPTION3X,
		NSA_MODE_OPTION4,
		NSA_MODE_OPTION4A,
		NSA_MODE_OPTION7,
		NSA_MODE_OPTION7A,
		NSA_MODE_OPTION7X,
		NSA_MODE_LAST,
	}LTENR_NSA_MODES;
	static const char* strLTENR_NSA_MODES[] =
	{ "OPTION_3","OPTION_3A","OPTION_3X","OPTION_4","OPTION_4A","OPTION_7",
		"OPTION_7A", "OPTION_7X","Unknown" };
#pragma endregion

#pragma region BLER_MCS_SELECTION
	typedef enum
	{
		LTENR_ZERO_BLER,
		LTENR_BLER_ENABLE,
	}LTENR_BLER_MODEL;
	static const char* strLTENR_BLER_MODEL[] =
	{ "ZERO_BLER", "BLER_ENABLE" };

	typedef enum
	{
		LTENR_IDEAL_SHANNON_THEOREM_BASED_RATE,
		LTENR_SHANNON_RATE_WITH_ATTENUATION_FACTOR,
	}LTENR_MCS_SELECTION_MODEL;
	static const char* strLTENR_MCS_SELECTION_MODEL[] =
	{ "IDEAL_SHANNON_THEOREM_BASED_RATE","SHANNON_RATE_WITH_ATTENUATION_FACTOR", };
#pragma endregion

#pragma region PROTODATA_AND_APIs
	typedef struct stru_LTE_NR_Protocol_Data
	{
		LTENR_DEVICETYPE deviceType;
		NETSIM_ID deviceId;
		NETSIM_ID interfaceId;
		LTENR_INTERFACE_TYPE IFType;
		MAC_LAYER_PROTOCOL macProtocol;

		//Core
		NETSIM_ID associateCount;
		NETSIM_ID NGAP_COUNT;
		bool isNGAPDATA;
		ptrLTE_NR_NGAP_DATA NGAPDATA;

		NETSIM_ID PFCP_COUNT;
		bool isPFCPDATA;
		ptrLTE_NR_PFCP_DATA PFCPDATA;

		NETSIM_ID GTPU_COUNT;
		bool isGTPUDATA;
		ptrLTE_NR_GTPU_DATA GTPUDATA;
		
		NETSIM_ID N11STACK_COUNT;
		bool isN11STACK;
		ptrLTE_NR_N11STACK_DATA N11STACKDATA;

		char* connectedAMFName;
		char* connectedSMFName;
		char* connectedUPFName;
		
		//Config parameter -- RRC
		double T300;
		double ueMeasurementReportPeriod;
		double mibPeriod;
		double sib1Period;
		double InteruptionTime_ms;
		//Config parameter -- PDCP
		double discardTimer;
		bool isHdrCompression;
		bool isoutOfOrderDelivery;
		double t_reorderingTimer;

		//Config parameter -- RLC
		char* maxRetxThreshold;
		char* pollPdu;
		char* pollBytes;
		char* t_reassembly;
		char* t_pollRetransmit;
		char* t_statusProhibit;

		//Config parameter -- MAC
		char* schedulingType;
		double EWMA_Learning_Rate;

		//Config parameter -- HARQ
		bool isHARQEnable;
		UINT HARQ_ProcessCount;
		UINT maxCBGPerTB; // 2,4,6,8
		UINT HARQ_RetryLimit;

		//Config parameter -- HandoverMargin
		double HandoverMargin; //ms

		//Config parameter -- TIME_TO_TRIGGER
		double TTT; //ms

		//Config parameter -- Network Slicing
		bool isSlicing;
		double LagrangeMultiplierLearningRate;

		//Config parameter -- OLLA
		bool isOLLAEnable;
		double targetBLER;
		
		//BLER model
		LTENR_BLER_MODEL blerModel;
		
		//MCS Selection model
		LTENR_MCS_SELECTION_MODEL mcsSelctionModel;
		double alpha; //attenuation factor

		ptrLTENR_ASSOCIATIONINFO associationInfo;
		void* LTENR_LAYER_DATA[LTENR_DEVICETYPE_LAST][LTENR_SUBLAYER_LAST];

		int series3GPP;

		//Dual connectivity
		bool isDCEnable;
		bool isSNConnect;
		LTENR_CELL_TYPES MasterCellType;
		LTENR_CELL_TYPES SecCellType;
		LTENR_NSA_MODES NSA_MODE;
	}LTENR_PROTODATA,*ptrLTENR_PROTODATA;
	ptrLTENR_PROTODATA LTENR_PROTOCOLDATA_ALLOC(NETSIM_ID d,
												NETSIM_ID in);
	void LTENR_PROTOCOLDATA_FREE(NETSIM_ID d,
								 NETSIM_ID in);
	void* LTENR_SUBLAYER_DATA_GET(NETSIM_ID d, NETSIM_ID in,
								  LTENR_DEVICETYPE devType,
								  LTENR_SUBLAYER subLayer);
	void LTENR_SUBLAYER_DATA_SET(NETSIM_ID d, NETSIM_ID in,
								 LTENR_DEVICETYPE devType,
								 LTENR_SUBLAYER subLayer,
								 void* data);
	bool LTENR_SUBLAYER_DATA_IsInitialized(NETSIM_ID d, NETSIM_ID in,
		LTENR_DEVICETYPE devType,
		LTENR_SUBLAYER subLayer);
	ptrLTENR_PROTODATA LTENR_PROTODATA_GET(NETSIM_ID d, NETSIM_ID in);
#define LTENR_UEPHY_GET(d,i)			(LTENR_SUBLAYER_DATA_GET(d,i,LTENR_DEVICETYPE_UE,LTENR_SUBLAYER_PHY))
#define LTENR_UEMAC_GET(d,i)			(LTENR_SUBLAYER_DATA_GET(d,i,LTENR_DEVICETYPE_UE,LTENR_SUBLAYER_MAC))
#define LTENR_UERLC_GET(d,i)			(LTENR_SUBLAYER_DATA_GET(d,i,LTENR_DEVICETYPE_UE,LTENR_SUBLAYER_RLC))
#define LTENR_UEMACSCHEDULER_GET(d,i)	(LTENR_SUBLAYER_DATA_GET(d,i,LTENR_DEVICETYPE_UE,LTENR_SUBLAYER_MACSCHEDULER))
#define LTENR_UEPDCP_GET(d,i)			(LTENR_SUBLAYER_DATA_GET(d,i,LTENR_DEVICETYPE_UE,LTENR_SUBLAYER_PDCP))
#define LTENR_UERRC_GET(d,i)			(LTENR_SUBLAYER_DATA_GET(d,i,LTENR_DEVICETYPE_UE,LTENR_SUBLAYER_RRC))
#define LTENR_UESDAP_GET(d,i)			(LTENR_SUBLAYER_DATA_GET(d,i,LTENR_DEVICETYPE_UE,LTENR_SUBLAYER_SDAP))
#define LTENR_GNBPHY_GET(d,i)			((ptrLTENR_GNBPHY)(LTENR_SUBLAYER_DATA_GET(d,i,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_PHY)))
#define LTENR_GNBMAC_GET(d,i)			(LTENR_SUBLAYER_DATA_GET(d,i,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_MAC))
#define LTENR_GNBRLC_GET(d,i)			(LTENR_SUBLAYER_DATA_GET(d,i,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_RLC))
#define LTENR_GNBMACSCHEDULER_GET(d,i)	(LTENR_SUBLAYER_DATA_GET(d,i,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_MACSCHEDULER))
#define LTENR_GNBPDCP_GET(d,i)			(LTENR_SUBLAYER_DATA_GET(d,i,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_PDCP))
#define LTENR_GNBRRC_GET(d,i)			(LTENR_SUBLAYER_DATA_GET(d,i,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_RRC))
#define LTENR_GNBSDAP_GET(d,i)			(LTENR_SUBLAYER_DATA_GET(d,i,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_SDAP))
#define LTENR_EPC_GET(d,i)				(LTENR_SUBLAYER_DATA_GET(d,i,LTENR_DEVICETYPE_EPC,LTENR_SUBLAYER_EPC))

	//5G core -- AMF
#define LTENR_AMF_NGAP_GET(d)				(LTENR_SUBLAYER_DATA_GET(d,0,LTENR_DEVICETYPE_AMF,LTENR_SUBLAYER_NGAP))
#define LTENR_AMF_NGAP_SET(d,data)			(LTENR_SUBLAYER_DATA_SET(d,0,LTENR_DEVICETYPE_AMF,LTENR_SUBLAYER_NGAP,data))
#define LTENR_AMF_NAS_MM_GET(d)				(LTENR_SUBLAYER_DATA_GET(d,0,LTENR_DEVICETYPE_AMF,LTENR_SUBLAYER_NAS_MM))
#define LTENR_AMF_NAS_MM_SET(d,data)		(LTENR_SUBLAYER_DATA_SET(d,0,LTENR_DEVICETYPE_AMF,LTENR_SUBLAYER_NAS_MM,data))

	//5G core -- SMF
#define LTENR_SMF_GTP_U_GET(d)				(LTENR_SUBLAYER_DATA_GET(d,0,LTENR_DEVICETYPE_SMF,LTENR_SUBLAYER_GTP_U))
#define LTENR_SMF_GTP_U_SET(d,data)			(LTENR_SUBLAYER_DATA_SET(d,0,LTENR_DEVICETYPE_SMF,LTENR_SUBLAYER_GTP_U,data))
#define LTENR_SMF_NAS_SM_GET(d)				(LTENR_SUBLAYER_DATA_GET(d,0,LTENR_DEVICETYPE_SMF,LTENR_SUBLAYER_NAS_SM))
#define LTENR_SMF_NAS_SM_SET(d,data)		(LTENR_SUBLAYER_DATA_SET(d,0,LTENR_DEVICETYPE_SMF,LTENR_SUBLAYER_NAS_SM,data))
#define LTENR_SMF_PFCP_GET(d)				(LTENR_SUBLAYER_DATA_GET(d,0,LTENR_DEVICETYPE_SMF,LTENR_SUBLAYER_PFCP))
#define LTENR_SMF_PFCP_SET(d,data)			(LTENR_SUBLAYER_DATA_SET(d,0,LTENR_DEVICETYPE_SMF,LTENR_SUBLAYER_PFCP,data))

	//5G core -- UPF
#define LTENR_UPF_GTP_U_GET(d)				(LTENR_SUBLAYER_DATA_GET(d,0,LTENR_DEVICETYPE_UPF,LTENR_SUBLAYER_GTP_U))
#define LTENR_UPF_GTP_U_SET(d,data)			(LTENR_SUBLAYER_DATA_SET(d,0,LTENR_DEVICETYPE_UPF,LTENR_SUBLAYER_GTP_U,data))
#define LTENR_UPF_PFCP_GET(d)				(LTENR_SUBLAYER_DATA_GET(d,0,LTENR_DEVICETYPE_UPF,LTENR_SUBLAYER_PFCP))
#define LTENR_UPF_PFCP_SET(d,data)			(LTENR_SUBLAYER_DATA_SET(d,0,LTENR_DEVICETYPE_UPF,LTENR_SUBLAYER_PFCP,data))

	//5G core -- gNB
#define LTENR_GNB_NGAP_GET(d)				(LTENR_SUBLAYER_DATA_GET(d,0,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_NGAP))
#define LTENR_GNB_NGAP_SET(d,data)			(LTENR_SUBLAYER_DATA_SET(d,0,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_NGAP,data))
#define LTENR_GNB_GTP_U_GET(d)				(LTENR_SUBLAYER_DATA_GET(d,0,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_GTP_U))
#define LTENR_GNB_GTP_U_SET(d,data)			(LTENR_SUBLAYER_DATA_SET(d,0,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_GTP_U,data))

#define LTENR_UEPHY_SET(d,i,data)			(LTENR_SUBLAYER_DATA_SET(d,i,LTENR_DEVICETYPE_UE,LTENR_SUBLAYER_PHY,data))
#define LTENR_UEMAC_SET(d,i,data)			(LTENR_SUBLAYER_DATA_SET(d,i,LTENR_DEVICETYPE_UE,LTENR_SUBLAYER_MAC,data))
#define LTENR_UERLC_SET(d,i,data)			(LTENR_SUBLAYER_DATA_SET(d,i,LTENR_DEVICETYPE_UE,LTENR_SUBLAYER_RLC,data))
#define LTENR_UEMACSCHEDULER_SET(d,i,data)	(LTENR_SUBLAYER_DATA_SET(d,i,LTENR_DEVICETYPE_UE,LTENR_SUBLAYER_MACSCHEDULER,data))
#define LTENR_UEPDCP_SET(d,i,data)			(LTENR_SUBLAYER_DATA_SET(d,i,LTENR_DEVICETYPE_UE,LTENR_SUBLAYER_PDCP,data))
#define LTENR_UERRC_SET(d,i,data)			(LTENR_SUBLAYER_DATA_SET(d,i,LTENR_DEVICETYPE_UE,LTENR_SUBLAYER_RRC,data))
#define LTENR_UESDAP_SET(d,i,data)			(LTENR_SUBLAYER_DATA_SET(d,i,LTENR_DEVICETYPE_UE,LTENR_SUBLAYER_SDAP,data))
#define LTENR_GNBPHY_SET(d,i,data)			(LTENR_SUBLAYER_DATA_SET(d,i,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_PHY,data))
#define LTENR_GNBMAC_SET(d,i,data)			(LTENR_SUBLAYER_DATA_SET(d,i,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_MAC,data))
#define LTENR_GNBRLC_SET(d,i,data)			(LTENR_SUBLAYER_DATA_SET(d,i,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_RLC,data))
#define LTENR_GNBMACSCHEDULER_SET(d,i,data)	(LTENR_SUBLAYER_DATA_SET(d,i,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_MACSCHEDULER,data))
#define LTENR_GNBPDCP_SET(d,i,data)			(LTENR_SUBLAYER_DATA_SET(d,i,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_PDCP,data))
#define LTENR_GNBRRC_SET(d,i,data)			(LTENR_SUBLAYER_DATA_SET(d,i,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_RRC,data))
#define LTENR_GNBSDAP_SET(d,i,data)			(LTENR_SUBLAYER_DATA_SET(d,i,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_SDAP,data))
#define LTENR_EPC_SET(d,i,data)				(LTENR_SUBLAYER_DATA_SET(d,i,LTENR_DEVICETYPE_EPC,LTENR_SUBLAYER_EPC,data))
#define LTENR_DEVICETYPE_GET(d,i)			(LTENR_PROTODATA_GET(d,i)->deviceType)
#define LTENR_DEVICETYPE_SET(d,i,type)		(LTENR_PROTODATA_GET(d,i)->deviceType=type)
#define LTENR_GNBPDCP_ISINIT(d,i)			(LTENR_SUBLAYER_DATA_IsInitialized(d,i,LTENR_DEVICETYPE_GNB,LTENR_SUBLAYER_PDCP))
#define LTENR_UEPDCP_ISINIT(d,i)			(LTENR_SUBLAYER_DATA_IsInitialized(d,i,LTENR_DEVICETYPE_UE,LTENR_SUBLAYER_PDCP))
#define isUE(d,i)	(LTENR_DEVICETYPE_GET(d,i) == LTENR_DEVICETYPE_UE)
#define isGNB(d,i)	(LTENR_DEVICETYPE_GET(d,i) == LTENR_DEVICETYPE_GNB)
#define isEPC(d,i)	(LTENR_DEVICETYPE_GET(d,i) == LTENR_DEVICETYPE_EPC)
#define isAMF(d)	(LTENR_DEVICETYPE_GET(d,0) == LTENR_DEVICETYPE_AMF)
#define isSMF(d)	(LTENR_DEVICETYPE_GET(d,0) == LTENR_DEVICETYPE_SMF)
#define isUPF(d)	(LTENR_DEVICETYPE_GET(d,0) == LTENR_DEVICETYPE_UPF)
#define isNONRANGNB(d)	(LTENR_DEVICETYPE_GET(d,0) == LTENR_DEVICETYPE_GNB)
#define setUE(d,i)	(LTENR_DEVICETYPE_SET(d,i,LTENR_DEVICETYPE_UE))
#define setGNB(d,i)	(LTENR_DEVICETYPE_SET(d,i,LTENR_DEVICETYPE_GNB))
#define setEPC(d,i)	(LTENR_DEVICETYPE_SET(d,i,LTENR_DEVICETYPE_EPC))
#define setAMF(d)	(LTENR_DEVICETYPE_SET(d,0,LTENR_DEVICETYPE_AMF))
#define setSMF(d)	(LTENR_DEVICETYPE_SET(d,0,LTENR_DEVICETYPE_SMF))
#define setUPF(d)	(LTENR_DEVICETYPE_SET(d,0,LTENR_DEVICETYPE_UPF))
#define LTENR_PDCP_GET(d,i) (isUE(d,i)?LTENR_UEPDCP_GET(d,i):(isGNB(d,i)?LTENR_GNBPDCP_GET(d,i):NULL))
#define LTENR_RLC_GET(d,i)	(isUE(d,i)?LTENR_UERLC_GET(d,i):(isGNB(d,i)?LTENR_GNBRLC_GET(d,i):NULL))
#define LTENR_SDAP_GET(d,i)	(isUE(d,i)?LTENR_UESDAP_GET(d,i):(isGNB(d,i)?LTENR_GNBSDAP_GET(d,i):NULL))
#pragma endregion	

#pragma region INTERFERENCE_MODEL
	
	typedef enum enum_LTENR_CELL_INTERFERENCE_TYPE {
		NONE,
		LTENR_GRADED_DISTANCE_BASED_WYNER_MODEL_INTERFERENCE,
		LTENR_EXACT_GEOMETRIC_MODEL_INTERFERENCE,
		LTENR_INTERFERENCE_OVER_THERMAL_INTERFERENCE
	}LTENR_CELL_INTERFERENCEACE_TYPE;
	static const char* strLTENR_CELL_INTERFERENCEACE_TYPE[] =
	{ "NO_INTERFERENCE","GRADED_DISTANCE_BASED_WYNER_MODEL","EXACT_GEOMETRIC_MODEL","INTERFERENCE_OVER_THERMAL" };
	
	// Free the interference head for Wyner Model
	void free_interference();
#pragma endregion

#pragma region FUN_DEF_FOR_CORE_INIT
	//CORE
	void fn_NetSim_LTENR_AMF_INIT(NETSIM_ID AMFID);
	void fn_NetSim_LTENR_SMF_INIT(NETSIM_ID SMFID);
	void fn_NetSim_LTENR_UPF_INIT(NETSIM_ID UPFID);
	void fn_NetSim_LTENR_NON_RAN_GNB_INIT(NETSIM_ID GNBID);
#pragma endregion

#pragma region HELPER_APIs
	//LTE_NR Helper 
	bool isLTE_NRInterface(NETSIM_ID d, NETSIM_ID i);
	bool isLTENR_RANInterface(NETSIM_ID d, NETSIM_ID in);
	NETSIM_ID fn_NetSim_Get_LTENR_INTERFACE_ID_FROM_DEVICE_ID(NETSIM_ID r);
	bool fn_NetSim_LTENR_IS_S1_INTERFACE_EXISTS(NETSIM_ID d);
	NETSIM_ID fn_NetSim_LTENR_CORE_INTERFACE(NETSIM_ID d, LTENR_INTERFACE_TYPE type);
	bool fn_NetSim_isDeviceLTENR(NETSIM_ID r, NETSIM_ID rin);
	NETSIM_ID LTENR_HELPER_Get_connected_interface(NETSIM_ID d, NETSIM_ID r);
	NETSIM_ID fn_NetSim_LTENR_INTERFACE_ID_FROM_CONNECTED_DEVICE(NETSIM_ID d, NETSIM_ID in, NETSIM_ID r);
	NETSIM_ID fn_NetSim_FIND_LTENR_INTERFACE(NETSIM_ID r);
	NETSIM_ID fn_NetSim_FIND_LTE_INTERFACE(NETSIM_ID r);
	NETSIM_ID fn_NetSim_LTENR_SEC_INTEFACE_FOR_UE(NETSIM_ID d, NETSIM_ID in);
	NETSIM_ID fn_NetSim_LTENR_MASTER_INTEFACE_FOR_UE(NETSIM_ID d, NETSIM_ID in);
	bool fn_NetSim_LTENR_NSA_IS_CORE_SPLIT_EXISTS(NETSIM_ID d, NETSIM_ID in);
	bool fn_NetSim_LTENR_NSA_IS_OPTION_X_EXISTS(NETSIM_ID d, NETSIM_ID in);
	bool LTENR_GET_PACKET_FROM_ACCESS_BUFFER();
	void fn_NetSim_LTENR_SNED_TO_XN_INTERFACE(NetSim_PACKET* packet, NETSIM_ID d, NETSIM_ID in,
		NETSIM_ID r, NETSIM_ID rin, char* gateway, char* nexthop, bool flag);
#pragma endregion	

#pragma region EPC_APIs
	//LTE-NR EPC interface function
	void LTENR_EPC_ASSOCIATION(NETSIM_ID epcId, NETSIM_ID epcIf,
							   NETSIM_ID gnbId, NETSIM_ID gnbIf,
							   NETSIM_ID ueId, NETSIM_ID ueIf);
	void LTENR_EPC_ALLOC(NETSIM_ID d, NETSIM_ID in);
	void LTENR_GNB_SETEPC(NETSIM_ID d, NETSIM_ID in,
						  NETSIM_ID* e, NETSIM_ID* ein);
#pragma endregion

#pragma region FUN_DEF_FOR_INTI_METRICS_APIs
	//LTENR Interference 
	void LTENR_fn_InterferenceInit();
	//LTENR CORE
	void fn_NetSim_LTENR_CORE_Init();
	//LTE-NR NAS 
	void fn_NetSim_LTENR_NAS_Init();

	//LTE-NR SDAP interface function
	void fn_NetSim_LTENR_SDAP_Init();
	void fn_NetSim_LTENR_UESDAP_Init(NETSIM_ID ueId, NETSIM_ID ueIf);
	void fn_NetSim_LTENR_GNBSDAP_Init(NETSIM_ID gnbId, NETSIM_ID gnbIf);
	int fn_NetSim_LTENR_SDAP_Metrics_F(PMETRICSWRITER metricsWriter);
	int fn_NetSim_LTENR_Cell_Metrics_F(PMETRICSWRITER metricsWriter);

	// LTE-NR RRC interface function
	void fn_NetSim_LTENR_RRC_Init();
	void fn_NetSim_LTENR_UERRC_Init(NETSIM_ID ueId,
									NETSIM_ID ueInterface);
	void fn_NetSim_LTENR_GNBRRC_Init(NETSIM_ID gnbId,
									 NETSIM_ID gnbInterface);
	bool fn_NetSim_LTENR_RRC_isConnected(NETSIM_ID d, NETSIM_ID in,
										 NETSIM_ID r, NETSIM_ID rin);
	bool fn_NetSim_LTENR_RRC_isActive(NETSIM_ID d, NETSIM_ID in,
		NETSIM_ID r, NETSIM_ID rin);

	//LTE-NR RRC Conditiona Handover TTT function
	void fn_NetSim_LTENR_Conditional_HO_TTT_Matrix_Add(NETSIM_ID gnbId, NETSIM_ID gnbIf,
		NETSIM_ID ueId, NETSIM_ID ueIf);
	void fn_NetSim_LTENR_Conditional_HO_TTT_Matrix_Remove(NETSIM_ID gnbId, NETSIM_ID gnbIf,
		NETSIM_ID ueId, NETSIM_ID ueIf);
	double fn_NetSim_LTENR_Conditional_HO_TTT_Matrix_Find(void* list,
		NETSIM_ID ueID, NETSIM_ID targetID);
	void fn_NetSim_LTENR_Conditional_HO_TTT_Matrix_Set(void* list,
		NETSIM_ID ueID, NETSIM_ID targetID, double value);
	void fn_NetSim_LTENR_Conditional_HO_TTT_Matrix_Free(NETSIM_ID gnbId, NETSIM_ID gnbIf);

	//LTE-NR PDCP interface function
	void fn_NetSim_LTENR_PDCP_Init();
	void fn_NetSim_LTENR_UEPDCP_Init(NETSIM_ID ueId,
									 NETSIM_ID ueIf);
	void fn_NetSim_LTENR_GNBPDCP_Init(NETSIM_ID gnbId,
									  NETSIM_ID gnbIf);
	
	//LTE-NR RLC interface function
	void fn_NetSim_LTENR_RLC_Init();
	void fn_NetSim_LTENR_UERLC_Init(NETSIM_ID ueId, NETSIM_ID ueIf);
	void fn_NetSim_LTENR_GNBRLC_Init(NETSIM_ID gnbId, NETSIM_ID gnbIf);
	UINT fn_NetSim_LTENR_RLC_BufferStatusNotificaton(NETSIM_ID d, NETSIM_ID in,
													 NETSIM_ID r, NETSIM_ID rin,
													 LTENR_LOGICALCHANNEL logicalChannel);
	void fn_NetSim_LTENR_RLC_TransmissionStatusNotification(NETSIM_ID d, NETSIM_ID in,
															NETSIM_ID r, NETSIM_ID rin,
															UINT size, LTENR_LOGICALCHANNEL logicalChannel);
	// LTE-NR MAC Scheduler

	//LTE-NR Multiplexer
	int LTENR_Mutiplexer_LDPC_SelectBaseGraph(double TBS, double R);
	void LTENR_Multiplexer_ComputeCodeBlockSize(int LDPCG, UINT TBS, UINT* C, UINT* cbs, UINT* cbs_);

	//LTE-NR MAC interface function
	void fn_NetSim_LTENR_MAC_Init();
	void fn_NetSim_LTENR_GNBMAC_Init(NETSIM_ID gnbId, NETSIM_ID gnbIf);
	void fn_NetSim_LTENR_UEMAC_Init(NETSIM_ID ueId, NETSIM_ID ueIf);
	void LTENR_NotifyMACForStartingSlot();

	//LTE-NR PHY interface function
	void fn_NetSim_LTENR_GNBPHY_Init(NETSIM_ID gnbId, NETSIM_ID gnbIf);
	void fn_NetSim_LTENR_UEPHY_Init(NETSIM_ID ueId, NETSIM_ID ueIf);
	void fn_NetSim_LTENR_PHY_Init();
	double LTENR_PHY_GetSlotEndTime(NETSIM_ID d, NETSIM_ID in);
	void fn_NetSim_LTENR_HandlePhyOut();

	//BLER Model
	void LTENR_FreeBLERTable();
	double LTENR_GetBLER(int tableId, int mcsIndex, int bg, int cbs, double sinr);
	bool isBLEREnable(NETSIM_ID d, NETSIM_ID in);

	//LTE-NR Radio Measurements log
	void LTENR_RadioMeasurements_Init();
	void LTENR_RadioMeasurements_Finish();
	void LTENR_RadioMeasurements_PDSCH_Log(ptrLTENR_GNBPHY phy, int CA_ID,
		ptrLTENR_ASSOCIATEDUEPHYINFO info);
	void LTENR_RadioMeasurements_PUSCH_Log(ptrLTENR_GNBPHY phy, int CA_ID,
		ptrLTENR_ASSOCIATEDUEPHYINFO info);
	void LTENR_RadioMeasurements_PBSCH_Log(ptrLTENR_GNBPHY phy, int CA_ID,
		ptrLTENR_ASSOCIATEDUEPHYINFO info);

	//LTENR OLLA log
	void LTENR_OLLA_Log_Init();
	void LTENR_OLLA_Log_Finish();

	//LTE-NR MSG
	void LTENR_MSG_WriteTrace(NetSim_PACKET* packet, char* s);
	void LTENR_MSG_WriteTrace_APP(NetSim_PACKET* packet, char* s);
	//DC
	void fn_NetSim_LTENR_NSA_SN_HANDOVER_INIT();
#define isLTENRControlPacket(packet) ((packet)->nPacketType == PacketType_Control && ((packet)->nControlDataType/100 == MAC_PROTOCOL_LTE_NR || (packet)->nControlDataType/100 == MAC_PROTOCOL_LTE))
#pragma endregion

#pragma region FUN_DEF_LOGs_AND_APIs
	//LTE-NR log
	void fn_NetSim_LTE_MAC_PRBLOG();
	void close_ltenr_PRB_log();
	bool get_ltenr_log_status();
	void print_ltenr_log(char* format, ...);
	void print_ltenr_packet_log(NETSIM_ID d, NETSIM_ID in,
								LTENR_SUBLAYER layer, NetSim_PACKET* packet,
								bool isOut);
	void fn_NetSim_LTE_HandoverLOG();
	void close_ltenr_handover_log();

	//LTE-NR Function def
	void fn_NetSim_LTENR_EPC_RouteHOPacket();
	void fn_NetSim_PHY_MOBILITY_HANDLE(NETSIM_ID d);

#define LTENR_SDAP_PRINT_PACKET_LOG_OUT()	print_ltenr_packet_log(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId,LTENR_SUBLAYER_SDAP,pstruEventDetails->pPacket,true)
#define LTENR_SDAP_PRINT_PACKET_LOG_IN()	print_ltenr_packet_log(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId,LTENR_SUBLAYER_SDAP,pstruEventDetails->pPacket,false)
#define LTENR_RRC_PRINT_PACKET_LOG_OUT()	print_ltenr_packet_log(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId,LTENR_SUBLAYER_RRC,pstruEventDetails->pPacket,true)
#define LTENR_RRC_PRINT_PACKET_LOG_IN()		print_ltenr_packet_log(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId,LTENR_SUBLAYER_RRC,pstruEventDetails->pPacket,false)
#define LTENR_PDCP_PRINT_PACKET_LOG_OUT()	print_ltenr_packet_log(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId,LTENR_SUBLAYER_PDCP,pstruEventDetails->pPacket,true)
#define LTENR_PDCP_PRINT_PACKET_LOG_IN()	print_ltenr_packet_log(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId,LTENR_SUBLAYER_PDCP,pstruEventDetails->pPacket,false)
#define LTENR_RLC_PRINT_PACKET_LOG_OUT()	print_ltenr_packet_log(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId,LTENR_SUBLAYER_RLC,pstruEventDetails->pPacket,true)
#define LTENR_RLC_PRINT_PACKET_LOG_IN()		print_ltenr_packet_log(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId,LTENR_SUBLAYER_RLC,pstruEventDetails->pPacket,false)
#define LTENR_MAC_PRINT_PACKET_LOG_OUT()	print_ltenr_packet_log(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId,LTENR_SUBLAYER_MAC,pstruEventDetails->pPacket,true)
#define LTENR_MAC_PRINT_PACKET_LOG_IN()		print_ltenr_packet_log(pstruEventDetails->nDeviceId,pstruEventDetails->nInterfaceId,LTENR_SUBLAYER_MAC,pstruEventDetails->pPacket,false)
#pragma endregion

#pragma region GLOBAL_VARIABLE_AND_UITILITY
	//CA
#define MAX_CA_COUNT (16*2) /*For TDD - 16
							* For FDD - 16 *(1 for DL & one for UL
							*/
//BWP
#define MAX_BWP_COUNT (4) 
#define MAX_LAYER_COUNT (16) 


	//LTENR-Utility
	double str_to_time(char* time);
	UINT str_to_bytes(char* bytes);
#define CompareVal(arg0, arg1, op) \
( \
	(!_stricmp(op, ">"))	?	(arg0 > arg1)	:( \
	(!_stricmp(op, ">="))	?	(arg0 >= arg1)	:(\
	(!_stricmp(op, "<"))	?	(arg0 < arg1)	:(\
	(!_stricmp(op, "<="))	?	(arg0 <= arg1)	:(\
	(!_stricmp(op, "=="))	?	(arg0 == arg1)	:(\
	(!_stricmp(op, "!="))	?	(arg0 != arg1)	:false \
))))))
#pragma endregion

#pragma region PACKET_APIs
	//Core packet
	void LTENR_Core_Send_Packet(NETSIM_ID src, NETSIM_ID dest,
								NETSIM_IPAddress srcIP, NETSIM_IPAddress destIP,
								void* hdr, UINT size,
								NETSIM_ID packetType, char* szPacketType, NETSIM_ID type);
	void LTENR_Handover_Send_Packet(NETSIM_ID src, NETSIM_ID dest,
		NETSIM_IPAddress srcIP, NETSIM_IPAddress destIP,
		void* hdr, UINT size,
		NETSIM_ID packetType, char* szPacketType, NETSIM_ID type);
	

	//GNB core
	void gNB_N3_Forward_packet_to_RAN(NETSIM_ID d, NETSIM_ID in, NETSIM_ID rin, NetSim_PACKET* packet);
	void gNB_N3_Forward_packet_to_MAC(NETSIM_ID d, NETSIM_ID rin, NetSim_PACKET* packet);
	void fn_NetSim_LTENR_LTE_S1_CONTROL_PACKET_TO_RAN(NETSIM_ID d, NETSIM_ID RANIF, NetSim_PACKET* packet);
#pragma endregion

#pragma region DEBUGGER_FUN
	//debugger function. use when necessary.
	bool ltenr_validate_list(void* ls, UINT size, UINT next, char* file, UINT line);

	//Fast Performance
	void set_fast_performance();
	void fn_NetSim_LTENR_GNB_XN_INTERFACE_INIT();
#pragma endregion

#pragma region CELL_LIST

	typedef struct stru_LTENR_Cell_List {
		NETSIM_ID d;
		NETSIM_ID in;
		_ptr_ele ele;
	}LTENR_Cell_List, *ptrLTENR_Cell_List;
	ptrLTENR_Cell_List gnbDCList;
	ptrLTENR_Cell_List enbDCList;
#define LTENR_Cell_List_ALLOC()			(list_alloc(sizeof(LTENR_Cell_List),offsetof(LTENR_Cell_List,ele)))
#define LTENR_Cell_List_NEXT(info)	(LIST_NEXT(info))
#define LTENR_Cell_List_ADD(info,e)		(LIST_ADD_LAST(&(info),e))
#define LTENR_Cell_List_REMOVE(info,e)	(LIST_REMOVE(&(info),e)) 
#pragma endregion

#pragma region DC_SN_FUN
	void fn_NetSim_LTENR_NSA_RECV();
	void fn_NetSim_SN_HANDOVER_PACKET_HANDLER();
	void LTENR_NSA_XN_INTERFACE_TO_RAN(NETSIM_ID d, NETSIM_ID in, NetSim_PACKET* packet);
	NETSIM_ID fn_NetSim_LTENR_NSA_Splitting_Alogrithm_For_EPC(NETSIM_ID dest);
#pragma endregion

#pragma region XML_CONFIGURATION
	int LTENR_ConvertStrToEnumWithErrorMsg(const char** enumstr, const char* str, int length, char* name);
	#define LTENR_ConvertStrToEnum(enumName,val) LTENR_ConvertStrToEnumWithErrorMsg(str ## enumName, val, sizeof(str ## enumName)/sizeof(str ## enumName ## [0]), #enumName)
#pragma endregion

#pragma region GUI_PROTOCOL_LOG_READER
	bool get_protocol_log_status(char* logname);
#pragma endregion

	//OLLA
	void LTENR_OLLA_MarkTBSError(NETSIM_ID gNBId, NETSIM_ID gNBIf,
		NETSIM_ID ueId, NETSIM_ID ueIf, bool isUplink,
		UINT caId, UINT layerId);
	void LTENR_OLLA_MarkTBSSuccess(NETSIM_ID gNBId, NETSIM_ID gNBIf,
		NETSIM_ID ueId, NETSIM_ID ueIf, bool isUplink,
		UINT caId, UINT layerId);
	double LTENR_OLLA_GetDeltaSINR(NETSIM_ID gNBId, NETSIM_ID gNBIf,
		NETSIM_ID ueId, NETSIM_ID ueIf, bool isUplink,
		UINT caId, UINT layerId);
	void test_olla(NETSIM_ID gNBId, NETSIM_ID gNBIf, NETSIM_ID ueId, NETSIM_ID ueIf);
#ifdef  __cplusplus
}
#endif
#endif /* _NETSIM_LTE_NR_H_ */
