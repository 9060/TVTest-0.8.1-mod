#ifndef __EPG_DATA_CAP_2_H__
#define __EPG_DATA_CAP_2_H__

#include "EpgBonCommonDef.h"

//DLL�̏�����
//�߂�l�F����ID�i-1�ŃG���[�j
typedef int (WINAPI *InitializeEP)(
	BOOL bAsyncMode //[IN]TRUE:�񓯊����[�h�AFALSE:�������[�h
	);

//DLL�̊J��
//�߂�l�F�G���[�R�[�h
typedef DWORD (WINAPI *UnInitializeEP)(
	int iID //[IN] InitializeEP�̖߂�l
	);

//��͑Ώۂ�TS�p�P�b�g��ǂݍ��܂���
//�߂�l�F�G���[�R�[�h
typedef DWORD (WINAPI *AddTSPacketEP)(
	int iID, //[IN] InitializeEP�̖߂�l
	BYTE* pbData, //[IN] TS1�p�P�b�g
	DWORD dwSize //[IN] pbData�̃T�C�Y188�Œ�
	);

//��̓f�[�^�̃N���A
//�߂�l�F�Ȃ�
typedef void (WINAPI *ClearEP)(
	int iID //[IN] InitializeEP�̖߂�l
	);

//PF�f�[�^�̎擾
//�߂�l�F�G���[�R�[�h
typedef DWORD (WINAPI *GetPFDataEP)(
	int iID, //[IN] InitializeEP�̖߂�l
	WORD wSID, //[IN] �ԑg���擾����SID
	EPG_DATA_INFO** pstEpgData, //[OUT] �ԑg���i����GetPFDataEP�ĂԂ܂ŗL���j
	BOOL bNextData //[IN] TRUE:���̔ԑg���AFALSE:���݂̔ԑg���
	);

//���݂̔ԑg��EventID���擾
//�߂�l�FEventID�i-1�ł܂���͂ł���p�P�b�g�������Ă��Ȃ��j
typedef int (WINAPI *GetNowEventIDEP)(
	int iID, //[IN] InitializeEP�̖߂�l
	WORD wSID //[IN] �擾����SID
	);

//EPG��̓��[�h�̐ݒ�
//�߂�l�F�Ȃ�
typedef void (WINAPI *SetPFOnlyEP)(
	int iID, //[IN] InitializeEP�̖߂�l
	BOOL bPFOnly //[IN] TRUE:PF�f�[�^�̂݉�́AFALSE:1�T�ԕ��̃f�[�^�����
	);

//EPG��̓��[�h�̐ݒ�
//�߂�l�F�Ȃ�
typedef void (WINAPI *SetBasicModeEP)(
	int iID, //[IN] InitializeEP�̖߂�l
	BOOL bBasicOnly //[IN] TRUE:��{���݂̂Œ~�ϔ���AFALSE:�g�������~�ϔ���
	);

//���݂̃X�g���[����TSID���擾
//�߂�l�FTSID�i-1�ł܂���͂ł���p�P�b�g�������Ă��Ȃ��j
typedef int (WINAPI *GetTSIDEP)(
	int iID //[IN] InitializeEP�̖߂�l
	);

//���݂̃X�g���[����PMT��PID���X�g���擾
//�߂�l�F�Ȃ�
typedef DWORD (WINAPI *GetPmtPIDListEP)(
	int iID, //[IN] InitializeEP�̖߂�l
	DWORD** pdwPIDList, //[OUT] PMT��PID���X�g�i����GetPmtPIDListEP�ĂԂ܂ŗL���j
	DWORD* dwListCount, //[OUT] pdwPIDList�̌�
	DWORD* dwVersion //[OUT] PAT�̃o�[�W����
	);

//���݂̃X�g���[����PMT��PID���X�g���擾
//�߂�l�F�Ȃ�
typedef DWORD (WINAPI *GetPmtPIDListEP)(
	int iID, //[IN] InitializeEP�̖߂�l
	DWORD** pdwPIDList, //[OUT] PMT��PID���X�g�i����GetPmtPIDListEP�ĂԂ܂ŗL���j
	DWORD* dwListCount, //[OUT] pdwPIDList�̌�
	DWORD* dwVersion //[OUT] PAT�̃o�[�W����
	);

//���݂̃X�g���[���̎w��SID��PMT��PID���擾
//�߂�l�FPMT��PID�i-1�ł܂���͂ł���p�P�b�g�������Ă��Ȃ��j
typedef int (WINAPI *GetPmtPIDEP)(
	int iID, //[IN] InitializeEP�̖߂�l
	DWORD dwSID, //[IN] ServiceID
	DWORD* dwVersion //[OUT] PAT�̃o�[�W����
	);

//���݂̃X�g���[���̎w��SID��PMT��PID���擾
//�߂�l�F�G���[�R�[�h
typedef DWORD (WINAPI *GetESInfoEP)(
	int iID, //[IN] InitializeEP�̖߂�l
	DWORD dwSID, //[IN] ServiceID
	DWORD* pdwPcrPID, //[OUT] PCR��PID
	DWORD* pdwEcmPID, //[OUT] ���Y�T�[�r�X�S�̂�ECM��PID ���݂��Ȃ��ꍇ��0x1FFF ES�Ōʂ̏ꍇ��ELEMENT_INFO���̂��̂�
	ELEMENT_INFO** pstESList, //[OUT]ES���̃��X�g�i����GetESInfoEP�ĂԂ܂ŗL���j
	DWORD* dwListCount, //[OUT] pstESList�̌�
	DWORD* dwVersion //[OUT] PMT�̃o�[�W����
	);

//���݂̃X�g���[����EMM��PID���擾
//�߂�l�FEMM��PID�i-1�ł܂���͂ł���p�P�b�g�������Ă��Ȃ��j
typedef int (WINAPI *GetEmmPIDEP)(
	int iID, //[IN] InitializeEP�̖߂�l
	DWORD* dwVersion //[OUT] CAT�̃o�[�W����
	);

//���݂̃X�g���[���̃T�[�r�XID���X�g���擾
//�߂�l�F�G���[�R�[�h
typedef DWORD (WINAPI *GetServiceListEP)(
	int iID, //[IN] InitializeEP�̖߂�l
	SERVICE_INFO** pstList, //[OUT] �T�[�r�X�ꗗ�i����GetServiceListEP or GetServiceListDBEP�ĂԂ܂ŗL���j
	DWORD* dwListSize //[OUT] pstList�̌�
	);

//���݂̃X�g���[�����ōŌ�ɉ�͂ł������Ԃ��擾
//�߂�l�FFILETIME��__int64�ɃV�t�g�������́i-1�ł܂���͂ł���p�P�b�g�������Ă��Ȃ��j
typedef __int64 (WINAPI *GetNowTimeEP)(
	int iID //[IN] InitializeEP�̖߂�l
	);

//�n�f�W�̏ꍇ�A���݂̃X�g���[���̕����ǖ�
//�߂�l�FNULL�ł܂���͂ł���p�P�b�g�������Ă��Ȃ� or BS/CS
typedef const WCHAR* (WINAPI *GetTsNameEP)(
	int iID //[IN] InitializeEP�̖߂�l
	);

//�~�ύς݂̃T�[�r�XID���X�g���擾
//�߂�l�F�G���[�R�[�h
typedef DWORD (WINAPI *GetServiceListDBEP)(
	int iID, //[IN] InitializeEP�̖߂�l
	SERVICE_INFO** pstList, //[OUT] �T�[�r�X�ꗗ�i����GetServiceListEP or GetServiceListDBEP�ĂԂ܂ŗL���j
	DWORD* dwListSize //[OUT] pstList�̌�
	);

//�~�ύς݂̔ԑg�����擾
//�߂�l�FTRUE:�����AFALSE:���s
typedef BOOL (WINAPI *GetEpgDataListDBEP)(
	int iID, //[IN] InitializeEP�̖߂�l
	DWORD dwONID, //[IN] �擾�������T�[�r�X��OriginalNetworkID
	DWORD dwTSID, //[IN] �擾�������T�[�r�X��TransportStreamID
	DWORD dwSID, //[IN] �擾�������T�[�r�X��ServiceID
	EPG_DATA_INFO2** pstList, //[OUT] �ԑg���ꗗ�i����GetEpgDataListDBEP�ĂԂ܂ŗL���j
	DWORD* dwListSize //[OUT] pstList�̌�
	);

//�~�ύς݂̔ԑg��񂩂�w��EventID�̔ԑg�����擾
//�߂�l�FTRUE:�����AFALSE:���s
typedef BOOL (WINAPI *GetEpgDataDBEP)(
	int iID, //[IN] InitializeEP�̖߂�l
	DWORD dwONID, //[IN] �擾�������T�[�r�X��OriginalNetworkID
	DWORD dwTSID, //[IN] �擾�������T�[�r�X��TransportStreamID
	DWORD dwSID, //[IN] �擾�������T�[�r�X��ServiceID
	DWORD dwEID, //[IN] �擾�������T�[�r�X��EventID
	EPG_DATA_INFO2** pstEpgData //[OUT] �ԑg���i����GetEpgDataDBEP�ĂԂ܂ŗL���j
	);

//�~�ύς݂̔ԑg�����擾
//�߂�l�FTRUE:�����AFALSE:���s
typedef BOOL (WINAPI *GetEpgDataListDB2EP)(
	int iID, //[IN] InitializeEP�̖߂�l
	DWORD dwONID, //[IN] �擾�������T�[�r�X��OriginalNetworkID
	DWORD dwTSID, //[IN] �擾�������T�[�r�X��TransportStreamID
	DWORD dwSID, //[IN] �擾�������T�[�r�X��ServiceID
	EPG_DATA_INFO3** pstList, //[OUT] �ԑg���ꗗ�i����GetEpgDataListDB2EP�ĂԂ܂ŗL���j
	DWORD* dwListSize //[OUT] pstList�̌�
	);

//�~�ύς݂̔ԑg��񂩂�w��EventID�̔ԑg�����擾
//�߂�l�FTRUE:�����AFALSE:���s
typedef BOOL (WINAPI *GetEpgDataDB2EP)(
	int iID, //[IN] InitializeEP�̖߂�l
	DWORD dwONID, //[IN] �擾�������T�[�r�X��OriginalNetworkID
	DWORD dwTSID, //[IN] �擾�������T�[�r�X��TransportStreamID
	DWORD dwSID, //[IN] �擾�������T�[�r�X��ServiceID
	DWORD dwEID, //[IN] �擾�������T�[�r�X��EventID
	EPG_DATA_INFO3** pstEpgData //[OUT] �ԑg���i����GetEpgDataDB2EP�ĂԂ܂ŗL���j
	);

//PF�f�[�^�̎擾
//�߂�l�F�G���[�R�[�h
typedef DWORD (WINAPI *GetPFData2EP)(
	int iID, //[IN] InitializeEP�̖߂�l
	WORD wSID, //[IN] �ԑg���擾����SID
	EPG_DATA_INFO3** pstEpgData, //[OUT] �ԑg���i����GetPFData2EP�ĂԂ܂ŗL���j
	BOOL bNextData //[IN] TRUE:���̔ԑg���AFALSE:���݂̔ԑg���
	);

//�~�ύς݂̔ԑg���̃N���A
//�߂�l�F�Ȃ�
typedef void (WINAPI *ClearDBEP)(
	int iID //[IN] InitializeEP�̖߂�l
	);

#endif