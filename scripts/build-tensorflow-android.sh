#!/bin/bash

# Assumes the user ran ./configure in tensorflow/ already
cd tensorflow

ANDROID_SDK_HOME="$ANDROID_HOME"
ANDROID_NDK_HOME="$ANDROID_HOME/ndk/23.1.7779620"
ANDROID_NDK_API_LEVEL="26"
ANDROID_BUILD_TOOLS_VERSION="30.0.3"
ANDROID_SDK_API_LEVEL="30"

bazel build -c opt --fat_apk_cpu=x86,x86_64,arm64-v8a,armeabi-v7a \
  --host_crosstool_top=@bazel_tools//tools/cpp:toolchain \
  --define=android_dexmerger_tool=d8_dexmerger \
  --define=android_incremental_dexing_tool=d8_dexbuilder \
  //tensorflow/lite/java:tensorflow-lite

cd ..
