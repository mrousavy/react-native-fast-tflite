#include "HybridTfliteModule.hpp"
#include "TfliteHelpers.hpp"

#ifdef ANDROID
#include <tflite/c/c_api.h>
#include <tflite/delegates/gpu/delegate.h>
#include <tflite/delegates/nnapi/nnapi_delegate_c_api.h>
#else
#include <TensorFlowLiteC/TensorFlowLiteC.h>

#if FAST_TFLITE_ENABLE_CORE_ML
#include <TensorFlowLiteCCoreML/TensorFlowLiteCCoreML.h>
#endif
#endif

namespace margelo::nitro::nitrotflite {

std::shared_ptr<HybridTfliteModelSpec>
HybridTfliteModule::createModel(const std::shared_ptr<ArrayBuffer>& data,
                                TensorflowModelDelegate delegate) {
  auto model = TfLiteModelCreate(data->data(), data->size());
  if (model == nullptr) {
    throw std::runtime_error("Failed to create TFLite model from data!");
  }

  auto options = TfLiteInterpreterOptionsCreate();

  switch (delegate) {
    case TensorflowModelDelegate::CORE_ML: {
#if FAST_TFLITE_ENABLE_CORE_ML
      TfLiteCoreMlDelegateOptions delegateOptions;
      auto coremlDelegate = TfLiteCoreMlDelegateCreate(&delegateOptions);
      TfLiteInterpreterOptionsAddDelegate(options, coremlDelegate);
      break;
#else
      throw std::runtime_error("CoreML Delegate is not enabled! Set $EnableCoreMLDelegate to true "
                               "in Podfile and rebuild.");
#endif
    }
    case TensorflowModelDelegate::METAL: {
      throw std::runtime_error("Metal Delegate is not supported!");
    }
#ifdef ANDROID
    case TensorflowModelDelegate::NNAPI: {
      TfLiteNnapiDelegateOptions delegateOptions = TfLiteNnapiDelegateOptionsDefault();
      auto nnapiDelegate = TfLiteNnapiDelegateCreate(&delegateOptions);
      TfLiteInterpreterOptionsAddDelegate(options, nnapiDelegate);
      break;
    }
    case TensorflowModelDelegate::ANDROID_GPU: {
      TfLiteGpuDelegateOptionsV2 delegateOptions = TfLiteGpuDelegateOptionsV2Default();
      auto gpuDelegate = TfLiteGpuDelegateV2Create(&delegateOptions);
      TfLiteInterpreterOptionsAddDelegate(options, gpuDelegate);
      break;
    }
#else
    case TensorflowModelDelegate::NNAPI: {
      throw std::runtime_error("Nnapi Delegate is only supported on Android!");
    }
    case TensorflowModelDelegate::ANDROID_GPU: {
      throw std::runtime_error("Android-Gpu Delegate is only supported on Android!");
    }
#endif
    default: {
      // use default CPU delegate.
      break;
    }
  }

  auto interpreter = TfLiteInterpreterCreate(model, options);

  // Options and model object can be deleted immediately after interpreter creation.
  // (per TFLite C API docs — the model_data buffer must still outlive the interpreter,
  // which is handled by _modelData shared_ptr in HybridTfliteModel)
  TfLiteInterpreterOptionsDelete(options);
  TfLiteModelDelete(model);

  if (interpreter == nullptr) {
    throw std::runtime_error("Failed to create TFLite interpreter!");
  }

  // Wrap in HybridTfliteModel — stores shared_ptr<ArrayBuffer> to keep bytes alive
  return std::make_shared<HybridTfliteModel>(interpreter, data, delegate);
}

} // namespace margelo::nitro::nitrotflite
