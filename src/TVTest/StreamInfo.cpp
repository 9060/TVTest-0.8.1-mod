#include "stdafx.h"
#include "TVTest.h"
#include "AppMain.h"
#include "StreamInfo.h"
#include "DialogUtil.h"
#include "resource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif




CStreamInfo::CStreamInfo()
	: m_pEventHandler(NULL)
	, m_fCreateFirst(true)
{
}


CStreamInfo::~CStreamInfo()
{
}


bool CStreamInfo::Create(HWND hwndOwner)
{
	if (m_fCreateFirst) {
		if (m_pEventHandler!=NULL)
			m_pEventHandler->OnRestoreSettings();
		m_fCreateFirst=false;
	}

	return CreateDialogWindow(hwndOwner,GetAppClass().GetResourceInstance(),MAKEINTRESOURCE(IDD_STREAMINFO));
}


void CStreamInfo::SetEventHandler(CEventHandler *pHandler)
{
	m_pEventHandler=pHandler;
}


INT_PTR CStreamInfo::DlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	INT_PTR Result=CResizableDialog::DlgProc(hDlg,uMsg,wParam,lParam);

	switch (uMsg) {
	case WM_INITDIALOG:
		SetService();

		AddControl(IDC_STREAMINFO_STREAM,ALIGN_HORZ);
		AddControl(IDC_STREAMINFO_NETWORK,ALIGN_HORZ);
		AddControl(IDC_STREAMINFO_SERVICE,ALIGN_ALL);
		AddControl(IDC_STREAMINFO_UPDATE,ALIGN_BOTTOM_RIGHT);
		AddControl(IDC_STREAMINFO_COPY,ALIGN_BOTTOM_RIGHT);
#if 0
		AddControl(IDC_STREAMINFO_CONTRACT_LABEL,ALIGN_BOTTOM);
		AddControl(IDC_STREAMINFO_CONTRACT_SERVICE,ALIGN_HORZ_BOTTOM);
		AddControl(IDC_STREAMINFO_CONTRACT_CHECK,ALIGN_BOTTOM_RIGHT);
		AddControl(IDC_STREAMINFO_CONTRACT_INFO,ALIGN_HORZ_BOTTOM);
#endif

		ApplyPosition();
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_STREAMINFO_UPDATE:
			SetService();
			return TRUE;

		case IDC_STREAMINFO_COPY:
			{
				int Length;
				LPTSTR pszText,p;
				HWND hwndTree=::GetDlgItem(hDlg,IDC_STREAMINFO_SERVICE);

				Length=0x8000;
				pszText=new TCHAR[Length];
				p=pszText;
				::GetDlgItemText(hDlg,IDC_STREAMINFO_STREAM,p,Length);
				p+=::lstrlen(p);
				*p++=_T('\r');
				*p++=_T('\n');
				::GetDlgItemText(hDlg,IDC_STREAMINFO_NETWORK,p,Length-(int)(p-pszText));
				p+=::lstrlen(p);
				*p++=_T('\r');
				*p++=_T('\n');
				GetTreeViewText(hwndTree,TreeView_GetChild(hwndTree,TreeView_GetRoot(hwndTree)),true,
								p,Length-(int)(p-pszText));
				CopyTextToClipboard(GetAppClass().GetUICore()->GetMainWindow(),pszText);
				delete [] pszText;
			}
			return TRUE;

#if 0
		case IDC_STREAMINFO_CONTRACT_CHECK:
			{
				LRESULT Sel=DlgComboBox_GetCurSel(hDlg,IDC_STREAMINFO_CONTRACT_SERVICE);

				if (Sel>=0) {
					LRESULT Data=DlgComboBox_GetItemData(hDlg,IDC_STREAMINFO_CONTRACT_SERVICE,Sel);
					SYSTEMTIME st;
					CCasProcessor::ContractStatus Status=
						GetAppClass().GetCoreEngine()->m_DtvEngine.m_CasProcessor.GetContractPeriod(
							LOWORD(Data),HIWORD(Data),&st);
					TCHAR szText[256];

					switch (Status) {
					case CCasProcessor::CONTRACT_CONTRACTED:
						StdUtil::snprintf(szText,lengthof(szText),
										  TEXT("�_����� %d�N%d��%d�� �܂�"),st.wYear,st.wMonth,st.wDay);
						break;
					case CCasProcessor::CONTRACT_UNCONTRACTED:
						::lstrcpy(szText,TEXT("���_��ł�"));
						break;
					case CCasProcessor::CONTRACT_UNKNOWN:
						::lstrcpy(szText,TEXT("�_������m�F�ł��܂���"));
						break;
					case CCasProcessor::CONTRACT_ERROR:
					default:
						::lstrcpy(szText,TEXT("�G���[���������܂���"));
						break;
					}
					::SetDlgItemText(hDlg,IDC_STREAMINFO_CONTRACT_INFO,szText);
				}
			}
			return TRUE;
#endif

		case IDOK:
		case IDCANCEL:
			if (m_pEventHandler==NULL || m_pEventHandler->OnClose())
				::DestroyWindow(hDlg);
			return TRUE;
		}
		return TRUE;

	case WM_NOTIFY:
		switch (reinterpret_cast<NMHDR*>(lParam)->code) {
		case NM_RCLICK:
			{
				NMHDR *pnmhdr=reinterpret_cast<NMHDR*>(lParam);
				//HTREEITEM hItem=TreeView_GetSelection(pnmhdr->hwndFrom);
				DWORD Pos=::GetMessagePos();
				TVHITTESTINFO tvhti;
				tvhti.pt.x=(SHORT)LOWORD(Pos);
				tvhti.pt.y=(SHORT)HIWORD(Pos);
				::ScreenToClient(pnmhdr->hwndFrom,&tvhti.pt);
				HTREEITEM hItem=TreeView_HitTest(pnmhdr->hwndFrom,&tvhti);
				if (hItem!=NULL) {
					HMENU hmenu=::CreatePopupMenu();
					::AppendMenu(hmenu,MFT_STRING | MFS_ENABLED,1,TEXT("�R�s�[(&C)"));
					POINT pt;
					::GetCursorPos(&pt);
					switch (::TrackPopupMenu(hmenu,TPM_RIGHTBUTTON | TPM_RETURNCMD,pt.x,pt.y,0,hDlg,NULL)) {
					case 1:
						{
							int Length=0x8000;
							LPTSTR pszText=new TCHAR[Length];

							if (GetTreeViewText(pnmhdr->hwndFrom,hItem,false,pszText,Length)>0)
								CopyTextToClipboard(GetAppClass().GetUICore()->GetMainWindow(),pszText);
							delete [] pszText;
						}
						break;
					}
				}
			}
			return TRUE;
		}
		break;
	}
	return Result;
}


static LPCTSTR GetStreamTypeText(BYTE StreamType)
{
	switch (StreamType) {
	case STREAM_TYPE_MPEG1:	return TEXT("MPEG-1");
	case STREAM_TYPE_MPEG2:	return TEXT("MPEG-2");
	case STREAM_TYPE_AAC:	return TEXT("AAC");
	case STREAM_TYPE_H264:	return TEXT("H.264");
	}
	return TEXT("Unknown");
}

static LPCTSTR GetAreaText(WORD AreaCode)
{
	switch (AreaCode) {
	// �L�敄��
	case 0x5A5:	return TEXT("�֓��L�挗");
	case 0x72A:	return TEXT("�����L�挗");
	case 0x8D5:	return TEXT("�ߋE�L�挗");
	case 0x699:	return TEXT("����E������");
	case 0x553:	return TEXT("���R�E���쌗");
	// ���敄��
	case 0x16B:	return TEXT("�k�C��");
	case 0x467:	return TEXT("�X");
	case 0x5D4:	return TEXT("���");
	case 0x758:	return TEXT("�{��");
	case 0xAC6:	return TEXT("�H�c");
	case 0xE4C:	return TEXT("�R�`");
	case 0x1AE:	return TEXT("����");
	case 0xC69:	return TEXT("���");
	case 0xE38:	return TEXT("�Ȗ�");
	case 0x98B:	return TEXT("�Q�n");
	case 0x64B:	return TEXT("���");
	case 0x1C7:	return TEXT("��t");
	case 0xAAC:	return TEXT("����");
	case 0x56C:	return TEXT("�_�ސ�");
	case 0x4CE:	return TEXT("�V��");
	case 0x539:	return TEXT("�x�R");
	case 0x6A6:	return TEXT("�ΐ�");
	case 0x92D:	return TEXT("����");
	case 0xD4A:	return TEXT("�R��");
	case 0x9D2:	return TEXT("����");
	case 0xA65:	return TEXT("��");
	case 0xA5A:	return TEXT("�É�");
	case 0x966:	return TEXT("���m");
	case 0x2DC:	return TEXT("�O�d");
	case 0xCE4:	return TEXT("����");
	case 0x59A:	return TEXT("���s");
	case 0xCB2:	return TEXT("���");
	case 0x674:	return TEXT("����");
	case 0xA93:	return TEXT("�ޗ�");
	case 0x396:	return TEXT("�a�̎R");
	case 0xD23:	return TEXT("����");
	case 0x31B:	return TEXT("����");
	case 0x2B5:	return TEXT("���R");
	case 0xB31:	return TEXT("�L��");
	case 0xB98:	return TEXT("�R��");
	case 0xE62:	return TEXT("����");
	case 0x9B4:	return TEXT("����");
	case 0x19D:	return TEXT("���Q");
	case 0x2E3:	return TEXT("���m");
	case 0x62D:	return TEXT("����");
	case 0x959:	return TEXT("����");
	case 0xA2B:	return TEXT("����");
	case 0x8A7:	return TEXT("�F�{");
	case 0xC8D:	return TEXT("�啪");
	case 0xD1C:	return TEXT("�{��");
	case 0xD45:	return TEXT("������");
	case 0x372:	return TEXT("����");
	}
	return TEXT("?");
}

void CStreamInfo::SetService()
{
	CDtvEngine &DtvEngine=GetAppClass().GetCoreEngine()->m_DtvEngine;
	CTsAnalyzer *pAnalyzer=&DtvEngine.m_TsAnalyzer;
	TCHAR szText[1024];
	int Length;

	const WORD TSID=pAnalyzer->GetTransportStreamID();
	if (TSID!=0) {
		Length=StdUtil::snprintf(szText,lengthof(szText),TEXT("TSID 0x%04x (%d)"),TSID,TSID);
		TCHAR szTsName[64];
		if (pAnalyzer->GetTsName(szTsName,lengthof(szTsName))>0) {
			StdUtil::snprintf(szText+Length,lengthof(szText)-Length,TEXT(" %s"),szTsName);
		}
	} else {
		szText[0]='\0';
	}
	::SetDlgItemText(m_hDlg,IDC_STREAMINFO_STREAM,szText);

	const WORD NID=pAnalyzer->GetNetworkID();
	if (NID!=0) {
		Length=StdUtil::snprintf(szText,lengthof(szText),TEXT("NID 0x%04x (%d)"),NID,NID);
		TCHAR szName[64];
		if (pAnalyzer->GetNetworkName(szName,lengthof(szName))>0) {
			StdUtil::snprintf(szText+Length,lengthof(szText)-Length,TEXT(" %s"),szName);
		}
	} else {
		szText[0]='\0';
	}
	::SetDlgItemText(m_hDlg,IDC_STREAMINFO_NETWORK,szText);

	CTsAnalyzer::ServiceList ServiceList;
	pAnalyzer->GetServiceList(&ServiceList);

	HWND hwndTree=::GetDlgItem(m_hDlg,IDC_STREAMINFO_SERVICE);
	TVINSERTSTRUCT tvis;
	HTREEITEM hItem;

	TreeView_DeleteAllItems(hwndTree);

	// �T�[�r�X�ꗗ
	tvis.hParent=TVI_ROOT;
	tvis.hInsertAfter=TVI_LAST;
	tvis.item.mask=TVIF_STATE | TVIF_TEXT | TVIF_CHILDREN;
	tvis.item.state=TVIS_EXPANDED;
	tvis.item.stateMask=(UINT)-1;
	tvis.item.pszText=TEXT("�T�[�r�X");
	tvis.item.cChildren=!ServiceList.empty()?1:0;
	hItem=TreeView_InsertItem(hwndTree,&tvis);
	if (hItem!=NULL) {
		int i,j;

		for (i=0;i<(int)ServiceList.size();i++) {
			const CTsAnalyzer::ServiceInfo &ServiceInfo=ServiceList[i];
			WORD ServiceID,PID;

			tvis.hParent=hItem;
			tvis.item.state=0;
			tvis.item.cChildren=1;
			Length=StdUtil::snprintf(szText,lengthof(szText),TEXT("�T�[�r�X%d"),i+1);
			if (ServiceInfo.szServiceName[0]!='\0')
				Length+=StdUtil::snprintf(szText+Length,lengthof(szText)-Length,
										  TEXT(" (%s)"),ServiceInfo.szServiceName);
			ServiceID=ServiceInfo.ServiceID;
			Length+=StdUtil::snprintf(szText+Length,lengthof(szText)-Length,
									  TEXT(" : SID 0x%04x (%d)"),ServiceID,ServiceID);
			if (ServiceInfo.ServiceType!=0xFF) {
				StdUtil::snprintf(szText+Length,lengthof(szText)-Length,
								  TEXT(" / Type 0x%02x"),ServiceInfo.ServiceType);
			}
			tvis.item.pszText=szText;
			tvis.hParent=TreeView_InsertItem(hwndTree,&tvis);

			tvis.item.cChildren=0;
			PID=ServiceInfo.PmtPID;
			StdUtil::snprintf(szText,lengthof(szText),TEXT("PMT : PID 0x%04x (%d)"),PID,PID);
			TreeView_InsertItem(hwndTree,&tvis);

			PID=ServiceInfo.VideoEs.PID;
			if (PID!=CTsAnalyzer::PID_INVALID) {
				BYTE StreamType=ServiceInfo.VideoStreamType;
				StdUtil::snprintf(szText,lengthof(szText),
					TEXT("�f�� : PID 0x%04x (%d) / Type 0x%02x (%s) / Component tag 0x%02x"),
					PID,PID,StreamType,GetStreamTypeText(StreamType),
					ServiceInfo.VideoEs.ComponentTag);
				TreeView_InsertItem(hwndTree,&tvis);
			}

			int NumAudioStreams=(int)ServiceInfo.AudioEsList.size();
			for (j=0;j<NumAudioStreams;j++) {
				PID=ServiceInfo.AudioEsList[j].PID;
				StdUtil::snprintf(szText,lengthof(szText),
								  TEXT("����%d : PID 0x%04x (%d) / Component tag 0x%02x"),
								  j+1,PID,PID,ServiceInfo.AudioEsList[j].ComponentTag);
				TreeView_InsertItem(hwndTree,&tvis);
			}

			int NumCaptionStreams=(int)ServiceInfo.CaptionEsList.size();
			for (j=0;j<NumCaptionStreams;j++) {
				PID=ServiceInfo.CaptionEsList[j].PID;
				StdUtil::snprintf(szText,lengthof(szText),
								  TEXT("����%d : PID 0x%04x (%d) / Component tag 0x%02x"),
								  j+1,PID,PID,ServiceInfo.CaptionEsList[j].ComponentTag);
				TreeView_InsertItem(hwndTree,&tvis);
			}

			int NumDataStreams=(int)ServiceInfo.DataCarrouselEsList.size();
			for (j=0;j<NumDataStreams;j++) {
				PID=ServiceInfo.DataCarrouselEsList[j].PID;
				StdUtil::snprintf(szText,lengthof(szText),
								  TEXT("�f�[�^%d : PID 0x%04x (%d) / Component tag 0x%02x"),
								  j+1,PID,PID,ServiceInfo.DataCarrouselEsList[j].ComponentTag);
				TreeView_InsertItem(hwndTree,&tvis);
			}

			PID=ServiceInfo.PcrPID;
			if (PID!=CTsAnalyzer::PID_INVALID) {
				StdUtil::snprintf(szText,lengthof(szText),TEXT("PCR : PID 0x%04x (%d)"),PID,PID);
				TreeView_InsertItem(hwndTree,&tvis);
			}

			int NumEcmStreams=(int)ServiceInfo.EcmList.size();
			for (j=0;j<NumEcmStreams;j++) {
				PID=ServiceInfo.EcmList[j].PID;
				StdUtil::snprintf(szText,lengthof(szText),
								  TEXT("ECM%d : PID 0x%04x (%d) / CA system ID 0x%02x"),
								  j+1,PID,PID,ServiceInfo.EcmList[j].CaSystemID);
				TreeView_InsertItem(hwndTree,&tvis);
			}
		}
	}

	// �`�����l���t�@�C���p�t�H�[�}�b�g�ꗗ
	const CChannelInfo *pChannelInfo=GetAppClass().GetChannelManager()->GetCurrentChannelInfo();
	if (pChannelInfo!=NULL) {
		tvis.hParent=TVI_ROOT;
		tvis.hInsertAfter=TVI_LAST;
		tvis.item.mask=TVIF_STATE | TVIF_TEXT | TVIF_CHILDREN;
		tvis.item.state=TVIS_EXPANDED;
		tvis.item.stateMask=~0U;
		tvis.item.pszText=TEXT("�`�����l���t�@�C���p�t�H�[�}�b�g");
		tvis.item.cChildren=!ServiceList.empty()?1:0;;
		hItem=TreeView_InsertItem(hwndTree,&tvis);
		if (hItem!=NULL) {
			const int RemoteControlKeyID=pAnalyzer->GetRemoteControlKeyID();

			for (int i=0;i<(int)ServiceList.size();i++) {
				const CTsAnalyzer::ServiceInfo &ServiceInfo=ServiceList[i];

				tvis.hParent=hItem;
				tvis.item.state=0;
				tvis.item.cChildren=0;
				if (ServiceInfo.szServiceName[0]!='\0')
					Length=StdUtil::snprintf(szText,lengthof(szText),TEXT("%s"),ServiceInfo.szServiceName);
				else
					Length=StdUtil::snprintf(szText,lengthof(szText),TEXT("�T�[�r�X%d"),i+1);
				StdUtil::snprintf(szText+Length,lengthof(szText)-Length,
								  TEXT(",%d,%d,%d,%d,%d,%d,%d"),
								  pChannelInfo->GetSpace(),
								  pChannelInfo->GetChannelIndex(),
								  RemoteControlKeyID,
								  ServiceInfo.ServiceType,
								  ServiceInfo.ServiceID,NID,TSID);
				tvis.item.pszText=szText;
				TreeView_InsertItem(hwndTree,&tvis);
			}
		}
	}

	// �l�b�g���[�NTS�ꗗ
	CTsAnalyzer::NetworkTsList TsList;
	if (pAnalyzer->GetNetworkTsList(&TsList) && !TsList.empty()) {
		tvis.hParent=TVI_ROOT;
		tvis.hInsertAfter=TVI_LAST;
		tvis.item.mask=TVIF_STATE | TVIF_TEXT | TVIF_CHILDREN;
		tvis.item.state=0;
		tvis.item.stateMask=~0U;
		tvis.item.pszText=TEXT("�l�b�g���[�NTS (NIT)");
		tvis.item.cChildren=1;
		hItem=TreeView_InsertItem(hwndTree,&tvis);
		if (hItem!=NULL) {
			for (size_t i=0;i<TsList.size();i++) {
				const CTsAnalyzer::NetworkTsInfo &TsInfo=TsList[i];

				if (TsInfo.OriginalNetworkID==NID) {
					StdUtil::snprintf(szText,lengthof(szText),
						TEXT("TS%d : TSID 0x%04x (%d)"),
						(int)i+1,
						TsInfo.TransportStreamID,TsInfo.TransportStreamID);
					tvis.hParent=hItem;
					tvis.item.state=0;
					tvis.item.cChildren=1;
					tvis.item.pszText=szText;
					tvis.hParent=TreeView_InsertItem(hwndTree,&tvis);
					tvis.item.cChildren=0;
					for (size_t j=0;j<TsInfo.ServiceList.size();j++) {
						const CTsAnalyzer::NetworkServiceInfo &ServiceInfo=TsInfo.ServiceList[j];
						StdUtil::snprintf(szText,lengthof(szText),
							TEXT("�T�[�r�X%d : SID 0x%04x (%d) / Type 0x%02x"),
							(int)j+1,
							ServiceInfo.ServiceID,ServiceInfo.ServiceID,
							ServiceInfo.ServiceType);
						TreeView_InsertItem(hwndTree,&tvis);
					}
				}
			}
		}
	}

	// �l�b�g���[�N�T�[�r�X�ꗗ
	CTsAnalyzer::SdtTsList SdtList;
	if (pAnalyzer->GetSdtTsList(&SdtList) && !SdtList.empty()) {
		tvis.hParent=TVI_ROOT;
		tvis.hInsertAfter=TVI_LAST;
		tvis.item.mask=TVIF_STATE | TVIF_TEXT | TVIF_CHILDREN;
		tvis.item.state=0;
		tvis.item.stateMask=~0U;
		tvis.item.pszText=TEXT("�l�b�g���[�N�T�[�r�X (SDT)");
		tvis.item.cChildren=1;
		hItem=TreeView_InsertItem(hwndTree,&tvis);
		if (hItem!=NULL) {
			for (size_t i=0;i<SdtList.size();i++) {
				const CTsAnalyzer::SdtTsInfo &TsInfo=SdtList[i];

				if (TsInfo.OriginalNetworkID==NID) {
					StdUtil::snprintf(szText,lengthof(szText),
						TEXT("TS%d : TSID 0x%04x (%d)"),
						(int)i+1,
						TsInfo.TransportStreamID,TsInfo.TransportStreamID);
					tvis.hParent=hItem;
					tvis.item.state=0;
					tvis.item.cChildren=1;
					tvis.item.pszText=szText;
					tvis.hParent=TreeView_InsertItem(hwndTree,&tvis);
					tvis.item.cChildren=0;
					for (size_t j=0;j<TsInfo.ServiceList.size();j++) {
						const CTsAnalyzer::SdtServiceInfo &ServiceInfo=TsInfo.ServiceList[j];
						StdUtil::snprintf(szText,lengthof(szText),
							TEXT("�T�[�r�X%d (%s) : SID 0x%04x (%d) / Type 0x%02x / CA %d"),
							(int)j+1,
							ServiceInfo.szServiceName,
							ServiceInfo.ServiceID,ServiceInfo.ServiceID,
							ServiceInfo.ServiceType,
							ServiceInfo.bFreeCaMode);
						TreeView_InsertItem(hwndTree,&tvis);
					}
				}
			}
		}
	}

	// �n��/�q�����z�V�X�e��
	CTsAnalyzer::TerrestrialDeliverySystemList TerrestrialList;
	if (pAnalyzer->GetTerrestrialDeliverySystemList(&TerrestrialList)) {
		tvis.hParent=TVI_ROOT;
		tvis.hInsertAfter=TVI_LAST;
		tvis.item.mask=TVIF_STATE | TVIF_TEXT | TVIF_CHILDREN;
		tvis.item.state=TVIS_EXPANDED;
		tvis.item.stateMask=~0U;
		tvis.item.pszText=TEXT("�n�㕪�z�V�X�e��");
		tvis.item.cChildren=!TerrestrialList.empty()?1:0;
		hItem=TreeView_InsertItem(hwndTree,&tvis);
		if (hItem!=NULL) {
			for (int i=0;i<(int)TerrestrialList.size();i++) {
				const CTsAnalyzer::TerrestrialDeliverySystemInfo &Info=TerrestrialList[i];

				StdUtil::snprintf(szText,lengthof(szText),
					TEXT("TSID 0x%04x (%d) / �G���A %s / �K�[�h�C���^�[�o�� %s / �`�����[�h %s"),
					Info.TransportStreamID,
					Info.TransportStreamID,
					GetAreaText(Info.AreaCode),
					Info.GuardInterval==0?TEXT("1/32"):
					Info.GuardInterval==1?TEXT("1/16"):
					Info.GuardInterval==2?TEXT("1/8"):
					Info.GuardInterval==3?TEXT("1/4"):TEXT("?"),
					Info.TransmissionMode==0?TEXT("Mode1"):
					Info.TransmissionMode==1?TEXT("Mode2"):
					Info.TransmissionMode==2?TEXT("Mode3"):TEXT("?"));
				tvis.hParent=hItem;
				tvis.item.state=0;
				tvis.item.cChildren=1;
				tvis.item.pszText=szText;
				tvis.hParent=TreeView_InsertItem(hwndTree,&tvis);
				tvis.item.cChildren=0;
				for (size_t j=0;j<TerrestrialList[i].Frequency.size();j++) {
					StdUtil::snprintf(szText,lengthof(szText),TEXT("���g��%d : %d MHz"),
									  (int)j+1,TerrestrialList[i].Frequency[j]/7);
					TreeView_InsertItem(hwndTree,&tvis);
				}
			}
		}
	} else {
		CTsAnalyzer::SatelliteDeliverySystemList SatelliteList;
		if (pAnalyzer->GetSatelliteDeliverySystemList(&SatelliteList)) {
			tvis.hParent=TVI_ROOT;
			tvis.hInsertAfter=TVI_LAST;
			tvis.item.mask=TVIF_STATE | TVIF_TEXT | TVIF_CHILDREN;
			tvis.item.state=0;
			tvis.item.stateMask=~0U;
			tvis.item.pszText=TEXT("�q�����z�V�X�e��");
			tvis.item.cChildren=!SatelliteList.empty()?1:0;
			hItem=TreeView_InsertItem(hwndTree,&tvis);
			if (hItem!=NULL) {
				for (int i=0;i<(int)SatelliteList.size();i++) {
					const CTsAnalyzer::SatelliteDeliverySystemInfo &Info=SatelliteList[i];

					StdUtil::snprintf(szText,lengthof(szText),
									  TEXT("TS%d : TSID 0x%04x (%d) / ���g�� %ld.%05ld GHz"),
									  i+1,
									  Info.TransportStreamID,
									  Info.TransportStreamID,
									  Info.Frequency/100000,
									  Info.Frequency%100000);
					tvis.hParent=hItem;
					tvis.item.state=0;
					tvis.item.cChildren=0;
					tvis.item.pszText=szText;
					TreeView_InsertItem(hwndTree,&tvis);
				}
			}
		}
	}

#if 0
	// �_����m�F�T�[�r�X
	DlgComboBox_Clear(m_hDlg,IDC_STREAMINFO_CONTRACT_SERVICE);
	int ContractCount=0;
	if (!ServiceList.empty()) {
		for (size_t i=0;i<ServiceList.size();i++) {
			const CTsAnalyzer::ServiceInfo &ServiceInfo=ServiceList[i];

			if (ServiceInfo.szServiceName[0]!=_T('\0')
					&& DtvEngine.m_CasProcessor.HasContractInfo(NID,ServiceInfo.ServiceID)) {
				LRESULT Index=DlgComboBox_AddString(m_hDlg,IDC_STREAMINFO_CONTRACT_SERVICE,ServiceInfo.szServiceName);
				DlgComboBox_SetItemData(m_hDlg,IDC_STREAMINFO_CONTRACT_SERVICE,Index,MAKELPARAM(NID,ServiceInfo.ServiceID));
				ContractCount++;
			}
		}
		if (ContractCount>0)
			DlgComboBox_SetCurSel(m_hDlg,IDC_STREAMINFO_CONTRACT_SERVICE,0);
	}
	EnableDlgItems(m_hDlg,IDC_STREAMINFO_CONTRACT_SERVICE,IDC_STREAMINFO_CONTRACT_CHECK,ContractCount>0);

	::SetDlgItemText(m_hDlg,IDC_STREAMINFO_CONTRACT_INFO,TEXT(""));
#endif
}


int CStreamInfo::GetTreeViewText(HWND hwndTree,HTREEITEM hItem,bool fSiblings,LPTSTR pszText,int MaxText,int Level)
{
	if (MaxText<=0)
		return 0;

	TV_ITEM tvi;
	LPTSTR p=pszText;

	tvi.mask=TVIF_TEXT;
	tvi.hItem=hItem;
	while (tvi.hItem!=NULL && MaxText>2) {
		if (Level>0) {
			if (MaxText<=Level)
				break;
			for (int i=0;i<Level;i++)
				*p++=_T('\t');
			MaxText-=Level;
		}
		tvi.pszText=p;
		tvi.cchTextMax=MaxText;
		if (TreeView_GetItem(hwndTree,&tvi)) {
			int Len=::lstrlen(p);
			p+=Len;
			*p++=_T('\r');
			*p++=_T('\n');
			MaxText-=Len+2;
		}
		HTREEITEM hChild=TreeView_GetChild(hwndTree,tvi.hItem);
		if (hChild!=NULL) {
			int Length=GetTreeViewText(hwndTree,hChild,true,p,MaxText,Level+1);
			p+=Length;
			MaxText-=Length;
		}
		if (!fSiblings)
			break;
		tvi.hItem=TreeView_GetNextSibling(hwndTree,tvi.hItem);
	}
	*p=_T('\0');
	return (int)(p-pszText);
}
