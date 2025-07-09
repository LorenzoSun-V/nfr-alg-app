rm -rf build
mkdir build
cd build
cmake .. -DUSE_SOPHON=ON -DUSE_NVIDIA=OFF -DUSE_ROCKCHIP=OFF
make -j$(nproc)