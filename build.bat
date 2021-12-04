@echo off

if not exist build ( mkdir build )

set cflags=/nologo /Zi /std:c++20 /EHsc /Od
set lflags=/INCREMENTAL:NO /subsystem:windows

pushd build
cl %cflags% ../transparent_window.cpp /link %lflags%
del *.obj
popd
