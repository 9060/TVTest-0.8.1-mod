// AacConverter.h: CAacConverter �N���X�̃C���^�[�t�F�C�X
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include "MediaDecoder.h"
#include "AacDecoder.h"


/////////////////////////////////////////////////////////////////////////////
// AAC�f�R�[�_(AAC��PCM�Ƀf�R�[�h����)
/////////////////////////////////////////////////////////////////////////////
// Input	#0	: CAdtsFrame		���̓f�[�^
// Output	#0	: CMediaData		�o�̓f�[�^
/////////////////////////////////////////////////////////////////////////////

class CAacConverter :	public CMediaDecoder,
						protected CAacDecoder::IPcmHandler
{
public:
	CAacConverter(IEventHandler *pEventHandler = NULL);
	virtual ~CAacConverter();

// IMediaDecoder
	virtual void Reset(void);
	virtual const DWORD GetInputNum(void) const;
	virtual const DWORD GetOutputNum(void) const;
	virtual const bool InputMedia(CMediaData *pMediaData, const DWORD dwInputIndex = 0UL);

// CAacConverter
	const BYTE GetLastChannelNum(void) const;

protected:
	virtual void OnPcmFrame(const CAacDecoder *pAacDecoder, const BYTE *pData, const DWORD dwSamples, const BYTE byChannel);

	CAacDecoder m_AacDecoder;
	CMediaData m_PcmBuffer;

	BYTE m_byLastChannelNum;

private:
	static const DWORD DownMixMono(short *pDst, const short *pSrc, const DWORD dwSamples);
	static const DWORD DownMixStreao(short *pDst, const short *pSrc, const DWORD dwSamples);
	static const DWORD DownMixSurround(short *pDst, const short *pSrc, const DWORD dwSamples);
};
