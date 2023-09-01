//
//  TensorHelpers.h
//  VisionCamera
//
//  Created by Marc Rousavy on 29.06.23.
//  Copyright © 2023 mrousavy. All rights reserved.
//

#pragma once

#include "jsi/TypedArray.h"
#include <jsi/jsi.h>

#ifdef ANDROID
#include <tensorflow/lite/c/c_api.h>
#else
#include <TensorFlowLiteC/TensorFlowLiteC.h>
#endif

using namespace facebook;

class TensorHelpers {
public:
  /**
   Get the size of a value of the given `TFLTensorDataType`.
   */
  static size_t getTFLTensorDataTypeSize(TfLiteType dataType);
  /**
   Create a pre-allocated TypedArray for the given TFLTensor.
   */
  static mrousavy::TypedArrayBase createJSBufferForTensor(jsi::Runtime& runtime,
                                                          const TfLiteTensor* tensor);
  /**
   Copies the Tensor's data into a jsi::TypedArray and correctly casts to the given type.
   */
  static void updateJSBufferFromTensor(jsi::Runtime& runtime, mrousavy::TypedArrayBase& jsBuffer,
                                       const TfLiteTensor* outputTensor);
  /**
   Copies the data from the jsi::TypedArray into the given input buffer.
   */
  static void updateTensorFromJSBuffer(jsi::Runtime& runtime, TfLiteTensor* inputTensor,
                                       mrousavy::TypedArrayBase& jsBuffer);
  /**
   Convert a tensor to a JS Object
   */
  static jsi::Object tensorToJSObject(jsi::Runtime& runtime, const TfLiteTensor* tensor);
};
