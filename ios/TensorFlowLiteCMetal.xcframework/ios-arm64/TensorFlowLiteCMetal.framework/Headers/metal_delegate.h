/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef TENSORFLOW_LITE_DELEGATES_GPU_METAL_DELEGATE_H_
#define TENSORFLOW_LITE_DELEGATES_GPU_METAL_DELEGATE_H_

#import <Metal/Metal.h>

#include "TensorFlowLiteC/common.h"

#ifdef __cplusplus
extern "C" {
#else
// For "C" 'bool' is not built-in type.
#include <stdbool.h>
#endif  // __cplusplus

typedef struct TfLiteDelegate TfLiteDelegate;

typedef enum {
  // waitUntilCompleted
  TFLGpuDelegateWaitTypePassive,
  // Minimize latency. It uses active spinning instead of mutex and consumes
  // additional CPU resources.
  TFLGpuDelegateWaitTypeActive,
  // Useful when the output is used with GPU pipeline then or if external
  // command encoder is set.
  TFLGpuDelegateWaitTypeDoNotWait,
  // Tries to avoid GPU sleep mode.
  TFLGpuDelegateWaitTypeAggressive,
} TFLGpuDelegateWaitType;

// Creates a new delegate instance that need to be destroyed with
// DeleteFlowDelegate when delegate is no longer used by tflite.
typedef struct {
  // Allows to quantify tensors, downcast values, process in float16 etc.
  bool allow_precision_loss;
  TFLGpuDelegateWaitType wait_type;
  // Allows execution of integer quantized models
  bool enable_quantization;
} TFLGpuDelegateOptions;

// Populates TFLGpuDelegateOptions as follows:
//   allow_precision_loss = false;
//   wait_type = TFLGpuDelegateWaitType::TFLGpuDelegateWaitTypePassive;
//   enable_quantization = true;
TFL_CAPI_EXPORT extern TFLGpuDelegateOptions TFLGpuDelegateOptionsDefault(void);

// Creates a new delegate instance that need to be destroyed with
// `TFLDeleteTfLiteGpuDelegate` when delegate is no longer used by TFLite.
// When `options` is set to `nullptr`, the following default values are used:
// .precision_loss_allowed = false,
// .wait_type = kPassive,
TFL_CAPI_EXPORT extern TfLiteDelegate* TFLGpuDelegateCreate(
    const TFLGpuDelegateOptions* options);

// Destroys a delegate created with `TFLGpuDelegateCreate` call.
TFL_CAPI_EXPORT extern void TFLGpuDelegateDelete(TfLiteDelegate* delegate);

// Binds Metal buffer to an input or an output tensor in the initialized
// delegate. Bound buffer should have sufficient storage to accommodate all
// elements of a tensor. For quantized model, the buffer is bound to internal
// dequantized float32 tensor.
// Returns non-zero on success, or zero otherwise.
//
// *** Must be called *after* `Interpreter::ModifyGraphWithDelegate`. ***
// WARNING: This is an experimental API and subject to change.
TFL_CAPI_EXPORT extern bool TFLGpuDelegateBindMetalBufferToTensor(
    TfLiteDelegate* delegate, int tensor_index, id<MTLBuffer> metal_buffer);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // TENSORFLOW_LITE_DELEGATES_GPU_METAL_DELEGATE_H_
