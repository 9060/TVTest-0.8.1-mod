/*
	TVTest �v���O�C���w�b�_ ver.0.0.5

	���̃t�@�C���͍Ĕz�z�E���ςȂǎ��R�ɍs���č\���܂���B
	�������A���ς����ꍇ�̓I���W�i���ƈႤ�|���L�ڂ��Ē�����ƁA�������Ȃ��Ă�
	���Ǝv���܂��B

	���ɃR���p�C���Ɉˑ�����L�q�͂Ȃ��͂��Ȃ̂ŁABorland �Ȃǂł����v�ł��B
	�܂��A�w�b�_���ڐA����� C++ �ȊO�̌���Ńv���O�C�����쐬���邱�Ƃ��\��
	�͂��ł�(�����ʓ|�Ȃ̂ŒN�����Ȃ��Ǝv���܂���)�B

	�v���O�C���̎d�l�͂܂��b��ł��̂ŁA����ύX�����\��������܂�(�ł��邾
	���݊����͈ێ��������Ǝv���Ă��܂���)�B

	���ۂɃv���O�C�����쐬����ꍇ�A���̃w�b�_���������Ă����炭�Ӗ��s�����Ǝv
	���܂��̂ŁA�T���v�����Q�l�ɂ��Ă��������B
*/


/*
	TVTest �v���O�C���̊T�v

	�v���O�C����32�r�b�g DLL �̌`���ł��B�g���q�� .tvtp �Ƃ��܂��B
	�v���O�C���ł́A�ȉ��̊֐����G�N�X�|�[�g���܂��B

	DWORD WINAPI TVTGetVersion()
	BOOL WINAPI TVTGetPluginInfo(PluginInfo *pInfo)
	BOOL WINAPI TVTInitialize(PluginParam *pParam)
	BOOL WINAPI TVTFinalize()

	�e�֐��ɂ��ẮA���̃w�b�_�̍Ō�̕��ɂ���v���O�C���N���X�ł̃G�N�X�|�[�g
	�֐��̎���(#ifdef TVTEST_PLUGIN_CLASS_IMPLEMENT �̕���)���Q�Ƃ��Ă��������B

	�v���O�C������́A�R�[���o�b�N�֐���ʂ��ă��b�Z�[�W�𑗐M���邱�Ƃɂ��A
	TVTest �̋@�\�𗘗p���邱�Ƃ��ł��܂��B
	���̂悤�ȕ��@�ɂȂ��Ă���̂́A�����I�Ȋg�����e�Ղł��邽�߂ł��B

	�܂��A�C�x���g�R�[���o�b�N�֐���o�^���邱�Ƃɂ��ATVTest ����C�x���g����
	�m����܂��B

	���b�Z�[�W�̑��M�̓X���b�h�Z�[�t�ł͂���܂���̂ŁA�ʃX���b�h���烁�b�Z�[
	�W�R�[���o�b�N�֐����Ăяo���Ȃ��ł��������B

	TVTEST_PLUGIN_CLASS_IMPLEMENT �V���{���� #define ����Ă���ƁA�G�N�X�|�[�g
	�֐��𒼐ڋL�q���Ȃ��Ă��A�N���X�Ƃ��ăv���O�C�����L�q���邱�Ƃ��ł��܂��B
	���̏ꍇ�ATVTestPlugin �N���X����v���O�C���N���X��h�������܂��B

	�ȉ��͍Œ���̎������s�����T���v���ł��B

	#include <windows.h>
	#define TVTEST_PLUGIN_CLASS_IMPLEMENT	// �v���O�C�����N���X�Ƃ��Ď���
	#include "TVTestPlugin.h"

	// �v���O�C���N���X�BCTVTestPlugin ����h��������
	class CMyPlugin : public TVTest::CTVTestPlugin
	{
	public:
		virtual bool GetPluginInfo(TVTest::PluginInfo *pInfo) {
			// �v���O�C���̏���Ԃ�
			pInfo->Type           = TVTest::PLUGIN_TYPE_NORMAL;
			pInfo->Flags          = 0;
			pInfo->pszPluginName  = L"�T���v��";
			pInfo->pszCopyright   = L"Copyright(c) 2008 Taro Yamada";
			pInfo->pszDescription = L"�������Ȃ��v���O�C��";
			return true;	// false ��Ԃ��ƃv���O�C���̃��[�h�����s�ɂȂ�
		}
		virtual bool Initialize() {
			// �����ŏ��������s��
			// �������Ȃ��̂ł���΃I�[�o�[���C�h���Ȃ��Ă��ǂ�
			return true;	// false ��Ԃ��ƃv���O�C���̃��[�h�����s�ɂȂ�
		}
		virtual bool Finalize() {
			// �����ŃN���[���A�b�v���s��
			// �������Ȃ��̂ł���΃I�[�o�[���C�h���Ȃ��Ă��ǂ�
			return true;
		}
	};

	// CreatePluginClass �֐��ŁA�v���O�C���N���X�̃C���X�^���X�𐶐����ĕԂ�
	TVTest::CTVTestPlugin *CreatePluginClass()
	{
		return new CMyPlugin;
	}
*/


/*
	�X�V����

    ver.0.0.5 (TVTest ver.0.5.41 or later)
    �E�ȉ��̃C�x���g��ǉ�����
      �EEVENT_RESET
      �EEVENT_STATUSRESET
      �EEVENT_AUDIOSTREAMCHANGE
    �EMESSAGE_RESETSTATUS ��ǉ�����

	ver.0.0.4 (TVTest ver.0.5.33 or later)
	�EEVENT_EXECUTE ��ǉ�����

	ver.0.0.3 (TVTest ver.0.5.27 or later)
	�E�ȉ��̃��b�Z�[�W��ǉ�����
	  �EMESSAGE_ISPLUGINENABLED
	  �EMESSAGE_REGISTERCOMMAND
	  �EMESSAGE_ADDLOG
	�EEVENT_STANDBY �� EVENT_COMMAND ��ǉ�����

	ver.0.0.2
	�EMESSAGE_GETAUDIOSTREAM �� MESSAGE_SETAUDIOSTREAM ��ǉ�����
	�EServiceInfo �\���̂� AudioComponentType �� SubtitlePID �����o��ǉ�����
	�EStatusInfo �\���̂� DropPacketCount �� BcasCardStatus �����o��ǉ�����

	ver.0.0.1
	�E�ȉ��̃��b�Z�[�W��ǉ�����
	  �EMESSAGE_QUERYEVENT
	  �EMESSAGE_GETTUNINGSPACE
	  �EMESSAGE_GETTUNINGSPACEINFO
	  �EMESSAGE_SETNEXTCHANNEL
	�EChannelInfo �\���̂ɂ����������o��ǉ�����

	ver.0.0.0
	�E�ŏ��̃o�[�W����
*/


#ifndef TVTEST_PLUGIN_H
#define TVTEST_PLUGIN_H


#include <pshpack1.h>


namespace TVTest {


// �v���O�C���̃o�[�W����
#define TVTEST_PLUGIN_VERSION_0_0_0	0x00000000UL
#define TVTEST_PLUGIN_VERSION_0_0_1	0x00000001UL
#define TVTEST_PLUGIN_VERSION_0_0_2	0x00000002UL
#define TVTEST_PLUGIN_VERSION_0_0_3	0x00000003UL
#define TVTEST_PLUGIN_VERSION_0_0_4	0x00000004UL
#define TVTEST_PLUGIN_VERSION_0_0_5	0x00000005UL
#ifndef TVTEST_PLUGIN_VERSION
#define TVTEST_PLUGIN_VERSION TVTEST_PLUGIN_VERSION_0_0_5
#endif

// �G�N�X�|�[�g�֐���`�p
#define TVTEST_EXPORT(type) extern "C" __declspec(dllexport) type WINAPI

// offsetof
#define TVTEST_OFFSETOF(type,member) \
	((size_t)((BYTE*)&((type*)0)->member-(BYTE*)(type*)0))

// �v���O�C���̎��
enum {
	PLUGIN_TYPE_NORMAL	// ����
};

// �v���O�C���̃t���O
enum {
	PLUGIN_FLAG_HASSETTINGS		=0x00000001UL,	// �ݒ�_�C�A���O������
	PLUGIN_FLAG_ENABLEDEFAULT	=0x00000002UL	// �f�t�H���g�ŗL��
												// ���ʂȗ��R����������g��Ȃ�
};

// �v���O�C���̏��
struct PluginInfo {
	DWORD Type;				// ���(PLUGIN_TYPE_???)
	DWORD Flags;			// �t���O(PLUGIN_FLAG_???)
	LPCWSTR pszPluginName;	// �v���O�C����
	LPCWSTR pszCopyright;	// ���쌠���
	LPCWSTR pszDescription;	// ������
};

// ���b�Z�[�W���M�p�R�[���o�b�N�֐�
typedef LRESULT (CALLBACK *MessageCallbackFunc)(struct PluginParam *pParam,UINT Message,LPARAM lParam1,LPARAM lParam2);

// �v���O�C���p�����[�^
struct PluginParam {
	MessageCallbackFunc Callback;	// �R�[���o�b�N�֐�
	HWND hwndApp;					// ���C���E�B���h�E�̃n���h��
	void *pClientData;				// �v���O�C�����ōD���Ɏg����f�[�^
	void *pInternalData;			// TVTest���Ŏg�p����f�[�^�B�A�N�Z�X�֎~
};

// �G�N�X�|�[�g�֐�
typedef DWORD (WINAPI *GetVersionFunc)();
typedef BOOL (WINAPI *GetPluginInfoFunc)(PluginInfo *pInfo);
typedef BOOL (WINAPI *InitializeFunc)(PluginParam *pParam);
typedef BOOL (WINAPI *FinalizeFunc)();

// ���b�Z�[�W
enum {
	MESSAGE_GETVERSION,
	MESSAGE_QUERYMESSAGE,
	MESSAGE_MEMORYALLOC,
	MESSAGE_SETEVENTCALLBACK,
	MESSAGE_GETCURRENTCHANNELINFO,
	MESSAGE_SETCHANNEL,
	MESSAGE_GETSERVICE,
	MESSAGE_SETSERVICE,
	MESSAGE_GETTUNINGSPACENAME,
	MESSAGE_GETCHANNELINFO,
	MESSAGE_GETSERVICEINFO,
	MESSAGE_GETDRIVERNAME,
	MESSAGE_SETDRIVERNAME,
	MESSAGE_STARTRECORD,
	MESSAGE_STOPRECORD,
	MESSAGE_PAUSERECORD,
	MESSAGE_GETRECORD,
	MESSAGE_MODIFYRECORD,
	MESSAGE_GETZOOM,
	MESSAGE_SETZOOM,
	MESSAGE_GETPANSCAN,
	MESSAGE_SETPANSCAN,
	MESSAGE_GETSTATUS,
	MESSAGE_GETRECORDSTATUS,
	MESSAGE_GETVIDEOINFO,
	MESSAGE_GETVOLUME,
	MESSAGE_SETVOLUME,
	MESSAGE_GETSTEREOMODE,
	MESSAGE_SETSTEREOMODE,
	MESSAGE_GETFULLSCREEN,
	MESSAGE_SETFULLSCREEN,
	MESSAGE_GETPREVIEW,
	MESSAGE_SETPREVIEW,
	MESSAGE_GETSTANDBY,
	MESSAGE_SETSTANDBY,
	MESSAGE_GETALWAYSONTOP,
	MESSAGE_SETALWAYSONTOP,
	MESSAGE_CAPTUREIMAGE,
	MESSAGE_SAVEIMAGE,
	MESSAGE_RESET,
	MESSAGE_CLOSE,
	MESSAGE_SETSTREAMCALLBACK,
	MESSAGE_ENABLEPLUGIN,
	MESSAGE_GETCOLOR,
	MESSAGE_DECODEARIBSTRING,
	MESSAGE_GETCURRENTPROGRAMINFO,
#if TVTEST_PLUGIN_VERSION>=TVTEST_PLUGIN_VERSION_0_0_1
	MESSAGE_QUERYEVENT,
	MESSAGE_GETTUNINGSPACE,
	MESSAGE_GETTUNINGSPACEINFO,
	MESSAGE_SETNEXTCHANNEL,
#endif
#if TVTEST_PLUGIN_VERSION>=TVTEST_PLUGIN_VERSION_0_0_2
	MESSAGE_GETAUDIOSTREAM,
	MESSAGE_SETAUDIOSTREAM,
#endif
#if TVTEST_PLUGIN_VERSION>=TVTEST_PLUGIN_VERSION_0_0_3
	MESSAGE_ISPLUGINENABLED,
	MESSAGE_REGISTERCOMMAND,
	MESSAGE_ADDLOG,
#endif
#if TVTEST_PLUGIN_VERSION>=TVTEST_PLUGIN_VERSION_0_0_5
	MESSAGE_RESETSTATUS,
#endif
	MESSAGE_TRAILER
};

// �C�x���g�p�R�[���o�b�N�֐�
typedef LRESULT (CALLBACK *EventCallbackFunc)(UINT Event,LPARAM lParam1,LPARAM lParam2,void *pClientData);

// �C�x���g
// �e�C�x���g�������̃p�����[�^�� CTVTestEventHadler ���Q�Ƃ��Ă��������B
enum {
	EVENT_PLUGINENABLE,			// �L����Ԃ��ω�����
	EVENT_PLUGINSETTINGS,		// �ݒ���s��
	EVENT_CHANNELCHANGE,		// �`�����l�����ύX���ꂽ
	EVENT_SERVICECHANGE,		// �T�[�r�X���ύX���ꂽ
	EVENT_DRIVERCHANGE,			// �h���C�o���ύX���ꂽ
	EVENT_SERVICEUPDATE,		// �T�[�r�X�̍\�����ω�����
	EVENT_RECORDSTATUSCHANGE,	// �^���Ԃ��ω�����
	EVENT_FULLSCREENCHANGE,		// �S��ʕ\����Ԃ��ω�����
	EVENT_PREVIEWCHANGE,		// �v���r���[�\����Ԃ��ω�����
	EVENT_VOLUMECHANGE,			// ���ʂ��ω�����
	EVENT_STEREOMODECHANGE,		// �X�e���I���[�h���ω�����
	EVENT_COLORCHANGE,			// �F�̐ݒ肪�ω�����
#if TVTEST_PLUGIN_VERSION>=TVTEST_PLUGIN_VERSION_0_0_3
	EVENT_STANDBY,				// �ҋ@��Ԃ��ω�����
	EVENT_COMMAND,				// �R�}���h���I�����ꂽ
#endif
#if TVTEST_PLUGIN_VERSION>=TVTEST_PLUGIN_VERSION_0_0_4
	EVENT_EXECUTE,				// �����N���֎~���ɕ����N�����ꂽ
#endif
#if TVTEST_PLUGIN_VERSION>=TVTEST_PLUGIN_VERSION_0_0_5
	EVENT_RESET,				// ���Z�b�g���ꂽ
	EVENT_STATUSRESET,			// �X�e�[�^�X�����Z�b�g���ꂽ
	EVENT_AUDIOSTREAMCHANGE,	// �����X�g���[�����ύX���ꂽ
#endif
	EVENT_TRAILER
};

inline DWORD MakeVersion(BYTE Major,WORD Minor,WORD Build) {
	return ((DWORD)Major<<24) | ((DWORD)Minor<<12) | Build;
}
inline DWORD GetMajorVersion(DWORD Version) { return Version>>24; }
inline DWORD GetMinorVersion(DWORD Version) { return (Version&0x00FFF000UL)>>12; }
inline DWORD GetBuildVersion(DWORD Version) { return Version&0x00000FFFUL; }

// TVTest�̃o�[�W�������擾����
// ���8�r�b�g�����W���[�o�[�W�����A����12�r�b�g���}�C�i�[�o�[�W�����A
// ����12�r�b�g���r���h�i���o�[
// GetMajorVersion/GetMinorVersion/GetBuildVersion���g���Ď擾�ł���
inline DWORD MsgGetVersion(PluginParam *pParam) {
	return (*pParam->Callback)(pParam,MESSAGE_GETVERSION,0,0);
}

// �w�肳�ꂽ���b�Z�[�W�ɑΉ����Ă��邩�₢���킹��
inline bool MsgQueryMessage(PluginParam *pParam,UINT Message) {
	return (*pParam->Callback)(pParam,MESSAGE_QUERYMESSAGE,Message,0)!=0;
}

// �������Ċm��
// �d�l��realloc�Ɠ���
inline void *MsgMemoryReAlloc(PluginParam *pParam,void *pData,DWORD Size) {
	return (void*)(*pParam->Callback)(pParam,MESSAGE_MEMORYALLOC,(LPARAM)pData,Size);
}

// �������m��
// �d�l��realloc(NULL,Size)�Ɠ���
inline void *MsgMemoryAlloc(PluginParam *pParam,DWORD Size) {
	return (void*)(*pParam->Callback)(pParam,MESSAGE_MEMORYALLOC,(LPARAM)(void*)NULL,Size);
}

// �������J��
// �d�l��realloc(pData,0)�Ɠ���
// (���ۂ�realloc�Ń������J�����Ă���R�[�h�͌������Ɩ�������...)
inline void MsgMemoryFree(PluginParam *pParam,void *pData) {
	(*pParam->Callback)(pParam,MESSAGE_MEMORYALLOC,(LPARAM)pData,0);
}

// �C�x���g�n���h���p�R�[���o�b�N�̐ݒ�
// pClientData�̓R�[���o�b�N�̌Ăяo�����ɓn�����
inline bool MsgSetEventCallback(PluginParam *pParam,EventCallbackFunc Callback,void *pClientData=NULL) {
	return (*pParam->Callback)(pParam,MESSAGE_SETEVENTCALLBACK,(LPARAM)Callback,(LPARAM)pClientData)!=0;
}

// �`�����l���̏��
struct ChannelInfo {
	DWORD Size;							// �\���̂̃T�C�Y
	int Space;							// �`���[�j���O���(BonDriver�̃C���f�b�N�X)
	int Channel;						// �`�����l��(BonDriver�̃C���f�b�N�X)
	int RemoteControlKeyID;				// �����R��ID
	WORD NetworkID;						// �l�b�g���[�NID
	WORD TransportStreamID;				// �g�����X�|�[�g�X�g���[��ID
	WCHAR szNetworkName[32];			// �l�b�g���[�N��
	WCHAR szTransportStreamName[32];	// �g�����X�|�[�g�X�g���[����
	WCHAR szChannelName[64];			// �`�����l����
#if TVTEST_PLUGIN_VERSION>=TVTEST_PLUGIN_VERSION_0_0_1
	int PhysicalChannel;				// �����`�����l���ԍ�(�ꍇ�ɂ���Ă͐M�p�ł��Ȃ�)
										// �s���̏ꍇ��0
	WORD ServiceIndex;					// �T�[�r�X�̃C���f�b�N�X
	WORD ServiceID;						// �T�[�r�XID
	// �T�[�r�X�̓`�����l���t�@�C���Őݒ肳��Ă�����̂��擾�����
	// �T�[�r�X�̓��[�U�[���؂�ւ�����̂ŁA���ۂɎ������̃T�[�r�X������ł���Ƃ͌���Ȃ�
	// ���ۂɎ������̃T�[�r�X�� MESSAGE_GETSERVICE �Ŏ擾�ł���
#endif
};

#if TVTEST_PLUGIN_VERSION>=TVTEST_PLUGIN_VERSION_0_0_1
enum { CHANNELINFO_SIZE_V1=TVTEST_OFFSETOF(ChannelInfo,PhysicalChannel) };
#endif

// ���݂̃`�����l���̏����擾����
// ���O�� ChannelInfo �� Size �����o��ݒ肵�Ă���
inline bool MsgGetCurrentChannelInfo(PluginParam *pParam,ChannelInfo *pInfo) {
	return (*pParam->Callback)(pParam,MESSAGE_GETCURRENTCHANNELINFO,(LPARAM)pInfo,0)!=0;
}

// �`�����l����ݒ肷��
inline bool MsgSetChannel(PluginParam *pParam,int Space,int Channel) {
	return (*pParam->Callback)(pParam,MESSAGE_SETCHANNEL,Space,Channel)!=0;
}

// ���݂̃T�[�r�X�y�уT�[�r�X�����擾����
// �T�[�r�X�̃C���f�b�N�X���Ԃ�B�G���[����-1���Ԃ�
// pNumServices �� NULL �łȂ��ꍇ�́A�T�[�r�X�̐����Ԃ����
inline int MsgGetService(PluginParam *pParam,int *pNumServices=NULL) {
	return (*pParam->Callback)(pParam,MESSAGE_GETSERVICE,(LPARAM)pNumServices,0);
}

// �T�[�r�X��ݒ肷��
// fByID=false �̏ꍇ�̓C���f�b�N�X�AfByID=true�̏ꍇ�̓T�[�r�XID
inline bool MsgSetService(PluginParam *pParam,int Service,bool fByID=false) {
	return (*pParam->Callback)(pParam,MESSAGE_SETSERVICE,Service,fByID)!=0;
}

// �`���[�j���O��Ԗ����擾����
// �`���[�j���O��Ԗ��̒������Ԃ�BIndex���͈͊O�̏ꍇ��0���Ԃ�
// pszName��NULL�ŌĂׂΒ����������擾�ł���
inline int MsgGetTuningSpaceName(PluginParam *pParam,int Index,LPWSTR pszName,int MaxLength) {
	return (*pParam->Callback)(pParam,MESSAGE_GETTUNINGSPACENAME,(LPARAM)pszName,MAKELPARAM(Index,min(MaxLength,0xFFFF)));
}

// �`�����l���̏����擾����
// ���O�� ChannelInfo �� Size �����o��ݒ肵�Ă���
// szNetworkName,szTransportStreamName �� MESSAGE_GETCURRENTCHANNEL �ł����擾�ł��Ȃ�
// NetworkID,TransportStreamID �͎擾�ł�����ł��Ȃ�������(�擾�ł��Ȃ������ꍇ��0)
inline bool MsgGetChannelInfo(PluginParam *pParam,int Space,int Index,ChannelInfo *pInfo) {
	return (*pParam->Callback)(pParam,MESSAGE_GETCHANNELINFO,(LPARAM)pInfo,MAKELPARAM(Space,Index))!=0;
}

// �T�[�r�X�̏��
struct ServiceInfo {
	DWORD Size;					// �\���̂̃T�C�Y
	WORD ServiceID;				// �T�[�r�XID
	WORD VideoPID;				// �r�f�I�X�g���[����PID
	int NumAudioPIDs;			// ����PID�̐�
	WORD AudioPID[4];			// �����X�g���[����PID
	WCHAR szServiceName[32];	// �T�[�r�X��
#if TVTEST_PLUGIN_VERSION>=TVTEST_PLUGIN_VERSION_0_0_2
	BYTE AudioComponentType[4];	// �����R���|�[�l���g�^�C�v
	WORD SubtitlePID;			// �����X�g���[����PID(�����ꍇ��0)
	WORD Reserved;				// �\��
#endif
};

#if TVTEST_PLUGIN_VERSION>=TVTEST_PLUGIN_VERSION_0_0_2
enum { SERVICEINFO_SIZE_V1=TVTEST_OFFSETOF(ServiceInfo,AudioComponentType) };
#endif

// �T�[�r�X�̏����擾����
// ���O��ServiceInfo��Size�����o��ݒ肵�Ă���
inline bool MsgGetServiceInfo(PluginParam *pParam,int Index,ServiceInfo *pInfo) {
	return (*pParam->Callback)(pParam,MESSAGE_GETSERVICEINFO,Index,(LPARAM)pInfo)!=0;
}

// �h���C�o�̃t�@�C�������擾����
// �߂�l�̓t�@�C�����̒���(�i��������)
// pszName��NULL�ŌĂׂΒ����������擾�ł���
inline int MsgGetDriverName(PluginParam *pParam,LPWSTR pszName,int MaxLength) {
	return (*pParam->Callback)(pParam,MESSAGE_GETDRIVERNAME,(LPARAM)pszName,MaxLength);
}

// �h���C�o��ݒ肷��
inline bool MsgSetDriverName(PluginParam *pParam,LPCWSTR pszName) {
	return (*pParam->Callback)(pParam,MESSAGE_SETDRIVERNAME,(LPARAM)pszName,0)!=0;
}

// �^����̃}�X�N
enum {
	RECORD_MASK_FLAGS		=0x00000001UL,
	RECORD_MASK_FILENAME	=0x00000002UL,
	RECORD_MASK_STARTTIME	=0x00000004UL,
	RECORD_MASK_STOPTIME	=0x00000008UL
};

// �^��t���O
enum {
	RECORD_FLAG_CANCEL		=0x10000000UL	// �L�����Z��
};

// �^��J�n���Ԃ̎w����@
enum {
	RECORD_START_NOTSPECIFIED,	// ���w��
	RECORD_START_TIME,			// �����w��
	RECORD_START_DELAY			// �����w��
};

// �^���~���Ԃ̎w����@
enum {
	RECORD_STOP_NOTSPECIFIED,	// ���w��
	RECORD_STOP_TIME,			// �����w��
	RECORD_STOP_DURATION		// �����w��
};

// �^����
struct RecordInfo {
	DWORD Size;				// �\���̂̃T�C�Y
	DWORD Mask;				// �}�X�N(RECORD_MASK_???)
	DWORD Flags;			// �t���O(RECORD_FLAG_???)
	LPWSTR pszFileName;		// �t�@�C����(NULL�Ńf�t�H���g)
	int MaxFileName;		// �t�@�C�����̍ő咷(MESSAGE_GETRECORD�݂̂Ŏg�p)
	FILETIME ReserveTime;	// �^��\�񂳂ꂽ����(MESSAGE_GETRECORD�݂̂Ŏg�p)
	DWORD StartTimeSpec;	// �^��J�n���Ԃ̎w����@(RECORD_START_???)
	union {
		FILETIME Time;		// �^��J�n����(StartTimeSpec==RECORD_START_TIME)
							// ���[�J������
		ULONGLONG Delay;	// �^��J�n����(StartTimeSpec==RECORD_START_DELAY)
							// �^����J�n����܂ł̎���(ms)
	} StartTime;
	DWORD StopTimeSpec;		// �^���~���Ԃ̎w����@(RECORD_STOP_???)
	union {
		FILETIME Time;		// �^���~����(StopTimeSpec==RECORD_STOP_TIME)
							// ���[�J������
		ULONGLONG Duration;	// �^���~����(StopTimeSpec==RECORD_STOP_DURATION)
							// �J�n���Ԃ���̃~���b
	} StopTime;
};

// �^����J�n����
// pInfo �� NULL �ő����^��J�n
inline bool MsgStartRecord(PluginParam *pParam,const RecordInfo *pInfo=NULL) {
	return (*pParam->Callback)(pParam,MESSAGE_STARTRECORD,(LPARAM)pInfo,0)!=0;
}

// �^����~����
inline bool MsgStopRecord(PluginParam *pParam) {
	return (*pParam->Callback)(pParam,MESSAGE_STOPRECORD,0,0)!=0;
}

// �^����ꎞ��~/�ĊJ����
inline bool MsgPauseRecord(PluginParam *pParam,bool fPause=true) {
	return (*pParam->Callback)(pParam,MESSAGE_PAUSERECORD,fPause,0)!=0;
}

// �^��ݒ���擾����
inline bool MsgGetRecord(PluginParam *pParam,RecordInfo *pInfo) {
	return (*pParam->Callback)(pParam,MESSAGE_GETRECORD,(LPARAM)pInfo,0)!=0;
}

// �^��ݒ��ύX����
// ���ɘ^�撆�ł���ꍇ�́A�t�@�C�����ƊJ�n���Ԃ̎w��͖��������
inline bool MsgModifyRecord(PluginParam *pParam,const RecordInfo *pInfo) {
	return (*pParam->Callback)(pParam,MESSAGE_MODIFYRECORD,(LPARAM)pInfo,0)!=0;
}

// �\���{�����擾����(%�P��)
inline int MsgGetZoom(PluginParam *pParam) {
	return (*pParam->Callback)(pParam,MESSAGE_GETZOOM,0,0);
}

// �\���{����ݒ肷��
// %�P�ʂ����ł͂Ȃ��ANum=1/Denom=3�ȂǂƂ��Ċ���؂�Ȃ��{����ݒ肷�邱�Ƃ��ł���
inline bool MsgSetZoom(PluginParam *pParam,int Num,int Denom=100) {
	return (*pParam->Callback)(pParam,MESSAGE_SETZOOM,Num,Denom)!=0;
}

// �p���X�L�����̎��
enum {
	PANSCAN_NONE,		// �Ȃ�
	PANSCAN_LETTERBOX,	// ���^�[�{�b�N�X
	PANSCAN_SIDECUT,	// �T�C�h�J�b�g
	PANSCAN_SUPERFRAME	// ���z��
};

// �p���X�L�����̏��
struct PanScanInfo {
	DWORD Size;		// �\���̂̃T�C�Y
	int Type;		// ���(PANSCAN_???)
	int XAspect;	// �����A�X�y�N�g��
	int YAspect;	// �����A�X�y�N�g��
};

// �p���X�L�����̐ݒ���擾����
inline bool MsgGetPanScan(PluginParam *pParam,PanScanInfo *pInfo) {
	return (*pParam->Callback)(pParam,MESSAGE_GETPANSCAN,(LPARAM)pInfo,0)!=0;
}

// �p���X�L������ݒ肷��
inline bool MsgSetPanScan(PluginParam *pParam,const PanScanInfo *pInfo) {
	return (*pParam->Callback)(pParam,MESSAGE_SETPANSCAN,(LPARAM)pInfo,0)!=0;
}

// B-CAS �J�[�h�̏��
enum {
	BCAS_STATUS_OK,				// �G���[�Ȃ�
	BCAS_STATUS_NOTOPEN,		// �J����Ă��Ȃ�(�X�N�����u�������Ȃ�)
	BCAS_STATUS_NOCARDREADER,	// �J�[�h���[�_������
	BCAS_STATUS_NOCARD,			// �J�[�h���Ȃ�
	BCAS_STATUS_OPENERROR,		// �I�[�v���G���[
	BCAS_STATUS_TRANSMITERROR,	// �ʐM�G���[
	BCAS_STATUS_ESTABLISHERROR	// �R���e�L�X�g�m�����s
};

// �X�e�[�^�X���
struct StatusInfo {
	DWORD Size;							// �\���̂̃T�C�Y
	float SignalLevel;					// �M�����x��(dB)
	DWORD BitRate;						// �r�b�g���[�g(Bits/Sec)
	DWORD ErrorPacketCount;				// �G���[�p�P�b�g��
										// DropPacketCount ���܂܂��
	DWORD ScramblePacketCount;			// �����R��p�P�b�g��
#if TVTEST_PLUGIN_VERSION>=TVTEST_PLUGIN_VERSION_0_0_2
	DWORD DropPacketCount;				// �h���b�v�p�P�b�g��
	DWORD BcasCardStatus;				// B-CAS �J�[�h�̏��(BCAS_STATUS_???)
#endif
};

#if TVTEST_PLUGIN_VERSION>=TVTEST_PLUGIN_VERSION_0_0_2
enum { STATUSINFO_SIZE_V1=TVTEST_OFFSETOF(StatusInfo,DropPacketCount) };
#endif

// �X�e�[�^�X���擾����
// ���O��StatusInfo��Size�����o��ݒ肵�Ă���
inline bool MsgGetStatus(PluginParam *pParam,StatusInfo *pInfo) {
	return (*pParam->Callback)(pParam,MESSAGE_GETSTATUS,(LPARAM)pInfo,0)!=0;
}

// �^��̏��
enum {
	RECORD_STATUS_NOTRECORDING,	// �^�悵�Ă��Ȃ�
	RECORD_STATUS_RECORDING,	// �^�撆
	RECORD_STATUS_PAUSED		// �^��ꎞ��~��
};

// �^��X�e�[�^�X���
struct RecordStatusInfo {
	DWORD Size;				// �\���̂̃T�C�Y
	DWORD Status;			// ���(RECORD_STATUS_???)
	FILETIME StartTime;		// �^��J�n����(���[�J������)
	DWORD RecordTime;		// �^�掞��(ms) �ꎞ��~�����܂܂Ȃ�
	DWORD PauseTime;		// �ꎞ��~����(ms)
	DWORD StopTimeSpec;		// �^���~���Ԃ̎w����@(RECORD_STOP_???)
	union {
		FILETIME Time;		// �^���~����(StopTimeSpec==RECORD_STOP_TIME)
							// ���[�J������
		ULONGLONG Duration;	// �^���~����(StopTimeSpec==RECORD_STOP_DURATION)
							// �J�n����(StartTime)����̃~���b
	} StopTime;
};

// �^��X�e�[�^�X���擾����
// ���O��RecordStatusInfo��Size�����o��ݒ肵�Ă���
inline bool MsgGetRecordStatus(PluginParam *pParam,RecordStatusInfo *pInfo) {
	return (*pParam->Callback)(pParam,MESSAGE_GETRECORDSTATUS,(LPARAM)pInfo,0)!=0;
}

// �f���̏��
struct VideoInfo {
	DWORD Size;			// �\���̂̃T�C�Y
	int Width;			// ��(�s�N�Z���P��)
	int Height;			// ����(�s�N�Z���P��)
	int XAspect;		// �����A�X�y�N�g��
	int YAspect;		// �����A�X�y�N�g��
	RECT SourceRect;	// �\�[�X�̕\���͈�
};

// �f���̏����擾����
// ���O��VideoInfo��Size�����o��ݒ肵�Ă���
inline bool MsgGetVideoInfo(PluginParam *pParam,VideoInfo *pInfo) {
	return (*pParam->Callback)(pParam,MESSAGE_GETVIDEOINFO,(LPARAM)pInfo,0)!=0;
}

// ���ʂ��擾����(0-100)
inline int MsgGetVolume(PluginParam *pParam) {
	return LOWORD((*pParam->Callback)(pParam,MESSAGE_GETVOLUME,0,0));
}

// ���ʂ�ݒ肷��(0-100)
inline bool MsgSetVolume(PluginParam *pParam,int Volume) {
	return (*pParam->Callback)(pParam,MESSAGE_SETVOLUME,Volume,0)!=0;
}

// ������Ԃł��邩�擾����
inline bool MsgGetMute(PluginParam *pParam) {
	return HIWORD((*pParam->Callback)(pParam,MESSAGE_GETVOLUME,0,0))!=0;
}

// ������Ԃ�ݒ肷��
inline bool MsgSetMute(PluginParam *pParam,bool fMute) {
	return (*pParam->Callback)(pParam,MESSAGE_SETVOLUME,-1,fMute)!=0;
}

// �X�e���I���[�h
enum {
	STEREOMODE_STEREO,	// �X�e���I
	STEREOMODE_LEFT,	// ��(�剹��)
	STEREOMODE_RIGHT	// �E(������)
};

// �X�e���I���[�h���擾����
inline int MsgGetStereoMode(PluginParam *pParam) {
	return (*pParam->Callback)(pParam,MESSAGE_GETSTEREOMODE,0,0);
}

// �X�e���I���[�h��ݒ肷��
inline bool MsgSetStereoMode(PluginParam *pParam,int StereoMode) {
	return (*pParam->Callback)(pParam,MESSAGE_SETSTEREOMODE,StereoMode,0)!=0;
}

// �S��ʕ\���̏�Ԃ��擾����
inline bool MsgGetFullscreen(PluginParam *pParam) {
	return (*pParam->Callback)(pParam,MESSAGE_GETFULLSCREEN,0,0)!=0;
}

// �S��ʕ\���̏�Ԃ�ݒ肷��
inline bool MsgSetFullscreen(PluginParam *pParam,bool fFullscreen) {
	return (*pParam->Callback)(pParam,MESSAGE_SETFULLSCREEN,fFullscreen,0)!=0;
}

// �\�����L���ł��邩�擾����
inline bool MsgGetPreview(PluginParam *pParam) {
	return (*pParam->Callback)(pParam,MESSAGE_GETPREVIEW,0,0)!=0;
}

// �\���̗L����Ԃ�ݒ肷��
inline bool MsgSetPreview(PluginParam *pParam,bool fPreview) {
	return (*pParam->Callback)(pParam,MESSAGE_SETPREVIEW,fPreview,0)!=0;
}

// �ҋ@��Ԃł��邩�擾����
inline bool MsgGetStandby(PluginParam *pParam) {
	return (*pParam->Callback)(pParam,MESSAGE_GETSTANDBY,0,0)!=0;
}

// �ҋ@��Ԃ�ݒ肷��
inline bool MsgSetStandby(PluginParam *pParam,bool fStandby) {
	return (*pParam->Callback)(pParam,MESSAGE_SETSTANDBY,fStandby,0)!=0;
}

// ��ɍőO�ʕ\���̏�Ԃ��擾����
inline bool MsgGetAlwaysOnTop(PluginParam *pParam) {
	return (*pParam->Callback)(pParam,MESSAGE_GETALWAYSONTOP,0,0)!=0;
}

// ��ɍőO�ʕ\���̏�Ԃ�ݒ肷��
inline bool MsgSetAlwaysOnTop(PluginParam *pParam,bool fAlwaysOnTop) {
	return (*pParam->Callback)(pParam,MESSAGE_SETALWAYSONTOP,fAlwaysOnTop,0)!=0;
}

// �摜���L���v�`������
// �߂�l��DIB�f�[�^�ւ̃|�C���^
// �s�v�ɂȂ����ꍇ��MsgMemoryFree�ŊJ������
inline void *MsgCaptureImage(PluginParam *pParam,DWORD Flags=0) {
	return (void*)(*pParam->Callback)(pParam,MESSAGE_CAPTUREIMAGE,Flags,0);
}

// �摜��ۑ�����
inline bool MsgSaveImage(PluginParam *pParam) {
	return (*pParam->Callback)(pParam,MESSAGE_SAVEIMAGE,0,0)!=0;
}

// ���Z�b�g���s��
inline bool MsgReset(PluginParam *pParam) {
	return (*pParam->Callback)(pParam,MESSAGE_RESET,0,0)!=0;
}

// �E�B���h�E�N���[�Y�̃t���O
enum {
	CLOSE_EXIT	=0x00000001UL	// �K���I��������
};

// �E�B���h�E�����
inline bool MsgClose(PluginParam *pParam,DWORD Flags=0) {
	return (*pParam->Callback)(pParam,MESSAGE_CLOSE,Flags,0)!=0;
}

// �X�g���[���R�[���o�b�N�֐�
typedef BOOL (CALLBACK *StreamCallbackFunc)(BYTE *pData,void *pClientData);

// �X�g���[���R�[���o�b�N�t���O
enum {
	STREAM_CALLBACK_REMOVE	=0x00000001UL	// �R�[���o�b�N�̍폜
};

// �X�g���[���R�[���o�b�N�̏��
struct StreamCallbackInfo {
	DWORD Size;						// �\���̂̃T�C�Y
	DWORD Flags;					// �t���O(STREAM_CALLBACK_???)
	StreamCallbackFunc Callback;	// �R�[���o�b�N�֐�
	void *pClientData;				// �R�[���o�b�N�֐��ɓn�����f�[�^
};

// �X�g���[���R�[���o�b�N��ݒ肷��
// �X�g���[���R�[���o�b�N�֐��ŏ������x������ƑS�̂��x������̂ŁA
// ���Ԃ��|���鏈���͕ʃX���b�h�ōs���Ȃǂ��Ă�������
inline bool MsgSetStreamCallback(PluginParam *pParam,DWORD Flags,
					StreamCallbackFunc Callback,void *pClientData=NULL) {
	StreamCallbackInfo Info;
	Info.Size=sizeof(StreamCallbackInfo);
	Info.Flags=Flags;
	Info.Callback=Callback;
	Info.pClientData=pClientData;
	return (*pParam->Callback)(pParam,MESSAGE_SETSTREAMCALLBACK,(LPARAM)&Info,0)!=0;
}

// �v���O�C���̗L����Ԃ�ݒ肷��
inline bool MsgEnablePlugin(PluginParam *pParam,bool fEnable) {
	return (*pParam->Callback)(pParam,MESSAGE_ENABLEPLUGIN,fEnable,0)!=0;
}

// �F�̐ݒ���擾����
inline COLORREF MsgGetColor(PluginParam *pParam,LPCWSTR pszColor) {
	return (*pParam->Callback)(pParam,MESSAGE_GETCOLOR,(LPARAM)pszColor,0);
}

// ARIB������̃f�R�[�h���
struct ARIBStringDecodeInfo {
	DWORD Size;				// �\���̂̃T�C�Y
	DWORD Flags;			// �t���O(���݂͏��0)
	const void *pSrcData;	// �ϊ����f�[�^
	DWORD SrcLength;		// �ϊ����T�C�Y(�o�C�g�P��)
	LPWSTR pszDest;			// �ϊ���o�b�t�@
	DWORD DestLength;		// �ϊ���o�b�t�@�̃T�C�Y(�����P��)
};

// ARIB��������f�R�[�h����
inline bool MsgDecodeARIBString(PluginParam *pParam,const void *pSrcData,
							DWORD SrcLength,LPWSTR pszDest,DWORD DestLength) {
	ARIBStringDecodeInfo Info;
	Info.Size=sizeof(ARIBStringDecodeInfo);
	Info.Flags=0;
	Info.pSrcData=pSrcData;
	Info.SrcLength=SrcLength;
	Info.pszDest=pszDest;
	Info.DestLength=DestLength;
	return (*pParam->Callback)(pParam,MESSAGE_DECODEARIBSTRING,(LPARAM)&Info,0)!=0;
}

// �ԑg�̏��
struct ProgramInfo {
	DWORD Size;				// �\���̂̃T�C�Y
	WORD ServiceID;			// �T�[�r�XID
	WORD EventID;			// �C�x���gID
	LPWSTR pszEventName;	// �C�x���g��
	int MaxEventName;		// �C�x���g���̍ő咷
	LPWSTR pszEventText;	// �C�x���g�e�L�X�g
	int MaxEventText;		// �C�x���g�e�L�X�g�̍ő咷
	LPWSTR pszEventExtText;	// �ǉ��C�x���g�e�L�X�g
	int MaxEventExtText;	// �ǉ��C�x���g�e�L�X�g�̍ő咷
	SYSTEMTIME StartTime;	// �J�n����(���[�J������)
	DWORD Duration;			// ����(�b�P��)
};

// ���݂̔ԑg�̏����擾����
inline bool MsgGetCurrentProgramInfo(PluginParam *pParam,ProgramInfo *pInfo,bool fNext=false) {
	return (*pParam->Callback)(pParam,MESSAGE_GETCURRENTPROGRAMINFO,(LPARAM)pInfo,fNext)!=0;
}

#if TVTEST_PLUGIN_VERSION>=TVTEST_PLUGIN_VERSION_0_0_1

// �w�肳�ꂽ�C�x���g�ɑΉ����Ă��邩�擾����
inline bool MsgQueryEvent(PluginParam *pParam,UINT Event) {
	return (*pParam->Callback)(pParam,MESSAGE_QUERYEVENT,Event,0)!=0;
}

// ���݂̃`���[�j���O��ԋy�у`���[�j���O��Ԑ����擾����
inline int MsgGetTuningSpace(PluginParam *pParam,int *pNumSpaces=NULL) {
	return (*pParam->Callback)(pParam,MESSAGE_GETTUNINGSPACE,(LPARAM)pNumSpaces,0);
}

// �`���[�j���O��Ԃ̎��
enum {
	TUNINGSPACE_UNKNOWN,		// �s��
	TUNINGSPACE_TERRESTRIAL,	// �n��f�W�^��
	TUNINGSPACE_BS,				// BS
	TUNINGSPACE_110CS			// 110�xCS
};

// �`���[�j���O��Ԃ̏��
struct TuningSpaceInfo {
	DWORD Size;			// �\���̂̃T�C�Y
	int Space;			// �`���[�j���O��Ԃ̎��(TUNINGSPACE_???)
						// �ꍇ�ɂ���Ă͐M�p�ł��Ȃ�
	WCHAR szName[64];	// �`���[�j���O��Ԗ�
};

// �`���[�j���O��Ԃ̏����擾����
// ���O�� TuningSpaceInfo �� Size �����o��ݒ肵�Ă���
inline bool MsgGetTuningSpaceInfo(PluginParam *pParam,int Index,TuningSpaceInfo *pInfo) {
	return (*pParam->Callback)(pParam,MESSAGE_GETTUNINGSPACENAME,Index,(LPARAM)pInfo)!=0;
}

// �`�����l�������ɐݒ肷��
inline bool MsgSetNextChannel(PluginParam *pParam,bool fNext=true)
{
	return (*pParam->Callback)(pParam,MESSAGE_SETNEXTCHANNEL,fNext,0)!=0;
}

#endif	// TVTEST_PLUGIN_VERSION>=TVTEST_PLUGIN_VERSION_0_0_1

#if TVTEST_PLUGIN_VERSION>=TVTEST_PLUGIN_VERSION_0_0_2

// ���݂̉����X�g���[�����擾����
// �����X�g���[���̐��� MESSAGE_GETSERVICEINFO �Ŏ擾�ł���
inline int MsgGetAudioStream(PluginParam *pParam)
{
	return (*pParam->Callback)(pParam,MESSAGE_GETAUDIOSTREAM,0,0);
}

// �����X�g���[����ݒ肷��
inline bool MsgSetAudioStream(PluginParam *pParam,int Index)
{
	return (*pParam->Callback)(pParam,MESSAGE_SETAUDIOSTREAM,Index,0)!=0;
}

#endif	// TVTEST_PLUGIN_VERSION>=TVTEST_PLUGIN_VERSION_0_0_2

#if TVTEST_PLUGIN_VERSION>=TVTEST_PLUGIN_VERSION_0_0_3

// �v���O�C���̗L����Ԃ��擾����
inline bool MsgIsPluginEnabled(PluginParam *pParam)
{
	return (*pParam->Callback)(pParam,MESSAGE_ISPLUGINENABLED,0,0)!=0;
}

// �R�}���h�̏��
struct CommandInfo {
	int ID;				// ���ʎq
	LPCWSTR pszText;	// �R�}���h�̕�����
	LPCWSTR pszName;	// �R�}���h�̖��O
};

// �R�}���h��o�^����
// TVTInitialize ���ŌĂт܂��B
// �ȉ��̂悤�Ɏg�p���܂��B
// MsgRegisterCommand(pParam, ID_MYCOMMAND, L"MyCommand", L"���̃R�}���h");
// �R�}���h�����s������ EVENT_COMMAND �C�x���g�������܂��B
// ���̍ہA�p�����[�^�Ƃ��Ď��ʎq���n����܂��B
inline bool MsgRegisterCommand(PluginParam *pParam,int ID,LPCWSTR pszText,LPCWSTR pszName)
{
	CommandInfo Info;
	Info.ID=ID;
	Info.pszText=pszText;
	Info.pszName=pszName;
	return (*pParam->Callback)(pParam,MESSAGE_REGISTERCOMMAND,(LPARAM)&Info,1)!=0;
}

// �R�}���h��o�^����
// TVTInitialize ���ŌĂт܂��B
inline bool MsgRegisterCommand(PluginParam *pParam,const CommandInfo *pCommandList,int NumCommands)
{
	return (*pParam->Callback)(pParam,MESSAGE_REGISTERCOMMAND,(LPARAM)pCommandList,NumCommands)!=0;
}

// ���O���L�^����
// �ݒ�̃��O�̍��ڂɕ\������܂��B
inline bool MsgAddLog(PluginParam *pParam,LPCWSTR pszText)
{
	return (*pParam->Callback)(pParam,MESSAGE_ADDLOG,(LPARAM)pszText,0)!=0;
}

#endif	// TVTEST_PLUGIN_VERSION>=TVTEST_PLUGIN_VERSION_0_0_3

#if TVTEST_PLUGIN_VERSION>=TVTEST_PLUGIN_VERSION_0_0_5

// �X�e�[�^�X(MESSAGE_GETSTATUS �Ŏ擾�ł�����e)�����Z�b�g����
// ���Z�b�g���s����� EVENT_STATUSRESET ��������
inline bool MsgResetStatus(PluginParam *pParam)
{
	return (*pParam->Callback)(pParam,MESSAGE_RESETSTATUS,0,0)!=0;
}

#endif	// TVTEST_PLUGIN_VERSION>=TVTEST_PLUGIN_VERSION_0_0_5


/*
	TVTest �A�v���P�[�V�����N���X

	TVTest �̊e��@�\���Ăяo�����߂̃N���X�B
	�����̃��b�p�[�Ȃ̂Ŏg��Ȃ��Ă������ł��B
	TVTInitialize �֐����Ă΂ꂽ���ɁA������ PluginParam �\���̂ւ̃|�C���^��
	�R���X�g���N�^�ɓn���ăC���X�^���X�𐶐����܂��B
*/
class CTVTestApp
{
protected:
	PluginParam *m_pParam;
public:
	CTVTestApp(PluginParam *pParam) : m_pParam(pParam) {}
	virtual ~CTVTestApp() {}
	HWND GetAppWindow() {
		return m_pParam->hwndApp;
	}
	DWORD GetVersion() {
		return MsgGetVersion(m_pParam);
	}
	bool QueryMessage(UINT Message) {
		return MsgQueryMessage(m_pParam,Message);
	}
	void *MemoryReAlloc(void *pData,DWORD Size) {
		return MsgMemoryReAlloc(m_pParam,pData,Size);
	}
	void *MemoryAlloc(DWORD Size) {
		return MsgMemoryAlloc(m_pParam,Size);
	}
	void MemoryFree(void *pData) {
		MsgMemoryFree(m_pParam,pData);
	}
	bool SetEventCallback(EventCallbackFunc Callback,void *pClientData=NULL) {
		return MsgSetEventCallback(m_pParam,Callback,pClientData);
	}
	bool GetCurrentChannelInfo(ChannelInfo *pInfo) {
		pInfo->Size=sizeof(ChannelInfo);
		return MsgGetCurrentChannelInfo(m_pParam,pInfo);
	}
	bool SetChannel(int Space,int Channel) {
		return MsgSetChannel(m_pParam,Space,Channel);
	}
	int GetService(int *pNumServices=NULL) {
		return MsgGetService(m_pParam,pNumServices);
	}
	bool SetService(int Service,bool fByID=false) {
		return MsgSetService(m_pParam,Service,fByID);
	}
	int GetTuningSpaceName(int Index,LPWSTR pszName,int MaxLength) {
		return MsgGetTuningSpaceName(m_pParam,Index,pszName,MaxLength);
	}
	bool GetChannelInfo(int Space,int Index,ChannelInfo *pInfo) {
		pInfo->Size=sizeof(ChannelInfo);
		return MsgGetChannelInfo(m_pParam,Space,Index,pInfo);
	}
	bool GetServiceInfo(int Index,ServiceInfo *pInfo) {
		pInfo->Size=sizeof(ServiceInfo);
		return MsgGetServiceInfo(m_pParam,Index,pInfo);
	}
	int GetDriverName(LPWSTR pszName,int MaxLength) {
		return MsgGetDriverName(m_pParam,pszName,MaxLength);
	}
	bool SetDriverName(LPCWSTR pszName) {
		return MsgSetDriverName(m_pParam,pszName);
	}
	bool StartRecord(RecordInfo *pInfo=NULL) {
		if (pInfo!=NULL)
			pInfo->Size=sizeof(RecordInfo);
		return MsgStartRecord(m_pParam,pInfo);
	}
	bool StopRecord() {
		return MsgStopRecord(m_pParam);
	}
	bool PauseRecord(bool fPause=true) {
		return MsgPauseRecord(m_pParam,fPause);
	}
	bool GetRecord(RecordInfo *pInfo) {
		pInfo->Size=sizeof(RecordInfo);
		return MsgGetRecord(m_pParam,pInfo);
	}
	bool ModifyRecord(RecordInfo *pInfo) {
		pInfo->Size=sizeof(RecordInfo);
		return MsgModifyRecord(m_pParam,pInfo);
	}
	int GetZoom() {
		return MsgGetZoom(m_pParam);
	}
	int SetZoom(int Num,int Denom=100) {
		return MsgSetZoom(m_pParam,Num,Denom);
	}
	bool GetPanScan(PanScanInfo *pInfo) {
		pInfo->Size=sizeof(PanScanInfo);
		return MsgGetPanScan(m_pParam,pInfo);
	}
	bool SetPanScan(PanScanInfo *pInfo) {
		pInfo->Size=sizeof(PanScanInfo);
		return MsgSetPanScan(m_pParam,pInfo);
	}
	bool GetStatus(StatusInfo *pInfo) {
		pInfo->Size=sizeof(StatusInfo);
		return MsgGetStatus(m_pParam,pInfo);
	}
	bool GetRecordStatus(RecordStatusInfo *pInfo) {
		pInfo->Size=sizeof(RecordStatusInfo);
		return MsgGetRecordStatus(m_pParam,pInfo);
	}
	bool GetVideoInfo(VideoInfo *pInfo) {
		pInfo->Size=sizeof(VideoInfo);
		return MsgGetVideoInfo(m_pParam,pInfo);
	}
	int GetVolume() {
		return MsgGetVolume(m_pParam);
	}
	bool SetVolume(int Volume) {
		return MsgSetVolume(m_pParam,Volume);
	}
	bool GetMute() {
		return MsgGetMute(m_pParam);
	}
	bool SetMute(bool fMute) {
		return MsgSetMute(m_pParam,fMute);
	}
	int GetStereoMode() {
		return MsgGetStereoMode(m_pParam);
	}
	bool SetStereoMode(int StereoMode) {
		return MsgSetStereoMode(m_pParam,StereoMode);
	}
	bool GetFullscreen() {
		return MsgGetFullscreen(m_pParam);
	}
	bool SetFullscreen(bool fFullscreen) {
		return MsgSetFullscreen(m_pParam,fFullscreen);
	}
	bool GetPreview() {
		return MsgGetPreview(m_pParam);
	}
	bool SetPreview(bool fPreview) {
		return MsgSetPreview(m_pParam,fPreview);
	}
	bool GetStandby() {
		return MsgGetStandby(m_pParam);
	}
	bool SetStandby(bool fStandby) {
		return MsgSetStandby(m_pParam,fStandby);
	}
	bool GetAlwaysOnTop() {
		return MsgGetAlwaysOnTop(m_pParam);
	}
	bool SetAlwaysOnTop(bool fAlwaysOnTop) {
		return MsgSetAlwaysOnTop(m_pParam,fAlwaysOnTop);
	}
	void *CaptureImage(DWORD Flags=0) {
		return MsgCaptureImage(m_pParam,Flags);
	}
	bool SaveImage() {
		return MsgSaveImage(m_pParam);
	}
	bool Reset() {
		return MsgReset(m_pParam);
	}
	bool Close(DWORD Flags=0) {
		return MsgClose(m_pParam,Flags);
	}
	bool SetStreamCallback(DWORD Flags,StreamCallbackFunc Callback,void *pClientData=NULL) {
		return MsgSetStreamCallback(m_pParam,Flags,Callback,pClientData);
	}
	bool EnablePlugin(bool fEnable) {
		return MsgEnablePlugin(m_pParam,fEnable);
	}
	COLORREF GetColor(LPCWSTR pszColor) {
		return MsgGetColor(m_pParam,pszColor);
	}
	bool DecodeARIBString(const void *pSrcData,DWORD SrcLength,
						  LPWSTR pszDest,DWORD DestLength) {
		return MsgDecodeARIBString(m_pParam,pSrcData,SrcLength,pszDest,DestLength);
	}
	bool GetCurrentProgramInfo(ProgramInfo *pInfo,bool fNext=false) {
		pInfo->Size=sizeof(ProgramInfo);
		return MsgGetCurrentProgramInfo(m_pParam,pInfo,fNext);
	}
#if TVTEST_PLUGIN_VERSION>=TVTEST_PLUGIN_VERSION_0_0_1
	bool QueryEvent(UINT Event) {
		return MsgQueryEvent(m_pParam,Event);
	}
	int GetTuningSpace(int *pNumSpaces=NULL) {
		return MsgGetTuningSpace(m_pParam,pNumSpaces);
	}
	bool GetTuningSpaceInfo(int Index,TuningSpaceInfo *pInfo) {
		pInfo->Size=sizeof(TuningSpaceInfo);
		return MsgGetTuningSpaceInfo(m_pParam,Index,pInfo);
	}
	bool SetNextChannel(bool fNext=true) {
		return MsgSetNextChannel(m_pParam,fNext);
	}
#endif
#if TVTEST_PLUGIN_VERSION>=TVTEST_PLUGIN_VERSION_0_0_2
	int GetAudioStream() {
		return MsgGetAudioStream(m_pParam);
	}
	bool SetAudioStream(int Index) {
		return MsgSetAudioStream(m_pParam,Index);
	}
#endif
#if TVTEST_PLUGIN_VERSION>=TVTEST_PLUGIN_VERSION_0_0_3
	bool IsPluginEnabled() {
		return MsgIsPluginEnabled(m_pParam);
	}
	bool RegisterCommand(int ID,LPCWSTR pszText,LPCWSTR pszName) {
		return MsgRegisterCommand(m_pParam,ID,pszText,pszName);
	}
	bool RegisterCommand(const CommandInfo *pCommandList,int NumCommands) {
		return MsgRegisterCommand(m_pParam,pCommandList,NumCommands);
	}
	bool AddLog(LPCWSTR pszText) {
		return MsgAddLog(m_pParam,pszText);
	}
#endif
#if TVTEST_PLUGIN_VERSION>=TVTEST_PLUGIN_VERSION_0_0_5
	bool ResetStatus() {
		return MsgResetStatus(m_pParam);
	}
#endif
};

/*
	TVTest �v���O�C���N���X

	�v���O�C�����N���X�Ƃ��ċL�q���邽�߂̒��ۃN���X�ł��B
	���̃N���X���e�v���O�C���Ŕh�������āA�v���O�C���̓��e���L�q���܂��B
	���̃N���X���g�킸�ɒ��ڃG�N�X�|�[�g�֐��������Ă������ł��B
*/
class CTVTestPlugin
{
protected:
	PluginParam *m_pPluginParam;
	CTVTestApp *m_pApp;
public:
	CTVTestPlugin() : m_pPluginParam(NULL),m_pApp(NULL) {}
	void SetPluginParam(PluginParam *pParam) {
		m_pPluginParam=pParam;
		m_pApp=new CTVTestApp(pParam);
	}
	virtual ~CTVTestPlugin() { delete m_pApp; }
	virtual DWORD GetVersion() { return TVTEST_PLUGIN_VERSION; }
	virtual bool GetPluginInfo(PluginInfo *pInfo)=0;
	virtual bool Initialize() { return true; }
	virtual bool Finalize() { return true; }
};

/*
	�C�x���g�n���h���N���X

	�C�x���g�����p�N���X�ł��B
	���̃N���X��h�������ăC�x���g�������s�����Ƃ��ł��܂��B
	�C�x���g�R�[���o�b�N�֐��Ƃ��ēo�^�����֐����� HandleEvent ���Ăт܂��B
	�������g��Ȃ��Ă������ł��B

	�ȉ��͎�����ł��B

	class CMyEventHandler : public TVTest::CTVTestEventHandler
	{
	public:
		virtual bool OnPluginEnable(bool fEnable) {
			if (fEnable) {
				if (MessageBox(NULL, TEXT("�L���ɂ����"), TEXT("�C�x���g"),
							   MB_OKCANCEL) != IDOK) {
					return false;
				}
			}
			return true;
		}
	};

	CMyEventHandler Handler;

	// ���̊֐����C�x���g�R�[���o�b�N�֐��Ƃ��ēo�^����Ă�����̂Ƃ��܂�
	LRESULT CALLBACK EventCallback(UINT Event,LPARAM lParam1,LPARAM lParam2,void *pClientData)
	{
		Handler.HandleEvent(Event,lParam1,lParam2,pClientData);
	}
*/
class CTVTestEventHandler
{
protected:
	void *m_pClientData;

	// �C�x���g�n���h���́A���ɋL�q�̖������͉̂������������� true ��Ԃ��܂�

	// �L����Ԃ��ω�����
	// �ω������ۂ���ꍇ false ��Ԃ��܂�
	virtual bool OnPluginEnable(bool fEnable) { return false; }
	// �ݒ���s��
	// �v���O�C���̃t���O�� PLUGIN_FLAG_HASSETTINGS ���ݒ肳��Ă���ꍇ�ɌĂ΂�܂�
	// �ݒ肪 OK ���ꂽ�� true ��Ԃ��܂�
	virtual bool OnPluginSettings(HWND hwndOwner) { return false; }
	// �`�����l�����ύX���ꂽ
	virtual bool OnChannelChange() { return false; }
	// �T�[�r�X���ύX���ꂽ
	virtual bool OnServiceChange() { return false; }
	// �h���C�o���ύX���ꂽ
	virtual bool OnDriverChange() { return false; }
	// �T�[�r�X�̍\�����ω�����
	virtual bool OnServiceUpdate() { return false; }
	// �^���Ԃ��ω�����
	virtual bool OnRecordStatusChange(int Status) { return false; }
	// �S��ʕ\����Ԃ��ω�����
	virtual bool OnFullscreenChange(bool fFullscreen) { return false; }
	// �v���r���[�\����Ԃ��ω�����
	virtual bool OnPreviewChange(bool fPreview) { return false; }
	// ���ʂ��ω�����
	virtual bool OnVolumeChange(int Volume,bool fMute) { return false; }
	// �X�e���I���[�h���ω�����
	virtual bool OnStereoModeChange(int StereoMode) { return false; }
	// �F�̐ݒ肪�ω�����
	virtual bool OnColorChange() { return false; }
#if TVTEST_PLUGIN_VERSION>=TVTEST_PLUGIN_VERSION_0_0_3
	// �ҋ@��Ԃ��ω�����
	virtual bool OnStandby(bool fStandby) { return false; }
	// �R�}���h���I�����ꂽ
	virtual bool OnCommand(int ID) { return false; }
#endif
#if TVTEST_PLUGIN_VERSION>=TVTEST_PLUGIN_VERSION_0_0_4
	// �����N���֎~���ɕ����N�����ꂽ
	virtual bool OnExecute(LPCWSTR pszCommandLine) { return false; }
#endif
#if TVTEST_PLUGIN_VERSION>=TVTEST_PLUGIN_VERSION_0_0_5
	// ���Z�b�g���ꂽ
	virtual bool OnReset() { return false; }
	// �X�e�[�^�X(MESSAGE_GETSTUATUS�Ŏ擾�ł�����e)�����Z�b�g���ꂽ
	virtual bool OnStatusReset() { return false; }
	// �����X�g���[�����ύX���ꂽ
	virtual bool OnAudioStreamChange(int Stream) { return false; }
#endif
public:
	virtual ~CTVTestEventHandler() {}
	LRESULT HandleEvent(UINT Event,LPARAM lParam1,LPARAM lParam2,void *pClientData) {
		m_pClientData=pClientData;
		switch (Event) {
		case EVENT_PLUGINENABLE:		return OnPluginEnable(lParam1!=0);
		case EVENT_PLUGINSETTINGS:		return OnPluginSettings((HWND)lParam1);
		case EVENT_CHANNELCHANGE:		return OnChannelChange();
		case EVENT_SERVICECHANGE:		return OnServiceChange();
		case EVENT_DRIVERCHANGE:		return OnDriverChange();
		case EVENT_SERVICEUPDATE:		return OnServiceUpdate();
		case EVENT_RECORDSTATUSCHANGE:	return OnRecordStatusChange(lParam1);
		case EVENT_FULLSCREENCHANGE:	return OnFullscreenChange(lParam1!=0);
		case EVENT_PREVIEWCHANGE:		return OnPreviewChange(lParam1!=0);
		case EVENT_VOLUMECHANGE:		return OnVolumeChange(lParam1,lParam2!=0);
		case EVENT_STEREOMODECHANGE:	return OnStereoModeChange(lParam1);
		case EVENT_COLORCHANGE:			return OnColorChange();
#if TVTEST_PLUGIN_VERSION>=TVTEST_PLUGIN_VERSION_0_0_3
		case EVENT_STANDBY:				return OnStandby(lParam1!=0);
		case EVENT_COMMAND:				return OnCommand(lParam1);
#endif
#if TVTEST_PLUGIN_VERSION>=TVTEST_PLUGIN_VERSION_0_0_4
		case EVENT_EXECUTE:				return OnExecute((LPCWSTR)lParam1);
#endif
#if TVTEST_PLUGIN_VERSION>=TVTEST_PLUGIN_VERSION_0_0_5
		case EVENT_RESET:				return OnReset();
		case EVENT_STATUSRESET:			return OnStatusReset();
		case EVENT_AUDIOSTREAMCHANGE:	return OnAudioStreamChange(lParam1);
#endif
		}
		return 0;
	}
};


}	// namespace TVTest


#include <poppack.h>


#ifdef TVTEST_PLUGIN_CLASS_IMPLEMENT
/*
	�v���O�C�����N���X�Ƃ��ċL�q�ł���悤�ɂ��邽�߂́A�G�N�X�|�[�g�֐��̎���
	�ł��B
	������g���΁A�G�N�X�|�[�g�֐��������Ŏ�������K�v���Ȃ��Ȃ�܂��B

	�v���O�C�����ł� CreatePluginClass �֐����������āACTVTestPlugin �N���X����
	�h���������v���O�C���N���X�̃C���X�^���X�� new �Ő������ĕԂ��܂��B�Ⴆ�΁A
	�ȉ��̂悤�ɏ����܂��B

	TVTest::CTVTestPlugin *CreatePluginClass()
	{
		return new CMyPluginClass;
	}
*/


TVTest::CTVTestPlugin *CreatePluginClass();

HINSTANCE g_hinstDLL;
TVTest::CTVTestPlugin *g_pPlugin;


BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
		g_hinstDLL=hinstDLL;
		g_pPlugin=CreatePluginClass();
		if (g_pPlugin==NULL)
			return FALSE;
		break;
	case DLL_PROCESS_DETACH:
		if (g_pPlugin) {
			delete g_pPlugin;
			g_pPlugin=NULL;
		}
		break;
	}
	return TRUE;
}

// �v���O�C���̏�������v���O�C���d�l�̃o�[�W������Ԃ�
// �v���O�C�������[�h�����ƍŏ��ɂ��̊֐����Ă΂�A
// �Ή����Ă��Ȃ��o�[�W�������Ԃ��ꂽ�ꍇ�͂����ɃA�����[�h����܂��B
TVTEST_EXPORT(DWORD) TVTGetVersion()
{
	return g_pPlugin->GetVersion();
}

// �v���O�C���̏����擾����
// TVTGetVersion �̎��ɌĂ΂��̂ŁA�v���O�C���̏��� PluginInfo �\���̂ɐݒ肵�܂��B
// FALSE ���Ԃ��ꂽ�ꍇ�A�����ɃA�����[�h����܂��B
TVTEST_EXPORT(BOOL) TVTGetPluginInfo(TVTest::PluginInfo *pInfo)
{
	return g_pPlugin->GetPluginInfo(pInfo);
}

// ���������s��
// TVTGetPluginInfo �̎��ɌĂ΂��̂ŁA�������������s���܂��B
// FALSE ���Ԃ��ꂽ�ꍇ�A�����ɃA�����[�h����܂��B
TVTEST_EXPORT(BOOL) TVTInitialize(TVTest::PluginParam *pParam)
{
	g_pPlugin->SetPluginParam(pParam);
	return g_pPlugin->Initialize();
}

// �I���������s��
// �v���O�C�����A�����[�h�����O�ɌĂ΂��̂ŁA�I���������s���܂��B
// ���̊֐����Ă΂��̂� TVTInitialize �֐��� TRUE ��Ԃ����ꍇ�����ł��B
TVTEST_EXPORT(BOOL) TVTFinalize()
{
	bool fOK=g_pPlugin->Finalize();
	delete g_pPlugin;
	g_pPlugin=NULL;
	return fOK;
}


#endif	// TVTEST_PLUGIN_CLASS_IMPLEMENT


#endif	// TVTEST_PLUGIN_H
