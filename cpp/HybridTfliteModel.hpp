#pragma once

#include "HybridTfliteModelSpec.hpp"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#if defined(ANDROID)
#include <tflite/c/c_api.h>
#elif defined(__APPLE__)
#include <TensorFlowLiteC/TensorFlowLiteC.h>
#else
#error "Invalid Platform!"
#endif

namespace margelo::nitro::tflite {

class HybridTfliteModel : public HybridTfliteModelSpec {
public:
  explicit HybridTfliteModel(TfLiteInterpreter* interpreter, std::shared_ptr<ArrayBuffer> modelData,
                             std::vector<TensorflowModelDelegate> delegates,
                             std::vector<std::function<void()>> delegateDeleters);
  ~HybridTfliteModel();

  // Properties (from HybridTfliteModelSpec)
  std::vector<TensorflowModelDelegate> getDelegates() override;
  std::vector<Tensor> getInputs() override;
  std::vector<Tensor> getOutputs() override;

  // Methods (from HybridTfliteModelSpec)
  std::vector<std::shared_ptr<ArrayBuffer>>
  runSync(const std::vector<std::shared_ptr<ArrayBuffer>>& input) override;
  std::shared_ptr<Promise<std::vector<std::shared_ptr<ArrayBuffer>>>>
  run(const std::vector<std::shared_ptr<ArrayBuffer>>& input) override;

private:
  void copyInputBuffers(const std::vector<std::shared_ptr<ArrayBuffer>>& input);
  void invoke();
  std::vector<std::shared_ptr<ArrayBuffer>> copyOutputBuffers();
  std::shared_ptr<ArrayBuffer> getOutputBufferForTensor(const TfLiteTensor* tensor);

private:
  TfLiteInterpreter* _interpreter = nullptr;
  std::vector<TensorflowModelDelegate> _delegates;
  std::shared_ptr<ArrayBuffer> _modelData;
  // Free the TFLite delegates (GPU/CoreML/NNAPI) — the interpreter does not
  // own them, so without these every model destruction leaks the delegate's
  // compiled kernels / driver contexts.
  std::vector<std::function<void()>> _delegateDeleters;
  std::unordered_map<std::string, std::shared_ptr<ArrayBuffer>> _outputBuffers;
};

} // namespace margelo::nitro::tflite
