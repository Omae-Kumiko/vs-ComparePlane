/*This file contains all the avx2 optimizing functions.*/
#ifndef COMPLANE_AVX2_CPP
#define COMPLANE_AVX2_CPP
#include "complane.h"
#include <cmath>
#include <immintrin.h>
template<typename pixel_t> float complane_avx2_psnr(const void* _src1p, const void* _src2p, const uint16_t width, const uint16_t height, const ptrdiff_t stirde, const ComparePlaneData* const VS_RESTRICT d) noexcept{
    const pixel_t *src1p = reinterpret_cast<const pixel_t*>(_src1p);
    const pixel_t *src2p = reinterpret_cast<const pixel_t*>(_src2p);
    float psnr = 0;
    uint16_t step = 0;
    if constexpr (std::is_same_v<pixel_t,uint8_t>){     //for 8-bit input
        int64_t diffsum = 0;
        step = 32;
        const uint32_t max_val = (1<<d->vi_1->format->bitsPerSample)-1;
        for (uint16_t y=0; y < height; y++){
            __m256i diff = _mm256_setzero_si256();
            const uint8_t mask[64] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
            for(uint16_t x=0; x < width; x += step){
                __m256i src1,src2;
                src1 = _mm256_lddqu_si256((__m256i const*)(src1p+x));
                src2 = _mm256_lddqu_si256((__m256i const*)(src2p+x));
                if( width-x < step){
                    __m256i load_mask = _mm256_lddqu_si256((__m256i const*)(mask+32-(width-x)));
                    src1 = _mm256_and_si256(src1, load_mask);
                    src2 = _mm256_and_si256(src2, load_mask);
                }
                __m256i tmp1, tmp2, tmp3, tmp4;
                tmp1 = _mm256_subs_epu8(src1,src2);
                tmp2 = _mm256_subs_epu8(src2,src1);
                tmp1 = _mm256_or_si256(tmp1, tmp2);
                tmp3 = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(tmp1));
                tmp4 = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(tmp1, 1));
                tmp3 = _mm256_mullo_epi16(tmp3, tmp3);
                tmp4 = _mm256_mullo_epi16(tmp4, tmp4);
                diff = _mm256_add_epi32(diff, _mm256_cvtepu16_epi32(_mm256_castsi256_si128(tmp3)));
                diff = _mm256_add_epi32(diff, _mm256_cvtepu16_epi32(_mm256_extracti128_si256(tmp3,1)));
                diff = _mm256_add_epi32(diff, _mm256_cvtepu16_epi32(_mm256_castsi256_si128(tmp4)));
                diff = _mm256_add_epi32(diff, _mm256_cvtepu16_epi32(_mm256_extracti128_si256(tmp4,1)));
            }
            const uint32_t* difft = (const uint32_t*) &diff;
            diffsum = diffsum + difft[0] + difft[1] + difft[2] + difft[3] + difft[4] + difft[5] + difft[6] + difft[7];
            src1p += stirde;
            src2p += stirde;
        }
        float MSE = 1.0f * diffsum / (height*width);
        MSE = MSE > 1e-8? MSE : 1e-8;
        psnr = 10 * std::log10(max_val * max_val / MSE);
    } else if constexpr (std::is_same_v<pixel_t,uint16_t>){     //for 10-bit and 16-bit input  
        int64_t diffsum = 0;
        step = 16;
        const uint16_t mask[32] = {0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        const uint32_t max_val = (1<<d->vi_1->format->bitsPerSample)-1;
        for (uint16_t y=0; y < height; y++){
            __m256i diff = _mm256_setzero_si256();
            for(uint16_t x=0; x < width; x += step){
                __m256i src1,src2;
                src1 = _mm256_lddqu_si256((__m256i const*)(src1p+x));
                src2 = _mm256_lddqu_si256((__m256i const*)(src2p+x));
                if( width-x < step){
                    __m256i load_mask = _mm256_lddqu_si256((__m256i const*)(mask+16-(width-x)));
                    src1 = _mm256_and_si256(src1, load_mask);
                    src2 = _mm256_and_si256(src2, load_mask);
                }
                __m256i tmp1, tmp2, tmp3, tmp4;
                tmp1 = _mm256_subs_epu16(src1,src2);
                tmp2 = _mm256_subs_epu16(src2,src1);
                tmp1 = _mm256_or_si256(tmp1, tmp2);
                tmp3 = _mm256_cvtepu16_epi32(_mm256_castsi256_si128(tmp1));
                tmp4 = _mm256_cvtepu16_epi32(_mm256_extracti128_si256(tmp1, 1));
                tmp3 = _mm256_mullo_epi32(tmp3, tmp3);
                tmp4 = _mm256_mullo_epi32(tmp4, tmp4);
                diff = _mm256_add_epi64(diff, _mm256_cvtepu32_epi64(_mm256_castsi256_si128(tmp3)));
                diff = _mm256_add_epi64(diff, _mm256_cvtepu32_epi64(_mm256_extracti128_si256(tmp3,1)));
                diff = _mm256_add_epi64(diff, _mm256_cvtepu32_epi64(_mm256_castsi256_si128(tmp4)));
                diff = _mm256_add_epi64(diff, _mm256_cvtepu32_epi64(_mm256_extracti128_si256(tmp4,1)));
            }
            const uint64_t* difft = (const uint64_t*) &diff;
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
            diff = _mm256_fmadd_ps(tmp1, tmp1, diff);
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
template float complane_avx2_psnr<uint8_t>(const void* _src1p, const void* _src2p, const uint16_t width, const uint16_t height, const ptrdiff_t stirde, const ComparePlaneData* const VS_RESTRICT d) noexcept;
template float complane_avx2_psnr<uint16_t>(const void* _src1p, const void* _src2p, const uint16_t width, const uint16_t height, const ptrdiff_t stirde, const ComparePlaneData* const VS_RESTRICT d) noexcept;
template float complane_avx2_psnr<float_t>(const void* _src1p, const void* _src2p, const uint16_t width, const uint16_t height, const ptrdiff_t stirde, const ComparePlaneData* const VS_RESTRICT d) noexcept;
#endif // !COMPLANE_PSNR_CPP
