#include <jni.h>
#include <jsi/jsi.h>
#include <fbjni/fbjni.h>
#include <exception>
#include <memory>

// TODO: Uncomment this when tensorflow-lite C/C++ API can be successfully built/linked here
// #include "TensorflowPlugin.h"
#include <ReactCommon/CallInvoker.h>
#include <ReactCommon/CallInvokerHolder.h>

namespace mrousavy {

using namespace facebook;
using namespace facebook::jni;

// Java Insaller
struct TfliteModule : public jni::JavaClass<TfliteModule> {
 public:
  static constexpr auto kJavaDescriptor = "Lcom/tflite/TfliteModule;";

  static jboolean nativeInstall(jni::alias_ref<jni::JClass>,
                                jlong runtimePtr,
                                jni::alias_ref <react::CallInvokerHolder::javaobject> jsCallInvokerHolder) {
    auto runtime = reinterpret_cast<jsi::Runtime *>(runtimePtr);
    if (runtime == nullptr) {
      // Runtime was null!
      return false;
    }
    auto jsCallInvoker = jsCallInvokerHolder->cthis()->getCallInvoker();

      // TODO: Uncomment this when tensorflow-lite C/C++ API can be successfully built/linked here
    /*auto fetchByteDataFromUrl = [](std::string url) {
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
     */

    try {
        // TODO: Uncomment this when tensorflow-lite C/C++ API can be successfully built/linked here
        // TensorflowPlugin::installToRuntime(*runtime, jsCallInvoker, fetchByteDataFromUrl);

        // TODO: Remove this when tensorflow-lite C/C++ API can be successfully built/linked here
        auto func = jsi::Function::createFromHostFunction(*runtime,
                                                          jsi::PropNameID::forAscii(*runtime, "__loadTensorflowModel"),
                                                          1,
                                                          [=](jsi::Runtime& runtime,
                                                              const jsi::Value& thisValue,
                                                              const jsi::Value* arguments,
                                                              size_t count) -> jsi::Value {
            throw jsi::JSError(runtime, "react-native-fast-tflite is not yet supported on Android! I couldn't manage to get TFLite to build for NDK/C++ :/");
        });
        runtime->global().setProperty(*runtime, "__loadTensorflowModel", func);
    } catch (std::exception &exc) {
      return false;
    }

    return true;
  }

  static void registerNatives() {
    javaClassStatic()->registerNatives({
      makeNativeMethod("nativeInstall", TfliteModule::nativeInstall),
    });
  }
};

} // namespace mrousavy

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *) {
  return facebook::jni::initialize(vm, [] {
    mrousavy::TfliteModule::registerNatives();
  });
}

