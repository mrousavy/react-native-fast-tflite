package com.margelo.nitro.tflite

import android.net.Uri
import android.util.Log
import androidx.annotation.Keep
import com.facebook.proguard.annotations.DoNotStrip
import com.margelo.nitro.NitroModules
import com.margelo.nitro.core.ArrayBuffer
import com.margelo.nitro.core.Promise
import java.io.ByteArrayOutputStream
import java.io.File
import java.io.FileInputStream
import java.net.HttpURLConnection
import java.net.URL

@Keep
@DoNotStrip
class HybridAssetLoader : HybridAssetLoaderSpec() {
  override fun loadAsset(path: String): Promise<ArrayBuffer> {
    return Promise.async {
      val bytes = loadBytes(path)
      return@async ArrayBuffer.copy(bytes)
    }
  }

  companion object {
    private const val TAG = "TfliteAssetLoader"

    @Suppress("DiscouragedApi")
    private fun getRawResourceId(name: String): Int {
      val context =
        NitroModules.applicationContext
          ?: throw IllegalStateException("React context is not available")
      return context.resources.getIdentifier(name, "raw", context.packageName)
    }

    private fun readStream(stream: java.io.InputStream): ByteArray {
      val buffer = ByteArrayOutputStream()
      val chunk = ByteArray(2048)
      var read: Int
      while (stream.read(chunk).also { read = it } != -1) {
        buffer.write(chunk, 0, read)
      }
      return buffer.toByteArray()
    }

    private fun loadFromFile(uri: Uri, path: String): ByteArray {
      val filePath = uri.path ?: throw IllegalArgumentException("File path is null: $path")
      val file = File(filePath)
      if (!file.exists() || !file.canRead()) {
        throw IllegalArgumentException("File does not exist or is not readable: $filePath")
      }
      if (!file.name.lowercase().endsWith(".tflite")) {
        throw SecurityException("Only .tflite files are allowed")
      }
      return FileInputStream(file).use(::readStream)
    }

    private fun loadFromHttp(uri: Uri, path: String): ByteArray {
      val connection = URL(uri.toString()).openConnection() as HttpURLConnection
      connection.connectTimeout = 30_000
      connection.readTimeout = 60_000
      try {
        if (connection.responseCode !in 200..299) {
          throw RuntimeException("HTTP request failed (${connection.responseCode}) for $path")
        }
        return connection.inputStream.use(::readStream)
      } finally {
        connection.disconnect()
      }
    }

    /** Release: `require()` resolves to `assets_*` raw resource name. */
    private fun loadFromRawResource(path: String): ByteArray {
      val context =
        NitroModules.applicationContext
          ?: throw IllegalStateException("React context is not available")

      val rawId = getRawResourceId(path)
      if (rawId == 0) {
        throw IllegalArgumentException("Unknown raw resource for TFLite model: $path")
      }
      return context.resources.openRawResource(rawId).use(::readStream)
    }

    private fun loadBytes(path: String): ByteArray {
      Log.i(TAG, "Loading TFLite asset from: $path")

      if (path.contains("://")) {
        val uri = Uri.parse(path)
        return when (uri.scheme?.lowercase()) {
          "file" -> loadFromFile(uri, path)
          "http", "https" -> loadFromHttp(uri, path)
          else ->
            throw IllegalArgumentException("Unsupported URI scheme: ${uri.scheme} ($path)")
        }
      }

      return loadFromRawResource(path)
    }
  }
}
