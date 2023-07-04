//
//  TensorflowPlugin.m
//  VisionCamera
//
//  Created by Marc Rousavy on 26.06.23.
//  Copyright © 2023 mrousavy. All rights reserved.
//

#include "TensorflowPlugin.h"

#include "JSITypedArray.h"
#include "TensorHelpers.h"
#include <TensorFlowLiteC/TensorFlowLiteC.h>
#include <ReactCommon/ReactCommon/TurboModuleUtils.h>
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
    
    auto promise = react::createPromiseAsJSIValue(runtime, [=](jsi::Runtime &runtime,
                                                               std::shared_ptr<react::Promise> promise) -> void {
      // Launch C++ async task
      auto future = std::async(std::launch::async, [=]() {
        // Fetch model from URL (JS bundle)
        Buffer buffer = fetchURL(modelPath);
        
        // Load Model into Tensorflow
        auto model = TfLiteModelCreate(buffer.data, buffer.size);
        if (model == nullptr) {
          promise->reject("Failed to load model from \"" + modelPath + "\"!");
          return;
        }
        
        // Create TensorFlow Interpreter
        auto options = TfLiteInterpreterOptionsCreate();
        auto interpreter = TfLiteInterpreterCreate(model, options);
        if (interpreter == nullptr) {
          promise->reject("Failed to create TFLite interpreter from model \"" + modelPath + "\"!");
          return;
        }
        
        // Initialize Model and allocate memory buffers
        auto plugin = std::make_shared<TensorflowPlugin>(interpreter, model, delegate);
        
        // Resolve Promise back on JS Thread
        callInvoker->invokeAsync([=]() {
          auto hostObject = jsi::Object::createFromHostObject(promise->runtime_, plugin);
          promise->resolve(std::move(hostObject));
          
          auto end = std::chrono::steady_clock::now();
          log("Successfully loaded Tensorflow Model in %i ms!",
              std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
        });
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

std::shared_ptr<TypedArrayBase> TensorflowPlugin::getOutputArrayForTensor(jsi::Runtime& runtime, TfLiteTensor* tensor) {
  auto name = std::string(TfLiteTensorName(tensor));
  if (_outputBuffers.find(name) == _outputBuffers.end()) {
    _outputBuffers[name] = std::make_shared<TypedArrayBase>(TensorHelpers::createJSBufferForTensor(runtime, tensor));
  }
  return _outputBuffers[name];
}

jsi::Value TensorflowPlugin::run(jsi::Runtime &runtime, jsi::Value inputValues) {
  // Input has to be array in input tensor size
  auto array = inputValues.asObject(runtime).asArray(runtime);
  size_t count = array.size(runtime);
  
  for (size_t i = 0; i < count; i++) {
    TfLiteTensor* tensor = TfLiteInterpreterGetInputTensor(_interpreter, i);
    
    auto value = array.getValueAtIndex(runtime, i);
    auto inputBuffer = getTypedArray(runtime, value.asObject(runtime));
    
    TensorHelpers::updateTensorFromJSBuffer(runtime, tensor, inputBuffer);
  }
  
  // Run Model
  TfLiteInterpreterInvoke(_interpreter);
  
  // Copy output to `NSData` to process the inference results.
  size_t outputTensorsCount = _interpreter.outputTensorCount;
  jsi::Array result(runtime, outputTensorsCount);
  for (size_t i = 0; i < outputTensorsCount; i++) {
    TFLTensor* outputTensor = [_interpreter outputTensorAtIndex:i error:&error];
    if (error != nil) {
      throw jsi::JSError(runtime, std::string("Failed to get output tensor! Error: ") + [error.description UTF8String]);
    }
    auto outputBuffer = getOutputArrayForTensor(runtime, outputTensor);
    TensorHelpers::updateJSBuffer(runtime, *outputBuffer, outputTensor);
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
      auto frame = arguments[0].asObject(runtime).asHostObject<FrameHostObject>(runtime);
      return this->run(runtime, frame->frame);
    });
  } else if (propName == "inputs") {
    jsi::Array tensors(runtime, _interpreter.inputTensorCount);
    for (size_t i = 0; i < _interpreter.inputTensorCount; i++) {
      NSError* error;
      TFLTensor* tensor = [_interpreter inputTensorAtIndex:i error:&error];
      if (error != nil) {
        throw jsi::JSError(runtime, "Failed to get input tensor " + std::to_string(i) + "! " + error.description.UTF8String);
      }
      
      jsi::Object object = TensorHelpers::tensorToJSObject(runtime, tensor);
      tensors.setValueAtIndex(runtime, i, object);
    }
    return tensors;
  } else if (propName == "outputs") {
    jsi::Array tensors(runtime, _interpreter.outputTensorCount);
    for (size_t i = 0; i < _interpreter.outputTensorCount; i++) {
      NSError* error;
      TFLTensor* tensor = [_interpreter outputTensorAtIndex:i error:&error];
      if (error != nil) {
        throw jsi::JSError(runtime, "Failed to get output tensor " + std::to_string(i) + "! " + error.description.UTF8String);
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