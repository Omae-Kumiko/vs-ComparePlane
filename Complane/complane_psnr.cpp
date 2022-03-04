/*This file contains all the optimizing functions to get psnr score.
*/
#ifndef COMPLANE_PSNR_CPP
#define COMPLANE_PSNR_CPP
#include "complane.h"
#include <cmath>
template<typename pixel_t> float complane_psnr_avx2(const void* _src1p, const void* _src2p, const uint16_t width, const uint16_t height, const ptrdiff_t stirde, const ComparePlaneData* const VS_RESTRICT d) noexcept{
    const pixel_t *src1p = reinterpret_cast<const pixel_t*>(_src1p);
    const pixel_t *src2p = reinterpret_cast<const pixel_t*>(_src2p);
    float psnr = 0;
    uint16_t step = 0;
    if constexpr (std::is_same_v<pixel_t,uint8_t>){     //for 8-bit input
        int64_t diffsum = 0;
        step = 16;
        const uint32_t max_val = (1<<d->vi_1->format->bitsPerSample)-1;

        for (uint16_t y=0; y < height; y++){
            for(uint16_t x=0; x < width; x += step){
                Vec16uc src1, src2;
                if( width-x >= step){
                    src1.load(src1p+x);
                    src2.load(src2p+x);
                }else{
                    src1.load_partial(width-x, src1p+x);
                    src2.load_partial(width-x, src2p+x);
                }
                Vec16us tmp1, tmp2, tmp3;
                tmp1 = extend(src1);
                tmp2 = extend(src2);
                tmp3 = select(tmp1>tmp2, tmp1-tmp2 , tmp2 - tmp1);
                tmp3 = tmp3 * tmp3;
                diffsum += (horizontal_add(extend_low(tmp3)) + horizontal_add(extend_high(tmp3)));
            }
            src1p += stirde;
            src2p += stirde;
        }
        float MSE = 1.0f * diffsum / (height*width);
        MSE = MSE > 1e-8? MSE : 1e-8;
        psnr = 10 * std::log10(max_val * max_val / MSE);
    } else if constexpr (std::is_same_v<pixel_t,uint16_t>){     //for 10-bit and 16-bit input  
        int64_t diffsum = 0;
        step = 8;
        const uint32_t max_val = (1<<d->vi_1->format->bitsPerSample)-1;
        for (uint16_t y=0; y < height; y++){
            for(uint16_t x=0; x < width; x += step){
                Vec8us src1, src2;
                if( width-x >= step){
                    src1.load(src1p+x);
                    src2.load(src2p+x);
                }else{
                    src1.load_partial(width-x, src1p+x);
                    src2.load_partial(width-x, src2p+x);
                }
                Vec8ui tmp1, tmp2, tmp3;
                tmp1 = extend(src1);
                tmp2 = extend(src2);
                tmp3 = select(tmp1>tmp2, tmp1-tmp2 , tmp2 - tmp1);
                tmp3 = tmp3 * tmp3;
                diffsum += (horizontal_add(extend_low(tmp3)) + horizontal_add(extend_high(tmp3)));
            }
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
        // for (uint16_t y=0; y < height; y++){
            for(uint32_t x=0; x < width*height; x += step){
                Vec8f src1, src2, tmp1;
                if( width-x >= step){
                    src1.load(src1p+x);
                    src2.load(src2p+x);
                }else{
                    src1.load_partial(width-x, src1p+x);
                    src2.load_partial(width-x, src2p+x);
                }
                tmp1 = src1 - src2;
                tmp1 = tmp1 * tmp1;
                diffsum += horizontal_add(tmp1);
            }
        // }
        float MSE = diffsum / (1.0*height*width);
        MSE = MSE > 1e-8? MSE : 1e-8;
        psnr = -10 * std::log10(MSE);
    }
    psnr = psnr>100? 100 : psnr;
    if (psnr>0) return psnr;
    else return NULL;
}

template<typename pixel_t> float complane_psnr_sse2(const void* _src1p, const void* _src2p, const uint16_t width, const uint16_t height, const ptrdiff_t stirde, const ComparePlaneData* const VS_RESTRICT d) noexcept{
    const pixel_t *src1p = reinterpret_cast<const pixel_t*>(_src1p);
    const pixel_t *src2p = reinterpret_cast<const pixel_t*>(_src2p);
    float psnr = 0;
    uint16_t step = 0;
    if constexpr (std::is_same_v<pixel_t,uint8_t>){     //for 8-bit input
        int64_t diffsum = 0;
        step = 16;
        const uint32_t max_val = (1<<d->vi_1->format->bitsPerSample)-1;

        for (uint16_t y=0; y < height; y++){
            for(uint16_t x=0; x < width; x += step){
                Vec16uc src1, src2;
                if( width-x >= step){
                    src1.load(src1p+x);
                    src2.load(src2p+x);
                }else{
                    src1.load_partial(width-x, src1p+x);
                    src2.load_partial(width-x, src2p+x);
                }
                Vec8us tmp1, tmp2, tmp3, tmp4;
                tmp1 = extend_low(src1);
                tmp2 = extend_low(src2);
                tmp3 = select(tmp1>tmp2, tmp1-tmp2 , tmp2 - tmp1);
                tmp3 = tmp3 * tmp3;
                tmp1 = extend_high(src1);
                tmp2 = extend_high(src2);
                tmp4 = select(tmp1>tmp2, tmp1-tmp2 , tmp2 - tmp1);
                tmp4 = tmp4 * tmp4;
                diffsum += (horizontal_add(extend_low(tmp3)) + horizontal_add(extend_high(tmp3))+horizontal_add(extend_low(tmp4)) + horizontal_add(extend_high(tmp4)));
            }
            src1p += stirde;
            src2p += stirde;
        }
        float MSE = 1.0f * diffsum / (height*width);
        MSE = MSE > 1e-8? MSE : 1e-8;
        psnr = 10 * std::log10(max_val * max_val / MSE);
    } else if constexpr (std::is_same_v<pixel_t,uint16_t>){     //for 10-bit and 16-bit input  
        int64_t diffsum = 0;
        step = 8;
        const uint32_t max_val = (1<<d->vi_1->format->bitsPerSample)-1;
        for (uint16_t y=0; y < height; y++){
            for(uint16_t x=0; x < width; x += step){
                Vec8us src1, src2;
                if( width-x >= step){
                    src1.load(src1p+x);
                    src2.load(src2p+x);
                }else{
                    src1.load_partial(width-x, src1p+x);
                    src2.load_partial(width-x, src2p+x);
                }
                Vec4ui tmp1, tmp2, tmp3, tmp4;
                tmp1 = extend_low(src1);
                tmp2 = extend_low(src2);
                tmp3 = select(tmp1>tmp2, tmp1-tmp2 , tmp2 - tmp1);
                tmp3 = tmp3 * tmp3;
                tmp1 = extend_high(src1);
                tmp2 = extend_high(src2);
                tmp4 = select(tmp1>tmp2, tmp1-tmp2 , tmp2 - tmp1);
                tmp4 = tmp4 * tmp4;
                diffsum += (horizontal_add(extend_low(tmp3)) + horizontal_add(extend_high(tmp3))+horizontal_add(extend_low(tmp4)) + horizontal_add(extend_high(tmp4)));
            }
            src1p += stirde;
            src2p += stirde;
        }
        float MSE = 1.0f * diffsum / (height*width);
        MSE = MSE > 1e-8? MSE : 1e-8;
        psnr = 10 * std::log10(max_val * max_val / MSE);
    }
    else if constexpr (std::is_same_v<pixel_t, float_t>){     //for float input  
        float_t diffsum = 0.0f;
        step = 4;
        // for (uint16_t y=0; y < height; y++){
            for(uint32_t x=0; x < width*height; x += step){
                Vec4f src1, src2, tmp1;
                if( width-x >= step){
                    src1.load(src1p+x);
                    src2.load(src2p+x);
                }else{
                    src1.load_partial(width-x, src1p+x);
                    src2.load_partial(width-x, src2p+x);
                }
                tmp1 = src1 - src2;
                tmp1 = tmp1 * tmp1;
                diffsum += horizontal_add(tmp1);
            }
        // }
        float MSE = diffsum / (1.0*height*width);
        MSE = MSE > 1e-8? MSE : 1e-8;
        psnr = -10 * std::log10(MSE);
    }
    psnr = psnr>100? 100 : psnr;
    if (psnr>0) return psnr;
    else return NULL;
}
template float complane_psnr_avx2<uint8_t>(const void* _src1p, const void* _src2p, const uint16_t width, const uint16_t height, const ptrdiff_t stirde, const ComparePlaneData* const VS_RESTRICT d) noexcept;
template float complane_psnr_avx2<uint16_t>(const void* _src1p, const void* _src2p, const uint16_t width, const uint16_t height, const ptrdiff_t stirde, const ComparePlaneData* const VS_RESTRICT d) noexcept;
template float complane_psnr_avx2<float_t>(const void* _src1p, const void* _src2p, const uint16_t width, const uint16_t height, const ptrdiff_t stirde, const ComparePlaneData* const VS_RESTRICT d) noexcept;
template float complane_psnr_sse2<uint8_t>(const void* _src1p, const void* _src2p, const uint16_t width, const uint16_t height, const ptrdiff_t stirde, const ComparePlaneData* const VS_RESTRICT d) noexcept;
template float complane_psnr_sse2<uint16_t>(const void* _src1p, const void* _src2p, const uint16_t width, const uint16_t height, const ptrdiff_t stirde, const ComparePlaneData* const VS_RESTRICT d) noexcept;
template float complane_psnr_sse2<float_t>(const void* _src1p, const void* _src2p, const uint16_t width, const uint16_t height, const ptrdiff_t stirde, const ComparePlaneData* const VS_RESTRICT d) noexcept;
#endif // !COMPLANE_PSNR_CPP
