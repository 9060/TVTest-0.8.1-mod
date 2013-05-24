// MediaViewer.h: CMediaViewer �N���X�̃C���^�[�t�F�C�X
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include "MediaDecoder.h"
#include "TsUtilClass.h"
#include "BonSrcFilter.h"
#include "AacDecFilter.h"
#include <Bdaiface.h>


/////////////////////////////////////////////////////////////////////////////
// ���f�B�A�r���[�A(�f���y�щ������Đ�����)
/////////////////////////////////////////////////////////////////////////////
// Input	#0	: CTsPacket		���̓f�[�^
/////////////////////////////////////////////////////////////////////////////

class CMediaViewer : public CMediaDecoder  
{
public:
	CMediaViewer(IEventHandler *pEventHandler = NULL);
	virtual ~CMediaViewer();

// IMediaDecoder
	virtual void Reset(void);

	virtual const DWORD GetInputNum(void) const;
	virtual const DWORD GetOutputNum(void) const;

	virtual const bool InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex = 0UL);

// CMediaViewer
	const bool OpenViewer(HWND hOwnerHwnd = NULL);
	void CloseViewer(void);

	const bool Play(void);
	const bool Stop(void);
	
	const bool SetVideoPID(const WORD wPID);
	const bool SetAudioPID(const WORD wPID);

protected:
	CCriticalLock m_CriticalLock;

	// DirectShow�C���^�t�F�[�X
	IGraphBuilder *m_pFilterGraph;
	IMediaControl *m_pMediaControl;
	
	// DirectShow�t�B���^
	CBonSrcFilter *m_pBonSrcFilter;
	CAacDecFilter *m_pAacDecFilter;
	IBaseFilter *m_pMp2DemuxFilter;

	// MPEG2Demultiplexer�C���^�t�F�[�X
	IMpeg2Demultiplexer *m_pMp2DemuxInterface;
	IReferenceClock *m_pMp2DemuxRefClock;

	// DirectShow�s��
	IPin *m_pBonSrcOutputPin;

	IPin *m_pMp2DemuxInputPin;
	IPin *m_pMp2DemuxVideoPin;
	IPin *m_pMp2DemuxAudioPin;

	IPin *m_pAacDecInputPin;
	IPin *m_pAacDecOutputPin;
	
	// �r�f�IPID�}�b�v
	IMPEG2PIDMap *m_pMp2DemuxVideoMap;
	IMPEG2PIDMap *m_pMp2DemuxAudioMap;
	
	// �r�f�I�E�B���h�E
	IVideoWindow *m_pVideoWindow;
	
	// Elementary Stream��PID
	WORD m_wVideoEsPID;
	WORD m_wAudioEsPID;
	
private:
	HRESULT AddToRot(IUnknown *pUnkGraph, DWORD *pdwRegister) const;
	void RemoveFromRot(const DWORD pdwRegister) const;

	DWORD m_dwRegister;
};
