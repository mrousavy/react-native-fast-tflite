#import "Tflite.h"
#import <React/RCTBridge+Private.h>
#import <React-callinvoker/ReactCommon/CallInvoker.h>
#import <React-NativeModulesApple/ReactCommon/RCTTurboModule.h>
#import <jsi/jsi.h>
#import "../cpp/TensorflowPlugin.h"

using namespace facebook;

@implementation Tflite
RCT_EXPORT_MODULE()

RCT_EXPORT_BLOCKING_SYNCHRONOUS_METHOD(install) {
  RCTBridge* bridge = [self bridge];
  RCTCxxBridge* cxxBridge = (RCTCxxBridge*)bridge;
  if (!cxxBridge.runtime) {
    return @(false);
  }
  jsi::Runtime& runtime = *(jsi::Runtime*)cxxBridge.runtime;
  
  try {
    TensorflowPlugin::installToRuntime(runtime, [bridge jsCallInvoker]);
  } catch (std::exception& exc) {
    NSLog(@"Failed to install TensorFlow Lite plugin to Runtime! %s", exc.what());
    return @(false);
  }
  
  return @(true);
}

// Don't compile this code when we build for the old architecture.
#ifdef RCT_NEW_ARCH_ENABLED
- (std::shared_ptr<facebook::react::TurboModule>)getTurboModule:
    (const facebook::react::ObjCTurboModule::InitParams &)params
{
    return std::make_shared<facebook::react::NativeTfliteSpecJSI>(params);
}
#endif

@end
