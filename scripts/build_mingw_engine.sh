#!/bin/bash

# Build engine

mkdir -p mingw-build && cd mingw-build
export CC="ccache i686-w64-mingw32-gcc"
export CXX="ccache i686-w64-mingw32-g++"
export CFLAGS="-static-libgcc -no-pthread"
export CXXFLAGS="-static-libgcc -static-libstdc++"
cmake \
	-DXASH_DOWNLOAD_DEPENDENCIES=ON \
	-DCMAKE_SYSTEM_NAME=Windows \
	-DCMAKE_BUILD_TYPE=RelWithDebInfo \
	-DXASH_STATIC=ON \
	-DXASH_VGUI=ON \
	-DXASH_SDL=ON ../
make -j2 VERBOSE=1
make install # Install everything
cp /usr/i686-w64-mingw32/lib/libwinpthread-1.dll . # a1ba: remove when travis will be updated to xenial
