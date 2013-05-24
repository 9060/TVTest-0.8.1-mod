// FileWriter.h: CFileWriter �N���X�̃C���^�[�t�F�C�X
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include "MediaDecoder.h"
#include "NCachedFile.h"


/////////////////////////////////////////////////////////////////////////////
// �ėp�t�@�C���o��(CMediaData�����̂܂܃t�@�C���ɏ����o��)
/////////////////////////////////////////////////////////////////////////////
// Input	#0	: CMediaData		�������݃f�[�^
/////////////////////////////////////////////////////////////////////////////

class CFileWriter : public CMediaDecoder  
{
public:
	CFileWriter(IEventHandler *pEventHandler = NULL);
	virtual ~CFileWriter();

// IMediaDecoder
	virtual void Reset(void);

	virtual const DWORD GetInputNum(void) const;
	virtual const DWORD GetOutputNum(void) const;

	virtual const bool InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex = 0UL);

// CFileWriter
	const bool OpenFile(LPCTSTR lpszFileName);
	void CloseFile(void);

	const LONGLONG GetWriteSize(void) const;
	const LONGLONG GetWriteCount(void) const;

protected:
	CNCachedFile m_OutFile;
	
	LONGLONG m_llWriteSize;
	LONGLONG m_llWriteCount;
};
