#include "stdafx.h"
#include "CommandLine.h"




CCommandLineParser::CCommandLineParser()
{
	m_szDriverName[0]='\0';
	m_fNoDescramble=false;
	m_fUseNetworkRemocon=false;
	m_UDPPort=1234;
	m_Channel=0;
	m_ControllerChannel=0;
	m_ServiceID=0;
	m_fRecord=false;
	m_RecordDelay=0;
	m_RecordDuration=0;
	m_szRecordFileName[0]='\0';
	m_fFullscreen=false;
	m_fNoView=false;
	m_fSchedule=false;
	m_fInitialSettings=false;
	m_fSaveLog=false;
}


/*
	���p�\�ȃR�}���h���C���I�v�V����

	/ch				�`�����l�� (e.g. /ch 13)
	/d				�h���C�o�̎w�� (e.g. /d BonDriver.dll)
	/f				�t���X�N���[��
	/init			�����ݒ�_�C�A���O��\������
	/log			�I�����Ƀ��O��ۑ�����
	/nd				�X�N�����u���������Ȃ�
	/nr				�l�b�g���[�N�����R�����g�p����
	/p				UDP �̃|�[�g�ԍ� (e.g. /p 1234)
	/rch			�����R���`�����l��
	/rec			�^��
	/recdelay		�^��܂ł̎���(�b)
	/recduration	�^�掞��(�b)
	/recfile		�^��t�@�C����
	/s				�����N�����Ȃ�
	/sid			�T�[�r�XID
*/
void CCommandLineParser::Parse(LPCWSTR pszCmdLine)
{
	LPWSTR *ppszArgList;
	int Args;

	ppszArgList=::CommandLineToArgvW(pszCmdLine,&Args);
	if (ppszArgList!=NULL) {
		int i;

		for (i=0;i<Args;i++) {
			if (ppszArgList[i][0]=='/' || ppszArgList[i][0]=='-') {
				if (::lstrcmpi(ppszArgList[i]+1,TEXT("ch"))==0) {
					i++;
					if (i<Args)
						m_Channel=::_wtoi(ppszArgList[i]);
				} else if (::lstrcmpi(ppszArgList[i]+1,TEXT("d"))==0) {
					i++;
					if (i<Args && ::lstrlen(ppszArgList[i])<MAX_PATH)
						::lstrcpy(m_szDriverName,ppszArgList[i]);
				} else if (::lstrcmpi(ppszArgList[i]+1,TEXT("fullscreen"))==0
						|| ::lstrcmpi(ppszArgList[i]+1,TEXT("f"))==0) {
					m_fFullscreen=true;
				} else if (::lstrcmpi(ppszArgList[i]+1,TEXT("init"))==0) {
					m_fInitialSettings=true;
				} else if (::lstrcmpi(ppszArgList[i]+1,TEXT("log"))==0) {
					m_fSaveLog=true;
				} else if (::lstrcmpi(ppszArgList[i]+1,TEXT("nd"))==0) {
					m_fNoDescramble=true;
				} else if (::lstrcmpi(ppszArgList[i]+1,TEXT("noview"))==0) {
					m_fNoView=true;
				} else if (::lstrcmpi(ppszArgList[i]+1,TEXT("nr"))==0) {
					m_fUseNetworkRemocon=true;
				} else if (::lstrcmpi(ppszArgList[i]+1,TEXT("p"))==0
						|| ::lstrcmpi(ppszArgList[i]+1,TEXT("port"))==0) {
					i++;
					if (i<Args)
						m_UDPPort=::_wtoi(ppszArgList[i]);
				} else if (::lstrcmpi(ppszArgList[i]+1,TEXT("rec"))==0) {
					m_fRecord=true;
				} else if (::lstrcmpi(ppszArgList[i]+1,TEXT("recdelay"))==0) {
					i++;
					if (i<Args)
						m_RecordDelay=::_wtoi(ppszArgList[i]);
				} else if (::lstrcmpi(ppszArgList[i]+1,TEXT("recduration"))==0) {
					i++;
					if (i<Args)
						m_RecordDuration=::_wtoi(ppszArgList[i]);
				} else if (::lstrcmpi(ppszArgList[i]+1,TEXT("recfile"))==0) {
					i++;
					if (i<Args && ::lstrlen(ppszArgList[i])<MAX_PATH)
						::lstrcpy(m_szRecordFileName,ppszArgList[i]);
				} else if (::lstrcmpi(ppszArgList[i]+1,TEXT("rch"))==0) {
					i++;
					if (i<Args)
						m_ControllerChannel=::_wtoi(ppszArgList[i]);
				} else if (::lstrcmpi(ppszArgList[i]+1,TEXT("schedule"))==0
						|| ::lstrcmpi(ppszArgList[i]+1,TEXT("s"))==0) {
					m_fSchedule=true;
				} else if (::lstrcmpi(ppszArgList[i]+1,TEXT("sid"))==0) {
					i++;
					if (i<Args)
						m_ServiceID=::_wtoi(ppszArgList[i]);
				}
			} else if (::wcsncmp(ppszArgList[i],L"udp://@:",8)==0) {
				// �Ȃ���udp://@:1234�̂悤�Ƀ|�[�g���w��ł���Ǝv���Ă���l�������̂ŁA�Ή����Ă���
				m_UDPPort=::_wtoi(ppszArgList[i]+8);
			}
		}
		::LocalFree(ppszArgList);
	}
}
