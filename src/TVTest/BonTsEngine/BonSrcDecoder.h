// BonSrcDecoder.h: CBonSrcDecoder �N���X�̃C���^�[�t�F�C�X
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include "MediaDecoder.h"
#include "IBonDriver.h"
#include "IBonDriver2.h"


/////////////////////////////////////////////////////////////////////////////
// Bon�\�[�X�f�R�[�_(�`���[�i����TS�����X�g���[������M����)
/////////////////////////////////////////////////////////////////////////////
// Output	#0	: CMediaData		����TS�X�g���[��
/////////////////////////////////////////////////////////////////////////////

class CBonSrcDecoder : public CMediaDecoder
{
public:
	// �G���[�R�[�h
	enum {
		ERR_NOERROR,		// �G���[�Ȃ�
		ERR_DRIVER,			// �h���C�o�G���[
		ERR_TUNEROPEN,		// �`���[�i�I�[�v���G���[
		ERR_TUNER,			// �`���[�i�G���[
		ERR_NOTOPEN,		// �`���[�i���J����Ă��Ȃ�
		ERR_ALREADYOPEN,	// �`���[�i�����ɊJ����Ă���
		ERR_NOTPLAYING,		// �Đ�����Ă��Ȃ�
		ERR_ALREADYPLAYING,	// ���ɍĐ�����Ă���
		ERR_TIMEOUT,		// �^�C���A�E�g
		ERR_INTERNAL		// �����G���[
	};

	CBonSrcDecoder(IEventHandler *pEventHandler = NULL);
	virtual ~CBonSrcDecoder();

// IMediaDecoder
	virtual void Reset(void);
	virtual const bool InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex = 0UL);

// CBonSrcDecoder
	const bool OpenTuner(HMODULE hBonDrvDll);
	const bool CloseTuner(void);
	const bool IsOpen() const;

	const bool Play(void);
	const bool Stop(void);

	const bool SetChannel(const BYTE byChannel);
	const bool SetChannel(const DWORD dwSpace, const DWORD dwChannel);
	const bool SetChannelAndPlay(const DWORD dwSpace, const DWORD dwChannel);
	const float GetSignalLevel(void);

	const bool IsBonDriver2(void) const;
	LPCTSTR GetSpaceName(const DWORD dwSpace) const;
	LPCTSTR GetChannelName(const DWORD dwSpace, const DWORD dwChannel) const;

	const bool PurgeStream(void);

	// Append by HDUSTest�̒��̐l
	int NumSpaces() const;
	LPCTSTR GetTunerName() const;
	int GetCurSpace() const;
	int GetCurChannel() const;
	DWORD GetBitRate() const;
	DWORD GetStreamRemain() const;
	bool SetStreamThreadPriority(int Priority);
	void SetPurgeStreamOnChannelChange(bool bPurge);

private:
	static DWORD WINAPI StreamRecvThread(LPVOID pParam);
	bool LockStream();
	void UnlockStream();

	IBonDriver *m_pBonDriver;
	IBonDriver2 *m_pBonDriver2;	

	HANDLE m_hStreamRecvThread;
	CCriticalLock m_StreamLock;
	volatile bool m_bKillSignal;
	volatile bool m_bPauseSignal;

	volatile bool m_bIsPlaying;
	DWORD m_dwLastError;

	CBitRateCalculator m_BitRateCalculator;
	DWORD m_StreamRemain;

	int m_StreamThreadPriority;
	bool m_bPurgeStreamOnChannelChange;
};
