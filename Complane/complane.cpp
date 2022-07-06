//  Copyright (c) 2022 Omae-Kumiko.  All rights reserved.
//  
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef COMPLANE_CPP
#define COMPLANE_CPP
#define COMPLANE_VERSION "1.1.1"
#include "complane.h"
#include <stdio.h>

template<typename pixel_t> extern float complane_avx2_psnr(const void* _src1p, const void* _src2p, const uint16_t width, const uint16_t height, const ptrdiff_t stirde, const ComparePlaneData* const VS_RESTRICT d) noexcept;
template<typename pixel_t> extern float complane_avx_psnr(const void* _src1p, const void* _src2p, const uint16_t width, const uint16_t height, const ptrdiff_t stirde, const ComparePlaneData* const VS_RESTRICT d) noexcept;

//This functione iniates the psnr filter.
static void VS_CC PSNRInit(VSMap *in, VSMap *out, void **instanceData, VSNode *node, VSCore *core, const VSAPI *vsapi) {
    ComparePlaneData *d = (ComparePlaneData *) * instanceData;
    vsapi->setVideoInfo(d->vi_1, 1, node);
}

//This function is responsible for getting the psnr score.
static const VSFrameRef *VS_CC PSNRGet(int n, int activationReason, void **instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi) {
    ComparePlaneData *d = (ComparePlaneData *) * instanceData;
    if (activationReason == arInitial) {
        // Request the source frame on the first call
        vsapi->requestFrameFilter(n, d->node1, frameCtx);
        vsapi->requestFrameFilter(n, d->node2, frameCtx);
    } else if (activationReason == arAllFramesReady) {
        const VSFrameRef *src1 = vsapi->getFrameFilter(n, d->node1, frameCtx);
        const VSFrameRef *src2 = vsapi->getFrameFilter(n, d->node2, frameCtx);
        const VSFormat *fi_1 = d->vi_1->format;
        int height = vsapi->getFrameHeight(src1, 0);
        int width = vsapi->getFrameWidth(src1, 0);
        const VSFrameRef *frame[] = {src1};
        const int planes[] = {0}; 
        VSFrameRef *dst = vsapi->newVideoFrame2(fi_1, width, height, frame, planes, src1, core);
        const void *src1p = vsapi->getReadPtr(src1, 0);
        const void *src2p = vsapi->getReadPtr(src2, 0);
        const int32_t stride = vsapi->getStride(src1, 0) / d->vi_1->format->bytesPerSample;
        VSMap *prop = vsapi->getFramePropsRW(dst);
        const float psnr = d->getPSNR(src1p, src2p, width, height, stride, d);

        vsapi->propSetFloat(prop, d->propname, psnr, paReplace);
        // Release the source frame
        vsapi->freeFrame(src1);
        vsapi->freeFrame(src2);
        return dst;
    }
    return 0;
}

//This function frees the psnr filter.
static void VS_CC PSNRFree(void *instanceData, VSCore *core, const VSAPI *vsapi) {
    ComparePlaneData *d = (ComparePlaneData *)instanceData;
    vsapi->freeNode(d->node1);
    vsapi->freeNode(d->node2);
    free(d);
}

//This function creates the psnr filter.
static void VS_CC PSNRCreate(const VSMap *in, VSMap *out, void *userData, VSCore *core, const VSAPI *vsapi) {
    
    ComparePlaneData d;
    ComparePlaneData *data;
    int err = 0;
    d.node1 = vsapi->propGetNode(in, "clip1", 0, &err);
    d.node2 = vsapi->propGetNode(in, "clip2", 0, &err);
    d.vi_1 = vsapi->getVideoInfo(d.node1);
    d.vi_2 = vsapi->getVideoInfo(d.node2);
    if(err)
    {
        vsapi->freeNode(d.node1);
        vsapi->freeNode(d.node2);
        vsapi->setError(out, "PSNR:The Para 'clip1' and 'clip2' must be set.");
        return;        
    }
    if (d.vi_1->format->colorFamily != cmGray || d.vi_2->format->colorFamily != cmGray)
    {
        vsapi->freeNode(d.node1);
        vsapi->freeNode(d.node2);
        vsapi->setError(out, "PSNR: The colorFamily of input clips must be Gray.");
        return;
    }
    if (d.vi_1->format->id != d.vi_2->format->id)
    {
        vsapi->freeNode(d.node1);
        vsapi->freeNode(d.node2);
        vsapi->setError(out, "PSNR: The formats of input clips must be same.");
        return;
    }
    if (d.vi_1->height != d.vi_2->height || d.vi_1->width != d.vi_2->width)
    {
        vsapi->freeNode(d.node1);
        vsapi->freeNode(d.node2);
        vsapi->setError(out, "PSNR: The dimension of input clips must be same.");
        return;
    }
    err = 0;
    d.opt = vsapi->propGetInt(in, "opt", 0, &err);
    if (err)
        d.opt = 0;
    if (d.opt < 0) {
        vsapi->freeNode(d.node1);
        vsapi->freeNode(d.node2);
        vsapi->setError(out, "PSNR: The 'opt' must be Positive");
        return;
    }
    err = 0;
    d.cache = vsapi->propGetInt(in, "cache", 0, &err);
    if(err) d.cache = 1;
    d.cache = d.cache > 1? 1 : d.cache;

    err = 0;
    const char* propname = vsapi->propGetData(in, "propname", 0, &err);
    if(err) 
    {
        const char* defaultpropname = "PlanePSNR";
        strcpy(d.propname, defaultpropname);
    } else  strcpy(d.propname , propname);

    //All checks are done.

    data = (ComparePlaneData*) malloc(sizeof(d));
    *data = d;
    if (d.opt == 0){
        if(__builtin_cpu_supports("avx2")){
            if (data->vi_1->format->bytesPerSample == 1) data->getPSNR = complane_avx2_psnr<uint8_t>;
            else if (data->vi_1->format->bytesPerSample == 2) data->getPSNR = complane_avx2_psnr<uint16_t>;
            else if (data->vi_1->format->bytesPerSample == 4) data->getPSNR = complane_avx2_psnr<float_t>;
        } else if(__builtin_cpu_supports("avx")) {
            if (data->vi_1->format->bytesPerSample == 1) data->getPSNR = complane_avx_psnr<uint8_t>;
            else if (data->vi_1->format->bytesPerSample == 2) data->getPSNR = complane_avx_psnr<uint16_t>;
            else if (data->vi_1->format->bytesPerSample == 4) data->getPSNR = complane_avx_psnr<float_t>;         
        } else {
            vsapi->setError(out, "PSNR: You system or CPU doesn't support AVX2 or AVX.");
        }
    }
    else if (d.opt >= 2){
            if (data->vi_1->format->bytesPerSample == 1) data->getPSNR = complane_avx2_psnr<uint8_t>;
            else if (data->vi_1->format->bytesPerSample == 2) data->getPSNR = complane_avx2_psnr<uint16_t>;
            else if (data->vi_1->format->bytesPerSample == 4) data->getPSNR = complane_avx2_psnr<float_t>;
    }
    else if (d.opt == 1){
            if (data->vi_1->format->bytesPerSample == 1) data->getPSNR = complane_avx_psnr<uint8_t>;
            else if (data->vi_1->format->bytesPerSample == 2) data->getPSNR = complane_avx_psnr<uint16_t>;
            else if (data->vi_1->format->bytesPerSample == 4) data->getPSNR = complane_avx_psnr<float_t>;  
    }
    vsapi->createFilter(in, out, "psnr", PSNRInit, PSNRGet, PSNRFree, fmParallel, d.cache==1? 0:nfNoCache, data, core);
}
void VS_CC versionCreate(const VSMap *in, VSMap *out, void *user_data, VSCore *core, const VSAPI *vsapi)
{
    vsapi->propSetData(out, "version", COMPLANE_VERSION, -1, paAppend);
}
VS_EXTERNAL_API(void) VapourSynthPluginInit(VSConfigPlugin configFunc, VSRegisterFunction registerFunc, VSPlugin *plugin) {
    configFunc("com.amusementclub.complane", "complane", "VapourSynth compare plane and get score", VAPOURSYNTH_API_VERSION, 1, plugin);
    registerFunc("PSNR", "clip1:clip;clip2:clip;propname:data:opt;opt:int:opt;cache:int:opt", PSNRCreate, nullptr, plugin);
    registerFunc("Version", "", versionCreate, nullptr, plugin);
}
#endif // !COMPLANE_CPP