#pragma once

#include <cstdint>

#ifdef __SSE2__
#include <emmintrin.h>
#endif

#ifdef __SSE4_2__
#include <nmmintrin.h>
#endif


namespace detail
{

template <char s0>
inline bool is_in(char x)
{
    return x == s0;
}

template <char s0, char s1, char... tail>
inline bool is_in(char x)
{
    return x == s0 || is_in<s1, tail...>(x);
}

#if __SSE2__
template <char s0>
inline __m128i mm_is_in(__m128i bytes)
{
    //_mm_set1_epi8 返回一个__m128i , 返回值的16个字节均被设置为s0
    //_mm_cmpeq_epi8 的返回值:
    // 	r0 := (a0 == b0) ? 0xff : 0x0
    // r1 := (a1 == b1) ? 0xff : 0x0
    // ...
    // r15 := (a15 == b15) ? 0xff : 0x0
	// 也就是说,如果bytes中包含字符s0,那么对应的位置置为全1
    __m128i eq0 = _mm_cmpeq_epi8(bytes, _mm_set1_epi8(s0));
    return eq0;
}

template <char s0, char s1, char... tail>
inline __m128i mm_is_in(__m128i bytes)
{
	//搜索第一个字符,如果存在其中,那么eq0对应的位置就为0xff
    __m128i eq0 = _mm_cmpeq_epi8(bytes, _mm_set1_epi8(s0));
    __m128i eq = mm_is_in<s1, tail...>(bytes);
	//最后或操作后,就是在bytes中找到对于字符的位置都会被置为0xff, 总共16个字符
    return _mm_or_si128(eq0, eq);
}
#endif


template <char... symbols>
inline const char * find_first_symbols_sse2(const char * begin, const char * end)
{
#if __SSE2__
    //一次处理16个字节
    for (; begin + 15 < end; begin += 16)
    {
        //将begin指针指向的16个字节强制转化并加载到bytes中
        __m128i bytes = _mm_loadu_si128(reinterpret_cast<const __m128i *>(begin));

		//获取symbols在bytes中的位置
        __m128i eq = mm_is_in<symbols...>(bytes);

		//返回一个16bit整数，r=(_A15[7] << 15) | (_A14[7] << 14) ... (_A1[7] << 1) | _A0[7],   
		//也就是返回每个8bit位置的最高位,组成一个16位的数字
		// 注意, _mm_movemask_epi8刚好逆序
		//举个例子:  
		//   假设bytes = "abcdefghijklmnop"  ,  symbols = 'h', 'o'
		//那么经过计算, eq = 0x00_00_00_00_00_00_00_ff_00_00_00_00_00_00_ff_00
		// 经过下列函数计算后, bit_mask = 0100000010000000B  也就是说,末尾的0有7个,刚好就是h相对于a的偏移
        uint16_t bit_mask = _mm_movemask_epi8(eq);
        if (bit_mask)
			//__builtin_ctz返回bit_mask末尾0的个数
            return begin + __builtin_ctz(bit_mask);
    }
#endif
	//剩余非16个字符,依次计算
    for (; begin < end; ++begin)
        if (is_in<symbols...>(*begin))
            return begin;
    return end;
}


template <size_t num_chars,
         char c01,     char c02 = 0, char c03 = 0, char c04 = 0,
         char c05 = 0, char c06 = 0, char c07 = 0, char c08 = 0,
         char c09 = 0, char c10 = 0, char c11 = 0, char c12 = 0,
         char c13 = 0, char c14 = 0, char c15 = 0, char c16 = 0>
inline const char * find_first_symbols_sse42_impl(const char * begin, const char * end)
{
#if __SSE4_2__
//定义模式, 无符号8bit, 任意相等模式,  设置和_SIDD_BIT_MASK相同bit (以bit形式返回)
#define MODE (_SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_ANY | _SIDD_LEAST_SIGNIFICANT)
	//反序加载要查找的字符,也就是高位默认为0
    __m128i set = _mm_setr_epi8(c01, c02, c03, c04, c05, c06, c07, c08, c09, c10, c11, c12, c13, c14, c15, c16);

    for (; begin + 15 < end; begin += 16)
    {
		//加载要比较的字符,16字节
        __m128i bytes = _mm_loadu_si128(reinterpret_cast<const __m128i *>(begin));
		//参数含义:   set: 待查找的字符
		//					  num_chars : 待查找的字符个数
		//					 bytes:  在哪里查找
		//					16:	被查找的字符始终都是16字节的
		//					比较模式:	_SIDD_UBYTE_OPS  无符号8bit字符
		//					_SIDD_CMP_EQUAL_ANY             任意相等模式(也就是查找)
		//					_SIDD_LEAST_SIGNIFICANT      最右边bit为1的就返回对于的index
        if (_mm_cmpestrc(set, num_chars, bytes, 16, MODE))
            return begin + _mm_cmpestri(set, num_chars, bytes, 16, MODE);
    }
#undef MODE
#endif

    for (; begin < end; ++begin)
        if (   (num_chars >= 1 && *begin == c01)
                || (num_chars >= 2 && *begin == c02)
                || (num_chars >= 3 && *begin == c03)
                || (num_chars >= 4 && *begin == c04)
                || (num_chars >= 5 && *begin == c05)
                || (num_chars >= 6 && *begin == c06)
                || (num_chars >= 7 && *begin == c07)
                || (num_chars >= 8 && *begin == c08)
                || (num_chars >= 9 && *begin == c09)
                || (num_chars >= 10 && *begin == c10)
                || (num_chars >= 11 && *begin == c11)
                || (num_chars >= 12 && *begin == c12)
                || (num_chars >= 13 && *begin == c13)
                || (num_chars >= 14 && *begin == c14)
                || (num_chars >= 15 && *begin == c15)
                || (num_chars >= 16 && *begin == c16))
            return begin;
    return end;
}


template <char... symbols>
inline const char * find_first_symbols_sse42(const char * begin, const char * end)
{
    return find_first_symbols_sse42_impl<sizeof...(symbols), symbols...>(begin, end);
}

}



//通过sse指令来快速搜索首个匹配的字符
template <char... symbols>
inline const char * find_first_symbols(const char * begin, const char * end)
{
#if __SSE4_2__
	//如果要查找的字符超过了5个,那么走SSE4_2
    if (sizeof...(symbols) >= 5)
        return detail::find_first_symbols_sse42<symbols...>(begin, end);
    else
#endif
        return detail::find_first_symbols_sse2<symbols...>(begin, end);
}
