//
//  TensorflowPlugin.h
//  VisionCamera
//
//  Created by Marc Rousavy on 26.06.23.
//  Copyright © 2023 mrousavy. All rights reserved.
//

#pragma once

#include "jsi/TypedArray.h"
#include <jsi/jsi.h>
#include <memory>
#include <string>
#include <unordered_map>

#ifdef ANDROID
#include <ReactCommon/CallInvoker.h>
#include <tensorflow/lite/c/c_api.h>
#else
#include <React-callinvoker/ReactCommon/CallInvoker.h>
#include <TensorFlowLiteC/TensorFlowLiteC.h>
#endif

using namespace facebook;
using namespace mrousavy;

struct Buffer {
  void* data;
  size_t size;
};
typedef std::function<Buffer(std::string)> FetchURLFunc;

class TensorflowPlugin : public jsi::HostObject {
public:
  // TFL Delegate Type
  enum Delegate { Default, Metal, CoreML, NnApi, AndroidGPU };

public:
  explicit TensorflowPlugin(TfLiteInterpreter* interpreter, Buffer model, Delegate delegate,
                            std::shared_ptr<react::CallInvoker> callInvoker);
  ~TensorflowPlugin();

  jsi::Value get(jsi::Runtime& runtime, const jsi::PropNameID& name) override;
  std::vector<jsi::PropNameID> getPropertyNames(jsi::Runtime& runtime) override;

  static void installToRuntime(jsi::Runtime& runtime,
                               std::shared_ptr<react::CallInvoker> callInvoker,
                               FetchURLFunc fetchURL);

private:
  void copyInputBuffers(jsi::Runtime& runtime, jsi::Object inputValues);
  void run();
  jsi::Value copyOutputBuffers(jsi::Runtime& runtime);

  std::shared_ptr<TypedArrayBase> getOutputArrayForTensor(jsi::Runtime& runtime,
                                                          const TfLiteTensor* tensor);

private:
  TfLiteInterpreter* _interpreter = nullptr;
  Delegate _delegate = Delegate::Default;
  Buffer _model;
  std::shared_ptr<react::CallInvoker> _callInvoker;

  std::unordered_map<std::string, std::shared_ptr<TypedArrayBase>> _outputBuffers;
};
