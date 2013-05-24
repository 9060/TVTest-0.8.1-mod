#ifndef __EPG_BON_COMMON_DEF_H__
#define __EPG_BON_COMMON_DEF_H__

#include <windows.h>
#include <TCHAR.h>
#include "Util.h"
#include "StructDef.h"

#define SAVE_FOLDER L"\\EpgTimerBon"
#define EPG_SAVE_FOLDER L"\\EpgData"
#define LOGO_SAVE_FOLDER L"\\LogoData"
#define BON_DLL_FOLDER L"\\BonDriver"

#define EPG_TIMER_SERVICE_EXE L"EpgTimerSrv.exe"

#define EPG_TIMER_BON_MUTEX L"Global\\EpgTimer_Bon2"
#define EPG_TIMER_BON_SRV_MUTEX L"Global\\EpgTimer_Bon_Service"
#define SERVICE_NAME L"EpgTimer_Bon Service"

#define ERR_FALSE FALSE //�ėp�G���[
#define NO_ERR TRUE //����
#define ERR_INIT		10
#define ERR_NOT_INIT	11
#define ERR_SIZE		12
#define ERR_NEED_NEXT_PACKET	20 //����TS�p�P�b�g����Ȃ��Ɖ�͂ł��Ȃ�
#define ERR_CAN_NOT_ANALYZ		21 //�{����TS�p�P�b�g�H��͕s�\
#define ERR_NOT_FIRST 			22 //�ŏ���TS�p�P�b�g������
#define ERR_INVALID_PACKET		23 //�{����TS�p�P�b�g�H�p�P�b�g���ŉ��Ă邩��
#define ERR_NO_CHAGE			30	//�o�[�W�����̕ύX�Ȃ����߉�͕s�v

#define NO_ERR_EPG_ALL 100 //EPG��񒙂܂��� Basic��Extend����
#define NO_ERR_EPG_BASIC 101 //EPG��񒙂܂��� Basic�̂�
#define NO_ERR_EPG_EXTENDED 102 //EPG��񒙂܂��� Extend�̂�


#define RECMODE_ALL 0 //�S�T�[�r�X
#define RECMODE_SERVICE 1 //�w��T�[�r�X�̂�
#define RECMODE_ALL_NOB25 2 //�S�T�[�r�X�iB25�����Ȃ��j
#define RECMODE_SERVICE_NOB25 3 //�w��T�[�r�X�̂݁iB25�����Ȃ��j
#define RECMODE_VIEW 4 //����
#define RECMODE_NO 5 //����
#define RECMODE_EPG 0xFF //EPG�擾

#define RESERVE_EXECUTE 0 //���ʂɗ\����s
#define RESERVE_PILED_UP 1 //�d�Ȃ��Ď��s�ł��Ȃ��\�񂠂�
#define RESERVE_NO_EXECUTE 2 //�d�Ȃ��Ď��s�ł��Ȃ�
#define RESERVE_NO 3 //����

#define RECSERVICEMODE_DEF	0x00000000	//�f�t�H���g�ݒ�
#define RECSERVICEMODE_SET	0x00000001	//�ݒ�l�g�p
#define RECSERVICEMODE_CAP	0x00000010	//�����f�[�^�܂�
#define RECSERVICEMODE_DATA	0x00000020	//�f�[�^�J���[�Z���܂�

static wstring charContentTBL[]={
	L"�j���[�X�E��",
	L"�X�|�[�c",
	L"���E���C�h�V���[",
	L"�h���}",
	L"���y",
	L"�o���G�e�B",
	L"�f��",
	L"�A�j���E����",
	L"�h�L�������^���[�E���{",
	L"����E����",
	L"��E����",
	L"����"
};

__int64 _Create64Key( DWORD dwONID, DWORD dwTSID, DWORD dwSID );
void _SafeReleaseEpgDataInfo(EPG_DATA_INFO* pData);
void _SafeReleaseServiceInfo(SERVICE_INFO* pData);
void _SafeReleaseEpgDataInfo2(EPG_DATA_INFO2* pData);
void _SafeReleaseEpgDataInfo3(EPG_DATA_INFO3* pData);
BOOL _GetStartEndTime(RESERVE_DATA* pItem, int iDefStartMargine, int iDefEndMargine, __int64* i64Start, __int64* i64End, SYSTEMTIME* StartTime, SYSTEMTIME* EndTime );
unsigned long Crc32(int n,  BYTE c[]);

#endif
