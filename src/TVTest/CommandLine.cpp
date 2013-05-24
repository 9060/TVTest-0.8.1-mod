#include "stdafx.h"
#include "TVTest.h"
#include "CommandLine.h"




class CArgsParser {
	LPWSTR *m_ppszArgList;
	int m_Args;
	int m_CurPos;
public:
	CArgsParser(LPCWSTR pszCmdLine);
	~CArgsParser();
	bool IsSwitch() const;
	bool IsOption(LPCWSTR pszOption) const;
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
{
	m_szIniFileName[0]='\0';
	m_szDriverName[0]='\0';
	m_fNoDescramble=false;
	m_fUseNetworkRemocon=false;
	m_UDPPort=1234;
	m_Channel=0;
	m_ControllerChannel=0;
	m_TuningSpace=-1;
	m_ServiceID=0;
	m_NetworkID=0;
	m_TransportStreamID=0;
	m_fRecord=false;
	m_RecordDelay=0;
	m_RecordDuration=0;
	m_szRecordFileName[0]='\0';
	m_fRecordCurServiceOnly=false;
	m_fExitOnRecordEnd=false;
	m_fRecordStop=false;
	m_fFullscreen=false;
	m_fMinimize=false;
	m_fNoDriver=false;
	m_fStandby;
	m_fNoView=false;
	m_fNoDirectShow=false;
	m_fSilent=false;
	m_fNoPlugin=false;
	m_fSchedule=false;
	m_fInitialSettings=false;
	m_fSaveLog=false;
	m_fRecordOnly=false;
	m_TvRockDID=0;
}


/*
	���p�\�ȃR�}���h���C���I�v�V����

	/ch				�`�����l�� (e.g. /ch 13)
	/chspace		�`���[�j���O��� (e.g. /chspace 1)
	/d				�h���C�o�̎w�� (e.g. /d BonDriver.dll)
	/f				�t���X�N���[��
	/ini			INI�t�@�C����
	/init			�����ݒ�_�C�A���O��\������
	/log			�I�����Ƀ��O��ۑ�����
	/min			�ŏ�����ԂŋN��
	/nd				�X�N�����u���������Ȃ�
	/nid			�l�b�g���[�NID
	/nodriver		BonDriver��ǂݍ��܂Ȃ�
	/nodshow		DirectShow�̏����������Ȃ�
	/noplugin		�v���O�C����ǂݍ��܂Ȃ�
	/noview			�v���r���[����
	/nr				�l�b�g���[�N�����R�����g�p����
	/p				UDP �̃|�[�g�ԍ� (e.g. /p 1234)
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
			if (Args.IsOption(TEXT("ch"))) {
				if (Args.Next())
					Args.GetValue(&m_Channel);
			} else if (Args.IsOption(TEXT("chspace"))) {
				if (Args.Next())
					Args.GetValue(&m_TuningSpace);
			} else if (Args.IsOption(TEXT("d"))) {
				if (Args.Next())
					Args.GetText(m_szDriverName,MAX_PATH);
			} else if (Args.IsOption(TEXT("fullscreen"))
					|| Args.IsOption(TEXT("f"))) {
				m_fFullscreen=true;
			} else if (Args.IsOption(TEXT("ini"))) {
				if (Args.Next())
					Args.GetText(m_szIniFileName,MAX_PATH);
			} else if (Args.IsOption(TEXT("init"))) {
				m_fInitialSettings=true;
			} else if (Args.IsOption(TEXT("log"))) {
				m_fSaveLog=true;
			} else if (Args.IsOption(TEXT("min"))) {
				m_fMinimize=true;
			} else if (Args.IsOption(TEXT("nd"))) {
				m_fNoDescramble=true;
			} else if (Args.IsOption(TEXT("nid"))) {
				if (Args.Next())
					Args.GetValue(&m_NetworkID);
			} else if (Args.IsOption(TEXT("nodriver"))) {
				m_fNoDriver=true;
			} else if (Args.IsOption(TEXT("nodshow"))) {
				m_fNoDirectShow=true;
			} else if (Args.IsOption(TEXT("noplugin"))) {
				m_fNoPlugin=true;
			} else if (Args.IsOption(TEXT("noview"))) {
				m_fNoView=true;
			} else if (Args.IsOption(TEXT("nr"))) {
				m_fUseNetworkRemocon=true;
			} else if (Args.IsOption(TEXT("p"))
					|| Args.IsOption(TEXT("port"))) {
				if (Args.Next())
					Args.GetValue(&m_UDPPort);
			} else if (Args.IsOption(TEXT("rec"))) {
				m_fRecord=true;
			} else if (Args.IsOption(TEXT("reccurservice"))) {
				m_fRecordCurServiceOnly=true;
			} else if (Args.IsOption(TEXT("recdelay"))) {
				if (Args.Next())
					Args.GetDurationValue(&m_RecordDelay);
			} else if (Args.IsOption(TEXT("recduration"))) {
				if (Args.Next())
					Args.GetDurationValue(&m_RecordDuration);
			} else if (Args.IsOption(TEXT("recexit"))) {
				m_fExitOnRecordEnd=true;
			} else if (Args.IsOption(TEXT("recfile"))) {
				if (Args.Next())
					Args.GetText(m_szRecordFileName,MAX_PATH);
			} else if (Args.IsOption(TEXT("reconly"))) {
				m_fRecordOnly=true;
			} else if (Args.IsOption(TEXT("recstop"))) {
				m_fRecordStop=true;
			} else if (Args.IsOption(TEXT("rch"))) {
				if (Args.Next())
					Args.GetValue(&m_ControllerChannel);
			} else if (Args.IsOption(TEXT("schedule"))
					|| Args.IsOption(TEXT("s"))) {
				m_fSchedule=true;
			} else if (Args.IsOption(TEXT("sid"))) {
				if (Args.Next())
					Args.GetValue(&m_ServiceID);
			} else if (Args.IsOption(TEXT("silent"))) {
				m_fSilent=true;
			} else if (Args.IsOption(TEXT("standby"))) {
				m_fStandby=true;
			} else if (Args.IsOption(TEXT("tsid"))) {
				if (Args.Next())
					Args.GetValue(&m_TransportStreamID);
			} else if (Args.IsOption(TEXT("did"))) {
				if (Args.Next()) {
					int DID;

					Args.GetValue(&DID);
					if (DID>='A' && DID<='Z')
						m_TvRockDID=DID-'A';
					else if (DID>='a' && DID<='z')
						m_TvRockDID=DID='a';
				}
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
