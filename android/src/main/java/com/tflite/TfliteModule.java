package com.tflite;

import android.annotation.SuppressLint;
import android.content.Context;
import android.net.Uri;
import android.util.Log;

import androidx.annotation.NonNull;

import com.facebook.proguard.annotations.DoNotStrip;
import com.facebook.react.bridge.JavaScriptContextHolder;
import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.bridge.ReactContextBaseJavaModule;
import com.facebook.react.bridge.ReactMethod;
import com.facebook.react.module.annotations.ReactModule;
import com.facebook.react.turbomodule.core.CallInvokerHolderImpl;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.ref.WeakReference;
import java.util.Objects;

import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;

/** @noinspection JavaJniMissingFunction*/
@ReactModule(name = TfliteModule.NAME)
public class TfliteModule extends ReactContextBaseJavaModule {
  public static final String NAME = "Tflite";
  private static WeakReference<ReactApplicationContext> weakContext;
  private static final OkHttpClient client = new OkHttpClient();

  public TfliteModule(ReactApplicationContext reactContext) {
    super(reactContext);
    weakContext = new WeakReference<>(reactContext);
  }

  @Override
  @NonNull
  public String getName() {
    return NAME;
  }

  @SuppressLint("DiscouragedApi")
  private static int getResourceId(Context context, String name) {
    return context.getResources().getIdentifier(
            name,
            "raw",
            context.getPackageName()
    );
  }

  /** @noinspection unused*/
  @DoNotStrip
  public static byte[] fetchByteDataFromUrl(String url) throws Exception {
    Log.i(NAME, "Loading byte data from URL: " + url + "...");

    Uri uri = null;
    Integer resourceId = null;
    if (url.contains("://")) {
      Log.i(NAME, "Parsing URL...");
      uri = Uri.parse(url);
      Log.i(NAME, "Parsed URL: " + uri.toString());
    } else {
      Log.i(NAME, "Parsing resourceId...");
      resourceId = getResourceId(weakContext.get(), url);
      Log.i(NAME, "Parsed resourceId: " + resourceId);
    }

    if (uri != null) {
      if (Objects.equals(uri.getScheme(), "file")) {
        // It's a file URL
        String path = Objects.requireNonNull(uri.getPath(), "File path cannot be null");
        File file = new File(path);

        // Check if file exists and is readable
        if (!file.exists() || !file.canRead()) {
          throw new IOException("File does not exist or is not readable: " + path);
        }

        // Check if the file has a .tflite extension
        if (!file.getName().toLowerCase().endsWith(".tflite")) {
          throw new SecurityException("Only .tflite files are allowed");
        }

        // Read the file
        try (FileInputStream stream = new FileInputStream(file)) {
          return getLocalFileBytes(stream, file);
        } catch (IOException e) {
          Log.e(NAME, "Error reading file: " + path, e);
          throw new RuntimeException("Failed to read file: " + path, e);
        }
      } else {
        // It's a network URL/http resource
        Request request = new Request.Builder().url(uri.toString()).build();
        try (Response response = client.newCall(request).execute()) {
          if (response.isSuccessful() && response.body() != null) {
            return response.body().bytes();
          } else {
            throw new RuntimeException("Response was not successful!");
          }
        } catch (Exception ex) {
          Log.e(NAME, "Failed to fetch URL " + url + "!", ex);
          throw ex;
        }
      }
    } else if (resourceId != null) {
      // It's bundled into the Android resources/assets
      Context context = weakContext.get();
      if (context == null) {
        throw new Exception("React Context has already been destroyed!");
      }
      try (InputStream stream = context.getResources().openRawResource(resourceId)) {
        ByteArrayOutputStream byteStream = new ByteArrayOutputStream();
        byte[] buffer = new byte[2048];
        int length;
        while ((length = stream.read(buffer)) != -1) {
          byteStream.write(buffer, 0, length);
        }
        return byteStream.toByteArray();
      }
    } else {
      // It's a bird? it's a plane? not it's an error
      throw new Exception("Input is neither a valid URL, nor a resourceId - " +
              "cannot load TFLite model! (Input: " + url + ")");
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

  private static byte[] getLocalFileBytes(InputStream stream, File file) throws IOException {
    long fileSize = file.length();

    if (fileSize > Integer.MAX_VALUE) {
      throw new IOException("File is too large to read into memory");
    }

    byte[] data = new byte[(int) fileSize];

    int bytesRead = 0;
    int chunk;
    while (bytesRead < fileSize && (chunk = stream.read(data, bytesRead, (int)fileSize - bytesRead)) != -1) {
      bytesRead += chunk;
    }

    if (bytesRead != fileSize) {
      throw new IOException("Could not completely read file " + file.getName());
    }

    return data;
  }
}
