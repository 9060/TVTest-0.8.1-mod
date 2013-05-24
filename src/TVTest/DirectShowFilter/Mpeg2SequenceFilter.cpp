#include "StdAfx.h"
#include "Mpeg2SequenceFilter.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif




CMpeg2SequenceFilter::CMpeg2SequenceFilter(LPUNKNOWN pUnk, HRESULT *phr)
	//: CTransformFilter(MPEG2SEQUENCEFILTER_NAME, pUnk, CLSID_MPEG2SEQFILTER)
	: CTransInPlaceFilter(MPEG2SEQUENCEFILTER_NAME, pUnk, CLSID_MPEG2SEQFILTER, phr, FALSE)
	,m_Mpeg2Parser(this)
	,m_VideoInfo()
{
	TRACE(TEXT("CMpeg2SequenceFilter::CMpeg2SequenceFilter %p\n"),this);

	m_pfnVideoInfoRecvFunc=NULL;
	m_pCallbackParam=NULL;
	*phr=S_OK;
}


CMpeg2SequenceFilter::~CMpeg2SequenceFilter(void)
{
	TRACE(TEXT("CMpeg2SequenceFilter::~CMpeg2SequenceFilter\n"));
}


IBaseFilter* WINAPI CMpeg2SequenceFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr,CMpeg2SequenceFilter **ppClassIf)
{
	// �C���X�^���X���쐬����
	if(ppClassIf) *ppClassIf = NULL;
	CMpeg2SequenceFilter *pNewFilter = new CMpeg2SequenceFilter(pUnk, phr);
	/*
	if(!pNewFilter) {
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


HRESULT CMpeg2SequenceFilter::CheckInputType(const CMediaType* mtIn)
{
    CheckPointer(mtIn, E_POINTER);

	if(*mtIn->Type() == MEDIATYPE_Video){
		if(*mtIn->Subtype() == MEDIASUBTYPE_MPEG2_VIDEO){

				return S_OK;
			}
		}
	return VFW_E_TYPE_NOT_ACCEPTED;
}


/*
HRESULT CMpeg2SequenceFilter::CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut)
{
	CheckPointer(mtIn, E_POINTER);
	CheckPointer(mtOut, E_POINTER);

	if(*mtOut->Type() == MEDIATYPE_Video){
		if(*mtOut->Subtype() == MEDIASUBTYPE_MPEG2_VIDEO){
			return S_OK;
			}
		}

	return VFW_E_TYPE_NOT_ACCEPTED;
}
*/


HRESULT CMpeg2SequenceFilter::CompleteConnect(PIN_DIRECTION direction,IPin *pReceivePin)
{
	return S_OK;
}


/*
HRESULT CMpeg2SequenceFilter::DecideBufferSize(IMemAllocator * pAllocator, ALLOCATOR_PROPERTIES *pprop)
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
*/


HRESULT CMpeg2SequenceFilter::GetMediaType(int iPosition, CMediaType *pMediaType)
{
	CAutoLock AutoLock(m_pLock);
	CheckPointer(pMediaType, E_POINTER);

	if(iPosition < 0)return E_INVALIDARG;
	if(iPosition > 0)return VFW_S_NO_MORE_ITEMS;
	*pMediaType=this->m_pInput->CurrentMediaType();
	return S_OK;
}


/*
HRESULT CMpeg2SequenceFilter::StartStreaming(void)
{
	return S_OK;
}


HRESULT CMpeg2SequenceFilter::StopStreaming(void)
{
	return S_OK;
}


HRESULT CMpeg2SequenceFilter::Transform(IMediaSample *pIn, IMediaSample *pOut)
{
	// ���̓f�[�^�|�C���^���擾����
	BYTE *pInData = NULL;
	pIn->GetPointer(&pInData);
	long lDataSize = pIn->GetActualDataLength();

	// �o�̓|�C���^�擾
	pOut->SetActualDataLength(0);
	const DWORD dwOffset = pOut->GetActualDataLength();
	BYTE *pOutBuff = NULL;
	pOut->GetPointer(&pOutBuff);
	pOutBuff = &pOutBuff[dwOffset];

	// �V�[�P���X���擾
	m_Mpeg2Parser.StoreEs(pInData,lDataSize);

	// ���̂܂܏o�͂ɃR�s�[
	::CopyMemory(pOutBuff,pInData,lDataSize);
	pOut->SetActualDataLength(dwOffset + lDataSize);
	return S_OK;
}
*/


HRESULT CMpeg2SequenceFilter::Transform(IMediaSample *pSample)
{
	BYTE *pData = NULL;
	pSample->GetPointer(&pData);
	LONG DataSize = pSample->GetActualDataLength();

	// �V�[�P���X���擾
	m_Mpeg2Parser.StoreEs(pData,DataSize);

	return S_OK;
}


HRESULT CMpeg2SequenceFilter::Receive(IMediaSample *pSample)
{
	Transform(pSample);
	return m_pOutput->Deliver(pSample);
}


void CMpeg2SequenceFilter::SetRecvCallback(MPEG2SEQUENCE_VIDEOINFO_FUNC pCallback, const PVOID pParam)
{
	m_pfnVideoInfoRecvFunc = pCallback;
	m_pCallbackParam = pParam;
}


void CMpeg2SequenceFilter::OnMpeg2Sequence(const CMpeg2Parser *pMpeg2Parser, const CMpeg2Sequence *pSequence)
{
	BYTE AspectX,AspectY;
	WORD OrigWidth,OrigHeight;
	WORD DisplayWidth,DisplayHeight;

	switch (pSequence->GetAspectRatioInfo()) {
	case 2:
		AspectX = 4;
		AspectY = 3;
		break;
	case 3:
		AspectX = 16;
		AspectY = 9;
		break;
	case 4:
		AspectX = 221;
		AspectY = 100;
		break;
	default:
		AspectX = AspectY = 0;
	}

	OrigWidth=pSequence->GetHorizontalSize();
	OrigHeight=pSequence->GetVerticalSize();

	if (pSequence->GetExtendDisplayInfo()) {
		DisplayWidth = pSequence->GetExtendDisplayHorizontalSize();
		DisplayHeight = pSequence->GetExtendDisplayVerticalSize();
	} else {
		DisplayWidth = OrigWidth;
		DisplayHeight = OrigHeight;
	}

	CMpeg2VideoInfo Info(OrigWidth,OrigHeight,DisplayWidth,DisplayHeight,AspectX,AspectY);

	if (Info!=m_VideoInfo) {
		// �f���T�C�Y���ς����
		m_VideoInfo=Info;
		// �ʒm
		if (m_pfnVideoInfoRecvFunc)
			m_pfnVideoInfoRecvFunc(&m_VideoInfo,m_pCallbackParam);
	}
}


const bool CMpeg2SequenceFilter::GetVideoSize(WORD *pX, WORD *pY) const
{
	if (m_VideoInfo.m_DisplayWidth==0 || m_VideoInfo.m_DisplayHeight==0)
		return false;
	if (pX) *pX = m_VideoInfo.m_DisplayWidth;
	if (pY) *pY = m_VideoInfo.m_DisplayHeight;
	return true;
}


const bool CMpeg2SequenceFilter::GetAspectRatio(BYTE *pX,BYTE *pY) const
{
	if (m_VideoInfo.m_AspectRatioX==0 || m_VideoInfo.m_AspectRatioY==0)
		return false;
	if (pX) *pX = m_VideoInfo.m_AspectRatioX;
	if (pY) *pY = m_VideoInfo.m_AspectRatioY;
	return true;
}


const bool CMpeg2SequenceFilter::GetOriginalVideoSize(WORD *pWidth,WORD *pHeight) const
{
	if (m_VideoInfo.m_OrigWidth==0 || m_VideoInfo.m_OrigHeight==0)
		return false;
	if (pWidth)
		*pWidth=m_VideoInfo.m_OrigWidth;
	if (pHeight)
		*pHeight=m_VideoInfo.m_OrigHeight;
	return true;
}


const bool CMpeg2SequenceFilter::GetVideoInfo(CMpeg2VideoInfo *pInfo) const
{
	*pInfo=m_VideoInfo;
	return true;
}
