// BonSrcDecoder.h: CBonSrcDecoder �N���X�̃C���^�[�t�F�C�X
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include "MediaDecoder.h"
#include "IBonDriver.h"
#include "TsUtilClass.h"


// �G���[�R�[�h
#define BSDEC_NOERROR			0x00000000UL	// �G���[�Ȃ�
#define BSDEC_TUNERERROR		0x00000001UL	// �`���[�i�G���[
#define BSDEC_INTERNALERROR		0x00000002UL	// �����G���[
#define BSDEC_TUNERNOTOPEN		0x00000003UL	// �`���[�i�����ɕ����Ă���
#define BSDEC_ALREADYOPEN		0x00000004UL	// �`���[�i�����ɊJ����Ă���
#define BSDEC_ALREADYPLAYING	0x00000005UL	// ���ɍĐ�����Ă���
#define BSDEC_NOTPLAYING		0x00000006UL	// �Đ�����Ă��Ȃ�


/////////////////////////////////////////////////////////////////////////////
// Bon�\�[�X�f�R�[�_(�`���[�i����TS�����X�g���[������M����)
/////////////////////////////////////////////////////////////////////////////
// Output	#0	: CMediaData		����TS�X�g���[��
/////////////////////////////////////////////////////////////////////////////

class CBonSrcDecoder : public CMediaDecoder  
{
public:
	CBonSrcDecoder(IEventHandler *pEventHandler = NULL);
	virtual ~CBonSrcDecoder();

// IMediaDecoder
	virtual void Reset(void);

	virtual const DWORD GetInputNum(void) const;
	virtual const DWORD GetOutputNum(void) const;

	virtual const bool InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex = 0UL);

// CBonSrcDecoder
	const bool OpenTuner(void);
	const bool CloseTuner(void);

	const bool Play(void);
	const bool Stop(void);

	const bool SetChannel(const BYTE byChannel);
	const float GetSignalLevel(void);

	const DWORD GetLastError(void) const;

protected:
	const bool PurgeStream(void);
	virtual void OnTsStream(BYTE *pStreamData, DWORD dwStreamSize);

	CMediaData m_TsStream;

	CCriticalLock m_CriticalLock;
	bool m_bIsPlaying;
	DWORD m_dwLastError;

private:
	static DWORD WINAPI StreamRecvThread(LPVOID pParam);

	IBonDriver *m_pBonDriver;
	HANDLE m_hStreamRecvThread;
	bool m_bKillSignal;
};
