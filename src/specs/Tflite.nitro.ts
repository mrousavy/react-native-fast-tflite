import type { HybridObject } from 'react-native-nitro-modules'

export type TensorflowModelDelegate =
  | 'metal'
  | 'core-ml'
  | 'nnapi'
  | 'android-gpu'

export interface Tensor {
  name: string
  dataType: string
  shape: number[]
}

export interface TfliteModel
  extends HybridObject<{ ios: 'c++'; android: 'c++' }> {
  readonly delegates: TensorflowModelDelegate[]
  readonly inputs: Tensor[]
  readonly outputs: Tensor[]
  runSync(input: ArrayBuffer[]): ArrayBuffer[]
  run(input: ArrayBuffer[]): Promise<ArrayBuffer[]>
}

export interface TfliteModule
  extends HybridObject<{ ios: 'c++'; android: 'c++' }> {
  /**
   * Create a new {@linkcode TfliteModel} with the given
   * {@linkcode modelData} (a binary representation of the
   * TFLite model), and optionally a list of hardware
   * accelerating {@linkcode TensorflowModelDelegate}s.
   *
   * If {@linkcode delegates} is empty (`[]`), the default
   * CPU delegate will be used.
   */
  createModel(
    modelData: ArrayBuffer,
    delegates: TensorflowModelDelegate[]
  ): TfliteModel
}

export interface AssetLoader
  extends HybridObject<{ ios: 'swift'; android: 'kotlin' }> {
  /**
   * Load an asset from the given {@linkcode path} and
   * return its contents as an {@linkcode ArrayBuffer}.
   */
  loadAsset(path: string): Promise<ArrayBuffer>
}
