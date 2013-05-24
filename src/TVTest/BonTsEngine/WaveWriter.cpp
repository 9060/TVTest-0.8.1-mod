// WaveWriter.cpp: CWaveWriter �N���X�̃C���v�������e�[�V����
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WaveWriter.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////

CWaveWriter::CWaveWriter(IEventHandler *pEventHandler)
	: CFileWriter(pEventHandler)
{

}

CWaveWriter::~CWaveWriter()
{
	CloseFile();
}

const bool CWaveWriter::InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex)
{
	if(dwInputIndex > GetInputNum())return false;

	// 4GB�̐����`�F�b�N
	if((m_llWriteSize + (ULONGLONG)pMediaData->GetSize()) >= 0xFFFFFFFFULL)return true;

	// �t�@�C����������
	return CFileWriter::InputMedia(pMediaData, dwInputIndex);
}

const bool CWaveWriter::OpenFile(LPCTSTR lpszFileName)
{
	if(!CFileWriter::OpenFile(lpszFileName))return false;

	// RIFF�w�b�_����������
	static const BYTE WaveHead[] =
	{
		'R', 'I', 'F', 'F',				// +0	RIFF
		0x00U, 0x00U, 0x00U, 0x00U,		// +4	����ȍ~�̃t�@�C���T�C�Y(�t�@�C���T�C�Y - 8)
		'W', 'A', 'V', 'E',				// +8	WAVE
		'f', 'm', 't', ' ',				// +12	fmt
		0x10U, 0x00U, 0x00U, 0x00U,		// +16	fmt �`�����N�̃o�C�g��
		0x01U, 0x00U,					// +18	�t�H�[�}�b�gID
		0x02U, 0x00U,					// +20	�X�e���I
		0x80U, 0xBBU, 0x00U, 0x00U,		// +24	48KHz
		0x00U, 0xEEU, 0x02U, 0x00U,		// +28	192000Byte/s
		0x04U, 0x00U,					// +30	�u���b�N�T�C�Y
		0x10U, 0x00U,					// +32	�T���v��������̃r�b�g��
		'd', 'a', 't', 'a',				// +36	data
		0x00U, 0x00U, 0x00U, 0x00U		// +40	�g�`�f�[�^�̃o�C�g��
	};

	return m_OutFile.Write(WaveHead, sizeof(WaveHead));
}

void CWaveWriter::CloseFile(void)
{
	// RIFF�w�b�_�ɃT�C�Y����������
	DWORD dwLength = (DWORD)m_llWriteSize;
	m_OutFile.Write((BYTE *)&dwLength, 4UL, 40ULL);
	dwLength = (dwLength > 36)? (dwLength - 36UL) : 0UL;
	m_OutFile.Write((BYTE *)&dwLength, 4UL, 4ULL);		

	// �t�@�C�������
	CFileWriter::CloseFile();
}
