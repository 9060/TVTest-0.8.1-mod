-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA1

TvtPlay�̔{���Đ���TS�h���b�v�����Œ����ԃt���[�Y���Ȃ��悤��+�N�����t���[�Y����
���悤�ɂ���TVTest
2012-09-26
By TvtPlay���

���T�v
�Y�t��TVTestMod.zip(2sen/hdus/up0637)��TVTestModBuild.zip�̂Â��ł��B������
TVTest.exe���I���W�i����TVTest0.8.0�̂��̂ƒu�������ė��p�ł��܂��B
TVTestModBuild.zip�̓f�b�h���b�N�΍�ƈ���������(���Ԃ�V���A���C�Y��Ԃ��ɒ[��
�������߂�)���ɃV���O���R�A���œ��삪�d���Ȃ錻�ۂ��݂�ꂽ���߁A
AacDecFilter.cpp������ɏC�����܂����B�܂��x�[�X��TVTest0.8.0�ɕύX���܂����B
TVTest0.7.23���߁X���J��~���ꂻ���ȋC������̂ŁA�Y�t��TVTestModBuild.zip�̓o�C
�i�����폜���Ă��܂��B

�r���h���@�͌�q�uTVTest0.8.0�r���h�菇�v���Q�Ƃ��Ă��������B
DBCTRADO(http://dbctrado.allalla.com/)����TVTest0.8.0�̃\�[�X�R�[�h���擾���A
diff_src.zip�̑Ή�����t�@�C���ƒu��������Γ����̂��̂��ł��܂��B�Ȃ��A������
TVTest(orig).exe�͑ΏƎ����̂���diff_src.zip��AacDecFilter.h/.cpp�݂̂�u������
���Ƀr���h�������́A�܂�I���W�i����TVTest.exe�Ɠ������������͂��̂��̂ł��B

�ȉ��̂����ꂩ�̏ꍇ�Ɍ��J���~����\��ł�:
1.TVTest0.8.0�̌��J����~���ꂽ(GPL�𖞂����Ȃ�����)
2.������TVTest.exe�ɒv���I�ȕs����݂�����
3.���̑�(���v���Ȃ��Ȃ����Ȃ�)

��TVTest0.8.0�r���h�菇
DBCTRADO�̐����O�̃r���h���@�Ȃ̂ŁA���̎菇�Ńr���h����TVTest�ł̂ݔ��������G��
�[�Ȃǂ�DBCTRADO�̒��̐l�ɕ񍐂���̂͂�߂Ă��������B

[�p��+�C���X�g�[���������]
�EVisual C++ 2010 Express(�ȉ�VC++)
�EWindows SDK for Windows 7 (�uDirectShow BaseClasses�v�̃r���h�̂���)
�Efaad2-2.7.zip (FAAD2 Source http://www.audiocoding.com ����DL)
�ETVTest_0.8.0.7z�ɂ���TVTest_0.8.0_Src.7z
[�菇]
���e�v���W�F�N�g���r���h����Ƃ��́A�v���W�F�N�g�̃v���p�e�B�́u�����^�C�����C�u
  �����v�̐ݒ����v�����邱��(�łȂ���LNK2005�G���[���N����)
1. �y�uFAAD2�v(libfaad.lib)���r���h�z
   "faad2-2.7.zip"��W�J�A"frontend\faad.sln"��VC++�ŊJ����(�r���A�ϊ��E�B�U�[�h
   ���o��)�r���h����
2. �y�uDirectShow BaseClasses�v(strmbasd.lib strmbase.lib)���r���h�z
   �f�t�H���g�ł� "Program Files\Microsoft SDKs\Windows\v7.1\Samples\multimedia\
   directshow\baseclasses" �t�H���_�ɂ���"baseclasses.sln"���J���ăr���h����
3. "TVTest_0.8.0.7z"�Ƃ��̒��g"TVTest_0.8.0_Src.7z"��W�J����
4. "TVTest.sln"��VC++�ŊJ��
5. TVTest�v���W�F�N�g�̃v���p�e�B���J���A��L"baseclasses"�t�H���_���u�ǉ��̃C��
   �N���[�h�f�B���N�g���v�ɉ�����B�܂��A���t�H���_�ɂ���"Debug"�܂���"Release"
   �t�H���_�A�����"libfaad.lib"���������ꂽ�t�H���_���u�ǉ��̃��C�u�����f�B���N
   �g���v�ɉ�����
6. "VMR9Renderless.cpp"��ATL��CComPtr���g���Ă���̂ŁA15�s�ڂ�����ɉ��L�R�[�h
   �������A5�s�� #include <atlbase.h> ���R�����g�A�E�g

#ifndef __ATLBASE_H__
template <class T> class CComPtr
{
public:
	CComPtr() : p(NULL) {}
	CComPtr(T *lp) : p(lp) { if (p) p->AddRef(); }
	CComPtr(const CComPtr<T> &other) { p = other.p; if (p) p->AddRef(); }
	~CComPtr() { Release(); }
	T *operator=(T *lp) { Attach(lp); if (p) p->AddRef(); return p; }
	T *operator=(const CComPtr<T> &other) { return *this = other.p; }
	bool operator==(T *lp) const { return p == lp; }
	operator T*() const { return p; }
	T **operator&() { _ASSERT(!p); return &p; }
	T *operator->() const { _ASSERT(p); return p; }
	void Release() { if (p) p->Release(); p = NULL; }
	void Attach(T *lp) { Release(); p = lp; }
	HRESULT CopyTo(T **lpp) { _ASSERT(lpp); if (!lpp) return E_POINTER; *lpp = p; if (p) p->AddRef(); return S_OK; }
	T *p;
};
#endif

7. TVTest�v���W�F�N�g�ɂ��Ă͈ȏ�Ńr���h��������͂��BTVH264�����l�Ƀv���p�e
   �B���C������΃r���h�ł���͂�

��SHA-1
f0230c72c40e0f4fd895d20d4e1717317bbe308f *TVTestMod(up0637)(obsolete).zip
e5ffc009e403f5c18c8b435ff2dffb7b331a92a4 *TVTestModBuild(obsolete).zip
b69313ed513f315d2f3f7eb65c0d8c83b4c7574b *diff_src.zip
445444c7050e71b2c47e7131c8ee194bc44084c0 *TVTest(orig).exe
c43eee908cf305fb4dca7b7a83cbfe4b3283ebc1 *TVTest.exe
-----BEGIN PGP SIGNATURE-----
Version: GnuPG v1.4.12 (MingW32)

iQEcBAEBAgAGBQJQYs5/AAoJEOCRC1JRujjtfsQH/1c2X1smbTXUgy0z2adhyWiH
cpfiRsFcXGD0h3kqHvWKNV0XwrIo9QeMx7MhY86ixoD3dtYtbIVlk3a16jZeQ9la
PA1c2SxF0ovmZFNiPucp+NsjKzpxJi3TYj9zzlbKAnc09uiDAcetNA9YGAlw+s57
BXpe3lJXbCJJN96X0P0324eNYKlIR6Ufs6GHvZZjm3y0sqttf06czLoz10tokXNZ
PEZ3VkmmxpMKsVE+ZCB/m8fYxUNNI84cNBF863t31NoDGyDyImP1lo05JRDXk7Tz
G4mPPK2a3YENZ14v5V3yZj+E/fdNLiGhPO4g0GG4UK1K0gZTFLiEFcVdHcSoQF0=
=vRi7
-----END PGP SIGNATURE-----
