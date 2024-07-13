#!/bin/bash

echo "Did you run ./configure with iOS + Android support in ./tensorflow/ already?"
echo "Did you run ./configure with iOS + Android support in ./tensorflow/ already?"
echo "Did you run ./configure with iOS + Android support in ./tensorflow/ already?"
echo "Did you run ./configure with iOS + Android support in ./tensorflow/ already?"
echo "Did you run ./configure with iOS + Android support in ./tensorflow/ already?"
echo "Did you run ./configure with iOS + Android support in ./tensorflow/ already?"
echo "Did you run ./configure with iOS + Android support in ./tensorflow/ already?"
echo "Did you run ./configure with iOS + Android support in ./tensorflow/ already?"

read -p "Did you run ./configure with iOS + Android in /tensorflow/ already? [yN]: " -n 1 -r
echo ""
if [[ $REPLY =~ ^[Yy]$ ]]
then
  # Assumes the user ran ./configure in tensorflow/ already
  cd tensorflow

  bazel build --config=android # -c opt --cxxopt=--std=c++17 //tensorflow/lite/ios:TensorFlowLiteC_framework

  cd ..

  cp -f -r tensorflow/bazel-bin/tensorflow/lite/android/ android/

  echo "What now?"
fi
