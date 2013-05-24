// Multi2Decoder.h: CMulti2Decoder �N���X�̃C���^�[�t�F�C�X
//
//////////////////////////////////////////////////////////////////////

#pragma once


#define MULTI2_USE_INTRINSIC	// �g�ݍ��݊֐��𗘗p
#define MULTI2_SSE2				// SSE2�Ή�
#define MULTI2_SSE2_ICC			// Intel C++ Compiler �𗘗p����

#if defined(MULTI2_SSE2) && !defined(MULTI2_SSE2_ICC)
#include <emmintrin.h>
#endif


class CMulti2Decoder
{
public:
	CMulti2Decoder(void);
	~CMulti2Decoder(void);

	const bool Initialize(const BYTE *pSystemKey, const BYTE *pInitialCbc);
	const bool SetScrambleKey(const BYTE *pScrambleKey);
	const bool Decode(BYTE *pData, const DWORD dwSize, const BYTE byScrCtrl) const;

#ifdef MULTI2_SSE2
	const bool DecodeSSE2(BYTE *pData, const DWORD dwSize, const BYTE byScrCtrl) const;
	typedef const bool (CMulti2Decoder::*DecodeFunc)(BYTE *pData, const DWORD dwSize, const BYTE byScrCtrl) const;
	static bool IsSSE2Available();
#endif

	class SYSKEY	// System Key(Sk), Expanded Key(Wk) 256bit
	{
	public:
		inline void SetHexData(const BYTE *pHexData);
		inline void GetHexData(BYTE *pHexData) const;

		union {
#if !defined(MULTI2_USE_INTRINSIC) || !defined(WIN64)
			struct {
				DWORD dwKey1, dwKey2, dwKey3, dwKey4, dwKey5, dwKey6, dwKey7, dwKey8;
			};
#else
			struct {
				DWORD dwKey2, dwKey1, dwKey4, dwKey3, dwKey6, dwKey5, dwKey8, dwKey7;
			};
			unsigned __int64 Data64[4];
#endif
			BYTE Data[32];
		};
	};

private:
	class DATKEY	// Data Key(Dk) 64bit
	{
	public:
		inline void SetHexData(const BYTE *pHexData);
		inline void GetHexData(BYTE *pHexData) const;
		DATKEY &operator^=(const DATKEY &Operand) {
#ifndef WIN64
			dwRight ^= Operand.dwRight;
			dwLeft ^= Operand.dwLeft;
#else
			Data64 ^= Operand.Data64;
#endif
			return *this;
		}

		union {
			struct {
				DWORD dwRight, dwLeft;
			};
			unsigned __int64 Data64;
			BYTE Data[8];
		};
	};

	static inline void KeySchedule(SYSKEY &WorkKey, const SYSKEY &SysKey, DATKEY &DataKey);

	static inline void RoundFuncPi1(DATKEY &Block);
	static inline void RoundFuncPi2(DATKEY &Block, const DWORD dwK1);
	static inline void RoundFuncPi3(DATKEY &Block, const DWORD dwK2, const DWORD dwK3);
	static inline void RoundFuncPi4(DATKEY &Block, const DWORD dwK4);

	static inline const DWORD LeftRotate(const DWORD dwValue, const DWORD dwRotate);

#if defined(MULTI2_SSE2) && !defined(MULTI2_SSE2_ICC)
	static inline void RoundFuncPi1SSE2(__m128i &Left, __m128i &Right);
	static inline void RoundFuncPi2SSE2(__m128i &Left, __m128i &Right, DWORD Key1);
	static inline void RoundFuncPi3SSE2(__m128i &Left, __m128i &Right, DWORD Key2, DWORD Key3);
	static inline void RoundFuncPi4SSE2(__m128i &Left, __m128i &Right, DWORD Key4);
	static inline __m128i LeftRotate(const __m128i &Value, const __m128i &Rotate, const __m128i &InvRotate);
	static inline __m128i ByteSwap128(const __m128i &Value);
#endif

	DATKEY m_InitialCbc;
	SYSKEY m_SystemKey;
	SYSKEY m_WorkKeyOdd, m_WorkKeyEven;

	bool m_bIsSysKeyValid;
	bool m_bIsWorkKeyValid;
};
