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

// �����̕ӂ͑S���̎b��ł�


//////////////////////////////////////////////////////////////////////
// �f�W�^��TV�G���W���N���X
//////////////////////////////////////////////////////////////////////

class CDtvEngineHandler;

class CDtvEngine : protected CMediaDecoder::IEventHandler
{
public:
	CDtvEngine(void);
	~CDtvEngine(void);

	const bool OpenEngine(CDtvEngineHandler *pDtvEngineHandler, HWND hHostHwnd);
	const bool CloseEngine(void);
	const bool ResetEngine(void);
	
	const bool EnablePreview(const bool bEnable = true);

	const bool SetChannel(const BYTE byTuningSpace, const WORD wChannel);
	const bool SetService(const WORD wService);
	const WORD GetService(void) const;
	
	const bool PlayFile(LPCTSTR lpszFileName);
	void StopFile(void);
	
	const DWORD GetLastError(void) const;

//protected:
	const DWORD SendDtvEngineEvent(const DWORD dwEventID, PVOID pParam = NULL);
	virtual const DWORD OnDecoderEvent(CMediaDecoder *pDecoder, const DWORD dwEventID, PVOID pParam);

	// IMediaDecoder ����h���������f�B�A�f�R�[�_�N���X
	CBonSrcDecoder m_BonSrcDecoder;			// TS�\�[�X�`���[�i�[(HAL�����ׂ�)
	CTsPacketParser m_TsPacketParser;		// TS�p�P�b�^�C�U�[
	CTsDescrambler m_TsDescrambler;			// TS�f�X�N�����u���[
	CProgManager m_ProgManager;				// TS�v���O�����}�l�[�W���[
	CMediaViewer m_MediaViewer;				// ���f�B�A�r���[�A�[
	CMediaTee m_MediaTee;					// ���f�B�A�e�B�[
	CFileWriter m_FileWriter;				// �t�@�C�����C�^�[
	CFileReader m_FileReader;				// �t�@�C�����[�_�[

	CDtvEngineHandler *m_pDtvEngineHandler;
	WORD m_wCurService;

	bool m_bIsFileMode;
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
