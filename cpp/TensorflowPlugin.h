//
//  TensorflowPlugin.h
//  VisionCamera
//
//  Created by Marc Rousavy on 26.06.23.
//  Copyright © 2023 mrousavy. All rights reserved.
//

#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <jsi/jsi.h>
#include <React-callinvoker/ReactCommon/CallInvoker.h>
#include "jsi/TypedArray.h"
#include <TensorFlowLiteC/TensorFlowLiteC.h>

using namespace facebook;
using namespace mrousavy;

struct Buffer {
  void* data;
  size_t size;
};
typedef std::function<Buffer(std::string)> FetchURLFunc;

class TensorflowPlugin: public jsi::HostObject {
public:
  // TFL Delegate Type
  enum Delegate { Default, Metal, CoreML };

public:
  explicit TensorflowPlugin(TfLiteInterpreter* interpreter,
                            Buffer model,
                            Delegate delegate);
  ~TensorflowPlugin();

  jsi::Value get(jsi::Runtime& runtime, const jsi::PropNameID& name) override;
  std::vector<jsi::PropNameID> getPropertyNames(jsi::Runtime& runtime) override;

  static void installToRuntime(jsi::Runtime& runtime,
                               std::shared_ptr<react::CallInvoker> callInvoker,
                               FetchURLFunc fetchURL);

private:
  jsi::Value run(jsi::Runtime& runtime, jsi::Object inputArray);
  std::shared_ptr<TypedArrayBase> getOutputArrayForTensor(jsi::Runtime& runtime, const TfLiteTensor* tensor);

private:
  TfLiteInterpreter* _interpreter = nullptr;
  Delegate _delegate = Delegate::Default;
  Buffer _model;

  std::unordered_map<std::string, std::shared_ptr<TypedArrayBase>> _outputBuffers;
};
