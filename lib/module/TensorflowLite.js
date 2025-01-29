import { useEffect, useState } from 'react';
import { Image } from 'react-native';
import { TensorflowModule } from './TensorflowModule';
// Installs the JSI bindings into the global namespace.
console.log('Installing bindings...');
const result = TensorflowModule?.install();
if (result !== true) console.error('Failed to install Tensorflow Lite bindings!');
console.log('Successfully installed!');

// In React Native, `require(..)` returns a number.
// ReturnType<typeof require>

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
export function loadTensorflowModel(source) {
  let delegate = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 'default';
  let uri;
  if (typeof source === 'number') {
    console.log(`Loading Tensorflow Lite Model ${source}`);
    const asset = Image.resolveAssetSource(source);
    uri = asset.uri;
    console.log(`Resolved Model path: ${asset.uri}`);
  } else if (typeof source === 'object' && 'url' in source) {
    uri = source.url;
  } else {
    throw new Error('TFLite: Invalid source passed! Source should be either a React Native require(..) or a `{ url: string }` object!');
  }
  return global.__loadTensorflowModel(uri, delegate);
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
export function useTensorflowModel(source) {
  let delegate = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 'default';
  const [state, setState] = useState({
    model: undefined,
    state: 'loading'
  });
  useEffect(() => {
    const load = async () => {
      try {
        setState({
          model: undefined,
          state: 'loading'
        });
        const m = await loadTensorflowModel(source, delegate);
        setState({
          model: m,
          state: 'loaded'
        });
        console.log('Model loaded!');
      } catch (e) {
        console.error(`Failed to load Tensorflow Model ${source}!`, e);
        setState({
          model: undefined,
          state: 'error',
          error: e
        });
      }
    };
    load();
  }, [delegate, source]);
  return state;
}
//# sourceMappingURL=TensorflowLite.js.map