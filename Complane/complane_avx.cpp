/*This file contains all the avx optimizing functions.*/
#ifndef COMPLANE_AVX_CPP
#define COMPLANE_AVX_CPP
#include "complane.h"
#include <cmath>
#include <immintrin.h>
template<typename pixel_t> float complane_avx_psnr(const void* _src1p, const void* _src2p, const uint16_t width, const uint16_t height, const ptrdiff_t stirde, const ComparePlaneData* const VS_RESTRICT d) noexcept{
    const pixel_t *src1p = reinterpret_cast<const pixel_t*>(_src1p);
    const pixel_t *src2p = reinterpret_cast<const pixel_t*>(_src2p);
    float psnr = 0;
    uint16_t step = 0;
    if constexpr (std::is_same_v<pixel_t,uint8_t>){     //for 8-bit input
        int64_t diffsum = 0;
        step = 32;
        const uint32_t max_val = (1<<d->vi_1->format->bitsPerSample)-1;
        for (uint16_t y=0; y < height; y++){
            __m128i diff = _mm_setzero_si128();
            const uint8_t mask[32] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
            for(uint16_t x=0; x < width; x += step){
                __m256i src1,src2;
                src1 = _mm256_lddqu_si256((__m256i const*)(src1p+x));
                src2 = _mm256_lddqu_si256((__m256i const*)(src2p+x));
                if( width-x < 16){
                    __m128i load_mask = _mm_load_si128((__m128i const*)(mask+16-(width-x)));
                    __m128i src1lo, src2lo;
                    src1lo = _mm256_castsi256_si128(src1);
                    src2lo = _mm256_castsi256_si128(src2);

                    src1lo = _mm_and_si128(src1lo, load_mask);
                    src2lo = _mm_and_si128(src2lo, load_mask);
                    __m128i tmp1, tmp2, tmp3, tmp4;
                    tmp1 = _mm_subs_epu8(src1lo,src2lo);
                    tmp2 = _mm_subs_epu8(src2lo,src1lo);
                    tmp1 = _mm_or_si128(tmp1, tmp2);

                    tmp3 = _mm_cvtepu8_epi16(_mm_set1_epi64x(_mm_extract_epi64(tmp1, 0)));
                    tmp4 = _mm_cvtepu8_epi16(_mm_set1_epi64x(_mm_extract_epi64(tmp1, 1)));
                    tmp3 = _mm_mullo_epi16(tmp3, tmp3);
                    tmp4 = _mm_mullo_epi16(tmp4, tmp4);
                    
                    diff = _mm_add_epi32(diff, _mm_cvtepu16_epi32((_mm_set1_epi64x(_mm_extract_epi64(tmp3, 0)))));
                    diff = _mm_add_epi32(diff, _mm_cvtepu16_epi32((_mm_set1_epi64x(_mm_extract_epi64(tmp3, 1)))));
                    diff = _mm_add_epi32(diff, _mm_cvtepu16_epi32((_mm_set1_epi64x(_mm_extract_epi64(tmp4, 0)))));
                    diff = _mm_add_epi32(diff, _mm_cvtepu16_epi32((_mm_set1_epi64x(_mm_extract_epi64(tmp4, 1)))));
                } else {
                    __m128i src1lo, src1hi, src2lo, src2hi;
                    src1lo = _mm256_castsi256_si128(src1);
                    src1hi = _mm256_extractf128_si256(src1, 1);
                    src2lo = _mm256_castsi256_si128(src2);
                    src2hi = _mm256_extractf128_si256(src2, 1);
                    if(width-x < step){
                        __m128i load_mask = _mm_load_si128((__m128i const*)(mask+16-(width-x-16)));
                        src1hi = _mm_and_si128(src1hi, load_mask);
                        src2hi = _mm_and_si128(src2hi, load_mask);
                    }
                    __m128i tmp1, tmp2, tmp3, tmp4;
                    tmp1 = _mm_subs_epu8(src1lo,src2lo);
                    tmp2 = _mm_subs_epu8(src2lo,src1lo);
                    tmp1 = _mm_or_si128(tmp1, tmp2);

                    tmp3 = _mm_cvtepu8_epi16(_mm_set1_epi64x(_mm_extract_epi64(tmp1, 0)));
                    tmp4 = _mm_cvtepu8_epi16(_mm_set1_epi64x(_mm_extract_epi64(tmp1, 1)));
                    tmp3 = _mm_mullo_epi16(tmp3, tmp3);
                    tmp4 = _mm_mullo_epi16(tmp4, tmp4);
                    
                    diff = _mm_add_epi32(diff, _mm_cvtepu16_epi32((_mm_set1_epi64x(_mm_extract_epi64(tmp3, 0)))));
                    diff = _mm_add_epi32(diff, _mm_cvtepu16_epi32((_mm_set1_epi64x(_mm_extract_epi64(tmp3, 1)))));
                    diff = _mm_add_epi32(diff, _mm_cvtepu16_epi32((_mm_set1_epi64x(_mm_extract_epi64(tmp4, 0)))));
                    diff = _mm_add_epi32(diff, _mm_cvtepu16_epi32((_mm_set1_epi64x(_mm_extract_epi64(tmp4, 1)))));
                    
                    tmp1 = _mm_subs_epu8(src1hi,src2hi);
                    tmp2 = _mm_subs_epu8(src2hi,src1hi);
                    tmp1 = _mm_or_si128(tmp1, tmp2);

                    tmp3 = _mm_cvtepu8_epi16(_mm_set1_epi64x(_mm_extract_epi64(tmp1, 0)));
                    tmp4 = _mm_cvtepu8_epi16(_mm_set1_epi64x(_mm_extract_epi64(tmp1, 1)));
                    tmp3 = _mm_mullo_epi16(tmp3, tmp3);
                    tmp4 = _mm_mullo_epi16(tmp4, tmp4);
                    
                    diff = _mm_add_epi32(diff, _mm_cvtepu16_epi32((_mm_set1_epi64x(_mm_extract_epi64(tmp3, 0)))));
                    diff = _mm_add_epi32(diff, _mm_cvtepu16_epi32((_mm_set1_epi64x(_mm_extract_epi64(tmp3, 1)))));
                    diff = _mm_add_epi32(diff, _mm_cvtepu16_epi32((_mm_set1_epi64x(_mm_extract_epi64(tmp4, 0)))));
                    diff = _mm_add_epi32(diff, _mm_cvtepu16_epi32((_mm_set1_epi64x(_mm_extract_epi64(tmp4, 1)))));

                }
            }
            const uint32_t* difft = (const uint32_t*) &diff;
            diffsum = diffsum + difft[0] + difft[1] + difft[2] + difft[3];
            src1p += stirde;
            src2p += stirde;
        }
        float MSE = 1.0f * diffsum / (height*width);
        MSE = MSE > 1e-8? MSE : 1e-8;
        psnr = 10 * std::log10(max_val * max_val / MSE);
    } else if constexpr (std::is_same_v<pixel_t,uint16_t>){     //for 10-bit and 16-bit input  
        int64_t diffsum = 0;
        step = 16;
        const uint32_t max_val = (1<<d->vi_1->format->bitsPerSample)-1;
        for (uint16_t y=0; y < height; y++){
            __m128i diff = _mm_setzero_si128();
            const uint16_t mask[16] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0,0,0,0,0,0,0,0};
            for(uint16_t x=0; x < width; x += step){
                __m256i src1,src2;
                src1 = _mm256_lddqu_si256((__m256i const*)(src1p+x));
                src2 = _mm256_lddqu_si256((__m256i const*)(src2p+x));
                if( width-x < 8){
                    __m128i load_mask = _mm_load_si128((__m128i const*)(mask+8-(width-x)));
                    __m128i src1lo, src2lo;
                    src1lo = _mm256_castsi256_si128(src1);
                    src2lo = _mm256_castsi256_si128(src2);

                    src1lo = _mm_and_si128(src1lo, load_mask);
                    src2lo = _mm_and_si128(src2lo, load_mask);
                    __m128i tmp1, tmp2, tmp3, tmp4;
                    tmp1 = _mm_subs_epu16(src1lo,src2lo);
                    tmp2 = _mm_subs_epu16(src2lo,src1lo);
                    tmp1 = _mm_or_si128(tmp1, tmp2);

                    tmp3 = _mm_cvtepu16_epi32(_mm_set1_epi64x(_mm_extract_epi64(tmp1, 0)));
                    tmp4 = _mm_cvtepu16_epi32(_mm_set1_epi64x(_mm_extract_epi64(tmp1, 1)));
                    tmp3 = _mm_mullo_epi32(tmp3, tmp3);
                    tmp4 = _mm_mullo_epi32(tmp4, tmp4);
                    
                    diff = _mm_add_epi64(diff, _mm_cvtepu32_epi64((_mm_set1_epi64x(_mm_extract_epi64(tmp3, 0)))));
                    diff = _mm_add_epi64(diff, _mm_cvtepu32_epi64((_mm_set1_epi64x(_mm_extract_epi64(tmp3, 1)))));
                    diff = _mm_add_epi64(diff, _mm_cvtepu32_epi64((_mm_set1_epi64x(_mm_extract_epi64(tmp4, 0)))));
                    diff = _mm_add_epi64(diff, _mm_cvtepu32_epi64((_mm_set1_epi64x(_mm_extract_epi64(tmp4, 1)))));
                } else {
                    __m128i src1lo, src1hi, src2lo, src2hi;
                    src1lo = _mm256_castsi256_si128(src1);
                    src1hi = _mm256_extractf128_si256(src1, 1);
                    src2lo = _mm256_castsi256_si128(src2);
                    src2hi = _mm256_extractf128_si256(src2, 1);
                    if(width-x < step){
                        __m128i load_mask = _mm_load_si128((__m128i const*)(mask+16-(width-x-16)));
                        src1hi = _mm_and_si128(src1hi, load_mask);
                        src2hi = _mm_and_si128(src2hi, load_mask);
                    }
                    __m128i tmp1, tmp2, tmp3, tmp4;
                    tmp1 = _mm_subs_epu16(src1lo,src2lo);
                    tmp2 = _mm_subs_epu16(src2lo,src1lo);
                    tmp1 = _mm_or_si128(tmp1, tmp2);

                    tmp3 = _mm_cvtepu16_epi32(_mm_set1_epi64x(_mm_extract_epi64(tmp1, 0)));
                    tmp4 = _mm_cvtepu16_epi32(_mm_set1_epi64x(_mm_extract_epi64(tmp1, 1)));
                    tmp3 = _mm_mullo_epi32(tmp3, tmp3);
                    tmp4 = _mm_mullo_epi32(tmp4, tmp4);
                    
                    diff = _mm_add_epi64(diff, _mm_cvtepu32_epi64((_mm_set1_epi64x(_mm_extract_epi64(tmp3, 0)))));
                    diff = _mm_add_epi64(diff, _mm_cvtepu32_epi64((_mm_set1_epi64x(_mm_extract_epi64(tmp3, 1)))));
                    diff = _mm_add_epi64(diff, _mm_cvtepu32_epi64((_mm_set1_epi64x(_mm_extract_epi64(tmp4, 0)))));
                    diff = _mm_add_epi64(diff, _mm_cvtepu32_epi64((_mm_set1_epi64x(_mm_extract_epi64(tmp4, 1)))));
                    
                    tmp1 = _mm_subs_epu16(src1hi,src2hi);
                    tmp2 = _mm_subs_epu16(src2hi,src1hi);
                    tmp1 = _mm_or_si128(tmp1, tmp2);

                    tmp3 = _mm_cvtepu16_epi32(_mm_set1_epi64x(_mm_extract_epi64(tmp1, 0)));
                    tmp4 = _mm_cvtepu16_epi32(_mm_set1_epi64x(_mm_extract_epi64(tmp1, 1)));
                    tmp3 = _mm_mullo_epi32(tmp3, tmp3);
                    tmp4 = _mm_mullo_epi32(tmp4, tmp4);
                    
                    diff = _mm_add_epi64(diff, _mm_cvtepu32_epi64((_mm_set1_epi64x(_mm_extract_epi64(tmp3, 0)))));
                    diff = _mm_add_epi64(diff, _mm_cvtepu32_epi64((_mm_set1_epi64x(_mm_extract_epi64(tmp3, 1)))));
                    diff = _mm_add_epi64(diff, _mm_cvtepu32_epi64((_mm_set1_epi64x(_mm_extract_epi64(tmp4, 0)))));
                    diff = _mm_add_epi64(diff, _mm_cvtepu32_epi64((_mm_set1_epi64x(_mm_extract_epi64(tmp4, 1)))));

                }
            }
            const uint32_t* difft = (const uint32_t*) &diff;
            diffsum = diffsum + difft[0] + difft[1] + difft[2] + difft[3];
            src1p += stirde;
            src2p += stirde;
        }
        float MSE = 1.0f * diffsum / (height*width);
        MSE = MSE > 1e-8? MSE : 1e-8;
        psnr = 10 * std::log10(max_val * max_val / MSE);
    }
    else if constexpr (std::is_same_v<pixel_t, float_t>){     //for float input  
        float_t diffsum = 0.0f;
        step = 8;
        __m256 diff = _mm256_setzero_ps();
        const uint32_t mask[16] = {0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF, 0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF, 0,0,0,0, 0,0,0,0};
        for(uint32_t x=0; x < width*height; x += step){
            __m256 src1,src2;
            if (width*height-x >= step){
            src1 = _mm256_loadu_ps(src1p+x);
            src2 = _mm256_loadu_ps(src2p+x);
            } else {
                __m256i load_mask = _mm256_lddqu_si256((__m256i const*)(mask+8-(width-x)));
                src1 = _mm256_maskload_ps(src1p, load_mask);
                src2 = _mm256_maskload_ps(src2p, load_mask);
            }
            __m256 tmp1 = _mm256_sub_ps(src1, src2);
            tmp1 = _mm256_mul_ps(tmp1, tmp1);
            diff = _mm256_add_ps(diff, tmp1);
        }

        const float_t* difft = (const float_t*) &diff;
        diffsum = difft[0] + difft[1] + difft[2] + difft[3] + difft[4] + difft[5] + difft[6] + difft[7];
        float MSE = diffsum / (1.0*height*width);
        MSE = MSE > 1e-8? MSE : 1e-8;
        psnr = -10 * std::log10(MSE);
    }
    psnr = psnr>100? 100 : psnr;
    if (psnr>0) return psnr;
    else return NULL;
}

template float complane_avx_psnr<uint8_t>(const void* _src1p, const void* _src2p, const uint16_t width, const uint16_t height, const ptrdiff_t stirde, const ComparePlaneData* const VS_RESTRICT d) noexcept;
template float complane_avx_psnr<uint16_t>(const void* _src1p, const void* _src2p, const uint16_t width, const uint16_t height, const ptrdiff_t stirde, const ComparePlaneData* const VS_RESTRICT d) noexcept;
template float complane_avx_psnr<float_t>(const void* _src1p, const void* _src2p, const uint16_t width, const uint16_t height, const ptrdiff_t stirde, const ComparePlaneData* const VS_RESTRICT d) noexcept;
#endif