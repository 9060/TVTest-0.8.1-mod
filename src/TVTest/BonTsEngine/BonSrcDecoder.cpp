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

using namespace std;

CBonSrcDecoder::CBonSrcDecoder(IEventHandler *pEventHandler)
	: CMediaDecoder(pEventHandler, 0UL, 1UL)
	, m_pBonDriver(NULL)
	, m_pBonDriver2(NULL)
	, m_hStreamRecvThread(NULL)
	, m_bPauseSignal(false)
	, m_bResumeSignal(false)
	, m_bKillSignal(false)
	, m_TsStream(0x10000UL)
	, m_bIsPlaying(false)
	, m_dwLastError(BSDEC_NOERROR)
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
	CBlockLock Lock(&m_CriticalLock);

	PauseStreamRecieve();

	// �������̃X�g���[����j������
	m_pBonDriver->PurgeTsStream();

	// ���ʃf�R�[�_�����Z�b�g����
	CMediaDecoder::Reset();

	ResumeStreamRecieve();
}

const bool CBonSrcDecoder::InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex)
{
	// �\�[�X�f�R�[�_�̂��ߓ��͂͏������Ȃ�
	return false;
}

const bool CBonSrcDecoder::OpenTuner(HMODULE hBonDrvDll)
{
	CBlockLock Lock(&m_CriticalLock);
	
	// �I�[�v���`�F�b�N
	if(m_pBonDriver){
		m_dwLastError = BSDEC_ALREADYOPEN;
		return false;
		}

	// �h���C�o�|�C���^�̎擾
	PFCREATEBONDRIVER *pf=(PFCREATEBONDRIVER*)GetProcAddress(hBonDrvDll,"CreateBonDriver");
	if(!pf){
		m_dwLastError = BSDEC_DRIVERERROR;
		return false;
		}

	// �h���C�o�C���X�^���X����
	if(!(m_pBonDriver = pf())){
		m_dwLastError = BSDEC_TUNERERROR;
		return false;
		}

	// IBonDriver2�C���^�t�F�[�X�擾
	try{
		m_pBonDriver2 = dynamic_cast<IBonDriver2 *>(m_pBonDriver);
		}
	catch(std::__non_rtti_object){
		m_pBonDriver2 = NULL;
		}

	// �`���[�i���J��
	if(!m_pBonDriver->OpenTuner()){
		CloseTuner();
		m_dwLastError = BSDEC_TUNERERROR;
		return false;
		}
	
	// �����`�����l�����Z�b�g����
	/*
	if(m_pBonDriver2){
		// IBonDriver2
		if(!m_pBonDriver2->SetChannel(0UL, 0UL)){
			CloseTuner();
			m_dwLastError = BSDEC_TUNERERROR;
			return false;
			}
		}
	else{
		// IBonDriver
		if(!m_pBonDriver->SetChannel(13U)){
			CloseTuner();
			m_dwLastError = BSDEC_TUNERERROR;
			return false;
			}
		}
	*/

	// �X�g���[����M�X���b�h�N��
	DWORD dwThreadID = 0UL;

	m_bPauseSignal=false;
	m_bKillSignal=false;
	m_bIsPlaying = false;

	if(!(m_hStreamRecvThread = ::CreateThread(NULL, 0UL, CBonSrcDecoder::StreamRecvThread, (LPVOID)this, 0UL, &dwThreadID))){
		CloseTuner();
		m_dwLastError = BSDEC_INTERNALERROR;
		return false;
		}

	m_dwLastError = BSDEC_NOERROR;

	ResetBitRate();
	return true;
}

const bool CBonSrcDecoder::CloseTuner(void)
{
	CTryBlockLock Lock(&m_CriticalLock);
	// ���΂炭�҂��Ă�����Ȃ��ꍇ�͋��炭�f�b�h���b�N���Ă���̂ŁA�����I�ɏI��������
	Lock.TryLock(2000);

	// �X�g���[����~
	m_bIsPlaying = false;

	if (m_hStreamRecvThread) {
		// �X�g���[����M�X���b�h��~
		m_bKillSignal=true;
		m_bPauseSignal=true;
		if (::WaitForSingleObject(m_hStreamRecvThread,1000UL)!=WAIT_OBJECT_0) {
			// �X���b�h�����I��
			TRACE(TEXT("Terminate stream recieve thread.\n"));
			::TerminateThread(m_hStreamRecvThread, 0UL);
		}
		::CloseHandle(m_hStreamRecvThread);
		m_hStreamRecvThread = NULL;
	}

	if (m_pBonDriver) {
		// �`���[�i�����
		m_pBonDriver->CloseTuner();

		// �h���C�o�C���X�^���X�J��
		m_pBonDriver->Release();
		m_pBonDriver = NULL;
		m_pBonDriver2 = NULL;
	}

	m_dwLastError = BSDEC_NOERROR;

	return true;
}

const bool CBonSrcDecoder::IsOpen() const
{
	return m_pBonDriver!=NULL;
}

const bool CBonSrcDecoder::Play(void)
{
	CTryBlockLock Lock(&m_CriticalLock);
	if (!Lock.TryLock(1000))
		return false;

	if(!m_pBonDriver){
		// �`���[�i���J����Ă��Ȃ�
		m_dwLastError = BSDEC_TUNERNOTOPEN;
		return false;
		}

	if(m_bIsPlaying){
		// ���ɍĐ���
		m_dwLastError = BSDEC_ALREADYPLAYING;
		return false;
		}

	PauseStreamRecieve();

	// �������̃X�g���[����j������
	m_pBonDriver->PurgeTsStream();

	// ���ʃf�R�[�_�����Z�b�g����
	CMediaDecoder::Reset();

	ResumeStreamRecieve();

	// �X�g���[�����Đ���Ԃɂ���
	m_bIsPlaying = true;
	m_dwLastError = BSDEC_NOERROR;

	return true;
}

const bool CBonSrcDecoder::Stop(void)
{
	if(!m_bIsPlaying){
		// �X�g���[���͍Đ����łȂ�
		m_dwLastError = BSDEC_NOTPLAYING;
		return false;
		}

	m_bIsPlaying = false;
	m_dwLastError = BSDEC_NOERROR;

	return true;
}

const bool CBonSrcDecoder::SetChannel(const BYTE byChannel)
{
	CTryBlockLock Lock(&m_CriticalLock);
	if (!Lock.TryLock(1000))
		return false;

	if(!m_pBonDriver){
		// �`���[�i���J����Ă��Ȃ�
		m_dwLastError = BSDEC_TUNERNOTOPEN;
		return false;
		}

	PauseStreamRecieve();

	// �������̃X�g���[����j������
	m_pBonDriver->PurgeTsStream();

	// �`�����l����ύX����
	if(!m_pBonDriver->SetChannel(byChannel)){
		ResumeStreamRecieve();
		m_dwLastError = BSDEC_TUNERERROR;
		return false;
		}

	// ���ʃf�R�[�_�����Z�b�g����
	CMediaDecoder::Reset();

	ResumeStreamRecieve();

	m_dwLastError = BSDEC_NOERROR;

	return true;
}

const bool CBonSrcDecoder::SetChannel(const DWORD dwSpace, const DWORD dwChannel)
{
	if(!IsBonDriver2())return SetChannel((BYTE)dwChannel + 13U);

	CTryBlockLock Lock(&m_CriticalLock);
	if (!Lock.TryLock(1000))
		return false;

	if (!m_pBonDriver2) {
		// �`���[�i���J����Ă��Ȃ�
		m_dwLastError = BSDEC_TUNERNOTOPEN;
		return false;
	}

	PauseStreamRecieve();

	// �������̃X�g���[����j������
	m_pBonDriver2->PurgeTsStream();
	// �`�����l����ύX����
	if (!m_pBonDriver2->SetChannel(dwSpace, dwChannel)) {
		ResumeStreamRecieve();
		m_dwLastError = BSDEC_TUNERERROR;
		return false;
	}

	// ���ʃf�R�[�_�����Z�b�g����
	CMediaDecoder::Reset();

	ResumeStreamRecieve();

	m_dwLastError = BSDEC_NOERROR;

	return true;
}

const float CBonSrcDecoder::GetSignalLevel(void)
{
	if(!m_pBonDriver){
		// �`���[�i���J����Ă��Ȃ�
		m_dwLastError = BSDEC_TUNERNOTOPEN;
		return 0.0f;
		}

	m_dwLastError = BSDEC_NOERROR;

	// �M�����x����Ԃ�
	return m_pBonDriver->GetSignalLevel();
}

const bool CBonSrcDecoder::IsBonDriver2(void) const
{
	// IBonDriver2�C���^�t�F�[�X�̎g�p�ۂ�Ԃ�
	return (m_pBonDriver2)? true : false;
}

LPCTSTR CBonSrcDecoder::GetSpaceName(const DWORD dwSpace) const
{
	// �`���[�j���O��Ԗ���Ԃ�
	return (IsBonDriver2())? m_pBonDriver2->EnumTuningSpace(dwSpace) : NULL;
}

LPCTSTR CBonSrcDecoder::GetChannelName(const DWORD dwSpace, const DWORD dwChannel) const
{
	// �`�����l������Ԃ�
	return (IsBonDriver2())? m_pBonDriver2->EnumChannelName(dwSpace, dwChannel) : NULL;
}

const DWORD CBonSrcDecoder::GetLastError(void) const
{
	// �Ō�ɔ��������G���[��Ԃ�
	return m_dwLastError;
}

const bool CBonSrcDecoder::PurgeStream(void)
{
	CTryBlockLock Lock(&m_CriticalLock);
	if (!Lock.TryLock(1000))
		return false;

	if(!m_pBonDriver){
		// �`���[�i���J����Ă��Ȃ�
		m_dwLastError = BSDEC_TUNERNOTOPEN;
		return false;
		}

	PauseStreamRecieve();

	// �������̃X�g���[����j������
	m_pBonDriver->PurgeTsStream();

	ResumeStreamRecieve();

	m_dwLastError = BSDEC_NOERROR;

	return true;
}


void CBonSrcDecoder::OnTsStream(BYTE *pStreamData, DWORD dwStreamSize)
{
	//CBlockLock Lock(&m_CriticalLock);

	if(!m_bIsPlaying)return;

	// �ŏ�ʃf�R�[�_�ɓ��͂���
	m_TsStream.SetData(pStreamData, dwStreamSize);
	OutputMedia(&m_TsStream);
}


DWORD WINAPI CBonSrcDecoder::StreamRecvThread(LPVOID pParam)
{
	CBonSrcDecoder *pThis = static_cast<CBonSrcDecoder *>(pParam);

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
						Size=min(Remain,188*64);
						pThis->OnTsStream(pStreamData+(dwStreamSize-Remain),Size);
					}
				}
				TotalSize+=dwStreamSize;

				// �r�b�g���[�g�v�Z
				DWORD Now=::GetTickCount();
				if (Now>=pThis->m_BitRateTime) {
					if (Now-pThis->m_BitRateTime>=1000) {
						pThis->m_BitRate=(TotalSize)*1000/(Now-pThis->m_BitRateTime);
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
			::Sleep(1UL);
			//pThis->m_pBonDriver->WaitTsStream(20);
		}
		if (pThis->m_bKillSignal)
			break;
		pThis->m_bResumeSignal=true;
		while (pThis->m_bPauseSignal)
			Sleep(1);
	}

	TRACE(TEXT("CBonSrcDecoder::StreamRecvThread() return\n"));

	return 0UL;
}


/*
	�X�g���[���̎�M���ꎞ��~������
	Purge�����X�g���[����ǂ݂ɍs���ė����邱�Ƃ�����̂ŁA���̑΍�
*/
void CBonSrcDecoder::PauseStreamRecieve()
{
	m_bResumeSignal=false;
	m_bPauseSignal=true;
	while (!m_bResumeSignal)
		Sleep(1);
	ResetBitRate();
}


void CBonSrcDecoder::ResumeStreamRecieve()
{
	m_bPauseSignal=false;
}


int CBonSrcDecoder::NumSpaces() const
{
	int i;

	for (i=0;GetSpaceName(i)!=NULL;i++);
	return i;
}


LPCTSTR CBonSrcDecoder::GetTunerName() const
{
	if (m_pBonDriver2)
		return m_pBonDriver2->GetTunerName();
	return NULL;
}


DWORD CBonSrcDecoder::GetBitRate() const
{
	return m_BitRate;
}


void CBonSrcDecoder::ResetBitRate()
{
	m_BitRateTime=GetTickCount();
	m_BitRate=0;

}


DWORD CBonSrcDecoder::GetStreamRemain() const
{
	return m_StreamRemain;
}
