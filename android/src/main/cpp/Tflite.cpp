#include <jni.h>
#include <jsi/jsi.h>
#include <fbjni/fbjni.h>

#include "TensorflowPlugin.h"
#include <ReactCommon/CallInvoker.h>
#include <ReactCommon/CallInvokerHolder.h>

using namespace facebook;

class TfliteModule: jni::JavaClass<TfliteModule> {
 public:
  static constexpr auto kJavaDescriptor = "Lcom/tflite/TfliteModule;";

  static bool nativeInstall(jlong runtimePtr, jni::alias_ref<facebook::react::CallInvokerHolder::javaobject> jsCallInvokerHolder) {
      auto runtime = reinterpret_cast<jsi::Runtime*>(runtimePtr);
      if (runtime == nullptr) {
          // Runtime was null!
          return false;
      }
      auto jsCallInvoker = jsCallInvokerHolder->cthis()->getCallInvoker();

      return true;
  }

  static void registerNatives() {
    javaClassStatic()->registerNatives({
       makeNativeMethod("nativeInstall", nativeInstall);
    });
  }
};

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *) {
  return facebook::jni::initialize(vm, [] {
    TfliteModule::registerNatives();
  });
}

