// AacConverter.cpp: CAacConverter �N���X�̃C���v�������e�[�V����
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AacConverter.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// 5.1ch�_�E���~�b�N�X�ݒ�
#define DMR_CENTER			0.5		// 50%
#define DMR_FRONT			1.0		// 100%
#define DMR_REAR			0.7		// 70%
#define DMR_LFE				0.7		// 70%


//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////

CAacConverter::CAacConverter(IEventHandler *pEventHandler)
	: CMediaDecoder(pEventHandler, 1UL, 1UL)
	, m_AacDecoder(this)
	, m_PcmBuffer(0x00200000UL)
	, m_byLastChannelNum(0U)
{
	// AAC�f�R�[�_�I�[�v��
	m_AacDecoder.OpenDecoder();
}

CAacConverter::~CAacConverter()
{
	// // AAC�f�R�[�_�N���[�Y
	m_AacDecoder.CloseDecoder();
}

void CAacConverter::Reset(void)
{
	// ��ԃ��Z�b�g
	m_byLastChannelNum = 0U;
	m_AacDecoder.ResetDecoder();
	m_PcmBuffer.ClearSize();

	CMediaDecoder::Reset();
}

const bool CAacConverter::InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex)
{
	if(dwInputIndex > GetInputNum())return false;

	CAdtsFrame *pAdtsFrame = static_cast<CAdtsFrame *>(pMediaData);

	// ���̓��f�B�A�f�[�^�͌݊������Ȃ�
	if(!pAdtsFrame)return false;

	// AAC�f�R�[�_�ɓ���
	return m_AacDecoder.Decode(pAdtsFrame);
}

const BYTE CAacConverter::GetLastChannelNum(void) const
{
	// �ŐV�̃`�����l������Ԃ�
	return m_byLastChannelNum;
}

void CAacConverter::OnPcmFrame(const CAacDecoder *pAacDecoder, const BYTE *pData, const DWORD dwSamples, const BYTE byChannel)
{
	// �o�̓|�C���^�擾
	DWORD dwOutSize = 0UL;
	m_PcmBuffer.SetSize(dwSamples * 4UL);
	BYTE *pOutBuff = m_PcmBuffer.GetData();

	// �_�E���~�b�N�X
	switch(byChannel){
		case 1U:	dwOutSize = DownMixMono((short *)pOutBuff, (const short *)pData, dwSamples);		break;
		case 2U:	dwOutSize = DownMixStreao((short *)pOutBuff, (const short *)pData, dwSamples);		break;
		case 6U:	dwOutSize = DownMixSurround((short *)pOutBuff, (const short *)pData, dwSamples);	break;
		}

	// ���̃f�R�[�_�ɃT���v����n��
	m_byLastChannelNum = byChannel;
	m_PcmBuffer.SetSize(dwOutSize);
	OutputMedia(&m_PcmBuffer);
}

const DWORD CAacConverter::DownMixMono(short *pDst, const short *pSrc, const DWORD dwSamples)
{
	// 1ch �� 2ch ��d��
	for(register DWORD dwPos = 0UL ; dwPos < dwSamples ; dwPos++){
		pDst[dwPos * 2UL + 0UL] = pSrc[dwPos];	// L
		pDst[dwPos * 2UL + 1UL] = pSrc[dwPos];	// R
		}

	// �o�b�t�@�T�C�Y��Ԃ�
	return dwSamples * 4UL;
}

const DWORD CAacConverter::DownMixStreao(short *pDst, const short *pSrc, const DWORD dwSamples)
{
	// 2ch �� 2ch �X���[
	::CopyMemory(pDst, pSrc, dwSamples * 4UL);

	// �o�b�t�@�T�C�Y��Ԃ�
	return dwSamples * 4UL;
}

const DWORD CAacConverter::DownMixSurround(short *pDst, const short *pSrc, const DWORD dwSamples)
{
	// 5.1ch �� 2ch �_�E���~�b�N�X
	int iOutLch, iOutRch;

	for(register DWORD dwPos = 0UL ; dwPos < dwSamples ; dwPos++){
		// �_�E���~�b�N�X
		iOutLch = (int)(
					(double)pSrc[dwPos * 6UL + 1UL]	* DMR_FRONT		+
					(double)pSrc[dwPos * 6UL + 3UL]	* DMR_REAR		+
					(double)pSrc[dwPos * 6UL + 0UL]	* DMR_CENTER	+
					(double)pSrc[dwPos * 6UL + 5UL]	* DMR_LFE
					);

		iOutRch = (int)(
					(double)pSrc[dwPos * 6UL + 2UL]	* DMR_FRONT		+
					(double)pSrc[dwPos * 6UL + 4UL]	* DMR_REAR		+
					(double)pSrc[dwPos * 6UL + 0UL]	* DMR_CENTER	+
					(double)pSrc[dwPos * 6UL + 5UL]	* DMR_LFE
					);

		// �N���b�v
		if(iOutLch > 32767L)iOutLch = 32767L;
		else if(iOutLch < -32768L)iOutLch = -32768L;

		if(iOutRch > 32767L)iOutRch = 32767L;
		else if(iOutRch < -32768L)iOutRch = -32768L;

		pDst[dwPos * 2UL + 0UL] = (short)iOutLch;	// L
		pDst[dwPos * 2UL + 1UL] = (short)iOutRch;	// R
		}

	// �o�b�t�@�T�C�Y��Ԃ�
	return dwSamples * 4UL;
}
