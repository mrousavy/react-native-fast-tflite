import * as React from 'react';

import {
  StyleSheet,
  View,
  Text,
  ActivityIndicator,
  Platform,
} from 'react-native';
import {
  Tensor,
  TensorflowModel,
  useTensorflowModel,
} from 'react-native-fast-tflite';
import { NitroModules } from 'react-native-nitro-modules';
import {
  Camera,
  useCameraDevice,
  useCameraPermission,
  useFrameProcessor,
} from 'react-native-vision-camera';
import { useResizePlugin } from 'vision-camera-resize-plugin';

function tensorToString(tensor: Tensor): string {
  return `\n  - ${tensor.dataType} ${tensor.name}[${tensor.shape}]`;
}
function modelToString(model: TensorflowModel): string {
  return (
    `TFLite Model (${model.delegates.join(', ')}):\n` +
    `- Inputs: ${model.inputs.map(tensorToString).join('')}\n` +
    `- Outputs: ${model.outputs.map(tensorToString).join('')}`
  );
}

export default function App(): React.ReactNode {
  const { hasPermission, requestPermission } = useCameraPermission();
  const device = useCameraDevice('back');

  // from https://www.kaggle.com/models/tensorflow/efficientdet/frameworks/tfLite
  const model = useTensorflowModel(
    require('../assets/efficientdet.tflite'),
    Platform.OS === 'ios' ? ['core-ml', 'default'] : ['default'],
  );
  const actualModel = model.state === 'loaded' ? model.model : undefined;
  // Nitro HybridObjects use jsi::NativeState which is not directly accessible
  // in VisionCamera's worklet runtime. Boxing converts the HybridObject into a
  // jsi::HostObject so it can safely cross the worklet boundary. Call .unbox()
  // inside the worklet to recover the full TfliteModel.
  const boxedModel = React.useMemo(
    () => (actualModel != null ? NitroModules.box(actualModel) : undefined),
    [actualModel],
  );

  React.useEffect(() => {
    if (actualModel == null) return;
    console.log(`Model loaded! Shape:\n${modelToString(actualModel)}]`);
  }, [actualModel]);

  const { resize } = useResizePlugin();

  const frameProcessor = useFrameProcessor(
    frame => {
      'worklet';
      if (boxedModel == null) {
        // model is still loading...
        return;
      }

      // Unbox the HybridObject inside the worklet to access runSync
      const tflite = boxedModel.unbox();
      console.log(`Running inference on ${frame}`);
      const resized = resize(frame, {
        scale: {
          width: 320,
          height: 320,
        },
        pixelFormat: 'rgb',
        dataType: 'uint8',
      });
      const inputBuffer = resized.buffer.slice(
        resized.byteOffset,
        resized.byteOffset + resized.byteLength,
      );
      const result = tflite.runSync([inputBuffer]);
      const num_detections = new Float32Array(result[3])[0] ?? 0;
      console.log('Result: ' + num_detections);
    },
    [boxedModel],
  );

  React.useEffect(() => {
    requestPermission();
  }, [requestPermission]);

  console.log(`Model: ${model.state} (${model.model != null})`);

  return (
    <View style={styles.container}>
      {hasPermission && device != null ? (
        <Camera
          device={device}
          style={StyleSheet.absoluteFill}
          isActive={true}
          frameProcessor={frameProcessor}
          pixelFormat="yuv"
        />
      ) : (
        <Text>No Camera available.</Text>
      )}

      {model.state === 'loading' && (
        <ActivityIndicator size="small" color="white" />
      )}

      {model.state === 'error' && (
        <Text>Failed to load model! {model.error.message}</Text>
      )}
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    alignItems: 'center',
    justifyContent: 'center',
  },
});
