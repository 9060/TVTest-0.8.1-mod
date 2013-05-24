// MediaTee.cpp: CMediaTee �N���X�̃C���v�������e�[�V����
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MediaTee.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////

CMediaTee::CMediaTee(IEventHandler *pEventHandler)
	: CMediaDecoder(pEventHandler)
{
	
}

CMediaTee::~CMediaTee()
{

}

const DWORD CMediaTee::GetInputNum(void) const
{
	return 1UL;
}

const DWORD CMediaTee::GetOutputNum(void) const
{
	return 2UL;
}

const bool CMediaTee::InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex)
{
	if(dwInputIndex > GetInputNum())return false;

	// ���ʃf�R�[�_�Ƀf�[�^��n��
	for(DWORD dwOutIndex = 0UL ; dwOutIndex < GetOutputNum() ; dwOutIndex++){
		OutputMedia(pMediaData, dwOutIndex);
		}

	return true;
}
