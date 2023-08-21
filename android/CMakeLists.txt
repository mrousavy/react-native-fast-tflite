project(VisionCameraTflite)
cmake_minimum_required(VERSION 3.9.0)

set (CMAKE_VERBOSE_MAKEFILE ON)
set (PACKAGE_NAME "VisionCameraTflite")
set (CMAKE_CXX_STANDARD 17)

find_package(ReactAndroid REQUIRED CONFIG)
find_package(fbjni REQUIRED CONFIG)

add_library(
  ${PACKAGE_NAME}
  SHARED
  ../cpp/jsi/Promise.cpp
  ../cpp/jsi/TypedArray.cpp
  ../cpp/TensorflowPlugin.cpp
  ../cpp/TensorHelpers.cpp
  src/main/cpp/Tflite.cpp
)

# Specifies a path to native header files.
target_include_directories(
  ${PACKAGE_NAME}
  PRIVATE
  "../cpp"
  "src/main/cpp"
  "${NODE_MODULES_DIR}/react-native/ReactCommon"
  "${NODE_MODULES_DIR}/react-native/ReactCommon/callinvoker"
  "${NODE_MODULES_DIR}/react-native/ReactAndroid/src/main/jni/react/turbomodule" # <-- CallInvokerHolder JNI wrapper
)

set_target_properties(${PACKAGE_NAME} PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(
  ${PACKAGE_NAME}
  android                         # <-- log
  ReactAndroid::jsi               # <-- jsi.h
  ReactAndroid::reactnativejni    # <-- CallInvokerImpl
  fbjni::fbjni                    # <-- fbjni.h
)
