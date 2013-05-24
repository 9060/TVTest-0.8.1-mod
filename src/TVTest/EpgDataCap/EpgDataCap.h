#ifndef __EPG_DATA_CAP_H__
#define __EPG_DATA_CAP_H__

#include "EpgDataCapDef.h"

//DLL�̏�����
//�߂�l�F�G���[�R�[�h
typedef DWORD (WINAPI *InitializeEP)(
	BOOL bAsyncMode
	);

//DLL�̊J��
//�߂�l�F�G���[�R�[�h
typedef DWORD (WINAPI *UnInitializeEP)();

//TS�p�P�b�g��ǂݍ��܂���
//�߂�l�F�G���[�R�[�h
typedef DWORD (WINAPI *AddTSPacketEP)(
	BYTE* pbData,
	DWORD dwSize
	);

//PF�f�[�^�̎擾
//�߂�l�F�G���[�R�[�h
typedef DWORD (WINAPI *GetPFDataEP)(
	WORD wSID,
	EPG_DATA_INFO* pstEpgData,
	BOOL bNextData
	);

//PF�f�[�^�̎擾�Ɏg�p�����������̊J��
//�߂�l�F�G���[�R�[�h
typedef DWORD (WINAPI *ReleasePFDataEP)(
	EPG_DATA_INFO* pstEpgData
	);

typedef void (WINAPI *ClearDataEP)(
	);

typedef void (WINAPI *SetPFOnlyEP)(
	BOOL bPFOnly
	);

typedef void (WINAPI *SetBasicModeEP)(
	BOOL bBasicOnly
	);

typedef int (WINAPI *GetTSIDEP)();

typedef __int64 (WINAPI *GetNowTimeEP)();

typedef DWORD (WINAPI *GetEpgCapStatusEP)();

typedef int (WINAPI *GetDefSIDEP)();

typedef const WCHAR* (WINAPI *GetTsNameEP)();

typedef DWORD (WINAPI *GetServiceListEP)(
	SERVICE_INFO** pstList,
	DWORD* dwListSize
	);

typedef void (WINAPI *ReleaseServiceListEP)();

typedef void (WINAPI *ClearBuffEP)();

typedef int (WINAPI *GetPmtPIDListEP)(DWORD** dwPIDList, DWORD* dwListCount, DWORD* dwVersion);

typedef int (WINAPI *GetPCRPIDListEP)(DWORD** dwPIDList, DWORD* dwListCount, DWORD* dwVersion);

typedef int (WINAPI *GetElementIDEP)(DWORD dwSID, DWORD** dwPIDList, DWORD* dwListCount, DWORD* dwVersion);

typedef int (WINAPI *GetPCRPIDEP)(DWORD dwSID, DWORD* dwPID, DWORD* dwVersion);

typedef int (WINAPI *GetPmtPIDEP)(DWORD dwSID, DWORD* dwPID, DWORD* dwVersion);

typedef int (WINAPI *GetEmmPIDEP)();

typedef int (WINAPI *GetNowEventIDEP)(DWORD dwSID);

typedef BOOL (WINAPI *GetServiceListDBEP)(SERVICE_INFO** pstList, DWORD* dwListSizeD);

typedef BOOL (WINAPI *GetEpgDataListDBEP)(DWORD dwONID, DWORD dwTSID, DWORD dwSID, EPG_DATA_INFO2** pstList, DWORD* dwListSize);

typedef int (WINAPI *GetLowQualityElementIDEP)(DWORD dwSID, DWORD** dwPIDList, DWORD* dwListCount, DWORD* dwVersion);

#endif