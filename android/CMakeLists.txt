project(NitroTflite)
cmake_minimum_required(VERSION 3.9.0)

set (PACKAGE_NAME NitroTflite)
set (CMAKE_VERBOSE_MAKEFILE ON)
set (CMAKE_CXX_STANDARD 20)

# Find TFLite libraries
find_library(
  TFLITE
  tensorflowlite_jni
  PATHS "./src/main/cpp/lib/litert/jni/${ANDROID_ABI}"
  NO_DEFAULT_PATH
  NO_CMAKE_FIND_ROOT_PATH
)

find_library(
  TFLITE_GPU
  tensorflowlite_gpu_jni
  PATHS "./src/main/cpp/lib/litert/jni/${ANDROID_ABI}"
  NO_DEFAULT_PATH
  NO_CMAKE_FIND_ROOT_PATH
)

# -DANDROID flag
string(APPEND CMAKE_CXX_FLAGS " -DANDROID")

# Define C++ library and add all sources
add_library(${PACKAGE_NAME} SHARED
        src/main/cpp/cpp-adapter.cpp
        ../cpp/HybridTfliteModel.cpp
        ../cpp/HybridTfliteModule.cpp
        ../cpp/TfliteHelpers.cpp
)

# Add Nitrogen specs :)
include(${CMAKE_SOURCE_DIR}/../nitrogen/generated/android/NitroTflite+autolinking.cmake)

# Page size linker options
target_link_options(${PACKAGE_NAME}
    PRIVATE
    "-Wl,-z,max-page-size=16384"
    "-Wl,-z,common-page-size=16384"
)

# Set up local includes
include_directories(
        "src/main/cpp"
        "../cpp"
        "src/main/cpp/lib/litert/headers"
        "src/main/cpp/lib/litert/headers/tflite"
)

find_library(LOG_LIB log)

# Link all libraries together
target_link_libraries(
        ${PACKAGE_NAME}
        ${LOG_LIB}
        android
        ${TFLITE}
        ${TFLITE_GPU}
)
