cmake_minimum_required(VERSION 3.10)
project(app_tests)

set(ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/..)


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
find_package(nlohmann_json 3.2.0 REQUIRED)
# video decode
set(VIDEODECODE_LIBS videodec-c++)
# yolov8 infer
set(YOLOV8_LIBS yolov8-c++)

set(OPENCV_LIBS opencv_highgui opencv_core opencv_imgproc opencv_imgcodecs opencv_videoio)
SET(CMAKE_CXX_FLAGS "-std=c++17  -pthread -Wall -Ofast -Wfatal-errors -D_MWAITXINTRIN_H_INCLUDED -O3")

# videodecode 
add_executable(rtsp_test app_rtsp_test.cpp)
target_include_directories(rtsp_test PRIVATE
    ${ROOT_PATH}/src
    ${ROOT_PATH}/api/include
    ${OPENCV_INCLUDE}
)
target_link_libraries(rtsp_test ${OPENCV_LIBS} ${VIDEODECODE_LIBS})

# yolov8infer
add_executable(yolov8_test app_yolov8_test.cpp)
target_include_directories(yolov8_test PRIVATE
    ${ROOT_PATH}/src
    ${ROOT_PATH}/api/include
    ${OPENCV_INCLUDE}
)
target_link_libraries(yolov8_test ${OPENCV_LIBS} ${VIDEODECODE_LIBS} ${YOLOV8_LIBS} nlohmann_json::nlohmann_json)