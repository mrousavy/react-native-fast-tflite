#include "HybridTfliteModule.hpp"
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

/**
 * Return a Hardware accelerated delegate, or throws
 * if the given delegate type is not available.
 */
TfLiteDelegate* getDelegate(TensorflowModelDelegate delegateType) {
  switch (delegateType) {
    case TensorflowModelDelegate::CORE_ML:
      return getCoreMLDelegate();
    case TensorflowModelDelegate::METAL:
      return getMetalDelegate();
    case TensorflowModelDelegate::NNAPI:
      return getNNAPIDelegate();
    case TensorflowModelDelegate::ANDROID_GPU:
      return getAndroidGPUDelegate();
  }
  throw std::runtime_error("Unknown Delegate \"" + std::to_string(static_cast<int>(delegateType)) +
                           "\"!");
}

std::shared_ptr<HybridTfliteModelSpec>
HybridTfliteModule::createModel(const std::shared_ptr<ArrayBuffer>& modelData,
                                const std::vector<TensorflowModelDelegate>& delegates) {
  TfLiteModel* model = TfLiteModelCreate(modelData->data(), modelData->size());
  if (model == nullptr) {
    throw std::runtime_error("Failed to create TFLite model from data!");
  }

  // Configure interpreter via options
  TfLiteInterpreterOptions* options = TfLiteInterpreterOptionsCreate();

  // Add all hardware accelerated delegates (e.g. GPU, NPU, ...)
  // if any. The default CPU delegate will always be available.
  for (const TensorflowModelDelegate& delegateType : delegates) {
    TfLiteDelegate* delegate = getDelegate(delegateType);
    TfLiteInterpreterOptionsAddDelegate(options, delegate);
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

  // Wrap in HybridTfliteModel — stores shared_ptr<ArrayBuffer> to keep model data bytes alive
  return std::make_shared<HybridTfliteModel>(interpreter, modelData, delegates);
}

} // namespace margelo::nitro::tflite
