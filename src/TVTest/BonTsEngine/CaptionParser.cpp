#include "stdafx.h"
#include "CaptionParser.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


#ifdef _DEBUG
// �����f�[�^���f�o�b�O�o�͂���
//#define TRACE_CAPTION_DATA
#endif


CCaptionParser::CCaptionParser()
	: m_PesParser(this)
	, m_pHandler(NULL)
	, m_DataGroupVersion(0xFF)
{
}


CCaptionParser::~CCaptionParser()
{
}


void CCaptionParser::Reset()
{
	m_PesParser.Reset();
	m_LanguageList.clear();
	m_DataGroupVersion = 0xFF;
}


bool CCaptionParser::StorePacket(const CTsPacket *pPacket)
{
	return m_PesParser.StorePacket(pPacket);
}


void CCaptionParser::SetCaptionHandler(ICaptionHandler *pHandler)
{
	m_pHandler = pHandler;
}


int CCaptionParser::GetLanguageNum() const
{
	return (int)m_LanguageList.size();
}


bool CCaptionParser::GetLanguageCode(int LanguageTag, char *pCode) const
{
	if (pCode == NULL)
		return false;

	int Index = GetLanguageIndex(LanguageTag);
	if (Index < 0)
		return false;

	::CopyMemory(pCode, m_LanguageList[Index].LanguageCode, 4);
	return true;
}


void CCaptionParser::OnPesPacket(const CPesParser *pPesParser, const CPesPacket *pPacket)
{
	const BYTE *pData = pPacket->GetPayloadData();
	const DWORD DataSize = pPacket->GetPayloadSize();

	if (pData == NULL || DataSize < 3)
		return;

	if (pData[0] != 0x80 && pData[0] != 0x81)	// data_identifier
		return;
	if (pData[1] != 0xFF)	// private_stream_id
		return;

	const BYTE HeaderLength = pData[2] & 0x0F;	// PES_data_packet_header_length
	if (3UL + HeaderLength + 5UL >= DataSize)
		return;

	DWORD Pos = 3 + HeaderLength;

	const BYTE DataGroupID = pData[Pos] >> 2;
	const BYTE DataGroupVersion = pData[Pos] & 0x03;
	/*
	const BYTE DataGroupLinkNumber = pData[Pos + 1];
	const BYTE LastDataGroupLinkNumber = pData[Pos + 2];
	*/
	const WORD DataGroupSize = ((WORD)pData[Pos + 3] << 8) | (WORD)pData[Pos + 4];
	Pos += 5;
	if (Pos + DataGroupSize > DataSize)
		return;

	if (m_DataGroupVersion != DataGroupVersion) {
		m_LanguageList.clear();
		m_DataGroupVersion = DataGroupVersion;
	}

	m_DataGroupID = DataGroupID;

	if (DataGroupID == 0x00 || DataGroupID == 0x20) {
		// �����Ǘ��f�[�^
		ParseManagementData(&pData[Pos], DataGroupSize);
	} else {
		// �����f�[�^
		ParseCaptionData(&pData[Pos], DataGroupSize);
	}
}


bool CCaptionParser::ParseManagementData(const BYTE *pData, const DWORD DataSize)
{
	if (DataSize < 2 + 5 + 3)
		return false;

	DWORD Pos = 0;
	const BYTE TMD = pData[Pos++] >> 6;
	if (TMD == 0x02) {
		// OTM
		Pos += 5;
	}
	const BYTE NumLanguages = pData[Pos++];
	if (Pos + NumLanguages * 5 + 3 > DataSize)
		return false;
	bool fChanged = false;
	for (BYTE i = 0; i < NumLanguages; i++) {
		LanguageInfo LangInfo;

		LangInfo.LanguageTag = pData[Pos] >> 5;
		LangInfo.DMF = pData[Pos] & 0x0F;
		if (LangInfo.DMF == 0x0C || LangInfo.DMF == 0x0D) {
			LangInfo.DC = pData[Pos + 1];
			Pos++;
		}
		LangInfo.LanguageCode[0] = pData[Pos + 1];
		LangInfo.LanguageCode[1] = pData[Pos + 2];
		LangInfo.LanguageCode[2] = pData[Pos + 3];
		LangInfo.LanguageCode[3] = '\0';
		LangInfo.Format = pData[Pos + 4] >> 4;
		LangInfo.TCS = (pData[Pos + 4] & 0x0C) >> 2;
		LangInfo.RollupMode = pData[Pos + 4] & 0x03;

		int Index = GetLanguageIndex(LangInfo.LanguageTag);
		if (Index < 0) {
			m_LanguageList.push_back(LangInfo);
			fChanged  = true;
		} else {
			if (m_LanguageList[Index] != LangInfo) {
				m_LanguageList[Index] = LangInfo;
				fChanged  = true;
			}
		}

		Pos += 5;
	}
	if (fChanged && m_pHandler != NULL)
		m_pHandler->OnLanguageUpdate(this);

	const DWORD UnitLoopLength = ((DWORD)pData[Pos] << 16) | ((DWORD)pData[Pos + 1] << 8) | (DWORD)pData[Pos + 2];
	Pos += 3;
	if (UnitLoopLength > 0 && Pos + UnitLoopLength <= DataSize) {
		DWORD ReadSize = 0;
		do {
			DWORD Size = UnitLoopLength - ReadSize;
			if (!ParseUnitData(&pData[Pos + ReadSize], &Size))
				return false;
			ReadSize += Size;
		} while (ReadSize < UnitLoopLength);
	}
	return true;
}


bool CCaptionParser::ParseCaptionData(const BYTE *pData, const DWORD DataSize)
{
	if (DataSize <= 1 + 3)
		return false;

	DWORD Pos = 0;
	const BYTE TMD = pData[Pos++] >> 6;
	if (TMD == 0x01 || TMD == 0x02) {
		// STM
		if (Pos + 5 + 3 >= DataSize)
			return false;
		Pos+=5;
	}
	const DWORD UnitLoopLength = ((DWORD)pData[Pos] << 16) | ((DWORD)pData[Pos + 1] << 8) | (DWORD)pData[Pos + 2];
	Pos += 3;
	if (UnitLoopLength > 0 && Pos + UnitLoopLength <= DataSize) {
		DWORD ReadSize = 0;
		do {
			DWORD Size = UnitLoopLength - ReadSize;
			if (!ParseUnitData(&pData[Pos + ReadSize], &Size))
				return false;
			ReadSize += Size;
		} while (ReadSize < UnitLoopLength);
	}
	return true;
}


bool CCaptionParser::ParseUnitData(const BYTE *pData, DWORD *pDataSize)
{
	if (pData == NULL || *pDataSize < 5)
		return false;
	if (pData[0] != 0x1F)	// unit_separator
		return false;
	const DWORD UnitSize = ((DWORD)pData[2] << 16) | ((DWORD)pData[3] << 8) | (DWORD)pData[4];
	if (5 + UnitSize > *pDataSize)
		return false;
	const BYTE DataUnitParameter = pData[1];
	if (DataUnitParameter != 0x20) {
		*pDataSize = 5 + UnitSize;
		return true;
	}
	if (UnitSize > 0 && m_pHandler) {
		TCHAR szText[2048];
		CAribString::FormatList FormatList;

		if (CAribString::CaptionToString(szText, sizeof(szText) / sizeof(TCHAR), &pData[5], UnitSize, &FormatList) > 0) {
#ifdef TRACE_CAPTION_DATA
			TCHAR szTrace[4096];
			int Len = ::wsprintf(szTrace, TEXT("Caption %d : "), m_DataGroupID & 0x0F);
			for (DWORD i = 0; i < UnitSize; i++)
				Len += ::wsprintf(szTrace + Len, TEXT("%02x "), pData[5+i]);
			::wsprintf(szTrace + Len, TEXT("\n"));
			TRACE(szTrace);
#endif
			OnCaption(szText, &FormatList);
		}
	}
	*pDataSize = 5 + UnitSize;
	return true;
}


int CCaptionParser::GetLanguageIndex(const BYTE LanguageTag) const
{
	for (int i = 0; i < (int)m_LanguageList.size(); i++) {
		if (m_LanguageList[i].LanguageTag == LanguageTag)
			return i;
	}
	return -1;
}


void CCaptionParser::OnCaption(LPCTSTR pszText, const CAribString::FormatList *pFormatList)
{
#ifdef TRACE_CAPTION_DATA
	TRACE(TEXT("Caption %d : %s\n"), m_DataGroupID & 0x0F, pszText);
#endif
	if (m_pHandler)
		m_pHandler->OnCaption(this, (m_DataGroupID & 0x0F) - 1, pszText, pFormatList);
}




CCaptionParser::ICaptionHandler::~ICaptionHandler()
{
}
