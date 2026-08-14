#include "HybridTfliteModel.hpp"
#include "TfliteHelpers.hpp"

#include <NitroModules/ArrayBuffer.hpp>
#include <NitroModules/Promise.hpp>
#include <string>

#if defined(ANDROID)
#include <tflite/c/c_api.h>
#elif defined(__APPLE__)
#include <TensorFlowLiteC/TensorFlowLiteC.h>
#else
#error "Invalid Platform!"
#endif

namespace margelo::nitro::tflite {

HybridTfliteModel::HybridTfliteModel(TfLiteInterpreter* interpreter,
                                     std::shared_ptr<ArrayBuffer> modelData,
                                     std::vector<TensorflowModelDelegate> delegates,
                                     std::vector<std::function<void()>> delegateDeleters)
    : HybridObject(TAG), _interpreter(interpreter), _delegates(std::move(delegates)),
      _modelData(modelData), _delegateDeleters(std::move(delegateDeleters)) {
  TfLiteStatus status = TfLiteInterpreterAllocateTensors(_interpreter);
  if (status != kTfLiteOk) {
    releaseNativeResources();
    throw std::runtime_error(
        "TFLite: Failed to allocate memory for input/output tensors! Status: " +
        tfLiteStatusToString(status));
  }
}

HybridTfliteModel::~HybridTfliteModel() {
  // No lock: the destructor only runs once the last shared_ptr dropped, so no
  // other thread can be inside a method. If dispose() already ran, all
  // pointers are null and this is a no-op.
  releaseNativeResources();
}

void HybridTfliteModel::releaseNativeResources() {
  if (_interpreter != nullptr) {
    TfLiteInterpreterDelete(_interpreter);
    _interpreter = nullptr;
  }
  // Delegates must outlive the interpreter — delete them second.
  for (auto& deleter : _delegateDeleters) {
    deleter();
  }
  _delegateDeleters.clear();
  // Model bytes must outlive the interpreter — drop our reference last.
  // (JS-side references to output buffers keep their own shared_ptrs alive.)
  _modelData = nullptr;
  _outputBuffers.clear();
}

void HybridTfliteModel::dispose() {
  // The lock waits out an in-flight inference on another thread — freeing
  // the interpreter under a running TfLiteInterpreterInvoke would be a
  // native crash.
  std::lock_guard<std::mutex> lock(_lifecycleMutex);
  if (_disposed.exchange(true, std::memory_order_acq_rel)) {
    return;
  }
  releaseNativeResources();
}

std::vector<TensorflowModelDelegate> HybridTfliteModel::getDelegates() {
  return _delegates;
}

std::vector<Tensor> HybridTfliteModel::getInputs() {
  std::lock_guard<std::mutex> lock(_lifecycleMutex);
  throwIfDisposed();
  int count = TfLiteInterpreterGetInputTensorCount(_interpreter);
  std::vector<Tensor> tensors;
  tensors.reserve(count);
  for (int32_t i = 0; i < count; i++) {
    TfLiteTensor* tensor = TfLiteInterpreterGetInputTensor(_interpreter, i);
    if (tensor == nullptr) {
      throw std::runtime_error("TFLite: Failed to get input tensor " + std::to_string(i) + "!");
    }
    int dimensions = TfLiteTensorNumDims(tensor);
    std::vector<double> shape;
    shape.reserve(dimensions);
    for (int32_t d = 0; d < dimensions; d++) {
      int32_t size = TfLiteTensorDim(tensor, d);
      shape.push_back(static_cast<double>(size));
    }
    tensors.push_back(Tensor(std::string(TfLiteTensorName(tensor)),
                             getTensorDataType(TfLiteTensorType(tensor)), std::move(shape)));
  }
  return tensors;
}

std::vector<Tensor> HybridTfliteModel::getOutputs() {
  std::lock_guard<std::mutex> lock(_lifecycleMutex);
  throwIfDisposed();
  int count = TfLiteInterpreterGetOutputTensorCount(_interpreter);
  std::vector<Tensor> tensors;
  tensors.reserve(count);
  for (int32_t i = 0; i < count; i++) {
    const TfLiteTensor* tensor = TfLiteInterpreterGetOutputTensor(_interpreter, i);
    if (tensor == nullptr) {
      throw std::runtime_error("TFLite: Failed to get output tensor " + std::to_string(i) + "!");
    }
    int dimensions = TfLiteTensorNumDims(tensor);
    std::vector<double> shape;
    shape.reserve(dimensions);
    for (int32_t d = 0; d < dimensions; d++) {
      int32_t size = TfLiteTensorDim(tensor, d);
      shape.push_back(static_cast<double>(size));
    }
    tensors.push_back(Tensor(std::string(TfLiteTensorName(tensor)),
                             getTensorDataType(TfLiteTensorType(tensor)), std::move(shape)));
  }
  return tensors;
}

void HybridTfliteModel::copyInputBuffers(const std::vector<std::shared_ptr<ArrayBuffer>>& input) {
  size_t inputCount = TfLiteInterpreterGetInputTensorCount(_interpreter);
  if (input.size() != inputCount) [[unlikely]] {
    throw std::runtime_error("TFLite: Input array size (" + std::to_string(input.size()) +
                             ") does not match input tensor count (" + std::to_string(inputCount) +
                             ")!");
  }

  for (int32_t i = 0; i < inputCount; i++) {
    TfLiteTensor* tensor = TfLiteInterpreterGetInputTensor(_interpreter, i);
    const std::shared_ptr<ArrayBuffer>& buffer = input[i];
    TfLiteStatus status = TfLiteTensorCopyFromBuffer(tensor, buffer->data(), buffer->size());
    if (status != kTfLiteOk) [[unlikely]] {
      // TfLiteTensorCopyFromBuffer requires input_data_size == TfLiteTensorByteSize(tensor).
      // On mismatch it leaves the tensor untouched, so without this check inference would
      // silently run on the previous (or zero-initialized) contents of the tensor.
      throw std::runtime_error("TFLite: Input buffer " + std::to_string(i) + " size (" +
                               std::to_string(buffer->size()) + ") does not match input tensor \"" +
                               std::string(TfLiteTensorName(tensor)) + "\" expected size (" +
                               std::to_string(TfLiteTensorByteSize(tensor)) +
                               ")! Status: " + tfLiteStatusToString(status));
    }
  }
}

// Pre-allocates a buffer per output tensor, reuses on subsequent calls.
std::shared_ptr<ArrayBuffer>
HybridTfliteModel::getOutputBufferForTensor(const TfLiteTensor* tensor) {
  std::string name = TfLiteTensorName(tensor);
  if (_outputBuffers.find(name) == _outputBuffers.end()) {
    TfLiteType dataType = TfLiteTensorType(tensor);
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

  for (int32_t i = 0; i < outputCount; i++) {
    const TfLiteTensor* tensor = TfLiteInterpreterGetOutputTensor(_interpreter, i);
    std::shared_ptr<ArrayBuffer> outputBuffer = getOutputBufferForTensor(tensor);

    std::string name = TfLiteTensorName(tensor);

    void* tensorData = TfLiteTensorData(tensor);
    if (tensorData == nullptr) {
      [[unlikely]];
      throw std::runtime_error("TFLite: Failed to get data from tensor \"" + name + "\"!");
    }

    memcpy(outputBuffer->data(), tensorData, outputBuffer->size());
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
  // Held for the whole inference so dispose() can never free the interpreter
  // mid-invoke. The disposed-throw is a catchable JS error on any runtime.
  std::lock_guard<std::mutex> lock(_lifecycleMutex);
  throwIfDisposed();
  copyInputBuffers(input);
  invoke();
  return copyOutputBuffers();
}

std::shared_ptr<Promise<std::vector<std::shared_ptr<ArrayBuffer>>>>
HybridTfliteModel::run(const std::vector<std::shared_ptr<ArrayBuffer>>& input) {
  {
    // Copy input buffers on caller (JS) thread first — input ArrayBuffers are
    // non-owning JS buffers that may be GC'd if we access them async.
    std::lock_guard<std::mutex> lock(_lifecycleMutex);
    throwIfDisposed();
    copyInputBuffers(input);
  }
  std::shared_ptr<HybridTfliteModel> sharedThis = shared_cast<HybridTfliteModel>();
  return Promise<std::vector<std::shared_ptr<ArrayBuffer>>>::async(
      [sharedThis]() -> std::vector<std::shared_ptr<ArrayBuffer>> {
        // Re-acquire on the async thread: dispose() may have landed between
        // the input copy above and this lambda running.
        std::lock_guard<std::mutex> lock(sharedThis->_lifecycleMutex);
        sharedThis->throwIfDisposed();
        sharedThis->invoke();
        return sharedThis->copyOutputBuffers();
      });
}

} // namespace margelo::nitro::tflite
