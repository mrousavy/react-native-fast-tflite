#include "HybridTfliteModule.hpp"
#include "TfliteHelpers.hpp"

#include <functional>
#include <utility>
#include <vector>

#if defined(ANDROID)
#include <tflite/c/c_api.h>
#include <tflite/delegates/gpu/delegate.h>
#include <tflite/delegates/nnapi/nnapi_delegate_c_api.h>
#elif defined(__APPLE__)
#include <TensorFlowLiteC/TensorFlowLiteC.h>
#if FAST_TFLITE_ENABLE_CORE_ML
#include <TensorFlowLiteCCoreML/TensorFlowLiteCCoreML.h>
#endif
#else
#error "Invalid Platform!"
#endif

namespace margelo::nitro::tflite {

/**
 * TFLite's C API does not transfer delegate ownership to the interpreter —
 * the caller must keep the delegate alive and free it itself (after the
 * interpreter has been deleted). Returns nullptr for delegate types without
 * a public delete function.
 */
static std::function<void()> makeDelegateDeleter(TensorflowModelDelegate delegateType,
                                                 TfLiteDelegate* delegate) {
  switch (delegateType) {
#if defined(__APPLE__) && FAST_TFLITE_ENABLE_CORE_ML
    case TensorflowModelDelegate::CORE_ML:
      return [delegate]() { TfLiteCoreMlDelegateDelete(delegate); };
#endif
#if defined(ANDROID)
    case TensorflowModelDelegate::ANDROID_GPU:
      return [delegate]() { TfLiteGpuDelegateV2Delete(delegate); };
    case TensorflowModelDelegate::NNAPI:
      return [delegate]() { TfLiteNnapiDelegateDelete(delegate); };
#endif
    default:
      // METAL never produces a delegate; unknown types were rejected earlier.
      return nullptr;
  }
}

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
  std::vector<TensorflowModelDelegate> effectiveDelegates;
  std::vector<std::function<void()>> delegateDeleters;
  effectiveDelegates.reserve(delegates.size());
  for (const TensorflowModelDelegate& delegateType : delegates) {
    TfLiteDelegate* delegate = getDelegate(delegateType);
    if (delegate == nullptr) {
      // e.g. CoreML on devices without a Neural Engine — fall back to CPU
      // instead of registering a null delegate with the interpreter.
      continue;
    }
    TfLiteInterpreterOptionsAddDelegate(options, delegate);
    effectiveDelegates.push_back(delegateType);
    if (auto deleter = makeDelegateDeleter(delegateType, delegate)) {
      delegateDeleters.push_back(std::move(deleter));
    }
  }

  TfLiteInterpreter* interpreter = TfLiteInterpreterCreate(model, options);

  // Options and model object can be deleted immediately after interpreter creation.
  // (per TFLite C API docs — the model_data buffer must still outlive the interpreter,
  // which is handled by _modelData shared_ptr in HybridTfliteModel)
  TfLiteInterpreterOptionsDelete(options);
  TfLiteModelDelete(model);

  if (interpreter == nullptr) {
    for (auto& deleter : delegateDeleters) {
      deleter();
    }
    throw std::runtime_error("Failed to create TFLite interpreter!");
  }

  // Wrap in HybridTfliteModel — stores shared_ptr<ArrayBuffer> to keep model data bytes alive
  return std::make_shared<HybridTfliteModel>(interpreter, modelData, effectiveDelegates,
                                             std::move(delegateDeleters));
}

} // namespace margelo::nitro::tflite
