// TsDescrambler.cpp: CTsDescrambler �N���X�̃C���v�������e�[�V����
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TsDescrambler.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CTsDescrambler �\�z/����
//////////////////////////////////////////////////////////////////////

CTsDescrambler::CTsDescrambler(IEventHandler *pEventHandler)
	: CMediaDecoder(pEventHandler, 1UL, 1UL)
	, m_dwInputPacketCount(0UL)
	, m_dwScramblePacketCount(0UL)
	, m_DescrambleServiceID(0)
{
	// PAT�e�[�u��PID�}�b�v�ǉ�
	m_PidMapManager.MapTarget(0x0000U, new CPatTable, OnPatUpdated, this);
}

CTsDescrambler::~CTsDescrambler()
{
	CloseBcasCard();
}

void CTsDescrambler::Reset(void)
{
	m_Queue.Clear();

	// ������Ԃ�����������
	m_PidMapManager.UnmapAllTarget();

	// PAT�e�[�u��PID�}�b�v�ǉ�
	m_PidMapManager.MapTarget(0x0000U, new CPatTable, OnPatUpdated, this);

	// ���v�f�[�^������
	m_dwInputPacketCount = 0UL;
	m_dwScramblePacketCount = 0UL;

	// �X�N�����u�������^�[�Q�b�g������
	m_DescramblePIDList.clear();
	m_DescrambleServiceID=0;

	// �����f�R�[�_������������
	CMediaDecoder::Reset();
}

const bool CTsDescrambler::InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex)
{
	/*
	if(dwInputIndex >= GetInputNum())return false;

	CTsPacket *pTsPacket = dynamic_cast<CTsPacket *>(pMediaData);

	// ���̓��f�B�A�f�[�^�͌݊������Ȃ�
	if(!pTsPacket)return false;
	*/

	CTsPacket *pTsPacket = static_cast<CTsPacket *>(pMediaData);

	// ���̓p�P�b�g���J�E���g
	//if(m_dwInputPacketCount < 0xFFFFFFFFUL)m_dwInputPacketCount++;
	m_dwInputPacketCount++;

	if (!pTsPacket->IsScrambled() || IsTargetPID(pTsPacket->GetPID())) {
		// PID���[�e�B���O
		m_PidMapManager.StorePacket(pTsPacket);

		/*
		if (pTsPacket->IsScrambled()) {
			// �����R��p�P�b�g���J�E���g
			//if(m_dwScramblePacketCount < 0xFFFFFFFFUL)m_dwScramblePacketCount++;
			m_dwScramblePacketCount++;
		}
		*/
	}

	// �p�P�b�g�������f�R�[�_�Ƀf�[�^��n��
	OutputMedia(pMediaData);

	return true;
}

const bool CTsDescrambler::OpenBcasCard(CCardReader::ReaderType ReaderType,DWORD *pErrorCode)
{
	CloseBcasCard();

	// �J�[�h���[�_����B-CAS�J�[�h���������ĊJ��
	const bool bReturn = m_BcasCard.OpenCard(ReaderType);

	// �G���[�R�[�h�Z�b�g
	if(pErrorCode)*pErrorCode = m_BcasCard.GetLastError();

	if (bReturn)
		m_Queue.BeginBcasThread();

	return bReturn;
}

void CTsDescrambler::CloseBcasCard(void)
{
	m_Queue.EndBcasThread();
	// B-CAS�J�[�h�����
	m_BcasCard.CloseCard();
}

const bool CTsDescrambler::IsBcasCardOpen() const
{
	return m_BcasCard.IsCardOpen();
}

const bool CTsDescrambler::GetBcasCardID(BYTE *pCardID)
{
	// �J�[�hID�擾
	const BYTE *pBuff = m_BcasCard.GetBcasCardID();
	
	// �o�b�t�@�ɃR�s�[
	if(pCardID && pBuff)::CopyMemory(pCardID, pBuff, 6UL);
	
	return (pBuff)? true : false;
}

LPCTSTR CTsDescrambler::GetCardReaderName() const
{
	return m_BcasCard.GetCardReaderName();
}

const DWORD CTsDescrambler::GetInputPacketCount(void) const
{
	// ���̓p�P�b�g����Ԃ�
	return m_dwInputPacketCount;
}

const DWORD CTsDescrambler::GetScramblePacketCount(void) const
{
	// �����R��p�P�b�g����Ԃ�
	return m_dwScramblePacketCount;
}

bool CTsDescrambler::SetTargetPID(const WORD *pPIDList,int NumPIDs)
{
	CBlockLock Lock(&m_DescrambleListLock);

	TRACE(TEXT("CTsDescrambler::SetTargetPID()\n"));
	m_DescramblePIDList.clear();
	if (pPIDList!=NULL) {
		for (int i=0;i<NumPIDs;i++) {
			TRACE(TEXT("Descramble target PID %d = %04x\n"),i,pPIDList[i]);
			m_DescramblePIDList.push_back(pPIDList[i]);
		}
	}
	m_DescrambleServiceID=0;
	return true;
}

bool CTsDescrambler::IsTargetPID(WORD PID)
{
	CBlockLock Lock(&m_DescrambleListLock);

	if (m_DescramblePIDList.size()==0)
		return true;
	for (size_t i=0;i<m_DescramblePIDList.size();i++) {
		if (m_DescramblePIDList[i]==PID)
			return true;
	}
	return false;
}

bool CTsDescrambler::SetTargetServiceID(WORD ServiceID)
{
	CBlockLock Lock(&m_DescrambleListLock);

	m_DescramblePIDList.clear();
	m_DescrambleServiceID=ServiceID;
	return true;
}

DWORD CTsDescrambler::IncrementScramblePacketCount()
{
	return InterlockedIncrement((LONG*)&m_dwScramblePacketCount);
}

void CALLBACK CTsDescrambler::OnPatUpdated(const WORD wPID, CTsPidMapTarget *pMapTarget, CTsPidMapManager *pMapManager, const PVOID pParam)
{
	// PAT���X�V���ꂽ
	CTsDescrambler *pThis = static_cast<CTsDescrambler *>(pParam);
	CPatTable *pPatTable = dynamic_cast<CPatTable *>(pMapTarget);

#ifdef _DEBUG
	if(!pPatTable)::DebugBreak();
#endif

	// PMT�e�[�u��PID�}�b�v�ǉ�
	for(WORD wIndex = 0U ; wIndex < pPatTable->GetProgramNum() ; wIndex++){
		if (pThis->m_DescrambleServiceID==0
			|| pPatTable->GetProgramID(wIndex)==pThis->m_DescrambleServiceID)
			pMapManager->MapTarget(pPatTable->GetPmtPID(wIndex), new CPmtTable, OnPmtUpdated, pParam);
	}
}

void CALLBACK CTsDescrambler::OnPmtUpdated(const WORD wPID, CTsPidMapTarget *pMapTarget, CTsPidMapManager *pMapManager, const PVOID pParam)
{
	// PMT���X�V���ꂽ
	CTsDescrambler *pThis = static_cast<CTsDescrambler *>(pParam);
	CPmtTable *pPmtTable = dynamic_cast<CPmtTable *>(pMapTarget);

#ifdef _DEBUG
	if(!pPmtTable)::DebugBreak();
#endif

	// ECM��PID�}�b�v�ǉ�
	const WORD wEcmPID = pPmtTable->GetEcmPID();
	if(wEcmPID >= 0x1FFFU)return;

	// ������ECM�����^�[�Q�b�g���m�F
	CEcmProcessor *pEcmProcessor = dynamic_cast<CEcmProcessor *>(pMapManager->GetMapTarget(wEcmPID));

	if (!pEcmProcessor) {
		// ECM���������N���X�V�K�}�b�v
		pEcmProcessor = new CEcmProcessor(pThis, &pThis->m_BcasCard, &pThis->m_Queue);
		pMapManager->MapTarget(wEcmPID, pEcmProcessor);
	}

	// ES��PID�}�b�v�ǉ�
	for (WORD wIndex = 0U ; wIndex < pPmtTable->GetEsInfoNum() ; wIndex++) {
		pEcmProcessor->AddEsPID(pPmtTable->GetEsPID(wIndex));
		pMapManager->MapTarget(pPmtTable->GetEsPID(wIndex), new CEsProcessor(pEcmProcessor), NULL, pParam);
	}
}


//////////////////////////////////////////////////////////////////////
// CEcmProcessor �\�z/����
//////////////////////////////////////////////////////////////////////

CEcmProcessor::CEcmProcessor(CTsDescrambler *pDescrambler, CBcasCard *pBcasCard, CBcasAccessQueue *pQueue)
	: CDynamicReferenceable()
	, CPsiSingleTable(true)
	, m_pDescrambler(pDescrambler)
	, m_pBcasCard(pBcasCard)
	, m_pQueue(pQueue)
	, m_bInQueue(false)
	, m_bSetScrambleKey(false)
	, m_bLastEcmSucceed(true)
{
	// MULTI2�f�R�[�_�ɃV�X�e���L�[�Ə���CBC���Z�b�g
	m_Multi2Decoder.Initialize(m_pBcasCard->GetSystemKey(), m_pBcasCard->GetInitialCbc());
}

void CEcmProcessor::OnPidMapped(const WORD wPID, const PVOID pParam)
{
	// �Q�ƃJ�E���g�ǉ�
	AddRef();
}

void CEcmProcessor::OnPidUnmapped(const WORD wPID)
{
	// �Q�ƃJ�E���g�J��
	ReleaseRef();
}

const bool CEcmProcessor::DescramblePacket(CTsPacket *pTsPacket)
{
	if (!m_bInQueue) {	// �܂�ECM�����Ă��Ȃ�
		m_pDescrambler->IncrementScramblePacketCount();
		return false;
	}
	for (int i=0;!m_bSetScrambleKey;i++) {
		if (i==500)	// �f�b�h���b�N���
			return false;
		Sleep(1);
	}
	// �X�N�����u������
	m_Multi2Lock.Lock();
	bool bOK=m_Multi2Decoder.Decode(pTsPacket->GetPayloadData(),
									(DWORD)pTsPacket->GetPayloadSize(),
									pTsPacket->m_Header.byTransportScramblingCtrl);
	m_Multi2Lock.Unlock();
	if (bOK) {
		// �g�����X�|�[�g�X�N�����u������Đݒ�
		pTsPacket->SetAt(3UL, pTsPacket->GetAt(3UL) & 0x3FU);
		pTsPacket->m_Header.byTransportScramblingCtrl = 0U;
	} else {
		m_pDescrambler->IncrementScramblePacketCount();
	}
	return bOK;
}

const bool CEcmProcessor::OnTableUpdate(const CPsiSection *pCurSection, const CPsiSection *pOldSection)
{
#if 0
	return SetScrambleKey(pCurSection->GetPayloadData(), pCurSection->GetPayloadSize());
#else
	// B-CAS�A�N�Z�X�L���[�ɒǉ�
	if (m_pQueue->Enqueue(this, pCurSection->GetPayloadData(), pCurSection->GetPayloadSize()))
		m_bInQueue=true;
#endif
	return true;
}

const bool CEcmProcessor::SetScrambleKey(const BYTE *pEcmData, DWORD EcmSize)
{
	bool bHasTargetPID=false;
	for (size_t i=0;i<m_EsPIDList.size();i++) {
		if (m_pDescrambler->IsTargetPID(m_EsPIDList[i])) {
			bHasTargetPID=true;
			break;
		}
	}
	if (!bHasTargetPID)
		return false;

	// ECM��B-CAS�J�[�h�ɓn���ăL�[�擾
	const BYTE *pKsData = m_pBcasCard->GetKsFromEcm(pEcmData, EcmSize);

	// ECM�������s���͈�x����B-CAS�J�[�h���ď���������
	if (!pKsData && m_bLastEcmSucceed && (m_pBcasCard->GetLastError() != BCEC_ECMREFUSED)){
		if (m_pBcasCard->ReOpenCard()) {
			TRACE(TEXT("CTsDescrambler::CEcmProcessor::SetScrambleKey() Re open card.\n"));
			pKsData = m_pBcasCard->GetKsFromEcm(pEcmData, EcmSize);
		}
	}

	// �X�N�����u���L�[�X�V
	m_Multi2Lock.Lock();
	m_Multi2Decoder.SetScrambleKey(pKsData);
	m_Multi2Lock.Unlock();

	// ECM����������ԍX�V
	m_bLastEcmSucceed = pKsData!=NULL;

	m_bSetScrambleKey=true;

	return true;
}

const bool CEcmProcessor::AddEsPID(WORD PID)
{
	m_EsPIDList.push_back(PID);
	return true;
}


//////////////////////////////////////////////////////////////////////
// CTsDescrambler::CEsProcessor �\�z/����
//////////////////////////////////////////////////////////////////////

CTsDescrambler::CEsProcessor::CEsProcessor(CEcmProcessor *pEcmProcessor)
	: CTsPidMapTarget()
	, m_pEcmProcessor(pEcmProcessor)
{
	// �Q�ƃJ�E���g�ǉ�
	m_pEcmProcessor->AddRef();
}

CTsDescrambler::CEsProcessor::~CEsProcessor()
{
	// �Q�ƃJ�E���g�폜
	m_pEcmProcessor->ReleaseRef();
}

const bool CTsDescrambler::CEsProcessor::StorePacket(const CTsPacket *pPacket)
{
	// �X�N�����u������
	m_pEcmProcessor->DescramblePacket(const_cast<CTsPacket *>(pPacket));

	return false;
}

void CTsDescrambler::CEsProcessor::OnPidUnmapped(const WORD wPID)
{
	// �C���X�^���X�J��
	delete this;
}




CBcasAccess::CBcasAccess(CEcmProcessor *pEcmProcessor, const BYTE *pData, DWORD Size)
{
	m_pEcmProcessor = pEcmProcessor;
	m_pEcmProcessor->AddRef();
	::CopyMemory(m_EcmData, pData, Size);
	m_EcmSize = Size;
}


CBcasAccess::CBcasAccess(const CBcasAccess &BcasAccess)
{
	m_pEcmProcessor = BcasAccess.m_pEcmProcessor;
	m_pEcmProcessor->AddRef();
	::CopyMemory(m_EcmData, BcasAccess.m_EcmData, BcasAccess.m_EcmSize);
	m_EcmSize = BcasAccess.m_EcmSize;
}


CBcasAccess::~CBcasAccess()
{
	m_pEcmProcessor->ReleaseRef();
}


CBcasAccess &CBcasAccess::operator=(const CBcasAccess &BcasAccess)
{
	m_pEcmProcessor->ReleaseRef();
	m_pEcmProcessor = BcasAccess.m_pEcmProcessor;
	m_pEcmProcessor->AddRef();
	::CopyMemory(m_EcmData, BcasAccess.m_EcmData, BcasAccess.m_EcmSize);
	m_EcmSize = BcasAccess.m_EcmSize;
	return *this;
}


bool CBcasAccess::SetScrambleKey()
{
	return m_pEcmProcessor->SetScrambleKey(m_EcmData, m_EcmSize);
}




CBcasAccessQueue::CBcasAccessQueue()
	: m_hThread(NULL)
	, m_hEvent(NULL)
	, m_bKillEvent(false)
{
}


CBcasAccessQueue::~CBcasAccessQueue()
{
	EndBcasThread();
	//Clear();
}


void CBcasAccessQueue::Clear()
{
	CBlockLock Lock(&m_Lock);

	m_Queue.clear();
}


bool CBcasAccessQueue::Enqueue(CEcmProcessor *pEcmProcessor, const BYTE *pData, DWORD Size)
{
	if (!m_hThread || Size>256)
		return false;

	CBlockLock Lock(&m_Lock);

	CBcasAccess BcasAccess(pEcmProcessor, pData, Size);
	m_Queue.push_back(BcasAccess);
	::SetEvent(m_hEvent);
	return true;
}


bool CBcasAccessQueue::BeginBcasThread()
{
	if (m_hThread)
		return false;
	DWORD ThreadID;
	m_hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	m_bKillEvent = false;
	m_hThread = ::CreateThread(NULL, 0, BcasAccessThread, this, 0, &ThreadID);
	if (m_hThread == NULL) {
		::CloseHandle(m_hEvent);
		m_hEvent = NULL;
		return false;
	}
	return true;
}


bool CBcasAccessQueue::EndBcasThread()
{
	if (m_hThread) {
		m_bKillEvent = true;
		::SetEvent(m_hEvent);
		if (::WaitForSingleObject(m_hThread, 1000) == WAIT_TIMEOUT) {
			::TerminateThread(m_hThread, 1);
		}
		::CloseHandle(m_hThread);
		m_hThread = NULL;
		::CloseHandle(m_hEvent);
		m_hEvent = NULL;
	}
	return true;
}


DWORD CALLBACK CBcasAccessQueue::BcasAccessThread(LPVOID lpParameter)
{
	CBcasAccessQueue *pThis=static_cast<CBcasAccessQueue*>(lpParameter);

	while (true) {
		::WaitForSingleObject(pThis->m_hEvent, INFINITE);
		if (pThis->m_bKillEvent)
			break;
		while (true) {
			pThis->m_Lock.Lock();
			if (pThis->m_Queue.empty()) {
				pThis->m_Lock.Unlock();
				break;
			}
			CBcasAccess BcasAccess = pThis->m_Queue.front();
			pThis->m_Queue.pop_front();
			pThis->m_Lock.Unlock();
			BcasAccess.SetScrambleKey();
		}
	}
	return 0;
}
