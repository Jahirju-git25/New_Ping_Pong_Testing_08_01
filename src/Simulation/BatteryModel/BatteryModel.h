#pragma once
/************************************************************************************
* Copyright (C) 2023                                                               *
* TETCOS, Bangalore. India                                                         *
*                                                                                  *
* Tetcos owns the intellectual property rights in the Product and its content.     *
* The copying, redistribution, reselling or publication of any or all of the       *
* Product or its content without express prior written consent of Tetcos is        *
* prohibited. Ownership and / or any other right relating to the software and all  *
* intellectual property rights therein shall remain at all times with Tetcos.      *
*                                                                                  *
* Author:    Shashi Kant Suman                                                     *
*                                                                                  *
* ---------------------------------------------------------------------------------*/

#ifndef _NETSIM_BATTERY_MODEL_H_
#define _NETSIM_BATTERY_MODEL_H_
#ifdef  __cplusplus
extern "C" {
#endif
typedef struct stru_mode
{
	int mode;
	double current;
	double consumedEnergy;
	char* heading;
}BATTERYMODE, * ptrBATTERYMODE;

typedef struct stru_battery
{
	NETSIM_ID deviceId;
	NETSIM_ID interfaceId;
	double initialEnergy;
	double consumedEnergy;
	double remainingEnergy;
	double voltage;
	double rechargingCurrent;
	int modeCount;
	double rechargingEnergy;
	double SumSleepEnergy;
	double SumTransmitEnergy;
	double SumConsumedEnergy;
	double SumIdleEnergy;
	double SumReceiveEnergy;
	double SumHarvestedEnergy;
	ptrBATTERYMODE mode;

	int currentMode;
	double modeChangedTime;

	void* animHandle;
	struct stru_battery* next;
}BATTERY, * ptrBATTERY;

#ifndef _BATTERY_MODEL_CODE_
#pragma comment(lib,"BatteryModel.lib")
	//typedef void* ptrBATTERY;
#endif

	_declspec(dllexport) ptrBATTERY battery_find(NETSIM_ID d,
												 NETSIM_ID in);
	_declspec(dllexport) void battery_add_new_mode(ptrBATTERY battery,
												   int mode,
												   double current,
												   char* heading);
	_declspec(dllexport) ptrBATTERY battery_init_new(NETSIM_ID deviceId,
													 NETSIM_ID interfaceId,
													 double initialEnergy,
													 double voltage,
													 double dRechargingCurrent);
	_declspec(dllexport) bool battery_set_mode(ptrBATTERY battery,
											   int mode,
											   double time);
	_declspec(dllexport) void battery_animation();
	_declspec(dllexport) void battery_metrics(PMETRICSWRITER metricsWriter);
	_declspec(dllexport) double battery_get_remaining_energy(ptrBATTERY battery);
	_declspec(dllexport) double battery_get_consumed_energy(ptrBATTERY battery, int mode);

	// Battery Logs
	void init_battery_log(ptrBATTERY battery);
	void close_battery_log();
	bool get_protocol_log_status(char* logname);
	void  Battery_RadioMeasurements_Log(ptrBATTERY battery, double harvestedEnergy, double ConsumedEnergy, int prev_mode, int curr_mode, double mode_change_time, ptrBATTERYMODE pm);
	
#ifdef  __cplusplus
}
#endif
#endif //_NETSIM_BATTERY_MODEL_H_