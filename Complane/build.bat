"D:\LLVM\bin\clang++.exe" -c complane.cpp -o complane.o -g -Wall -fcolor-diagnostics --target=x86_64-w64-mingw -std=c++1z -I "D:\Github\AmusementClub\vapoursynth-classic\include" -fno-exceptions
"D:\LLVM\bin\clang++.exe" -c complane_avx2.cpp -o complane_avx2.o -g -Wall -fcolor-diagnostics --target=x86_64-w64-mingw -std=c++1z -I "D:\Github\AmusementClub\vapoursynth-classic\include" -fno-exceptions -mavx2 -mfma
"D:\LLVM\bin\clang++.exe" -c complane_avx.cpp -o complane_avx.o -g -Wall -fcolor-diagnostics --target=x86_64-w64-mingw -std=c++1z -I "D:\Github\AmusementClub\vapoursynth-classic\include" -fno-exceptions -mavx -mfma
"D:\LLVM\bin\clang++.exe" -o "D:\Vapoursynth\VS-portable-2022H1.2\vapoursynth64\plugins\complane.dll" complane.o complane_avx2.o complane_avx.o -g -Wall -static-libgcc -fcolor-diagnostics --target=x86_64-w64-mingw -std=c++1z -lstdc++ -shared -fno-exceptions
del complane.o -o complane_avx2.o complane_avx.o
pause