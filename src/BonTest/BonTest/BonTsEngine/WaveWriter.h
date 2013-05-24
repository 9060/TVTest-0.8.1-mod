// WaveWriter.h: CWaveWriter �N���X�̃C���^�[�t�F�C�X
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include "FileWriter.h"


/////////////////////////////////////////////////////////////////////////////
// Wave�t�@�C���o��(48KHz 16bit Streo PCM�f�[�^��Wav�t�@�C���ɏ����o��)
/////////////////////////////////////////////////////////////////////////////
// Input	#0	: CMediaData		�������݃f�[�^
/////////////////////////////////////////////////////////////////////////////

class CWaveWriter : public CFileWriter  
{
public:
	CWaveWriter(IEventHandler *pEventHandler = NULL);
	virtual ~CWaveWriter();

// CFileWriter
	virtual const bool InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex = 0UL);
	const bool OpenFile(LPCTSTR lpszFileName);
	void CloseFile(void);
};
