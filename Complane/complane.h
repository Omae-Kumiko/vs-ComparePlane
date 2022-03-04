#ifndef COMPLANE_H
#define COMPLANE_H
#include "VapourSynth.h"
#include "VSHelper.h"
#include "VCL2/vectorclass.h"

struct ComparePlaneData final
{
    VSNodeRef *node1;
    const VSVideoInfo* vi_1;
    VSNodeRef *node2;
    const VSVideoInfo* vi_2;
    uint8_t opt;
    uint8_t cache;
    float (*getPSNR)(const void* _src1p, const void* _src2p, const uint16_t width, const uint16_t height, const ptrdiff_t stirde, const ComparePlaneData* const VS_RESTRICT d) noexcept; //Get psnr score function.
};
#endif // !COMPLANE_H
