// ProgManager.cpp: CProgManager �N���X�̃C���v�������e�[�V����
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TsTable.h"
#include "TsProgGuide.h"
#include "ProgManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// CProgManager �\�z/����
//////////////////////////////////////////////////////////////////////

CProgManager::CProgManager(IEventHandler *pEventHandler)
	: CMediaDecoder(pEventHandler, 1UL, 1UL)
	, m_pProgDatabase(new CProgDatabase(*this))
{
//	m_PidMapManager.MapTarget(0x0012U, new CEitParser(NULL));
}


CProgManager::~CProgManager()
{
	// �v���O�����f�[�^�x�[�X�C���X�^���X�J��
	delete m_pProgDatabase;
}


void CProgManager::Reset()
{
	CBlockLock Lock(&m_DecoderLock);

	// �T�[�r�X���X�g���N���A
	m_ServiceList.clear();

	// �v���O�����f�[�^�x�[�X���Z�b�g
	m_pProgDatabase->Reset();

	// ���ʃf�R�[�_�����Z�b�g
	ResetDownstreamDecoder();
}


const bool CProgManager::InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex)
{
	CBlockLock Lock(&m_DecoderLock);

	/*
	if(dwInputIndex >= GetInputNum())return false;

	CTsPacket *pTsPacket = dynamic_cast<CTsPacket *>(pMediaData);

	// ���̓��f�B�A�f�[�^�͌݊������Ȃ�
	if(!pTsPacket)return false;
	*/

	CTsPacket *pTsPacket = static_cast<CTsPacket *>(pMediaData);

	// PID���[�e�B���O
	m_PidMapManager.StorePacket(pTsPacket);

	// ���̃t�B���^�Ƀf�[�^��n��
	OutputMedia(pMediaData);

	return true;
}


const WORD CProgManager::GetServiceNum(void)
{
	CBlockLock Lock(&m_DecoderLock);

	// �T�[�r�X����Ԃ�
	return m_ServiceList.size();
}


const bool CProgManager::GetServiceID(WORD *pwServiceID, const WORD wIndex)
{
	CBlockLock Lock(&m_DecoderLock);

	if (pwServiceID == NULL)
		return false;

	// �T�[�r�XID���擾����
	if (wIndex == 0xFFFF) {
		if (m_pProgDatabase->m_ServiceList.size() == 0
				|| !m_pProgDatabase->m_ServiceList[0].bIsUpdated)
			return false;
		*pwServiceID = m_pProgDatabase->m_ServiceList[0].wServiceID;
	} else if ((size_t)wIndex < m_ServiceList.size()) {
		*pwServiceID = m_ServiceList[wIndex].wServiceID;
	} else {
		return false;
	}
	return true;
}


const bool CProgManager::GetVideoEsPID(WORD *pwVideoPID, const WORD wIndex)
{
	CBlockLock Lock(&m_DecoderLock);

	if (pwVideoPID && (size_t)wIndex < m_ServiceList.size()) {
		*pwVideoPID = m_ServiceList[wIndex].wVideoEsPID;
		return true;
	}
	return false;
}


const bool CProgManager::GetAudioEsPID(WORD *pwAudioPID, const WORD wAudioIndex, const WORD wIndex)
{
	CBlockLock Lock(&m_DecoderLock);

	if (pwAudioPID && (size_t)wIndex < m_ServiceList.size()
			&& (size_t)wAudioIndex < m_ServiceList[wIndex].AudioEsPIDs.size()) {
		*pwAudioPID = m_ServiceList[wIndex].AudioEsPIDs[wAudioIndex];
		return true;
	}
	return false;
}


const WORD CProgManager::GetAudioEsNum(const WORD wIndex)
{
	CBlockLock Lock(&m_DecoderLock);

	if ((size_t)wIndex < m_ServiceList.size())
		return m_ServiceList[wIndex].AudioEsPIDs.size();
	return 0;
}


const bool CProgManager::GetPcrTimeStamp(unsigned __int64 *pu64PcrTimeStamp, const WORD wIndex)
{
	CBlockLock Lock(&m_DecoderLock);

	// PCR���擾����
	if ((size_t)wIndex < m_ServiceList.size() && pu64PcrTimeStamp) {
		*pu64PcrTimeStamp = m_ServiceList[wIndex].u64TimeStamp;
		return true;
	}
	return false;
}


const DWORD CProgManager::GetServiceName(LPTSTR lpszDst, const WORD wIndex)
{
	CBlockLock Lock(&m_DecoderLock);

	// �T�[�r�X�����擾����
	if ((size_t)wIndex < m_ServiceList.size()) {
		if (lpszDst)
			::lstrcpy(lpszDst, m_ServiceList[wIndex].szServiceName);
		return ::lstrlen(m_ServiceList[wIndex].szServiceName);
	}
	return 0U;
}


const WORD CProgManager::GetTransportStreamID() const
{
	return m_pProgDatabase->m_wTransportStreamID;
}


WORD CProgManager::GetNetworkID(void) const
{
	return m_pProgDatabase->m_NitInfo.wNetworkID;
}


BYTE CProgManager::GetBroadcastingID(void) const
{
	return m_pProgDatabase->m_NitInfo.byBroadcastingID;
}


DWORD CProgManager::GetNetworkName(LPTSTR pszName,int MaxLength)
{
	CBlockLock Lock(&m_DecoderLock);

	if (pszName)
		::lstrcpyn(pszName,m_pProgDatabase->m_NitInfo.szNetworkName,MaxLength);
	return ::lstrlen(m_pProgDatabase->m_NitInfo.szNetworkName);
}


BYTE CProgManager::GetRemoteControlKeyID(void) const
{
	return m_pProgDatabase->m_NitInfo.byRemoteControlKeyID;
}


DWORD CProgManager::GetTSName(LPTSTR pszName,int MaxLength)
{
	CBlockLock Lock(&m_DecoderLock);

	if (pszName)
		::lstrcpyn(pszName,m_pProgDatabase->m_NitInfo.szTSName,MaxLength);
	return ::lstrlen(m_pProgDatabase->m_NitInfo.szTSName);
}


void CProgManager::OnServiceListUpdated(void)
{
	// �T�[�r�X���X�g�N���A�A���T�C�Y
	m_ServiceList.clear();

	// �T�[�r�X���X�g�\�z
	for (WORD wIndex = 0U, wServiceNum = 0U ; wIndex < m_pProgDatabase->m_ServiceList.size() ; wIndex++) {
		if (m_pProgDatabase->m_ServiceList[wIndex].wVideoEsPID != 0xFFFFU) {
			// MPEG2�f���̂�(�����Z�O�A�f�[�^�����ȊO)
			m_ServiceList.resize(wServiceNum + 1);
			m_ServiceList[wServiceNum].wServiceID = m_pProgDatabase->m_ServiceList[wIndex].wServiceID;
			m_ServiceList[wServiceNum].wVideoEsPID = m_pProgDatabase->m_ServiceList[wIndex].wVideoEsPID;
			m_ServiceList[wServiceNum].AudioEsPIDs = m_pProgDatabase->m_ServiceList[wIndex].AudioEsPIDs;
			m_ServiceList[wServiceNum].szServiceName[0] = TEXT('\0');
			wServiceNum++;
		}
	}

	TRACE(TEXT("CProgManager::OnServiceListUpdated()\n"));

	SendDecoderEvent(EID_SERVICE_LIST_UPDATED);
}


void CProgManager::OnServiceInfoUpdated(void)
{
	// �T�[�r�X�����X�V����
	for (WORD wIndex = 0U, wServiceNum = 0U ; wIndex < GetServiceNum() ; wIndex++) {
		const WORD wServiceIndex = m_pProgDatabase->GetServiceIndexByID(m_ServiceList[wIndex].wServiceID);

		if (wServiceIndex != 0xFFFFU) {
			if (m_pProgDatabase->m_ServiceList[wIndex].szServiceName[0]) {
				::lstrcpy(m_ServiceList[wIndex].szServiceName,
						  m_pProgDatabase->m_ServiceList[wServiceIndex].szServiceName);
			} else {
				::wsprintf(m_ServiceList[wIndex].szServiceName, TEXT("�T�[�r�X%d"), wIndex + 1);
			}
		}
	}

	TRACE(TEXT("CProgManager::OnServiceInfoUpdated()\n"));

	SendDecoderEvent(EID_SERVICE_INFO_UPDATED);
}


void CProgManager::OnPcrTimestampUpdated(void)
{
	// PCR���X�V����
	for (WORD wIndex = 0U, wServiceNum = 0U ; wIndex < GetServiceNum() ; wIndex++) {
		const WORD wServiceIndex = m_pProgDatabase->GetServiceIndexByID(m_ServiceList[wIndex].wServiceID);

		if (wServiceIndex != 0xFFFFU) {
			m_ServiceList[wIndex].u64TimeStamp = m_pProgDatabase->m_ServiceList[wServiceIndex].u64TimeStamp;
		}
	}

//	TRACE(TEXT("CProgManager::OnPcrTimeStampUpdated()\n"));

	SendDecoderEvent(EID_PCR_TIMESTAMP_UPDATED);
}


//////////////////////////////////////////////////////////////////////
// CProgDatabase �\�z/����
//////////////////////////////////////////////////////////////////////

CProgManager::CProgDatabase::CProgDatabase(CProgManager &ProgManager)
	: m_ProgManager(ProgManager)
	, m_PidMapManager(ProgManager.m_PidMapManager)
	, m_wTransportStreamID(0x0000U)
{
	Reset();
}


CProgManager::CProgDatabase::~CProgDatabase()
{
	UnmapTable();
}


void CProgManager::CProgDatabase::Reset(void)
{
	// �S�e�[�u���A���}�b�v
	UnmapTable();

	// PAT�e�[�u��PID�}�b�v�ǉ�
	m_PidMapManager.MapTarget(0x0000U, new CPatTable, CProgDatabase::OnPatUpdated, this);

	// NIT�e�[�u��PID�}�b�v�ǉ�
	m_PidMapManager.MapTarget(0x0010U, new CNitTable, CProgDatabase::OnNitUpdated, this);
	::ZeroMemory(&m_NitInfo,sizeof(m_NitInfo));
}


void CProgManager::CProgDatabase::UnmapTable(void)
{
	// �SPMT PID�A���}�b�v
	for(WORD wIndex = 0U ; wIndex < m_ServiceList.size() ; wIndex++) {
		m_PidMapManager.UnmapTarget(m_ServiceList[wIndex].wPmtTablePID);
	}

	// �T�[�r�X���X�g�N���A
	m_ServiceList.clear();

	// �g�����X�|�[�g�X�g���[��ID������
	m_wTransportStreamID = 0x0000U;

	// PAT�e�[�u�����Z�b�g
	CPatTable *pPatTable = dynamic_cast<CPatTable *>(m_PidMapManager.GetMapTarget(0x0000U));
	if (pPatTable)
		pPatTable->Reset();
}


const WORD CProgManager::CProgDatabase::GetServiceIndexByID(const WORD wServiceID)
{
	// �v���O����ID����T�[�r�X�C���f�b�N�X����������
	for (size_t Index = 0 ; Index < m_ServiceList.size() ; Index++) {
		if (m_ServiceList[Index].wServiceID == wServiceID)
			return Index;
	}

	// �v���O����ID��������Ȃ�
	return 0xFFFFU;
}


void CALLBACK CProgManager::CProgDatabase::OnPatUpdated(const WORD wPID, CTsPidMapTarget *pMapTarget, CTsPidMapManager *pMapManager, const PVOID pParam)
{
	// PAT���X�V���ꂽ
	CProgDatabase *pThis = static_cast<CProgDatabase *>(pParam);
	CPatTable *pPatTable = dynamic_cast<CPatTable *>(pMapTarget);

	// �g�����X�|�[�g�X�g���[��ID�X�V
	pThis->m_wTransportStreamID = pPatTable->m_CurSection.GetTableIdExtension();

	// ��PMT/PCR��PID���A���}�b�v����
	for (size_t Index = 0 ; Index < pThis->m_ServiceList.size() ; Index++) {
		WORD wPID;
		wPID = pThis->m_ServiceList[Index].wPmtTablePID;
		pMapManager->UnmapTarget(wPID);
		wPID = pThis->m_ServiceList[Index].wPcrPID;
		pMapManager->UnmapTarget(wPID);
	}

	// �VPMT���X�g�A����
	pThis->m_ServiceList.resize(pPatTable->GetProgramNum());

	for (size_t Index = 0 ; Index < pThis->m_ServiceList.size() ; Index++) {
		// �T�[�r�X���X�g�X�V
		pThis->m_ServiceList[Index].bIsUpdated = false;
		pThis->m_ServiceList[Index].wServiceID = pPatTable->GetProgramID(Index);
		pThis->m_ServiceList[Index].wPmtTablePID = pPatTable->GetPmtPID(Index);
		pThis->m_ServiceList[Index].wVideoEsPID = 0xFFFFU;
		pThis->m_ServiceList[Index].AudioEsPIDs.clear();
		pThis->m_ServiceList[Index].wPcrPID = 0xFFFFU;
		pThis->m_ServiceList[Index].byVideoComponentTag = 0xFFU;
		pThis->m_ServiceList[Index].byAudioComponentTag = 0xFFU;
		pThis->m_ServiceList[Index].byServiceType = 0xFFU;
		pThis->m_ServiceList[Index].byRunningStatus = 0xFFU;
		pThis->m_ServiceList[Index].bIsCaService = false;
		pThis->m_ServiceList[Index].szServiceName[0] = TEXT('\0');

		// PMT��PID���}�b�v
		pMapManager->MapTarget(pPatTable->GetPmtPID(Index), new CPmtTable, CProgDatabase::OnPmtUpdated, pParam);
	}
}


void CALLBACK CProgManager::CProgDatabase::OnPmtUpdated(const WORD wPID, CTsPidMapTarget *pMapTarget, CTsPidMapManager *pMapManager, const PVOID pParam)
{
	// PMT���X�V���ꂽ
	CProgDatabase *pThis = static_cast<CProgDatabase *>(pParam);
	CPmtTable *pPmtTable = dynamic_cast<CPmtTable *>(pMapTarget);

	// �T�[�r�X�C���f�b�N�X������
	const WORD wServiceIndex = pThis->GetServiceIndexByID(pPmtTable->m_CurSection.GetTableIdExtension());
	if(wServiceIndex == 0xFFFFU)return;

	// �r�f�IES��PID���X�g�A
	pThis->m_ServiceList[wServiceIndex].wVideoEsPID = 0xFFFFU;

	for (WORD wEsIndex = 0U ; wEsIndex < pPmtTable->GetEsInfoNum() ; wEsIndex++) {
		// �uITU-T Rec. H.262|ISO/IEC 13818-2 Video or ISO/IEC 11172-2�v�̃X�g���[���^�C�v������
		if (pPmtTable->GetStreamTypeID(wEsIndex) == 0x02U) {
			pThis->m_ServiceList[wServiceIndex].wVideoEsPID = pPmtTable->GetEsPID(wEsIndex);
			break;
		}
	}

	// �I�[�f�B�IES��PID���X�g�A
	pThis->m_ServiceList[wServiceIndex].AudioEsPIDs.clear();

	for (WORD wEsIndex = 0U ; wEsIndex < pPmtTable->GetEsInfoNum() ; wEsIndex++) {
		// �uISO/IEC 13818-7 Audio (ADTS Transport Syntax)�v�̃X�g���[���^�C�v������
		if (pPmtTable->GetStreamTypeID(wEsIndex) == 0x0FU) {
			pThis->m_ServiceList[wServiceIndex].AudioEsPIDs.push_back(pPmtTable->GetEsPID(wEsIndex));
		}
	}

	WORD wPcrPID = pPmtTable->GetPcrPID();
	if (wPcrPID < 0x1FFFU) {
		pThis->m_ServiceList[wServiceIndex].wPcrPID = wPcrPID;
		CTsPidMapTarget *pMap = pMapManager->GetMapTarget(wPcrPID);
		if (!pMap) {
			// �V�KMap
			pMapManager->MapTarget(wPcrPID, new CPcrTable(wServiceIndex), CProgDatabase::OnPcrUpdated, pParam);
		} else {
			// ����Map
			CPcrTable *pPcrTable = dynamic_cast<CPcrTable*>(pMap);
			if(pPcrTable) {
				// �T�[�r�X�ǉ�
				pPcrTable->AddServiceIndex(wServiceIndex);
			}
		}
	}

	// �X�V�ς݃}�[�N
	pThis->m_ServiceList[wServiceIndex].bIsUpdated = true;

/*
	�����ǂɂ���Ă�PAT�Ɋ܂܂��T�[�r�X�S�Ă�PMT�𗬂��Ă��Ȃ��ꍇ������B
�@�@(���x���n���h�����Ăяo�����̂�h�~���邽�ߑS�Ă̏�񂪂�������i�K�ŌĂяo����������)

	// ����PMT�̍X�V�󋵂𒲂ׂ�
	for (WORD wIndex = 0U ; wIndex < pThis->m_ServiceList.size() ; wIndex++) {
		if(!pThis->m_ServiceList[wIndex].bIsUpdated)
			return;
	}
*/

	// SDT�e�[�u�����ă}�b�v����
	pMapManager->MapTarget(0x0011U, new CSdtTable, CProgDatabase::OnSdtUpdated, pParam);

	// �C�x���g�n���h���Ăяo��
	pThis->m_ProgManager.OnServiceListUpdated();
}


void CALLBACK CProgManager::CProgDatabase::OnSdtUpdated(const WORD wPID, CTsPidMapTarget *pMapTarget, CTsPidMapManager *pMapManager, const PVOID pParam)
{
	// SDT���X�V���ꂽ
	CProgDatabase *pThis = static_cast<CProgDatabase *>(pParam);
	CSdtTable *pSdtTable = dynamic_cast<CSdtTable *>(pMapTarget);

	for (WORD wSdtIndex = 0U ; wSdtIndex < pSdtTable->GetServiceNum() ; wSdtIndex++) {
		// �T�[�r�XID������
		const WORD wServiceIndex = pThis->GetServiceIndexByID(pSdtTable->GetServiceID(wSdtIndex));
		if (wServiceIndex == 0xFFFFU)
			continue;

		// �T�[�r�X���X�V
		pThis->m_ServiceList[wServiceIndex].byRunningStatus = pSdtTable->GetRunningStatus(wSdtIndex);
		pThis->m_ServiceList[wServiceIndex].bIsCaService = pSdtTable->GetFreeCaMode(wSdtIndex);

		// �T�[�r�X���X�V
		pThis->m_ServiceList[wServiceIndex].szServiceName[0] = TEXT('\0');

		const CDescBlock *pDescBlock = pSdtTable->GetItemDesc(wSdtIndex);
		const CServiceDesc *pServiceDesc = dynamic_cast<const CServiceDesc *>(pDescBlock->GetDescByTag(CServiceDesc::DESC_TAG));

		if (pServiceDesc) {
			pServiceDesc->GetServiceName(pThis->m_ServiceList[wServiceIndex].szServiceName);
			pThis->m_ServiceList[wServiceIndex].byServiceType = pServiceDesc->GetServiceType();
		}
	}

	// �C�x���g�n���h���Ăяo��
	pThis->m_ProgManager.OnServiceInfoUpdated();
}


void CALLBACK CProgManager::CProgDatabase::OnPcrUpdated(const WORD wPID, CTsPidMapTarget *pMapTarget, CTsPidMapManager *pMapManager, const PVOID pParam)
{
	// PCR���X�V���ꂽ
	CProgDatabase *pThis = static_cast<CProgDatabase *>(pParam);
	CPcrTable *pPcrTable = dynamic_cast<CPcrTable *>(pMapTarget);

	const unsigned __int64 u64TimeStamp = pPcrTable->GetPcrTimeStamp();

	WORD wServiceIndex;
	for (WORD wIndex = 0 ; pPcrTable->GetServiceIndex(&wServiceIndex,wIndex); wIndex++) {
		if (wServiceIndex < pThis->m_ServiceList.size()) {
			pThis->m_ServiceList[wServiceIndex].u64TimeStamp = u64TimeStamp;
		}
	}

	// �C�x���g�n���h���Ăяo��
	pThis->m_ProgManager.OnPcrTimestampUpdated();
}


void CALLBACK CProgManager::CProgDatabase::OnNitUpdated(const WORD wPID, CTsPidMapTarget *pMapTarget, CTsPidMapManager *pMapManager, const PVOID pParam){

	CProgDatabase *pThis = static_cast<CProgDatabase*>(pParam);
	CNitTable *pNitTable = dynamic_cast<CNitTable*>(pMapTarget);

	pThis->m_NitInfo.wNetworkID = pNitTable->GetNetworkID();

	const CDescBlock *pDescBlock;
	pDescBlock = pNitTable->GetNetworkNameDesc();
	if (pDescBlock) {
		const CNetworkNameDesc *pNetworkDesc = dynamic_cast<const CNetworkNameDesc *>(pDescBlock->GetDescByTag(CNetworkNameDesc::DESC_TAG));
		if(pNetworkDesc) {
			pNetworkDesc->GetNetworkName(pThis->m_NitInfo.szNetworkName);
		}
	}

	pDescBlock = pNitTable->GetSystemManageDesc();
	if (pDescBlock) {
		const CSystemManageDesc *pSysManageDesc = dynamic_cast<const CSystemManageDesc *>(pDescBlock->GetDescByTag(CSystemManageDesc::DESC_TAG));
		if (pSysManageDesc) {
			pThis->m_NitInfo.byBroadcastingID = pSysManageDesc->GetBroadcastingID();
		}
	}

	pDescBlock = pNitTable->GetTSInfoDesc();
	if (pDescBlock) {
		const CTSInfoDesc *pTsInfoDesc = dynamic_cast<const CTSInfoDesc *>(pDescBlock->GetDescByTag(CTSInfoDesc::DESC_TAG));
		if (pTsInfoDesc) {
			pTsInfoDesc->GetTSName(pThis->m_NitInfo.szTSName);
			pThis->m_NitInfo.byRemoteControlKeyID = pTsInfoDesc->GetRemoteControlKeyID();
		}
	}
}
