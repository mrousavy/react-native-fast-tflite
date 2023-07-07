#!/bin/bash

# Assumes the user ran ./configure in tensorflow/ already
cd tensorflow

# Build tflite, Metal delegate and CoreML delegate
bazel build --config=ios_fat -c opt --cxxopt=--std=c++17 //tensorflow/lite/ios:TensorFlowLiteC_framework
bazel build --config=ios_fat -c opt --cxxopt=--std=c++17 //tensorflow/lite/ios:TensorFlowLiteCCoreML_framework
bazel build --config=ios_fat -c opt --cxxopt=--std=c++17 //tensorflow/lite/ios:TensorFlowLiteCMetal_framework

cd ..
cp -r tensorflow/bazel-bin/tensorflow/lite/ios/ ios/

# Extract tflite
unzip -o ios/TensorFlowLiteC_framework.zip -d ios
rm ios/TensorFlowLiteC_framework.zip

# Extract CoreML delegate
unzip -o ios/TensorFlowLiteCCoreML_framework.zip -d ios
rm ios/TensorFlowLiteCCoreML_framework.zip

# Extract Metal delegate
unzip -o ios/TensorFlowLiteCMetal_framework.zip -d ios
rm ios/TensorFlowLiteCMetal_framework.zip
