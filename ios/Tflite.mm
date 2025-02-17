#import "Tflite.h"
#import "../cpp/TensorflowPlugin.h"
#import <React-callinvoker/ReactCommon/CallInvoker.h>
#import <React/RCTBridge+Private.h>
#import <jsi/jsi.h>
#import <string>

// This is defined in RCTTurboModule.h.
// for future versions of React we might need to figure out another approach to get the
// JSCallInvoker!
@interface RCTBridge (RCTTurboModule)
- (std::shared_ptr<facebook::react::CallInvoker>)jsCallInvoker;
@end

using namespace facebook;

@implementation Tflite
RCT_EXPORT_MODULE(Tflite)

- (NSNumber *)install {
  RCTBridge* bridge = [RCTBridge currentBridge];
  RCTCxxBridge* cxxBridge = (RCTCxxBridge*)bridge;
  if (!cxxBridge.runtime) {
    return @(false);
  }
  jsi::Runtime& runtime = *(jsi::Runtime*)cxxBridge.runtime;

  auto fetchByteDataFromUrl = [](std::string url) {
    NSString* string = [NSString stringWithUTF8String:url.c_str()];
    NSLog(@"Fetching %@...", string);
    NSURL* nsURL = [NSURL URLWithString:string];
    NSData* contents = [NSData dataWithContentsOfURL:nsURL];

    void* data = malloc(contents.length * sizeof(uint8_t));
    memcpy(data, contents.bytes, contents.length);
    return Buffer{.data = data, .size = contents.length};
  };

  try {
    TensorflowPlugin::installToRuntime(runtime, [bridge jsCallInvoker], fetchByteDataFromUrl);
  } catch (std::exception& exc) {
    NSLog(@"Failed to install TensorFlow Lite plugin to Runtime! %s", exc.what());
    return @(false);
  }

  return @(true);
}

// Don't compile this code when we build for the old architecture.
#ifdef RCT_NEW_ARCH_ENABLED
- (std::shared_ptr<facebook::react::TurboModule>)getTurboModule:
    (const facebook::react::ObjCTurboModule::InitParams&)params {
  return std::make_shared<facebook::react::NativeRNTfliteSpecJSI>(params);
}
#endif

@end
