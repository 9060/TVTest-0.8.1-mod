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
	, m_SpecServiceID(0xFFFF)
	, m_CurAudioStream(0)
	, m_u64CurPcrTimeStamp(0UL)
	, m_BonSrcDecoder(this)
	, m_TsPacketParser(this)
	, m_TsAnalyzer(this)
	, m_TsDescrambler(this)
	, m_ProgManager(this)
	, m_MediaViewer(this)
	, m_MediaTee(this)
	, m_FileWriter(this)
	, m_FileReader(this)
	, m_MediaBuffer(this)
	, m_MediaGrabber(this)
	, m_TsSelector(this)
	, m_bBuiled(false)
	, m_bBuildComplete(false)
	, m_bIsFileMode(false)
	, m_bDescramble(true)
	, m_bBuffering(false)
	, m_bDescrambleCurServiceOnly(false)
	, m_bWriteCurServiceOnly(false)
{
}


CDtvEngine::~CDtvEngine(void)
{
	CloseEngine();
}


const bool CDtvEngine::BuildEngine(CDtvEngineHandler *pDtvEngineHandler,
								   bool bDescramble, bool bBuffering)
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
	m_TsPacketParser.SetOutputDecoder(&m_TsAnalyzer);
	m_TsAnalyzer.SetOutputDecoder(&m_TsDescrambler);// Output #0 : CTsPacket
	m_TsDescrambler.SetOutputDecoder(&m_MediaTee);		// Output #0 : CTsPacket
	m_TsDescrambler.EnableDescramble(bDescramble);
	m_bDescramble = bDescramble;
	m_MediaTee.SetOutputDecoder(&m_ProgManager, 0UL);		// Output #0 : CTsPacket
	m_MediaTee.SetOutputDecoder(&m_MediaGrabber, 1UL);
	m_MediaGrabber.SetOutputDecoder(&m_FileWriter);
	if (!bBuffering) {
		m_ProgManager.SetOutputDecoder(&m_MediaViewer);		// Output #0 : CTsPacket
	} else {
		m_ProgManager.SetOutputDecoder(&m_MediaBuffer);
		m_MediaBuffer.SetOutputDecoder(&m_MediaViewer);
		m_MediaBuffer.Play();
	}
	m_bBuffering=bBuffering;

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

	Trace(TEXT("DtvEngine����Ă��܂�..."));

	//m_MediaViewer.Stop();

	ReleaseSrcFilter();

	Trace(TEXT("�o�b�t�@�̃X�g���[�~���O���~���Ă��܂�..."));
	m_MediaBuffer.Stop();

	Trace(TEXT("�J�[�h���[�_����Ă��܂�..."));
	m_TsDescrambler.CloseBcasCard();

	Trace(TEXT("���f�B�A�r���[�A����Ă��܂�..."));
	m_MediaViewer.CloseViewer();

	// �C�x���g�n���h������
	m_pDtvEngineHandler = NULL;

	m_bBuiled=false;

	Trace(TEXT("DtvEngine����܂����B"));

	return true;
}


const bool CDtvEngine::ResetEngine(void)
{
	if (!m_bBuiled)
		return false;

	// �f�R�[�_�O���t���Z�b�g
	ResetStatus();
	if (m_bIsFileMode)
		m_FileReader.ResetGraph();
	else
		m_BonSrcDecoder.ResetGraph();

	return true;
}


const bool CDtvEngine::OpenSrcFilter_BonDriver(HMODULE hBonDriverDll)
{
	ReleaseSrcFilter();
	// �\�[�X�t�B���^���J��
	Trace(TEXT("�`���[�i���J���Ă��܂�..."));
	if (!m_BonSrcDecoder.OpenTuner(hBonDriverDll)) {
		SetError(m_BonSrcDecoder.GetLastErrorException());
		m_bBuildComplete=false;
		return false;
	}
	m_MediaBuffer.SetFileMode(false);
	m_BonSrcDecoder.SetOutputDecoder(&m_TsPacketParser);
	Trace(TEXT("�X�g���[���̍Đ����J�n���Ă��܂�..."));
	if (!m_BonSrcDecoder.Play()) {
		SetError(m_BonSrcDecoder.GetLastErrorException());
		m_bBuildComplete=false;
		return false;
	}
	//ResetEngine();
	ResetStatus();

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
	m_FileReader.SetOutputDecoder(&m_TsPacketParser);
	if (!m_FileReader.StartReadAnsync()) {
		m_bBuildComplete=false;
		return false;
	}
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


const int CDtvEngine::GetAudioStreamNum(const WORD wService)
{
	return m_ProgManager.GetAudioEsNum(wService);
}


const bool CDtvEngine::SetAudioStream(int StreamIndex)
{
	if (StreamIndex == m_CurAudioStream)
		return true;
	if (StreamIndex < 0 || StreamIndex >= GetAudioStreamNum(m_wCurService))
		return false;

	WORD wAudioPID;

	if (!m_ProgManager.GetAudioEsPID(&wAudioPID, StreamIndex, m_wCurService))
		return false;

	if (!m_MediaViewer.SetAudioPID(wAudioPID))
		return false;

	m_CurAudioStream = StreamIndex;

	return true;
}


const int CDtvEngine::GetAudioStream() const
{
	return m_CurAudioStream;
}


const BYTE CDtvEngine::GetAudioComponentType()
{
	return m_ProgManager.GetAudioComponentType(m_CurAudioStream, m_wCurService);
}


const bool CDtvEngine::SetStereoMode(int iMode)
{
	return m_MediaViewer.SetStereoMode(iMode);
}


const WORD CDtvEngine::GetEventID()
{
	return m_ProgManager.GetEventID(m_wCurService);
}


const int CDtvEngine::GetEventName(LPTSTR pszName, int MaxLength, bool fNext)
{
	return m_ProgManager.GetEventName(m_wCurService,pszName,MaxLength,fNext);
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
	// �`�����l���ύX
	m_SpecServiceID = 0xFFFF;
	if (!m_BonSrcDecoder.SetChannel((DWORD)byTuningSpace, (DWORD)wChannel)) {
		SetError(m_BonSrcDecoder.GetLastErrorException());
		return false;
	}
	ResetStatus();
	return true;
}


const bool CDtvEngine::SetService(const WORD wService)
{
	CBlockLock Lock(&m_EngineLock);

	// �T�[�r�X�ύX(wService==0xFFFF�Ȃ�PAT�擪�T�[�r�X)

	if (wService == 0xFFFF || wService < m_ProgManager.GetServiceNum()) {
		WORD wServiceID = 0xFFFF;
		WORD wVideoPID = CMediaViewer::PID_INVALID;
		WORD wAudioPID = CMediaViewer::PID_INVALID;

		if (wService == 0xFFFF) {
			m_wCurService = 0;
		} else {
			m_wCurService = wService;
		}
		// �擪PMT����������܂Ŏ��s�ɂ���
		if (!m_ProgManager.GetServiceID(&wServiceID, wService))
			return false;

		m_ProgManager.GetVideoEsPID(&wVideoPID, m_wCurService);
		if (!m_ProgManager.GetAudioEsPID(&wAudioPID, m_CurAudioStream, m_wCurService)
				&& m_CurAudioStream != 0) {
			m_ProgManager.GetAudioEsPID(&wAudioPID, 0, m_wCurService);
			m_CurAudioStream = 0;
		}

		TRACE(TEXT("------- Service Select -------\n"));
		TRACE(TEXT("%d (ServiceID = %04X)\n"), m_wCurService, wServiceID);

		m_MediaViewer.SetVideoPID(wVideoPID);
		m_MediaViewer.SetAudioPID(wAudioPID);

		if (m_bDescrambleCurServiceOnly)
			SetDescrambleService(m_wCurService);

		if (m_bWriteCurServiceOnly)
			SetWriteService(m_wCurService);

		return true;
	}

	return false;
}


const WORD CDtvEngine::GetService(void) const
{
	// �T�[�r�X�擾
	return m_wCurService;
}


const bool CDtvEngine::GetServiceID(WORD *pServiceID)
{
	// �T�[�r�XID�擾
	return m_ProgManager.GetServiceID(pServiceID, m_wCurService);
}


const bool CDtvEngine::SetServiceByID(const WORD ServiceID)
{
	WORD Index;

	m_SpecServiceID = ServiceID;
	Index = m_ProgManager.GetServiceIndexByID(ServiceID);
	if (Index != 0xFFFF)
		SetService(Index);
	return true;
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
	if (pDecoder == &m_ProgManager) {
		// �v���O�����}�l�[�W������̃C�x���g
		WORD wTransportStream = m_ProgManager.GetTransportStreamID();
		switch (dwEventID) {
		case CProgManager::EID_SERVICE_LIST_UPDATED :
			// �T�[�r�X�̍\�����ω�����
			m_CurAudioStream = 0;
			if (m_wCurTransportStream != wTransportStream) {
				// �X�g���[��ID���ς���Ă���Ȃ珉����
				TRACE(TEXT("��Stream Change!! %04X\n"),wTransportStream);
				// ���̎��_�ł܂��T�[�r�X���S�����ĂȂ����Ƃ�����̂ŁA��������ۗ�
				WORD Service;
				if (m_SpecServiceID != 0xFFFF)
					Service = m_ProgManager.GetServiceIndexByID(m_SpecServiceID);
				else
					Service = 0xFFFF;
				if (SetService(Service)) {
					m_wCurTransportStream = wTransportStream;
					SendDtvEngineEvent(EID_SERVICE_LIST_UPDATED, &m_ProgManager);
				}
			} else {
				// �X�g���[��ID�͓��������A�\��ES��PID���ς�����\��������
				SetService(m_wCurService);
			}
			return 0UL;

		case CProgManager::EID_SERVICE_INFO_UPDATED :
			// �T�[�r�X�����X�V���ꂽ
			SendDtvEngineEvent(EID_SERVICE_INFO_UPDATED, &m_ProgManager);
			return 0UL;

		case CProgManager::EID_PCR_TIMESTAMP_UPDATED :
			// �^�C���X�^���v���X�V���ꂽ
			if (m_wCurService!=0xFFFF) {
				m_ProgManager.GetPcrTimeStamp(&m_u64CurPcrTimeStamp,m_wCurService);
			}
			SendDtvEngineEvent(EID_PCR_TIMESTAMP_UPDATED, static_cast<PVOID>(&m_ProgManager));
			return 0UL;
		}
	} else if(pDecoder == &m_FileReader) {
		CFileReader *pFileReader = dynamic_cast<CFileReader *>(pDecoder);

		// �t�@�C�����[�_����̃C�x���g
		switch(dwEventID){
		case CFileReader::EID_READ_ASYNC_START:
			// �񓯊����[�h�J�n
			return 0UL;

		case CFileReader::EID_READ_ASYNC_END:
			// �񓯊����[�h�I��
			return 0UL;

		case CFileReader::EID_READ_ASYNC_POSTREAD:
			// �񓯊����[�h��
			if (pFileReader->GetReadPos() >= pFileReader->GetFileSize()) {
				// �ŏ��Ɋ����߂�(���[�v�Đ�)
				pFileReader->SetReadPos(0ULL);
				//pFileReader->Reset();
				ResetEngine();
			}
			return 0UL;
		}
	} else if (pDecoder == &m_FileWriter) {
		switch (dwEventID) {
		case CFileWriter::EID_WRITE_ERROR:
			// �������݃G���[����������
			SendDtvEngineEvent(EID_FILE_WRITE_ERROR, pDecoder);
			return 0UL;
		}
	} else if (pDecoder == &m_MediaViewer) {
		switch (dwEventID) {
		case CMediaViewer::EID_FILTER_GRAPH_FLUSH:
			//m_BonSrcDecoder.PurgeStream();
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
	CVideoRenderer::RendererType VideoRenderer,LPCWSTR pszMpeg2Decoder,LPCWSTR pszAudioDevice)
{
	if (!m_MediaViewer.OpenViewer(hwndHost,hwndMessage,VideoRenderer,
								  pszMpeg2Decoder,pszAudioDevice)) {
		SetError(m_MediaViewer.GetLastErrorException());
		m_bBuildComplete=CheckBuildComplete();
		return false;
	}
	m_bBuildComplete=CheckBuildComplete();
	return true;
}


bool CDtvEngine::RebuildMediaViewer(HWND hwndHost,HWND hwndMessage,
	CVideoRenderer::RendererType VideoRenderer,LPCWSTR pszMpeg2Decoder,LPCWSTR pszAudioDevice)
{
	bool bOK;

	EnablePreview(false);
	m_ProgManager.SetOutputDecoder(NULL);
	m_MediaViewer.CloseViewer();
	bOK=m_MediaViewer.OpenViewer(hwndHost,hwndMessage,VideoRenderer,
								 pszMpeg2Decoder,pszAudioDevice);
	if (!bOK) {
		SetError(m_MediaViewer.GetLastErrorException());
		m_bBuildComplete=false;
	} else {
		m_bBuildComplete=CheckBuildComplete();
	}
	if (m_bBuffering) {
		m_MediaBuffer.SetOutputDecoder(&m_MediaViewer);
		m_ProgManager.SetOutputDecoder(&m_MediaBuffer);
	} else {
		m_ProgManager.SetOutputDecoder(&m_MediaViewer);
	}
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
			m_bBuildComplete = CheckBuildComplete();
			return false;
		}
	} else if (m_TsDescrambler.IsBcasCardOpen()) {
		m_TsDescrambler.CloseBcasCard();
	}
	m_bBuildComplete = CheckBuildComplete();
	/*
	if (m_bBuildComplete)
		ResetEngine();
	*/
	return true;
}


bool CDtvEngine::SetDescramble(bool bDescramble)
{
	if (!m_bBuiled) {
		SetError(0,TEXT("�����G���[ : DtvEngine���\�z����Ă��܂���B"));
		return false;
	}

	if (m_bDescramble != bDescramble) {
		m_TsDescrambler.EnableDescramble(bDescramble);
		m_bDescramble = bDescramble;
		m_bBuildComplete = CheckBuildComplete();
	}
	return true;
}


bool CDtvEngine::ResetBuffer()
{
	m_MediaBuffer.ResetBuffer();
	return true;
}


bool CDtvEngine::GetOriginalVideoSize(WORD *pWidth,WORD *pHeight)
{
	return m_MediaViewer.GetOriginalVideoSize(pWidth,pHeight);
}


bool CDtvEngine::SetDescrambleService(WORD Service)
{
	/*
	if (Service!=0xFFFF) {
		WORD TargetPIDs[2] = {0xFFFF,0xFFFF};

		if (!m_ProgManager.GetServiceEsPID(&TargetPIDs[0], &TargetPIDs[1], Service))
			return false;
		return m_TsDescrambler.SetTargetPID(TargetPIDs,2);
	}
	return m_TsDescrambler.SetTargetPID();
	*/
	WORD ServiceID;

	if (Service!=0xFFFF) {
		if (!m_ProgManager.GetServiceID(&ServiceID,Service))
			return false;
	} else {
		ServiceID=0;
	}
	return m_TsDescrambler.SetTargetServiceID(ServiceID);
}


bool CDtvEngine::SetDescrambleCurServiceOnly(bool bOnly)
{
	if (m_bDescrambleCurServiceOnly!=bOnly) {
		m_bDescrambleCurServiceOnly=bOnly;
		SetDescrambleService(bOnly?m_wCurService:0xFFFF);
	}
	return true;
}


bool CDtvEngine::SetWriteService(WORD Service,DWORD Stream)
{
	WORD ServiceID;

	if (Service!=0xFFFF) {
		if (!m_ProgManager.GetServiceID(&ServiceID,Service))
			return false;
	} else {
		ServiceID=0;
	}
	return m_TsSelector.SetTargetServiceID(ServiceID,Stream);
}


bool CDtvEngine::SetWriteCurServiceOnly(bool bOnly,DWORD Stream)
{
	if (m_bWriteCurServiceOnly!=bOnly) {
		m_bWriteCurServiceOnly=bOnly;
		if (bOnly) {
			m_TsSelector.Reset();
			SetWriteService(m_wCurService,Stream);
			m_TsSelector.SetOutputDecoder(&m_FileWriter);
			m_MediaGrabber.SetOutputDecoder(&m_TsSelector);
		} else {
			m_MediaGrabber.SetOutputDecoder(&m_FileWriter);
			m_TsSelector.SetOutputDecoder(NULL);
		}
	}
	return true;
}


CEpgDataInfo *CDtvEngine::GetEpgDataInfo(WORD ServiceID, bool bNext)
{
	return m_TsPacketParser.GetEpgDataInfo(ServiceID,bNext);
}


void CDtvEngine::SetTracer(CTracer *pTracer)
{
	CBonBaseClass::SetTracer(pTracer);
	m_MediaViewer.SetTracer(pTracer);
}


void CDtvEngine::ResetStatus()
{
	m_wCurTransportStream = 0;
}


//////////////////////////////////////////////////////////////////////
// CDtvEngineHandler �\�z/����
//////////////////////////////////////////////////////////////////////

const DWORD CDtvEngineHandler::OnDtvEngineEvent(CDtvEngine *pEngine, const DWORD dwEventID, PVOID pParam)
{
	// �f�t�H���g�̏���
	return 0UL;
}
