#include "StdAfx.h"
#include <mmreg.h>
#include "AacDecFilter.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// ���g��(48kHz)
#define FREQUENCY 48000

// REFERENCE_TIME�̈�b
#define REFERENCE_TIME_SECOND 10000000LL

// 5.1ch�_�E���~�b�N�X�ݒ�
// �{���͌Œ�l����ʖڂ݂��������ǁc
#define DMR_CENTER		0.5		// 50%
#define DMR_FRONT		1.0		// 100%
#define DMR_REAR		0.7		// 70%
#define DMR_LFE			0.7		// 70%

// �������Z�ɂ��_�E���~�b�N�X
#define DOWNMIX_INT

#ifdef DOWNMIX_INT
#define IDOWNMIX_DENOM	4096
#define IDOWNMIX_CENTER	((int)(DMR_CENTER * IDOWNMIX_DENOM + 0.5))
#define IDOWNMIX_FRONT	((int)(DMR_FRONT * IDOWNMIX_DENOM + 0.5))
#define IDOWNMIX_REAR	((int)(DMR_REAR * IDOWNMIX_DENOM + 0.5))
#define IDOWNMIX_LFE	((int)(DMR_LFE * IDOWNMIX_DENOM + 0.5))
#endif


CAacDecFilter::CAacDecFilter(LPUNKNOWN pUnk, HRESULT *phr)
	: CTransformFilter(AACDECFILTER_NAME, pUnk, CLSID_AACDECFILTER)
	, m_AdtsParser(&m_AacDecoder)
	, m_AacDecoder(this)
	, m_pOutSample(NULL)
	, m_byCurChannelNum(0)
	, m_StereoMode(STEREOMODE_STEREO)
	, m_bDownMixSurround(true)
	, m_pStreamCallback(NULL)
	, m_bNormalize(false)
	, m_bAdjustStreamTime(false)
	, m_StartTime(0)
	, m_SampleCount(0)
{
	TRACE(TEXT("CAacDecFilter::CAacDecFilter %p\n"), this);

	// ���f�B�A�^�C�v�ݒ�
	m_MediaType.InitMediaType();
	m_MediaType.SetType(&MEDIATYPE_Audio);
	m_MediaType.SetSubtype(&MEDIASUBTYPE_PCM);
	m_MediaType.SetTemporalCompression(FALSE);
	m_MediaType.SetSampleSize(0);
	m_MediaType.SetFormatType(&FORMAT_WaveFormatEx);

	 // �t�H�[�}�b�g�\���̊m��
#if 1
	WAVEFORMATEX *pWaveInfo = reinterpret_cast<WAVEFORMATEX *>(m_MediaType.AllocFormatBuffer(sizeof(WAVEFORMATEX)));
	if (pWaveInfo == NULL) {
		*phr = E_OUTOFMEMORY;
		return;
	}
	// WAVEFORMATEX�\���̐ݒ�(48kHz 16bit �X�e���I�Œ�)
	pWaveInfo->wFormatTag = WAVE_FORMAT_PCM;
	pWaveInfo->nChannels = 2;
	pWaveInfo->nSamplesPerSec = FREQUENCY;
	pWaveInfo->wBitsPerSample = 16;
	pWaveInfo->nBlockAlign = pWaveInfo->wBitsPerSample * pWaveInfo->nChannels / 8;
	pWaveInfo->nAvgBytesPerSec = pWaveInfo->nSamplesPerSec * pWaveInfo->nBlockAlign;
#else
	WAVEFORMATEXTENSIBLE *pWaveInfo = reinterpret_cast<WAVEFORMATEXTENSIBLE *>(m_MediaType.AllocFormatBuffer(sizeof(WAVEFORMATEXTENSIBLE)));
	if (pWaveInfo == NULL) {
		*phr = E_OUTOFMEMORY;
		return;
	}
	// WAVEFORMATEXTENSIBLE�\���̐ݒ�(48KHz 16bit 5.1ch�Œ�)
	pWaveInfo->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	pWaveInfo->Format.nChannels = 6;
	pWaveInfo->Format.nSamplesPerSec = FREQUENCY;
	pWaveInfo->Format.wBitsPerSample = 16;
	pWaveInfo->Format.nBlockAlign = pWaveInfo->Format.wBitsPerSample * pWaveInfo->Format.nChannels / 8;
	pWaveInfo->Format.nAvgBytesPerSec = pWaveInfo->Format.nSamplesPerSec * pWaveInfo->Format.nBlockAlign;
	pWaveInfo->Format.cbSize  = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
	pWaveInfo->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT |
							   SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY |
							   SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT;
	pWaveInfo->Samples.wValidBitsPerSample = 16;
	pWaveInfo->SubFormat = MEDIASUBTYPE_PCM;
#endif

	*phr = S_OK;
}

CAacDecFilter::~CAacDecFilter(void)
{
	TRACE(TEXT("CAacDecFilter::~CAacDecFilter\n"));
}

IBaseFilter* WINAPI CAacDecFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr, CAacDecFilter **ppClassIf)
{
	// �C���X�^���X���쐬����
	CAacDecFilter *pNewFilter = new CAacDecFilter(pUnk, phr);
	/*
	if (!pNewFilter) {
		*phr = E_OUTOFMEMORY;
		goto OnError;
	}
	*/
	if (FAILED(*phr))
		goto OnError;

	IBaseFilter *pFilter;
	*phr = pNewFilter->QueryInterface(IID_IBaseFilter, (void**)&pFilter);
	if (FAILED(*phr))
		goto OnError;
	if (ppClassIf)
		*ppClassIf = pNewFilter;
	return pFilter;

OnError:
	delete pNewFilter;
	if (ppClassIf)
		*ppClassIf = NULL;
	return NULL;
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

	if (*mtOut->Type() == MEDIATYPE_Audio) {
		if (*mtOut->Subtype() == MEDIASUBTYPE_PCM) {

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

	if (iPosition < 0)
		return E_INVALIDARG;
	if (iPosition > 0)
		return VFW_S_NO_MORE_ITEMS;

	*pMediaType = m_MediaType;

	return S_OK;
}

HRESULT CAacDecFilter::StartStreaming(void)
{
	CAutoLock AutoLock(m_pLock);

	// ADTS�p�[�T������
	m_AdtsParser.Reset();

	// AAC�f�R�[�_�I�[�v��
	if(!m_AacDecoder.OpenDecoder())
		return E_FAIL;

	m_StartTime = 0;
	m_SampleCount = 0;

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

inline LONGLONG llabs(LONGLONG val)
{
	return val<0?-val:val;
}

HRESULT CAacDecFilter::Transform(IMediaSample *pIn, IMediaSample *pOut)
{
	CAutoLock AutoLock(m_pLock);

	// ���̓f�[�^�|�C���^���擾����
	BYTE *pInData;
	HRESULT hr = pIn->GetPointer(&pInData);
	if (FAILED(hr))
		return hr;

	// �o�̓��f�B�A�T���v���ݒ�
	m_pOutSample = pOut;
	pOut->SetActualDataLength(0);

	// ADTS�p�[�T�ɓ���
	m_AdtsParser.StoreEs(pInData, pIn->GetActualDataLength());

	if (pOut->GetActualDataLength() == 0)
		return S_FALSE;

#if 0	// InitializeOutputSample() �ł���Ă���̂ŕs�v
	// �^�C���X�^���v�ݒ�
	REFERENCE_TIME StartTime, EndTime;

	// �^�C���X�^���v�ݒ�
	if (pIn->GetTime(&StartTime, &EndTime) == S_OK) {
		pOut->SetTime(&StartTime, &EndTime);
	}

	// ���f�B�A�^�C���ݒ�
	if (pIn->GetMediaTime(&StartTime, &EndTime) == S_OK) {
		pOut->SetMediaTime(&StartTime, &EndTime);
	}
#endif

	if (m_bAdjustStreamTime) {
		// �X�g���[�����Ԃ����ۂ̃T���v���̒��������ɐݒ肷��
		bool bAdjusted = false;
		REFERENCE_TIME StartTime, EndTime;

		if (pIn->GetTime(&StartTime, &EndTime) == S_OK) {
			REFERENCE_TIME CurTime;

			if (m_StartTime != 0)
				CurTime = m_StartTime + (m_SampleCount * REFERENCE_TIME_SECOND / FREQUENCY);
			if (m_StartTime == 0
					|| llabs(StartTime - CurTime) <= REFERENCE_TIME_SECOND / 5LL) {
				DWORD Samples = pOut->GetActualDataLength() / 2;
				if (m_bDownMixSurround || m_byCurChannelNum == 2)
					Samples /= 2;
				else
					Samples /= 6;

				if (m_StartTime == 0)
					m_StartTime = StartTime;
				else
					StartTime = CurTime;
				m_SampleCount += Samples;
				EndTime = m_StartTime + (m_SampleCount * REFERENCE_TIME_SECOND / FREQUENCY);
				pOut->SetTime(&StartTime, &EndTime);

				bAdjusted = true;
			}
		}
		if (!bAdjusted) {
			TRACE(TEXT("Reset audio time\n"));
			m_StartTime = 0;
			m_SampleCount = 0;
		}
	}

	return S_OK;
}

HRESULT CAacDecFilter::Receive(IMediaSample *pSample)
{
	const AM_SAMPLE2_PROPERTIES *pProps = m_pInput->SampleProps();
	if (pProps->dwStreamId != AM_STREAM_MEDIA)
		return m_pOutput->Deliver(pSample);

	IMediaSample *pOutSample;
	HRESULT hr;

	hr = InitializeOutputSample(pSample, &pOutSample);
	if (FAILED(hr))
		return hr;
	hr = Transform(pSample, pOutSample);
	if (SUCCEEDED(hr)) {
		if (hr == S_OK) {
			hr = m_pOutput->Deliver(pOutSample);
		} else {
			hr = S_OK;
		}
		m_bSampleSkipped = FALSE;
	}
	pOutSample->Release();

	return hr;
}

void CAacDecFilter::OnPcmFrame(const CAacDecoder *pAacDecoder, const BYTE *pData, const DWORD dwSamples, const BYTE byChannel)
{
	// �o�̓|�C���^�擾
	BYTE *pOutBuff = NULL;
	if (FAILED(m_pOutSample->GetPointer(&pOutBuff)))
		return;

	DWORD dwOffset = m_pOutSample->GetActualDataLength();
	DWORD dwOutSize;

	if ((!m_bDownMixSurround && byChannel != m_byCurChannelNum
								&& (byChannel == 6 || m_byCurChannelNum == 6))
			|| (m_bDownMixSurround && reinterpret_cast<WAVEFORMATEX*>(m_MediaType.Format())->nChannels > 2)) {
		WAVEFORMATEX *pWaveInfo = reinterpret_cast<WAVEFORMATEX *>(m_MediaType.AllocFormatBuffer(
			byChannel == 2 ? sizeof(WAVEFORMATEX) : sizeof(WAVEFORMATEXTENSIBLE)));
		if (pWaveInfo == NULL)
			return;

		if (byChannel == 2) {
			pWaveInfo->wFormatTag = WAVE_FORMAT_PCM;
			pWaveInfo->nChannels = 2;
		} else {
			WAVEFORMATEXTENSIBLE *pwfex = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(pWaveInfo);

			pWaveInfo->wFormatTag = WAVE_FORMAT_EXTENSIBLE;
			pWaveInfo->nChannels = 6;
			pWaveInfo->cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
			pwfex->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT |
								   SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY |
								   SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT;
			pwfex->Samples.wValidBitsPerSample = 16;
			pwfex->SubFormat = MEDIASUBTYPE_PCM;
		}
		pWaveInfo->nSamplesPerSec = FREQUENCY;
		pWaveInfo->wBitsPerSample = 16;
		pWaveInfo->nBlockAlign = pWaveInfo->wBitsPerSample * pWaveInfo->nChannels / 8;
		pWaveInfo->nAvgBytesPerSec = pWaveInfo->nSamplesPerSec * pWaveInfo->nBlockAlign;

		/*
			���̕ӂ͂��Ȃ�蔲��
			�{���ł����IPin::QueryAccept/IPinConnection::DynamicQueryAccept��
			�Ă�Ńt�H�[�}�b�g�ύX�ł��邩�m�F���ׂ�
		*/
		if (FAILED(m_pOutSample->SetMediaType(&m_MediaType)))
			return;
		m_pOutput->SetMediaType(&m_MediaType);

		if (dwOffset > 0) {
			if (byChannel == 2) {
				dwOffset = DownMixSurround((short*)pOutBuff, (const short*)pOutBuff, dwOffset / (sizeof(short) * 2));
			} else if (byChannel == 6) {
				// �蔲��
				dwOffset *= 3;
				::ZeroMemory(pOutBuff, dwOffset);
			}
		}

		m_StartTime = 0;
		m_SampleCount = 0;
	}
	m_byCurChannelNum = byChannel;

	pOutBuff = &pOutBuff[dwOffset];
	// �_�E���~�b�N�X
	switch (byChannel) {
	case 1U:
		dwOutSize = DownMixMono((short *)pOutBuff, (const short *)pData, dwSamples);
		break;
	case 2U:
		dwOutSize = DownMixStereo((short *)pOutBuff, (const short *)pData, dwSamples);
		break;
	case 6U:
		if (m_bDownMixSurround) {
			dwOutSize = DownMixSurround((short *)pOutBuff, (const short *)pData, dwSamples);
		} else {
			dwOutSize = MapSurroundChannels((short *)pOutBuff, (const short *)pData, dwSamples);
		}
		break;
	default:
		return;
	}

	if (m_bNormalize)
		Normalize((short*)pOutBuff, dwOutSize / sizeof(short));

	if (m_pStreamCallback)
		m_pStreamCallback((short*)pOutBuff, dwSamples, m_bDownMixSurround ? 2 : byChannel, m_pStreamCallbackParam);

	// ���f�B�A�T���v���L���T�C�Y�ݒ�
	m_pOutSample->SetActualDataLength(dwOffset + dwOutSize);
}

const DWORD CAacDecFilter::DownMixMono(short *pDst, const short *pSrc, const DWORD dwSamples)
{
	// 1ch �� 2ch ��d��
	const short *p = pSrc, *pEnd = pSrc + dwSamples;
	short *q = pDst;

	while (p < pEnd) {
		short Value = *p++;
		*q++ = Value;	// L
		*q++ = Value;	// R
	}

	// �o�b�t�@�T�C�Y��Ԃ�
	return dwSamples * (sizeof(short) * 2);
}

const DWORD CAacDecFilter::DownMixStereo(short *pDst, const short *pSrc, const DWORD dwSamples)
{
	if (m_StereoMode == STEREOMODE_STEREO) {
		// 2ch �� 2ch �X���[
		::CopyMemory(pDst, pSrc, dwSamples * (sizeof(short) * 2));
	} else {
		const short *p = pSrc, *pEnd = pSrc + dwSamples * 2;
		short *q = pDst;

		if (m_StereoMode == STEREOMODE_LEFT) {
			// ���̂�
			while (p < pEnd) {
				short Value = *p;
				*q++ = Value;	// L
				*q++ = Value;	// R
				p += 2;
			}
		} else {
			// �E�̂�
			while (p < pEnd) {
				short Value = p[1];
				*q++ = Value;	// L
				*q++ = Value;	// R
				p += 2;
			}
		}
	}

	// �o�b�t�@�T�C�Y��Ԃ�
	return dwSamples * (sizeof(short) * 2);
}

#ifndef DOWNMIX_INT

/*
	���������_�����Z��(�I���W�i��)
*/
const DWORD CAacDecFilter::DownMixSurround(short *pDst, const short *pSrc, const DWORD dwSamples)
{
	// 5.1ch �� 2ch �_�E���~�b�N�X

	int iOutLch, iOutRch;

	for(DWORD dwPos = 0UL ; dwPos < dwSamples ; dwPos++){
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
	return dwSamples * (sizeof(short) * 2);
}

#else	// DOWNMIX_INT

/*
	�������Z��
*/
const DWORD CAacDecFilter::DownMixSurround(short *pDst, const short *pSrc, const DWORD dwSamples)
{
	// 5.1ch �� 2ch �_�E���~�b�N�X

	int iOutLch, iOutRch;

	for (DWORD dwPos = 0UL ; dwPos < dwSamples ; dwPos++) {
		// �_�E���~�b�N�X
		iOutLch =	((int)pSrc[dwPos * 6UL + 1UL] * IDOWNMIX_FRONT	+
					 (int)pSrc[dwPos * 6UL + 3UL] * IDOWNMIX_REAR	+
					 (int)pSrc[dwPos * 6UL + 0UL] * IDOWNMIX_CENTER	+
					 (int)pSrc[dwPos * 6UL + 5UL] * IDOWNMIX_LFE) /
					IDOWNMIX_DENOM;

		iOutRch =	((int)pSrc[dwPos * 6UL + 2UL] * IDOWNMIX_FRONT	+
					 (int)pSrc[dwPos * 6UL + 4UL] * IDOWNMIX_REAR	+
					 (int)pSrc[dwPos * 6UL + 0UL] * IDOWNMIX_CENTER	+
					 (int)pSrc[dwPos * 6UL + 5UL] * IDOWNMIX_LFE) /
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
	return dwSamples * (sizeof(short) * 2);
}

#endif	// DOWNMIX_INT


const DWORD CAacDecFilter::MapSurroundChannels(short *pDst, const short *pSrc, const DWORD dwSamples)
{
	for (DWORD i = 0 ; i < dwSamples ; i++) {
		pDst[i * 6 + 0] = pSrc[i * 6 + 1];	// Front left
		pDst[i * 6 + 1] = pSrc[i * 6 + 2];	// Font right
		pDst[i * 6 + 2] = pSrc[i * 6 + 0];	// Front center
		pDst[i * 6 + 3] = pSrc[i * 6 + 5];	// Low frequency
		pDst[i * 6 + 4] = pSrc[i * 6 + 3];	// Back left
		pDst[i * 6 + 5] = pSrc[i * 6 + 4];	// Back right
	}

	// �o�b�t�@�T�C�Y��Ԃ�
	return dwSamples * (sizeof(short) * 6);
}


bool CAacDecFilter::ResetDecoder()
{
	CAutoLock AutoLock(m_pLock);

	return m_AacDecoder.ResetDecoder();
}


bool CAacDecFilter::SetStereoMode(int StereoMode)
{
	switch (StereoMode) {
	case STEREOMODE_STEREO:
	case STEREOMODE_LEFT:
	case STEREOMODE_RIGHT:
		m_StereoMode = StereoMode;
		return true;
	}
	return false;
}


bool CAacDecFilter::SetDownMixSurround(bool bDownMix)
{
	if (bDownMix != m_bDownMixSurround) {
		CAutoLock AutoLock(m_pLock);

		m_bDownMixSurround = bDownMix;
	}
	return true;
}


bool CAacDecFilter::SetNormalize(bool bNormalize,float Level)
{
	m_bNormalize = bNormalize;
	m_NormalizeLevel = Level;
	return true;
}


bool CAacDecFilter::GetNormalize(float *pLevel) const
{
	if (pLevel)
		*pLevel = m_NormalizeLevel;
	return m_bNormalize;
}


#define NORMALIZE_SHIFT_BITS	12
#define NORMALIZE_DENOM			(1<<NORMALIZE_SHIFT_BITS)

/*
	�{���̓_�E���~�b�N�X�Ɠ����ɍs���������ǂ�
*/
void CAacDecFilter::Normalize(short *pBuffer,DWORD Samples)
{
	int Level = (int)(m_NormalizeLevel * (float)NORMALIZE_DENOM);
	short *p, *pEnd;
	int Value;

	p= pBuffer;
	pEnd= p + Samples;
	while (p < pEnd) {
		Value = (*p * Level) / NORMALIZE_DENOM;
		*p++ = Value > 32767 ? 32767 : Value < -32768 ? -32768 : Value;
	}
}


bool CAacDecFilter::SetAdjustStreamTime(bool bAdjust)
{
	CAutoLock AutoLock(m_pLock);

	m_bAdjustStreamTime = bAdjust;

	m_StartTime = 0;
	m_SampleCount = 0;

	return true;
}


bool CAacDecFilter::SetStreamCallback(StreamCallback pCallback, void *pParam)
{
	CAutoLock AutoLock(m_pLock);

	m_pStreamCallback = pCallback;
	m_pStreamCallbackParam = pParam;

	return true;
}
