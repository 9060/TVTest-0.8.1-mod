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
	, m_wCurTransportStream(0U)
	, m_wCurService(0xFFFFU)
	, m_wCurVideoPID(0x1FFFU)
	, m_wCurAudioPID(0x1FFFU)
	, m_u64CurPcrTimeStamp(0UL)
	, m_BonSrcDecoder(this)
	, m_TsPacketParser(this)
	, m_TsDescrambler(this)
	, m_ProgManager(this)
	, m_MediaViewer(this)
	, m_MediaTee(this)
	, m_FileWriter(this)
	, m_FileReader(this)
	, m_MediaBuffer(this)
	, m_bBuiled(false)
	, m_bBuildComplete(false)
	, m_bIsFileMode(false)
	, m_bDescramble(true)
	, m_bDescrambleCurServiceOnly(false)
	, m_pTracer(NULL)
{
}


CDtvEngine::~CDtvEngine(void)
{
	CloseEngine();
}


const bool CDtvEngine::BuildEngine(CDtvEngineHandler *pDtvEngineHandler,
											bool bDescramble,bool bBuffering)
{
	// ���S�Ɏb��
	if (m_bBuiled)
		return true;

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

	Trace(TEXT("�f�R�[�_�O���t���\�z���Ă��܂�..."));

	// �f�R�[�_�O���t�\�z
	if (bDescramble) {
		// �f�X�N�����u����ʂ�
		m_TsPacketParser.SetOutputDecoder(&m_TsDescrambler);// Output #0 : CTsPacket
		m_TsDescrambler.SetOutputDecoder(&m_MediaTee);		// Output #0 : CTsPacket
	} else {
		// �f�X�N�����u����ʂ��Ȃ�
		m_TsPacketParser.SetOutputDecoder(&m_MediaTee);		// Output #0 : CTsPacket
	}
	m_bDescramble=bDescramble;
	m_MediaTee.SetOutputDecoder(&m_ProgManager, 0UL);		// Output #0 : CTsPacket
	m_MediaTee.SetOutputDecoder(&m_FileWriter, 1UL);		// Output #1 : CTsPacket
	if (!bBuffering) {
		m_ProgManager.SetOutputDecoder(&m_MediaViewer);			// Output #0 : CTsPacket
	} else {
		m_ProgManager.SetOutputDecoder(&m_MediaBuffer);
		m_MediaBuffer.SetOutputDecoder(&m_MediaViewer);
		m_MediaBuffer.Play();
	}

	// �C�x���g�n���h���ݒ�
	m_pDtvEngineHandler = pDtvEngineHandler;

	m_bBuiled=true;
	m_bBuildComplete=CheckBuildComplete();

	return true;
}


const bool CDtvEngine::CloseEngine(void)
{
	//if (!m_bBuiled)
	//	return true;

	//m_MediaViewer.Stop();
	ReleaseSrcFilter();
	m_MediaBuffer.Stop();
	// �f�R�[�_�N���[�Y
	//m_BonSrcDecoder.CloseTuner();	// ReleaseSrcFilter�Ŋ��ɕ����Ă���
	m_TsDescrambler.CloseBcasCard();
	m_MediaViewer.CloseViewer();

	// �C�x���g�n���h������
	m_pDtvEngineHandler = NULL;

	m_bBuiled=false;

	return true;
}


const bool CDtvEngine::ResetEngine(void)
{
	if (!m_bBuiled)
		return false;

	// �f�R�[�_�O���t���Z�b�g
	m_wCurTransportStream = 0;
	m_wCurVideoPID = m_wCurAudioPID = 0;
	if (m_bIsFileMode)
		m_FileReader.Reset();
	else
		m_BonSrcDecoder.Reset();

	return true;
}


const bool CDtvEngine::OpenSrcFilter_BonDriver(HMODULE hBonDriverDll)
{
	ReleaseSrcFilter();
	// �\�[�X�t�B���^���J��
	Trace(TEXT("�`���[�i���J���Ă��܂�..."));
	if (!m_BonSrcDecoder.OpenTuner(hBonDriverDll)) {
		m_bBuildComplete=false;
		return false;
	}
	m_MediaBuffer.SetFileMode(false);
	Trace(TEXT("�X�g���[���̍Đ����J�n���Ă��܂�..."));
	if (!m_BonSrcDecoder.Play()) {
		m_bBuildComplete=false;
		return false;
	}
	m_BonSrcDecoder.SetOutputDecoder(&m_TsPacketParser);
	//ResetEngine();
	ResetParams();

	m_bBuildComplete=CheckBuildComplete();

	m_bIsFileMode = false;
	return true;
}


const bool CDtvEngine::OpenSrcFilter_File(LPCTSTR lpszFileName)
{
	ReleaseSrcFilter();
	// �t�@�C�����J��
	if (!m_FileReader.OpenFile(lpszFileName)) {
		m_bBuildComplete=false;
		return false;
	}
	m_MediaBuffer.SetFileMode(true);
	if (!m_FileReader.StartReadAnsync()) {
		m_bBuildComplete=false;
		return false;
	}
	m_FileReader.SetOutputDecoder(&m_TsPacketParser);
	ResetEngine();

	m_bBuildComplete=CheckBuildComplete();

	m_bIsFileMode = true;
	return true;
}


#if 0
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
#endif


const bool CDtvEngine::ReleaseSrcFilter()
{
	// �\�[�X�t�B���^���J������
	if (m_bIsFileMode) {
		m_FileReader.StopReadAnsync();
		m_FileReader.CloseFile();
		m_FileReader.SetOutputDecoder(NULL);
	} else {
		if (m_BonSrcDecoder.IsOpen()) {
			m_BonSrcDecoder.CloseTuner();
			m_BonSrcDecoder.SetOutputDecoder(NULL);
		}
	}
	return true;
}


const bool CDtvEngine::IsSrcFilterOpen() const
{
	if (m_bIsFileMode)
		return m_FileReader.IsOpen();
	return m_BonSrcDecoder.IsOpen();
}


const bool CDtvEngine::EnablePreview(const bool bEnable)
{
	if (!m_MediaViewer.IsOpen())
		return false;

	bool bOK;

	if (bEnable) {
		// �v���r���[�L��
		bOK=m_MediaViewer.Play();
	} else {
		// �v���r���[����
		bOK=m_MediaViewer.Stop();
	}

	return bOK;
}


const bool CDtvEngine::SetViewSize(const int x,const int y)
{
	// �E�B���h�E�T�C�Y��ݒ肷��
	return m_MediaViewer.SetViewSize(x,y);
}


const bool CDtvEngine::SetVolume(const float fVolume)
{
	// �I�[�f�B�I�{�����[����ݒ肷��( -100.0(����) < fVolume < 0(�ő�) )
	return m_MediaViewer.SetVolume(fVolume);
}


const bool CDtvEngine::GetVideoSize(WORD *pwWidth,WORD *pwHeight)
{
	return m_MediaViewer.GetVideoSize(pwWidth,pwHeight);
}


const bool CDtvEngine::GetVideoAspectRatio(BYTE *pbyAspectRateX,BYTE *pbyAspectRateY)
{
	return m_MediaViewer.GetVideoAspectRatio(pbyAspectRateX,pbyAspectRateY);
}


const BYTE CDtvEngine::GetAudioChannelNum()
{
	return m_MediaViewer.GetAudioChannelNum();
}


const bool CDtvEngine::SetStereoMode(int iMode)
{
	return m_MediaViewer.SetStereoMode(iMode);
}


const bool CDtvEngine::GetVideoDecoderName(LPWSTR lpName,int iBufLen)
{
	return m_MediaViewer.GetVideoDecoderName(lpName, iBufLen);
}


const bool CDtvEngine::DisplayVideoDecoderProperty(HWND hWndParent)
{
	return m_MediaViewer.DisplayVideoDecoderProperty(hWndParent);
}


const bool CDtvEngine::SetChannel(const BYTE byTuningSpace, const WORD wChannel)
{
	if (m_MediaViewer.CheckHangUp(500))
		return false;
	// �`�����l���ύX
	bool bRet = m_BonSrcDecoder.SetChannel((DWORD)byTuningSpace, (DWORD)wChannel);
	//if(bRet) ResetEngine();
	if (bRet)
		ResetParams();
	return bRet;
}


const bool CDtvEngine::SetService(const WORD wService)
{
	// �T�[�r�X�ύX(wService==0xFFFF�Ȃ�PAT�擪�T�[�r�X)

	if(wService < m_ProgManager.GetServiceNum() || wService==0xFFFF){
		WORD wServiceID = 0xFFFF;
		WORD wVideoPID = 0xFFFF;
		WORD wAudioPID = 0xFFFF;

		// �擪PMT����������܂Ŏ��s�ɂ���
		if(!m_ProgManager.GetServiceID(&wServiceID,wService)) return false;
		if(wService==0xFFFF){
			m_wCurService = 0;
			m_wCurVideoPID = m_wCurAudioPID = 0;
		} else {
			m_wCurService = wService;
		}
		
		m_ProgManager.GetServiceEsPID(&wVideoPID, &wAudioPID, m_wCurService);

		TRACE(TEXT("------- Service Select -------\n"));
		TRACE(TEXT("ServiceID = %04X(%d)\n"),m_wCurService,wServiceID);

		if(m_wCurVideoPID != wVideoPID){
			m_MediaViewer.SetVideoPID(wVideoPID);
			m_wCurVideoPID = wVideoPID;
			TRACE(TEXT("Change Video PID = %04X\n"),wVideoPID);
			}
		if(m_wCurAudioPID != wAudioPID){
			m_MediaViewer.SetAudioPID(wAudioPID);
			m_wCurAudioPID = wAudioPID;
			TRACE(TEXT("Change Audio PID = %04X\n"),wAudioPID);
			}

		if (m_bDescrambleCurServiceOnly)
			SetDescrambleService(m_wCurService);

		return true;
		}

	return false;
}


const WORD CDtvEngine::GetService(void) const
{
	// �T�[�r�X�擾
	return m_wCurService;
}


const unsigned __int64 CDtvEngine::GetPcrTimeStamp() const
{
	// PCR�^�C���X�^���v�擾
	return m_u64CurPcrTimeStamp;
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
		WORD wTransportStream = m_ProgManager.GetTransportStreamID();
		switch(dwEventID){
			case CProgManager::EID_SERVICE_LIST_UPDATED :
				// �T�[�r�X�̍\�����ω�����				
				if(m_wCurTransportStream!=wTransportStream) {
					// �X�g���[��ID���ς���Ă���Ȃ珉����
					TRACE(TEXT("��Stream Change!! %04X\n"),wTransportStream);
					// ���̎��_�ł܂��T�[�r�X���S�����ĂȂ����Ƃ�����̂ŁA��������ۗ�
					if(SetService(0xFFFF)){
						m_wCurTransportStream = wTransportStream;
						SendDtvEngineEvent(EID_SERVICE_LIST_UPDATED, static_cast<PVOID>(&m_ProgManager));
						}
				} else {
					// �X�g���[��ID�͓��������A�\��ES��PID���ς�����\��������
					SetService(m_wCurService);
				}
				return 0UL;
				
			case CProgManager::EID_SERVICE_INFO_UPDATED :
				// �T�[�r�X�����X�V���ꂽ
				SendDtvEngineEvent(EID_SERVICE_INFO_UPDATED, static_cast<PVOID>(&m_ProgManager));
				
				return 0UL;
			case CProgManager::EID_PCR_TIMESTAMP_UPDATED :
				// �^�C���X�^���v���X�V���ꂽ
				if(m_wCurService!=0xFFFF){
					m_ProgManager.GetPcrTimeStamp(&m_u64CurPcrTimeStamp,m_wCurService);
					}
				SendDtvEngineEvent(EID_PCR_TIMESTAMP_UPDATED, static_cast<PVOID>(&m_ProgManager));
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
					ResetEngine();
					}
				
				return 0UL;
			}		
		}
	else if (pDecoder == &m_FileWriter) {
		switch (dwEventID) {
		case CFileWriter::EID_WRITE_ERROR:
			// �������݃G���[����������
			SendDtvEngineEvent(EID_FILE_WRITE_ERROR,pDecoder);
			return 0UL;
		}
	} else if (pDecoder == &m_MediaViewer) {
		switch (dwEventID) {
		case CMediaViewer::EID_FILTER_GRAPH_FLUSH:
			//ResetEngine();
			m_BonSrcDecoder.PurgeStream();
			return 0UL;
		}
	}

	return 0UL;
}


bool CDtvEngine::CheckBuildComplete() const
{
	return m_bBuiled && IsSrcFilterOpen() && m_MediaViewer.IsOpen()
		&& (!m_bDescramble || m_TsDescrambler.IsBcasCardOpen());
}


bool CDtvEngine::BuildMediaViewer(HWND hwndHost,HWND hwndMessage,
			CVideoRenderer::RendererType VideoRenderer,LPCWSTR pszMpeg2Decoder)
{
	if (!m_MediaViewer.OpenViewer(hwndHost,hwndMessage,VideoRenderer,pszMpeg2Decoder)) {
		SetError(m_MediaViewer.GetLastErrorException());
		m_bBuildComplete=false;
		return false;
	}
	m_bBuildComplete=CheckBuildComplete();
	if (m_bBuildComplete)
		ResetEngine();
	return true;
}


bool CDtvEngine::RebuildMediaViewer(HWND hwndHost,HWND hwndMessage,
			CVideoRenderer::RendererType VideoRenderer,LPCWSTR pszMpeg2Decoder)
{
	bool bOK;

	EnablePreview(false);
	m_ProgManager.SetOutputDecoder(NULL);
	m_MediaViewer.CloseViewer();
	bOK=m_MediaViewer.OpenViewer(hwndHost,hwndMessage,VideoRenderer,pszMpeg2Decoder);
	if (!bOK) {
		SetError(m_MediaViewer.GetLastErrorException());
		m_bBuildComplete=false;
	} else {
		m_bBuildComplete=CheckBuildComplete();
	}
	m_ProgManager.SetOutputDecoder(&m_MediaViewer);
	if (m_bBuildComplete)
		ResetEngine();
	return bOK;
}


bool CDtvEngine::OpenBcasCard(CCardReader::ReaderType CardReaderType)
{
	// B-CAS�J�[�h���J��
	if (CardReaderType!=CCardReader::READER_NONE) {
		Trace(TEXT("B-CAS�J�[�h���J���Ă��܂�..."));
		if (!m_TsDescrambler.OpenBcasCard(CardReaderType)) {
			SetError(0,TEXT("B-CAS�J�[�h�̏������Ɏ��s���܂����B"),
					 TEXT("�J�[�h���[�_���ڑ�����Ă��邩�A�ݒ�ŗL���ȃJ�[�h���[�_���I������Ă��邩�m�F���Ă��������B"));
			m_bBuildComplete=false;
			return false;
		}
	} else if (m_TsDescrambler.IsBcasCardOpen()) {
		m_TsDescrambler.CloseBcasCard();
	}
	m_bBuildComplete=CheckBuildComplete();
	if (m_bBuildComplete)
		ResetEngine();
	return true;
}


bool CDtvEngine::SetDescramble(bool bDescramble)
{
	if (!m_bBuiled) {
		SetError(0,TEXT("�����G���[ : DtvEngine���\�z����Ă��܂���B"));
		return false;
	}

	if (m_bDescramble==bDescramble)
		return true;

	if (bDescramble) {
		m_TsPacketParser.SetOutputDecoder(&m_TsDescrambler);
		m_TsDescrambler.SetOutputDecoder(&m_MediaTee);
	} else {
		m_TsPacketParser.SetOutputDecoder(&m_MediaTee);
	}
	m_bDescramble=bDescramble;
	m_bBuildComplete=CheckBuildComplete();
	return true;
}


bool CDtvEngine::ResetBuffer()
{
	m_MediaBuffer.ResetBuffer();
	return true;
}


bool CDtvEngine::GetOriginalVideoSize(WORD *pWidth,WORD *pHeight) const
{
	return m_MediaViewer.GetOriginalVideoSize(pWidth,pHeight);
}


bool CDtvEngine::SetDescrambleService(WORD Service)
{
	if (Service!=0xFFFF) {
		WORD TargetPIDs[2] = {0xFFFF,0xFFFF};

		if (!m_ProgManager.GetServiceEsPID(&TargetPIDs[0], &TargetPIDs[1], Service))
			return false;
		return m_TsDescrambler.SetTargetPID(TargetPIDs,2);
	}
	return m_TsDescrambler.SetTargetPID();
}


bool CDtvEngine::SetDescrambleCurServiceOnly(bool bOnly)
{
	if (m_bDescrambleCurServiceOnly!=bOnly) {
		m_bDescrambleCurServiceOnly=bOnly;
		SetDescrambleService(bOnly?m_wCurService:0xFFFF);
	}
	return true;
}


CEpgDataInfo *CDtvEngine::GetEpgDataInfo(WORD wSID, bool bNext)
{
	WORD wID = 0;

	m_ProgManager.GetServiceID(&wID,wSID);
	return m_TsPacketParser.GetEpgDataInfo(wID,bNext);
}


bool CDtvEngine::SetTracer(CTracer *pTracer)
{
	m_pTracer=pTracer;
	m_MediaViewer.SetTracer(pTracer);
	return true;
}


void CDtvEngine::Trace(LPCTSTR pszOutput, ...)
{
	va_list Args;

	va_start(Args,pszOutput);
	if (m_pTracer!=NULL)
		m_pTracer->TraceV(pszOutput,Args);
	va_end(Args);
}


void CDtvEngine::ResetParams()
{
	m_wCurTransportStream = 0;
	m_wCurVideoPID = m_wCurAudioPID = 0;
}


//////////////////////////////////////////////////////////////////////
// CDtvEngineHandler �\�z/����
//////////////////////////////////////////////////////////////////////

const DWORD CDtvEngineHandler::OnDtvEngineEvent(CDtvEngine *pEngine, const DWORD dwEventID, PVOID pParam)
{
	// �f�t�H���g�̏���
	return 0UL;
}
