// BonTestDlg.h : �w�b�_�[ �t�@�C��
//

#pragma once

#include "DtvEngine.h"
#include "ConfigData.h"


// CBonTestDlg �_�C�A���O
class CBonTestDlg : public CDialog, public CDtvEngineHandler
{
// �R���X�g���N�V����
public:
	CBonTestDlg(CWnd* pParent = NULL);	// �W���R���X�g���N�^

// �_�C�A���O �f�[�^
	enum { IDD = IDD_BONTEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �T�|�[�g

// ����
protected:
	// �������ꂽ�A���b�Z�[�W���蓖�Ċ֐�
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	
	const DWORD OnDtvEngineEvent(CDtvEngine *pEngine, const DWORD dwEventID, PVOID pParam);
	
	HICON m_hIcon;
	CDtvEngine m_DtvEngine;
	CConfigString m_csRecordPath;
	
public:
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedReset();
	afx_msg void OnSelChangeChannel();
	afx_msg void OnSelChangeService();

	int m_iChannel;
	int m_iService;
	afx_msg void OnBnClickedStartRecord();
	afx_msg void OnBnClickedStopRecord();
	afx_msg void OnBnClickedBrowsePath();
	afx_msg void OnBnClickedOpenFile();
	afx_msg void OnBnClickedPlay();
	afx_msg void OnBnClickedPause();
};
