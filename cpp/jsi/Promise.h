#pragma once

#include <jsi/jsi.h>
#include <utility>
#include <vector>

namespace mrousavy {

using namespace facebook;

typedef std::function<void(jsi::Value)> Resolver;
typedef std::function<void(std::string)> Rejecter;

class Promise {
public:
  Promise(jsi::Runtime& runtime, Resolver resolve, Rejecter reject);
  
public:
  Resolver resolve;
  Rejecter reject;
  jsi::Runtime& runtime;
  
public:
  static jsi::Value createPromise(jsi::Runtime& runtime, std::function<void(jsi::Runtime& runtime, std::shared_ptr<Promise>)> run);
};

} // namespace mrousavy;
