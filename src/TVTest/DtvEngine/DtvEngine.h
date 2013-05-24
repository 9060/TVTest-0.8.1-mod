// DtvEngine.h: CDtvEngine �N���X�̃C���^�[�t�F�C�X
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include "BonBaseClass.h"
#include "BonSrcDecoder.h"
#include "TsPacketParser.h"
#include "TsAnalyzer.h"
#include "TsDescrambler.h"
#include "MediaViewer.h"
#include "MediaTee.h"
#include "FileWriter.h"
//#include "FileReader.h"
#include "MediaBuffer.h"
#include "MediaGrabber.h"
#include "TsSelector.h"
#ifndef TVH264
#define DTVENGINE_CAPTION_SUPPORT
#include "CaptionDecoder.h"
#endif

// �����̕ӂ͑S���̎b��ł�


//////////////////////////////////////////////////////////////////////
// �f�W�^��TV�G���W���N���X
//////////////////////////////////////////////////////////////////////

class CDtvEngine : protected CMediaDecoder::IEventHandler, public CBonBaseClass
{
public:
	enum {
		SID_INVALID		= 0xFFFF,
		SERVICE_INVALID	= 0xFFFF
	};

	class __declspec(novtable) CEventHandler {
		friend CDtvEngine;
	public:
		virtual ~CEventHandler() = 0 {}
	protected:
		CDtvEngine *m_pDtvEngine;
		virtual void OnServiceListUpdated(CTsAnalyzer *pTsAnalyzer, bool bStreamChanged) {}
		virtual void OnServiceInfoUpdated(CTsAnalyzer *pTsAnalyzer) {}
		//virtual void OnPcrTimeStampUpdated(CProgManager *pProgManager) {}
		virtual void OnFileWriteError(CFileWriter *pFileWriter) {}
		virtual void OnVideoSizeChanged(CMediaViewer *pMediaViewer) {}
		virtual void OnEmmProcessed(const BYTE *pEmmData) {}
		virtual void OnEcmError(LPCTSTR pszText) {}
	};

	CDtvEngine(void);
	~CDtvEngine(void);

	const bool BuildEngine(CEventHandler *pEventHandler,
						   bool bDescramble = true, bool bBuffering = false);
	const bool IsEngineBuild() const { return m_bBuiled; };
	const bool IsBuildComplete() const;
	const bool CloseEngine(void);
	const bool ResetEngine(void);

	const bool OpenSrcFilter_BonDriver(HMODULE hBonDriverDll);
	//const bool OpenSrcFilter_File(LPCTSTR lpszFileName);
	//const bool PlayFile(LPCTSTR lpszFileName);
	//void StopFile(void);
	const bool ReleaseSrcFilter();
	const bool IsSrcFilterOpen() const;

	const bool EnablePreview(const bool bEnable = true);
	const bool SetViewSize(const int x,const int y);
	const bool SetVolume(const float fVolume);
	const bool GetVideoSize(WORD *pwWidth,WORD *pwHeight);
	const bool GetVideoAspectRatio(BYTE *pbyAspectRateX,BYTE *pbyAspectRateY);
	const BYTE GetAudioChannelNum();
	const int GetAudioStreamNum(const int Service = -1);
	const bool SetAudioStream(int StreamIndex);
	const int GetAudioStream() const;
	const BYTE GetAudioComponentType();
	const int GetAudioComponentText(LPTSTR pszText, int MaxLength);
	const bool SetStereoMode(int iMode);
	const WORD GetEventID(bool bNext = false);
	const int GetEventName(LPTSTR pszName, int MaxLength, bool bNext = false);
	const int GetEventText(LPTSTR pszText, int MaxLength, bool bNext = false);
	const int GetEventExtendedText(LPTSTR pszText, int MaxLength, bool bNext = false);
	const bool GetEventTime(SYSTEMTIME *pStartTime, SYSTEMTIME *pEndTime, bool bNext = false);
	const DWORD GetEventDuration(bool bNext = false);
	const bool GetEventSeriesInfo(CTsAnalyzer::EventSeriesInfo *pInfo, bool bNext = false);
	const bool GetEventInfo(CTsAnalyzer::EventInfo *pInfo, bool bNext = false);
	const bool GetEventAudioInfo(CTsAnalyzer::EventAudioInfo *pInfo, bool bNext = false);
	const bool GetVideoDecoderName(LPWSTR lpName,int iBufLen);
	const bool DisplayVideoDecoderProperty(HWND hWndParent);

	const bool SetChannel(const BYTE byTuningSpace, const WORD wChannel, const WORD ServiceID = SID_INVALID);
	const bool SetService(const WORD wService);
	//const WORD GetService(void);
	const bool GetServiceID(WORD *pServiceID);
	const bool SetServiceByID(const WORD ServiceID, const bool bReserve = true);
	const unsigned __int64 GetPcrTimeStamp();

	bool BuildMediaViewer(HWND hwndHost,HWND hwndMessage,
		CVideoRenderer::RendererType VideoRenderer=CVideoRenderer::RENDERER_DEFAULT,
		LPCWSTR pszMpeg2Decoder=NULL,LPCWSTR pszAudioDevice=NULL);
	bool RebuildMediaViewer(HWND hwndHost,HWND hwndMessage,
		CVideoRenderer::RendererType VideoRenderer=CVideoRenderer::RENDERER_DEFAULT,
		LPCWSTR pszMpeg2Decoder=NULL,LPCWSTR pszAudioDevice=NULL);
	bool CloseMediaViewer();
	bool ResetMediaViewer();
	bool OpenBcasCard(CCardReader::ReaderType CardReaderType, LPCTSTR pszReaderName = NULL);
	bool CloseBcasCard();
	bool SetDescramble(bool bDescramble);
	bool ResetBuffer();
	bool GetOriginalVideoSize(WORD *pWidth,WORD *pHeight);
	bool SetDescrambleService(WORD ServiceID);
	bool SetDescrambleCurServiceOnly(bool bOnly);
	bool GetDescrambleCurServiceOnly() const { return m_bDescrambleCurServiceOnly; }
	bool SetWriteService(WORD ServiceID, DWORD Stream=CTsSelector::STREAM_ALL);
	bool SetWriteCurServiceOnly(bool bOnly, DWORD Stream=CTsSelector::STREAM_ALL);
	bool GetWriteCurServiceOnly() const { return m_bWriteCurServiceOnly; }
	//CEpgDataInfo *GetEpgDataInfo(WORD ServiceID, bool bNext=false);
// CBonBaseClass
	void SetTracer(CTracer *pTracer);

//protected:
	// CMediaDecoder ����h���������f�B�A�f�R�[�_�N���X
	CBonSrcDecoder m_BonSrcDecoder;			// TS�\�[�X�`���[�i�[(HAL�����ׂ�)
	CTsPacketParser m_TsPacketParser;		// TS�p�P�b�^�C�U�[
	CTsAnalyzer m_TsAnalyzer;
	CTsDescrambler m_TsDescrambler;			// TS�f�X�N�����u���[
	CMediaViewer m_MediaViewer;				// ���f�B�A�r���[�A�[
	CMediaTee m_MediaTee;					// ���f�B�A�e�B�[
	CFileWriter m_FileWriter;				// �t�@�C�����C�^�[
	//CFileReader m_FileReader;				// �t�@�C�����[�_�[
	CMediaBuffer m_MediaBuffer;
	CMediaGrabber m_MediaGrabber;
	CTsSelector m_TsSelector;
#ifdef DTVENGINE_CAPTION_SUPPORT
	CCaptionDecoder m_CaptionDecoder;
#endif

protected:
	virtual const DWORD OnDecoderEvent(CMediaDecoder *pDecoder, const DWORD dwEventID, PVOID pParam);

	CCriticalLock m_EngineLock;
	CEventHandler *m_pEventHandler;
	WORD m_wCurTransportStream;
	WORD m_CurServiceIndex;
	WORD m_CurServiceID;
	WORD m_SpecServiceID;
	int m_CurAudioStream;
	unsigned __int64 m_u64CurPcrTimeStamp;

	bool m_bBuiled;
	bool m_bIsFileMode;
	bool m_bDescramble;
	bool m_bBuffering;

	bool m_bDescrambleCurServiceOnly;
	bool m_bWriteCurServiceOnly;
	DWORD m_WriteStream;

	void ResetStatus();
};
