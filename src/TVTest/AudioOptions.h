#ifndef AUDIO_OPTIONS_H
#define AUDIO_OPTIONS_H


#include "Options.h"


class CAudioOptions : public COptions {
	enum { MAX_AUDIO_DEVICE_NAME=128 };
	TCHAR m_szAudioDeviceName[MAX_AUDIO_DEVICE_NAME];
	bool m_fDownMixSurround;
	bool m_fRestoreMute;
	bool m_fUseAudioRendererClock;
	static CAudioOptions *GetThis(HWND hDlg);

public:
	enum {
		UPDATE_CLOCK
	};

	CAudioOptions();
	~CAudioOptions();
	LPCTSTR GetAudioDeviceName() const { return m_szAudioDeviceName; }
	bool GetDownMixSurround() const { return m_fDownMixSurround; }
	bool GetRestoreMute() const { return m_fRestoreMute; }
	bool GetUseAudioRendererClock() const { return m_fUseAudioRendererClock; }
	static BOOL CALLBACK DlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
	// COptions
	bool Apply(DWORD Flags);
	bool Read(CSettings *pSettings);
	bool Write(CSettings *pSettings) const;
};


#endif
