//
//  TensorHelpers.h
//  VisionCamera
//
//  Created by Marc Rousavy on 29.06.23.
//  Copyright © 2023 mrousavy. All rights reserved.
//

#pragma once

#include <jsi/jsi.h>
#include "JSITypedArray.h"
#include <TensorFlowLiteC/TensorFlowLiteC.h>

class TensorHelpers {
public:
  /**
   Get the size of a value of the given `TFLTensorDataType`.
   */
  static size_t getTFLTensorDataTypeSize(TfLiteType dataType);
  /**
   Create a pre-allocated TypedArray for the given TFLTensor.
   */
  static mrousavy::TypedArrayBase createJSBufferForTensor(jsi::Runtime& runtime, TfLiteTensor* tensor);
  /**
   Copies the Tensor's data into a jsi::TypedArray and correctly casts to the given type.
   */
  static void updateJSBuffer(jsi::Runtime& runtime, mrousavy::TypedArrayBase& jsBuffer, TfLiteTensor* tensor);
  /**
   Copies the data from the jsi::TypedArray into the given input buffer.
   */
  static void updateTensorFromJSBuffer(jsi::Runtime& runtime, TfLiteTensor* tensor, mrousavy::TypedArrayBase& jsBuffer);
  /**
   Convert a tensor to a JS Object
   */
  static jsi::Object tensorToJSObject(jsi::Runtime& runtime, TfLiteTensor* tensor);
};
