#pragma once

#include "TensorDataType.hpp"
#include <string>

#if defined(ANDROID)
#include <tflite/c/c_api.h>
#elif defined(__APPLE__)
#include <TensorFlowLiteC/TensorFlowLiteC.h>
#else
#error "Invalid Platform!"
#endif

namespace margelo::nitro::tflite {

std::string tfLiteStatusToString(TfLiteStatus status);
TensorDataType getTensorDataType(TfLiteType dataType);
size_t getTFLTensorDataTypeSize(TfLiteType dataType);
int getTensorTotalLength(const TfLiteTensor* tensor);

TfLiteDelegate* getCoreMLDelegate();
TfLiteDelegate* getMetalDelegate();
TfLiteDelegate* getNNAPIDelegate();
TfLiteDelegate* getAndroidGPUDelegate();

} // namespace margelo::nitro::tflite
