type TypedArray = Float32Array | Float64Array | Int8Array | Int16Array | Int32Array | Uint8Array | Uint16Array | Uint32Array | BigInt64Array | BigUint64Array;
declare global {
    /**
     * Loads the Model into memory. Path is fetchable resource, e.g.:
     * http://192.168.8.110:8081/assets/assets/model.tflite?platform=ios&hash=32e9958c83e5db7d0d693633a9f0b175
     */
    var __loadTensorflowModel: (path: string, delegate: TensorflowModelDelegate) => Promise<TensorflowModel>;
}
export type TensorflowModelDelegate = 'default' | 'metal' | 'core-ml' | 'nnapi' | 'android-gpu';
export interface Tensor {
    /**
     * The name of the Tensor.
     */
    name: string;
    /**
     * The data-type all values of this Tensor are represented in.
     */
    dataType: 'bool' | 'uint8' | 'int8' | 'int16' | 'int32' | 'int64' | 'float16' | 'float32' | 'float64' | 'invalid';
    /**
     * The shape of the data from this tensor.
     */
    shape: number[];
}
export interface TensorflowModel {
    /**
     * The computation delegate used by this Model.
     * While CoreML and Metal delegates might be faster as they use the GPU, not all models support those delegates.
     */
    delegate: TensorflowModelDelegate;
    /**
     * Run the Tensorflow Model with the given input buffer.
     * The input buffer has to match the input tensor's shape.
     */
    run(input: TypedArray[]): Promise<TypedArray[]>;
    /**
     * Synchronously run the Tensorflow Model with the given input buffer.
     * The input buffer has to match the input tensor's shape.
     */
    runSync(input: TypedArray[]): TypedArray[];
    /**
     * All input tensors of this Tensorflow Model.
     */
    inputs: Tensor[];
    /**
     * All output tensors of this Tensorflow Model.
     * The user is responsible for correctly interpreting this data.
     */
    outputs: Tensor[];
}
type Require = number;
type ModelSource = Require | {
    url: string;
};
export type TensorflowPlugin = {
    model: TensorflowModel;
    state: 'loaded';
} | {
    model: undefined;
    state: 'loading';
} | {
    model: undefined;
    error: Error;
    state: 'error';
};
/**
 * Load a Tensorflow Lite Model from the given `.tflite` asset.
 *
 * * If you are passing in a `.tflite` model from your app's bundle using `require(..)`, make sure to add `tflite` as an asset extension to `metro.config.js`!
 * * If you are passing in a `{ url: ... }`, make sure the URL points directly to a `.tflite` model. This can either be a web URL (`http://..`/`https://..`), or a local file (`file://..`).
 *
 * @param source The `.tflite` model in form of either a `require(..)` statement or a `{ url: string }`.
 * @param delegate The delegate to use for computations. Uses the standard CPU delegate per default. The `core-ml` or `metal` delegates are GPU-accelerated, but don't work on every model.
 * @returns The loaded Model.
 */
export declare function loadTensorflowModel(source: ModelSource, delegate?: TensorflowModelDelegate): Promise<TensorflowModel>;
/**
 * Load a Tensorflow Lite Model from the given `.tflite` asset into a React State.
 *
 * * If you are passing in a `.tflite` model from your app's bundle using `require(..)`, make sure to add `tflite` as an asset extension to `metro.config.js`!
 * * If you are passing in a `{ url: ... }`, make sure the URL points directly to a `.tflite` model. This can either be a web URL (`http://..`/`https://..`), or a local file (`file://..`).
 *
 * @param source The `.tflite` model in form of either a `require(..)` statement or a `{ url: string }`.
 * @param delegate The delegate to use for computations. Uses the standard CPU delegate per default. The `core-ml` or `metal` delegates are GPU-accelerated, but don't work on every model.
 * @returns The state of the Model.
 */
export declare function useTensorflowModel(source: ModelSource, delegate?: TensorflowModelDelegate): TensorflowPlugin;
export {};
//# sourceMappingURL=TensorflowLite.d.ts.map