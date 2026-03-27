#include "TfliteHelpers.hpp"

#ifdef ANDROID
#include <tflite/c/c_api.h>
#include <tflite/delegates/gpu/delegate.h>
#include <tflite/delegates/nnapi/nnapi_delegate_c_api.h>
#endif

#ifdef __APPLE__
#include <TensorFlowLiteC/TensorFlowLiteC.h>
#if FAST_TFLITE_ENABLE_CORE_ML
#include <TensorFlowLiteCCoreML/TensorFlowLiteCCoreML.h>
#endif
#endif

namespace margelo::nitro::tflite {

// TODO: Remove this, this doesn't seem like a good idea at all.
typedef float float32_t;
typedef double float64_t;

std::string tfLiteStatusToString(TfLiteStatus status) {
  switch (status) {
    case kTfLiteOk:
      return "ok";
    case kTfLiteError:
      return "error";
    case kTfLiteDelegateError:
      return "delegate-error";
    case kTfLiteApplicationError:
      return "application-error";
    case kTfLiteDelegateDataNotFound:
      return "delegate-data-not-found";
    case kTfLiteDelegateDataWriteError:
      return "delegate-data-write-error";
    case kTfLiteDelegateDataReadError:
      return "delegate-data-read-error";
    case kTfLiteUnresolvedOps:
      return "unresolved-ops";
    case kTfLiteCancelled:
      return "cancelled";
  }
  return "unknown";
}

std::string dataTypeToString(TfLiteType dataType) {
  switch (dataType) {
    case kTfLiteFloat16:
      return "float16";
    case kTfLiteFloat32:
      return "float32";
    case kTfLiteFloat64:
      return "float64";
    case kTfLiteBFloat16:
      return "bfloat16";
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
  }
  return "invalid";
}

size_t getTFLTensorDataTypeSize(TfLiteType dataType) {
  switch (dataType) {
    case kTfLiteBool:
      return sizeof(bool);
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
      throw std::runtime_error("Tensor DataType \"" + dataTypeToString(dataType) +
                               "\" is not supported!");
  }
}

int getTensorTotalLength(const TfLiteTensor* tensor) {
  int dimensions = TfLiteTensorNumDims(tensor);
  if (dimensions < 1)
    return 0;
  int size = 1;
  for (int32_t i = 0; i < dimensions; i++) {
    size *= TfLiteTensorDim(tensor, i);
  }
  return size;
}

TfLiteDelegate* getCoreMLDelegate() {
#ifdef __APPLE__
#if FAST_TFLITE_ENABLE_CORE_ML
  TfLiteCoreMlDelegateOptions delegateOptions;
  TfLiteDelegate* coreMlDelegate = TfLiteCoreMlDelegateCreate(&delegateOptions);
  return coreMlDelegate;
#else // FAST_TFLITE_ENABLE_CORE_ML
  throw std::runtime_error("The CoreML Delegate (\"core-ml\") is not enabled! "
                           "Set `$EnableCoreMLDelegate` to `true` in your Podfile, and rebuild.");
#endif
#else // __APPLE__
  throw std::runtime_error(
      "The CoreML Delegate (\"core-ml\") is only supported on Apple Platforms!");
#endif
}

TfLiteDelegate* getMetalDelegate() {
  throw std::runtime_error("Metal Delegate is not yet supported!");
}

TfLiteDelegate* getNNAPIDelegate() {
#ifdef ANDROID
  TfLiteNnapiDelegateOptions delegateOptions = TfLiteNnapiDelegateOptionsDefault();
  TfLiteDelegate* nnapiDelegate = TfLiteNnapiDelegateCreate(&delegateOptions);
  return nnapiDelegate;
#else // ANDROID
  throw std::runtime_error("The NNAPI Delegate (\"nnapi\") is only supported on Android!");
#endif
}

TfLiteDelegate* getAndroidGPUDelegate() {
#ifdef ANDROID
  TfLiteGpuDelegateOptionsV2 delegateOptions = TfLiteGpuDelegateOptionsV2Default();
  TfLiteDelegate* gpuDelegate = TfLiteGpuDelegateV2Create(&delegateOptions);
#else // ANDROID
  throw std::runtime_error(
      "The Android GPU Delegate (\"android-gpu\") is only supported on Android!");
#endif
}

} // namespace margelo::nitro::tflite
