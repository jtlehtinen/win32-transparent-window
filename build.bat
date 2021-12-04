@echo off

if not exist build ( mkdir build )

set cflags=/nologo /Zi /std:c++20 /EHsc /Od /Wall /wd4100 /wd4668 /wd4820 /wd5039 /wd5045 /wd5246
set lflags=/INCREMENTAL:NO /subsystem:windows

pushd build
cl %cflags% ../transparent_window.cpp /link %lflags%
cl %cflags% ../transparent_window_dwrite.cpp /link %lflags%
del *.obj
popd
