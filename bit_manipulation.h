#pragma once
#include <cstdint>


typedef uint64_t Uint64;
typedef int64_t Sint64;
typedef uint32_t Uint32;
typedef int32_t Sint32;
typedef uint16_t Uint16;
typedef int16_t Sint16;
typedef uint8_t Uint8;
typedef int8_t Sint8;




#if defined(_M_AMD64) or defined(__amd64__)
#  include <intrin.h>
#  include <immintrin.h>



inline Uint64 bsf64(Uint64 x) {
	unsigned long res;
	_BitScanForward64(&res, x);
	return res;
}

inline Uint32 bsf32(Uint32 x) {
	unsigned long res;
	_BitScanForward(&res, x);
	return res;
}

inline Uint64 bsr64(Uint64 x) {
	unsigned long res;
	_BitScanReverse64(&res, x);
	return res;
}

inline Uint32 bsr32(Uint32 x) {
	unsigned long res;
	_BitScanReverse(&res, x);
	return res;
}

inline Uint64 popcount64(Uint64 x) {
	return _mm_popcnt_u64(x);
}

inline Uint32 popcount32(Uint32 x) {
	return _mm_popcnt_u32(x);
}

inline Uint16 popcount16(Uint16 x) {
	return _mm_popcnt_u32(x);
}

inline Uint8 popcount8(Uint8 x) {
	return _mm_popcnt_u32(x);
}


inline Uint64 pext64(Uint64 x, Uint64 mask) {
	return _pext_u64(x, mask);
}

inline Uint32 pext32(Uint32 x, Uint32 mask) {
	return _pext_u32(x, mask);
}

inline Uint64 pdep64(Uint64 x, Uint64 mask) {
	return _pdep_u64(x, mask);
}

inline Uint32 pdep32(Uint32 x, Uint32 mask) {
	return _pdep_u32(x, mask);
}


#define CLEAR_LOWEST_BIT(x) (x = _blsr_u64(x))
#else
#define CLEAR_LOWEST_BIT(x) (x &= (x-1))


static int constexpr __bsf_index64[64] = {
		 0,  1, 48,  2, 57, 49, 28,  3,
		61, 58, 50, 42, 38, 29, 17,  4,
		62, 55, 59, 36, 53, 51, 43, 22,
		45, 39, 33, 30, 24, 18, 12,  5,
		63, 47, 56, 27, 60, 41, 37, 16,
		54, 35, 52, 21, 44, 32, 23, 11,
		46, 26, 40, 15, 34, 20, 31, 10,
		25, 14, 19,  9, 13,  8,  7,  6
};

inline constexpr int bsf64(Uint64 x) {
	Uint64 constexpr debruijn64 = 0x03f79d71b4cb0a89ull;
	return __bsf_index64[((x & static_cast<Uint64>(-static_cast<Sint64>(x))) * debruijn64) >> 58];
}

/**
 * bitScanForward
 * @author Kim Walisch (2012)
 * @param bb bitboard to scan
 * @precondition bb != 0
 * @return index (0..63) of least significant one bit
 */


static int const __bsf_v2_index64[64] = {
		0, 47,  1, 56, 48, 27,  2, 60,
	   57, 49, 41, 37, 28, 16,  3, 61,
	   54, 58, 35, 52, 50, 42, 21, 44,
	   38, 32, 29, 23, 17, 11,  4, 62,
	   46, 55, 26, 59, 40, 36, 15, 53,
	   34, 51, 20, 43, 31, 22, 10, 45,
	   25, 39, 14, 33, 19, 30,  9, 24,
	   13, 18,  8, 12,  7,  6,  5, 63
};

inline constexpr int bsf64_v2(Uint64 bb) {

	Uint64 constexpr debruijn64 = 0x03f79d71b4cb0a89ull;
	return __bsf_v2_index64[((bb ^ (bb - 1)) * debruijn64) >> 58];
}


inline int popcount64(Uint64 x) {
	return __builtin_popcountll(x);
#define MASK1  0b0101010101010101010101010101010101010101010101010101010101010101
#define MASK2  0b0011001100110011001100110011001100110011001100110011001100110011
#define MASK4  0b0000111100001111000011110000111100001111000011110000111100001111
#define MASK8  0b0000000011111111000000001111111100000000111111110000000011111111
#define MASK16 0b0000000000000000111111111111111100000000000000001111111111111111
#define MASK32 0b0000000000000000000000000000000011111111111111111111111111111111

	x = (x & MASK1) + ((x >> 1) & MASK1);
	x = (x & MASK2) + ((x >> 2) & MASK2);
	x = (x & MASK4) + ((x >> 4) & MASK4);
	x = (x & MASK8) + ((x >> 8) & MASK8);
	x = (x & MASK16) + ((x >> 16) & MASK16);
	x = (x & MASK32) + (x >> 32); // the shift already sets the 32 higher bits to 0

	return (int)x;
#undef MASK1
#undef MASK2
#undef MASK4
#undef MASK8
#undef MASK16
#undef MASK32
}

inline int popcount32(Uint32 x) {
	return __builtin_popcountll(x);
#define MASK1  0b01010101010101010101010101010101
#define MASK2  0b00110011001100110011001100110011
#define MASK4  0b00001111000011110000111100001111
#define MASK8  0b00000000111111110000000011111111
#define MASK16 0b00000000000000001111111111111111

	x = (x & MASK1) + ((x >> 1) & MASK1);
	x = (x & MASK2) + ((x >> 2) & MASK2);
	x = (x & MASK4) + ((x >> 4) & MASK4);
	x = (x & MASK8) + ((x >> 8) & MASK8);
	x = (x & MASK16) + (x >> 16); // the shift already sets the 16 higher bits to 0

	return x;
#undef MASK1
#undef MASK2
#undef MASK4
#undef MASK8
#undef MASK16
}

inline int popcount16(Uint16 x) {
	return __builtin_popcount(x);
#define MASK1  0b0101010101010101
#define MASK2  0b0011001100110011
#define MASK4  0b0000111100001111
#define MASK8  0b0000000011111111

	x = (x & MASK1) + ((x >> 1) & MASK1);
	x = (x & MASK2) + ((x >> 2) & MASK2);
	x = (x & MASK4) + ((x >> 4) & MASK4);
	x = (x & MASK8) + (x >> 8); // the shift already sets the 8 higher bits to 0

	return x;
#undef MASK1
#undef MASK2
#undef MASK4
#undef MASK8
}

inline int popcount8(Uint8 x) {
	return __builtin_popcount(x);
#define MASK1  0b01010101
#define MASK2  0b00110011
#define MASK4  0b00001111

	x = (x & MASK1) + ((x >> 1) & MASK1);
	x = (x & MASK2) + ((x >> 2) & MASK2);
	x = (x & MASK4) + (x >> 4); // the shift already sets the 4 higher bits to 0

	return x;
#undef MASK1
#undef MASK2
#undef MASK4
}









#endif






/**
 * bitScanForward
 * @author Martin Läuter (1997)
 *         Charles E. Leiserson
 *         Harald Prokop
 *         Keith H. Randall
 * "Using de Bruijn Sequences to Index a 1 in a Computer Word"
 * @param bb bitboard to scan
 * @precondition x != 0
 * @return index (0..63) of least significant one bit
 */



// assumes x != 0
inline bool has_one_bit_set(Uint64 x) {
	return (x & (x - 1)) == 0;
}