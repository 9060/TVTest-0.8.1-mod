#include <windows.h>
#include <tchar.h>
#include "TVTest_Image.h"
#include "ImageUtil.h"
#include "BMP.h"
#include "ImageUtil.h"




bool SaveBMPFile(const ImageSaveInfo *pInfo)
{
	int Width,Height,BitsPerPixel;
	HANDLE hFile;
	DWORD dwWrite;
	SIZE_T InfoBytes,RowBytes,BitsBytes;

	Width=pInfo->pbmi->bmiHeader.biWidth;
	Height=abs(pInfo->pbmi->bmiHeader.biHeight);
	BitsPerPixel=pInfo->pbmi->bmiHeader.biBitCount;
	InfoBytes=sizeof(BITMAPINFOHEADER);
	if (BitsPerPixel<=8)
		InfoBytes+=(1<<BitsPerPixel)*sizeof(RGBQUAD);
	else if (pInfo->pbmi->bmiHeader.biCompression==BI_BITFIELDS)
		InfoBytes+=3*sizeof(DWORD);
	RowBytes=DIB_ROW_BYTES(Width,BitsPerPixel);
	BitsBytes=RowBytes*Height;
	hFile=CreateFile(pInfo->pszFileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,
												FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile==INVALID_HANDLE_VALUE) {
		return false;
	}
	/* BITMAPFILEHEADER�̏������� */
	BITMAPFILEHEADER bmfh;

	bmfh.bfType=0x4D42;
	bmfh.bfSize=sizeof(BITMAPFILEHEADER)+InfoBytes+BitsBytes;
	bmfh.bfReserved1=bmfh.bfReserved2=0;
	bmfh.bfOffBits=sizeof(BITMAPFILEHEADER)+InfoBytes;
	if (!WriteFile(hFile,&bmfh,sizeof(BITMAPFILEHEADER),&dwWrite,NULL)
										|| dwWrite!=sizeof(BITMAPFILEHEADER)) {
		CloseHandle(hFile);
		return false;
	}
	/* �w�b�_���������� */
	BITMAPINFOHEADER bmih;

	bmih.biSize=sizeof(BITMAPINFOHEADER);
	bmih.biWidth=Width;
	bmih.biHeight=Height;
	bmih.biPlanes=1;
	bmih.biBitCount=BitsPerPixel;
	bmih.biCompression=pInfo->pbmi->bmiHeader.biCompression==BI_BITFIELDS?
														BI_BITFIELDS:BI_RGB;
	bmih.biSizeImage=0;
	bmih.biXPelsPerMeter=0;
	bmih.biYPelsPerMeter=0;
	bmih.biClrUsed=0;
	bmih.biClrImportant=0;
	if (!WriteFile(hFile,&bmih,sizeof(BITMAPINFOHEADER),&dwWrite,NULL)
										|| dwWrite!=sizeof(BITMAPINFOHEADER)) {
		CloseHandle(hFile);
		return false;
	}
	if (InfoBytes>sizeof(BITMAPINFOHEADER)) {
		SIZE_T PalBytes=InfoBytes-sizeof(BITMAPINFOHEADER);

		if (!WriteFile(hFile,pInfo->pbmi->bmiColors,PalBytes,
										&dwWrite,NULL) || dwWrite!=PalBytes) {
			CloseHandle(hFile);
			return false;
		}
	}
	/* �r�b�g�f�[�^���������� */
	if (pInfo->pbmi->bmiHeader.biHeight>0) {
		if (!WriteFile(hFile,pInfo->pBits,BitsBytes,&dwWrite,NULL)
													|| dwWrite!=BitsBytes) {
			CloseHandle(hFile);
			return false;
		}
	} else {
		int y;
		const BYTE *p;

		p=static_cast<const BYTE*>(pInfo->pBits)+(Height-1)*RowBytes;
		for (y=0;y<Height;y++) {
			if (!WriteFile(hFile,p,RowBytes,&dwWrite,NULL)
													|| dwWrite!=RowBytes) {
				CloseHandle(hFile);
				return false;
			}
			p-=RowBytes;
		}
	}
	CloseHandle(hFile);
	return true;
}
