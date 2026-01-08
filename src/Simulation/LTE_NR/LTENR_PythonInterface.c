#include "stdafx.h"
#include "LTENR_MAC.h"
#include "LTENR_PHY.h"
#include "Stack.h"

static FILE* fpPythonlog = NULL;

void init_Python_log()
{

	char s[BUFSIZ];
	sprintf(s, "%s\\%s", pszIOLogPath, "LTENR_Python_Log.csv");
	fpPythonlog = fopen(s, "w");
	if (!fpPythonlog)
	{
		fnSystemError("Unable to open %s file", s);
		perror(s);
	}
	else
	{
		fprintf(fpPythonlog, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s", "Time","TxPower_1","TxPower_2","TxPower_3", "deltaP_1", "deltaP_2", "deltaP_3", "Beamforming_1", "Beamforming_2", "Beamforming_3", "Beamforming_4", "Beamforming_5", "Beamforming_6", "Initial_SINR_1", "Final_SINR_1", "Initial_SINR_2", "Final_SINR_2", "Initial_SINR_3", "Final_SINR_3", "Initial_SINR_4", "Final_SINR_4", "Initial_SINR_5", "Final_SINR_5", "Initial_SINR_6", "Final_SINR_6","Throughput_1", "Throughput_2", "Throughput_3", "Throughput_4", "Throughput_5", "Throughput_6");
		fprintf(fpPythonlog, "\n");
		if (nDbgFlag) fflush(fpPythonlog);
	}

}

void close_Python_log()
{
	if (fpPythonlog)
		fclose(fpPythonlog);
}

void LTENR_Python_Log(char* format, ...)
{
	if (fpPythonlog == NULL) return;

	va_list l;
	va_start(l, format);
	vfprintf(fpPythonlog, format, l);

	fprintf(fpPythonlog, "\n");
	if (nDbgFlag) fflush(fpPythonlog);

}