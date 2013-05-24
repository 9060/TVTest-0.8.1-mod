// BcasCard.h: CBcasCard �N���X�̃C���^�[�t�F�C�X
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include "CardReader.h"


// �G���[�R�[�h
#define BCEC_NOERROR			0x00000000UL	// �G���[�Ȃ�
#define BCEC_INTERNALERROR		0x00000001UL	// �����G���[
#define BCEC_NOTESTABLISHED		0x00000002UL	// �R���e�L�X�g�m�����s
#define BCEC_NOCARDREADERS		0x00000003UL	// �J�[�h���[�_���Ȃ�
#define BCEC_ALREADYOPEN		0x00000004UL	// ���ɃI�[�v���ς�
#define BCEC_CARDOPENERROR		0x00000005UL	// �J�[�h�I�[�v�����s
#define BCEC_CARDNOTOPEN		0x00000006UL	// �J�[�h���I�[�v��
#define BCEC_TRANSMITERROR		0x00000007UL	// �ʐM�G���[
#define BCEC_BADARGUMENT		0x00000008UL	// �������s��
#define BCEC_ECMREFUSED			0x00000009UL	// ECM��t����


class CBcasCard
{
public:
	CBcasCard();
	~CBcasCard();

	const DWORD GetCardReaderNum(void) const;
	LPCTSTR EnumCardReader(const DWORD dwIndex) const;

	const bool OpenCard(CCardReader::ReaderType ReaderType = CCardReader::READER_SCARD ,LPCTSTR lpszReader = NULL);
	void CloseCard(void);
	const bool ReOpenCard();
	const bool IsCardOpen() const;
	LPCTSTR GetCardReaderName() const;

	const BYTE * GetBcasCardID(void);
	const BYTE * GetInitialCbc(void);
	const BYTE * GetSystemKey(void);
	const BYTE * GetKsFromEcm(const BYTE *pEcmData, const DWORD dwEcmSize);
	const int FormatCardID(LPTSTR pszText, int MaxLength) const;
	const char GetCardManufacturerID() const;
	const BYTE GetCardVersion() const;

	const DWORD GetLastError(void) const;

protected:
	const bool InitialSetting(void);

	CCardReader *m_pCardReader;

	struct TAG_BCASCARDINFO
	{
		BYTE BcasCardID[6];			// Card ID
		BYTE SystemKey[32];			// Descrambling system key
		BYTE InitialCbc[8];			// Descrambler CBC initial value
		BYTE CardManufacturerID;	// Manufacturer identifier
		BYTE CardVersion;			// Version
		WORD CheckCode;				// Check code
	} m_BcasCardInfo;

	struct TAG_ECMSTATUS
	{
		DWORD dwLastEcmSize;	// �Ō�ɖ₢���킹�̂�����ECM�T�C�Y
		BYTE LastEcmData[256];	// �Ō�ɖ₢���킹�̂�����ECM�f�[�^
		BYTE KsData[16];		// Ks Odd + Even	
	} m_EcmStatus;

	DWORD m_dwLastError;
};
