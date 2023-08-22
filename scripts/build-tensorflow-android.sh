#!/bin/bash

# Assumes the user ran ./configure in tensorflow/ already
cd tensorflow

ANDROID_SDK_HOME="$ANDROID_HOME"
ANDROID_NDK_HOME="$ANDROID_HOME/ndk/21.4.7075529"
ANDROID_NDK_API_LEVEL="26"
ANDROID_BUILD_TOOLS_VERSION="30.0.3"
ANDROID_SDK_API_LEVEL="30"

cmake tensorflow/lite \
  -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI=arm64-v8a \
  -DTFLITE_ENABLE_INSTALL=ON \
  -DTFLITE_ENABLE_GPU=ON

# bazel build -c opt --fat_apk_cpu=x86,x86_64,arm64-v8a,armeabi-v7a \
#   --host_crosstool_top=@bazel_tools//tools/cpp:toolchain \
#   --define=android_dexmerger_tool=d8_dexmerger \
#   --define=android_incremental_dexing_tool=d8_dexbuilder \
#   //tensorflow/lite/java:tensorflow-lite

cd ..
