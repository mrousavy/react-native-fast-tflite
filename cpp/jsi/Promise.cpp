#include "Promise.h"
#include <jsi/jsi.h>
#include <utility>
#include <vector>

namespace mrousavy {

using namespace facebook;

Promise::Promise(jsi::Runtime& runtime, Resolver resolve, Rejecter reject): runtime(runtime), resolve(resolve), reject(reject) {
}

jsi::Value Promise::createPromise(jsi::Runtime& runtime, std::function<void(jsi::Runtime& runtime, std::shared_ptr<Promise>)> run) {
  auto promiseCtor = runtime.global().getPropertyAsFunction(runtime, "Promise");
  
  auto promiseCallback = jsi::Function::createFromHostFunction(runtime,
                                                               jsi::PropNameID::forUtf8(runtime, "PromiseCallback"),
                                                               2,
                                                               [=](jsi::Runtime& runtime,
                                                                   const jsi::Value& thisValue,
                                                                   const jsi::Value* arguments,
                                                                   size_t count) -> jsi::Value {
    //auto resolver = std::make_shared<jsi::Value>(std::move(arguments[0]));
    auto resolver = arguments[0].asObject(runtime).asFunction(runtime);
    auto rejecter = arguments[1].asObject(runtime).asFunction(runtime);
    Resolver resolve = [&runtime](jsi::Value value) {
      //resolver.call(runtime, value);
    };
    Rejecter reject = [&runtime](std::string message) {
      jsi::JSError error(runtime, message);
      //rejecter.call(runtime, error.value());
    };
    
    // Call function
    auto promise = std::make_shared<Promise>(runtime, resolve, reject);
    run(runtime, promise);
    
    return jsi::Value::undefined();
  });
  
  return promiseCtor.callAsConstructor(runtime, promiseCallback);
}

} // namespace mrousavy;
