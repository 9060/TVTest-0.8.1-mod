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


class CDescramblePmtTable : public CPmtTable
{
	CTsDescrambler *m_pDescrambler;
	CTsPidMapManager *m_pMapManager;
	CEcmProcessor *m_pEcmProcessor;
	WORD m_EcmPID;
	std::vector<WORD> m_EsPIDList;
public:
	CDescramblePmtTable(CTsDescrambler *pDescrambler);
	void OnTableUpdate();
	void SetTarget();
	void ResetTarget();
	// CTsPidMapTarget
	virtual void OnPidUnmapped(const WORD wPID);
};


//////////////////////////////////////////////////////////////////////
// CTsDescrambler �\�z/����
//////////////////////////////////////////////////////////////////////

CTsDescrambler::CTsDescrambler(IEventHandler *pEventHandler)
	: CMediaDecoder(pEventHandler, 1UL, 1UL)
	, m_bDescramble(true)
	, m_InputPacketCount(0)
	, m_ScramblePacketCount(0)
	, m_DescrambleServiceID(0)
	, m_Queue(&m_BcasCard)
{
	Reset();
}

CTsDescrambler::~CTsDescrambler()
{
	CloseBcasCard();
}

void CTsDescrambler::Reset(void)
{
	CBlockLock Lock(&m_DecoderLock);

	m_Queue.Clear();

	// ������Ԃ�����������
	m_PidMapManager.UnmapAllTarget();

	// PAT�e�[�u��PID�}�b�v�ǉ�
	m_PidMapManager.MapTarget(0x0000U, new CPatTable, OnPatUpdated, this);

	// ���v�f�[�^������
	m_InputPacketCount = 0;
	m_ScramblePacketCount = 0;

	// �X�N�����u�������^�[�Q�b�g������
	m_DescrambleServiceID = 0;
	m_ServiceList.clear();
}

const bool CTsDescrambler::InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex)
{
	CBlockLock Lock(&m_DecoderLock);

	/*
	if(dwInputIndex >= GetInputNum())return false;

	CTsPacket *pTsPacket = dynamic_cast<CTsPacket *>(pMediaData);

	// ���̓��f�B�A�f�[�^�͌݊������Ȃ�
	if(!pTsPacket)return false;
	*/

	CTsPacket *pTsPacket = static_cast<CTsPacket *>(pMediaData);

	// ���̓p�P�b�g���J�E���g
	m_InputPacketCount++;

	if (m_bDescramble) {
		// PID���[�e�B���O
		m_PidMapManager.StorePacket(pTsPacket);
	} else if (pTsPacket->IsScrambled()) {
		// �����R��p�P�b�g���J�E���g
		m_ScramblePacketCount++;
	}

	// �p�P�b�g�������f�R�[�_�Ƀf�[�^��n��
	OutputMedia(pMediaData);

	return true;
}

const bool CTsDescrambler::EnableDescramble(bool bDescramble)
{
	CBlockLock Lock(&m_DecoderLock);

	m_bDescramble = bDescramble;
	return true;
}

const bool CTsDescrambler::OpenBcasCard(CCardReader::ReaderType ReaderType)
{
	CloseBcasCard();

#if 0
	// �J�[�h���[�_����B-CAS�J�[�h���������ĊJ��
	const bool bReturn = m_BcasCard.OpenCard(ReaderType);

	// �G���[�R�[�h�Z�b�g
	if(pErrorCode)*pErrorCode = m_BcasCard.GetLastError();

	if (bReturn)
		m_Queue.BeginBcasThread();

	return bReturn;
#else
	// �J�[�h���[�_�ɃA�N�Z�X����X���b�h�ŃJ�[�h���[�_���J��
	// HDUS�̃J�[�h���[�_��COM���g�����߁A�A�N�Z�X����X���b�h��CoInitialize����
	bool bOK = m_Queue.BeginBcasThread(ReaderType);

	SetError(m_Queue.GetLastErrorException());

	return bOK;
#endif
}

void CTsDescrambler::CloseBcasCard(void)
{
	m_Queue.EndBcasThread();
	// B-CAS�J�[�h�����
	//m_BcasCard.CloseCard();
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
	if (pCardID && pBuff)
		::CopyMemory(pCardID, pBuff, 6UL);

	return (pBuff)? true : false;
}

LPCTSTR CTsDescrambler::GetCardReaderName() const
{
	return m_BcasCard.GetCardReaderName();
}

int CTsDescrambler::FormatBcasCardID(LPTSTR pszText,int MaxLength) const
{
	return m_BcasCard.FormatCardID(pszText, MaxLength);
}

char CTsDescrambler::GetBcasCardManufacturerID() const
{
	return m_BcasCard.GetCardManufacturerID();
}

BYTE CTsDescrambler::GetBcasCardVersion() const
{
	return m_BcasCard.GetCardVersion();
}

const DWORD CTsDescrambler::GetInputPacketCount(void) const
{
	// ���̓p�P�b�g����Ԃ�
	return (DWORD)m_InputPacketCount;
}

const DWORD CTsDescrambler::GetScramblePacketCount(void) const
{
	// �����R��p�P�b�g����Ԃ�
	return (DWORD)m_ScramblePacketCount;
}

void CTsDescrambler::ResetScramblePacketCount(void)
{
	m_ScramblePacketCount = 0;
}

int CTsDescrambler::GetServiceIndexByID(WORD ServiceID) const
{
	int Index;

	// �v���O����ID����T�[�r�X�C���f�b�N�X����������
	for (Index = m_ServiceList.size() - 1 ; Index >= 0  ; Index--) {
		if (m_ServiceList[Index].ServiceID == ServiceID)
			break;
	}

	return Index;
}

bool CTsDescrambler::SetTargetServiceID(WORD ServiceID)
{
	if (m_DescrambleServiceID != ServiceID) {
		CBlockLock Lock(&m_DecoderLock);

		m_DescrambleServiceID = ServiceID;

		for (size_t i = 0 ; i < m_ServiceList.size() ; i++) {
			CDescramblePmtTable *pPmtTable = dynamic_cast<CDescramblePmtTable *>(m_PidMapManager.GetMapTarget(m_ServiceList[i].PmtPID));

			if (pPmtTable) {
				const bool bTarget = m_DescrambleServiceID == 0
						|| m_ServiceList[i].ServiceID == m_DescrambleServiceID;

				if (bTarget && !m_ServiceList[i].bTarget) {
					pPmtTable->SetTarget();
					m_ServiceList[i].bTarget = true;
				}
			}
		}

		for (size_t i = 0 ; i < m_ServiceList.size() ; i++) {
			CDescramblePmtTable *pPmtTable = dynamic_cast<CDescramblePmtTable *>(m_PidMapManager.GetMapTarget(m_ServiceList[i].PmtPID));

			if (pPmtTable) {
				const bool bTarget = m_DescrambleServiceID == 0
						|| m_ServiceList[i].ServiceID == m_DescrambleServiceID;

				if (!bTarget && m_ServiceList[i].bTarget) {
					pPmtTable->ResetTarget();
					m_ServiceList[i].bTarget = false;
				}
			}
		}
	}
	return true;
}

void CTsDescrambler::IncrementScramblePacketCount()
{
	m_ScramblePacketCount++;
}

void CALLBACK CTsDescrambler::OnPatUpdated(const WORD wPID, CTsPidMapTarget *pMapTarget, CTsPidMapManager *pMapManager, const PVOID pParam)
{
	// PAT���X�V���ꂽ
	CTsDescrambler *pThis = static_cast<CTsDescrambler *>(pParam);
	CPatTable *pPatTable = dynamic_cast<CPatTable *>(pMapTarget);

	for (size_t i = 0 ; i < pThis->m_ServiceList.size() ; i++)
		pThis->m_PidMapManager.UnmapTarget(pThis->m_ServiceList[i].PmtPID);

	pThis->m_ServiceList.resize(pPatTable->GetProgramNum());
	for (WORD i = 0 ; i < pPatTable->GetProgramNum() ; i++) {
		const WORD PmtPID = pPatTable->GetPmtPID(i);

		pThis->m_ServiceList[i].bTarget = false;
		pThis->m_ServiceList[i].ServiceID = pPatTable->GetProgramID(i);
		pThis->m_ServiceList[i].PmtPID = PmtPID;
		pThis->m_ServiceList[i].EcmPID = 0xFFFF;
		pThis->m_ServiceList[i].EsPIDList.clear();

		pThis->m_PidMapManager.MapTarget(PmtPID, new CDescramblePmtTable(pThis), OnPmtUpdated, pThis);
	}
}

void CALLBACK CTsDescrambler::OnPmtUpdated(const WORD wPID, CTsPidMapTarget *pMapTarget, CTsPidMapManager *pMapManager, const PVOID pParam)
{
	// PMT���X�V���ꂽ
	CTsDescrambler *pThis = static_cast<CTsDescrambler *>(pParam);
	CDescramblePmtTable *pPmtTable = dynamic_cast<CDescramblePmtTable *>(pMapTarget);

	const WORD ServiceID = pPmtTable->m_CurSection.GetTableIdExtension();
	const int ServiceIndex = pThis->GetServiceIndexByID(ServiceID);
	if (ServiceIndex < 0)
		return;

	pThis->m_ServiceList[ServiceIndex].EcmPID = pPmtTable->GetEcmPID();

	pThis->m_ServiceList[ServiceIndex].EsPIDList.clear();
	for (WORD i = 0 ; i < pPmtTable->GetEsInfoNum() ; i++)
		pThis->m_ServiceList[ServiceIndex].EsPIDList.push_back(pPmtTable->GetEsPID(i));

	pThis->m_ServiceList[ServiceIndex].bTarget = pThis->m_DescrambleServiceID == 0
								|| ServiceID == pThis->m_DescrambleServiceID;
	if (pThis->m_ServiceList[ServiceIndex].bTarget)
		pPmtTable->OnTableUpdate();
	else
		pPmtTable->ResetTarget();
}


CDescramblePmtTable::CDescramblePmtTable(CTsDescrambler *pDescrambler)
	: m_pDescrambler(pDescrambler)
	, m_pMapManager(&pDescrambler->m_PidMapManager)
	, m_pEcmProcessor(NULL)
	, m_EcmPID(0)
{
}

void CDescramblePmtTable::OnTableUpdate()
{
	// ECM��PID�}�b�v�ǉ�
	WORD EcmPID = GetEcmPID();
	if (EcmPID >= 0x1FFFU)
		EcmPID = 0;

	if (m_EcmPID != EcmPID) {
		if (m_EcmPID != 0) {
			if (m_pEcmProcessor->GetTargetCount() > 1) {
				m_pEcmProcessor->DecrementTargetCount();
			} else {
				m_pMapManager->UnmapTarget(m_EcmPID);
			}
			m_pEcmProcessor = NULL;
		}
		if (EcmPID != 0) {
			// ������ECM�����^�[�Q�b�g���m�F
			m_pEcmProcessor = dynamic_cast<CEcmProcessor *>(m_pMapManager->GetMapTarget(EcmPID));

			if (m_pEcmProcessor) {
				m_pEcmProcessor->IncrementTargetCount();
			} else {
				// ECM���������N���X�V�K�}�b�v
				m_pEcmProcessor = new CEcmProcessor(m_pDescrambler, &m_pDescrambler->m_BcasCard, &m_pDescrambler->m_Queue);
				m_pMapManager->MapTarget(EcmPID, m_pEcmProcessor);
			}
		}
		m_EcmPID = EcmPID;
	}

	if (m_pEcmProcessor) {
		// ES��PID�}�b�v�ǉ�
		for (WORD i = 0 ; i < GetEsInfoNum() ; i++) {
			const WORD EsPID = GetEsPID(i);
			CTsDescrambler::CEsProcessor *pEsProcessor = dynamic_cast<CTsDescrambler::CEsProcessor *>(m_pMapManager->GetMapTarget(EsPID));

			if (pEsProcessor) {
				pEsProcessor->AddRef();
			} else {
				m_pMapManager->MapTarget(EsPID, new CTsDescrambler::CEsProcessor(m_pEcmProcessor));
			}
		}
	}

	for (size_t i = 0 ; i < m_EsPIDList.size() ; i++) {
		CTsDescrambler::CEsProcessor *pEsProcessor = dynamic_cast<CTsDescrambler::CEsProcessor *>(m_pMapManager->GetMapTarget(m_EsPIDList[i]));

		if (pEsProcessor) {
			if (pEsProcessor->GetRefCount() > 1)
				pEsProcessor->ReleaseRef();
			else
				m_pMapManager->UnmapTarget(m_EsPIDList[i]);
		}
	}

	m_EsPIDList.resize(GetEsInfoNum());
	for (WORD i = 0 ; i < GetEsInfoNum() ; i++ )
		m_EsPIDList[i] = GetEsPID(i);
}

void CDescramblePmtTable::SetTarget()
{
	// ECM��PID�}�b�v�ǉ�
	m_EcmPID = GetEcmPID();
	if (m_EcmPID >= 0x1FFFU)
		m_EcmPID = 0;

	if (m_EcmPID != 0) {
		// ������ECM�����^�[�Q�b�g���m�F
		m_pEcmProcessor = dynamic_cast<CEcmProcessor *>(m_pMapManager->GetMapTarget(m_EcmPID));

		if (m_pEcmProcessor) {
			m_pEcmProcessor->IncrementTargetCount();
		} else {
			// ECM���������N���X�V�K�}�b�v
			m_pEcmProcessor = new CEcmProcessor(m_pDescrambler, &m_pDescrambler->m_BcasCard, &m_pDescrambler->m_Queue);
			m_pMapManager->MapTarget(m_EcmPID, m_pEcmProcessor);
		}
	}

	if (m_pEcmProcessor) {
		// ES��PID�}�b�v�ǉ�
		for (WORD i = 0 ; i < GetEsInfoNum() ; i++) {
			const WORD EsPID = GetEsPID(i);
			CTsDescrambler::CEsProcessor *pEsProcessor = dynamic_cast<CTsDescrambler::CEsProcessor *>(m_pMapManager->GetMapTarget(EsPID));

			if (pEsProcessor) {
				pEsProcessor->AddRef();
			} else {
				m_pMapManager->MapTarget(EsPID, new CTsDescrambler::CEsProcessor(m_pEcmProcessor));
			}
		}
	}

	m_EsPIDList.resize(GetEsInfoNum());
	for (WORD i = 0 ; i < GetEsInfoNum() ; i++ )
		m_EsPIDList[i] = GetEsPID(i);
}

void CDescramblePmtTable::ResetTarget()
{
	if (m_EcmPID != 0) {
		if (m_pEcmProcessor->GetTargetCount() > 1) {
			m_pEcmProcessor->DecrementTargetCount();
		} else {
			m_pMapManager->UnmapTarget(m_EcmPID);
		}
		m_pEcmProcessor = NULL;
		m_EcmPID = 0;
	}

	if (m_EsPIDList.size() > 0) {
		for (size_t i = 0 ; i < m_EsPIDList.size() ; i++) {
			CTsDescrambler::CEsProcessor *pEsProcessor = dynamic_cast<CTsDescrambler::CEsProcessor *>(m_pMapManager->GetMapTarget(m_EsPIDList[i]));

			if (pEsProcessor) {
				if (pEsProcessor->GetRefCount() > 1)
					pEsProcessor->ReleaseRef();
				else
					m_pMapManager->UnmapTarget(m_EsPIDList[i]);
			}
		}

		m_EsPIDList.clear();
	}
}

void CDescramblePmtTable::OnPidUnmapped(const WORD wPID)
{
	ResetTarget();

	CPmtTable::OnPidUnmapped(wPID);
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
	, m_bLastEcmSucceed(true)
	, m_TargetCount(0)
{
	// MULTI2�f�R�[�_�ɃV�X�e���L�[�Ə���CBC���Z�b�g
	if (m_pBcasCard->IsCardOpen())
		m_Multi2Decoder.Initialize(m_pBcasCard->GetSystemKey(),
								   m_pBcasCard->GetInitialCbc());

	m_SetScrambleKeyEvent.Create(true);
}

void CEcmProcessor::OnPidMapped(const WORD wPID, const PVOID pParam)
{
	TRACE(TEXT("CEcmProcessor::OnPidMapped() PID = %d (0x%04x)\n"), wPID, wPID);
	// �Q�ƃJ�E���g�ǉ�
	AddRef();
	m_TargetCount++;
}

void CEcmProcessor::OnPidUnmapped(const WORD wPID)
{
	TRACE(TEXT("CEcmProcessor::OnPidUnmapped() PID = %d (0x%04x)\n"), wPID, wPID);
	m_TargetCount--;
	// �Q�ƃJ�E���g�J��
	ReleaseRef();
}

const bool CEcmProcessor::DescramblePacket(CTsPacket *pTsPacket)
{
	if (!m_bInQueue) {	// �܂�ECM�����Ă��Ȃ�
		m_pDescrambler->IncrementScramblePacketCount();
		return false;
	}
	if (m_SetScrambleKeyEvent.Wait(500) == WAIT_TIMEOUT)
		return false;

	// �X�N�����u������
	bool bOK;
	if (m_bLastEcmSucceed) {
		CBlockLock Lock(&m_Multi2Lock);

		bOK = m_Multi2Decoder.Decode(pTsPacket->GetPayloadData(),
									 (DWORD)pTsPacket->GetPayloadSize(),
									 pTsPacket->m_Header.byTransportScramblingCtrl);
	} else {
		bOK = false;
	}
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
		m_bInQueue = true;
#endif
	return true;
}

const bool CEcmProcessor::SetScrambleKey(const BYTE *pEcmData, DWORD EcmSize)
{
	// ECM��B-CAS�J�[�h�ɓn���ăL�[�擾
	const BYTE *pKsData = m_pBcasCard->GetKsFromEcm(pEcmData, EcmSize);

	// ECM�������s���͈�x����B-CAS�J�[�h���ď���������
	if (!pKsData && m_bLastEcmSucceed
			&& (m_pBcasCard->GetLastErrorCode() != BCEC_ECMREFUSED)) {
		if (m_pBcasCard->ReOpenCard()) {
			TRACE(TEXT("CEcmProcessor::SetScrambleKey() Re open card.\n"));
			pKsData = m_pBcasCard->GetKsFromEcm(pEcmData, EcmSize);
		}
	}

	// �X�N�����u���L�[�X�V
	m_Multi2Lock.Lock();
	m_Multi2Decoder.SetScrambleKey(pKsData);
	m_Multi2Lock.Unlock();

	// ECM����������ԍX�V
	m_bLastEcmSucceed = pKsData != NULL;

	m_SetScrambleKeyEvent.Set();

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
	if (pPacket->IsScrambled())
		m_pEcmProcessor->DescramblePacket(const_cast<CTsPacket *>(pPacket));

	return false;
}

void CTsDescrambler::CEsProcessor::OnPidMapped(const WORD wPID, const PVOID pParam)
{
	TRACE(TEXT("CEsProcessor::OnPidMapped() PID = %d (0x%04x)\n"), wPID, wPID);
	// �Q�ƃJ�E���g�ǉ�
	AddRef();
}

void CTsDescrambler::CEsProcessor::OnPidUnmapped(const WORD wPID)
{
	TRACE(TEXT("CEsProcessor::OnPidUnmapped() PID = %d (0x%04x)\n"), wPID, wPID);
	// �Q�ƃJ�E���g�폜
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




CBcasAccessQueue::CBcasAccessQueue(CBcasCard *pBcasCard)
	: m_pBcasCard(pBcasCard)
	, m_hThread(NULL)
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
	if (m_hThread == NULL || Size > 256)
		return false;

	CBlockLock Lock(&m_Lock);

	CBcasAccess BcasAccess(pEcmProcessor, pData, Size);
	m_Queue.push_back(BcasAccess);
	m_Event.Set();
	return true;
}


bool CBcasAccessQueue::BeginBcasThread(CCardReader::ReaderType ReaderType)
{
	if (m_hThread)
		return false;
	m_ReaderType = ReaderType;
	if (m_Event.IsCreated())
		m_Event.Reset();
	else
		m_Event.Create();
	m_bKillEvent = false;
	m_bStartEvent = false;
	m_hThread = ::CreateThread(NULL, 0, BcasAccessThread, this, 0, NULL);
	if (m_hThread == NULL)
		return false;
	m_Event.Wait();
	if (!m_pBcasCard->IsCardOpen()) {
		::WaitForSingleObject(m_hThread, INFINITE);
		::CloseHandle(m_hThread);
		m_hThread = NULL;
		return false;
	}
	m_bStartEvent = true;
	ClearError();
	return true;
}


bool CBcasAccessQueue::EndBcasThread()
{
	if (m_hThread) {
		m_bKillEvent = true;
		m_Event.Set();
		Clear();
		if (::WaitForSingleObject(m_hThread, 1000) == WAIT_TIMEOUT) {
			TRACE(TEXT("Terminate BcasAccessThread\n"));
			::TerminateThread(m_hThread, 1);
		}
		::CloseHandle(m_hThread);
		m_hThread = NULL;
	}
	return true;
}


DWORD CALLBACK CBcasAccessQueue::BcasAccessThread(LPVOID lpParameter)
{
	CBcasAccessQueue *pThis=static_cast<CBcasAccessQueue*>(lpParameter);

	// �J�[�h���[�_����B-CAS�J�[�h���������ĊJ��
	if (!pThis->m_pBcasCard->OpenCard(pThis->m_ReaderType)) {
		pThis->SetError(pThis->m_pBcasCard->GetLastErrorException());
		pThis->m_Event.Set();
		return 1;
	}
	pThis->m_Event.Set();
	while (!pThis->m_bStartEvent)
		::Sleep(0);

	while (true) {
		pThis->m_Event.Wait();
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

	// B-CAS�J�[�h�����
	pThis->m_pBcasCard->CloseCard();

	return 0;
}
