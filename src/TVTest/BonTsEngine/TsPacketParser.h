// TsPacketParser.h: CTsPacketParser �N���X�̃C���^�[�t�F�C�X
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include "MediaDecoder.h"
#include "TsStream.h"
#include "Epg.h"


/////////////////////////////////////////////////////////////////////////////
// TS�p�P�b�g���o�f�R�[�_(�o�C�i���f�[�^����TS�p�P�b�g�𒊏o����)
/////////////////////////////////////////////////////////////////////////////
// Input	#0	: CMediaData	TS�p�P�b�g���܂ރo�C�i���f�[�^
// Output	#0	: CTsPacket		TS�p�P�b�g
/////////////////////////////////////////////////////////////////////////////

class CTsPacketParser : public CMediaDecoder  
{
public:
	CTsPacketParser(IEventHandler *pEventHandler = NULL);
	virtual ~CTsPacketParser();

// IMediaDecoder
	virtual void Reset(void);
	virtual const bool InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex = 0UL);

// CTsPacketParser
	void SetOutputNullPacket(const bool bEnable = true);
	const DWORD GetInputPacketCount(void) const;
	const DWORD GetOutputPacketCount(void) const;
	const DWORD GetErrorPacketCount(void) const;
	const DWORD GetContinuityErrorPacketCount(void) const;
	void ResetErrorPacketCount(void);

	// Append by HDUSTest�̒��̐l
	bool InitializeEpgDataCap(LPCTSTR pszDllFileName);
	bool UnInitializeEpgDataCap();
	CEpgDataInfo *GetEpgDataInfo(WORD wSID,bool bNext);
	CEpgDataCapDllUtil *GetEpgDataCapDllUtil() { return &m_EpgCap; }
private:
	void inline SyncPacket(const BYTE *pData, const DWORD dwSize);
	bool inline ParsePacket(void);

	CTsPacket m_TsPacket;

	bool m_bOutputNullPacket;

	ULONGLONG m_InputPacketCount;
	ULONGLONG m_OutputPacketCount;
	ULONGLONG m_ErrorPacketCount;
	ULONGLONG m_ContinuityErrorPacketCount;
	BYTE m_abyContCounter[0x1FFF];

	// Append by HDUSTest�̒��̐l
	CEpgDataCapDllUtil m_EpgCap;
};
