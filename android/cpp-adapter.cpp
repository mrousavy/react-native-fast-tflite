#include <jni.h>
#include "react-native-tflite.h"

extern "C"
JNIEXPORT jint JNICALL
Java_com_tflite_TfliteModule_nativeMultiply(JNIEnv *env, jclass type, jdouble a, jdouble b) {
    return tflite::multiply(a, b);
}
