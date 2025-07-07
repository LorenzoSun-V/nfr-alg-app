rm -rf build
mkdir build
cd build
cmake .. -DUSE_SOPHON=OFF -DUSE_NVIDIA=OFF -DUSE_ROCKCHIP=ON
make -j$(nproc)