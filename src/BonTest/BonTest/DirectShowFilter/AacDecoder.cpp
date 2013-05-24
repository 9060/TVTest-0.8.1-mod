// AacDecoder.cpp: CAacDecoder �N���X�̃C���v�������e�[�V����
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AacDecoder.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// "Code from FAAD2 is copyright (c) Nero AG, www.nero.com"
#pragma comment(lib, "LibFaad.lib")


//////////////////////////////////////////////////////////////////////
// �\�z/����
//////////////////////////////////////////////////////////////////////

CAacDecoder::CAacDecoder(IPcmHandler *pPcmHandler)
	: m_pPcmHandler(pPcmHandler)
	, m_hDecoder(NULL)
	, m_InitRequest(false)
	, m_byLastChannelConfig(0U)
{

}

CAacDecoder::~CAacDecoder()
{
	CloseDecoder();
}

const bool CAacDecoder::OpenDecoder(void)
{
	CloseDecoder();

	// FAAD2�I�[�v��
	if(!(m_hDecoder = ::faacDecOpen()))return false;

	// �f�t�H���g�ݒ�擾
	faacDecConfigurationPtr pDecodeConfig = ::faacDecGetCurrentConfiguration(m_hDecoder);
	
	if(!pDecodeConfig){
		CloseDecoder();
		return false;
		}

	// �f�R�[�_�ݒ�
	pDecodeConfig->defSampleRate = 48000UL;
	pDecodeConfig->outputFormat = FAAD_FMT_16BIT;

	if(!::faacDecSetConfiguration(m_hDecoder, pDecodeConfig)){
		CloseDecoder();
		return false;
		}
	
	m_InitRequest = true;
	m_byLastChannelConfig = 0xFFU;

	return true;
}

void CAacDecoder::CloseDecoder()
{
	// FAAD2�N���[�Y
	if(m_hDecoder){
		::faacDecClose(m_hDecoder);
		m_hDecoder = NULL;
		}
}

const bool CAacDecoder::ResetDecoder(void)
{
	if(!m_hDecoder)return false;
	
	// FAAD2�N���[�Y
	::faacDecClose(m_hDecoder);

	// FAAD2�I�[�v��
	if(!(m_hDecoder = ::faacDecOpen()))return false;

	// �f�t�H���g�ݒ�擾
	faacDecConfigurationPtr pDecodeConfig = ::faacDecGetCurrentConfiguration(m_hDecoder);
	
	if(!pDecodeConfig){
		CloseDecoder();
		return false;
		}

	// �f�R�[�_�ݒ�
	pDecodeConfig->defSampleRate = 48000UL;
	pDecodeConfig->outputFormat = FAAD_FMT_16BIT;

	if(!::faacDecSetConfiguration(m_hDecoder, pDecodeConfig)){
		CloseDecoder();
		return false;
		}
	
	m_InitRequest = true;
	m_byLastChannelConfig = 0xFFU;

	return true;
}

const bool CAacDecoder::Decode(const CAdtsFrame *pFrame)
{
	if(!m_hDecoder)return false;

	// �f�R�[�h
	DWORD dwSamples = 0UL;
	BYTE byChannels = 0U;
	
	// �`�����l���ݒ���
	if(pFrame->GetChannelConfig() != m_byLastChannelConfig){
		// �`�����l���ݒ肪�ω������A�f�R�[�_���Z�b�g
		ResetDecoder();
		m_byLastChannelConfig = pFrame->GetChannelConfig();
		}	
	
	// ����t���[�����
	if(m_InitRequest){
		if(::faacDecInit(m_hDecoder, pFrame->GetData(), pFrame->GetSize(), &dwSamples, &byChannels) < 0){
			return false;
			}
		
		m_InitRequest = false;
		}
	
	// �f�R�[�h
	faacDecFrameInfo FrameInfo;
	::ZeroMemory(&FrameInfo, sizeof(FrameInfo));
	
	BYTE *pPcmBuffer = (BYTE *)::faacDecDecode(m_hDecoder, &FrameInfo, pFrame->GetData(), pFrame->GetSize());

	if((!FrameInfo.error) && (FrameInfo.samples > 0L)){
		// L-PCM�n���h���ɒʒm
		if(m_pPcmHandler)m_pPcmHandler->OnPcmFrame(this, pPcmBuffer, FrameInfo.samples / (DWORD)FrameInfo.channels, FrameInfo.channels);
		}
	else{
		// �G���[����
		m_InitRequest = true;
		}

	return true;
}

void CAacDecoder::OnAdtsFrame(const CAdtsParser *pAdtsParser, const CAdtsFrame *pFrame)
{
	// CAdtsParser::IFrameHandler�̎���
	Decode(pFrame);
}
