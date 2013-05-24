#include "stdafx.h"
#include "TVTest.h"
#include "CommandLine.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif




class CArgsParser {
	LPWSTR *m_ppszArgList;
	int m_Args;
	int m_CurPos;
public:
	CArgsParser(LPCWSTR pszCmdLine);
	~CArgsParser();
	bool IsSwitch() const;
	bool IsOption(LPCWSTR pszOption) const;
	bool GetOption(LPCWSTR pszOption,bool *pValue);
	bool GetOption(LPCWSTR pszOption,CDynamicString *pValue);
	bool GetOption(LPCWSTR pszOption,LPTSTR pszValue,int MaxLength);
	bool GetOption(LPCWSTR pszOption,int *pValue);
	bool GetOption(LPCWSTR pszOption,DWORD *pValue);
	bool GetDurationOption(LPCWSTR pszOption,DWORD *pValue);
	bool IsEnd() const { return m_CurPos>=m_Args; }
	bool Next();
	LPCWSTR GetText() const;
	bool GetText(LPWSTR pszText,int MaxLength) const;
	bool GetValue(int *pValue) const;
	bool GetValue(DWORD *pValue) const;
	bool GetDurationValue(DWORD *pValue) const;
};


CArgsParser::CArgsParser(LPCWSTR pszCmdLine)
{
	m_ppszArgList=::CommandLineToArgvW(pszCmdLine,&m_Args);
	if (m_ppszArgList==0)
		m_Args=0;
	m_CurPos=0;
}


CArgsParser::~CArgsParser()
{
	if (m_ppszArgList)
		::LocalFree(m_ppszArgList);
}


bool CArgsParser::IsSwitch() const
{
	if (IsEnd())
		return false;
	return m_ppszArgList[m_CurPos][0]=='-' || m_ppszArgList[m_CurPos][0]=='/';
}


bool CArgsParser::IsOption(LPCWSTR pszOption) const
{
	if (IsEnd())
		return false;
	return ::lstrcmpi(m_ppszArgList[m_CurPos]+1,pszOption)==0;
}


bool CArgsParser::GetOption(LPCWSTR pszOption,bool *pValue)
{
	if (IsOption(pszOption)) {
		*pValue=true;
		return true;
	}
	return false;
}


bool CArgsParser::GetOption(LPCWSTR pszOption,CDynamicString *pValue)
{
	if (IsOption(pszOption)) {
		if (Next())
			return pValue->Set(GetText());
	}
	return false;
}


bool CArgsParser::GetOption(LPCWSTR pszOption,LPTSTR pszValue,int MaxLength)
{
	if (IsOption(pszOption)) {
		if (Next())
			return GetText(pszValue,MaxLength);
	}
	return false;
}


bool CArgsParser::GetOption(LPCWSTR pszOption,int *pValue)
{
	if (IsOption(pszOption)) {
		if (Next())
			return GetValue(pValue);
	}
	return false;
}


bool CArgsParser::GetOption(LPCWSTR pszOption,DWORD *pValue)
{
	if (IsOption(pszOption)) {
		if (Next())
			return GetValue(pValue);
	}
	return false;
}


bool CArgsParser::GetDurationOption(LPCWSTR pszOption,DWORD *pValue)
{
	if (IsOption(pszOption)) {
		if (Next())
			return GetDurationValue(pValue);
	}
	return false;
}


bool CArgsParser::Next()
{
	m_CurPos++;
	return m_CurPos<m_Args;
}


LPCWSTR CArgsParser::GetText() const
{
	if (IsEnd())
		return TEXT("");
	return m_ppszArgList[m_CurPos];
}


bool CArgsParser::GetText(LPWSTR pszText,int MaxLength) const
{
	if (IsEnd())
		return false;
	if (::lstrlen(m_ppszArgList[m_CurPos])>=MaxLength)
		return false;
	::lstrcpy(pszText,m_ppszArgList[m_CurPos]);
	return true;
}


bool CArgsParser::GetValue(int *pValue) const
{
	if (IsEnd())
		return false;
	*pValue=_wtoi(m_ppszArgList[m_CurPos]);
	return true;
}


bool CArgsParser::GetValue(DWORD *pValue) const
{
	if (IsEnd())
		return false;
	*pValue=_wtoi(m_ppszArgList[m_CurPos]);
	return true;
}


bool CArgsParser::GetDurationValue(DWORD *pValue) const
{
	if (IsEnd())
		return false;

	// ?h?m?s �`���̎��Ԏw����p�[�X����
	// �P�ʂ̎w�肪�����ꍇ�͕b�P�ʂƉ��߂���
	LPCWSTR p=m_ppszArgList[m_CurPos];
	DWORD DurationSec=0,Duration=0;

	while (*p!='\0') {
		if (*p>='0' && *p<='9') {
			Duration=Duration*10+(*p-'0');
		} else {
			if (*p=='h') {
				DurationSec+=Duration*(60*60);
			} else if (*p=='m') {
				DurationSec+=Duration*60;
			} else if (*p=='s') {
				DurationSec+=Duration;
			}
			Duration=0;
		}
		p++;
	}
	DurationSec+=Duration;
	*pValue=DurationSec;

	return true;
}




CCommandLineParser::CCommandLineParser()
	: m_fNoDescramble(false)
	, m_fUseNetworkRemocon(false)
	, m_UDPPort(1234)
	, m_Channel(0)
	, m_ControllerChannel(0)
	, m_TuningSpace(-1)
	, m_ServiceID(0)
	, m_NetworkID(0)
	, m_TransportStreamID(0)
	, m_fRecord(false)
	, m_fRecordStop(false)
	, m_RecordDelay(0)
	, m_RecordDuration(0)
	, m_fRecordCurServiceOnly(false)
	, m_fExitOnRecordEnd(false)
	, m_fFullscreen(false)
	, m_fMinimize(false)
	, m_fMaximize(false)
	, m_fNoDriver(false)
	, m_fStandby(false)
	, m_fNoView(false)
	, m_fNoDirectShow(false)
	, m_fSilent(false)
	, m_fNoPlugin(false)
	, m_fSingleTask(false)
	, m_fInitialSettings(false)
	, m_fSaveLog(false)
	, m_fRecordOnly(false)
	, m_fNoEpg(false)
	, m_TvRockDID(0)
{
}


/*
	���p�\�ȃR�}���h���C���I�v�V����

	/ch				�`�����l�� (e.g. /ch 13)
	/chspace		�`���[�j���O��� (e.g. /chspace 1)
	/d				�h���C�o�̎w�� (e.g. /d BonDriver.dll)
	/f /fullscreen	�t���X�N���[��
	/ini			INI�t�@�C����
	/init			�����ݒ�_�C�A���O��\������
	/log			�I�����Ƀ��O��ۑ�����
	/max			�ő剻��ԂŋN��
	/min			�ŏ�����ԂŋN��
	/nd				�X�N�����u���������Ȃ�
	/nid			�l�b�g���[�NID
	/nodriver		BonDriver��ǂݍ��܂Ȃ�
	/nodshow		DirectShow�̏����������Ȃ�
	/noepg			EPG ���̎擾���s��Ȃ�
	/noplugin		�v���O�C����ǂݍ��܂Ȃ�
	/noview			�v���r���[����
	/nr				�l�b�g���[�N�����R�����g�p����
	/p /port		UDP �̃|�[�g�ԍ� (e.g. /p 1234)
	/plugin-		�w�肳�ꂽ�v���O�C����ǂݍ��܂Ȃ�
	/plugindir		�v���O�C���̃t�H���_
	/rch			�����R���`�����l��
	/rec			�^��
	/reccurservice	���݂̃T�[�r�X�̂ݘ^��
	/recdelay		�^��܂ł̎���(�b)
	/recduration	�^�掞��(�b)
	/recexit		�^��I�����Ƀv���O�������I��
	/recfile		�^��t�@�C����
	/reconly		�^���p���[�h
	/recstop		�^���~
	/s				�����N�����Ȃ�
	/sid			�T�[�r�XID
	/silent			�G���[���Ƀ_�C�A���O��\�����Ȃ�
	/standby		�ҋ@��ԂŋN��
	/tsid			�g�����X�|�[�g�X�g���[��ID
*/
void CCommandLineParser::Parse(LPCWSTR pszCmdLine)
{
	CArgsParser Args(pszCmdLine);

	if (Args.IsEnd())
		return;
	do {
		if (Args.IsSwitch()) {
			if (!Args.GetOption(TEXT("ch"),&m_Channel)
					&& !Args.GetOption(TEXT("chspace"),&m_TuningSpace)
					&& !Args.GetOption(TEXT("d"),&m_DriverName)
					&& !Args.GetOption(TEXT("f"),&m_fFullscreen)
					&& !Args.GetOption(TEXT("fullscreen"),&m_fFullscreen)
					&& !Args.GetOption(TEXT("ini"),&m_IniFileName)
					&& !Args.GetOption(TEXT("init"),&m_fInitialSettings)
					&& !Args.GetOption(TEXT("log"),&m_fSaveLog)
					&& !Args.GetOption(TEXT("max"),&m_fMaximize)
					&& !Args.GetOption(TEXT("min"),&m_fMinimize)
					&& !Args.GetOption(TEXT("nd"),&m_fNoDescramble)
					&& !Args.GetOption(TEXT("nodriver"),&m_fNoDriver)
					&& !Args.GetOption(TEXT("nodshow"),&m_fNoDirectShow)
					&& !Args.GetOption(TEXT("noepg"),&m_fNoEpg)
					&& !Args.GetOption(TEXT("noplugin"),&m_fNoPlugin)
					&& !Args.GetOption(TEXT("noview"),&m_fNoView)
					&& !Args.GetOption(TEXT("nr"),&m_fUseNetworkRemocon)
					&& !Args.GetOption(TEXT("nid"),&m_NetworkID)
					&& !Args.GetOption(TEXT("p"),&m_UDPPort)
					&& !Args.GetOption(TEXT("port"),&m_UDPPort)
					&& !Args.GetOption(TEXT("plugindir"),&m_PluginsDirectory)
					&& !Args.GetOption(TEXT("pluginsdir"),&m_PluginsDirectory)
					&& !Args.GetOption(TEXT("rec"),&m_fRecord)
					&& !Args.GetOption(TEXT("reccurservice"),&m_fRecordCurServiceOnly)
					&& !Args.GetDurationOption(TEXT("recdelay"),&m_RecordDelay)
					&& !Args.GetDurationOption(TEXT("recduration"),&m_RecordDuration)
					&& !Args.GetOption(TEXT("recexit"),&m_fExitOnRecordEnd)
					&& !Args.GetOption(TEXT("recfile"),&m_RecordFileName)
					&& !Args.GetOption(TEXT("reconly"),&m_fRecordOnly)
					&& !Args.GetOption(TEXT("recstop"),&m_fRecordStop)
					&& !Args.GetOption(TEXT("rch"),&m_ControllerChannel)
					&& !Args.GetOption(TEXT("s"),&m_fSingleTask)
					&& !Args.GetOption(TEXT("sid"),&m_ServiceID)
					&& !Args.GetOption(TEXT("silent"),&m_fSilent)
					&& !Args.GetOption(TEXT("standby"),&m_fStandby)
					&& !Args.GetOption(TEXT("tsid"),&m_TransportStreamID)) {
				if (Args.IsOption(TEXT("plugin-"))) {
					if (Args.Next()) {
						TCHAR szPlugin[MAX_PATH];
						if (Args.GetText(szPlugin,MAX_PATH))
							m_NoLoadPlugins.push_back(CDynamicString(szPlugin));
					}
				} else if (Args.IsOption(TEXT("did"))) {
					if (Args.Next()) {
						const TCHAR DID=Args.GetText()[0];

						if (DID>='A' && DID<='Z')
							m_TvRockDID=DID-'A';
						else if (DID>='a' && DID<='z')
							m_TvRockDID=DID-'a';
					}
				}
#ifdef _DEBUG
				else {
					TRACE(TEXT("Unknown command line option %s\n"),Args.GetText());
				}
#endif
			}
		} else {
			// �Ȃ���udp://@:1234�̂悤�Ƀ|�[�g���w��ł���Ǝv���Ă���l�������̂ŁA�Ή����Ă���
			if (::wcsncmp(Args.GetText(),L"udp://@:",8)==0)
				m_UDPPort=::_wtoi(Args.GetText()+8);
		}
	} while (Args.Next());
	if (m_fRecordOnly) {
		m_fNoDirectShow=true;
	}
}


bool CCommandLineParser::IsChannelSpecified() const
{
	return m_Channel>0 || m_ControllerChannel>0 || m_ServiceID>0
		|| m_NetworkID>0 || m_TransportStreamID>0;
}
