// FileWriter.cpp: CFileWriter �N���X�̃C���v�������e�[�V����
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FileWriter.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////

CFileWriter::CFileWriter(IEventHandler *pEventHandler)
	: CMediaDecoder(pEventHandler)
	, m_llWriteSize(0U)
	, m_llWriteCount(0U)
{

}

CFileWriter::~CFileWriter()
{
	CloseFile();
}

void CFileWriter::Reset(void)
{

}

const DWORD CFileWriter::GetInputNum(void) const
{
	return 1UL;
}

const DWORD CFileWriter::GetOutputNum(void) const
{
	return 0UL;
}

const bool CFileWriter::InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex)
{
	if(dwInputIndex > GetInputNum())return false;

	// �t�@�C���ɏ�������
	if(m_OutFile.Write(pMediaData->GetData(), pMediaData->GetSize())){
		m_llWriteSize += pMediaData->GetSize();
		m_llWriteCount++;
		return true;
		}

	return false;
}

const bool CFileWriter::OpenFile(LPCTSTR lpszFileName)
{
	// ��U����
	CloseFile();

	// �t�@�C�����J��
	return (m_OutFile.Open(lpszFileName, CNCachedFile::CNF_WRITE | CNCachedFile::CNF_NEW))? true : false;
}

void CFileWriter::CloseFile(void)
{
	// �t�@�C�������
	m_OutFile.Close();
	
	m_llWriteSize = 0U;
	m_llWriteCount = 0U;
}

const LONGLONG CFileWriter::GetWriteSize(void) const
{
	// �������ݍς݃T�C�Y��Ԃ�
	return m_llWriteSize;
}

const LONGLONG CFileWriter::GetWriteCount(void) const
{
	// �������݉񐔂�Ԃ�
	return m_llWriteCount;
}
