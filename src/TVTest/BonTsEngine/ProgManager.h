// ProgManager.h: CProgManager �N���X�̃C���^�[�t�F�C�X
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include <vector>
#include "MediaDecoder.h"
#include "TsStream.h"


using std::vector;


/////////////////////////////////////////////////////////////////////////////
// �ԑg���Ǘ��N���X
/////////////////////////////////////////////////////////////////////////////
// Input	#0	: CTsPacket	TS�p�P�b�g
// Output	#0	: CTsPacket	TS�p�P�b�g
/////////////////////////////////////////////////////////////////////////////

class CProgManager : public CMediaDecoder
{
public:
	enum EVENTID
	{
		EID_SERVICE_LIST_UPDATED,	// �T�[�r�X���X�g�X�V
		EID_SERVICE_INFO_UPDATED,	// �T�[�r�X���X�V
		EID_PCR_TIMESTAMP_UPDATED	// PCR�^�C���X�^���v�X�V(���Ȃ�p��)
	};

	CProgManager(IEventHandler *pEventHandler = NULL);
	virtual ~CProgManager();

// IMediaDecoder
	virtual void Reset(void);
	virtual const bool InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex = 0UL);

// CProgManager
	const WORD GetServiceNum(void);
	const bool GetServiceID(WORD *pwServiceID, const WORD wIndex = 0U);
	const bool GetVideoEsPID(WORD *pwVideoPID, const WORD wIndex = 0U);
	const bool GetAudioEsPID(WORD *pwAudioPID, const WORD wAudioIndex = 0U, const WORD wIndex = 0U);
	const WORD GetAudioEsNum(const WORD wIndex = 0U);
	const bool GetPcrTimeStamp(unsigned __int64 *pu64PcrTimeStamp, const WORD wServiceID = 0U);
	const DWORD GetServiceName(LPTSTR lpszDst, const WORD wIndex = 0U);
	const WORD GetTransportStreamID() const;

	WORD GetNetworkID(void) const;
	BYTE GetBroadcastingID(void) const;
	DWORD GetNetworkName(LPTSTR pszName,int MaxLength);
	BYTE GetRemoteControlKeyID(void) const;
	DWORD GetTSName(LPTSTR pszName,int MaxLength);

protected:
	class CProgDatabase;

	virtual void OnServiceListUpdated(void);
	virtual void OnServiceInfoUpdated(void);
	virtual void OnPcrTimestampUpdated(void);

	struct TAG_SERVICEINFO
	{
		WORD wServiceID;
		WORD wVideoEsPID;
		vector<WORD> AudioEsPIDs;
		TCHAR szServiceName[256];

		// �^�C���X�^���v
		unsigned __int64 u64TimeStamp;
	};

	vector<TAG_SERVICEINFO> m_ServiceList;

	CTsPidMapManager m_PidMapManager;
	CProgDatabase *m_pProgDatabase;
};


/////////////////////////////////////////////////////////////////////////////
// �ԑg���f�[�^�x�[�X�N���X
/////////////////////////////////////////////////////////////////////////////

class CProgManager::CProgDatabase
{
public:
	CProgDatabase(CProgManager &ProgManager);
	virtual ~CProgDatabase();

	void Reset(void);
	void UnmapTable(void);

	const WORD GetServiceIndexByID(const WORD wServiceID);

	// CProgManager�Ə�񂪃_�u���Ă���̂Ō������ׂ�
	struct TAG_SERVICEINFO
	{
		WORD wServiceID;
		WORD wVideoEsPID;
		vector<WORD> AudioEsPIDs;
		BYTE byServiceType;
		TCHAR szServiceName[256];

		bool bIsUpdated;

		// ���L�͏��Ƃ��ē��ɕs�v�H
		BYTE byVideoComponentTag;
		BYTE byAudioComponentTag;

		WORD wPmtTablePID;
		BYTE byRunningStatus;
		bool bIsCaService;

		// �^�C���X�^���v
		WORD wPcrPID;
		unsigned __int64 u64TimeStamp;
	};

	vector<TAG_SERVICEINFO> m_ServiceList;
	WORD m_wTransportStreamID;

	struct TAG_NITINFO
	{
		WORD wNetworkID;
		BYTE byBroadcastingID;
		BYTE byRemoteControlKeyID;
		TCHAR szNetworkName[32];
		TCHAR szTSName[32];
	};
	TAG_NITINFO m_NitInfo;

private:
	static void CALLBACK OnPatUpdated(const WORD wPID, CTsPidMapTarget *pMapTarget, CTsPidMapManager *pMapManager, const PVOID pParam);
	static void CALLBACK OnPmtUpdated(const WORD wPID, CTsPidMapTarget *pMapTarget, CTsPidMapManager *pMapManager, const PVOID pParam);
	static void CALLBACK OnPcrUpdated(const WORD wPID, CTsPidMapTarget *pMapTarget, CTsPidMapManager *pMapManager, const PVOID pParam);
	static void CALLBACK OnSdtUpdated(const WORD wPID, CTsPidMapTarget *pMapTarget, CTsPidMapManager *pMapManager, const PVOID pParam);	
	static void CALLBACK OnNitUpdated(const WORD wPID, CTsPidMapTarget *pMapTarget, CTsPidMapManager *pMapManager, const PVOID pParam);

	CProgManager &m_ProgManager;
	CTsPidMapManager &m_PidMapManager;
};
