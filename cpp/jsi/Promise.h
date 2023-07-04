#pragma once

#include <jsi/jsi.h>
#include <utility>
#include <vector>
#import <React-callinvoker/ReactCommon/CallInvoker.h>

namespace mrousavy {

using namespace facebook;

class Promise {
public:
  Promise(jsi::Runtime& runtime,
          std::shared_ptr<facebook::react::CallInvoker> callInvoker,
          jsi::Value resolver,
          jsi::Value rejecter);
  
  void resolve(jsi::Value&& result);
  void reject(std::string error);
  
public:
  jsi::Runtime& runtime;
private:
  jsi::Value _resolver;
  jsi::Value _rejecter;
  std::shared_ptr<facebook::react::CallInvoker> _callInvoker;
  
public:
  /**
   Create a new Promise and runs the given `run` function in a Thread pool.
   */
  static jsi::Value createPromise(jsi::Runtime& runtime,
                                  std::shared_ptr<facebook::react::CallInvoker> callInvoker,
                                  std::function<jsi::Value(jsi::Runtime& runtime)> run);
};

} // namespace mrousavy;
