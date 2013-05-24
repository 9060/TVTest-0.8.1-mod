#ifndef __EPG_DATA_CAP_DEF_H__
#define __EPG_DATA_CAP_DEF_H__

#define ERR_FALSE FALSE //�ėp�G���[
#define NO_ERR TRUE //����
#define ERR_INIT		10
#define ERR_NOT_INIT	11
#define ERR_SIZE		12

#define NO_ERR_EPG_ALL 100 //EPG��񒙂܂��� Basic��Extend����
#define NO_ERR_EPG_BASIC 101 //EPG��񒙂܂��� Basic�̂�
#define NO_ERR_EPG_EXTENDED 102 //EPG��񒙂܂��� Extend�̂�

typedef struct _EPG_DATA_INFO{
	DWORD dwServiceID;
	DWORD dwEventID;
	WCHAR* lpwszEventName;
	DWORD dwEventNameLength;
	WCHAR* lpwszEventText;
	DWORD dwEventTextLength;
	WCHAR* lpwszEventExtText;
	DWORD dwEventExtTextLength;
	SYSTEMTIME stStartTime;
	DWORD dwDurationSec;
	BOOL bUnkownStart;
	BOOL bUnkownDure;
}EPG_DATA_INFO;

typedef struct _SERVICE_INFO{
	DWORD dwTSID;
	DWORD dwOriginalNID;
	DWORD dwServiceID;
	WCHAR* lpwszServiceName;
	DWORD dwServiceNameLength;
	DWORD dwServiceType;
	DWORD dwNetworkMode;
}SERVICE_INFO;

typedef struct _EPG_DATA_INFO2{
	DWORD dwOriginalNID;
	DWORD dwTSID;
	DWORD dwServiceID;
	DWORD dwEventID;
	WCHAR* lpwszEventName;
	DWORD dwEventNameLength;
	WCHAR* lpwszEventText;
	DWORD dwEventTextLength;
	WCHAR* lpwszEventExtText;
	DWORD dwEventExtTextLength;
	SYSTEMTIME stStartTime;
	DWORD dwDurationSec;
	unsigned char ucComponentType;
	DWORD dwComponentTypeTextLength;
	WCHAR* lpwszComponentTypeText;
	unsigned char ucAudioComponentType;
	unsigned char ucESMultiLangFlag;
	unsigned char ucMainComponentFlag;
	unsigned char ucSamplingRate;
	DWORD dwAudioComponentTypeTextLength;
	WCHAR* lpwszAudioComponentTypeText;
	DWORD dwContentNibbleListCount;
	DWORD* dwContentNibbleList;
}EPG_DATA_INFO2;

#endif
