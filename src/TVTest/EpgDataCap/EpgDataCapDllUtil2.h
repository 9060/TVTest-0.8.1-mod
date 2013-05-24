#ifndef __EPG_DATA_CAP_DLL_UTIL_2_H__
#define __EPG_DATA_CAP_DLL_UTIL_2_H__

#include "EpgDataCap2.h"

class CEpgDataCapDllUtil2
{
public:
	CEpgDataCapDllUtil2(void);
	~CEpgDataCapDllUtil2(void);

	//DLL�̏�����
	//�߂�l�F�G���[�R�[�h
	DWORD Initialize(
		LPCTSTR pszFileName, //[IN] �t�@�C���p�X(by HDUSTest�̒��̐l)
		BOOL bAsyncMode //[IN]TRUE:�񓯊����[�h�AFALSE:�������[�h
		);

	//DLL�̊J��
	//�߂�l�F�Ȃ�
	void UnInitialize(
		);

	//���[�h�ς݂��擾(by HDUSTest�̒��̐l)
	bool IsLoaded() const;

	//��͑Ώۂ�TS�p�P�b�g��ǂݍ��܂���
	//�߂�l�F�G���[�R�[�h
	DWORD AddTSPacket(
		BYTE* pbData, //[IN] TS�p�P�b�g
		DWORD dwSize //[IN] pbData�̃T�C�Y�i188�̔{���j
		);

	//��̓f�[�^�̃N���A
	//�߂�l�F�Ȃ�
	void Clear(
		);

	//PF�f�[�^�̎擾
	//�߂�l�F�G���[�R�[�h
	DWORD GetPFData(
		WORD wSID, //[IN] �ԑg���擾����SID
		EPG_DATA_INFO** pstEpgData, //[OUT] �ԑg���i����GetPFDataEP�ĂԂ܂ŗL���j
		BOOL bNextData //[IN] TRUE:���̔ԑg���AFALSE:���݂̔ԑg���
		);

	//���݂̔ԑg��EventID���擾
	//�߂�l�FEventID�i-1�ł܂���͂ł���p�P�b�g�������Ă��Ȃ��j
	int GetNowEventID(
		WORD wSID //[IN] �擾����SID
		);

	//EPG��̓��[�h�̐ݒ�
	//�߂�l�F�Ȃ�
	void SetPFOnly(
		BOOL bPFOnly //[IN] TRUE:PF�f�[�^�̂݉�́AFALSE:1�T�ԕ��̃f�[�^�����
		);

	//EPG��̓��[�h�̐ݒ�
	//�߂�l�F�Ȃ�
	void SetBasicMode(
		BOOL bBasicOnly //[IN] TRUE:��{���݂̂Œ~�ϔ���AFALSE:�g�������~�ϔ���
		);

	//���݂̃X�g���[����TSID���擾
	//�߂�l�FTSID�i-1�ł܂���͂ł���p�P�b�g�������Ă��Ȃ��j
	int GetTSID(
		);

	//���݂̃X�g���[����PMT��PID���X�g���擾
	//�߂�l�F�Ȃ�
	DWORD GetPmtPIDList(
		DWORD** pdwPIDList, //[OUT] PMT��PID���X�g�i����GetPmtPIDListEP�ĂԂ܂ŗL���j
		DWORD* dwListCount, //[OUT] pdwPIDList�̌�
		DWORD* dwVersion //[OUT] PAT�̃o�[�W����
		);

	//���݂̃X�g���[���̎w��SID��PMT��PID���擾
	//�߂�l�FPMT��PID�i-1�ł܂���͂ł���p�P�b�g�������Ă��Ȃ��j
	int GetPmtPID(
		DWORD dwSID, //[IN] ServiceID
		DWORD* dwVersion //[OUT] PAT�̃o�[�W����
		);

	//���݂̃X�g���[���̎w��SID��PMT��PID���擾
	//�߂�l�F�G���[�R�[�h
	DWORD GetESInfo(
		DWORD dwSID, //[IN] ServiceID
		DWORD* pdwPcrPID, //[OUT] PCR��PID
		DWORD* pdwEcmPID, //[OUT] ���Y�T�[�r�X�S�̂�ECM��PID ���݂��Ȃ��ꍇ��0x1FFF ES�Ōʂ̏ꍇ��ELEMENT_INFO���̂��̂�
		ELEMENT_INFO** pstESList, //[OUT]ES���̃��X�g�i����GetESInfoEP�ĂԂ܂ŗL���j
		DWORD* dwListCount, //[OUT] pstESList�̌�
		DWORD* dwVersion //[OUT] PMT�̃o�[�W����
		);

	//���݂̃X�g���[����EMM��PID���擾
	//�߂�l�FEMM��PID�i-1�ł܂���͂ł���p�P�b�g�������Ă��Ȃ��j
	int GetEmmPID(
		DWORD* dwVersion //[OUT] CAT�̃o�[�W����
		);

	//���݂̃X�g���[���̃T�[�r�XID���X�g���擾
	//�߂�l�F�G���[�R�[�h
	DWORD GetServiceList(
		SERVICE_INFO** pstList, //[OUT] �T�[�r�X�ꗗ�i����GetServiceList or GetServiceListDB�ĂԂ܂ŗL���j
		DWORD* dwListSize //[OUT] pstList�̌�
		);

	//���݂̃X�g���[�����ōŌ�ɉ�͂ł������Ԃ��擾
	//�߂�l�FFILETIME��__int64�ɃV�t�g�������́i-1�ł܂���͂ł���p�P�b�g�������Ă��Ȃ��j
	__int64 GetNowTime(
		);

	//�n�f�W�̏ꍇ�A���݂̃X�g���[���̕����ǖ�
	//�߂�l�FNULL�ł܂���͂ł���p�P�b�g�������Ă��Ȃ� or BS/CS
	const WCHAR* GetTsName(
		);

	//�~�ύς݂̃T�[�r�XID���X�g���擾
	//�߂�l�F�G���[�R�[�h
	DWORD GetServiceListDB(
		SERVICE_INFO** pstList, //[OUT] �T�[�r�X�ꗗ�i����GetServiceList or GetServiceListDB�ĂԂ܂ŗL���j
		DWORD* dwListSize //[OUT] pstList�̌�
		);

	//�~�ύς݂̔ԑg�����擾
	//�߂�l�FTRUE:�����AFALSE:���s
	BOOL GetEpgDataListDB(
		DWORD dwONID, //[IN] �擾�������T�[�r�X��OriginalNetworkID
		DWORD dwTSID, //[IN] �擾�������T�[�r�X��TransportStreamID
		DWORD dwSID, //[IN] �擾�������T�[�r�X��ServiceID
		EPG_DATA_INFO2** pstList, //[OUT] �ԑg���ꗗ�i����GetEpgDataListDBEP�ĂԂ܂ŗL���j
		DWORD* dwListSize //[OUT] pstList�̌�
		);

	//�~�ύς݂̔ԑg��񂩂�w��EventID�̔ԑg�����擾
	//�߂�l�FTRUE:�����AFALSE:���s
	BOOL GetEpgDataDB(
		DWORD dwONID, //[IN] �擾�������T�[�r�X��OriginalNetworkID
		DWORD dwTSID, //[IN] �擾�������T�[�r�X��TransportStreamID
		DWORD dwSID, //[IN] �擾�������T�[�r�X��ServiceID
		DWORD dwEID, //[IN] �擾�������T�[�r�X��EventID
		EPG_DATA_INFO2** pstEpgData //[OUT] �ԑg���i����GetEpgDataDBEP�ĂԂ܂ŗL���j
		);

	//�~�ύς݂̔ԑg�����擾
	//�߂�l�FTRUE:�����AFALSE:���s
	BOOL GetEpgDataListDB2(
		DWORD dwONID, //[IN] �擾�������T�[�r�X��OriginalNetworkID
		DWORD dwTSID, //[IN] �擾�������T�[�r�X��TransportStreamID
		DWORD dwSID, //[IN] �擾�������T�[�r�X��ServiceID
		EPG_DATA_INFO3** pstList, //[OUT] �ԑg���ꗗ�i����GetEpgDataListDB2EP�ĂԂ܂ŗL���j
		DWORD* dwListSize //[OUT] pstList�̌�
		);

	//�~�ύς݂̔ԑg��񂩂�w��EventID�̔ԑg�����擾
	//�߂�l�FTRUE:�����AFALSE:���s
	BOOL GetEpgDataDB2(
		DWORD dwONID, //[IN] �擾�������T�[�r�X��OriginalNetworkID
		DWORD dwTSID, //[IN] �擾�������T�[�r�X��TransportStreamID
		DWORD dwSID, //[IN] �擾�������T�[�r�X��ServiceID
		DWORD dwEID, //[IN] �擾�������T�[�r�X��EventID
		EPG_DATA_INFO3** pstEpgData //[OUT] �ԑg���i����GetEpgDataDB2EP�ĂԂ܂ŗL���j
		);

	//PF�f�[�^�̎擾
	//�߂�l�F�G���[�R�[�h
	DWORD GetPFData2(
		WORD wSID, //[IN] �ԑg���擾����SID
		EPG_DATA_INFO3** pstEpgData, //[OUT] �ԑg���i����GetPFData2EP�ĂԂ܂ŗL���j
		BOOL bNextData //[IN] TRUE:���̔ԑg���AFALSE:���݂̔ԑg���
		);

	//�~�ύς݂̔ԑg���̃N���A
	//�߂�l�F�Ȃ�
	void ClearDB(
		);
protected:
	HMODULE m_hModule;
	int m_iID;

	InitializeEP pfnInitializeEP;
	UnInitializeEP pfnUnInitializeEP;
	AddTSPacketEP pfnAddTSPacketEP;
	ClearEP pfnClearEP;
	GetPFDataEP pfnGetPFDataEP;
	SetPFOnlyEP pfnSetPFOnlyEP;
	SetBasicModeEP pfnSetBasicModeEP;
	GetTSIDEP pfnGetTSIDEP;
	GetNowTimeEP pfnGetNowTimeEP;
	GetTsNameEP pfnGetTsNameEP;
	GetServiceListEP pfnGetServiceList;
	GetPmtPIDListEP pfnGetPmtPIDList;
	GetServiceListDBEP pfnGetServiceListDB;
	GetEpgDataListDBEP pfnGetEpgDataListDB;
	GetPmtPIDEP pfnGetPmtPID;
	GetESInfoEP pfnGetESInfo;
	GetEmmPIDEP pfnGetEmmPID;
	GetNowEventIDEP pfnGetNowEventID;
	GetEpgDataDBEP pfnGetEpgDataDB;
	ClearDBEP pfnClearDB;
	GetEpgDataListDB2EP pfnGetEpgDataListDB2;
	GetEpgDataDB2EP pfnGetEpgDataDB2;
	GetPFData2EP pfnGetPFData2;


protected:
	BOOL LoadDll(LPCTSTR pszFileName);
	BOOL UnLoadDll(void);
};

#endif
