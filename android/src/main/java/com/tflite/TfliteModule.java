package com.tflite;

import android.util.Log;

import com.facebook.proguard.annotations.DoNotStrip;

import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;

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
  public static final String NAME = "Tflite";

  public TfliteModule(ReactApplicationContext reactContext) {
    super(reactContext);
  }

  @Override
  @NonNull
  public String getName() {
    return NAME;
  }

  @DoNotStrip
  public static byte[] fetchByteDataFromUrl(String url) {
          Log.e("AssetsManager", "fetch URL 1 " + url + "!");

    OkHttpClient client = new OkHttpClient();

    Request request = new Request.Builder().url(url).build();

    try (Response response = client.newCall(request).execute()) {

      if (response.isSuccessful() && response.body() != null) {

        return response.body().bytes();
      } else {
        throw new RuntimeException("Response was not successful!");
      }
    } catch (Exception e) {
      Log.e("AssetsManager", "Failed to fetch URL " + url + "!", e);
      return null;
    }
  }

  @ReactMethod(isBlockingSynchronousMethod = true)
  public boolean install() {
    try {
      Log.i(NAME, "Loading C++ library...");
      System.loadLibrary("VisionCameraTflite");

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
