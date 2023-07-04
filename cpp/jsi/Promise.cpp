#include "Promise.h"
#include <jsi/jsi.h>
#include <utility>
#include <vector>
#include <future>

namespace mrousavy {

using namespace facebook;

Promise::Promise(jsi::Runtime& runtime,
                 std::shared_ptr<facebook::react::CallInvoker> callInvoker,
                 jsi::Value resolver,
                 jsi::Value rejecter):
  runtime(runtime), _callInvoker(callInvoker), _resolver(std::move(resolver)), _rejecter(std::move(rejecter)) {
}

jsi::Value Promise::createPromise(jsi::Runtime& runtime,
                                  std::shared_ptr<facebook::react::CallInvoker> callInvoker,
                                  std::function<jsi::Value(jsi::Runtime& runtime)> run) {
  // Get Promise ctor from global
  auto promiseCtor = runtime.global().getPropertyAsFunction(runtime, "Promise");
  
  auto promiseCallback = jsi::Function::createFromHostFunction(runtime,
                                                               jsi::PropNameID::forUtf8(runtime, "PromiseCallback"),
                                                               2,
                                                               [=](jsi::Runtime& runtime,
                                                                   const jsi::Value& thisValue,
                                                                   const jsi::Value* arguments,
                                                                   size_t count) -> jsi::Value {
    // Call function
    auto promise = std::make_shared<Promise>(runtime,
                                             callInvoker,
                                             arguments[0].asObject(runtime),
                                             arguments[1].asObject(runtime));
    
    auto callback = std::async(std::launch::async, [run, promise, &runtime]() {
      try {
        auto result = run(runtime);
        promise->resolve(std::move(result));
      } catch (std::runtime_error error) {
        promise->reject(error.what());
      } catch (...) {
        promise->reject("Promise rejected, unknown error!");
      }
    });
    
    
    return jsi::Value::undefined();
  });
  
  return promiseCtor.callAsConstructor(runtime, promiseCallback);
}

void Promise::resolve(jsi::Value&& result) {
  // TODO: Use std::move into Lambda to avoid shared_ptr here
  std::shared_ptr<jsi::Value> shared = std::make_shared<jsi::Value>(std::move(result));
  _callInvoker->invokeAsync([this, shared]() {
    this->_resolver.asObject(runtime).asFunction(runtime).call(runtime, *shared);
  });
}

void Promise::reject(std::string message) {
  _callInvoker->invokeAsync([this, message]() {
    jsi::JSError error(runtime, message);
    this->_rejecter.asObject(runtime).asFunction(runtime).call(runtime, error.value());
  });
}

} // namespace mrousavy;
