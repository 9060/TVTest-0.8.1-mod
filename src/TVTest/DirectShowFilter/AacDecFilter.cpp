#include "StdAfx.h"
#include "AacDecFilter.h"

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

/*
	�������Z�ɂ��_�E���~�b�N�X
	�e�X�g�ł͏������Ԃ�1/8���炢�ɂȂ���(������SSE���ɂ���ĕς��͂�)
*/
#define DOWNMIX_INT

#ifdef DOWNMIX_INT
#define IDOWNMIX_DENOM	4096
#define IDOWNMIX_CENTER	((int)(DMR_CENTER*IDOWNMIX_DENOM+0.5))
#define IDOWNMIX_FRONT	((int)(DMR_FRONT*IDOWNMIX_DENOM+0.5))
#define IDOWNMIX_REAR	((int)(DMR_REAR*IDOWNMIX_DENOM+0.5))
#define IDOWNMIX_LFE	((int)(DMR_LFE*IDOWNMIX_DENOM+0.5))
#endif




CAacDecFilter::CAacDecFilter(LPUNKNOWN pUnk, HRESULT *phr)
	: CTransformFilter(AACDECFILTER_NAME, pUnk, CLSID_AACDECFILTER)
	, m_AdtsParser(&m_AacDecoder)
	, m_AacDecoder(this)
	, m_pOutSample(NULL)
	, m_byCurChannelNum(0)
	, m_bNormalize(false)
{
	TRACE(TEXT("CAacDecFilter::CAacDecFilter %p\n"),this);

	*phr=S_OK;
}

CAacDecFilter::~CAacDecFilter(void)
{
	TRACE(TEXT("CAacDecFilter::~CAacDecFilter\n"));
}

IBaseFilter* WINAPI CAacDecFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr, CAacDecFilter **ppClassIf)
{
	// �C���X�^���X���쐬����
	if(ppClassIf) *ppClassIf = NULL;
	CAacDecFilter *pNewFilter = new CAacDecFilter(pUnk, phr);
	/*
	if (!pNewFilter) {
		*phr = E_OUTOFMEMORY;
		return NULL;
	}
	*/

	IBaseFilter *pFilter;
	*phr=pNewFilter->QueryInterface(IID_IBaseFilter,(void**)&pFilter);
	if (FAILED(*phr)) {
		delete pNewFilter;
		return NULL;
	}
	if(ppClassIf) *ppClassIf = pNewFilter;
	return pFilter;
}

HRESULT CAacDecFilter::CheckInputType(const CMediaType* mtIn)
{
    CheckPointer(mtIn, E_POINTER);

	// ���ł�OK

	return S_OK;
}

HRESULT CAacDecFilter::CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut)
{
    CheckPointer(mtIn, E_POINTER);
    CheckPointer(mtOut, E_POINTER);
    
	if(*mtOut->Type() == MEDIATYPE_Audio){
		if(*mtOut->Subtype() == MEDIASUBTYPE_PCM){

			// GUID_NULL�ł̓f�o�b�O�A�T�[�g����������̂Ń_�~�[��ݒ肵�ĉ��			
			CMediaType MediaType;
			MediaType.InitMediaType();
			MediaType.SetType(&MEDIATYPE_Stream);
			MediaType.SetSubtype(&MEDIASUBTYPE_None);
			
			m_pInput->SetMediaType(&MediaType);
			
			return S_OK;
			}
		}
    
	return VFW_E_TYPE_NOT_ACCEPTED;
}

HRESULT CAacDecFilter::DecideBufferSize(IMemAllocator * pAllocator, ALLOCATOR_PROPERTIES *pprop)
{
	CAutoLock AutoLock(m_pLock);
	CheckPointer(pAllocator, E_POINTER);
	CheckPointer(pprop, E_POINTER);

	// �o�b�t�@��1����΂悢
	if(!pprop->cBuffers)pprop->cBuffers = 1L;

	// �Ƃ肠����1MB�m��
	if(pprop->cbBuffer < 0x100000L)pprop->cbBuffer = 0x100000L;

	// �A���P�[�^�v���p�e�B��ݒ肵�Ȃ���
	ALLOCATOR_PROPERTIES Actual;
	HRESULT hr = pAllocator->SetProperties(pprop, &Actual);
	if(FAILED(hr))return hr;

	// �v�����󂯓����ꂽ������
	if(Actual.cbBuffer < pprop->cbBuffer)return E_FAIL;

	return S_OK;
}

HRESULT CAacDecFilter::GetMediaType(int iPosition, CMediaType *pMediaType)
{
	CAutoLock AutoLock(m_pLock);
	CheckPointer(pMediaType, E_POINTER);

	if(iPosition < 0)return E_INVALIDARG;
	if(iPosition > 0)return VFW_S_NO_MORE_ITEMS;

	// ���f�B�A�^�C�v�ݒ�
	pMediaType->InitMediaType();
	pMediaType->SetType(&MEDIATYPE_Audio);
	pMediaType->SetSubtype(&MEDIASUBTYPE_PCM);
	pMediaType->SetTemporalCompression(FALSE);
	pMediaType->SetSampleSize(0);
	pMediaType->SetFormatType(&FORMAT_WaveFormatEx);
		
	 // �t�H�[�}�b�g�\���̊m��
	WAVEFORMATEX *pWaveInfo = reinterpret_cast<WAVEFORMATEX *>(pMediaType->AllocFormatBuffer(sizeof(WAVEFORMATEX)));
	if(!pWaveInfo)return E_OUTOFMEMORY;
	//::ZeroMemory(pWaveInfo, sizeof(WAVEFORMATEX));

	// WAVEFORMATEX�\���̐ݒ�(48KHz 16bit �X�e���I�Œ�)
	pWaveInfo->wFormatTag = WAVE_FORMAT_PCM;
	pWaveInfo->nChannels = 2U;
	pWaveInfo->nSamplesPerSec = 48000UL;
	pWaveInfo->wBitsPerSample = 16U;
	pWaveInfo->nBlockAlign = pWaveInfo->wBitsPerSample * pWaveInfo->nChannels / 8U;
	pWaveInfo->nAvgBytesPerSec = pWaveInfo->nSamplesPerSec * pWaveInfo->nBlockAlign;

	return S_OK;
}

HRESULT CAacDecFilter::StartStreaming(void)
{
	CAutoLock AutoLock(m_pLock);

	// ADTS�p�[�T������
	m_AdtsParser.Reset();

	// AAC�f�R�[�_�I�[�v��
	if(!m_AacDecoder.OpenDecoder())return E_FAIL;

	return S_OK;
}

HRESULT CAacDecFilter::StopStreaming(void)
{
	CAutoLock AutoLock(m_pLock);

	// AAC�f�R�[�_�N���[�Y
	m_AacDecoder.CloseDecoder();

	return S_OK;
}

const BYTE CAacDecFilter::GetCurrentChannelNum()
{
	return m_byCurChannelNum;
}

HRESULT CAacDecFilter::Transform(IMediaSample *pIn, IMediaSample *pOut)
{
	CAutoLock AutoLock(m_pLock);

	// ���̓f�[�^�|�C���^���擾����
	BYTE *pInData = NULL;
	pIn->GetPointer(&pInData);

	// �o�̓��f�B�A�T���v���ݒ�
	m_pOutSample = pOut;
	pOut->SetActualDataLength(0UL);

	// ADTS�p�[�T�ɓ���
	m_AdtsParser.StoreEs(pInData, pIn->GetActualDataLength());

	if(!pOut->GetActualDataLength())
		//return S_FALSE;
		return S_OK;

	// �^�C���X�^���v�ݒ�
	REFERENCE_TIME StartTime = 0LL;
	REFERENCE_TIME EndTime = 0LL;

	// �^�C���X�^���v�ݒ�
	if (pIn->GetTime(&StartTime, &EndTime) == S_OK) {
		pOut->SetTime(&StartTime, &EndTime);
	}

	// �X�g���[���^�C���ݒ�
	if (pIn->GetMediaTime(&StartTime, &EndTime) == S_OK) {
		pOut->SetMediaTime(&StartTime, &EndTime);
	}

	return S_OK;
}

/*
HRESULT CAacDecFilter::Receive(IMediaSample *pSample)
{
	IMediaSample *pOutSample;
	HRESULT hr;

	hr = InitializeOutputSample(pSample, &pOutSample);
	if (FAILED(hr)) {
		return hr;
	}
	hr = Transform(pSample, pOutSample);
	if (hr == NOERROR) {
		hr = m_pOutput->Deliver(pOutSample);
	}
	pOutSample->Release();
	return hr;
}
*/

void CAacDecFilter::OnPcmFrame(const CAacDecoder *pAacDecoder, const BYTE *pData, const DWORD dwSamples, const BYTE byChannel)
{
	CAutoLock AutoLock(m_pLock);

	// �o�̓|�C���^�擾
	const DWORD dwOffset = m_pOutSample->GetActualDataLength();
	DWORD dwOutSize = 0UL;

	BYTE *pOutBuff = NULL;
	m_pOutSample->GetPointer(&pOutBuff);
	pOutBuff = &pOutBuff[dwOffset];
	m_byCurChannelNum = byChannel;

	// �_�E���~�b�N�X
	switch(byChannel){
		case 1U:	dwOutSize = DownMixMono((short *)pOutBuff, (const short *)pData, dwSamples);		break;
		case 2U:	dwOutSize = DownMixStreao((short *)pOutBuff, (const short *)pData, dwSamples);		break;
		case 6U:	dwOutSize = DownMixSurround((short *)pOutBuff, (const short *)pData, dwSamples);	break;
		}

	if (m_bNormalize)
		Normalize((short*)pOutBuff,dwOutSize/sizeof(short));

	// ���f�B�A�T���v���L���T�C�Y�ݒ�
    m_pOutSample->SetActualDataLength(dwOffset + dwOutSize);
}

const DWORD CAacDecFilter::DownMixMono(short *pDst, const short *pSrc, const DWORD dwSamples)
{
	// 1ch �� 2ch ��d��
	for(register DWORD dwPos = 0UL ; dwPos < dwSamples ; dwPos++){
		pDst[dwPos * 2UL + 0UL] = pSrc[dwPos];	// L
		pDst[dwPos * 2UL + 1UL] = pSrc[dwPos];	// R
		}

	// �o�b�t�@�T�C�Y��Ԃ�
	return dwSamples * 4UL;
}

const DWORD CAacDecFilter::DownMixStreao(short *pDst, const short *pSrc, const DWORD dwSamples)
{
	// 2ch �� 2ch �X���[
	::CopyMemory(pDst, pSrc, dwSamples * 4UL);

	// �o�b�t�@�T�C�Y��Ԃ�
	return dwSamples * 4UL;
}

#ifndef DOWNMIX_INT

/*
	���������_�����Z��(�I���W�i��)
*/
const DWORD CAacDecFilter::DownMixSurround(short *pDst, const short *pSrc, const DWORD dwSamples)
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

#else

/*
	�������Z��
*/
const DWORD CAacDecFilter::DownMixSurround(short *pDst, const short *pSrc, const DWORD dwSamples)
{
	// 5.1ch �� 2ch �_�E���~�b�N�X

	int iOutLch, iOutRch;

	for (register DWORD dwPos = 0UL ; dwPos < dwSamples ; dwPos++) {
		// �_�E���~�b�N�X
		iOutLch =	((int)pSrc[dwPos * 6UL + 1UL] * IDOWNMIX_FRONT	+
					 (int)pSrc[dwPos * 6UL + 3UL] * IDOWNMIX_REAR	+
					 (int)pSrc[dwPos * 6UL + 0UL] * IDOWNMIX_CENTER	+
					 (int)pSrc[dwPos * 6UL + 5UL] * IDOWNMIX_LFE)/
					IDOWNMIX_DENOM;

		iOutRch =	((int)pSrc[dwPos * 6UL + 2UL] * IDOWNMIX_FRONT	+
					 (int)pSrc[dwPos * 6UL + 4UL] * IDOWNMIX_REAR	+
					 (int)pSrc[dwPos * 6UL + 0UL] * IDOWNMIX_CENTER	+
					 (int)pSrc[dwPos * 6UL + 5UL] * IDOWNMIX_LFE)/
					IDOWNMIX_DENOM;

		// �N���b�v
		if(iOutLch > 32767)iOutLch = 32767;
		else if(iOutLch < -32768L)iOutLch = -32768;

		if(iOutRch > 32767)iOutRch = 32767;
		else if(iOutRch < -32768)iOutRch = -32768;

		pDst[dwPos * 2UL + 0UL] = (short)iOutLch;	// L
		pDst[dwPos * 2UL + 1UL] = (short)iOutRch;	// R
	}

	// �o�b�t�@�T�C�Y��Ԃ�
	return dwSamples * 4UL;
}

#endif




bool CAacDecFilter::SetNormalize(bool bNormalize,float Level)
{
	m_bNormalize=bNormalize;
	m_NormalizeLevel=Level;
	return true;
}


#define NORMALIZE_SHIFT_BITS	12
#define NORMALIZE_DENOM			(1<<NORMALIZE_SHIFT_BITS)


/*
	�{���̓_�E���~�b�N�X�Ɠ����ɍs���������ǂ�
*/
void CAacDecFilter::Normalize(short *pBuffer,DWORD Samples)
{
	int Level=(int)(m_NormalizeLevel*(float)NORMALIZE_DENOM);
	short *p,*pEnd;
	int Value;

	p=pBuffer;
	pEnd=p+Samples;
	while (p<pEnd) {
		Value=(*p*Level)/NORMALIZE_DENOM;
		*p++=Value>32767?32767:Value<-32768?-32768:Value;
	}
}
