#pragma once

#include "HybridTfliteModelSpec.hpp"
#include <memory>
#include <string>
#include <unordered_map>

#ifdef ANDROID
#include <tflite/c/c_api.h>
#else
#include <TensorFlowLiteC/TensorFlowLiteC.h>
#endif

namespace margelo::nitro::tflite {

class HybridTfliteModel : public HybridTfliteModelSpec {
public:
  explicit HybridTfliteModel(TfLiteInterpreter* interpreter, std::shared_ptr<ArrayBuffer> modelData,
                             std::vector<TensorflowModelDelegate> delegates);
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
  std::unordered_map<std::string, std::shared_ptr<ArrayBuffer>> _outputBuffers;
};

} // namespace margelo::nitro::tflite
