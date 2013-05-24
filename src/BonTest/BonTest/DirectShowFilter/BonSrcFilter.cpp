#include "StdAfx.h"
#include "BonSrcFilter.h"


CBonSrcFilter::CBonSrcFilter(LPUNKNOWN pUnk, HRESULT *phr)
	: CBaseFilter(TEXT("Bon Source Filter"), pUnk, &m_cStateLock, CLSID_BONSOURCE)
	, m_pSrcPin(NULL)
{
	// �s���̃C���X�^���X����
	m_pSrcPin = new CBonSrcPin(phr, this);
		
	*phr = (m_pSrcPin)? S_OK : E_OUTOFMEMORY;
}

CBonSrcFilter::~CBonSrcFilter()
{
	// �s���̃C���X�^���X���폜����
	if(m_pSrcPin)delete m_pSrcPin;
}

CUnknown * WINAPI CBonSrcFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
	// �C���X�^���X���쐬����
	CBonSrcFilter *pNewFilter = new CBonSrcFilter(pUnk, phr);
	if(!pNewFilter)*phr = E_OUTOFMEMORY;
	
	return dynamic_cast<CUnknown *>(pNewFilter);
}

int CBonSrcFilter::GetPinCount(void)
{
	// �s������Ԃ�
	return 1;
}

CBasePin * CBonSrcFilter::GetPin(int n)
{
	// �s���̃C���X�^���X��Ԃ�
	return (n == 0)? m_pSrcPin : NULL;
}

STDMETHODIMP CBonSrcFilter::Run(REFERENCE_TIME tStart)
{
	TRACE(L"��CBonSrcFilter::Run()\n");

	return CBaseFilter::Run(tStart);
}

STDMETHODIMP CBonSrcFilter::Pause(void)
{
	TRACE(L"��CBonSrcFilter::Pause()\n");

	return CBaseFilter::Pause();
}

STDMETHODIMP CBonSrcFilter::Stop(void)
{
	TRACE(L"��CBonSrcFilter::Stop()\n");

	return CBaseFilter::Stop();
}

const bool CBonSrcFilter::InputMedia(CMediaData *pMediaData)
{
	if(!m_pSrcPin)return false;
	else return m_pSrcPin->InputMedia(pMediaData);
}
