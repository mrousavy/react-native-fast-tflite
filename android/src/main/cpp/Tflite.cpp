#include <exception>
#include <fbjni/fbjni.h>
#include <jni.h>
#include <jsi/jsi.h>
#include <memory>

#include "TensorflowPlugin.h"
#include <ReactCommon/CallInvoker.h>
#include <ReactCommon/CallInvokerHolder.h>

namespace mrousavy {

JavaVM* java_machine;

using namespace facebook;
using namespace facebook::jni;

// Java Installer
struct TfliteModule : public jni::JavaClass<TfliteModule> {
public:
  static constexpr auto kJavaDescriptor = "Lcom/tflite/TfliteModule;";

  static jboolean
  nativeInstall(jni::alias_ref<jni::JClass>, jlong runtimePtr,
                jni::alias_ref<react::CallInvokerHolder::javaobject> jsCallInvokerHolder) {
    auto runtime = reinterpret_cast<jsi::Runtime*>(runtimePtr);
    if (runtime == nullptr) {
      // Runtime was null!
      return false;
    }
    auto jsCallInvoker = jsCallInvokerHolder->cthis()->getCallInvoker();

    auto fetchByteDataFromUrl = [](std::string url) {
      // Attaching Current Thread to JVM
      JNIEnv* env = nullptr;
      int getEnvStat = java_machine->GetEnv((void**)&env, JNI_VERSION_1_6);
      if (getEnvStat == JNI_EDETACHED) {
        if (java_machine->AttachCurrentThread(&env, nullptr) != 0) {
          throw std::runtime_error("Failed to attach thread to JVM");
        }
      }

      static const auto cls = javaClassStatic();
      static const auto method =
          cls->getStaticMethod<jbyteArray(std::string)>("fetchByteDataFromUrl");

      auto byteData = method(cls, url);

      auto size = byteData->size();
      auto bytes = byteData->getRegion(0, size);
      void* data = malloc(size);
      memcpy(data, bytes.get(), size);

      return Buffer{.data = data, .size = size};
    };

    try {
      TensorflowPlugin::installToRuntime(*runtime, jsCallInvoker, fetchByteDataFromUrl);
    } catch (std::exception& exc) {
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

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void*) {
  java_machine = vm;
  return facebook::jni::initialize(vm, [] { mrousavy::TfliteModule::registerNatives(); });
}
