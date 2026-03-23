#pragma once

#include <string>

#ifdef ANDROID
#include <tflite/c/c_api.h>
#else
#include <TensorFlowLiteC/TensorFlowLiteC.h>
#endif

std::string tfLiteStatusToString(TfLiteStatus status);
std::string dataTypeToString(TfLiteType dataType);
size_t getTFLTensorDataTypeSize(TfLiteType dataType);
int getTensorTotalLength(const TfLiteTensor* tensor);
