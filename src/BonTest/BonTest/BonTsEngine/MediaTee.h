// MediaTee.h: CMediaTee �N���X�̃C���^�[�t�F�C�X
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include "MediaDecoder.h"


/////////////////////////////////////////////////////////////////////////////
// ���f�B�A�e�B�[(�O���t��ʉ߂���CMediaData�𕪊򂳂���)
/////////////////////////////////////////////////////////////////////////////
// Input	#0	: CMediaData		���̓f�[�^
// Output	#0	: CMediaData		�o�̓f�[�^
// Output	#1	: CMediaData		�o�̓f�[�^
/////////////////////////////////////////////////////////////////////////////

class CMediaTee : public CMediaDecoder  
{
public:
	CMediaTee(IEventHandler *pEventHandler = NULL);
	virtual ~CMediaTee();

// IMediaDecoder
	virtual const DWORD GetInputNum(void) const;
	virtual const DWORD GetOutputNum(void) const;

	virtual const bool InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex = 0UL);
};
