# vs-ComparePlane
Optimize for several algorithms for compare images.

## Usage
```python
core.complane.PSNR(clip clip1, clip clip2, int opt, int cache)
```
The both input clips must be of GRAY colorFamily and their format must be exactly same. Both 8-16 int and float sample type are allowed.
The res will be reserved into the frame props of output.
1. Args:
  - ```clip1```: The clip you want to complete, the result have the same frame data and props (except the added by filter) as this clip.
  - ```clip2```: The other clip.
  - ```propname```(Default:"PlanePSNR"): The name of the PSNR score in frameprops you want to set. 
  - ```opt```: Optimize level:
    - 0: auto detect(default)
    - 1: avx
    - 2: avx2
  - ```cache```: Whether the output should be cache or not. Disabling cache maybe imporove a little bit of performance. If you want to access them later, it's recommand to enable cache.
    - 0: disable
    - 1: enable(default)  
```python
core.complane.Version()
```
Get Version.
## Build
You can use the meson to build the plugin by youself:
```
# if built for windows, place VapourSynth.h and VSHelper.h under Complane.
CXX=clang++ meson build
ninja -C build
```
Or on the windows platform, you can also build it simply with the clang:
```
clang++.exe -c complane.cpp -o complane.o -g -Wall -fcolor-diagnostics --target=x86_64-w64-mingw -std=c++1z -I "\path\to\vapoursynth\include" -fno-exceptions
clang++.exe -c complane_avx2.cpp -o complane_avx2.o -g -Wall -fcolor-diagnostics --target=x86_64-w64-mingw -std=c++1z -I "\path\to\vapoursynth\include" -fno-exceptions -mavx2  -mfma
clang++.exe -c complane_avx.cpp -o complane_avx.o -g -Wall -fcolor-diagnostics --target=x86_64-w64-mingw -std=c++1z -I "\path\to\vapoursynth\include" -fno-exceptions -mavx  -mfma
clang++.exe -o "\path\to\vapoursynth\vapoursynth64\plugins\complane.dll" complane.o complane_avx2.o complane_avx.o -g -Wall -static-libgcc -fcolor-diagnostics --target=x86_64-w64-mingw -std=c++1z -lstdc++ -shared -fno-exceptions
del complane.o -o complane_avx2.o complane_avx.o
```
