// TsProgGuide.h: TS EPG�N���X�̃C���^�[�t�F�C�X
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include <vector>
#include "TsTable.h"
#include "TsDescriptor.h"


using std::vector;


/////////////////////////////////////////////////////////////////////////////
// EIT�Z�N�V�����N���X
/////////////////////////////////////////////////////////////////////////////

class CEpgItem
{
public:
	CEpgItem();
	CEpgItem(const CEpgItem &Operand);
	CEpgItem & operator = (const CEpgItem &Operand);
	
	void Reset(void);

	const WORD ParseSection(const CPsiSection *pSection, const BYTE *pHexData, const WORD wHexSize);

	const BYTE GetTableID(void) const;
	const BYTE GetSectionNo(void) const;
	const BYTE GetLastSectionNo(void) const;
	const DWORD GetServiceID(void) const;
	const DWORD GetTsID(void) const;
	const DWORD GetOnID(void) const;
	const WORD GetEventID(void) const;
	const SYSTEMTIME & GetStartTime(void) const;
	const DWORD GetDuration(void) const;
	const BYTE GetRunningStatus(void) const;
	const bool GetFreeCaMode(void) const;
	const CDescBlock * GetDescBlock(void) const;
	
	const DWORD GetEventName(LPTSTR lpszDst) const;
	const DWORD GetEventDesc(LPTSTR lpszDst) const;
	
protected:
	struct TAG_EITITEM
	{
		BYTE byTableID;			// Table ID
		BYTE bySectionNo;		// Section Number
		BYTE byLastSectionNo;	// Last Section Number
		WORD wServiceID;		// Service ID
		WORD wTsID;				// Transport Stream ID
		WORD wOnID;				// Original Network ID
		WORD wEventID;			// Event ID
		SYSTEMTIME StartTime;	// Start Time
		DWORD dwDuration;		// Duration
		BYTE byRunningStatus;	// Running Status
		bool bIsCaService;		// Free CA Mode (true: CA / false: Free)
		CDescBlock DescBlock;	// Short Event Descriptor ��
	};

	TAG_EITITEM m_EitItem;
};


/////////////////////////////////////////////////////////////////////////////
// EIT�Z�N�V�����p�[�T�N���X
/////////////////////////////////////////////////////////////////////////////

class CEitParser : public CPsiSingleTable
{
public:
	class IEitHandler
	{
	public:
		virtual void OnEpgItem(const CEitParser *pEitParser, const CEpgItem *pEpgItem) = 0;
	};

	CEitParser(IEitHandler *pEitHandler);
	CEitParser(const CEitParser &Operand);
	CEitParser & operator = (const CEitParser &Operand);

// CEitTable
	virtual void Reset(void);

protected:
	virtual void OnPsiSection(const CPsiSectionParser *pPsiSectionParser, const CPsiSection *pSection);
	virtual void OnEpgItem(const CEpgItem *pEpgItem);

	IEitHandler *m_pEitHandler;
	CEpgItem m_EpgItem;
};


/////////////////////////////////////////////////////////////////////////////
// �T�[�r�X�P��EPG�e�[�u���N���X
/////////////////////////////////////////////////////////////////////////////

class CEpgServiceTable
{
public:
	CEpgServiceTable();
	~CEpgServiceTable();

protected:

	// �l����

	WORD m_wServiceID;
};
