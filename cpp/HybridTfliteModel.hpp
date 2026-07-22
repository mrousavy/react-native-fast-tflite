#pragma once

#include "HybridTfliteModelSpec.hpp"
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
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

  /**
   * Deterministically free the interpreter, delegates, and model buffer NOW,
   * without waiting for every runtime's GC to drop its reference (worklet
   * runtimes may not GC for a long time, especially while backgrounded).
   * Thread-safe: blocks until an in-flight inference on another thread
   * completes. All subsequent runSync/run/getInputs/getOutputs calls throw a
   * catchable JS error. Idempotent. Called by Nitro when JS invokes
   * `model.dispose()`.
   */
  void dispose() override;

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

  // Shared by the destructor and dispose(); frees in the required order
  // (interpreter -> delegates -> model buffer) and nulls the pointers so a
  // second invocation is a no-op. Caller must hold _lifecycleMutex (except
  // the destructor, which is only reachable single-threaded).
  void releaseNativeResources();

  // Caller must hold _lifecycleMutex. std::runtime_error is surfaced by
  // Nitro as a catchable JS error on any runtime.
  void throwIfDisposed() {
    if (_disposed.load(std::memory_order_acquire)) {
      throw std::runtime_error("TFLite: Model was disposed!");
    }
  }

private:
  TfLiteInterpreter* _interpreter = nullptr;
  std::vector<TensorflowModelDelegate> _delegates;
  std::shared_ptr<ArrayBuffer> _modelData;
  // Free the TFLite delegates (GPU/CoreML/NNAPI) — the interpreter does not
  // own them, so without these every model destruction leaks the delegate's
  // compiled kernels / driver contexts.
  std::vector<std::function<void()>> _delegateDeleters;
  std::unordered_map<std::string, std::shared_ptr<ArrayBuffer>> _outputBuffers;

  // Serializes inference against dispose(). Uncontended in normal operation
  // (one lock per inference, ~ns vs ~ms inference cost).
  std::mutex _lifecycleMutex;
  std::atomic<bool> _disposed{false};
};

} // namespace margelo::nitro::tflite
