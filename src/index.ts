import { useEffect, useState } from 'react'
import { Image } from 'react-native'
import { NitroModules } from 'react-native-nitro-modules'
import type {
  TfliteModel,
  TfliteModule,
  AssetLoader,
  Tensor,
  TensorflowModelDelegate,
} from './specs/Tflite.nitro'

export type { TensorflowModelDelegate, Tensor, TfliteModel }

/** @alias TfliteModel - backwards compatible name from react-native-fast-tflite v2 */
export type TensorflowModel = TfliteModel

// In React Native, `require(..)` returns a number.
type Require = number
type ModelSource = Require | { url: string }

export type TensorflowPlugin =
  | {
      model: TfliteModel
      state: 'loaded'
    }
  | {
      model: undefined
      state: 'loading'
    }
  | {
      model: undefined
      error: Error
      state: 'error'
    }

// -- Nitro module instances --

const assetLoader = NitroModules.createHybridObject<AssetLoader>('AssetLoader')
const tfliteModule =
  NitroModules.createHybridObject<TfliteModule>('TfliteModule')

// -- Public API --

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
export async function loadTensorflowModel(
  source: ModelSource,
  delegate: TensorflowModelDelegate = 'default'
): Promise<TfliteModel> {
  let uri: string
  if (typeof source === 'number') {
    console.log(`Loading Tensorflow Lite Model ${source}`)
    const asset = Image.resolveAssetSource(source)
    uri = asset.uri
    console.log(`Resolved Model path: ${asset.uri}`)
  } else if (typeof source === 'object' && 'url' in source) {
    uri = source.url
  } else {
    throw new Error(
      'TFLite: Invalid source passed! Source should be either a React Native require(..) or a `{ url: string }` object!'
    )
  }
  const data = await assetLoader.loadAsset(uri)
  return tfliteModule.createModel(data, delegate)
}

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
export function useTensorflowModel(
  source: ModelSource,
  delegate: TensorflowModelDelegate = 'default'
): TensorflowPlugin {
  const [state, setState] = useState<TensorflowPlugin>({
    model: undefined,
    state: 'loading',
  })

  useEffect(() => {
    const load = async (): Promise<void> => {
      try {
        setState({ model: undefined, state: 'loading' })
        const m = await loadTensorflowModel(source, delegate)
        setState({ model: m, state: 'loaded' })
        console.log('Model loaded!')
      } catch (e) {
        console.error(`Failed to load Tensorflow Model ${source}!`, e)
        setState({ model: undefined, state: 'error', error: e as Error })
      }
    }
    load()
  }, [delegate, source])

  return state
}
