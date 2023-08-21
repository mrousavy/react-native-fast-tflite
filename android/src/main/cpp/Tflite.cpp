#include <jni.h>
#include <jsi/jsi.h>
#include <fbjni/fbjni.h>
#include <exception>
#include <memory>

#include "TensorflowPlugin.h"
#include <ReactCommon/CallInvoker.h>
#include <ReactCommon/CallInvokerHolder.h>

namespace mrousavy {

using namespace facebook;
using namespace facebook::jni;

// Java Insaller
struct TfliteModule : public jni::JavaClass<TfliteModule> {
 public:
  static constexpr auto kJavaDescriptor = "Lcom/tflite/TfliteModule;";

  static jboolean nativeInstall(jlong runtimePtr,
                                jni::alias_ref <react::CallInvokerHolder::javaobject> jsCallInvokerHolder) {
    auto runtime = reinterpret_cast<jsi::Runtime *>(runtimePtr);
    if (runtime == nullptr) {
      // Runtime was null!
      return false;
    }
    auto jsCallInvoker = jsCallInvokerHolder->cthis()->getCallInvoker();

    auto fetchByteDataFromUrl = [](std::string url) {
      static const auto cls = javaClassStatic();
      static const auto method = cls->getStaticMethod<jbyteArray(std::string)>("fetchByteDataFromUrl");

      auto byteData = method(cls, url);
      auto size = byteData->size();
      auto bytes = byteData->getRegion(0, size);
      void* data = malloc(size);
      memcpy(data, bytes.get(), size);
      return Buffer {
          .data = data,
          .size = size
      };
    };

    try {
      TensorflowPlugin::installToRuntime(runtime, jsCallInvoker, fetchByteDataFromUrl);
    } catch (std::exception &exc) {
      return false;
    }

    return true;
  }

  static void registerNatives() {
    javaClassStatic()->registerNatives({
      makeNativeMethod("nativeInstall",
        TfliteModule::nativeInstall),
      });
  }
};

} // namespace mrousavy

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *) {
  return facebook::jni::initialize(vm, [] {
    mrousavy::TfliteModule::registerNatives();
  });
}

