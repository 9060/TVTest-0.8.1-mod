// BonTestDlg.cpp : �����t�@�C��
//

#include "stdafx.h"
#include "BonTest.h"
#include "BonTestDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// �A�v���P�[�V�����̃o�[�W�������Ɏg���� CAboutDlg �_�C�A���O

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �_�C�A���O �f�[�^
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g

// ����
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CBonTestDlg �_�C�A���O
CBonTestDlg::CBonTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBonTestDlg::IDD, pParent)
	, m_iChannel(0)
	, m_iService(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	
	// INI�L�[�o�^
	m_csRecordPath.RegisterKey(CONFSECT_GENERAL, TEXT("RecordPath"), TEXT("C:\\BonTest.ts"));
}

void CBonTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBonSampleDlg)
	DDX_CBIndex(pDX, IDC_CHCOMBO, m_iChannel);
	//}}AFX_DATA_MAP
	DDX_CBIndex(pDX, IDC_SVCOMBO, m_iService);
}

BEGIN_MESSAGE_MAP(CBonTestDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_RESET, &CBonTestDlg::OnBnClickedReset)
	ON_CBN_SELCHANGE(IDC_CHCOMBO, &CBonTestDlg::OnSelChangeChannel)
	ON_CBN_SELCHANGE(IDC_SVCOMBO, &CBonTestDlg::OnSelChangeService)
	ON_BN_CLICKED(IDC_STARTRECORD, &CBonTestDlg::OnBnClickedStartRecord)
	ON_BN_CLICKED(IDC_STOPRECORD, &CBonTestDlg::OnBnClickedStopRecord)
	ON_BN_CLICKED(IDC_BROWSEPATH, &CBonTestDlg::OnBnClickedBrowsePath)
	ON_BN_CLICKED(IDC_OPENFILE, &CBonTestDlg::OnBnClickedOpenFile)
	ON_BN_CLICKED(IDC_PLAY, &CBonTestDlg::OnBnClickedPlay)
	ON_BN_CLICKED(IDC_PAUSE, &CBonTestDlg::OnBnClickedPause)
END_MESSAGE_MAP()


// CBonTestDlg ���b�Z�[�W �n���h��

BOOL CBonTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// "�o�[�W�������..." ���j���[���V�X�e�� ���j���[�ɒǉ����܂��B

	// IDM_ABOUTBOX �́A�V�X�e�� �R�}���h�͈͓̔��ɂȂ���΂Ȃ�܂���B
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���̃_�C�A���O�̃A�C�R����ݒ肵�܂��B�A�v���P�[�V�����̃��C�� �E�B���h�E���_�C�A���O�łȂ��ꍇ�A
	//  Framework �́A���̐ݒ�������I�ɍs���܂��B
//	SetIcon(m_hIcon, TRUE);			// �傫���A�C�R���̐ݒ�
	SetIcon(m_hIcon, FALSE);		// �������A�C�R���̐ݒ�

	// TODO: �������������ɒǉ����܂��B

	// �ݒ蕜��
	SetDlgItemText(IDC_RECORDPATH, m_csRecordPath);

	// DTV�G���W���I�[�v��
	if(m_DtvEngine.OpenEngine(this, GetDlgItem(IDC_VIEW)->GetSafeHwnd())){
		// �v���r���[�J�n
		m_DtvEngine.EnablePreview();
		}

	UpdateData(FALSE);
	SetTimer(1, 500UL, NULL);

	return TRUE;  // �t�H�[�J�X���R���g���[���ɐݒ肵���ꍇ�������ATRUE ��Ԃ��܂��B
}

void CBonTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// �_�C�A���O�ɍŏ����{�^����ǉ�����ꍇ�A�A�C�R����`�悷�邽�߂�
//  ���̃R�[�h���K�v�ł��B�h�L�������g/�r���[ ���f�����g�� MFC �A�v���P�[�V�����̏ꍇ�A
//  ����́AFramework �ɂ���Ď����I�ɐݒ肳��܂��B

void CBonTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �`��̃f�o�C�X �R���e�L�X�g

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// �N���C�A���g�̎l�p�`�̈���̒���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �A�C�R���̕`��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// ���[�U�[���ŏ��������E�B���h�E���h���b�O���Ă���Ƃ��ɕ\������J�[�\�����擾���邽�߂ɁA
//  �V�X�e�������̊֐����Ăяo���܂��B
HCURSOR CBonTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CBonTestDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: �����Ƀ��b�Z�[�W �n���h�� �R�[�h��ǉ����܂��B

	KillTimer(1);
	
	// DTV�G���W���N���[�Y
	m_DtvEngine.CloseEngine();
}

void CBonTestDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �����Ƀ��b�Z�[�W �n���h�� �R�[�h��ǉ����邩�A����̏������Ăяo���܂��B
	
	// ��M���x���擾
	CString csText, csTemp;
	csText.Format(TEXT("��M���x���F�@%.2fdB"), m_DtvEngine.m_BonSrcDecoder.GetSignalLevel());
	SetDlgItemText(IDC_STATUS, csText);

	if(GetDlgItem(IDC_VIEW)->GetWindow(GW_CHILD)){
		GetDlgItem(IDC_VIEW)->GetWindow(GW_CHILD)->InvalidateRect(NULL);
		}

	// �X�e�[�^�X�o�[�X�V
	csText = TEXT("");
	
	csTemp.Format(TEXT("���́F %lu    "), m_DtvEngine.m_TsPacketParser.GetInputPacketCount());
	csText += csTemp;

	csTemp.Format(TEXT("�o�́F %lu    "), m_DtvEngine.m_TsPacketParser.GetOutputPacketCount());
	csText += csTemp;

	csTemp.Format(TEXT("�G���[�F %lu    "), m_DtvEngine.m_TsPacketParser.GetErrorPacketCount());
	csText += csTemp;

	csTemp.Format(TEXT("�����R��F %lu    "), m_DtvEngine.m_TsDescrambler.GetScramblePacketCount());
	csText += csTemp;

	csTemp.Format(TEXT("�^��T�C�Y�F %.2lfMB"), (double)m_DtvEngine.m_FileWriter.GetWriteSize() / (1024.0 * 1024.0));
	csText += csTemp;

	SetDlgItemText(IDC_INFOBAR, csText);

	CDialog::OnTimer(nIDEvent);
}

void CBonTestDlg::OnBnClickedReset()
{
	// TODO: �����ɃR���g���[���ʒm�n���h�� �R�[�h��ǉ����܂��B
	m_DtvEngine.ResetEngine();
}

void CBonTestDlg::OnSelChangeChannel()
{
	// TODO: �����ɃR���g���[���ʒm�n���h�� �R�[�h��ǉ����܂��B
	UpdateData(TRUE);

	if(!m_DtvEngine.SetChannel(0U, (WORD)m_iChannel + 13U)){
		::AfxMessageBox(TEXT("�`�����l���̐ؑւɎ��s���܂����B"));
		}
		
	CComboBox *pServiceCombo = static_cast<CComboBox *>(GetDlgItem(IDC_SVCOMBO));
	pServiceCombo->ResetContent();
	pServiceCombo->AddString(TEXT("�`�����l���ύX��..."));
	pServiceCombo->SetCurSel(0);
}

void CBonTestDlg::OnSelChangeService()
{
	// TODO: �����ɃR���g���[���ʒm�n���h�� �R�[�h��ǉ����܂��B
	UpdateData(TRUE);

	if(!m_DtvEngine.SetService(m_iService)){
		::AfxMessageBox(TEXT("�T�[�r�X�̐ؑւɎ��s���܂����B"));
		}
}

const DWORD CBonTestDlg::OnDtvEngineEvent(CDtvEngine *pEngine, const DWORD dwEventID, PVOID pParam)
{
	// ���S�Ɏb��
	CProgManager *pProgManager = static_cast<CProgManager *>(pParam);
	CComboBox *pServiceCombo = static_cast<CComboBox *>(GetDlgItem(IDC_SVCOMBO));
	
	pServiceCombo->ResetContent();
	TCHAR szServiceName[1024];
	
	// �T�[�r�X���X�V
	for(WORD wIndex = 0U ; wIndex < pProgManager->GetServiceNum() ; wIndex++){
		if(pProgManager->GetServiceName(szServiceName, wIndex)){
			pServiceCombo->AddString(szServiceName);		
			}
		else{
			pServiceCombo->AddString(TEXT("�T�[�r�X���擾��..."));
			}
		}	

	pServiceCombo->SetCurSel(pEngine->GetService());

	return 0UL;
}

void CBonTestDlg::OnBnClickedStartRecord()
{
	// TODO: �����ɃR���g���[���ʒm�n���h�� �R�[�h��ǉ����܂��B
	
	// �t�@�C���������݊J�n
	if(!m_DtvEngine.m_FileWriter.OpenFile(m_csRecordPath)){
		::AfxMessageBox(TEXT("�t�@�C���̃I�[�v���Ɏ��s���܂����B"));
		return;		
		}
		
	// �R���g���[����ԕύX
	GetDlgItem(IDC_STARTRECORD)->EnableWindow(FALSE);
	GetDlgItem(IDC_STOPRECORD)->EnableWindow(TRUE);
}

void CBonTestDlg::OnBnClickedStopRecord()
{
	// TODO: �����ɃR���g���[���ʒm�n���h�� �R�[�h��ǉ����܂��B
	
	// �t�@�C���������ݏI��
	m_DtvEngine.m_FileWriter.CloseFile();
	
	// �R���g���[����ԕύX
	GetDlgItem(IDC_STARTRECORD)->EnableWindow(TRUE);
	GetDlgItem(IDC_STOPRECORD)->EnableWindow(FALSE);
}

void CBonTestDlg::OnBnClickedBrowsePath()
{
	// TODO: �����ɃR���g���[���ʒm�n���h�� �R�[�h��ǉ����܂��B
	
	// �t�@�C���I��
	CFileDialog Dlg(FALSE, TEXT("ts"), m_csRecordPath, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, TEXT("MPEG2-TS (*.ts)|*.ts||"));
	if(Dlg.DoModal() != IDOK)return;
	
	// �R���g���[���X�V
	m_csRecordPath = Dlg.GetPathName();
	SetDlgItemText(IDC_RECORDPATH, m_csRecordPath);
}

void CBonTestDlg::OnBnClickedOpenFile()
{
	// TODO: �����ɃR���g���[���ʒm�n���h�� �R�[�h��ǉ����܂��B
	CFileDialog Dlg(TRUE, TEXT("ts"), NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, TEXT("TS�t�@�C�� (*.ts)|*.ts||"));
	if(Dlg.DoModal() != IDOK)return;

	if(!m_DtvEngine.PlayFile(Dlg.GetPathName())){
		::AfxMessageBox(TEXT("�t�@�C���̃I�[�v���Ɏ��s���܂����B"));
		}
}

void CBonTestDlg::OnBnClickedPlay()
{
	// TODO: �����ɃR���g���[���ʒm�n���h�� �R�[�h��ǉ����܂��B
	
}

void CBonTestDlg::OnBnClickedPause()
{
	// TODO: �����ɃR���g���[���ʒm�n���h�� �R�[�h��ǉ����܂��B
	m_DtvEngine.StopFile();
}
