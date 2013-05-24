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


#pragma comment(lib, "BonDriver.lib")


//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////

CBonSrcDecoder::CBonSrcDecoder(IEventHandler *pEventHandler)
	: CMediaDecoder(pEventHandler)
	, m_pBonDriver(NULL)
	, m_hStreamRecvThread(NULL)
	, m_TsStream(0x10000UL)
	, m_bKillSignal(true)
	, m_bIsPlaying(false)
	, m_dwLastError(BSDEC_NOERROR)
{

}

CBonSrcDecoder::~CBonSrcDecoder()
{
	CloseTuner();
}

void CBonSrcDecoder::Reset(void)
{
	CBlockLock Lock(&m_CriticalLock);

	// �������̃X�g���[����j������
	if(m_bIsPlaying){
		// �������̃X�g���[����j������
		m_pBonDriver->PurgeTsStream();
		}

	// ���ʃf�R�[�_�����Z�b�g����
	CMediaDecoder::Reset();
}

const DWORD CBonSrcDecoder::GetInputNum(void) const
{
	return 0UL;
}

const DWORD CBonSrcDecoder::GetOutputNum(void) const
{
	return 1UL;
}

const bool CBonSrcDecoder::InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex)
{
	// �\�[�X�f�R�[�_�̂��ߓ��͂͏������Ȃ�
	return false;
}

const bool CBonSrcDecoder::OpenTuner(void)
{
	CBlockLock Lock(&m_CriticalLock);
	
	// �I�[�v���`�F�b�N
	if(m_pBonDriver){
		m_dwLastError = BSDEC_ALREADYOPEN;
		return false;
		}

	// �h���C�o�C���X�^���X����
	if(!(m_pBonDriver = ::CreateBonDriver())){
		m_dwLastError = BSDEC_TUNERERROR;
		return false;
		}

	// �`���[�i���J��
	if(!m_pBonDriver->OpenTuner()){
		CloseTuner();
		m_dwLastError = BSDEC_TUNERERROR;
		return false;
		}

	// �����`�����l�����Z�b�g����
	if(!m_pBonDriver->SetChannel(13U)){
		CloseTuner();
		m_dwLastError = BSDEC_TUNERERROR;
		return false;
		}

	// �X�g���[����M�X���b�h�N��
	DWORD dwThreadID = 0UL;
	m_bKillSignal = false;
	m_bIsPlaying = false;

	if(!(m_hStreamRecvThread = ::CreateThread(NULL, 0UL, CBonSrcDecoder::StreamRecvThread, (LPVOID)this, 0UL, &dwThreadID))){
		CloseTuner();
		m_dwLastError = BSDEC_INTERNALERROR;
		return false;
		}

	m_dwLastError = BSDEC_NOERROR;

	return true;
}

const bool CBonSrcDecoder::CloseTuner(void)
{
	CBlockLock Lock(&m_CriticalLock);

	// �X�g���[����~
	m_bIsPlaying = false;
	m_bKillSignal = true;

	if(m_hStreamRecvThread){
		// �X�g���[����M�X���b�h��~
		if(::WaitForSingleObject(m_hStreamRecvThread, 1000UL) != WAIT_OBJECT_0){
			// �X���b�h�����I��
			::TerminateThread(m_hStreamRecvThread, 0UL);
			}

		::CloseHandle(m_hStreamRecvThread);
		m_hStreamRecvThread = NULL;
		}

	if(m_pBonDriver){
		// �`���[�i�����
		m_pBonDriver->CloseTuner();

		// �h���C�o�C���X�^���X�J��
		m_pBonDriver->Release();
		m_pBonDriver = NULL;		
		}

	m_dwLastError = BSDEC_NOERROR;

	return true;
}

const bool CBonSrcDecoder::Play(void)
{
	CBlockLock Lock(&m_CriticalLock);
	
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

	// �������̃X�g���[����j������
	m_pBonDriver->PurgeTsStream();

	// ���ʃf�R�[�_�����Z�b�g����
	CMediaDecoder::Reset();

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
	CBlockLock Lock(&m_CriticalLock);

	if(!m_pBonDriver){
		// �`���[�i���J����Ă��Ȃ�
		m_dwLastError = BSDEC_TUNERNOTOPEN;
		return false;
		}

	// �������̃X�g���[����j������
	m_pBonDriver->PurgeTsStream();

	// �`�����l����ύX����
	if(!m_pBonDriver->SetChannel(byChannel)){
		m_dwLastError = BSDEC_TUNERERROR;
		return false;
		}

	// ���ʃf�R�[�_�����Z�b�g����
	CMediaDecoder::Reset();

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

const DWORD CBonSrcDecoder::GetLastError(void) const
{
	// �Ō�ɔ��������G���[��Ԃ�
	return m_dwLastError;
}

const bool CBonSrcDecoder::PurgeStream(void)
{
	CBlockLock Lock(&m_CriticalLock);

	if(!m_pBonDriver){
		// �`���[�i���J����Ă��Ȃ�
		m_dwLastError = BSDEC_TUNERNOTOPEN;
		return false;
		}

	// �������̃X�g���[����j������
	m_pBonDriver->PurgeTsStream();

	// ���ʃf�R�[�_�����Z�b�g����
	CMediaDecoder::Reset();
	
	m_dwLastError = BSDEC_NOERROR;

	return true;
}

void CBonSrcDecoder::OnTsStream(BYTE *pStreamData, DWORD dwStreamSize)
{
	CBlockLock Lock(&m_CriticalLock);

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

	// �`���[�i����TS�f�[�^�����o���X���b�h
	while(!pThis->m_bKillSignal){
		
		// �����ȗ����̂��߃|�[�����O�������̗p����
		do{
			if(pThis->m_pBonDriver->GetTsStream(&pStreamData, &dwStreamSize, &dwStreamRemain)){
				if(pStreamData && dwStreamSize){
					pThis->OnTsStream(pStreamData, dwStreamSize);
					}
				}
			}
		while(dwStreamRemain);

		// �E�F�C�g(24Mbps�Ƃ��Ď��̃f�[�^�����܂Ŗ�15ms������)
		::Sleep(1UL);
		}

	return 0UL;
}

