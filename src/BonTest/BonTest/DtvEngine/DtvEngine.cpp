// DtvEngine.cpp: CDtvEngine �N���X�̃C���v�������e�[�V����
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DtvEngine.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CDtvEngine �\�z/����
//////////////////////////////////////////////////////////////////////

CDtvEngine::CDtvEngine(void)
	: m_pDtvEngineHandler(NULL)
	, m_wCurService(0U)
	, m_BonSrcDecoder(this)
	, m_TsPacketParser(this)
	, m_TsDescrambler(this)
	, m_ProgManager(this)
	, m_MediaViewer(this)
	, m_MediaTee(this)
	, m_FileWriter(this)
	, m_FileReader(this)
	, m_bIsFileMode(false)
{

}

CDtvEngine::~CDtvEngine(void)
{
	CloseEngine();
}

const bool CDtvEngine::OpenEngine(CDtvEngineHandler *pDtvEngineHandler, HWND hHostHwnd)
{
	// ���S�Ɏb��
	CloseEngine();

	// �O���t�\���}
	//
	// m_BonSrcDecoder or m_FileReader
	//		��
	// m_TsPacketParser
	//		��
	// m_TsDescrambler
	//		��
	// m_MediaTee������������
	//		��				��
	// m_ProgManager	m_FileWriter
	//		��
	// m_MediaViewer

	// �f�R�[�_�O���t�\�z
	m_BonSrcDecoder.SetOutputDecoder(&m_TsPacketParser);	// Output #0 : CMediaData
	m_TsPacketParser.SetOutputDecoder(&m_TsDescrambler);	// Output #0 : CTsPacket
	m_TsDescrambler.SetOutputDecoder(&m_MediaTee);			// Output #0 : CTsPacket
	m_MediaTee.SetOutputDecoder(&m_ProgManager, 0UL);		// Output #0 : CTsPacket
	m_MediaTee.SetOutputDecoder(&m_FileWriter, 1UL);		// Output #1 : CTsPacket
	m_ProgManager.SetOutputDecoder(&m_MediaViewer);			// Output #0 : CTsPacket

	// ��U���Z�b�g
	ResetEngine();

	// �C�x���g�n���h���ݒ�
	m_pDtvEngineHandler = pDtvEngineHandler;

	try{
		// �`���[�i���J��
		if(!m_BonSrcDecoder.OpenTuner())throw 1UL;
	
		// B-CAS�J�[�h���J��
		if(!m_TsDescrambler.OpenBcasCard())throw 2UL;

		// �r���[�A���J��
		if(!m_MediaViewer.OpenViewer(hHostHwnd))throw 3UL;
		}
	catch(DWORD dwErrorNo){
		// �G���[
		switch(dwErrorNo){
			case 1UL :	::AfxMessageBox(TEXT("�`���[�i�̏������Ɏ��s���܂����B"));		break;
			case 2UL :	::AfxMessageBox(TEXT("B-CAS�J�[�h�̏������Ɏ��s���܂����B"));	break;
			case 3UL :	::AfxMessageBox(TEXT("DirectShow�̏������Ɏ��s���܂����B"));	break;
			}		
		
		CloseEngine();
		
		return false;
		}
	
	return true;
}

const bool CDtvEngine::CloseEngine(void)
{
	// �f�R�[�_�N���[�Y
	m_BonSrcDecoder.CloseTuner();
	m_TsDescrambler.CloseBcasCard();
	m_MediaViewer.CloseViewer();

	// �C�x���g�n���h������
	m_pDtvEngineHandler = NULL;

	return true;
}

const bool CDtvEngine::ResetEngine(void)
{
	// �f�R�[�_�O���t���Z�b�g
	m_BonSrcDecoder.Reset();

	return true;
}

const bool CDtvEngine::EnablePreview(const bool bEnable)
{
	if(bEnable){
		// �v���r���[�L��
		if(!m_BonSrcDecoder.Play())return false;
		if(!m_MediaViewer.Play())return false;
		}
	else{
		// �v���r���[����
		if(!m_MediaViewer.Stop())return false;
		if(!m_BonSrcDecoder.Stop())return false;
		}

	return true;
}

const bool CDtvEngine::SetChannel(const BYTE byTuningSpace, const WORD wChannel)
{
	// �`�����l���ύX
	return  m_BonSrcDecoder.SetChannel((BYTE)wChannel);
}

const bool CDtvEngine::SetService(const WORD wService)
{
	// �T�[�r�X�ύX
	if(wService < m_ProgManager.GetServiceNum()){
		WORD wVideoPID = 0xFFFF;
		WORD wAudioPID = 0xFFFF;
	
		m_wCurService = wService;
		m_ProgManager.GetServiceEsPID(&wVideoPID, &wAudioPID, m_wCurService);
		m_MediaViewer.SetVideoPID(wVideoPID);
		m_MediaViewer.SetAudioPID(wAudioPID);
		
		return true;
		}

	return false;
}

const WORD CDtvEngine::GetService(void) const
{
	// �T�[�r�X�擾
	return m_wCurService;
}

const bool CDtvEngine::PlayFile(LPCTSTR lpszFileName)
{
	// ���O���t�S�̂̔r��������������Ȃ��Ƃ܂Ƃ��ɓ����Ȃ��I�I

	// �Đ����̏ꍇ�͕���
	m_FileReader.StopReadAnsync();

	// �X���b�h�I����҂�
	while(m_FileReader.IsAnsyncReadBusy()){
		::Sleep(1UL);
		}

	m_FileReader.CloseFile();

	// �f�o�C�X����Đ����̏ꍇ�̓O���t���č\�z����
	if(!m_bIsFileMode){
		m_BonSrcDecoder.Stop();
		m_BonSrcDecoder.SetOutputDecoder(NULL);
		m_FileReader.SetOutputDecoder(&m_TsPacketParser);
		}

	try{
		// �O���t���Z�b�g
		m_FileReader.Reset();

		// �t�@�C���I�[�v��
		if(!m_FileReader.OpenFile(lpszFileName))throw 0UL;
		
		// �񓯊��Đ��J�n
		if(!m_FileReader.StartReadAnsync())throw 1UL;
		}
	catch(const DWORD dwErrorStep){
		// �G���[����
		StopFile();
		return false;
		}

	m_bIsFileMode = true;

	return true;
}

void CDtvEngine::StopFile(void)
{
	// ���O���t�S�̂̔r��������������Ȃ��Ƃ܂Ƃ��ɓ����Ȃ��I�I

	m_bIsFileMode = false;

	// �Đ����̏ꍇ�͕���
	m_FileReader.StopReadAnsync();
	
	// �X���b�h�I����҂�
	while(m_FileReader.IsAnsyncReadBusy()){
		::Sleep(1UL);
		}

	m_FileReader.CloseFile();
	
	// �O���t���č\�z����
	m_FileReader.SetOutputDecoder(NULL);
	m_BonSrcDecoder.SetOutputDecoder(&m_TsPacketParser);
	m_BonSrcDecoder.Play();
}

const DWORD CDtvEngine::GetLastError(void) const
{
	return 0UL;
}

const DWORD CDtvEngine::SendDtvEngineEvent(const DWORD dwEventID, PVOID pParam)
{
	// �C�x���g�n���h���ɃC�x���g�𑗐M����
	if(m_pDtvEngineHandler){
		return m_pDtvEngineHandler->OnDtvEngineEvent(this, dwEventID, pParam);
		}

	return 0UL;
}

const DWORD CDtvEngine::OnDecoderEvent(CMediaDecoder *pDecoder, const DWORD dwEventID, PVOID pParam)
{
	// �f�R�[�_����̃C�x���g���󂯎��(�b��)
	if(pDecoder == &m_ProgManager){
		
		// �v���O�����}�l�[�W������̃C�x���g
		switch(dwEventID){
			case CProgManager::EID_SERVICE_LIST_UPDATED :
				// �T�[�r�X�̍\�����ω�����
				SetService(0U);
				SendDtvEngineEvent(0UL, static_cast<PVOID>(&m_ProgManager));

				return 0UL;
				
			case CProgManager::EID_SERVICE_INFO_UPDATED :
				// �T�[�r�X�����X�V���ꂽ
				SendDtvEngineEvent(0UL, static_cast<PVOID>(&m_ProgManager));
				
				return 0UL;
			}
		}
	else if(pDecoder == &m_FileReader){
		
		CFileReader *pFileReader = dynamic_cast<CFileReader *>(pDecoder);
		
		// �t�@�C�����[�_����̃C�x���g
		switch(dwEventID){
			case CFileReader::EID_READ_ASYNC_START :
				// �񓯊����[�h�J�n
				return 0UL;
			
			case CFileReader::EID_READ_ASYNC_END :
				// �񓯊����[�h�I��
				return 0UL;
			
			case CFileReader::EID_READ_ASYNC_POSTREAD :
				// �񓯊����[�h��
				
				if(pFileReader->GetReadPos() >= pFileReader->GetFileSize()){
					// �ŏ��Ɋ����߂�(���[�v�Đ�)
					pFileReader->SetReadPos(0ULL);
					pFileReader->Reset();
					}
				
				return 0UL;
			}		
		}

	return 0UL;
}


//////////////////////////////////////////////////////////////////////
// CDtvEngineHandler �\�z/����
//////////////////////////////////////////////////////////////////////

const DWORD CDtvEngineHandler::OnDtvEngineEvent(CDtvEngine *pEngine, const DWORD dwEventID, PVOID pParam)
{
	// �f�t�H���g�̏���
	return 0UL;
}
