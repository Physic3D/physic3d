# Build engine

mkdir -p build && cd build/
export CFLAGS="-m32"
export CXXFLAGS="-m32"
cmake \
	-DXASH_DEDICATED=ON \
	-DXASH_VGUI=ON \
	-DCMAKE_BUILD_TYPE=RelWithDebInfo ../
make
