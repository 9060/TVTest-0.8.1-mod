#include "stdafx.h"
#include "TVTest.h"
#include "EpgUtil.h"
#include "AppMain.h"
#include "resource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif




namespace EpgUtil
{

	VideoType GetVideoType(BYTE ComponentType)
	{
		if (ComponentType>=0xB1
				&& ComponentType<=0xB4)
			return VIDEO_TYPE_HD;
		if (ComponentType>=0x01
				&& ComponentType<=0x04)
			return VIDEO_TYPE_SD;
		return VIDEO_TYPE_UNKNOWN;
	}


	LPCTSTR GetVideoComponentTypeText(BYTE ComponentType)
	{
		static const struct {
			BYTE ComponentType;
			LPCTSTR pszText;
		} VideoComponentTypeList[] = {
			{0x01,TEXT("480i[4:3]")},
			{0x03,TEXT("480i[16:9]")},
			{0x04,TEXT("480i[>16:9]")},
			{0xA1,TEXT("480p[4:3]")},
			{0xA3,TEXT("480p[16:9]")},
			{0xA4,TEXT("480p[>16:9]")},
			{0xB1,TEXT("1080i[4:3]")},
			{0xB3,TEXT("1080i[16:9]")},
			{0xB4,TEXT("1080i[>16:9]")},
			{0xC1,TEXT("720p[4:3]")},
			{0xC3,TEXT("720p[16:9]")},
			{0xC4,TEXT("720p[>16:9]")},
			{0xD1,TEXT("240p[4:3]")},
			{0xD3,TEXT("240p[16:9]")},
			{0xD4,TEXT("240p[>16:9]")},
		};

		for (int i=0;i<lengthof(VideoComponentTypeList);i++) {
			if (VideoComponentTypeList[i].ComponentType==ComponentType)
				return VideoComponentTypeList[i].pszText;
		}

		return NULL;
	}


	LPCTSTR GetAudioComponentTypeText(BYTE ComponentType)
	{
		static const struct {
			BYTE ComponentType;
			LPCTSTR pszText;
		} AudioComponentTypeList[] = {
			{0x01,TEXT("Mono")},
			{0x02,TEXT("Dual mono")},
			{0x03,TEXT("Stereo")},
			{0x07,TEXT("3+1")},
			{0x08,TEXT("3+2")},
			{0x09,TEXT("5.1ch")},
		};

		for (int i=0;i<lengthof(AudioComponentTypeList);i++) {
			if (AudioComponentTypeList[i].ComponentType==ComponentType)
				return AudioComponentTypeList[i].pszText;
		}

		return NULL;
	}


	int FormatEventTime(const CEventInfoData *pEventInfo,
						LPTSTR pszTime,int MaxLength,unsigned int Flags)
	{
		if (pszTime==NULL || MaxLength<1)
			return 0;

		if (pEventInfo==NULL || !pEventInfo->m_fValidStartTime) {
			pszTime[0]=_T('\0');
			return 0;
		}

		return FormatEventTime(pEventInfo->m_stStartTime,pEventInfo->m_DurationSec,
							   pszTime,MaxLength,Flags);
	}


	int FormatEventTime(const SYSTEMTIME &StartTime,DWORD Duration,
						LPTSTR pszTime,int MaxLength,unsigned int Flags)
	{
		if (pszTime==NULL || MaxLength<1)
			return 0;

		TCHAR szDate[32];
		if ((Flags & EVENT_TIME_DATE)!=0) {
			int Length=0;
			if ((Flags & EVENT_TIME_YEAR)!=0) {
				Length=StdUtil::snprintf(szDate,lengthof(szDate),TEXT("%d/"),
										 StartTime.wYear);
			}
			StdUtil::snprintf(szDate+Length,lengthof(szDate)-Length,
							  TEXT("%d/%d(%s) "),
							  StartTime.wMonth,
							  StartTime.wDay,
							  GetDayOfWeekText(StartTime.wDayOfWeek));
		} else {
			szDate[0]=_T('\0');
		}

		LPCTSTR pszTimeFormat=
			(Flags & EVENT_TIME_HOUR_2DIGITS)!=0?TEXT("%02d:%02d"):TEXT("%d:%02d");
		TCHAR szStartTime[32],szEndTime[32];

		StdUtil::snprintf(szStartTime,lengthof(szStartTime),
						  pszTimeFormat,
						  StartTime.wHour,
						  StartTime.wMinute);

		szEndTime[0]=_T('\0');
		if ((Flags & EVENT_TIME_START_ONLY)==0) {
			if (Duration>0) {
				SYSTEMTIME EndTime=StartTime;
				if (OffsetSystemTime(&EndTime,Duration*1000)) {
					StdUtil::snprintf(szEndTime,lengthof(szEndTime),pszTimeFormat,
									  EndTime.wHour,EndTime.wMinute);
				}
			} else {
				if ((Flags & EVENT_TIME_UNDECIDED_TEXT)!=0)
					::lstrcpy(szEndTime,TEXT("(�I������)"));
			}
		}

		return StdUtil::snprintf(pszTime,MaxLength,TEXT("%s%s%s%s"),
								 szDate,
								 szStartTime,
								 (Flags & EVENT_TIME_START_ONLY)==0?TEXT("�`"):TEXT(""),
								 szEndTime);
	}


	LPCTSTR GetLanguageText(DWORD LanguageCode,LanguageTextType Type)
	{
		static const struct {
			DWORD LanguageCode;
			LPCTSTR pszLongText;
			LPCTSTR pszSimpleText;
			LPCTSTR pszShortText;
		} LanguageList[] = {
			{LANGUAGE_CODE_JPN,	TEXT("���{��"),		TEXT("���{��"),	TEXT("��")},
			{LANGUAGE_CODE_ENG,	TEXT("�p��"),		TEXT("�p��"),	TEXT("�p")},
			{LANGUAGE_CODE_DEU,	TEXT("�h�C�c��"),	TEXT("�ƌ�"),	TEXT("��")},
			{LANGUAGE_CODE_FRA,	TEXT("�t�����X��"),	TEXT("����"),	TEXT("��")},
			{LANGUAGE_CODE_ITA,	TEXT("�C�^���A��"),	TEXT("�Ɍ�"),	TEXT("��")},
			{LANGUAGE_CODE_RUS,	TEXT("���V�A��"),	TEXT("�I��"),	TEXT("�I")},
			{LANGUAGE_CODE_ZHO,	TEXT("������"),		TEXT("������"),	TEXT("��")},
			{LANGUAGE_CODE_KOR,	TEXT("�؍���"),		TEXT("�؍���"),	TEXT("��")},
			{LANGUAGE_CODE_SPA,	TEXT("�X�y�C����"),	TEXT("����"),	TEXT("��")},
			{LANGUAGE_CODE_ETC,	TEXT("�O����"),		TEXT("�O����"),	TEXT("�O")},
		};

		int i;
		for (i=0;i<lengthof(LanguageList)-1;i++) {
			if (LanguageList[i].LanguageCode==LanguageCode)
				break;
		}
		switch (Type) {
		case LANGUAGE_TEXT_LONG:	return LanguageList[i].pszLongText;
		case LANGUAGE_TEXT_SIMPLE:	return LanguageList[i].pszSimpleText;
		case LANGUAGE_TEXT_SHORT:	return LanguageList[i].pszShortText;
		}
		return TEXT("");
	}

}




LPCTSTR CEpgGenre::GetText(int Level1,int Level2) const
{
	static const struct {
		LPCTSTR pszText;
		LPCTSTR pszSubText[16];
	} GenreList[] = {
		{TEXT("�j���[�X�^��"),
			{
				TEXT("�莞�E����"),
				TEXT("�V�C"),
				TEXT("���W�E�h�L�������g"),
				TEXT("�����E����"),
				TEXT("�o�ρE�s��"),
				TEXT("�C�O�E����"),
				TEXT("���"),
				TEXT("���_�E��k"),
				TEXT("�񓹓���"),
				TEXT("���[�J���E�n��"),
				TEXT("���"),
				NULL,
				NULL,
				NULL,
				NULL,
				TEXT("���̑�")
			}
		},
		{TEXT("�X�|�[�c"),
			{
				TEXT("�X�|�[�c�j���[�X"),
				TEXT("�싅"),
				TEXT("�T�b�J�["),
				TEXT("�S���t"),
				TEXT("���̑��̋��Z"),
				TEXT("���o�E�i���Z"),
				TEXT("�I�����s�b�N�E���ۑ��"),
				TEXT("�}���\���E����E���j"),
				TEXT("���[�^�[�X�|�[�c"),
				TEXT("�}�����E�E�B���^�[�X�|�[�c"),
				TEXT("���n�E���c���Z"),
				NULL,
				NULL,
				NULL,
				NULL,
				TEXT("���̑�")
			}
		},
		{TEXT("���^���C�h�V���["),
			{
				TEXT("�|�\�E���C�h�V���["),
				TEXT("�t�@�b�V����"),
				TEXT("��炵�E�Z�܂�"),
				TEXT("���N�E���"),
				TEXT("�V���b�s���O�E�ʔ�"),
				TEXT("�O�����E����"),
				TEXT("�C�x���g"),
				TEXT("�ԑg�Љ�E���m�点"),
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				TEXT("���̑�")
			}
		},
		{TEXT("�h���}"),
			{
				TEXT("�����h���}"),
				TEXT("�C�O�h���}"),
				TEXT("���㌀"),
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				TEXT("���̑�")
			}
		},
		{TEXT("���y"),
			{
				TEXT("�������b�N�E�|�b�v�X"),
				TEXT("�C�O���b�N�E�|�b�v�X"),
				TEXT("�N���V�b�N�E�I�y��"),
				TEXT("�W���Y�E�t���[�W����"),
				TEXT("�̗w�ȁE����"),
				TEXT("���C�u�E�R���T�[�g"),
				TEXT("�����L���O�E���N�G�X�g"),
				TEXT("�J���I�P�E�̂ǎ���"),
				TEXT("���w�E�M�y"),
				TEXT("���w�E�L�b�Y"),
				TEXT("�������y�E���[���h�~���[�W�b�N"),
				NULL,
				NULL,
				NULL,
				NULL,
				TEXT("���̑�")
			}
		},
		{TEXT("�o���G�e�B"),
			{
				TEXT("�N�C�Y"),
				TEXT("�Q�[��"),
				TEXT("�g�[�N�o���G�e�B"),
				TEXT("���΂��E�R���f�B"),
				TEXT("���y�o���G�e�B"),
				TEXT("���o���G�e�B"),
				TEXT("�����o���G�e�B"),
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				TEXT("���̑�")
			}
		},
		{TEXT("�f��"),
			{
				TEXT("�m��"),
				TEXT("�M��"),
				TEXT("�A�j��"),
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				TEXT("���̑�")
			}
		},
		{TEXT("�A�j���^���B"),
			{
				TEXT("�����A�j��"),
				TEXT("�C�O�A�j��"),
				TEXT("���B"),
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				TEXT("���̑�")
			}
		},
		{TEXT("�h�L�������^���[�^���{"),
			{
				TEXT("�Љ�E����"),
				TEXT("���j�E�I�s"),
				TEXT("���R�E�����E��"),
				TEXT("�F���E�Ȋw�E��w"),
				TEXT("�J���`���[�E�`������"),
				TEXT("���w�E���|"),
				TEXT("�X�|�[�c"),
				TEXT("�h�L�������^���[�S��"),
				TEXT("�C���^�r���[�E���_"),
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				TEXT("���̑�")
			}
		},
		{TEXT("����^����"),
			{
				TEXT("���㌀�E�V��"),
				TEXT("�~���[�W�J��"),
				TEXT("�_���X�E�o���G"),
				TEXT("����E���|"),
				TEXT("�̕���E�ÓT"),
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				TEXT("���̑�")
			}
		},
		{TEXT("��^����"),
			{
				TEXT("���E�ނ�E�A�E�g�h�A"),
				TEXT("���|�E�y�b�g�E��|"),
				TEXT("���y�E���p�E�H�|"),
				TEXT("�͌�E����"),
				TEXT("�����E�p�`���R"),
				TEXT("�ԁE�I�[�g�o�C"),
				TEXT("�R���s���[�^�ETV�Q�[��"),
				TEXT("��b�E��w"),
				TEXT("�c���E���w��"),
				TEXT("���w���E���Z��"),
				TEXT("��w���E��"),
				TEXT("���U�w�K�E���i"),
				TEXT("������"),
				NULL,
				NULL,
				TEXT("���̑�")
			}
		},
		{TEXT("����"),
			{
				TEXT("�����"),
				TEXT("��Q��"),
				TEXT("�Љ��"),
				TEXT("�{�����e�B�A"),
				TEXT("��b"),
				TEXT("����(����)"),
				TEXT("�������"),
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				TEXT("���̑�")
			}
		},
	};

	if (Level2<0) {
		if (Level1>=0 && Level1<lengthof(GenreList))
			return GenreList[Level1].pszText;
		if (Level1==GENRE_OTHER)
			return TEXT("���̑�");
		return NULL;
	}
	if (Level1>=0 && Level1<lengthof(GenreList)
			&& Level2>=0 && Level2<16)
		return GenreList[Level1].pszSubText[Level2];
	return NULL;
}




bool CEpgIcons::Load()
{
	return CBitmap::Load(GetAppClass().GetInstance(),IDB_PROGRAMGUIDEICONS,LR_DEFAULTCOLOR);
}


bool CEpgIcons::Draw(HDC hdcDst,int DstX,int DstY,
					 HDC hdcSrc,int Icon,int Width,int Height,BYTE Opacity)
{
	if (hdcDst==NULL || hdcSrc==NULL || Icon<0 || Icon>ICON_LAST
			|| Width<=0 || Height<=0)
		return false;
	if (Opacity==255) {
		::BitBlt(hdcDst,DstX,DstY,Width,Height,
				 hdcSrc,Icon*ICON_WIDTH,0,SRCCOPY);
	} else {
		BLENDFUNCTION bf={AC_SRC_OVER,0,Opacity,0};
		::GdiAlphaBlend(hdcDst,DstX,DstY,Width,Height,
						hdcSrc,Icon*ICON_WIDTH,0,Width,Height,
						bf);
	}
	return true;
}


unsigned int CEpgIcons::GetEventIcons(const CEventInfoData *pEventInfo)
{
	unsigned int ShowIcons=0;

	EpgUtil::VideoType Video=EpgUtil::GetVideoType(pEventInfo->m_VideoInfo.ComponentType);
	if (Video==EpgUtil::VIDEO_TYPE_HD)
		ShowIcons|=IconFlag(ICON_HD);
	else if (Video==EpgUtil::VIDEO_TYPE_SD)
		ShowIcons|=IconFlag(ICON_SD);

	if (!pEventInfo->m_AudioList.empty()) {
		const CEventInfoData::AudioInfo *pAudioInfo=pEventInfo->GetMainAudioInfo();

		if (pAudioInfo->ComponentType==0x02) {
			if (pAudioInfo->bESMultiLingualFlag
					&& pAudioInfo->LanguageCode!=pAudioInfo->LanguageCode2)
				ShowIcons|=IconFlag(ICON_MULTILINGUAL);
			else
				ShowIcons|=IconFlag(ICON_SUB);
		} else {
			if (pAudioInfo->ComponentType==0x09)
				ShowIcons|=IconFlag(ICON_5_1CH);
			if (pEventInfo->m_AudioList.size()>=2
					&& pEventInfo->m_AudioList[0].LanguageCode!=0
					&& pEventInfo->m_AudioList[1].LanguageCode!=0) {
				if (pEventInfo->m_AudioList[0].LanguageCode!=
						pEventInfo->m_AudioList[1].LanguageCode)
					ShowIcons|=IconFlag(ICON_MULTILINGUAL);
				else
					ShowIcons|=IconFlag(ICON_SUB);
			}
		}
	}

	if (pEventInfo->m_NetworkID>=4 && pEventInfo->m_NetworkID<=10) {
		if (pEventInfo->m_CaType==CEventInfoData::CA_TYPE_FREE)
			ShowIcons|=IconFlag(ICON_FREE);
		else if (pEventInfo->m_CaType==CEventInfoData::CA_TYPE_CHARGEABLE)
			ShowIcons|=IconFlag(ICON_PAY);
	}

	return ShowIcons;
}
