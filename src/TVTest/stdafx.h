// stdafx.h : �W���̃V�X�e�� �C���N���[�h �t�@�C���̃C���N���[�h �t�@�C���A�܂���
// �Q�Ɖ񐔂������A�����܂�ύX����Ȃ��A�v���W�F�N�g��p�̃C���N���[�h �t�@�C��
// ���L�q���܂��B

#pragma once

// ���Ŏw�肳�ꂽ��`�̑O�ɑΏۃv���b�g�t�H�[�����w�肵�Ȃ���΂Ȃ�Ȃ��ꍇ�A�ȉ��̒�`��ύX���Ă��������B
// �قȂ�v���b�g�t�H�[���ɑΉ�����l�Ɋւ���ŐV���ɂ��ẮAMSDN ���Q�Ƃ��Ă��������B
#ifndef WINVER				// Windows XP �ȍ~�̃o�[�W�����ɌŗL�̋@�\�̎g�p�������܂��B
#define WINVER 0x0501		// ����� Windows �̑��̃o�[�W���������ɓK�؂Ȓl�ɕύX���Ă��������B
#endif

#ifndef _WIN32_WINNT		// Windows XP �ȍ~�̃o�[�W�����ɌŗL�̋@�\�̎g�p�������܂��B
#define _WIN32_WINNT 0x0501	// ����� Windows �̑��̃o�[�W���������ɓK�؂Ȓl�ɕύX���Ă��������B
#endif

#ifndef _WIN32_WINDOWS		// Windows 98 �ȍ~�̃o�[�W�����ɌŗL�̋@�\�̎g�p�������܂��B
#define _WIN32_WINDOWS 0x0410 // ����� Windows Me �܂��͂���ȍ~�̃o�[�W���������ɓK�؂Ȓl�ɕύX���Ă��������B
#endif

#ifndef _WIN32_IE			// IE 6.0 �ȍ~�̃o�[�W�����ɌŗL�̋@�\�̎g�p�������܂��B
#define _WIN32_IE 0x0600	// ����� IE �̑��̃o�[�W���������ɓK�؂Ȓl�ɕύX���Ă��������B
#endif

// Winsock2 �ƃw�b�_�������Ȃ��悤�ɂ���
#define _WINSOCKAPI_

#define _WIN32_DCOM	// for CoInitializeEx()

#include <stdio.h>
#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <streams.h>	// DirectShow BaseClasses

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#undef strdup
#define strdup strdup
#define DEBUG_NEW new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif // _DEBUG

#undef _WINSOCKAPI_

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


// �x���̖����ݒ�
#pragma warning(disable: 4355) // warning C4355: "'this' : �x�[�X �����o���������X�g�Ŏg�p����܂����B"
#pragma warning(disable: 4995) // warning C4995: ���O��������ꂽ #pragma �Ƃ��ċL�q����Ă��܂��B
#pragma warning(disable: 4996) // warning C4996: "This function or variable may be unsafe."


// ���C�u�����̃����N
#pragma comment(lib, "Kernel32.lib")
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "ComCtl32.lib")
#pragma comment(lib, "ComDlg32.lib")
#pragma comment(lib, "Strmiids.lib")
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "ShLwApi.lib")

#ifdef _DEBUG
#pragma comment(lib, "StrmBasd.lib")
#else
#pragma comment(lib, "StrmBase.lib")
#endif

// �g���[�X�o��
#ifdef _DEBUG
	#undef TRACE
	#define TRACE ::DebugTrace
	void DebugTrace(LPCTSTR szFormat, ...);
#else
	#define TRACE __noop
#endif


// BonTsEngine �̐ݒ�
#ifdef TVH264
#ifndef TVH264_FOR_HD
#define BONTSENGINE_1SEG_SUPPORT	// �����Z�O�Ή�
#define BONTSENGINE_RADIO_SUPPORT	// ���������Ή�
#endif
#define BONTSENGINE_H264_SUPPORT	// H.264 �Ή�
#else	// TVH264
#define BONTSENGINE_MPEG2_SUPPORT	// MPEG-2 �Ή�
#endif
