rm -rf build
mkdir build
cd build
cmake .. -DUSE_SOPHON=OFF -DUSE_NVIDIA=ON -DUSE_ROCKCHIP=OFF
make -j$(nproc)