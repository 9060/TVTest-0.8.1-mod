// MediaQueue.cpp: CMediaQueue �N���X�̃C���v�������e�[�V����
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MediaQueue.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////

CMediaQueue::CMediaQueue(IEventHandler *pEventHandler)
	: CMediaDecoder(pEventHandler)
	, m_dwMaxQueueCount(MAXQUEUECOUNT)
	, m_bIsQueuing(false)
{
	// �C�x���g�쐬
	m_hMediaEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
}

CMediaQueue::~CMediaQueue()
{
	// �C�x���g�폜
	if(m_hMediaEvent)::CloseHandle(m_hMediaEvent);
}

void CMediaQueue::Reset(void)
{
	// �L���[���p�[�W����
	PurgeMediaData();

	CMediaDecoder::Reset();
}

const DWORD CMediaQueue::GetInputNum(void) const
{
	return 1UL;
}

const DWORD CMediaQueue::GetOutputNum(void) const
{
	return 0UL;
}

const bool CMediaQueue::InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex)
{
	if(dwInputIndex > GetInputNum())return false;
	
	CBlockLock Lock(&m_CriticalLock);

	if(m_bIsQueuing){
		// �ő吔�𒴂����A�L���[����ɂ���
		while(m_MediaQueue.size() >= m_dwMaxQueueCount){
			m_MediaQueue.front()->Delete();
			m_MediaQueue.pop();
			}

		// ���f�B�A�f�[�^���L���[�Ƀv�b�V������
		m_MediaQueue.push(new CMediaData(*pMediaData));

		// �C�x���g���Z�b�g����
		::SetEvent(m_hMediaEvent);
		}

	return true;
}

void CMediaQueue::EnableQueuing(const DWORD dwMaxQueueCount)
{
	CBlockLock Lock(&m_CriticalLock);

	// �������̃f�[�^���p�[�W����
	while(!m_MediaQueue.empty()){
		m_MediaQueue.front()->Delete();
		m_MediaQueue.pop();
		}
		
	// �ő吔��ۑ�
	m_dwMaxQueueCount = (dwMaxQueueCount)? dwMaxQueueCount : MAXQUEUECOUNT;

	// �L���[�C���O��L���ɂ���
	m_bIsQueuing = true;

	m_bIsPreCharge = true;

	::ResetEvent(m_hMediaEvent);
}

void CMediaQueue::DisableQueuing(void)
{
	// �C�x���g���Z�b�g����
	::SetEvent(m_hMediaEvent);	
	
	CBlockLock Lock(&m_CriticalLock);

	// �L���[�C���O�𖳌��ɂ���
	m_bIsQueuing = false;

	// �������̃f�[�^���p�[�W����
	while(!m_MediaQueue.empty()){
		m_MediaQueue.front()->Delete();
		m_MediaQueue.pop();
		}
}

CMediaData * CMediaQueue::PeekMediaData(void)
{
	CMediaData *pMediaData = NULL;

	CBlockLock Lock(&m_CriticalLock);

	// �f�[�^������΃L���[������o��
	if(!m_MediaQueue.empty()){
		pMediaData = m_MediaQueue.front();
		m_MediaQueue.pop();
		
		// �L���[����̏ꍇ�̓C�x���g���N���A����
		if(m_MediaQueue.empty()){
			::ResetEvent(m_hMediaEvent);
			}		
		}

	return pMediaData;
}

CMediaData * CMediaQueue::GetMediaData(void)
{
	// �f�[�^�̓�����҂�
	if(!m_bIsQueuing)return NULL;

	if(::WaitForSingleObject(m_hMediaEvent, INFINITE) != WAIT_OBJECT_0){
		// �G���[
		DisableQueuing();
		return NULL;
		}

	return PeekMediaData();
}

void CMediaQueue::PurgeMediaData(void)
{
	CBlockLock Lock(&m_CriticalLock);
	
	// �������̃f�[�^���p�[�W����
	while(!m_MediaQueue.empty()){
		m_MediaQueue.front()->Delete();
		m_MediaQueue.pop();
		}
}
