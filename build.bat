@echo off

if not exist build ( mkdir build )

set cflags=/nologo /Zi /std:c++20 /EHsc /Od
set lflags=/INCREMENTAL:NO /subsystem:windows

pushd build
cl %cflags% ../main.cpp /link %lflags%
del *.obj
popd
