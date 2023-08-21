package com.tflite;

import android.util.Log;

import androidx.annotation.NonNull;

import com.facebook.react.bridge.JavaScriptContextHolder;
import com.facebook.react.bridge.Promise;
import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.bridge.ReactContextBaseJavaModule;
import com.facebook.react.bridge.ReactMethod;
import com.facebook.react.module.annotations.ReactModule;
import com.facebook.react.turbomodule.core.CallInvokerHolderImpl;

/** @noinspection JavaJniMissingFunction*/
@ReactModule(name = TfliteModule.NAME)
public class TfliteModule extends ReactContextBaseJavaModule {
  public static final String NAME = "vision-camera-tflite";

  public TfliteModule(ReactApplicationContext reactContext) {
    super(reactContext);
  }

  @Override
  @NonNull
  public String getName() {
    return NAME;
  }

  @ReactMethod(isBlockingSynchronousMethod = true)
  public boolean install() {
    try {
      Log.i(NAME, "Loading C++ library...");
      System.loadLibrary("vision-camera-tflite");

      JavaScriptContextHolder jsContext = getReactApplicationContext().getJavaScriptContextHolder();
      CallInvokerHolderImpl callInvoker = (CallInvokerHolderImpl) getReactApplicationContext().getCatalystInstance().getJSCallInvokerHolder();

      Log.i(NAME, "Installing JSI Bindings for VisionCamera Tflite plugin...");
      boolean successful = nativeInstall(jsContext.get(), callInvoker);
      if (successful) {
        Log.i(NAME, "Successfully installed JSI Bindings!");
        return true;
      } else {
        Log.e(NAME, "Failed to install JSI Bindings for VisionCamera Tflite plugin!");
        return false;
      }
    } catch (Exception exception) {
      Log.e(NAME, "Failed to install JSI Bindings!", exception);
      return false;
    }
  }

  private static native boolean nativeInstall(long jsiPtr, CallInvokerHolderImpl jsCallInvoker);
}
