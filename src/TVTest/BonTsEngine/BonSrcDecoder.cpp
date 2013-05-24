// BonSrcDecoder.cpp: CBonSrcDecoder �N���X�̃C���v�������e�[�V����
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <TypeInfo.h>
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
	, m_PauseEvent()
	, m_ResumeEvent()
	, m_bKillSignal(false)
	, m_TsStream(0x10000UL)
	, m_bIsPlaying(false)
	, m_BitRate(0)
	, m_StreamRemain(0)
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

	if (!PauseStreamRecieve()) {
		Trace(TEXT("�X�g���[����M�X���b�h���������܂���B"));
		return;
	}

	// �������̃X�g���[����j������
	m_pBonDriver->PurgeTsStream();

	ResumeStreamRecieve();
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
		SetError(ERR_ALREADYOPEN,NULL);
		return false;
	}

	Trace(TEXT("�`���[�i���J���Ă��܂�..."));

	// �h���C�o�|�C���^�̎擾
	PFCREATEBONDRIVER *pf=(PFCREATEBONDRIVER*)::GetProcAddress(hBonDrvDll,"CreateBonDriver");
	if (pf == NULL) {
		SetError(ERR_DRIVER,TEXT("CreateBonDriver()�̃A�h���X���擾�ł��܂���B"),
							TEXT("�w�肳�ꂽDLL��BonDriver�ł͂���܂���B"));
		return false;
	}
	if ((m_pBonDriver = pf()) == NULL) {
		SetError(ERR_DRIVER,TEXT("IBonDriver���擾�ł��܂���B"),
							TEXT("CreateBonDriver()�̌Ăяo����NULL���Ԃ���܂����B"));
		return false;
	}

	// �`���[�i���J��
	if (!m_pBonDriver->OpenTuner()) {
		SetError(ERR_TUNEROPEN,TEXT("�`���[�i���J���܂���B"),
							   TEXT("IBonDriver::OpenTuner()�̌Ăяo���ŃG���[���Ԃ���܂����B"));
		goto OnError;
	}

	// BonDriver_HDUS���A�Ȃ�������ɃX���b�h�̗D��x��HIGHEST�ɂ���̂Ō��ɖ߂�
	::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_NORMAL);

	// IBonDriver2�C���^�t�F�[�X�擾
	m_pBonDriver2 = dynamic_cast<IBonDriver2 *>(m_pBonDriver);

	/*
	// �����`�����l�����Z�b�g����
	if (m_pBonDriver2) {
		// IBonDriver2
		if (!m_pBonDriver2->SetChannel(0UL, 0UL)) {
			m_dwLastError = ERR_TUNER;
			goto OnError;
		}
	} else {
		// IBonDriver
		if (!m_pBonDriver->SetChannel(13U)) {
			m_dwLastError = ERR_TUNER;
			goto OnError;
		}
	}
	*/

	// �X�g���[����M�X���b�h�N��
	if (!m_PauseEvent.Create(true) || !m_ResumeEvent.Create()) {
		SetError(ERR_INTERNAL, TEXT("�C�x���g�I�u�W�F�N�g���쐬�ł��܂���B"));
		goto OnError;
	}
	m_bKillSignal = false;
	m_bIsPlaying = false;
	m_hStreamRecvThread = ::CreateThread(NULL, 0UL, CBonSrcDecoder::StreamRecvThread, this, 0UL, NULL);
	if (!m_hStreamRecvThread) {
		SetError(ERR_INTERNAL, TEXT("�X�g���[����M�X���b�h���쐬�ł��܂���B"));
		goto OnError;
	}

	ClearError();

	Trace(TEXT("�`���[�i���J���܂����B"));

	return true;

OnError:
	m_pBonDriver->CloseTuner();
	m_pBonDriver->Release();
	m_pBonDriver = NULL;
	m_pBonDriver2 = NULL;
	m_PauseEvent.Close();
	m_ResumeEvent.Close();
	return false;
}

const bool CBonSrcDecoder::CloseTuner(void)
{
	// �X�g���[����~
	m_bIsPlaying = false;

	if (m_hStreamRecvThread) {
		// �X�g���[����M�X���b�h��~
		Trace(TEXT("�X�g���[����M�X���b�h���~���Ă��܂�..."));
		m_bKillSignal = true;
		m_PauseEvent.Set();
		if (::WaitForSingleObject(m_hStreamRecvThread, 1000UL) != WAIT_OBJECT_0) {
			// �X���b�h�����I��
			::TerminateThread(m_hStreamRecvThread, 0UL);
			Trace(TEXT("�X�g���[����M�X���b�h�������I�����܂����B"));
		}
		::CloseHandle(m_hStreamRecvThread);
		m_hStreamRecvThread = NULL;
	}
	m_PauseEvent.Close();
	m_ResumeEvent.Close();

	if (m_pBonDriver) {
		// �`���[�i�����
		Trace(TEXT("�`���[�i����Ă��܂�..."));
		m_pBonDriver->CloseTuner();

		// �h���C�o�C���X�^���X�J��
		Trace(TEXT("�h���C�o��������Ă��܂�..."));
		m_pBonDriver->Release();
		m_pBonDriver = NULL;
		m_pBonDriver2 = NULL;
		Trace(TEXT("�`���[�i����܂����B"));
	}

	ClearError();

	return true;
}

const bool CBonSrcDecoder::IsOpen() const
{
	return m_pBonDriver!=NULL;
}

const bool CBonSrcDecoder::Play(void)
{
	if (m_pBonDriver == NULL) {
		// �`���[�i���J����Ă��Ȃ�
		SetError(ERR_NOTOPEN,NULL);
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

	if (!PauseStreamRecieve()) {
		SetError(ERR_TIMEOUT,TEXT("�X�g���[����M�X���b�h���������܂���B"));
		return false;
	}

	// �������̃X�g���[����j������
	m_pBonDriver->PurgeTsStream();

	// ���ʃf�R�[�_�����Z�b�g����
	ResetDownstreamDecoder();

	// �X�g���[�����Đ���Ԃɂ���
	m_bIsPlaying = true;

	ResumeStreamRecieve();

	ClearError();

	return true;
}

const bool CBonSrcDecoder::Stop(void)
{
	if (!m_bIsPlaying) {
		// �X�g���[���͍Đ����łȂ�
		/*
		SetError(ERR_NOTPLAYING,NULL);
		return false;
		*/
		return true;
	}

	if (!PauseStreamRecieve()) {
		SetError(ERR_TIMEOUT,TEXT("�X�g���[����M�X���b�h���������܂���B"));
		return false;
	}

	m_bIsPlaying = false;

	ResumeStreamRecieve();

	ClearError();

	return true;
}

const bool CBonSrcDecoder::SetChannel(const BYTE byChannel)
{
	if (m_pBonDriver == NULL) {
		// �`���[�i���J����Ă��Ȃ�
		SetError(ERR_NOTOPEN,NULL);
		return false;
	}

	if (!PauseStreamRecieve()) {
		SetError(ERR_TIMEOUT,TEXT("�X�g���[����M�X���b�h���������܂���B"));
		return false;
	}

	// �������̃X�g���[����j������
	m_pBonDriver->PurgeTsStream();

	// �`�����l����ύX����
	if (!m_pBonDriver->SetChannel(byChannel)) {
		ResumeStreamRecieve();
		SetError(ERR_TUNER,TEXT("�`�����l���̕ύX���ł��܂���B"),
						   TEXT("IBonDriver::SetChannel()�̌Ăяo���ŃG���[���Ԃ���܂����B"));
		return false;
	}

	// ���ʃf�R�[�_�����Z�b�g����
	ResetDownstreamDecoder();

	ResumeStreamRecieve();

	ClearError();

	return true;
}

const bool CBonSrcDecoder::SetChannel(const DWORD dwSpace, const DWORD dwChannel)
{
	if (m_pBonDriver2 == NULL) {
		// �`���[�i���J����Ă��Ȃ�
		SetError(ERR_NOTOPEN,NULL);
		return false;
	}

	if (!PauseStreamRecieve()) {
		SetError(ERR_TIMEOUT,TEXT("�X�g���[����M�X���b�h���������܂���B"));
		return false;
	}

	// �������̃X�g���[����j������
	m_pBonDriver2->PurgeTsStream();

	// �`�����l����ύX����
	if (!m_pBonDriver2->SetChannel(dwSpace, dwChannel)) {
		ResumeStreamRecieve();
		SetError(ERR_TUNER,TEXT("�`�����l���̕ύX���ł��܂���B"),
						   TEXT("IBonDriver2::SetChannel()�̌Ăяo���ŃG���[���Ԃ���܂����B"));
		return false;
	}

	// ���ʃf�R�[�_�����Z�b�g����
	ResetDownstreamDecoder();

	ResumeStreamRecieve();

	ClearError();

	return true;
}

const float CBonSrcDecoder::GetSignalLevel(void)
{
	if (m_pBonDriver == NULL) {
		// �`���[�i���J����Ă��Ȃ�
		SetError(ERR_NOTOPEN,NULL);
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
		//m_dwLastError = ERR_NOTOPEN;
		return NULL;
	}
	return m_pBonDriver2->EnumTuningSpace(dwSpace);
}

LPCTSTR CBonSrcDecoder::GetChannelName(const DWORD dwSpace, const DWORD dwChannel) const
{
	// �`�����l������Ԃ�
	if (m_pBonDriver2 == NULL) {
		//m_dwLastError = ERR_NOTOPEN;
		return NULL;
	}
	return m_pBonDriver2->EnumChannelName(dwSpace, dwChannel);
}

const bool CBonSrcDecoder::PurgeStream(void)
{
	if (m_pBonDriver == NULL) {
		// �`���[�i���J����Ă��Ȃ�
		SetError(ERR_NOTOPEN,NULL);
		return false;
	}

	if (!PauseStreamRecieve()) {
		SetError(ERR_TIMEOUT,TEXT("�X�g���[����M�X���b�h���������܂���B"));
		return false;
	}

	// �������̃X�g���[����j������
	m_pBonDriver->PurgeTsStream();

	ResumeStreamRecieve();

	ClearError();

	return true;
}


void CBonSrcDecoder::OnTsStream(BYTE *pStreamData, DWORD dwStreamSize)
{
	if (m_bIsPlaying) {
		// �ŏ�ʃf�R�[�_�ɓ��͂���
		m_TsStream.SetData(pStreamData, dwStreamSize);
		OutputMedia(&m_TsStream);
	}
}


DWORD WINAPI CBonSrcDecoder::StreamRecvThread(LPVOID pParam)
{
	CBonSrcDecoder *pThis = static_cast<CBonSrcDecoder *>(pParam);

	::CoInitialize(NULL);

	// �`���[�i����TS�f�[�^�����o���X���b�h
	while (true) {
		DWORD BitRateTime=::GetTickCount();
		DWORD TotalSize=0;
		pThis->m_BitRate=0;

		while (!pThis->m_PauseEvent.IsSignaled()) {
			// �����ȗ����̂��߃|�[�����O�������̗p����
			DWORD dwStreamRemain = 0UL;

			do {
				BYTE *pStreamData = NULL;
				DWORD dwStreamSize = 0UL;

				if (pThis->m_pBonDriver->GetTsStream(&pStreamData,&dwStreamSize,&dwStreamRemain)
						&& pStreamData && dwStreamSize) {
#if 1
					pThis->OnTsStream(pStreamData, dwStreamSize);
#else
					// ��x�ɑ���T�C�Y�������������CPU�g�p���̕ϓ������Ȃ��Ȃ�
					DWORD Remain,Size;

					for (Remain=dwStreamSize;Remain>0;Remain-=Size) {
						Size=min(Remain,188*64);
						pThis->OnTsStream(pStreamData+(dwStreamSize-Remain),Size);
					}
#endif
					TotalSize+=dwStreamSize;
				}

				// �r�b�g���[�g�v�Z
				DWORD Now=::GetTickCount();
				if (Now>=BitRateTime) {
					if (Now-BitRateTime>=1000) {
						pThis->m_BitRate=(DWORD)(((ULONGLONG)TotalSize*8*1000)/(ULONGLONG)(Now-BitRateTime));
						BitRateTime=Now;
						TotalSize=0;
					}
				} else {
					BitRateTime=Now;
					TotalSize=0;
				}
				pThis->m_StreamRemain=dwStreamRemain;
				if (pThis->m_PauseEvent.IsSignaled())
					goto Break;
			} while (dwStreamRemain>0);

			// �E�F�C�g(24Mbps�Ƃ��Ď��̃f�[�^�����܂Ŗ�15ms������)
			::Sleep(5UL);
			//pThis->m_pBonDriver->WaitTsStream(20);
		}
	Break:
		if (pThis->m_bKillSignal)
			break;
		pThis->m_PauseEvent.Reset();
		pThis->m_ResumeEvent.Set();
		pThis->m_PauseEvent.Wait(5000);
		pThis->m_PauseEvent.Reset();
		//pThis->m_ResumeEvent.Set();
	}

	pThis->m_BitRate=0;

	::CoUninitialize();

	TRACE(TEXT("CBonSrcDecoder::StreamRecvThread() return\n"));

	return 0UL;
}


/*
	�X�g���[���̎�M���ꎞ��~������
	Purge�����X�g���[����ǂ݂ɍs���ė����邱�Ƃ�����̂ŁA���̑΍�
*/
bool CBonSrcDecoder::PauseStreamRecieve(DWORD TimeOut)
{
	m_ResumeEvent.Reset();
	m_PauseEvent.Set();
	if (m_ResumeEvent.Wait(TimeOut) == WAIT_TIMEOUT) {
		m_PauseEvent.Reset();
		return false;
	}
	return true;
}


bool CBonSrcDecoder::ResumeStreamRecieve(DWORD TimeOut)
{
	m_PauseEvent.Set();
	/*
	if (m_ResumeEvent.Wait(TimeOut) == WAIT_TIMEOUT)
		return false;
	*/
	return true;
}


int CBonSrcDecoder::NumSpaces() const
{
	int i;

	for (i=0; GetSpaceName(i)!=NULL; i++);
	return i;
}


LPCTSTR CBonSrcDecoder::GetTunerName() const
{
	if (m_pBonDriver2 == NULL) {
		//m_dwLastError = ERR_NOTOPEN;
		return NULL;
	}
	return m_pBonDriver2->GetTunerName();
}


int CBonSrcDecoder::GetCurSpace() const
{
	if (m_pBonDriver2 == NULL) {
		//m_dwLastError = ERR_NOTOPEN;
		return -1;
	}
	return m_pBonDriver2->GetCurSpace();
}


int CBonSrcDecoder::GetCurChannel() const
{
	if (m_pBonDriver2 == NULL) {
		//m_dwLastError = ERR_NOTOPEN;
		return -1;
	}
	return m_pBonDriver2->GetCurChannel();
}


DWORD CBonSrcDecoder::GetBitRate() const
{
	return m_BitRate;
}


DWORD CBonSrcDecoder::GetStreamRemain() const
{
	return m_StreamRemain;
}
