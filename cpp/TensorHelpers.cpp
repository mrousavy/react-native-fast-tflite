//
//  TensorHelpers.mm
//  VisionCamera
//
//  Created by Marc Rousavy on 29.06.23.
//  Copyright © 2023 mrousavy. All rights reserved.
//

#include "TensorHelpers.h"

#ifdef ANDROID
#include <tensorflow/lite/c/c_api.h>
#else
#include <TensorFlowLiteC/TensorFlowLiteC.h>
#endif

using namespace mrousavy;

typedef float float32_t;
typedef double float64_t;

std::string dataTypeToString(TfLiteType dataType) {
  switch (dataType) {
    case kTfLiteFloat32:
      return "float32";
    case kTfLiteFloat64:
      return "float64";
    case kTfLiteInt4:
      return "int4";
    case kTfLiteInt8:
      return "int8";
    case kTfLiteInt16:
      return "int16";
    case kTfLiteInt32:
      return "int32";
    case kTfLiteInt64:
      return "int64";
    case kTfLiteUInt8:
      return "uint8";
    case kTfLiteUInt16:
      return "uint16";
    case kTfLiteUInt32:
      return "uint32";
    case kTfLiteUInt64:
      return "uint64";
    case kTfLiteNoType:
      return "none";
    case kTfLiteString:
      return "string";
    case kTfLiteBool:
      return "bool";
    case kTfLiteComplex64:
      return "complex64";
    case kTfLiteComplex128:
      return "complex128";
    case kTfLiteResource:
      return "resource";
    case kTfLiteVariant:
      return "variant";
    default:
      [[unlikely]];
      return "invalid";
  }
}

TfLiteType getTFLDataTypeForTypedArrayKind(TypedArrayKind kind) {
  switch (kind) {
    case TypedArrayKind::Int8Array:
      return kTfLiteInt8;
    case TypedArrayKind::Int16Array:
      return kTfLiteInt16;
    case TypedArrayKind::Int32Array:
      return kTfLiteInt32;
    case TypedArrayKind::Uint8Array:
      return kTfLiteUInt8;
    case TypedArrayKind::Uint8ClampedArray:
      return kTfLiteUInt8;
    case TypedArrayKind::Uint16Array:
      return kTfLiteUInt16;
    case TypedArrayKind::Uint32Array:
      return kTfLiteUInt32;
    case TypedArrayKind::Float32Array:
      return kTfLiteFloat32;
    case TypedArrayKind::Float64Array:
      return kTfLiteFloat64;
    case TypedArrayKind::BigInt64Array:
      return kTfLiteInt64;
    case TypedArrayKind::BigUint64Array:
      return kTfLiteUInt64;
  }
}

size_t TensorHelpers::getTFLTensorDataTypeSize(TfLiteType dataType) {
  switch (dataType) {
    case kTfLiteFloat32:
      return sizeof(float32_t);
    case kTfLiteInt32:
      return sizeof(int32_t);
    case kTfLiteUInt8:
      return sizeof(uint8_t);
    case kTfLiteInt64:
      return sizeof(int64_t);
    case kTfLiteInt16:
      return sizeof(int16_t);
    case kTfLiteInt8:
      return sizeof(int8_t);
    case kTfLiteFloat64:
      return sizeof(float64_t);
    case kTfLiteUInt64:
      return sizeof(uint64_t);
    case kTfLiteUInt32:
      return sizeof(uint32_t);
    case kTfLiteUInt16:
      return sizeof(uint16_t);
    default:
      [[unlikely]];
      throw std::runtime_error("TFLite: Unsupported output data type! " +
                               dataTypeToString(dataType));
  }
}

int getTensorTotalLength(const TfLiteTensor* tensor) {
  int dimensions = TfLiteTensorNumDims(tensor);
  if (dimensions < 1) {
    // TODO: Handle error here, there is something wrong with this tensor...
    return 0;
  }

  int size = 1;
  for (size_t i = 0; i < dimensions; i++) {
    size *= TfLiteTensorDim(tensor, i);
  }
  return size;
}

TypedArrayBase TensorHelpers::createJSBufferForTensor(jsi::Runtime& runtime,
                                                      const TfLiteTensor* tensor) {
  int size = getTensorTotalLength(tensor);

  auto dataType = TfLiteTensorType(tensor);
  switch (dataType) {
    case kTfLiteFloat32:
      return TypedArray<TypedArrayKind::Float32Array>(runtime, size);
    case kTfLiteFloat64:
      return TypedArray<TypedArrayKind::Float64Array>(runtime, size);
    case kTfLiteInt8:
      return TypedArray<TypedArrayKind::Int8Array>(runtime, size);
    case kTfLiteInt16:
      return TypedArray<TypedArrayKind::Int16Array>(runtime, size);
    case kTfLiteInt32:
      return TypedArray<TypedArrayKind::Int32Array>(runtime, size);
    case kTfLiteUInt8:
      return TypedArray<TypedArrayKind::Uint8Array>(runtime, size);
    case kTfLiteUInt16:
      return TypedArray<TypedArrayKind::Uint16Array>(runtime, size);
    case kTfLiteUInt32:
      return TypedArray<TypedArrayKind::Uint32Array>(runtime, size);
    case kTfLiteInt64:
      return TypedArray<TypedArrayKind::BigInt64Array>(runtime, size);
    case kTfLiteUInt64:
      return TypedArray<TypedArrayKind::BigUint64Array>(runtime, size);
    default:
      [[unlikely]];
      throw std::runtime_error("TFLite: Unsupported tensor data type! " +
                               dataTypeToString(dataType));
  }
}

void TensorHelpers::updateJSBufferFromTensor(jsi::Runtime& runtime, TypedArrayBase& jsBuffer,
                                             const TfLiteTensor* tensor) {
  auto name = std::string(TfLiteTensorName(tensor));
  auto dataType = TfLiteTensorType(tensor);

  void* data = TfLiteTensorData(tensor);
  if (data == nullptr) {
    [[unlikely]];
    throw std::runtime_error("TFLite: Failed to get data from tensor \"" + name + "\"!");
  }

  // count of bytes, may be larger than count of numbers (e.g. for float32)
  int size = getTensorTotalLength(tensor) * getTFLTensorDataTypeSize(dataType);

  switch (dataType) {
    case kTfLiteFloat32:
      getTypedArray(runtime, jsBuffer)
          .as<TypedArrayKind::Float32Array>(runtime)
          .updateUnsafe(runtime, (float32_t*)data, size);
      break;
    case kTfLiteFloat64:
      getTypedArray(runtime, jsBuffer)
          .as<TypedArrayKind::Float64Array>(runtime)
          .updateUnsafe(runtime, (float64_t*)data, size);
      break;
    case kTfLiteInt8:
      getTypedArray(runtime, jsBuffer)
          .as<TypedArrayKind::Int8Array>(runtime)
          .updateUnsafe(runtime, (int8_t*)data, size);
      break;
    case kTfLiteInt16:
      getTypedArray(runtime, jsBuffer)
          .as<TypedArrayKind::Int16Array>(runtime)
          .updateUnsafe(runtime, (int16_t*)data, size);
      break;
    case kTfLiteInt32:
      getTypedArray(runtime, jsBuffer)
          .as<TypedArrayKind::Int32Array>(runtime)
          .updateUnsafe(runtime, (int32_t*)data, size);
      break;
    case kTfLiteUInt8:
      getTypedArray(runtime, jsBuffer)
          .as<TypedArrayKind::Uint8Array>(runtime)
          .updateUnsafe(runtime, (uint8_t*)data, size);
      break;
    case kTfLiteUInt16:
      getTypedArray(runtime, jsBuffer)
          .as<TypedArrayKind::Uint16Array>(runtime)
          .updateUnsafe(runtime, (uint16_t*)data, size);
      break;
    case kTfLiteUInt32:
      getTypedArray(runtime, jsBuffer)
          .as<TypedArrayKind::Uint32Array>(runtime)
          .updateUnsafe(runtime, (uint32_t*)data, size);
      break;
    case kTfLiteInt64:
      getTypedArray(runtime, jsBuffer)
          .as<TypedArrayKind::BigInt64Array>(runtime)
          .updateUnsafe(runtime, (int64_t*)data, size);
      break;
    case kTfLiteUInt64:
      getTypedArray(runtime, jsBuffer)
          .as<TypedArrayKind::BigUint64Array>(runtime)
          .updateUnsafe(runtime, (uint64_t*)data, size);
      break;
    default:
      [[unlikely]];
      throw jsi::JSError(runtime,
                         "TFLite: Unsupported output data type! " + dataTypeToString(dataType));
  }
}

void TensorHelpers::updateTensorFromJSBuffer(jsi::Runtime& runtime, TfLiteTensor* tensor,
                                             TypedArrayBase& jsBuffer) {
#if DEBUG
  // Validate data-type
  TypedArrayKind kind = jsBuffer.getKind(runtime);
  TfLiteType receivedType = getTFLDataTypeForTypedArrayKind(kind);
  TfLiteType expectedType = TfLiteTensorType(tensor);
  if (receivedType != expectedType) {
    [[unlikely]];
    throw std::runtime_error("TFLite: Invalid input type! Model expected " +
                             dataTypeToString(expectedType) + ", but received " +
                             dataTypeToString(receivedType) + "!");
  }
#endif

  std::string name = TfLiteTensorName(tensor);
  jsi::ArrayBuffer buffer = jsBuffer.getBuffer(runtime);

#if DEBUG
  // Validate size
  int inputBufferSize = buffer.size(runtime);
  int tensorSize = getTensorTotalLength(tensor) * getTFLTensorDataTypeSize(tensor->type);
  if (tensorSize != inputBufferSize) {
    [[unlikely]];
    throw std::runtime_error("TFLite: Input Buffer size (" + std::to_string(inputBufferSize) +
                             ") does not match the Input Tensor's expected size (" +
                             std::to_string(tensorSize) +
                             ")! Make sure to resize the input values accordingly.");
  }
#endif

  TfLiteTensorCopyFromBuffer(tensor, buffer.data(runtime) + jsBuffer.byteOffset(runtime),
                             buffer.size(runtime));
}

jsi::Object TensorHelpers::tensorToJSObject(jsi::Runtime& runtime, const TfLiteTensor* tensor) {
  jsi::Object result(runtime);
  result.setProperty(runtime, "name",
                     jsi::String::createFromUtf8(runtime, TfLiteTensorName(tensor)));
  result.setProperty(
      runtime, "dataType",
      jsi::String::createFromUtf8(runtime, dataTypeToString(TfLiteTensorType(tensor))));

  int dimensions = TfLiteTensorNumDims(tensor);
  jsi::Array shapeArray(runtime, dimensions);
  for (size_t i = 0; i < dimensions; i++) {
    int size = TfLiteTensorDim(tensor, i);
    shapeArray.setValueAtIndex(runtime, i, jsi::Value(size));
  }
  result.setProperty(runtime, "shape", shapeArray);

  return result;
}
