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
	, m_bPauseSignal(false)
	, m_hResumeEvent(NULL)
	, m_bKillSignal(false)
	, m_TsStream(0x10000UL)
	, m_bIsPlaying(false)
	, m_dwLastError(ERR_NOERROR)
	, m_BitRate(0)
	, m_StreamRemain(0)
	/*
	, m_RequestSpace(-1)
	, m_RequestChannel(-1)
	*/
{
}

CBonSrcDecoder::~CBonSrcDecoder()
{
	CloseTuner();
}

void CBonSrcDecoder::Reset(void)
{
	CBlockLock Lock(&m_DecoderLock);

	PauseStreamRecieve();

	// �������̃X�g���[����j������
	m_pBonDriver->PurgeTsStream();

	// ���ʃf�R�[�_�����Z�b�g����
	ResetDownstreamDecoder();

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
		m_dwLastError = ERR_ALREADYOPEN;
		return false;
	}

	// �h���C�o�|�C���^�̎擾
	PFCREATEBONDRIVER *pf=(PFCREATEBONDRIVER*)GetProcAddress(hBonDrvDll,"CreateBonDriver");
	if (!pf || (m_pBonDriver = pf()) == NULL) {
		m_dwLastError = ERR_DRIVER;
		return false;
	}

	// �`���[�i���J��
	if (!m_pBonDriver->OpenTuner()) {
		m_dwLastError = ERR_TUNEROPEN;
		goto OnError;
	}

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
	m_bPauseSignal = false;
	m_hResumeEvent = ::CreateEvent(NULL,FALSE,FALSE,NULL);
	m_bKillSignal = false;
	m_bIsPlaying = false;
	m_hStreamRecvThread = ::CreateThread(NULL, 0UL, CBonSrcDecoder::StreamRecvThread, this, 0UL, NULL);
	if (!m_hStreamRecvThread) {
		m_dwLastError = ERR_INTERNAL;
		goto OnError;
	}

	m_dwLastError = ERR_NOERROR;

	ResetBitRate();

	return true;

OnError:
	m_pBonDriver->CloseTuner();
	m_pBonDriver->Release();
	m_pBonDriver = NULL;
	m_pBonDriver2 = NULL;
	return false;
}

const bool CBonSrcDecoder::CloseTuner(void)
{
	// �X�g���[����~
	m_bIsPlaying = false;

	if (m_hStreamRecvThread) {
		// �X�g���[����M�X���b�h��~
		m_bKillSignal=true;
		m_bPauseSignal=true;
		if (::WaitForSingleObject(m_hStreamRecvThread, 1000UL) != WAIT_OBJECT_0) {
			// �X���b�h�����I��
			TRACE(TEXT("Terminate stream recieve thread.\n"));
			::TerminateThread(m_hStreamRecvThread, 0UL);
		}
		::CloseHandle(m_hStreamRecvThread);
		m_hStreamRecvThread = NULL;
	}
	if (m_hResumeEvent) {
		::CloseHandle(m_hResumeEvent);
		m_hResumeEvent=NULL;
	}

	if (m_pBonDriver) {
		// �`���[�i�����
		m_pBonDriver->CloseTuner();

		// �h���C�o�C���X�^���X�J��
		m_pBonDriver->Release();
		m_pBonDriver = NULL;
		m_pBonDriver2 = NULL;
	}

	m_dwLastError = ERR_NOERROR;

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
		m_dwLastError = ERR_NOTOPEN;
		return false;
	}

	if (m_bIsPlaying) {
		// ���ɍĐ���
		m_dwLastError = ERR_ALREADYPLAYING;
		return false;
	}

	if (!PauseStreamRecieve()) {
		m_dwLastError = ERR_TIMEOUT;
		return false;
	}

	// �������̃X�g���[����j������
	m_pBonDriver->PurgeTsStream();

	// ���ʃf�R�[�_�����Z�b�g����
	ResetDownstreamDecoder();

	ResumeStreamRecieve();

	// �X�g���[�����Đ���Ԃɂ���
	m_bIsPlaying = true;
	m_dwLastError = ERR_NOERROR;

	return true;
}

const bool CBonSrcDecoder::Stop(void)
{
	if (!m_bIsPlaying) {
		// �X�g���[���͍Đ����łȂ�
		m_dwLastError = ERR_NOTPLAYING;
		return false;
	}

	if (!PauseStreamRecieve()) {
		m_dwLastError = ERR_TIMEOUT;
		return false;
	}

	m_bIsPlaying = false;

	ResumeStreamRecieve();

	m_dwLastError = ERR_NOERROR;

	return true;
}

const bool CBonSrcDecoder::SetChannel(const BYTE byChannel)
{
	if (m_pBonDriver == NULL) {
		// �`���[�i���J����Ă��Ȃ�
		m_dwLastError = ERR_NOTOPEN;
		return false;
	}

	/*
	m_RequestChannel = byChannel;
	if (!PauseStreamRecieve()) {
		m_RequestChannel = -1;
		m_dwLastError = ERR_TIMEOUT;
		return false;
	}

	if (!m_bSetChannelResult) {
		ResumeStreamRecieve();
		m_dwLastError = ERR_TUNER;
		return false;
	}
	*/

	if (!PauseStreamRecieve()) {
		m_dwLastError = ERR_TIMEOUT;
		return false;
	}

	// �������̃X�g���[����j������
	m_pBonDriver->PurgeTsStream();

	// �`�����l����ύX����
	if (!m_pBonDriver->SetChannel(byChannel)) {
		ResumeStreamRecieve();
		m_dwLastError = ERR_TUNER;
		return false;
	}

	// ���ʃf�R�[�_�����Z�b�g����
	ResetDownstreamDecoder();

	ResumeStreamRecieve();

	m_dwLastError = ERR_NOERROR;

	return true;
}

const bool CBonSrcDecoder::SetChannel(const DWORD dwSpace, const DWORD dwChannel)
{
	if (m_pBonDriver2 == NULL) {
		// �`���[�i���J����Ă��Ȃ�
		m_dwLastError = ERR_NOTOPEN;
		return false;
	}

	/*
	m_RequestSpace = dwSpace;
	m_RequestChannel = dwChannel;
	if (!PauseStreamRecieve()) {
		m_RequestSpace = -1;
		m_RequestChannel = -1;
		m_dwLastError = ERR_TIMEOUT;
		return false;
	}

	if (!m_bSetChannelResult) {
		ResumeStreamRecieve();
		m_dwLastError = ERR_TUNER;
		return false;
	}
	*/

	if (!PauseStreamRecieve()) {
		m_dwLastError = ERR_TIMEOUT;
		return false;
	}

	// �������̃X�g���[����j������
	m_pBonDriver2->PurgeTsStream();

	// �`�����l����ύX����
	if (!m_pBonDriver2->SetChannel(dwSpace, dwChannel)) {
		ResumeStreamRecieve();
		m_dwLastError = ERR_TUNER;
		return false;
	}

	// ���ʃf�R�[�_�����Z�b�g����
	ResetDownstreamDecoder();

	ResumeStreamRecieve();

	m_dwLastError = ERR_NOERROR;

	return true;
}

const float CBonSrcDecoder::GetSignalLevel(void)
{
	if (m_pBonDriver == NULL) {
		// �`���[�i���J����Ă��Ȃ�
		m_dwLastError = ERR_NOTOPEN;
		return 0.0f;
	}

	m_dwLastError = ERR_NOERROR;

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

const DWORD CBonSrcDecoder::GetLastError(void) const
{
	// �Ō�ɔ��������G���[��Ԃ�
	return m_dwLastError;
}

const bool CBonSrcDecoder::PurgeStream(void)
{
	if (m_pBonDriver == NULL) {
		// �`���[�i���J����Ă��Ȃ�
		m_dwLastError = ERR_NOTOPEN;
		return false;
	}

	if (!PauseStreamRecieve()) {
		m_dwLastError = ERR_TIMEOUT;
		return false;
	}

	// �������̃X�g���[����j������
	m_pBonDriver->PurgeTsStream();

	ResumeStreamRecieve();

	m_dwLastError = ERR_NOERROR;

	return true;
}


void CBonSrcDecoder::OnTsStream(BYTE *pStreamData, DWORD dwStreamSize)
{
	//CBlockLock Lock(&m_DecoderLock);

	if (!m_bIsPlaying)
		return;

	// �ŏ�ʃf�R�[�_�ɓ��͂���
	m_TsStream.SetData(pStreamData, dwStreamSize);
	OutputMedia(&m_TsStream);
}


DWORD WINAPI CBonSrcDecoder::StreamRecvThread(LPVOID pParam)
{
	CBonSrcDecoder *pThis = static_cast<CBonSrcDecoder *>(pParam);

	::CoInitialize(NULL);

	BYTE *pStreamData = NULL;
	DWORD dwStreamSize = 0UL;
	DWORD dwStreamRemain = 0UL;
	DWORD TotalSize=0;

	// �`���[�i����TS�f�[�^�����o���X���b�h
	while (true) {
		while (!pThis->m_bPauseSignal) {
			// �����ȗ����̂��߃|�[�����O�������̗p����
			do {
				if (pThis->m_pBonDriver->GetTsStream(&pStreamData,&dwStreamSize,&dwStreamRemain)
						&& pStreamData && dwStreamSize) {
					//pThis->OnTsStream(pStreamData, dwStreamSize);
					DWORD Remain,Size;

					for (Remain=dwStreamSize;Remain>0;Remain-=Size) {
						// ��x�ɑ���T�C�Y�������������CPU�g�p���̕ϓ������Ȃ��Ȃ�
						Size=min(Remain,188*64);
						pThis->OnTsStream(pStreamData+(dwStreamSize-Remain),Size);
					}
				}
				TotalSize+=dwStreamSize;

				// �r�b�g���[�g�v�Z
				DWORD Now=::GetTickCount();
				if (Now>=pThis->m_BitRateTime) {
					if (Now-pThis->m_BitRateTime>=1000) {
						pThis->m_BitRate=(DWORD)(((ULONGLONG)TotalSize*8*1000)/(ULONGLONG)(Now-pThis->m_BitRateTime));
						pThis->m_BitRateTime=Now;
						TotalSize=0;
					}
				} else {
					pThis->m_BitRateTime=Now;
					TotalSize=0;
				}
				pThis->m_StreamRemain=dwStreamRemain;
			} while (dwStreamRemain>0 && !pThis->m_bPauseSignal);

			if (pThis->m_bPauseSignal)
				break;

			// �E�F�C�g(24Mbps�Ƃ��Ď��̃f�[�^�����܂Ŗ�15ms������)
			::Sleep(5UL);
			//pThis->m_pBonDriver->WaitTsStream(20);
		}
		if (pThis->m_bKillSignal)
			break;
		/*
		if (pThis->m_RequestChannel>=0) {
			pThis->m_pBonDriver->PurgeTsStream();
			if (pThis->m_RequestSpace>=0) {
				pThis->m_bSetChannelResult=pThis->m_pBonDriver2->SetChannel(
						pThis->m_RequestSpace,pThis->m_RequestChannel)!=FALSE;
				pThis->m_RequestSpace=-1;
			} else {
				pThis->m_bSetChannelResult=
					pThis->m_pBonDriver->SetChannel(pThis->m_RequestChannel)!=FALSE;
			}
			pThis->m_RequestChannel=-1;
		}
		*/
		::SetEvent(pThis->m_hResumeEvent);
		while (pThis->m_bPauseSignal)
			Sleep(1);
		::SetEvent(pThis->m_hResumeEvent);
	}

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
	::ResetEvent(m_hResumeEvent);
	m_bPauseSignal = true;
	if (::WaitForSingleObject(m_hResumeEvent, TimeOut) == WAIT_TIMEOUT)
		return false;
	ResetBitRate();
	return true;
}


void CBonSrcDecoder::ResumeStreamRecieve()
{
	m_bPauseSignal = false;
	::WaitForSingleObject(m_hResumeEvent, INFINITE);
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


void CBonSrcDecoder::ResetBitRate()
{
	m_BitRateTime = GetTickCount();
	m_BitRate=0;

}


DWORD CBonSrcDecoder::GetStreamRemain() const
{
	return m_StreamRemain;
}
