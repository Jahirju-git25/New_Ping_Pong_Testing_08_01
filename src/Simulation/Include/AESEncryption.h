#pragma once
#ifndef _AESENCRYPTION_H_
#define _AESENCRYPTION_H_
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#ifdef _WIN32
#include <Windows.h>
#include <sal.h>
#endif
#include <time.h>
#include <stdint.h>


#ifdef  __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#ifndef _AES_CODE_
#pragma comment(lib,"AESEncryption.lib")
#define _AES_EXPORT_ _declspec(dllimport)
#else
#define _AES_EXPORT_ _declspec(dllexport)
#endif
#endif
#pragma region FILEENCRYPTION
#ifndef _AES_CODE_
typedef void* ptrFILEENCRYPTIONHANDLE;
#else
typedef struct stru_file_encryption_handle FILEENCRYPTIONHANDLE, * ptrFILEENCRYPTIONHANDLE;
#endif

_Check_return_
_AES_EXPORT_ ptrFILEENCRYPTIONHANDLE __cdecl open_encrypted_file(
	_In_z_ char const* _FileName
);

_Success_(return != -1)
_Check_return_opt_
_AES_EXPORT_ int __cdecl close_encrypted_file(
	_In_ _Post_invalid_ ptrFILEENCRYPTIONHANDLE _Stream
);

_Success_(return == _Buffer)
_Check_return_opt_
_AES_EXPORT_ char* __cdecl read_line_from_encrypted_file(
	_Out_writes_z_(_MaxCount) char* _Buffer,
	_In_                      int   _MaxCount,
	_Inout_                   ptrFILEENCRYPTIONHANDLE _Stream
);

_Success_(return == 0)
_Check_return_opt_
_AES_EXPORT_ int encrypt_file(
	_Inout_z_	const char* file,
	_In_		const char* keyId
);

#pragma endregion //FILEENCRPTION

#ifdef  __cplusplus
}
#endif
#endif //_AESENCRYPTION_H_