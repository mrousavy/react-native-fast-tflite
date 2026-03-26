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

#define WRONG_PLATFORM_HINT                                                                        \
  " Make sure you are using the correct delegates for the current platform "                       \
  "(e.g. CoreML/Metal on iOS, GPU/NNAPI on Android)."

namespace margelo::nitro::tflite {

std::shared_ptr<HybridTfliteModelSpec>
HybridTfliteModule::createModel(const std::shared_ptr<ArrayBuffer>& modelData,
                                const std::vector<TensorflowModelDelegate>& delegates) {
  TfLiteModel* model = TfLiteModelCreate(modelData->data(), modelData->size());
  if (model == nullptr) {
    throw std::runtime_error("Failed to create TFLite model from data!");
  }

  TfLiteInterpreterOptions* options = TfLiteInterpreterOptionsCreate();

  for (const TensorflowModelDelegate& delegate : delegates) {
    switch (delegate) {
      case TensorflowModelDelegate::CORE_ML: {
#if FAST_TFLITE_ENABLE_CORE_ML
        TfLiteCoreMlDelegateOptions delegateOptions;
        TfLiteDelegate* coremlDelegate = TfLiteCoreMlDelegateCreate(&delegateOptions);
        TfLiteInterpreterOptionsAddDelegate(options, coremlDelegate);
        break;
#else
        throw std::runtime_error("CoreML Delegate is not enabled! "
                                 "Set $EnableCoreMLDelegate to true in Podfile and rebuild.");
#endif
      }
      case TensorflowModelDelegate::METAL: {
        throw std::runtime_error("Metal Delegate is not supported!");
      }
#ifdef ANDROID
      case TensorflowModelDelegate::NNAPI: {
        TfLiteNnapiDelegateOptions delegateOptions = TfLiteNnapiDelegateOptionsDefault();
        TfLiteDelegate* nnapiDelegate = TfLiteNnapiDelegateCreate(&delegateOptions);
        TfLiteInterpreterOptionsAddDelegate(options, nnapiDelegate);
        break;
      }
      case TensorflowModelDelegate::ANDROID_GPU: {
        TfLiteGpuDelegateOptionsV2 delegateOptions = TfLiteGpuDelegateOptionsV2Default();
        TfLiteDelegate* gpuDelegate = TfLiteGpuDelegateV2Create(&delegateOptions);
        TfLiteInterpreterOptionsAddDelegate(options, gpuDelegate);
        break;
      }
#else
      case TensorflowModelDelegate::NNAPI: {
        throw std::runtime_error(
            "NNAPI Delegate is only supported on Android!" WRONG_PLATFORM_HINT);
      }
      case TensorflowModelDelegate::ANDROID_GPU: {
        throw std::runtime_error(
            "Android-GPU Delegate is only supported on Android!" WRONG_PLATFORM_HINT);
      }
#endif
      default: {
        // use default CPU delegate.
        break;
      }
    }
  }

  TfLiteInterpreter* interpreter = TfLiteInterpreterCreate(model, options);

  // Options and model object can be deleted immediately after interpreter creation.
  // (per TFLite C API docs — the model_data buffer must still outlive the interpreter,
  // which is handled by _modelData shared_ptr in HybridTfliteModel)
  TfLiteInterpreterOptionsDelete(options);
  TfLiteModelDelete(model);

  if (interpreter == nullptr) {
    throw std::runtime_error("Failed to create TFLite interpreter!");
  }

  // Wrap in HybridTfliteModel — stores shared_ptr<ArrayBuffer> to keep bytes alive
  return std::make_shared<HybridTfliteModel>(interpreter, modelData, delegates);
}

} // namespace margelo::nitro::tflite
