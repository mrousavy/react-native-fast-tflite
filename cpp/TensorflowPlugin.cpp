//
//  TensorflowPlugin.m
//  VisionCamera
//
//  Created by Marc Rousavy on 26.06.23.
//  Copyright © 2023 mrousavy. All rights reserved.
//

#include "TensorflowPlugin.h"

#include "jsi/TypedArray.h"
#include "jsi/Promise.h"
#include "TensorHelpers.h"
#include <TensorFlowLiteC/TensorFlowLiteC.h>
#include <chrono>
#include <thread>
#include <string>
#include <iostream>
#include <future>

using namespace facebook;
using namespace mrousavy;

void log(std::string string...) {
  // TODO: Figure out how to log to console
}

void TensorflowPlugin::installToRuntime(jsi::Runtime& runtime,
                                        std::shared_ptr<react::CallInvoker> callInvoker,
                                        FetchURLFunc fetchURL) {
  
  
  auto func = jsi::Function::createFromHostFunction(runtime,
                                                    jsi::PropNameID::forAscii(runtime, "__loadTensorflowModel"),
                                                    1,
                                                    [=](jsi::Runtime& runtime,
                                                        const jsi::Value& thisValue,
                                                        const jsi::Value* arguments,
                                                        size_t count) -> jsi::Value {
    auto start = std::chrono::steady_clock::now();
    auto modelPath = arguments[0].asString(runtime).utf8(runtime);
    
    log("Loading TensorFlow Lite Model from \"%s\"...", modelPath.c_str());
    
    // TODO: Figure out how to use Metal/CoreML delegates
    Delegate delegate = Delegate::Default;
    /*
    auto delegates = [[NSMutableArray alloc] init];
    if (count > 1 && arguments[1].isString()) {
      // user passed a custom delegate command
      auto delegate = arguments[1].asString(runtime).utf8(runtime);
      if (delegate == "core-ml") {
        NSLog(@"Using CoreML delegate.");
        [delegates addObject:[[TFLCoreMLDelegate alloc] init]];
        delegate = Delegate::CoreML;
      } else if (delegate == "metal") {
        NSLog(@"Using Metal delegate.");
        [delegates addObject:[[TFLMetalDelegate alloc] init]];
        delegate = Delegate::Metal;
      } else {
        NSLog(@"Using standard CPU delegate.");
        delegate = Delegate::Default;
      }
    }
     */
    
    auto promise = Promise::createPromise(runtime,
                                          [=, &runtime](std::shared_ptr<Promise> promise) {
      // Launch async thread
      std::async(std::launch::async, [=, &runtime]() {
        // Fetch model from URL (JS bundle)
        Buffer buffer = fetchURL(modelPath);
        
        // Load Model into Tensorflow
        auto model = TfLiteModelCreate(buffer.data, buffer.size);
        if (model == nullptr) {
          callInvoker->invokeAsync([=]() {
            promise->reject("Failed to load model from \"" + modelPath + "\"!");
          });
          return;
        }
        
        // Create TensorFlow Interpreter
        auto options = TfLiteInterpreterOptionsCreate();
        auto interpreter = TfLiteInterpreterCreate(model, options);
        if (interpreter == nullptr) {
          callInvoker->invokeAsync([=]() {
            promise->reject("Failed to create TFLite interpreter from model \"" + modelPath + "\"!");
          });
          return;
        }
        
        // Initialize Model and allocate memory buffers
        auto plugin = std::make_shared<TensorflowPlugin>(interpreter, buffer, delegate);
        
        callInvoker->invokeAsync([=, &runtime]() {
          auto result = jsi::Object::createFromHostObject(runtime, plugin);
          promise->resolve(std::move(result));
        });
        
        auto end = std::chrono::steady_clock::now();
        log("Successfully loaded Tensorflow Model in %i ms!",
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
      });
    });
    return promise;
  });
  
  runtime.global().setProperty(runtime, "__loadTensorflowModel", func);
}


TensorflowPlugin::TensorflowPlugin(TfLiteInterpreter* interpreter,
                                   Buffer model,
                                   Delegate delegate): _interpreter(interpreter), _model(model), _delegate(delegate) {
  // Allocate memory for the model's input/output `TFLTensor`s.
  TfLiteStatus status = TfLiteInterpreterAllocateTensors(_interpreter);
  if (status != kTfLiteOk) {
    throw std::runtime_error("Failed to allocate memory for input/output tensors! Status: " + std::to_string(status));
  }
  
  log("Successfully created Tensorflow Plugin!");
}

TensorflowPlugin::~TensorflowPlugin() {
  if (_model.data != nullptr) {
    free(_model.data);
    _model.data = nullptr;
    _model.size = 0;
  }
  if (_interpreter != nullptr) {
    TfLiteInterpreterDelete(_interpreter);
    _interpreter = nullptr;
  }
}

std::shared_ptr<TypedArrayBase> TensorflowPlugin::getOutputArrayForTensor(jsi::Runtime& runtime, const TfLiteTensor* tensor) {
  auto name = std::string(TfLiteTensorName(tensor));
  if (_outputBuffers.find(name) == _outputBuffers.end()) {
    _outputBuffers[name] = std::make_shared<TypedArrayBase>(TensorHelpers::createJSBufferForTensor(runtime, tensor));
  }
  return _outputBuffers[name];
}

jsi::Value TensorflowPlugin::run(jsi::Runtime &runtime, jsi::Object inputValues) {
  // Input has to be array in input tensor size
  auto array = inputValues.asArray(runtime);
  size_t count = array.size(runtime);
  if (count != TfLiteInterpreterGetInputTensorCount(_interpreter)) {
    throw std::runtime_error("TFLite: Input Values have different size than there are input tensors!");
  }
  
  for (size_t i = 0; i < count; i++) {
    TfLiteTensor* tensor = TfLiteInterpreterGetInputTensor(_interpreter, i);
    
    auto value = array.getValueAtIndex(runtime, i);
    auto inputBuffer = getTypedArray(runtime, value.asObject(runtime));
    
    TensorHelpers::updateTensorFromJSBuffer(runtime, tensor, inputBuffer);
  }
  
  // Run Model
  TfLiteStatus status = TfLiteInterpreterInvoke(_interpreter);
  if (status != kTfLiteOk) {
    throw std::runtime_error("Failed to run TFLite Model! Status: " + std::to_string(status));
  }
  
  // Copy output to result process the inference results.
  int outputTensorsCount = TfLiteInterpreterGetOutputTensorCount(_interpreter);
  jsi::Array result(runtime, outputTensorsCount);
  for (size_t i = 0; i < outputTensorsCount; i++) {
    const TfLiteTensor* outputTensor = TfLiteInterpreterGetOutputTensor(_interpreter, i);
    auto outputBuffer = getOutputArrayForTensor(runtime, outputTensor);
    TensorHelpers::updateJSBufferFromTensor(runtime, *outputBuffer, outputTensor);
    result.setValueAtIndex(runtime, i, *outputBuffer);
  }
  return result;
}


jsi::Value TensorflowPlugin::get(jsi::Runtime& runtime, const jsi::PropNameID& propNameId) {
  auto propName = propNameId.utf8(runtime);
  
  if (propName == "run") {
    return jsi::Function::createFromHostFunction(runtime,
                                                 jsi::PropNameID::forAscii(runtime, "runModel"),
                                                 1,
                                                 [=](jsi::Runtime& runtime,
                                                     const jsi::Value& thisValue,
                                                     const jsi::Value* arguments,
                                                     size_t count) -> jsi::Value {
      return this->run(runtime, arguments[0].asObject(runtime));
    });
  } else if (propName == "inputs") {
    int size = TfLiteInterpreterGetInputTensorCount(_interpreter);
    jsi::Array tensors(runtime, size);
    for (size_t i = 0; i < size; i++) {
      TfLiteTensor* tensor = TfLiteInterpreterGetInputTensor(_interpreter, i);
      if (tensor == nullptr) {
        throw jsi::JSError(runtime, "Failed to get input tensor " + std::to_string(i) + "!");
      }
      
      jsi::Object object = TensorHelpers::tensorToJSObject(runtime, tensor);
      tensors.setValueAtIndex(runtime, i, object);
    }
    return tensors;
  } else if (propName == "outputs") {
    int size = TfLiteInterpreterGetOutputTensorCount(_interpreter);
    jsi::Array tensors(runtime, size);
    for (size_t i = 0; i < size; i++) {
      const TfLiteTensor* tensor = TfLiteInterpreterGetOutputTensor(_interpreter, i);
      if (tensor == nullptr) {
        throw jsi::JSError(runtime, "Failed to get output tensor " + std::to_string(i) + "!");
      }
      
      jsi::Object object = TensorHelpers::tensorToJSObject(runtime, tensor);
      tensors.setValueAtIndex(runtime, i, object);
    }
    return tensors;
  } else if (propName == "delegate") {
    switch (_delegate) {
      case Delegate::Default:
        return jsi::String::createFromUtf8(runtime, "default");
      case Delegate::CoreML:
        return jsi::String::createFromUtf8(runtime, "coreml");
      case Delegate::Metal:
        return jsi::String::createFromUtf8(runtime, "metal");
    }
  }
  
  return jsi::HostObject::get(runtime, propNameId);
}


std::vector<jsi::PropNameID> TensorflowPlugin::getPropertyNames(jsi::Runtime& runtime) {
  std::vector<jsi::PropNameID> result;
  result.push_back(jsi::PropNameID::forAscii(runtime, "run"));
  result.push_back(jsi::PropNameID::forAscii(runtime, "inputs"));
  result.push_back(jsi::PropNameID::forAscii(runtime, "outputs"));
  result.push_back(jsi::PropNameID::forAscii(runtime, "delegate"));
  return result;
}
