#include "HybridTfliteModel.hpp"
#include "TfliteHelpers.hpp"

#include <NitroModules/ArrayBuffer.hpp>
#include <NitroModules/Promise.hpp>
#include <string>

namespace margelo::nitro::nitrotflite {

HybridTfliteModel::HybridTfliteModel(TfLiteInterpreter* interpreter,
                                     std::shared_ptr<ArrayBuffer> modelData,
                                     TensorflowModelDelegate delegate)
    : HybridObject(TAG), _interpreter(interpreter), _delegate(delegate), _modelData(modelData) {
  TfLiteStatus status = TfLiteInterpreterAllocateTensors(_interpreter);
  if (status != kTfLiteOk) {
    throw std::runtime_error(
        "TFLite: Failed to allocate memory for input/output tensors! Status: " +
        tfLiteStatusToString(status));
  }
}

HybridTfliteModel::~HybridTfliteModel() {
  if (_interpreter != nullptr) {
    TfLiteInterpreterDelete(_interpreter);
    _interpreter = nullptr;
  }
  // _modelData (shared_ptr<ArrayBuffer>) is automatically freed
}

TensorflowModelDelegate HybridTfliteModel::getDelegate() {
  return _delegate;
}

std::vector<Tensor> HybridTfliteModel::getInputs() {
  int count = TfLiteInterpreterGetInputTensorCount(_interpreter);
  std::vector<Tensor> tensors;
  tensors.reserve(count);
  for (size_t i = 0; i < count; i++) {
    TfLiteTensor* tensor = TfLiteInterpreterGetInputTensor(_interpreter, i);
    if (tensor == nullptr) {
      throw std::runtime_error("TFLite: Failed to get input tensor " + std::to_string(i) + "!");
    }
    int dimensions = TfLiteTensorNumDims(tensor);
    std::vector<double> shape;
    shape.reserve(dimensions);
    for (size_t d = 0; d < dimensions; d++) {
      shape.push_back(static_cast<double>(TfLiteTensorDim(tensor, d)));
    }
    tensors.push_back(Tensor(std::string(TfLiteTensorName(tensor)),
                             dataTypeToString(TfLiteTensorType(tensor)), std::move(shape)));
  }
  return tensors;
}

std::vector<Tensor> HybridTfliteModel::getOutputs() {
  int count = TfLiteInterpreterGetOutputTensorCount(_interpreter);
  std::vector<Tensor> tensors;
  tensors.reserve(count);
  for (size_t i = 0; i < count; i++) {
    const TfLiteTensor* tensor = TfLiteInterpreterGetOutputTensor(_interpreter, i);
    if (tensor == nullptr) {
      throw std::runtime_error("TFLite: Failed to get output tensor " + std::to_string(i) + "!");
    }
    int dimensions = TfLiteTensorNumDims(tensor);
    std::vector<double> shape;
    shape.reserve(dimensions);
    for (size_t d = 0; d < dimensions; d++) {
      shape.push_back(static_cast<double>(TfLiteTensorDim(tensor, d)));
    }
    tensors.push_back(Tensor(std::string(TfLiteTensorName(tensor)),
                             dataTypeToString(TfLiteTensorType(tensor)), std::move(shape)));
  }
  return tensors;
}

void HybridTfliteModel::copyInputBuffers(const std::vector<std::shared_ptr<ArrayBuffer>>& input) {
  size_t inputCount = TfLiteInterpreterGetInputTensorCount(_interpreter);
  if (input.size() != inputCount) {
    throw std::runtime_error("TFLite: Input array size (" + std::to_string(input.size()) +
                             ") does not match input tensor count (" + std::to_string(inputCount) +
                             ")!");
  }

  for (size_t i = 0; i < inputCount; i++) {
    TfLiteTensor* tensor = TfLiteInterpreterGetInputTensor(_interpreter, i);
    auto& buffer = input[i];
    TfLiteTensorCopyFromBuffer(tensor, buffer->data(), buffer->size());
  }
}

// Pre-allocates a buffer per output tensor, reuses on subsequent calls.
std::shared_ptr<ArrayBuffer>
HybridTfliteModel::getOutputBufferForTensor(const TfLiteTensor* tensor) {
  auto name = std::string(TfLiteTensorName(tensor));
  if (_outputBuffers.find(name) == _outputBuffers.end()) {
    auto dataType = TfLiteTensorType(tensor);
    int totalLength = getTensorTotalLength(tensor);
    size_t byteSize = totalLength * getTFLTensorDataTypeSize(dataType);
    _outputBuffers[name] = ArrayBuffer::allocate(byteSize);
  }
  return _outputBuffers[name];
}

std::vector<std::shared_ptr<ArrayBuffer>> HybridTfliteModel::copyOutputBuffers() {
  int outputCount = TfLiteInterpreterGetOutputTensorCount(_interpreter);
  std::vector<std::shared_ptr<ArrayBuffer>> results;
  results.reserve(outputCount);

  for (size_t i = 0; i < outputCount; i++) {
    const TfLiteTensor* tensor = TfLiteInterpreterGetOutputTensor(_interpreter, i);
    auto outputBuffer = getOutputBufferForTensor(tensor);

    auto name = std::string(TfLiteTensorName(tensor));
    auto dataType = TfLiteTensorType(tensor);

    void* tensorData = TfLiteTensorData(tensor);
    if (tensorData == nullptr) {
      [[unlikely]];
      throw std::runtime_error("TFLite: Failed to get data from tensor \"" + name + "\"!");
    }

    // Recalculate size from tensor each time (not from cached buffer).
    int size = getTensorTotalLength(tensor) * getTFLTensorDataTypeSize(dataType);
    memcpy(outputBuffer->data(), tensorData, size);
    results.push_back(outputBuffer);
  }

  return results;
}

void HybridTfliteModel::invoke() {
  TfLiteStatus status = TfLiteInterpreterInvoke(_interpreter);
  if (status != kTfLiteOk) {
    throw std::runtime_error("TFLite: Failed to run TFLite Model! Status: " +
                             tfLiteStatusToString(status));
  }
}

std::vector<std::shared_ptr<ArrayBuffer>>
HybridTfliteModel::runSync(const std::vector<std::shared_ptr<ArrayBuffer>>& input) {
  copyInputBuffers(input);
  invoke();
  return copyOutputBuffers();
}

std::shared_ptr<Promise<std::vector<std::shared_ptr<ArrayBuffer>>>>
HybridTfliteModel::run(const std::vector<std::shared_ptr<ArrayBuffer>>& input) {
  // Copy input buffers on caller (JS) thread first — input ArrayBuffers are
  // non-owning JS buffers that may be GC'd if we access them async.
  copyInputBuffers(input);
  return Promise<std::vector<std::shared_ptr<ArrayBuffer>>>::async(
      [this]() -> std::vector<std::shared_ptr<ArrayBuffer>> {
        invoke();
        return copyOutputBuffers();
      });
}

} // namespace margelo::nitro::nitrotflite
