#include "StdAfx.h"
#include "BonSrcPin.h"
#include "BonSrcFilter.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


#ifndef TS_PACKETSIZE
#define TS_PACKETSIZE	(188U)
#endif

#define SAMPLE_PACKETS 256




CBonSrcPin::CBonSrcPin(HRESULT *phr, CBonSrcFilter *pFilter)
	: CBaseOutputPin(TEXT("CBonSrcPin"), pFilter, pFilter->m_pLock, phr, L"TS")
	, m_pFilter(pFilter)
	, m_hThread(NULL)
	, m_pBuffer(NULL)
	, m_BufferLength(0x1000)
	, m_bOutputWhenPaused(false)
{
	TRACE(TEXT("CBonSrcPin::CBonSrcPin() %p\n"),this);

	*phr = S_OK;
}

CBonSrcPin::~CBonSrcPin(void)
{
	TRACE(TEXT("CBonSrcPin::~CBonSrcPin()\n"));

	EndStreamThread();
}

HRESULT CBonSrcPin::GetMediaType(int iPosition, CMediaType *pMediaType)
{
	CAutoLock AutoLock(&m_pFilter->m_cStateLock);
	CheckPointer(pMediaType, E_POINTER);

	if(iPosition < 0)return E_INVALIDARG;
	if(iPosition > 0)return VFW_S_NO_MORE_ITEMS;

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
	TRACE(TEXT("CBonSrcPin::Active()\n"));

	HRESULT hr=CBaseOutputPin::Active();
	if (FAILED(hr))
		return hr;

	if (m_hThread)
		return E_UNEXPECTED;

	m_bKillSignal=false;
	m_pBuffer=new BYTE[m_BufferLength*TS_PACKETSIZE];
	m_BufferUsed=0;
	m_BufferPos=0;
	m_hThread=::CreateThread(NULL,0,StreamThread,this,0,NULL);
	if (m_hThread==NULL) {
		delete [] m_pBuffer;
		m_pBuffer=NULL;
		return E_FAIL;
	}
	return S_OK;
}

void CBonSrcPin::EndStreamThread()
{
	if (m_hThread) {
		m_bKillSignal=true;
		if (::WaitForSingleObject(m_hThread, 1000) == WAIT_TIMEOUT) {
			::TerminateThread(m_hThread, -1);
		}
		::CloseHandle(m_hThread);
		m_hThread=NULL;
	}
	m_StreamLock.Lock();
	if (m_pBuffer) {
		m_BufferUsed=0;
		m_BufferPos=0;
		delete [] m_pBuffer;
		m_pBuffer=NULL;
	}
	m_StreamLock.Unlock();
}

HRESULT CBonSrcPin::Inactive(void)
{
	TRACE(TEXT("CBonSrcPin::Inactive()\n"));

	//DeliverEndOfStream();

	HRESULT hr=CBaseOutputPin::Inactive();
	if (FAILED(hr))
		return hr;

	EndStreamThread();

	return S_OK;
}

HRESULT CBonSrcPin::Run(REFERENCE_TIME tStart)
{
	TRACE(TEXT("CBonSrcPin::Run()\n"));

	return CBaseOutputPin::Run(tStart);
}

HRESULT CBonSrcPin::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest)
{
	CAutoLock AutoLock(&m_pFilter->m_cStateLock);
	CheckPointer(pAlloc, E_POINTER);
	CheckPointer(pRequest, E_POINTER);

	// �o�b�t�@��1����΂悢
	if(!pRequest->cBuffers)pRequest->cBuffers = 1;

	// �Ƃ肠����TS�p�P�b�g���m��
	if(pRequest->cbBuffer < TS_PACKETSIZE * SAMPLE_PACKETS)
		pRequest->cbBuffer = TS_PACKETSIZE * SAMPLE_PACKETS;

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
#if 0

	CAutoLock AutoLock(&m_pFilter->m_cStateLock);

	if (m_pFilter->m_State==State_Stopped
			&& (m_pFilter->m_State==State_Paused && !m_bOutputWhenPaused))
		return true;

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

	// �T���v�������̃t�B���^�ɓn��
	Deliver(pSample);

	pSample->Release();

#else

	m_StreamLock.Lock();

	if (m_pBuffer==NULL) {
		m_StreamLock.Unlock();
		return false;
	}

	for (int i=0;m_BufferUsed==m_BufferLength;i++) {
		m_StreamLock.Unlock();
		if (i==200) {
			Flush();
			return false;
		}
		::Sleep(5);
		m_StreamLock.Lock();
		if (m_BufferUsed==0) {
			// ���Z�b�g���ꂽ
			m_StreamLock.Unlock();
			return true;
		}
	}

	::CopyMemory(m_pBuffer+(m_BufferPos+m_BufferUsed)%m_BufferLength*TS_PACKETSIZE,
				 pMediaData->GetData(),TS_PACKETSIZE/*pMediaData->GetSize()*/);
	m_BufferUsed++;

	m_StreamLock.Unlock();

#endif

	return true;
}

void CBonSrcPin::Reset()
{
	m_StreamLock.Lock();
	m_BufferUsed=0;
	m_BufferPos=0;
	m_StreamLock.Unlock();
}

void CBonSrcPin::Flush()
{
	OutputDebugString(TEXT("CBonSrcPin::Flush()\n"));
	DeliverBeginFlush();
	Reset();
	DeliverEndFlush();
}

DWORD WINAPI CBonSrcPin::StreamThread(LPVOID lpParameter)
{
	CBonSrcPin *pThis=static_cast<CBonSrcPin*>(lpParameter);

	OutputDebugString(TEXT("CBonSrcPin::StreamThread() Start\n"));

	::CoInitialize(NULL);

	while (!pThis->m_bKillSignal) {
		pThis->m_StreamLock.Lock();
		if (pThis->m_BufferUsed > 0) {
			DWORD Size = min(SAMPLE_PACKETS, pThis->m_BufferUsed);
			if (Size > pThis->m_BufferLength - pThis->m_BufferPos)
				Size = pThis->m_BufferLength - pThis->m_BufferPos;
			BYTE *pBuffer = pThis->m_pBuffer + pThis->m_BufferPos * TS_PACKETSIZE;

			//CAutoLock Lock(&pThis->m_pFilter->m_cStateLock);

			// ��̃��f�B�A�T���v����v������
			IMediaSample *pSample = NULL;
			HRESULT hr = pThis->GetDeliveryBuffer(&pSample, NULL, NULL, 0);
			if (SUCCEEDED(hr)) {
				// �������ݐ�|�C���^���擾����
				BYTE *pSampleData = NULL;
				pSample->GetPointer(&pSampleData);

				// �󂯎�����f�[�^���R�s�[����
				::CopyMemory(pSampleData, pBuffer, Size * TS_PACKETSIZE);
				pSample->SetActualDataLength(Size * TS_PACKETSIZE);
			}

			pThis->m_BufferPos += Size;
			if (pThis->m_BufferPos == pThis->m_BufferLength)
				pThis->m_BufferPos = 0;
			pThis->m_BufferUsed -= Size;
			pThis->m_StreamLock.Unlock();

			if (SUCCEEDED(hr)) {
				// �T���v�������̃t�B���^�ɓn��
				pThis->Deliver(pSample);

				pSample->Release();
			}
		} else {
			pThis->m_StreamLock.Unlock();
			::Sleep(5);
		}
	}

	::CoUninitialize();

	OutputDebugString(TEXT("CBonSrcPin::StreamThread() End\n"));

	return 0;
}
