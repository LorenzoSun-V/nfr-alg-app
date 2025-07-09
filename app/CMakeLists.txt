cmake_minimum_required(VERSION 3.10)
project(main)

set(ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(CMAKE_BUILD_TYPE Debug)

# eigen3库
find_package(Eigen3 REQUIRED)
set(EIGEN3_INCLUDE "/usr/include/eigen3")  # sudo apt install libeigen3-dev

if( USE_ROCKCHIP )
    set(OPENCV_INCLUDE /usr/include/opencv4/ )
    set(OPENCV_LIBPATH /usr/lib/aarch64-linux-gnu/ )
    link_directories(${ROOT_PATH}/api/libs-rockchip)
elseif( USE_SOPHON )
    set(OPENCV_INCLUDE ${ROOT_PATH}/3rdlibs/arm/soc-sdk/include/opencv4/ )
    set(SOPHON_INCLUDE ${ROOT_PATH}/3rdlibs/arm/soc-sdk/include/)
    set(OPENCV_LIBPATH ${ROOT_PATH}/3rdlibs/arm/soc-sdk/lib/ ) 
    link_directories(${ROOT_PATH}/api/libs-sophon)
elseif( USE_NVIDIA )
    set(OPENCV_INCLUDE  ${ROOT_PATH}/3rdlibs/x86/opencv4.8_x86_64/include)
    set(OPENCV_LIBPATH  ${ROOT_PATH}/3rdlibs/x86/opencv4.8_x86_64/lib ) 
    link_directories(${ROOT_PATH}/api/libs-nvidia)
else()
    message(FATAL_ERROR "Please choose either USE_SOPHON or USE_ROCKCHIP option.")
endif()

# opencv
set(OPENCV_LIBS opencv_highgui opencv_core opencv_imgproc opencv_imgcodecs opencv_videoio)
# yaml-cpp库
set(YAML_INCLUDE ${ROOT_PATH}/3rdlibs/yaml-cpp/source_code/build/install/include)
set(YAML_LIBPATH ${ROOT_PATH}/3rdlibs/yaml-cpp/source_code/build/install/lib)
link_directories(${YAML_LIBPATH})
set(YAML_LIBS yaml-cpp)
# curl库
find_package(CURL REQUIRED)    # sudo apt install -y libcurl4-openssl-dev
# nlohmann_json库
find_package(nlohmann_json 3.2.0 REQUIRED)  # sudo apt install nlohmann-json3-dev
# video decode
set(VIDEODECODE_LIBS videodec-c++)
# utils
set(UTILS_LIBS utils-c++)
# yolov8 infer
set(YOLOV8_LIBS yolov8-c++)
# yolov8obb infer
set(YOLOV8OBB_LIBS yolov8obb-c++)

SET(CMAKE_CXX_FLAGS "-std=c++17  -pthread -Wall -Ofast -Wfatal-errors -D_MWAITXINTRIN_H_INCLUDED -O3")

# 创建测试可执行文件
add_executable(main main.cpp)

target_include_directories(main
    PRIVATE
    ${EIGEN3_INCLUDE}
    ${OPENCV_INCLUDE}
    ${YAML_INCLUDE}
    ${ROOT_PATH}/src
    ${ROOT_PATH}/api/include
)

# 链接需要的库
target_link_libraries(main
    PRIVATE
    ${VIDEODECODE_LIBS}
    ${YOLOV8_LIBS} 
    ${YOLOV8OBB_LIBS}
    ${UTILS_LIBS}
    ${OPENCV_LIBS}
    nlohmann_json::nlohmann_json
    ${YAML_LIBS}
    ${CURL_LIBRARIES}
    pthread
)