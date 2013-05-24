#include "StdAfx.h"
#include "BonSrcPin.h"
#include "BonSrcFilter.h"


CBonSrcPin::CBonSrcPin(HRESULT *phr, CBonSrcFilter *pFilter)
	: CBaseOutputPin(TEXT("CBonSrcPin"), pFilter, pFilter->m_pLock, phr, L"TS")
	, m_pFilter(pFilter)
{
	*phr = S_OK;
}

CBonSrcPin::~CBonSrcPin(void)
{
	
}

HRESULT CBonSrcPin::GetMediaType(int iPosition, CMediaType *pMediaType)
{
	CAutoLock AutoLock(&m_pFilter->m_cStateLock);
	CheckPointer(pMediaType, E_POINTER);
 
	if(iPosition < 0)return E_INVALIDARG;
	if(iPosition >0)return VFW_S_NO_MORE_ITEMS;
 
	// ���f�B�A�^�C�v�ݒ�
	pMediaType->InitMediaType();
	pMediaType->SetType(&MEDIATYPE_Stream);
	pMediaType->SetSubtype(&MEDIASUBTYPE_MPEG2_TRANSPORT);
	pMediaType->SetTemporalCompression(FALSE);
	pMediaType->SetSampleSize(188);

	return S_OK;
}

HRESULT CBonSrcPin::CheckMediaType(const CMediaType *pMediaType)
{
	CAutoLock AutoLock(&m_pFilter->m_cStateLock);
	CheckPointer(pMediaType, E_POINTER);

	// ���f�B�A�^�C�v���`�F�b�N����
	CMediaType m_mt;
	GetMediaType(0, &m_mt);

	return (*pMediaType == m_mt)? S_OK : E_FAIL;
}

HRESULT CBonSrcPin::Active(void)
{
	CAutoLock AutoLock(&m_pFilter->m_cStateLock);

	return CBaseOutputPin::Active();
}

HRESULT CBonSrcPin::Inactive(void)
{
	CAutoLock AutoLock(&m_pFilter->m_cStateLock);

	DeliverEndOfStream();

	return CBaseOutputPin::Inactive();
}

HRESULT CBonSrcPin::Run(REFERENCE_TIME tStart)
{
	return CBaseOutputPin::Run(tStart);
}

HRESULT CBonSrcPin::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest)
{
	CAutoLock AutoLock(&m_pFilter->m_cStateLock);
	CheckPointer(pAlloc, E_POINTER);
	CheckPointer(pRequest, E_POINTER);

	// �o�b�t�@��1����΂悢
	if(!pRequest->cBuffers)pRequest->cBuffers = 1024;

	// �Ƃ肠����TS�p�P�b�g���m��
    if(pRequest->cbBuffer < 256)pRequest->cbBuffer = 256;

	// �A���P�[�^�v���p�e�B��ݒ肵�Ȃ���
	ALLOCATOR_PROPERTIES Actual;
	HRESULT hr = pAlloc->SetProperties(pRequest, &Actual);
	if(FAILED(hr))return hr;

	// �v�����󂯓����ꂽ������
	if(Actual.cbBuffer < pRequest->cbBuffer)return E_FAIL;

	return S_OK;
}

const bool CBonSrcPin::InputMedia(CMediaData *pMediaData)
{
	CAutoLock AutoLock(&m_pFilter->m_cStateLock);

	if(IsStopped())return true;

	// ��̃��f�B�A�T���v����v������
	IMediaSample *pSample = NULL;
	HRESULT hr = GetDeliveryBuffer(&pSample, NULL, NULL, 0);
	if(FAILED(hr))return false;
	
	// �������ݐ�|�C���^���擾����
	BYTE *pSampleData = NULL;
	pSample->GetPointer(&pSampleData);
	
	// �󂯎�����f�[�^���R�s�[����
	::CopyMemory(pSampleData, pMediaData->GetData(), pMediaData->GetSize());
    pSample->SetActualDataLength(pMediaData->GetSize());
	pSample->SetSyncPoint(FALSE);
	
	// �T���v�������̃t�B���^�ɓn��
	Deliver(pSample);

	pSample->Release();

	return true;
}
