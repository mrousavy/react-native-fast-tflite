import type { HybridObject } from 'react-native-nitro-modules'

export type TensorflowModelDelegate =
  | 'default'
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
  createModel(
    modelData: ArrayBuffer,
    delegates: TensorflowModelDelegate[]
  ): TfliteModel
}

export interface AssetLoader
  extends HybridObject<{ ios: 'swift'; android: 'kotlin' }> {
  loadAsset(path: string): Promise<ArrayBuffer>
}
