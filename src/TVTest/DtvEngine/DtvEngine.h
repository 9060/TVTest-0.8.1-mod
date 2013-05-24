// DtvEngine.h: CDtvEngine �N���X�̃C���^�[�t�F�C�X
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include "BonSrcDecoder.h"
#include "TsPacketParser.h"
#include "TsDescrambler.h"
#include "ProgManager.h"
#include "MediaViewer.h"
#include "MediaTee.h"
#include "FileWriter.h"
#include "FileReader.h"
#include "MediaBuffer.h"
#include "Exception.h"
#include "../Tracer.h"

// �����̕ӂ͑S���̎b��ł�


//////////////////////////////////////////////////////////////////////
// �f�W�^��TV�G���W���N���X
//////////////////////////////////////////////////////////////////////

class CDtvEngineHandler;

class CDtvEngine : protected CMediaDecoder::IEventHandler, public CBonErrorHandler
{
public:
	enum EVENTID
	{
		EID_SERVICE_LIST_UPDATED,	// �T�[�r�X���X�g�X�V
		EID_SERVICE_INFO_UPDATED,	// �T�[�r�X���X�V
		EID_PCR_TIMESTAMP_UPDATED,	// PCR�^�C���X�^���v�X�V(���Ȃ�p��)
		EID_FILE_WRITE_ERROR
	};

	CDtvEngine(void);
	~CDtvEngine(void);

	const bool BuildEngine(CDtvEngineHandler *pDtvEngineHandler,
								bool bDescramble=true,bool bBuffering=false);
	const bool IsEngineBuild() const { return m_bBuiled; };
	const bool IsBuildComplete() const { return m_bBuildComplete; }
	const bool CloseEngine(void);
	const bool ResetEngine(void);

	const bool OpenSrcFilter_BonDriver(HMODULE hBonDriverDll);
	const bool OpenSrcFilter_File(LPCTSTR lpszFileName);
	const bool ReleaseSrcFilter();
	const bool IsSrcFilterOpen() const;

	const bool EnablePreview(const bool bEnable = true);
	const bool SetViewSize(const int x,const int y);
	const bool SetVolume(const float fVolume);
	const bool GetVideoSize(WORD *pwWidth,WORD *pwHeight);
	const bool GetVideoAspectRatio(BYTE *pbyAspectRateX,BYTE *pbyAspectRateY);
	const BYTE GetAudioChannelNum();
	const bool SetStereoMode(int iMode);
	const bool GetVideoDecoderName(LPWSTR lpName,int iBufLen);
	const bool DisplayVideoDecoderProperty(HWND hWndParent);

	const bool SetChannel(const BYTE byTuningSpace, const WORD wChannel);
	const bool SetService(const WORD wService);
	const WORD GetService(void) const;
	const unsigned __int64 GetPcrTimeStamp() const;

	/*
	const bool PlayFile(LPCTSTR lpszFileName);
	void StopFile(void);
	*/

	// Append by HDUSTest�̒��̐l
	bool BuildMediaViewer(HWND hwndHost,HWND hwndMessage,
		CVideoRenderer::RendererType VideoRenderer=CVideoRenderer::RENDERER_DEFAULT,
		LPCWSTR pszMpeg2Decoder=NULL);
	bool RebuildMediaViewer(HWND hwndHost,HWND hwndMessage,
		CVideoRenderer::RendererType VideoRenderer=CVideoRenderer::RENDERER_DEFAULT,
		LPCWSTR pszMpeg2Decoder=NULL);
	bool OpenBcasCard(CCardReader::ReaderType CardReaderType);
	bool SetDescramble(bool bDescramble);
	bool ResetBuffer();
	bool GetOriginalVideoSize(WORD *pWidth,WORD *pHeight) const;
	bool SetDescrambleService(WORD Service);
	bool SetDescrambleCurServiceOnly(bool bOnly);
	bool GetDescrambleCurServiceOnly() const { return m_bDescrambleCurServiceOnly; }
	CEpgDataInfo *GetEpgDataInfo(WORD wSID, bool bNext);
	bool SetTracer(CTracer *pTracer);

//protected:
	// IMediaDecoder ����h���������f�B�A�f�R�[�_�N���X
	CBonSrcDecoder m_BonSrcDecoder;			// TS�\�[�X�`���[�i�[(HAL�����ׂ�)
	CTsPacketParser m_TsPacketParser;		// TS�p�P�b�^�C�U�[
	CTsDescrambler m_TsDescrambler;			// TS�f�X�N�����u���[
	CProgManager m_ProgManager;				// TS�v���O�����}�l�[�W���[
	CMediaViewer m_MediaViewer;				// ���f�B�A�r���[�A�[
	CMediaTee m_MediaTee;					// ���f�B�A�e�B�[
	CFileWriter m_FileWriter;				// �t�@�C�����C�^�[
	CFileReader m_FileReader;				// �t�@�C�����[�_�[
	CMediaBuffer m_MediaBuffer;

protected:
	const DWORD SendDtvEngineEvent(const DWORD dwEventID, PVOID pParam = NULL);
	virtual const DWORD OnDecoderEvent(CMediaDecoder *pDecoder, const DWORD dwEventID, PVOID pParam);
	bool CheckBuildComplete() const;

	CDtvEngineHandler *m_pDtvEngineHandler;
	WORD m_wCurTransportStream;
	WORD m_wCurService;
	WORD m_wCurVideoPID;
	WORD m_wCurAudioPID;
	unsigned __int64 m_u64CurPcrTimeStamp;

	bool m_bBuiled;
	bool m_bBuildComplete;
	bool m_bIsFileMode;
	bool m_bDescramble;

	bool m_bDescrambleCurServiceOnly;

	CTracer *m_pTracer;
	void Trace(LPCTSTR pszOutput, ...);
	void ResetParams();
};


//////////////////////////////////////////////////////////////////////
// �f�W�^��TV�C�x���g�n���h���C���^�t�F�[�X
//////////////////////////////////////////////////////////////////////

// ����͏������z�֐��Ƃ��ׂ�
class CDtvEngineHandler
{
friend CDtvEngine;

protected:
	virtual const DWORD OnDtvEngineEvent(CDtvEngine *pEngine, const DWORD dwEventID, PVOID pParam);
};
