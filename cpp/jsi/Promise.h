#pragma once

#include <jsi/jsi.h>
#include <utility>
#include <vector>

#ifdef ANDROID
#include <ReactCommon/CallInvoker.h>
#else
#include <React-callinvoker/ReactCommon/CallInvoker.h>
#endif

namespace mrousavy {

using namespace facebook;

class Promise {
public:
  Promise(jsi::Runtime& runtime, jsi::Value resolver, jsi::Value rejecter);

  void resolve(jsi::Value&& result);
  void reject(std::string error);

public:
  jsi::Runtime& runtime;

private:
  jsi::Value _resolver;
  jsi::Value _rejecter;

public:
  /**
   Create a new Promise and runs the given `run` function.
   */
  static jsi::Value createPromise(jsi::Runtime& runtime,
                                  std::function<void(std::shared_ptr<Promise> promise)> run);
};

} // namespace mrousavy
