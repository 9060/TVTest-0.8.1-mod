// BcasCard.cpp: CBcasCard �N���X�̃C���v�������e�[�V����
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BcasCard.h"
#include "StdUtil.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif




CBcasCard::CBcasCard()
	: m_pCardReader(NULL)
{
	// ������ԏ�����
	::ZeroMemory(&m_BcasCardInfo, sizeof(m_BcasCardInfo));
	::ZeroMemory(&m_EcmStatus, sizeof(m_EcmStatus));
}


CBcasCard::~CBcasCard()
{
	CloseCard();
}


const DWORD CBcasCard::GetCardReaderNum(void) const
{
	// �J�[�h���[�_�[����Ԃ�
	if (m_pCardReader)
		return m_pCardReader->NumReaders();
	return 0;
}


LPCTSTR CBcasCard::EnumCardReader(const DWORD dwIndex) const
{
	if (m_pCardReader)
		return m_pCardReader->EnumReader(dwIndex);
	return NULL;
}


const bool CBcasCard::OpenCard(CCardReader::ReaderType ReaderType, LPCTSTR lpszReader)
{
	// ��U�N���[�Y����
	CloseCard();

	m_pCardReader = CCardReader::CreateCardReader(ReaderType);
	if (m_pCardReader == NULL) {
		SetError(BCEC_CARDOPENERROR, TEXT("Invalid card reader type"));
		return false;
	}
	if (!m_pCardReader->Open(lpszReader)) {
		SetError(BCEC_CARDOPENERROR, m_pCardReader->GetLastErrorText());
		delete m_pCardReader;
		m_pCardReader=NULL;
		return false;
	}

	// �J�[�h������
	if (!InitialSetting()) {
		CloseCard();
		return false;
	}

	ClearError();

	return true;
}


void CBcasCard::CloseCard(void)
{
	// �J�[�h���N���[�Y����
	if (m_pCardReader) {
		m_pCardReader->Close();
		delete m_pCardReader;
		m_pCardReader=NULL;
	}
}


const bool CBcasCard::ReOpenCard()
{
	if (m_pCardReader==NULL) {
		SetError(BCEC_CARDNOTOPEN,NULL);
		return false;
	}
	return OpenCard(m_pCardReader->GetReaderType(),m_pCardReader->GetReaderName());
}


const bool CBcasCard::IsCardOpen() const
{
	return m_pCardReader!=NULL;
}


LPCTSTR CBcasCard::GetCardReaderName() const
{
	if (m_pCardReader)
		return m_pCardReader->GetReaderName();
	return NULL;
}


const bool CBcasCard::InitialSetting(void)
{
	// �uInitial Setting Conditions Command�v����������
	/*
	if (!m_pCardReader) {
		SetError(BCEC_CARDNOTOPEN, NULL);
		return false;
	}
	*/

	// �o�b�t�@����
	DWORD dwRecvSize;
	BYTE RecvData[1024];

	// �R�}���h���M
	static const BYTE InitSettingCmd[] = {0x90U, 0x30U, 0x00U, 0x00U, 0x00U};
	::ZeroMemory(RecvData, sizeof(RecvData));
	dwRecvSize=sizeof(RecvData);
	if (!m_pCardReader->Transmit(InitSettingCmd, sizeof(InitSettingCmd), RecvData, &dwRecvSize)) {
		SetError(BCEC_TRANSMITERROR, m_pCardReader->GetLastErrorText());
		return false;
	}

	if (dwRecvSize < 57UL) {
		SetError(BCEC_TRANSMITERROR, TEXT("��M�f�[�^�̃T�C�Y���s���ł��B"));
		return false;
	}

	// ���X�|���X���
	::CopyMemory(m_BcasCardInfo.BcasCardID, &RecvData[8], 6UL);		// +8	Card ID
	::CopyMemory(m_BcasCardInfo.SystemKey, &RecvData[16], 32UL);	// +16	Descrambling system key
	::CopyMemory(m_BcasCardInfo.InitialCbc, &RecvData[48], 8UL);	// +48	Descrambler CBC initial value

	const static BYTE CardIDInfoCmd[] = {0x90, 0x32, 0x00, 0x00, 0x00};
	::ZeroMemory(RecvData, sizeof(RecvData));
	dwRecvSize=sizeof(RecvData);
	if (m_pCardReader->Transmit(CardIDInfoCmd, sizeof(CardIDInfoCmd), RecvData, &dwRecvSize)
			&& dwRecvSize >= 17) {
		m_BcasCardInfo.CardManufacturerID=RecvData[7];
		m_BcasCardInfo.CardVersion=RecvData[8];
		m_BcasCardInfo.CheckCode=(RecvData[15]<<8)|RecvData[16];
	}

	// ECM�X�e�[�^�X������
	::ZeroMemory(&m_EcmStatus, sizeof(m_EcmStatus));

	return true;
}


const BYTE * CBcasCard::GetBcasCardID(void)
{
	// Card ID ��Ԃ�
	if (!m_pCardReader) {
		SetError(BCEC_CARDNOTOPEN, NULL);
		return NULL;
	}

	ClearError();

	return m_BcasCardInfo.BcasCardID;
}


const BYTE * CBcasCard::GetInitialCbc(void)
{
	// Descrambler CBC Initial Value ��Ԃ�
	if (!m_pCardReader) {
		SetError(BCEC_CARDNOTOPEN, NULL);
		return NULL;
	}

	ClearError();

	return m_BcasCardInfo.InitialCbc;
}


const BYTE * CBcasCard::GetSystemKey(void)
{
	// Descrambling System Key ��Ԃ�
	if (!m_pCardReader) {
		SetError(BCEC_CARDNOTOPEN, NULL);
		return NULL;
	}

	ClearError();

	return m_BcasCardInfo.SystemKey;
}


#pragma intrinsic(memcpy, memset, memcmp)

const BYTE * CBcasCard::GetKsFromEcm(const BYTE *pEcmData, const DWORD dwEcmSize)
{
	static const BYTE EcmReceiveCmd[] = {0x90U, 0x34U, 0x00U, 0x00U};

	// �uECM Receive Command�v����������
	if (!m_pCardReader) {
		SetError(BCEC_CARDNOTOPEN, TEXT(""));
		return NULL;
	}

	// ECM�T�C�Y���`�F�b�N
	if (!pEcmData || (dwEcmSize < 30UL) || (dwEcmSize > 256UL)) {
		SetError(BCEC_BADARGUMENT, TEXT(""));
		return NULL;
	}

	// �L���b�V�����`�F�b�N����
	if (m_EcmStatus.dwLastEcmSize == dwEcmSize
			&& ::memcmp(m_EcmStatus.LastEcmData, pEcmData, dwEcmSize) == 0) {
		// ECM������̏ꍇ�̓L���b�V���ς�Ks��Ԃ�
		ClearError();
		return m_EcmStatus.KsData;
	}
	// ECM�f�[�^��ۑ�����
	m_EcmStatus.dwLastEcmSize = dwEcmSize;
	::CopyMemory(m_EcmStatus.LastEcmData, pEcmData, dwEcmSize);

	// �o�b�t�@����
	DWORD dwRecvSize = 0UL;
	BYTE SendData[1024];
	BYTE RecvData[1024];
	::ZeroMemory(RecvData, sizeof(RecvData));

	// �R�}���h�\�z
	::CopyMemory(SendData, EcmReceiveCmd, sizeof(EcmReceiveCmd));				// CLA, INS, P1, P2
	SendData[sizeof(EcmReceiveCmd)] = (BYTE)dwEcmSize;							// COMMAND DATA LENGTH
	::CopyMemory(&SendData[sizeof(EcmReceiveCmd) + 1], pEcmData, dwEcmSize);	// ECM
	SendData[sizeof(EcmReceiveCmd) + dwEcmSize + 1] = 0x00U;					// RESPONSE DATA LENGTH

	// �R�}���h���M
	dwRecvSize=sizeof(RecvData);
	if (!m_pCardReader->Transmit(SendData, sizeof(EcmReceiveCmd) + dwEcmSize + 2UL, RecvData, &dwRecvSize)){
		::ZeroMemory(&m_EcmStatus, sizeof(m_EcmStatus));
		SetError(BCEC_TRANSMITERROR, m_pCardReader->GetLastErrorText());
		return NULL;
	}

	// �T�C�Y�`�F�b�N
	if (dwRecvSize != 25UL) {
		::ZeroMemory(&m_EcmStatus, sizeof(m_EcmStatus));
		SetError(BCEC_TRANSMITERROR, TEXT("ECM�̃��X�|���X�T�C�Y���s���ł��B"));
		return NULL;
	}

	// ���X�|���X���
	::CopyMemory(m_EcmStatus.KsData, &RecvData[6], sizeof(m_EcmStatus.KsData));

	// ���^�[���R�[�h���
	switch (((WORD)RecvData[4] << 8) | (WORD)RecvData[5]) {
	// Purchased: Viewing
	case 0x0200U :	// Payment-deferred PPV
	case 0x0400U :	// Prepaid PPV
	case 0x0800U :	// Tier
		ClearError();
		return m_EcmStatus.KsData;
	}
	// ��L�ȊO(�����s��)

	SetError(BCEC_ECMREFUSED, TEXT("ECM���󂯕t�����܂���B"));

	return NULL;
}


const int CBcasCard::FormatCardID(LPTSTR pszText, int MaxLength) const
{
	if (pszText == NULL || MaxLength <= 0)
		return 0;

	ULONGLONG ID;

	ID = ((((ULONGLONG)(m_BcasCardInfo.BcasCardID[0] & 0x1F) << 40) |
		 ((ULONGLONG)m_BcasCardInfo.BcasCardID[1] << 32) |
		 ((ULONGLONG)m_BcasCardInfo.BcasCardID[2] << 24) |
		 ((ULONGLONG)m_BcasCardInfo.BcasCardID[3] << 16) |
		 ((ULONGLONG)m_BcasCardInfo.BcasCardID[4] << 8) |
		 (ULONGLONG)m_BcasCardInfo.BcasCardID[5]) * 100000ULL) +
		 m_BcasCardInfo.CheckCode;
	return StdUtil::snprintf(pszText, MaxLength,
			TEXT("%d%03lu %04lu %04lu %04lu %04lu"),
			m_BcasCardInfo.BcasCardID[0] >> 5,
			(DWORD)(ID / (10000ULL * 10000ULL * 10000ULL * 10000ULL)) % 10000,
			(DWORD)(ID / (10000ULL * 10000ULL * 10000ULL)) % 10000,
			(DWORD)(ID / (10000ULL * 10000ULL) % 10000ULL),
			(DWORD)(ID / 10000ULL % 10000ULL),
			(DWORD)(ID % 10000ULL));
}


const char CBcasCard::GetCardManufacturerID() const
{
	return m_BcasCardInfo.CardManufacturerID;
}


const BYTE CBcasCard::GetCardVersion() const
{
	return m_BcasCardInfo.CardVersion;
}
