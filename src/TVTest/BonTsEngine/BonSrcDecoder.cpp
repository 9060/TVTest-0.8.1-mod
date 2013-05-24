// BonSrcDecoder.cpp: CBonSrcDecoder �N���X�̃C���v�������e�[�V����
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BonSrcDecoder.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


typedef IBonDriver* (PFCREATEBONDRIVER)(void);


//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////

CBonSrcDecoder::CBonSrcDecoder(IEventHandler *pEventHandler)
	: CMediaDecoder(pEventHandler, 0UL, 1UL)
	, m_pBonDriver(NULL)
	, m_pBonDriver2(NULL)
	, m_hStreamRecvThread(NULL)
	, m_bPauseSignal(false)
	, m_bIsPlaying(false)
	, m_StreamRemain(0)
	, m_StreamThreadPriority(THREAD_PRIORITY_NORMAL)
	, m_bPurgeStreamOnChannelChange(true)
{
}

CBonSrcDecoder::~CBonSrcDecoder()
{
	CloseTuner();
}

void CBonSrcDecoder::Reset(void)
{
	if (m_pBonDriver == NULL)
		return;

	if (!LockStream()) {
		Trace(TEXT("�X�g���[����M�X���b�h���������܂���B"));
		return;
	}

	// �������̃X�g���[����j������
	m_pBonDriver->PurgeTsStream();

	UnlockStream();
}

const bool CBonSrcDecoder::InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex)
{
	// �\�[�X�f�R�[�_�̂��ߓ��͂͏������Ȃ�
	return false;
}

const bool CBonSrcDecoder::OpenTuner(HMODULE hBonDrvDll)
{
	// �I�[�v���`�F�b�N
	if (m_pBonDriver) {
		SetError(ERR_ALREADYOPEN, NULL);
		return false;
	}

	Trace(TEXT("�`���[�i���J���Ă��܂�..."));

	// �h���C�o�|�C���^�̎擾
	PFCREATEBONDRIVER *pfCreateBonDriver=(PFCREATEBONDRIVER*)::GetProcAddress(hBonDrvDll,"CreateBonDriver");
	if (pfCreateBonDriver == NULL) {
		SetError(ERR_DRIVER,TEXT("CreateBonDriver()�̃A�h���X���擾�ł��܂���B"),
							TEXT("�w�肳�ꂽDLL��BonDriver�ł͂���܂���B"));
		return false;
	}
	m_pBonDriver = pfCreateBonDriver();
	if (m_pBonDriver == NULL) {
		SetError(ERR_DRIVER,TEXT("IBonDriver���擾�ł��܂���B"),
							TEXT("CreateBonDriver()�̌Ăяo����NULL���Ԃ���܂����B"));
		return false;
	}

	const HANDLE hThread = ::GetCurrentThread();
	const int ThreadPriority = ::GetThreadPriority(hThread);
	BOOL bTunerOpened;

	try {
		// �`���[�i���J��
		bTunerOpened = m_pBonDriver->OpenTuner();

		// �Ȃ����X���b�h�̗D��x��ς���BonDriver������̂Ō��ɖ߂�
		::SetThreadPriority(hThread, ThreadPriority);

		if (!bTunerOpened) {
			SetError(ERR_TUNEROPEN,TEXT("�`���[�i���J���܂���B"),
								   TEXT("BonDriver�Ƀ`���[�i���J���悤�v�����܂������G���[���Ԃ���܂����B"));
			throw ERR_TUNEROPEN;
		}

		// IBonDriver2�C���^�t�F�[�X�擾
		m_pBonDriver2 = dynamic_cast<IBonDriver2 *>(m_pBonDriver);

		// �X�g���[����M�X���b�h�N��
		m_EndEvent.Create();
		m_bPauseSignal = false;
		m_bIsPlaying = false;
		m_hStreamRecvThread = (HANDLE)::_beginthreadex(NULL, 0, CBonSrcDecoder::StreamRecvThread, this, 0, NULL);
		if (!m_hStreamRecvThread) {
			SetError(ERR_INTERNAL, TEXT("�X�g���[����M�X���b�h���쐬�ł��܂���B"));
			throw ERR_INTERNAL;
		}
	} catch (...) {
		if (bTunerOpened)
			m_pBonDriver->CloseTuner();
		m_pBonDriver->Release();
		m_pBonDriver = NULL;
		m_pBonDriver2 = NULL;
		return false;
	}

	ClearError();

	Trace(TEXT("�`���[�i���J���܂����B"));

	return true;
}

const bool CBonSrcDecoder::CloseTuner(void)
{
	// �X�g���[����~
	m_bIsPlaying = false;

	if (m_hStreamRecvThread) {
		// �X�g���[����M�X���b�h��~
		Trace(TEXT("�X�g���[����M�X���b�h���~���Ă��܂�..."));
		m_EndEvent.Set();
		m_bPauseSignal = true;
		if (::WaitForSingleObject(m_hStreamRecvThread, 5000UL) != WAIT_OBJECT_0) {
			// �X���b�h�����I��
			Trace(TEXT("�X�g���[����M�X���b�h���������Ȃ����ߋ����I�����܂��B"));
			::TerminateThread(m_hStreamRecvThread, -1);
		}
		::CloseHandle(m_hStreamRecvThread);
		m_hStreamRecvThread = NULL;
	}

	m_EndEvent.Close();

	if (m_pBonDriver) {
		// �`���[�i�����
		Trace(TEXT("�`���[�i����Ă��܂�..."));
		m_pBonDriver->CloseTuner();

		// �h���C�o�C���X�^���X�J��
		Trace(TEXT("BonDriver�C���^�[�t�F�[�X��������Ă��܂�..."));
		m_pBonDriver->Release();
		m_pBonDriver = NULL;
		m_pBonDriver2 = NULL;
		Trace(TEXT("BonDriver�C���^�[�t�F�[�X��������܂����B"));
	}

	m_BitRateCalculator.Reset();
	m_StreamRemain = 0;

	ClearError();

	return true;
}

const bool CBonSrcDecoder::IsOpen() const
{
	return m_pBonDriver != NULL;
}

const bool CBonSrcDecoder::Play(void)
{
	TRACE(TEXT("CBonSrcDecoder::Play()\n"));

	if (m_pBonDriver == NULL) {
		// �`���[�i���J����Ă��Ȃ�
		SetError(ERR_NOTOPEN, NULL);
		return false;
	}

	if (m_bIsPlaying) {
		// ���ɍĐ���
		/*
		SetError(ERR_ALREADYPLAYING,NULL);
		return false;
		*/
		return true;
	}

	if (!LockStream()) {
		SetError(ERR_TIMEOUT, TEXT("�X�g���[����M�X���b�h���������܂���B"));
		return false;
	}

	// �������̃X�g���[����j������
	m_pBonDriver->PurgeTsStream();

	// ���ʃf�R�[�_�����Z�b�g����
	ResetDownstreamDecoder();

	// �X�g���[�����Đ���Ԃɂ���
	m_bIsPlaying = true;

	UnlockStream();

	ClearError();

	return true;
}

const bool CBonSrcDecoder::Stop(void)
{
	TRACE(TEXT("CBonSrcDecoder::Stop()\n"));

	if (!m_bIsPlaying) {
		// �X�g���[���͍Đ����łȂ�
		/*
		SetError(ERR_NOTPLAYING, NULL);
		return false;
		*/
		return true;
	}

	if (!LockStream()) {
		SetError(ERR_TIMEOUT, TEXT("�X�g���[����M�X���b�h���������܂���B"));
		return false;
	}

	m_bIsPlaying = false;

	UnlockStream();

	ClearError();

	return true;
}

const bool CBonSrcDecoder::SetChannel(const BYTE byChannel)
{
	TRACE(TEXT("CBonSrcDecoder::SetChannel(%d)\n"), byChannel);

	if (m_pBonDriver == NULL) {
		// �`���[�i���J����Ă��Ȃ�
		SetError(ERR_NOTOPEN, NULL);
		return false;
	}

	if (!LockStream()) {
		SetError(ERR_TIMEOUT, TEXT("�X�g���[����M�X���b�h���������܂���B"));
		return false;
	}

	// �������̃X�g���[����j������
	if (m_bPurgeStreamOnChannelChange)
		m_pBonDriver->PurgeTsStream();

	// �`�����l����ύX����
	if (!m_pBonDriver->SetChannel(byChannel)) {
		UnlockStream();
		SetError(ERR_TUNER,TEXT("�`�����l���̕ύX��BonDriver�Ɏ󂯕t�����܂���B"),
						   TEXT("IBonDriver::SetChannel()�̌Ăяo���ŃG���[���Ԃ���܂����B"));
		return false;
	}

	// ���ʃf�R�[�_�����Z�b�g����
	ResetDownstreamDecoder();

	UnlockStream();

	ClearError();

	return true;
}

const bool CBonSrcDecoder::SetChannel(const DWORD dwSpace, const DWORD dwChannel)
{
	TRACE(TEXT("CBonSrcDecoder::SetChannel(%lu, %lu)\n"), dwSpace, dwChannel);

	if (m_pBonDriver2 == NULL) {
		// �`���[�i���J����Ă��Ȃ�
		SetError(ERR_NOTOPEN, NULL);
		return false;
	}

	if (!LockStream()) {
		SetError(ERR_TIMEOUT, TEXT("�X�g���[����M�X���b�h���������܂���B"));
		return false;
	}

	// �������̃X�g���[����j������
	if (m_bPurgeStreamOnChannelChange)
		m_pBonDriver2->PurgeTsStream();

	// �`�����l����ύX����
	if (!m_pBonDriver2->SetChannel(dwSpace, dwChannel)) {
		UnlockStream();
		SetError(ERR_TUNER,TEXT("�`�����l���̕ύX��BonDriver�Ɏ󂯕t�����܂���B"),
						   TEXT("IBonDriver2::SetChannel()�̌Ăяo���ŃG���[���Ԃ���܂����B"));
		return false;
	}

	// ���ʃf�R�[�_�����Z�b�g����
	ResetDownstreamDecoder();

	UnlockStream();

	ClearError();

	return true;
}

const bool CBonSrcDecoder::SetChannelAndPlay(const DWORD dwSpace, const DWORD dwChannel)
{
	TRACE(TEXT("CBonSrcDecoder::SetChannelAndPlay(%lu, %lu)\n"), dwSpace, dwChannel);

	if (m_pBonDriver2 == NULL) {
		// �`���[�i���J����Ă��Ȃ�
		SetError(ERR_NOTOPEN, NULL);
		return false;
	}

	if (!LockStream()) {
		SetError(ERR_TIMEOUT, TEXT("�X�g���[����M�X���b�h���������܂���B"));
		return false;
	}

	// �������̃X�g���[����j������
	if (m_bPurgeStreamOnChannelChange)
		m_pBonDriver2->PurgeTsStream();

	// �`�����l����ύX����
	if (!m_pBonDriver2->SetChannel(dwSpace, dwChannel)) {
		UnlockStream();
		SetError(ERR_TUNER,TEXT("�`�����l���̕ύX��BonDriver�Ɏ󂯕t�����܂���B"),
						   TEXT("IBonDriver2::SetChannel()�̌Ăяo���ŃG���[���Ԃ���܂����B"));
		return false;
	}

	// ���ʃf�R�[�_�����Z�b�g����
	ResetDownstreamDecoder();

	m_bIsPlaying = true;

	UnlockStream();

	ClearError();

	return true;
}

const float CBonSrcDecoder::GetSignalLevel(void)
{
	if (m_pBonDriver == NULL) {
		// �`���[�i���J����Ă��Ȃ�
		SetError(ERR_NOTOPEN, NULL);
		return 0.0f;
	}

	ClearError();

	// �M�����x����Ԃ�
	return m_pBonDriver->GetSignalLevel();
}

const bool CBonSrcDecoder::IsBonDriver2(void) const
{
	// IBonDriver2�C���^�t�F�[�X�̎g�p�ۂ�Ԃ�
	return m_pBonDriver2 != NULL;
}

LPCTSTR CBonSrcDecoder::GetSpaceName(const DWORD dwSpace) const
{
	// �`���[�j���O��Ԗ���Ԃ�
	if (m_pBonDriver2 == NULL) {
		//SetError(ERR_NOTOPEN, NULL);
		return NULL;
	}
	return m_pBonDriver2->EnumTuningSpace(dwSpace);
}

LPCTSTR CBonSrcDecoder::GetChannelName(const DWORD dwSpace, const DWORD dwChannel) const
{
	// �`�����l������Ԃ�
	if (m_pBonDriver2 == NULL) {
		//SetError(ERR_NOTOPEN, NULL);
		return NULL;
	}
	return m_pBonDriver2->EnumChannelName(dwSpace, dwChannel);
}

const bool CBonSrcDecoder::PurgeStream(void)
{
	TRACE(TEXT("CBonSrcDecoder::PurgeStream()\n"));

	if (m_pBonDriver == NULL) {
		// �`���[�i���J����Ă��Ȃ�
		SetError(ERR_NOTOPEN, NULL);
		return false;
	}

	if (!LockStream()) {
		SetError(ERR_TIMEOUT, TEXT("�X�g���[����M�X���b�h���������܂���B"));
		return false;
	}

	// �������̃X�g���[����j������
	m_pBonDriver->PurgeTsStream();

	UnlockStream();

	ClearError();

	return true;
}

int CBonSrcDecoder::NumSpaces() const
{
	int i;

	for (i = 0; GetSpaceName(i) != NULL; i++);
	return i;
}

LPCTSTR CBonSrcDecoder::GetTunerName() const
{
	if (m_pBonDriver2 == NULL) {
		//SetError(ERR_NOTOPEN, NULL);
		return NULL;
	}
	return m_pBonDriver2->GetTunerName();
}

int CBonSrcDecoder::GetCurSpace() const
{
	if (m_pBonDriver2 == NULL) {
		//SetError(ERR_NOTOPEN, NULL);
		return -1;
	}
	return m_pBonDriver2->GetCurSpace();
}

int CBonSrcDecoder::GetCurChannel() const
{
	if (m_pBonDriver2 == NULL) {
		//SetError(ERR_NOTOPEN, NULL);
		return -1;
	}
	return m_pBonDriver2->GetCurChannel();
}

DWORD CBonSrcDecoder::GetBitRate() const
{
	return m_BitRateCalculator.GetBitRate();
}

DWORD CBonSrcDecoder::GetStreamRemain() const
{
	return m_StreamRemain;
}

bool CBonSrcDecoder::SetStreamThreadPriority(int Priority)
{
	if (m_StreamThreadPriority != Priority) {
		TRACE(TEXT("CBonSrcDecoder::SetStreamThreadPriority(%d)\n"), Priority);
		if (m_hStreamRecvThread) {
			if (!::SetThreadPriority(m_hStreamRecvThread, Priority))
				return false;
		}
		m_StreamThreadPriority = Priority;
	}
	return true;
}

void CBonSrcDecoder::SetPurgeStreamOnChannelChange(bool bPurge)
{
	TRACE(TEXT("CBonSrcDecoder::SetPurgeStreamOnChannelChange(%s)\n"),
		  bPurge ? TEXT("true") : TEXT("false"));
	m_bPurgeStreamOnChannelChange = bPurge;
}

unsigned int __stdcall CBonSrcDecoder::StreamRecvThread(LPVOID pParam)
{
	// �`���[�i����TS�f�[�^�����o���X���b�h
	CBonSrcDecoder *pThis = static_cast<CBonSrcDecoder *>(pParam);

	::CoInitialize(NULL);

	::SetThreadPriority(::GetCurrentThread(),pThis->m_StreamThreadPriority);

	pThis->m_BitRateCalculator.Initialize();

	CMediaData TsStream(0x10000UL);

	DWORD Wait;
	do {
		if (pThis->m_bPauseSignal) {
			Wait = 100;
		} else {
			BYTE *pStreamData = NULL;
			DWORD dwStreamSize = 0;
			DWORD dwStreamRemain = 0;

			pThis->m_StreamLock.Lock();
			if (pThis->m_pBonDriver->GetTsStream(&pStreamData, &dwStreamSize, &dwStreamRemain)
					&& pStreamData && dwStreamSize) {
				if (pThis->m_bIsPlaying) {
					// �ŏ�ʃf�R�[�_�ɓ��͂���
					TsStream.SetData(pStreamData, dwStreamSize);
					pThis->OutputMedia(&TsStream);
				}
			} else {
				dwStreamSize = 0;
				dwStreamRemain = 0;
			}
			pThis->m_StreamLock.Unlock();

			pThis->m_BitRateCalculator.Update(dwStreamSize);
			pThis->m_StreamRemain = dwStreamRemain;

			if (dwStreamRemain != 0)
				Wait = 0;
			else
				Wait = 10;
		}
	} while (pThis->m_EndEvent.Wait(Wait) == WAIT_TIMEOUT);

	pThis->m_BitRateCalculator.Reset();

	::CoUninitialize();

	TRACE(TEXT("CBonSrcDecoder::StreamRecvThread() return\n"));

	return 0;
}

bool CBonSrcDecoder::LockStream()
{
	m_bPauseSignal = true;
	bool bOK = m_StreamLock.TryLock(5000);
	m_bPauseSignal = false;
	return bOK;
}

void CBonSrcDecoder::UnlockStream()
{
	m_StreamLock.Unlock();
}
