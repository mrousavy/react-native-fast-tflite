export type {
  TensorflowModelDelegate,
  Tensor,
  TfliteModel,
} from './specs/Tflite.nitro'
export type { TensorflowPlugin } from './useTensorflowModel'
export type { ModelSource } from './loadTensorflowModel'
export { loadTensorflowModel } from './loadTensorflowModel'
export { useTensorflowModel } from './useTensorflowModel'

/** @alias TfliteModel - backwards compatible name from react-native-fast-tflite v2 */
export type { TfliteModel as TensorflowModel } from './specs/Tflite.nitro'
