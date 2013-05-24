// MediaViewer.cpp: CMediaViewer �N���X�̃C���v�������e�[�V����
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MediaViewer.h"
#include "StdUtil.h"
#include <Dvdmedia.h>
#ifdef USE_GRABBER_FILTER
#include "../Grabber.h"
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


#define LOCK_TIMEOUT 2000


//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////

CMediaViewer::CMediaViewer(IEventHandler *pEventHandler)
	: CMediaDecoder(pEventHandler, 1UL, 0UL)
	, m_bInit(false)
	, m_pMediaControl(NULL)

	, m_pFilterGraph(NULL)

	, m_pSrcFilter(NULL)
	, m_pBonSrcFilterClass(NULL)

	, m_pMpeg2SeqFilter(NULL)
	, m_pMpeg2SeqClass(NULL)

	, m_pAacDecFilter(NULL)
	, m_pAacDecClass(NULL)

	/*
	, m_pPcmSelFilter(NULL)
	, m_pPcmSelClass(NULL)
	*/

	, m_pMpeg2DecFilter(NULL)

	, m_pVideoRenderer(NULL)

	, m_pszMpeg2DecoderName(NULL)

	, m_pMp2DemuxFilter(NULL)
	, m_pMp2DemuxVideoMap(NULL)
	, m_pMp2DemuxAudioMap(NULL)

	, m_wVideoEsPID(PID_INVALID)
	, m_wAudioEsPID(PID_INVALID)

	, m_wVideoWindowX(0)
	, m_wVideoWindowY(0)
	, m_VideoInfo()
	, m_hOwnerWnd(NULL)
#ifdef DEBUG
	, m_dwRegister(0)
#endif

	, m_VideoRendererType(CVideoRenderer::RENDERER_UNDEFINED)
	, m_ForceAspectX(0)
	, m_ForceAspectY(0)
	, m_PanAndScan(0)
	, m_ViewStretchMode(STRETCH_KEEPASPECTRATIO)
	, m_bUseAudioRendererClock(true)
	, m_pImageMixer(NULL)
#ifdef USE_GRABBER_FILTER
	, m_bGrabber(false)
	, m_pGrabber(NULL)
#endif
	, m_pTracer(NULL)
	/*
	, m_hFlushThread(NULL)
	, m_hFlushEvent(NULL)
	, m_hFlushResumeEvent(NULL)
	, m_LastFlushTime(0)
	*/
{
	// COM���C�u����������
	//::CoInitialize(NULL);
}

CMediaViewer::~CMediaViewer()
{
	CloseViewer();

	// COM���C�u�����J��
	//::CoUninitialize();
}


void CMediaViewer::Reset(void)
{
	TRACE(TEXT("CMediaViewer::Reset()\n"));

	CTryBlockLock Lock(&m_DecoderLock);
	Lock.TryLock(LOCK_TIMEOUT);

	/*
	bool fResume=false;

	if (m_pMediaControl) {
		OAFilterState fs;

		if (m_pMediaControl->GetState(1000,&fs)==S_OK && fs==State_Running) {
			//Stop();
			fResume=true;
		}
	}
	*/

	if (m_pBonSrcFilterClass)
		m_pBonSrcFilterClass->Reset();

	Flush();
	//Stop();

	SetVideoPID(PID_INVALID);
	SetAudioPID(PID_INVALID);

	ResetDownstreamDecoder();

	/*
	if (fResume)
		Play();
	*/
}

const bool CMediaViewer::InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex)
{
	CBlockLock Lock(&m_DecoderLock);

	/*
	if(dwInputIndex >= GetInputNum())return false;

	CTsPacket *pTsPacket = dynamic_cast<CTsPacket *>(pMediaData);

	// ���̓��f�B�A�f�[�^�͌݊������Ȃ�
	if(!pTsPacket)return false;
	*/
	CTsPacket *pTsPacket = static_cast<CTsPacket *>(pMediaData);

	// �t�B���^�O���t�ɓ���
	if (m_pBonSrcFilterClass) {
		/*
		if (m_hFlushThread) {
			DWORD CurTime=::GetTickCount();

			if (CurTime<m_LastFlushTime)
				m_LastFlushTime=0;
			if (CurTime-m_LastFlushTime>=100) {
				m_LastFlushTime=CurTime;
				m_FlushEventType=FLUSH_RESET;
				::SetEvent(m_hFlushEvent);
			}
		}
		*/
		return m_pBonSrcFilterClass->InputMedia(pTsPacket);
	}

	return false;
}

const bool CMediaViewer::OpenViewer(HWND hOwnerHwnd,HWND hMessageDrainHwnd,
			CVideoRenderer::RendererType RendererType,LPCWSTR pszMpeg2Decoder)
{
	CTryBlockLock Lock(&m_DecoderLock);
	if (!Lock.TryLock(LOCK_TIMEOUT)) {
		SetError(TEXT("�^�C���A�E�g�G���[�ł��B"));
		return false;
	}

	if (m_bInit) {
		SetError(TEXT("���Ƀt�B���^�O���t���\�z����Ă��܂��B"));
		return false;
	}

	HRESULT hr=S_OK;

	IPin *pOutput=NULL;
	IPin *pOutputVideo=NULL;
	IPin *pOutputAudio=NULL;

	try {
		// �t�B���^�O���t�}�l�[�W�����\�z����
		if (::CoCreateInstance(CLSID_FilterGraph,NULL,CLSCTX_INPROC_SERVER,
				IID_IGraphBuilder,reinterpret_cast<LPVOID*>(&m_pFilterGraph)) != S_OK) {
			throw CBonException(TEXT("�t�B���^�O���t�}�l�[�W�����쐬�ł��܂���B"));
		}
#ifdef DEBUG
		AddToRot(m_pFilterGraph, &m_dwRegister);
#endif

		// IMediaControl�C���^�t�F�[�X�̃N�G���[
		if (m_pFilterGraph->QueryInterface(IID_IMediaControl, reinterpret_cast<LPVOID*>(&m_pMediaControl)) != S_OK) {
			throw CBonException(TEXT("���f�B�A�R���g���[�����擾�ł��܂���B"));
		}

		Trace(TEXT("�\�[�X�t�B���^�̐ڑ���..."));

		/* CBonSrcFilter */
		{
			// �C���X�^���X�쐬
			m_pSrcFilter = CBonSrcFilter::CreateInstance(NULL, &hr, &m_pBonSrcFilterClass);
			if (m_pSrcFilter==NULL || hr!=S_OK)
				throw CBonException(TEXT("�\�[�X�t�B���^���쐬�ł��܂���B"));
			m_pBonSrcFilterClass->SetOutputWhenPaused(RendererType==CVideoRenderer::RENDERER_DEFAULT);
			// �t�B���^�O���t�ɒǉ�
			if (m_pFilterGraph->AddFilter(m_pSrcFilter, L"BonSrcFilter") != S_OK)
				throw CBonException(TEXT("�\�[�X�t�B���^���t�B���^�O���t�ɒǉ��ł��܂���B"));
			// �o�̓s�����擾
			pOutput = DirectShowUtil::GetFilterPin(m_pSrcFilter,PINDIR_OUTPUT);
			if (pOutput==NULL)
				throw CBonException(TEXT("�\�[�X�t�B���^�̏o�̓s�����擾�ł��܂���B"));
		}

		Trace(TEXT("MPEG-2 Demultiplexer�t�B���^�̐ڑ���..."));

		/* MPEG-2 Demultiplexer */
		{
			CMediaType MediaTypeVideo;
			CMediaType MediaTypeAudio;
			IReferenceClock *pMp2DemuxRefClock;
			IMpeg2Demultiplexer *pMpeg2Demuxer;

			if (::CoCreateInstance(CLSID_MPEG2Demultiplexer,NULL,
					CLSCTX_INPROC_SERVER,IID_IBaseFilter,
					reinterpret_cast<LPVOID*>(&m_pMp2DemuxFilter))!=S_OK)
				throw CBonException(TEXT("MPEG-2 Demultiplexer�t�B���^���쐬�ł��܂���B"),
									TEXT("MPEG-2 Demultiplexer�t�B���^���C���X�g�[������Ă��邩�m�F���Ă��������B"));
			if (!DirectShowUtil::AppendFilterAndConnect(m_pFilterGraph,
					m_pMp2DemuxFilter,L"Mpeg2Demuxer",&pOutput))
				throw CBonException(TEXT("MPEG-2 Demultiplexer���t�B���^�O���t�ɒǉ��ł��܂���B"));
			// ���̎��_��pOutput==NULL�̂͂������O�̂���
			SAFE_RELEASE(pOutput);
			// IReferenceClock�C���^�t�F�[�X�̃N�G���[
			if (m_pMp2DemuxFilter->QueryInterface(IID_IReferenceClock,
						reinterpret_cast<LPVOID*>(&pMp2DemuxRefClock)) != S_OK)
				throw CBonException(TEXT("IReferenceClock���擾�ł��܂���B"));
			// ���t�@�����X�N���b�N�I��
			hr=m_pMp2DemuxFilter->SetSyncSource(pMp2DemuxRefClock);
			pMp2DemuxRefClock->Release();
			if (hr != S_OK)
				throw CBonException(TEXT("���t�@�����X�N���b�N��ݒ�ł��܂���B"));

			// IMpeg2Demultiplexer�C���^�t�F�[�X�̃N�G���[
			if (FAILED(m_pMp2DemuxFilter->QueryInterface(IID_IMpeg2Demultiplexer,
									reinterpret_cast<LPVOID*>(&pMpeg2Demuxer))))
				throw CBonException(TEXT("MPEG-2 Demultiplexer�C���^�[�t�F�[�X���擾�ł��܂���B"),
									TEXT("�݊����̂Ȃ��X�v���b�^�̗D��x��MPEG-2 Demultiplexer��荂���Ȃ��Ă���\��������܂��B"));

			// �f�����f�B�A�t�H�[�}�b�g�ݒ�
			MediaTypeVideo.InitMediaType();
			MediaTypeVideo.SetType(&MEDIATYPE_Video);
			MediaTypeVideo.SetSubtype(&MEDIASUBTYPE_MPEG2_VIDEO);
			MediaTypeVideo.SetVariableSize();
			MediaTypeVideo.SetTemporalCompression(TRUE);
			MediaTypeVideo.SetSampleSize(0);
			MediaTypeVideo.SetFormatType(&FORMAT_MPEG2Video);
			 // �t�H�[�}�b�g�\���̊m��
			MPEG2VIDEOINFO *pVideoInfo = (MPEG2VIDEOINFO *)MediaTypeVideo.AllocFormatBuffer(sizeof(MPEG2VIDEOINFO));
			if(!pVideoInfo) throw 1UL;
			::ZeroMemory(pVideoInfo, sizeof(MPEG2VIDEOINFO));
			// �r�f�I�w�b�_�ݒ�
			VIDEOINFOHEADER2 &VideoHeader = pVideoInfo->hdr;
			::SetRect(&VideoHeader.rcSource, 0, 0, 720, 480);
			VideoHeader.bmiHeader.biWidth = 720;
			VideoHeader.bmiHeader.biHeight = 480;
			// �f���o�̓s���쐬
			if (pMpeg2Demuxer->CreateOutputPin(&MediaTypeVideo,L"Video",&pOutputVideo) != S_OK) {
				pMpeg2Demuxer->Release();
				throw CBonException(TEXT("MPEG-2 Demultiplexer�̉f���o�̓s�����쐬�ł��܂���B"));
			}
			// �������f�B�A�t�H�[�}�b�g�ݒ�	
			MediaTypeAudio.InitMediaType();
			MediaTypeAudio.SetType(&MEDIATYPE_Audio);
			MediaTypeAudio.SetSubtype(&MEDIASUBTYPE_NULL);
			MediaTypeAudio.SetVariableSize();
			MediaTypeAudio.SetTemporalCompression(TRUE);
			MediaTypeAudio.SetSampleSize(0);
			MediaTypeAudio.SetFormatType(&FORMAT_None);
			// �����o�̓s���쐬
			hr=pMpeg2Demuxer->CreateOutputPin(&MediaTypeAudio,L"Audio",&pOutputAudio);
			pMpeg2Demuxer->Release();
			if (hr != S_OK)
				throw CBonException(TEXT("MPEG-2 Demultiplexer�̉����o�̓s�����쐬�ł��܂���B"));
			// �f���o�̓s����IMPEG2PIDMap�C���^�t�F�[�X�̃N�G���[
			if (pOutputVideo->QueryInterface(IID_IMPEG2PIDMap,(void**)&m_pMp2DemuxVideoMap) != S_OK)
				throw CBonException(TEXT("�f���o�̓s����IMPEG2PIDMap���擾�ł��܂���B"));
			// �����o�̓s����IMPEG2PIDMap�C���^�t�F�[�X�̃N�G��
			if (pOutputAudio->QueryInterface(IID_IMPEG2PIDMap,(void**)&m_pMp2DemuxAudioMap) != S_OK)
				throw CBonException(TEXT("�����o�̓s����IMPEG2PIDMap���擾�ł��܂���B"));
		}

		Trace(TEXT("MPEG-2�V�[�P���X�t�B���^�̐ڑ���..."));

		/* CMpeg2SequenceFilter */
		{
			// �C���X�^���X�쐬
			m_pMpeg2SeqFilter = CMpeg2SequenceFilter::CreateInstance(NULL, &hr,&m_pMpeg2SeqClass);
			if((!m_pMpeg2SeqFilter) || (hr != S_OK))
				throw CBonException(TEXT("MPEG-2�V�[�P���X�t�B���^���쐬�ł��܂���B"));
			m_pMpeg2SeqClass->SetRecvCallback(OnMpeg2VideoInfo,this);
			// �t�B���^�̒ǉ��Ɛڑ�
			if (!DirectShowUtil::AppendFilterAndConnect(m_pFilterGraph,
					m_pMpeg2SeqFilter,L"Mpeg2SequenceFilter",&pOutputVideo))
				throw CBonException(TEXT("MPEG-2�V�[�P���X�t�B���^���t�B���^�O���t�ɒǉ��ł��܂���B"));
		}

		Trace(TEXT("AAC�f�R�[�_�̐ڑ���..."));

		/* CAacDecFilter */
		{
			// CAacDecFilter�C���X�^���X�쐬
			m_pAacDecFilter=CAacDecFilter::CreateInstance(NULL,&hr,&m_pAacDecClass);
			if (!m_pAacDecFilter || hr!=S_OK)
				throw CBonException(TEXT("AAC�f�R�[�_�t�B���^���쐬�ł��܂���B"));
			// �t�B���^�̒ǉ��Ɛڑ�
			if (!DirectShowUtil::AppendFilterAndConnect(m_pFilterGraph,
					m_pAacDecFilter,L"AacDecFilter",&pOutputAudio))
				throw CBonException(TEXT("AAC�f�R�[�_�t�B���^���t�B���^�O���t�ɒǉ��ł��܂���B"));
		}

#if 0
		Trace(TEXT("PCM�Z���N�g�t�B���^�̐ڑ���..."));

		/* CPcmSelectFilter */
		{
			// �C���X�^���X�쐬
			m_pPcmSelFilter=CPcmSelectFilter::CreateInstance(NULL,&hr,&m_pPcmSelClass);
			if (!m_pPcmSelFilter || hr!=S_OK)
				throw CBonException(TEXT("PCM�Z���N�g�t�B���^���쐬�ł��܂���B"));
			// �t�B���^�̒ǉ��Ɛڑ�
			if (!DirectShowUtil::AppendFilterAndConnect(m_pFilterGraph,
					m_pPcmSelFilter,L"PcmSelFilter",&pOutputAudio))
				throw CBonException(TEXT("PCM�Z���N�g�t�B���^���t�B���^�O���t�ɒǉ��ł��܂���B"));
		}
#endif

		Trace(TEXT("MPEG-2�f�R�[�_�̐ڑ���..."));

		/* Mpeg2�f�R�[�_�[ */
		{
			CDirectShowFilterFinder FilterFinder;

			// ����
			if(!FilterFinder.FindFilter(&MEDIATYPE_Video,&MEDIASUBTYPE_MPEG2_VIDEO))
				throw CBonException(TEXT("MPEG-2�f�R�[�_�����t����܂���B"),
									TEXT("MPEG-2�f�R�[�_���C���X�g�[������Ă��邩�m�F���Ă��������B"));

			WCHAR szMpeg2Decoder[128];
			CLSID idMpeg2Vid;
			bool bConnectSuccess=false;

			for (int i=0;!bConnectSuccess && i<FilterFinder.GetFilterCount();i++){
				if (FilterFinder.GetFilterInfo(i,&idMpeg2Vid,szMpeg2Decoder,128)) {
					if (pszMpeg2Decoder!=NULL && pszMpeg2Decoder[0]!='\0'
							&& ::lstrcmpi(szMpeg2Decoder,pszMpeg2Decoder)!=0)
						continue;
					if (DirectShowUtil::AppendFilterAndConnect(m_pFilterGraph,
							idMpeg2Vid,szMpeg2Decoder,&m_pMpeg2DecFilter,
							&pOutputVideo,NULL,true)) {
						bConnectSuccess=true;
						break;
					}
				} else {
					// �t�B���^���擾���s
				}
			}
			// �ǂꂩ�̃t�B���^�Őڑ��ł�����
			if (bConnectSuccess) {
				m_pszMpeg2DecoderName=StdUtil::strdup(szMpeg2Decoder);
			} else {
				throw CBonException(TEXT("MPEG-2�f�R�[�_�t�B���^���t�B���^�O���t�ɒǉ��ł��܂���B"),
									TEXT("�ݒ�ŗL����MPEG-2�f�R�[�_���I������Ă��邩�m�F���Ă��������B"));
			}
		}

#ifdef USE_GRABBER_FILTER
		// �O���o���e�X�g�������������܂������܂������Ȃ��̂ŕۗ�
		if (m_bGrabber) {
			m_pGrabber=new CGrabber();
			IBaseFilter *pGrabberFilter;

			Trace(TEXT("�O���o�t�B���^��ڑ���..."));

			m_pGrabber->Init();
			pGrabberFilter=m_pGrabber->GetGrabberFilter();
			if (!DirectShowUtil::AppendFilterAndConnect(m_pFilterGraph,
									pGrabberFilter,L"Grabber",&pOutputVideo)) {
				delete m_pGrabber;
				m_pGrabber=NULL;
			}
		}
#endif

		Trace(TEXT("�f�������_���̍\�z��..."));

		if (!CVideoRenderer::CreateRenderer(RendererType,&m_pVideoRenderer)) {
			throw CBonException(TEXT("�f�������_�����쐬�ł��܂���B"),
								TEXT("�ݒ�ŗL���ȃ����_�����I������Ă��邩�m�F���Ă��������B"));
		}
		if (!m_pVideoRenderer->Initialize(m_pFilterGraph,pOutputVideo,
										  hOwnerHwnd,hMessageDrainHwnd)) {
			throw CBonException(m_pVideoRenderer->GetLastErrorException());
		}
		m_VideoRendererType=RendererType;

		Trace(TEXT("���������_���̍\�z��..."));

		// ���������_���\�z
		if (m_bUseAudioRendererClock) {
			bool fOK=false;
#if 1
			IBaseFilter *pAudioRenderer;

			if (SUCCEEDED(::CoCreateInstance(CLSID_DSoundRender,NULL,
					CLSCTX_INPROC_SERVER,IID_IBaseFilter,
					reinterpret_cast<LPVOID*>(&pAudioRenderer)))) {
				//if (SUCCEEDED(m_pFilterGraph->AddFilter(pAudioRenderer,L"Audio Renderer"))) {
				if (DirectShowUtil::AppendFilterAndConnect(m_pFilterGraph,
							pAudioRenderer,L"Audio Renderer",&pOutputAudio)) {
					IMediaFilter *pMediaFilter;

					if (SUCCEEDED(m_pFilterGraph->QueryInterface(IID_IMediaFilter,
								reinterpret_cast<LPVOID*>(&pMediaFilter)))) {
						IReferenceClock *pReferenceClock;

						if (SUCCEEDED(pAudioRenderer->QueryInterface(IID_IReferenceClock,
								reinterpret_cast<LPVOID*>(&pReferenceClock)))) {
							pMediaFilter->SetSyncSource(pReferenceClock);
							pReferenceClock->Release();
						}
						pMediaFilter->Release();
					}
					fOK=true;
				}
				pAudioRenderer->Release();
			}
#endif
			if (!fOK) {
				if (FAILED(m_pFilterGraph->Render(pOutputAudio)))
					throw CBonException(TEXT("���������_�����\�z�ł��܂���B"),
						TEXT("�L���ȃT�E���h�f�o�C�X�����݂��邩�m�F���Ă��������B"));
				m_pFilterGraph->SetDefaultSyncSource();
			}
		} else {
			if (FAILED(m_pFilterGraph->Render(pOutputAudio)))
				throw CBonException(TEXT("���������_�����\�z�ł��܂���B"),
					TEXT("�L���ȃT�E���h�f�o�C�X�����݂��邩�m�F���Ă��������B"));
		}

		// �I�[�i�E�B���h�E�ݒ�
		m_hOwnerWnd = hOwnerHwnd;

		//ResizeVideoWindow();

		/*
		OSVERSIONINFO osvi;
		osvi.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
		::GetVersionEx(&osvi);
		if (osvi.dwMajorVersion>=6) {
			DWORD ThreadID;

			m_hFlushEvent=::CreateEvent(NULL,FALSE,FALSE,NULL);
			m_hFlushResumeEvent=::CreateEvent(NULL,FALSE,FALSE,NULL);
			m_hFlushThread=::CreateThread(NULL,0,FlushThread,this,0,NULL);
		}
		m_LastFlushTime=0;
		*/

		m_bInit=true;
	} catch (CBonException &Exception) {
		SetError(Exception);
		CloseViewer();
		TRACE(TEXT("�t�B���^�O���t�\�z���s : %s\n"), GetLastErrorText());
		return false;
	}

	SAFE_RELEASE(pOutputVideo);
	SAFE_RELEASE(pOutputAudio);

	TRACE(TEXT("�t�B���^�O���t�\�z����\n"));
	return true;
}

void CMediaViewer::CloseViewer(void)
{
	CTryBlockLock Lock(&m_DecoderLock);
	Lock.TryLock(LOCK_TIMEOUT);

	/*
	if (!m_bInit)
		return;
	*/

	/*
	if (m_hFlushThread) {
		m_FlushEventType=FLUSH_ABORT;
		::SetEvent(m_hFlushEvent);
		::SetEvent(m_hFlushResumeEvent);
		if (::WaitForSingleObject(m_hFlushThread,1000)==WAIT_TIMEOUT) {
			TRACE(TEXT("CMediaViewer::CloseViewer() Terminate flush thread\n"));
			::TerminateThread(m_hFlushThread,1);
		}
		::CloseHandle(m_hFlushThread);
		m_hFlushThread=NULL;
		::CloseHandle(m_hFlushEvent);
		m_hFlushEvent=NULL;
		::CloseHandle(m_hFlushResumeEvent);
		m_hFlushResumeEvent=NULL;
	}
	*/

	//Flush();
	Stop();

	// COM�C���X�^���X���J������
	if (m_pVideoRenderer!=NULL) {
		m_pVideoRenderer->Finalize();
	}

	if (m_pImageMixer!=NULL) {
		delete m_pImageMixer;
		m_pImageMixer=NULL;
	}

#ifdef USE_GRABBER_FILTER
	if (m_pGrabber!=NULL) {
		m_pFilterGraph->RemoveFilter(m_pGrabber->GetGrabberFilter());
		delete m_pGrabber;
		m_pGrabber=NULL;
	}
#endif

	if (m_pszMpeg2DecoderName!=NULL) {
		delete [] m_pszMpeg2DecoderName;
		m_pszMpeg2DecoderName=NULL;
	}

	SAFE_RELEASE(m_pMpeg2DecFilter);

	/*
	SAFE_RELEASE(m_pPcmSelFilter);
	m_pPcmSelClass=NULL;
	*/
	SAFE_RELEASE(m_pAacDecFilter);
	m_pAacDecClass=NULL;

	SAFE_RELEASE(m_pMpeg2SeqFilter);
	m_pMpeg2SeqClass=NULL;

	SAFE_RELEASE(m_pMp2DemuxAudioMap);
	SAFE_RELEASE(m_pMp2DemuxVideoMap);
	SAFE_RELEASE(m_pMp2DemuxFilter);

	SAFE_RELEASE(m_pSrcFilter);
	m_pBonSrcFilterClass=NULL;

	SAFE_RELEASE(m_pMediaControl);

#ifdef DEBUG
	if(m_dwRegister!=0){
		RemoveFromRot(m_dwRegister);
		m_dwRegister = 0;
	}
#endif

#ifdef DEBUG
	if (m_pFilterGraph)
		TRACE(TEXT("FilterGraph RefCount = %d\n"),DirectShowUtil::GetRefCount(m_pFilterGraph));
#endif
	SAFE_RELEASE(m_pFilterGraph);

	if (m_pVideoRenderer!=NULL) {
		delete m_pVideoRenderer;
		m_pVideoRenderer=NULL;
	}

	m_bInit=false;
}

const bool CMediaViewer::IsOpen() const
{
	return m_bInit;
}

const bool CMediaViewer::Play(void)
{
	TRACE(TEXT("CMediaViewer::Play()\n"));

	CTryBlockLock Lock(&m_DecoderLock);
	if (!Lock.TryLock(LOCK_TIMEOUT))
		return false;

	if(!m_pMediaControl)return false;

	/*
	if (m_hFlushThread) {
		m_FlushEventType=FLUSH_RESET;
		::SetEvent(m_hFlushEvent);
		::SetEvent(m_hFlushResumeEvent);
	}
	*/

	// �t�B���^�O���t���Đ�����

	//return m_pMediaControl->Run()==S_OK;

	if (m_pMediaControl->Run()!=S_OK) {
		int i;
		OAFilterState fs;

		for (i=0;i<20;i++) {
			if (m_pMediaControl->GetState(100,&fs)==S_OK && fs==State_Running)
				return true;
		}
		return false;
	}
	return true;
}

const bool CMediaViewer::Stop(void)
{
	TRACE(TEXT("CMediaViewer::Stop()\n"));

	CTryBlockLock Lock(&m_DecoderLock);
	if (!Lock.TryLock(LOCK_TIMEOUT))
		return false;

	if (!m_pMediaControl)
		return false;

	/*
	// ���Ƀt�B���^�O���t���f�b�h���b�N���Ă���ꍇ�A�~�߂�Ɩ߂��Ă��Ȃ��̂�
	// �{���̓f�b�h���b�N���Ȃ��悤�ɂ��ׂ�����...
	if (CheckHangUp(1000))
		return false;
	*/

	if (m_pBonSrcFilterClass)
		m_pBonSrcFilterClass->Reset();

	/*
	if (m_hFlushThread) {
		m_FlushEventType=FLUSH_WAIT;
		::ResetEvent(m_hFlushResumeEvent);
		::SetEvent(m_hFlushEvent);
	}
	*/

	// �t�B���^�O���t���~����
	return m_pMediaControl->Stop()==S_OK;
}

const bool CMediaViewer::Pause()
{
	TRACE(TEXT("CMediaViewer::Pause()\n"));

	CTryBlockLock Lock(&m_DecoderLock);
	if (!Lock.TryLock(LOCK_TIMEOUT))
		return false;

	if (!m_pMediaControl)
		return false;

	if (m_pBonSrcFilterClass)
		m_pBonSrcFilterClass->Reset();

	/*
	if (m_hFlushThread) {
		m_FlushEventType=FLUSH_WAIT;
		::ResetEvent(m_hFlushResumeEvent);
		::SetEvent(m_hFlushEvent);
	}
	*/

	if (m_pMediaControl->Pause()!=S_OK) {
		int i;
		OAFilterState fs;
		HRESULT hr;

		for (i=0;i<20;i++) {
			hr=m_pMediaControl->GetState(100,&fs);
			if ((hr==S_OK || hr==VFW_S_CANT_CUE) && fs==State_Paused)
				return true;
		}
		return false;
	}
	return true;
}

const bool CMediaViewer::Flush()
{
	TRACE(TEXT("CMediaViewer::Flush()\n"));

	/*
	CTryBlockLock Lock(&m_DecoderLock);
	if (!Lock.TryLock(LOCK_TIMEOUT))
		return false;
	*/

	if (!m_pBonSrcFilterClass)
		return false;

	m_pBonSrcFilterClass->Flush();
	/*
	if (m_hFlushThread) {
		m_FlushEventType=FLUSH_RESET;
		::SetEvent(m_hFlushEvent);
	}
	*/
	return true;
}

const bool CMediaViewer::SetVideoPID(const WORD wPID)
{
	// �f���o�̓s����PID���}�b�s���O����
	if(!m_pMp2DemuxVideoMap)return false;

	if(wPID == m_wVideoEsPID)return true;

	TRACE(TEXT("CMediaViewer::SetVideoPID() %04X <- %04X\n"),wPID,m_wVideoEsPID);

	DWORD dwTempPID;

	// ���݂�PID���A���}�b�v
	if(m_wVideoEsPID != PID_INVALID){
		dwTempPID = (DWORD)m_wVideoEsPID;
		if(m_pMp2DemuxVideoMap->UnmapPID(1UL, &dwTempPID) != S_OK)return false;
		}

	// �V�K��PID���}�b�v
	if(wPID != PID_INVALID){
		dwTempPID = wPID;
		if(m_pMp2DemuxVideoMap->MapPID(1UL, &dwTempPID, MEDIA_ELEMENTARY_STREAM) != S_OK)return false;
		}

	m_wVideoEsPID = wPID;

	return true;
}

const bool CMediaViewer::SetAudioPID(const WORD wPID)
{
	// �����o�̓s����PID���}�b�s���O����
	if(!m_pMp2DemuxAudioMap)return false;

	if(wPID == m_wAudioEsPID)return true;

	TRACE(TEXT("CMediaViewer::SetAudioPID() %04X <- %04X\n"),wPID,m_wAudioEsPID);

	DWORD dwTempPID;

	// ���݂�PID���A���}�b�v
	if(m_wAudioEsPID != PID_INVALID){
		dwTempPID = (DWORD)m_wAudioEsPID;
		if(m_pMp2DemuxAudioMap->UnmapPID(1UL, &dwTempPID) != S_OK)return false;
		}

	// �V�K��PID���}�b�v
	if(wPID != PID_INVALID){
		dwTempPID = wPID;
		if(m_pMp2DemuxAudioMap->MapPID(1UL, &dwTempPID, MEDIA_ELEMENTARY_STREAM) != S_OK)return false;
		}

	m_wAudioEsPID = wPID;

	return true;
}

void CMediaViewer::OnMpeg2VideoInfo(const CMpeg2VideoInfo *pVideoInfo,const LPVOID pParam)
{
	// �r�f�I���̍X�V
	CMediaViewer *pThis=static_cast<CMediaViewer*>(pParam);

	CBlockLock Lock(&pThis->m_ResizeLock);

	//if (pThis->m_VideoInfo != *pVideoInfo) {
		// �r�f�I���̍X�V
		pThis->m_VideoInfo = *pVideoInfo;
		pThis->ResizeVideoWindow();
	//}
}

const bool CMediaViewer::ResizeVideoWindow()
{
	// �E�B���h�E�T�C�Y��ύX����
	if (m_pVideoRenderer && m_wVideoWindowX>0 && m_wVideoWindowY>0) {
		long WindowWidth,WindowHeight,VideoWidth,VideoHeight;

		WindowWidth = m_wVideoWindowX;
		WindowHeight = m_wVideoWindowY;
		if (m_ViewStretchMode!=STRETCH_FIT) {
			int AspectX,AspectY;
			double aspect_rate;
			double window_rate = (double)WindowWidth / (double)WindowHeight;

			if (m_ForceAspectX>0 && m_ForceAspectY>0) {
				AspectX = m_ForceAspectX;
				AspectY = m_ForceAspectY;
			} else if (m_VideoInfo.m_AspectRatioX>0 && m_VideoInfo.m_AspectRatioY>0) {
				AspectX = m_VideoInfo.m_AspectRatioX;
				AspectY = m_VideoInfo.m_AspectRatioY;
			} else {
				if (((m_VideoInfo.m_DisplayWidth==1920
							|| m_VideoInfo.m_DisplayWidth==1440)
						&& m_VideoInfo.m_DisplayHeight==1080)) {
					AspectX=16;
					AspectY=9;
				} else {
					AspectX=WindowWidth;
					AspectY=WindowHeight;
				}
			}
			aspect_rate = (double)AspectX / (double)AspectY;
			if ((m_ViewStretchMode==STRETCH_KEEPASPECTRATIO && aspect_rate>window_rate)
					|| (m_ViewStretchMode==STRETCH_CUTFRAME && aspect_rate<window_rate)) {
				VideoWidth = WindowWidth;
				VideoHeight = VideoWidth * AspectY  / AspectX;
			} else {
				VideoHeight = WindowHeight;
				VideoWidth = VideoHeight * AspectX / AspectY;
			}
		} else {
			VideoWidth = WindowWidth;
			VideoHeight = WindowHeight;
		}
		RECT rcSrc,rcDst,rcWindow;

		CalcSourceRect(&rcSrc);
		// ���W�l���}�C�i�X�ɂȂ�ƃ}���`�f�B�X�v���C�ł��������Ȃ�?
		/*
		rcDst.left=(WindowWidth-VideoWidth)/2;
		rcDst.top=(WindowHeight-VideoHeight)/2,
		rcDst.right=rcDst.left+VideoWidth;
		rcDst.bottom=rcDst.top+VideoHeight;
		*/
		if (WindowWidth<VideoWidth) {
			rcDst.left=0;
			rcDst.right=WindowWidth;
			rcSrc.left+=(VideoWidth-WindowWidth)*(rcSrc.right-rcSrc.left)/VideoWidth/2;
			rcSrc.right=m_VideoInfo.m_OrigWidth-rcSrc.left;
		} else {
			rcDst.left=(WindowWidth-VideoWidth)/2;
			rcDst.right=rcDst.left+VideoWidth;
		}
		if (WindowHeight<VideoHeight) {
			rcDst.top=0;
			rcDst.bottom=WindowHeight;
			rcSrc.top+=(VideoHeight-WindowHeight)*(rcSrc.bottom-rcSrc.top)/VideoHeight/2;
			rcSrc.bottom=m_VideoInfo.m_OrigHeight-rcSrc.top;
		} else {
			rcDst.top=(WindowHeight-VideoHeight)/2,
			rcDst.bottom=rcDst.top+VideoHeight;
		}
		rcWindow.left=0;
		rcWindow.top=0;
		rcWindow.right=WindowWidth;
		rcWindow.bottom=WindowHeight;
		return m_pVideoRenderer->SetVideoPosition(
			m_VideoInfo.m_OrigWidth,m_VideoInfo.m_OrigHeight,&rcSrc,&rcDst,&rcWindow);
	}
	return false;
}

const bool CMediaViewer::SetViewSize(const int x,const int y)
{
	CBlockLock Lock(&m_ResizeLock);

	// �E�B���h�E�T�C�Y��ݒ肷��
	if (x>0 && y>0) {
		m_wVideoWindowX = x;
		m_wVideoWindowY = y;
		return ResizeVideoWindow();
	}
	return false;
}


const bool CMediaViewer::SetVolume(const float fVolume)
{
	// �I�[�f�B�I�{�����[����dB�Őݒ肷��( -100.0(����) < fVolume < 0(�ő�) )
	IBasicAudio *pBasicAudio;
	bool fOK=false;

	if (m_pFilterGraph) {
		if (SUCCEEDED(m_pFilterGraph->QueryInterface(IID_IBasicAudio,
								reinterpret_cast<LPVOID *>(&pBasicAudio)))) {
			long lVolume = (long)(fVolume * 100.0f);

			if (lVolume>=-10000 && lVolume<=0) {
					TRACE(TEXT("Volume Control = %d\n"),lVolume);
				if (SUCCEEDED(pBasicAudio->put_Volume(lVolume)))
					fOK=true;
			}
			pBasicAudio->Release();
		}
	}
	return fOK;
}

const bool CMediaViewer::GetVideoSize(WORD *pwWidth,WORD *pwHeight)
{
	CBlockLock Lock(&m_ResizeLock);

	// �r�f�I�̃T�C�Y���擾����
	/*
	if (m_pMpeg2SeqClass)
		return m_pMpeg2SeqClass->GetVideoSize(pwWidth,pwHeight);
	*/
	if (m_VideoInfo.m_DisplayWidth > 0 && m_VideoInfo.m_DisplayHeight > 0) {
		if (pwWidth)
			*pwWidth = m_VideoInfo.m_DisplayWidth;
		if (pwHeight)
			*pwHeight = m_VideoInfo.m_DisplayHeight;
		return true;
	}
	return false;
}

const bool CMediaViewer::GetVideoAspectRatio(BYTE *pbyAspectRatioX,BYTE *pbyAspectRatioY)
{
	CBlockLock Lock(&m_ResizeLock);

	// �r�f�I�̃A�X�y�N�g����擾����
	/*
	if (m_pMpeg2SeqClass)
		return m_pMpeg2SeqClass->GetAspectRatio(pbyAspectRatioX,pbyAspectRatioY);
	*/
	if (m_VideoInfo.m_AspectRatioX > 0 && m_VideoInfo.m_AspectRatioY > 0) {
		if (pbyAspectRatioX)
			*pbyAspectRatioX = m_VideoInfo.m_AspectRatioX;
		if (pbyAspectRatioY)
			*pbyAspectRatioY = m_VideoInfo.m_AspectRatioY;
		return true;
	}
	return false;
}

const BYTE CMediaViewer::GetAudioChannelNum()
{
	// �I�[�f�B�I�̓��̓`�����l�������擾����
	if (m_pAacDecClass)
		return m_pAacDecClass->GetCurrentChannelNum();
	return 0;
}

const bool CMediaViewer::SetStereoMode(const int iMode)
{
	// �X�e���I�o�̓`�����l���̐ݒ�
	/*
	if(m_pPcmSelClass){
		m_pPcmSelClass->SetStereoMode(iMode);
		return true;
		}
	*/
	if (m_pAacDecClass)
		return m_pAacDecClass->SetStereoMode(iMode);
	return false;
}

const int CMediaViewer::GetStereoMode() const
{
	if (m_pAacDecClass)
		return m_pAacDecClass->GetStereoMode();
	return CAacDecFilter::STEREOMODE_STEREO;
}

const bool CMediaViewer::GetVideoDecoderName(LPWSTR lpName,int iBufLen)
{
	// �I������Ă���r�f�I�f�R�[�_�[���̎擾
	if (m_pszMpeg2DecoderName!=NULL) {
		::lstrcpynW(lpName,m_pszMpeg2DecoderName,iBufLen);
		return true;
	}
	if (iBufLen>0)
		lpName[0]='\0';
	return false;
}

const bool CMediaViewer::DisplayVideoDecoderProperty(HWND hWndParent)
{
	if (m_pMpeg2DecFilter)
		return DirectShowUtil::ShowPropertyPage(m_pMpeg2DecFilter,hWndParent);
	return false;
}

const bool CMediaViewer::DisplayVideoRandererProperty(HWND hWndParent)
{
	if (m_pVideoRenderer)
		return m_pVideoRenderer->ShowProperty(hWndParent);
	return false;
}

#ifdef DEBUG
HRESULT CMediaViewer::AddToRot(IUnknown *pUnkGraph, DWORD *pdwRegister) const
{
	// �f�o�b�O�p
	IMoniker * pMoniker;
	IRunningObjectTable *pROT;
	if(FAILED(::GetRunningObjectTable(0, &pROT)))return E_FAIL;

	WCHAR wsz[256];
	wsprintfW(wsz, L"FilterGraph %08p pid %08x", (DWORD_PTR)pUnkGraph, ::GetCurrentProcessId());

	HRESULT hr = ::CreateItemMoniker(L"!", wsz, &pMoniker);

	if(SUCCEEDED(hr)){
		hr = pROT->Register(0, pUnkGraph, pMoniker, pdwRegister);
		pMoniker->Release();
		}

	pROT->Release();

	return hr;
}

void CMediaViewer::RemoveFromRot(const DWORD dwRegister) const
{
	// �f�o�b�O�p
	IRunningObjectTable *pROT;

	if(SUCCEEDED(::GetRunningObjectTable(0, &pROT))){
		pROT->Revoke(dwRegister);
		pROT->Release();
		}
}
#endif




const bool CMediaViewer::ForceAspectRatio(int AspectX,int AspectY)
{
	m_ForceAspectX=AspectX;
	m_ForceAspectY=AspectY;
	return true;
}


const bool CMediaViewer::GetForceAspectRatio(int *pAspectX,int *pAspectY) const
{
	if (pAspectX)
		*pAspectX=m_ForceAspectX;
	if (pAspectY)
		*pAspectY=m_ForceAspectY;
	return true;
}


const bool CMediaViewer::GetEffectiveAspectRatio(BYTE *pAspectX,BYTE *pAspectY)
{
	if (m_ForceAspectX!=0 && m_ForceAspectY!=0) {
		if (pAspectX)
			*pAspectX=m_ForceAspectX;
		if (pAspectY)
			*pAspectY=m_ForceAspectY;
		return true;
	}
	return GetVideoAspectRatio(pAspectX,pAspectY);
}


const bool CMediaViewer::SetPanAndScan(int AspectX,int AspectY,BYTE PanScanFlags)
{
	if (m_ForceAspectX!=AspectX || m_ForceAspectY!=AspectY
			|| m_PanAndScan!=PanScanFlags) {
		CBlockLock Lock(&m_ResizeLock);

		m_ForceAspectX=AspectX;
		m_ForceAspectY=AspectY;
		m_PanAndScan=PanScanFlags;
		ResizeVideoWindow();
	}
	return true;
}


const bool CMediaViewer::SetViewStretchMode(ViewStretchMode Mode)
{
	if (m_ViewStretchMode!=Mode) {
		CBlockLock Lock(&m_ResizeLock);

		m_ViewStretchMode=Mode;
		return ResizeVideoWindow();
	}
	return true;
}


const bool CMediaViewer::GetOriginalVideoSize(WORD *pWidth,WORD *pHeight)
{
	CBlockLock Lock(&m_ResizeLock);

	/*
	if (m_pMpeg2SeqClass)
		return m_pMpeg2SeqClass->GetOriginalVideoSize(pWidth,pHeight);
	*/
	if (m_VideoInfo.m_OrigWidth > 0 && m_VideoInfo.m_OrigHeight > 0) {
		if (pWidth)
			*pWidth = m_VideoInfo.m_OrigWidth;
		if (pHeight)
			*pHeight = m_VideoInfo.m_OrigHeight;
		return true;
	}
	return false;
}


const bool CMediaViewer::GetCroppedVideoSize(WORD *pWidth,WORD *pHeight)
{
	RECT rc;

	if (!GetSourceRect(&rc))
		return false;
	if (pWidth)
		*pWidth = (WORD)(rc.right - rc.left);
	if (pHeight)
		*pHeight = (WORD)(rc.bottom - rc.top);
	return true;
}


const bool CMediaViewer::GetSourceRect(RECT *pRect)
{
	CBlockLock Lock(&m_ResizeLock);

	if (!pRect)
		return false;
	return CalcSourceRect(pRect);
}


const bool CMediaViewer::CalcSourceRect(RECT *pRect)
{
	long SrcX,SrcY,SrcWidth,SrcHeight;

	if (m_VideoInfo.m_OrigWidth==0 || m_VideoInfo.m_OrigHeight==0)
		return false;
	if (m_PanAndScan&PANANDSCAN_HORZ) {
		SrcWidth=m_VideoInfo.m_OrigWidth*12/16;
		SrcX=(m_VideoInfo.m_OrigWidth-SrcWidth)/2;
	} else {
		SrcWidth=m_VideoInfo.m_DisplayWidth;
		SrcX=m_VideoInfo.m_PosX;
	}
	if (m_PanAndScan&PANANDSCAN_VERT) {
		SrcHeight=m_VideoInfo.m_OrigHeight*9/12;
		SrcY=(m_VideoInfo.m_OrigHeight-SrcHeight)/2;
	} else {
		SrcHeight=m_VideoInfo.m_DisplayHeight;
		SrcY=m_VideoInfo.m_PosY;
	}
	pRect->left=SrcX;
	pRect->top=SrcY;
	pRect->right=SrcX+SrcWidth;
	pRect->bottom=SrcY+SrcHeight;
	return true;
}


const bool CMediaViewer::GetDestRect(RECT *pRect)
{
	if (m_pVideoRenderer && pRect) {
		if (m_pVideoRenderer->GetDestPosition(pRect))
			return true;
	}
	return false;
}


const bool CMediaViewer::GetDestSize(WORD *pWidth,WORD *pHeight)
{
	RECT rc;

	if (!GetDestRect(&rc))
		return false;
	if (pWidth)
		*pWidth=(WORD)(rc.right-rc.left);
	if (pHeight)
		*pHeight=(WORD)(rc.bottom-rc.top);
	return true;
}


bool CMediaViewer::SetVisible(bool fVisible)
{
	if (m_pVideoRenderer)
		return m_pVideoRenderer->SetVisible(fVisible);
	return false;
}


const void CMediaViewer::HideCursor(bool bHide)
{
	if (m_pVideoRenderer)
		m_pVideoRenderer->ShowCursor(!bHide);
}


const bool CMediaViewer::GetCurrentImage(BYTE **ppDib)
{
	/*
	CTryBlockLock Lock(&m_CriticalLock);
	if (!Lock.TryLock(1000))
		return false;
	*/

	bool fOK=false;

	if (m_pVideoRenderer) {
		void *pBuffer;

		if (m_pVideoRenderer->GetCurrentImage(&pBuffer)) {
			fOK=true;
			*ppDib=static_cast<BYTE*>(pBuffer);
		}
	}
	return fOK;
}


#ifdef USE_GRABBER_FILTER

bool CMediaViewer::SetGrabber(bool bGrabber)
{
	m_bGrabber=bGrabber;
	return true;
}


void *CMediaViewer::DoCapture(DWORD WaitTime)
{
	void *pDib;

	if (m_pGrabber==NULL)
		return NULL;
	if (!m_pGrabber->SetCapture(true))
		return NULL;
	if (!m_pGrabber->WaitCapture(WaitTime)) {
		m_pGrabber->SetCapture(false);
		return NULL;
	}
	pDib=m_pGrabber->GetCaptureBitmap();
	m_pGrabber->SetCapture(false);
	return pDib;
}

#endif


bool CMediaViewer::SetAudioNormalize(bool bNormalize,float Level)
{
	if (m_pAacDecClass==NULL)
		return false;
	return m_pAacDecClass->SetNormalize(bNormalize,Level);
}


CVideoRenderer::RendererType CMediaViewer::GetVideoRendererType() const
{
	return m_VideoRendererType;
}


bool CMediaViewer::SetUseAudioRendererClock(bool bUse)
{
	m_bUseAudioRendererClock=bUse;
	return true;
}


const bool CMediaViewer::RepaintVideo(HWND hwnd,HDC hdc)
{
	if (m_pVideoRenderer)
		return m_pVideoRenderer->RepaintVideo(hwnd,hdc);
	return false;
}


const bool CMediaViewer::DisplayModeChanged()
{
	if (m_pVideoRenderer)
		return m_pVideoRenderer->DisplayModeChanged();
	return false;
}


const bool CMediaViewer::DrawText(LPCTSTR pszText,int x,int y,
								  HFONT hfont,COLORREF crColor,int Opacity)
{
	IBaseFilter *pRenderer;
	int Width,Height;

	if (m_pVideoRenderer==NULL || !IsDrawTextSupported())
		return false;
	pRenderer=m_pVideoRenderer->GetRendererFilter();
	if (pRenderer==NULL)
		return false;
	if (m_pImageMixer==NULL) {
		m_pImageMixer=CImageMixer::CreateImageMixer(m_VideoRendererType,pRenderer);
		if (m_pImageMixer==NULL)
			return false;
	}
	if (!m_pImageMixer->GetMapSize(&Width,&Height))
		return false;
	x=x*Width/m_VideoInfo.m_OrigWidth;
	y=y*Height/m_VideoInfo.m_OrigHeight;
	return m_pImageMixer->SetText(pszText,x,y,hfont,crColor,Opacity);
}


const bool CMediaViewer::IsDrawTextSupported() const
{
	return CImageMixer::IsSupported(m_VideoRendererType);
}


const bool CMediaViewer::ClearOSD()
{
	if (m_pVideoRenderer==NULL)
		return false;
	if (m_pImageMixer!=NULL)
		m_pImageMixer->Clear();
	return true;
}


bool CMediaViewer::SetTracer(CTracer *pTracer)
{
	m_pTracer=pTracer;
	return true;
}


void CMediaViewer::Trace(LPCTSTR pszOutput, ...)
{
	va_list Args;

	va_start(Args,pszOutput);
	if (m_pTracer!=NULL)
		m_pTracer->TraceV(pszOutput,Args);
	va_end(Args);
}


/*
	�t���b�V�����ăf�b�h���b�N���������X���b�h
	�Ƃ肠�����b��
	�{���̓\�[�X�t�B���^�ŕʃX���b�h����T���v���𑗐M����悤�ɂ������������Ǝv��
*/
/*
DWORD WINAPI CMediaViewer::FlushThread(LPVOID lpParameter)
{
	CMediaViewer *pThis=static_cast<CMediaViewer*>(lpParameter);

	::CoInitialize(NULL);

	while (true) {
		if (::WaitForSingleObject(pThis->m_hFlushEvent,2000)==WAIT_OBJECT_0) {
			switch (pThis->m_FlushEventType) {
			case FLUSH_ABORT:
				goto End;
			case FLUSH_WAIT:
				::WaitForSingleObject(pThis->m_hFlushResumeEvent,INFINITE);
			case FLUSH_RESET:
				continue;
			}
		}
		if (pThis->CheckHangUp(100)) {
			::OutputDebugString(TEXT("CMediaViewer::FlushThread() Flush\n"));
			pThis->m_pBonSrcFilterClass->Flush();
			pThis->SendDecoderEvent(EID_FILTER_GRAPH_FLUSH);
		}
	}
End:
	::CoUninitialize();

	::OutputDebugString(TEXT("CMediaViewer::FlushThread() return\n"));
	return 0;
}
*/


bool CMediaViewer::CheckHangUp(DWORD TimeOut)
{
	/*
	if (m_pBonSrcFilterClass)
		return m_pBonSrcFilterClass->CheckHangUp(TimeOut);
	*/
	if (!m_DecoderLock.TryLock(TimeOut))
		return true;
	m_DecoderLock.Unlock();
	return false;
}
