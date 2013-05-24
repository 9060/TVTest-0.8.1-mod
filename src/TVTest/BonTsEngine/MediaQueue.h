// MediaQueue.h: CMediaQueue �N���X�̃C���^�[�t�F�C�X
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include "MediaDecoder.h"
#include "TsUtilClass.h"
#include <queue>


using std::queue;


/////////////////////////////////////////////////////////////////////////////
// ���f�B�A�L���[(CMediaData���L���[�C���O���Ĕ񓯊��Ɏ��o����i��񋟂���)
/////////////////////////////////////////////////////////////////////////////
// Input	#0	: CMediaData		���̓f�[�^
/////////////////////////////////////////////////////////////////////////////

#define MAXQUEUECOUNT	10UL

class CMediaQueue : public CMediaDecoder  
{
public:
	CMediaQueue(IEventHandler *pEventHandler = NULL);
	virtual ~CMediaQueue();

// IMediaDecoder
	virtual void Reset(void);
	virtual const bool InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex = 0UL);

// CMediaQueue
	void EnableQueuing(const DWORD dwMaxQueueCount = MAXQUEUECOUNT);
	void DisableQueuing(void);

	CMediaData * PeekMediaData(void);
	CMediaData * GetMediaData(void);

	void PurgeMediaData(void);

protected:
	queue<CMediaData *> m_MediaQueue;
	DWORD m_dwMaxQueueCount;

	CCriticalLock m_CriticalLock;
	HANDLE m_hMediaEvent;

	bool m_bIsQueuing;
	bool m_bIsPreCharge;
};
