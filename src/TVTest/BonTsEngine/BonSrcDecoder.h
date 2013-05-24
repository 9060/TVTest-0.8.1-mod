// BonSrcDecoder.h: CBonSrcDecoder �N���X�̃C���^�[�t�F�C�X
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include "MediaDecoder.h"
#include "IBonDriver.h"
#include "IBonDriver2.h"
#include "TsUtilClass.h"


// �G���[�R�[�h
#define BSDEC_NOERROR			0x00000000UL	// �G���[�Ȃ�
#define BSDEC_TUNERERROR		0x00000001UL	// �`���[�i�G���[
#define BSDEC_INTERNALERROR		0x00000002UL	// �����G���[
#define BSDEC_TUNERNOTOPEN		0x00000003UL	// �`���[�i�����ɕ����Ă���
#define BSDEC_ALREADYOPEN		0x00000004UL	// �`���[�i�����ɊJ����Ă���
#define BSDEC_ALREADYPLAYING	0x00000005UL	// ���ɍĐ�����Ă���
#define BSDEC_NOTPLAYING		0x00000006UL	// �Đ�����Ă��Ȃ�
#define BSDEC_DRIVERERROR		0x00000007UL	// �h���C�o�G���[


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
	virtual const bool InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex = 0UL);

// CBonSrcDecoder
	const bool OpenTuner(HMODULE hBonDrvDll);
	const bool CloseTuner(void);
	const bool IsOpen() const;

	const bool Play(void);
	const bool Stop(void);

	const bool SetChannel(const BYTE byChannel);
	const bool SetChannel(const DWORD dwSpace, const DWORD dwChannel);
	const float GetSignalLevel(void);

	const bool IsBonDriver2(void) const;
	LPCTSTR GetSpaceName(const DWORD dwSpace) const;
	LPCTSTR GetChannelName(const DWORD dwSpace, const DWORD dwChannel) const;

	const DWORD GetLastError(void) const;
	const bool PurgeStream(void);

	// Append by HDUSTest�̒��̐l
	int NumSpaces() const;
	LPCTSTR GetTunerName() const;
	int GetCurSpace() const;
	int GetCurChannel() const;
	DWORD GetBitRate() const;
	DWORD GetStreamRemain() const;
protected:
	virtual void OnTsStream(BYTE *pStreamData, DWORD dwStreamSize);

	void PauseStreamRecieve();
	void ResumeStreamRecieve();
	void ResetBitRate();

	CMediaData m_TsStream;

	CCriticalLock m_CriticalLock;
	bool m_bIsPlaying;
	DWORD m_dwLastError;

	DWORD m_BitRateTime;
	DWORD m_BitRate;
	DWORD m_StreamRemain;
private:
	static DWORD WINAPI StreamRecvThread(LPVOID pParam);

	IBonDriver *m_pBonDriver;
	IBonDriver2 *m_pBonDriver2;	

	HANDLE m_hStreamRecvThread;
	volatile bool m_bPauseSignal;
	HANDLE m_hResumeEvent;
	volatile bool m_bKillSignal;

	int m_RequestSpace;
	int m_RequestChannel;
	bool m_bSetChannelResult;
};
