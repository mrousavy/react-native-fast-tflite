import Foundation
import NitroModules

class HybridAssetLoader: HybridAssetLoaderSpec {
  func loadAsset(path: String) throws -> Promise<ArrayBuffer> {
    return Promise.async {
      guard let url = URL(string: path) else {
        throw NSError(domain: "AssetLoader", code: 1,
                      userInfo: [NSLocalizedDescriptionKey: "Invalid URL: \(path)"])
      }
      let data = try Data(contentsOf: url)
      return try ArrayBuffer.copy(data: data)
    }
  }
}
