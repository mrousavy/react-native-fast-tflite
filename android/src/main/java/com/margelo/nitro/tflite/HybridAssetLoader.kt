package com.margelo.nitro.tflite

import androidx.annotation.Keep
import com.facebook.proguard.annotations.DoNotStrip
import com.margelo.nitro.core.ArrayBuffer
import com.margelo.nitro.core.Promise
import java.net.URL

@Keep
@DoNotStrip
class HybridAssetLoader : HybridAssetLoaderSpec() {
  override fun loadAsset(path: String): Promise<ArrayBuffer> {
    return Promise.async {
      val bytes = URL(path).readBytes()
      return@async ArrayBuffer.copy(bytes)
    }
  }
}
