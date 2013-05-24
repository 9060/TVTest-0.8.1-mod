// AacDecoder.h: CAacDecoder �N���X�̃C���^�[�t�F�C�X
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include "neaacdec.h"
#include "TsMedia.h"


// FAAD2 AAC�f�R�[�_���b�p�[�N���X 
//
// "Code from FAAD2 is copyright (c) Nero AG, www.nero.com"
//


class CAacDecoder :	public CAdtsParser::IFrameHandler
{
public:
	class IPcmHandler
	{
	public:
		virtual void OnPcmFrame(const CAacDecoder *pAacDecoder, const BYTE *pData, const DWORD dwSamples, const BYTE byChannel) = 0;
	};

	CAacDecoder(IPcmHandler *pPcmHandler);
	virtual ~CAacDecoder();

	const bool OpenDecoder(void);
	void CloseDecoder(void);

	const bool ResetDecoder(void);
	const bool Decode(const CAdtsFrame *pFrame);

protected:
// CAdtsParser::IFrameHandler
	virtual void OnAdtsFrame(const CAdtsParser *pAdtsParser, const CAdtsFrame *pFrame);

private:
	IPcmHandler *m_pPcmHandler;

	NeAACDecHandle m_hDecoder;
	bool m_bInitRequest;
	BYTE m_byLastChannelConfig;
};
