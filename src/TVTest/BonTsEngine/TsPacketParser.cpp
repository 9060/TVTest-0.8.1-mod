// TsPacketParser.cpp: CTsPacketParser �N���X�̃C���v�������e�[�V����
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TsPacketParser.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


#define TS_HEADSYNCBYTE		(0x47U)


//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////


CTsPacketParser::CTsPacketParser(IEventHandler *pEventHandler)
	: CMediaDecoder(pEventHandler, 1UL, 1UL)
	, m_bOutputNullPacket(false)
	, m_dwInputPacketCount(0UL)
	, m_dwOutputPacketCount(0UL)
	, m_dwErrorPacketCount(0UL)
{
	// �p�P�b�g�A�����J�E���^������������
	::FillMemory(m_abyContCounter, sizeof(m_abyContCounter), 0x10UL);
}

CTsPacketParser::~CTsPacketParser()
{
	m_EpgCap.UnInitialize();
}

void CTsPacketParser::Reset(void)
{
	// �p�P�b�g�J�E���^���N���A����
	m_dwInputPacketCount =	0UL;
	m_dwOutputPacketCount = 0UL;
	m_dwErrorPacketCount =	0UL;

	// �p�P�b�g�A�����J�E���^������������
	::FillMemory(m_abyContCounter, sizeof(m_abyContCounter), 0x10UL);

	// ��Ԃ����Z�b�g����
	m_TsPacket.ClearSize();

	CMediaDecoder::Reset();
}

const bool CTsPacketParser::InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex)
{
	if(dwInputIndex >= GetInputNum())return false;
	if(!pMediaData)return false;

	// TS�p�P�b�g����������
	SyncPacket(pMediaData->GetData(), pMediaData->GetSize());

	return true;
}

void CTsPacketParser::SetOutputNullPacket(const bool bEnable)
{
	// NULL�p�P�b�g�̏o�͗L����ݒ肷��
	m_bOutputNullPacket = bEnable;
}

const DWORD CTsPacketParser::GetInputPacketCount(void) const
{
	// ���̓p�P�b�g����Ԃ�
	return m_dwInputPacketCount;
}

const DWORD CTsPacketParser::GetOutputPacketCount(void) const
{
	// �o�̓p�P�b�g����Ԃ�
	return m_dwOutputPacketCount;
}

const DWORD CTsPacketParser::GetErrorPacketCount(void) const
{
	// �G���[�p�P�b�g����Ԃ�
	return m_dwErrorPacketCount;
}

void inline CTsPacketParser::SyncPacket(const BYTE *pData, const DWORD dwSize)
{
	// �����̕��@�͊��S�ł͂Ȃ��A���������ꂽ�ꍇ�ɑO��Ăяo�����̃f�[�^�܂ł����̂ڂ��Ă͍ē����͂ł��Ȃ�
	DWORD dwCurSize;
	DWORD dwCurPos = 0UL;

	while (dwCurPos < dwSize) {
		dwCurSize = m_TsPacket.GetSize();

		if (dwCurSize==0) {
			// �����o�C�g�҂���
			for ( ; dwCurPos < dwSize ; dwCurPos++) {
				if (pData[dwCurPos] == TS_HEADSYNCBYTE) {
					// �����o�C�g����
					m_TsPacket.AddByte(TS_HEADSYNCBYTE);
					dwCurPos++;
					break;
				}
			}
		} else if (dwCurSize == TS_PACKETSIZE) {
			// �p�P�b�g�T�C�Y���f�[�^���������

			if (pData[dwCurPos] == TS_HEADSYNCBYTE) {
				// ���̃f�[�^�͓����o�C�g
				ParsePacket();
			} else {
				// �����G���[
				m_TsPacket.ClearSize();

				// �ʒu�����ɖ߂�
				if (dwCurPos >= (TS_PACKETSIZE - 1UL))
					dwCurPos -= (TS_PACKETSIZE - 1UL);
				else
					dwCurPos = 0UL;
			}
		} else {
			// �f�[�^�҂�
			DWORD dwRemain = (TS_PACKETSIZE - dwCurSize);
			if ((dwSize - dwCurPos) >= dwRemain) {
				m_TsPacket.AddData(&pData[dwCurPos], dwRemain);
				dwCurPos += dwRemain;
			} else {
				m_TsPacket.AddData(&pData[dwCurPos], dwSize - dwCurPos);
				break;
			}
		}
	}
}

void inline CTsPacketParser::ParsePacket(void)
{
	// �p�P�b�g����͂���
	m_TsPacket.ParseHeader();

	// �p�P�b�g���`�F�b�N����
	if (m_TsPacket.CheckPacket(&m_abyContCounter[m_TsPacket.GetPID()])) {
		// ���̓J�E���g�C���N�������g
		//if(m_dwInputPacketCount < 0xFFFFFFFFUL)m_dwInputPacketCount++;
		m_dwInputPacketCount++;

		// ���̃f�R�[�_�Ƀf�[�^��n��
		WORD PID;
		if (m_bOutputNullPacket || ((PID=m_TsPacket.GetPID()) != 0x1FFFU)) {
			if (PID == 0x0000 || PID == 0x0010 || PID == 0x0011
					|| PID == 0x0012 || PID == 0x0014) {
				m_EpgCap.AddTSPacket(m_TsPacket.GetData(),m_TsPacket.GetSize());
			}
			// �o�̓J�E���g�C���N�������g
			//if(m_dwOutputPacketCount < 0xFFFFFFFFUL)m_dwOutputPacketCount++;
			m_dwOutputPacketCount++;

			OutputMedia(&m_TsPacket);
		}
	} else {
		// �G���[�J�E���g�C���N�������g
		//if(m_dwErrorPacketCount < 0xFFFFFFFFUL)m_dwErrorPacketCount++;
		m_dwErrorPacketCount++;
	}

	// �T�C�Y���N���A�����̃X�g�A�ɔ�����
	m_TsPacket.ClearSize();
}


bool CTsPacketParser::InitializeEpgDataCap(LPCTSTR pszDllFileName)
{
	return m_EpgCap.Initialize(pszDllFileName,TRUE)==NO_ERR;
}


bool CTsPacketParser::UnInitializeEpgDataCap()
{
	return m_EpgCap.UnInitialize()==NO_ERR;
}


CEpgDataInfo *CTsPacketParser::GetEpgDataInfo(WORD wSID,bool bNext)
{
	EPG_DATA_INFO Item;
	CEpgDataInfo *pInfo=NULL;

	ZeroMemory(&Item,sizeof(EPG_DATA_INFO));
	if (m_EpgCap.GetPFData(wSID,&Item,bNext)==NO_ERR)
		pInfo=new CEpgDataInfo(&Item);
	m_EpgCap.ReleasePFData(&Item);
	return pInfo;
}
