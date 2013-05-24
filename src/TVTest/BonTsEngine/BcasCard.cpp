// BcasCard.cpp: CBcasCard �N���X�̃C���v�������e�[�V����
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BcasCard.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif




CBcasCard::CBcasCard()
	: m_pCardReader(NULL)
	, m_dwLastError(BCEC_NOERROR)
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

	m_pCardReader=CCardReader::CreateCardReader(ReaderType);
	if (m_pCardReader==NULL) {
		m_dwLastError = BCEC_CARDOPENERROR;
		return false;
	}
	if (!m_pCardReader->Open(lpszReader)) {
		CloseCard();
		m_dwLastError = BCEC_CARDOPENERROR;
		return false;
	}

	// �J�[�h������
	if (!InitialSetting())
		return false;

	m_dwLastError = BCEC_NOERROR;

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
	if (m_pCardReader==NULL)
		return false;
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


const DWORD CBcasCard::GetLastError(void) const
{
	// �Ō�ɔ��������G���[��Ԃ�
	return m_dwLastError;
}


const bool CBcasCard::InitialSetting(void)
{
	// �uInitial Setting Conditions Command�v����������
	/*
	if (!m_pCardReader) {
		m_dwLastError = BCEC_CARDNOTOPEN;
		return false;
	}
	*/

	static const BYTE InitSettingCmd[] = {0x90U, 0x30U, 0x00U, 0x00U, 0x00U};

	// �o�b�t�@����
	DWORD dwRecvSize;
	BYTE RecvData[1024];
	::ZeroMemory(RecvData, sizeof(RecvData));

	// �R�}���h���M
	dwRecvSize=sizeof(RecvData);
	if (!m_pCardReader->Transmit(InitSettingCmd, sizeof(InitSettingCmd), RecvData, &dwRecvSize)) {
		m_dwLastError = BCEC_TRANSMITERROR;
		return false;
	}

	if (dwRecvSize < 57UL) {
		m_dwLastError = BCEC_TRANSMITERROR;
		return false;
	}

	// ���X�|���X���
	::CopyMemory(m_BcasCardInfo.BcasCardID, &RecvData[8], 6UL);		// +8	Card ID
	::CopyMemory(m_BcasCardInfo.SystemKey, &RecvData[16], 32UL);	// +16	Descrambling system key
	::CopyMemory(m_BcasCardInfo.InitialCbc, &RecvData[48], 8UL);	// +48	Descrambler CBC initial value

	// ECM�X�e�[�^�X������
	::ZeroMemory(&m_EcmStatus, sizeof(m_EcmStatus));

	return true;
}


const BYTE * CBcasCard::GetBcasCardID(void)
{
	// Card ID ��Ԃ�
	if( !m_pCardReader){
		m_dwLastError = BCEC_CARDNOTOPEN;
		return NULL;
	}

	m_dwLastError = BCEC_NOERROR;

	return m_BcasCardInfo.BcasCardID;
}


const BYTE * CBcasCard::GetInitialCbc(void)
{
	// Descrambler CBC Initial Value ��Ԃ�
	/*
	if(!m_pCardReader){
		m_dwLastError = BCEC_CARDNOTOPEN;
		return NULL;
	}
	*/

	m_dwLastError = BCEC_NOERROR;

	return m_BcasCardInfo.InitialCbc;
}


const BYTE * CBcasCard::GetSystemKey(void)
{
	// Descrambling System Key ��Ԃ�
	/*
	if (!m_pCardReader) {
		m_dwLastError = BCEC_CARDNOTOPEN;
		return NULL;
	}
	*/

	m_dwLastError = BCEC_NOERROR;

	return m_BcasCardInfo.SystemKey;
}


const BYTE * CBcasCard::GetKsFromEcm(const BYTE *pEcmData, const DWORD dwEcmSize)
{
	static const BYTE EcmReceiveCmd[] = {0x90U, 0x34U, 0x00U, 0x00U};

	// �uECM Receive Command�v����������
	if (!m_pCardReader) {
		m_dwLastError = BCEC_CARDNOTOPEN;
		return NULL;
	}

	// ECM�T�C�Y���`�F�b�N
	if (!pEcmData || (dwEcmSize < 30UL) || (dwEcmSize > 256UL)) {
		m_dwLastError = BCEC_BADARGUMENT;
		return NULL;
	}

	// �L���b�V�����`�F�b�N����
	if (!StoreEcmData(pEcmData, dwEcmSize)) {
		// ECM������̏ꍇ�̓L���b�V���ς�Ks��Ԃ�
		m_dwLastError = BCEC_NOERROR;
		return m_EcmStatus.KsData;
	}

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
		m_dwLastError = BCEC_TRANSMITERROR;
		return NULL;
	}

	// �T�C�Y�`�F�b�N
	if (dwRecvSize != 25UL) {
		::ZeroMemory(&m_EcmStatus, sizeof(m_EcmStatus));
		m_dwLastError = BCEC_TRANSMITERROR;
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
		m_dwLastError = BCEC_NOERROR;
		return m_EcmStatus.KsData;
	
	// ��L�ȊO(�����s��)
	default :
		m_dwLastError = BCEC_ECMREFUSED;
		return NULL;
	}
}


const bool CBcasCard::StoreEcmData(const BYTE *pEcmData, const DWORD dwEcmSize)
{
	bool bUpdate = false;

	// ECM�f�[�^��r
	if (m_EcmStatus.dwLastEcmSize != dwEcmSize) {
		// �T�C�Y���ω�����
		bUpdate = true;
	} else {
		// �T�C�Y�������ꍇ�̓f�[�^���`�F�b�N����
		for (DWORD dwPos = 0UL ; dwPos < dwEcmSize ; dwPos++) {
			if (pEcmData[dwPos] != m_EcmStatus.LastEcmData[dwPos]) {
				// �f�[�^���s��v
				bUpdate = true;
				break;
			}
		}
	}

	// ECM�f�[�^��ۑ�����
	if (bUpdate) {
		m_EcmStatus.dwLastEcmSize = dwEcmSize;
		::CopyMemory(m_EcmStatus.LastEcmData, pEcmData, dwEcmSize);
	}

	return bUpdate;
}
