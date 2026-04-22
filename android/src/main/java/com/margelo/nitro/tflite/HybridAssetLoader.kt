package com.margelo.nitro.tflite

import androidx.annotation.Keep
import com.facebook.proguard.annotations.DoNotStrip
import com.margelo.nitro.NitroModules
import com.margelo.nitro.core.ArrayBuffer
import com.margelo.nitro.core.Promise
import java.net.URL

@Keep
@DoNotStrip
class HybridAssetLoader : HybridAssetLoaderSpec() {
  override fun loadAsset(path: String): Promise<ArrayBuffer> {
    return Promise.async {
      val bytes = if (path.startsWith("http://") ||
                      path.startsWith("https://") ||
                      path.startsWith("file://")) {
        URL(path).readBytes()
      } else {
        // In release builds, RN's AssetSourceResolver returns a bare Android
        // resource identifier (e.g. "app_assets_yolo_nas_pose_n_320_gpu_fp16")
        // for non-drawable extensions packaged into res/raw. java.net.URL cannot
        // read these — resolve via the app Context / Resources instead.
        val ctx = NitroModules.applicationContext
          ?: throw IllegalStateException("No ReactApplicationContext available to resolve asset: $path")
        val resId = ctx.resources.getIdentifier(path, "raw", ctx.packageName)
        if (resId == 0) throw IllegalArgumentException("Asset not found in res/raw: $path")
        ctx.resources.openRawResource(resId).use { it.readBytes() }
      }
      return@async ArrayBuffer.copy(bytes)
    }
  }
}
