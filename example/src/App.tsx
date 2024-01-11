import * as React from 'react'

import {
  StyleSheet,
  View,
  Text,
  Platform,
  ActivityIndicator,
} from 'react-native'
import { useTensorflowModel } from 'react-native-fast-tflite'
import {
  Camera,
  useCameraDevice,
  useCameraPermission,
  useFrameProcessor,
} from 'react-native-vision-camera'

export default function App() {
  const { hasPermission, requestPermission } = useCameraPermission()
  const device = useCameraDevice('back')

  const model = useTensorflowModel(
    require('../assets/object_detection_mobile_object_localizer_v1_1_default_1.tflite'),
    Platform.OS === 'ios' ? 'core-ml' : 'default'
  )
  const actualModel = model.state === 'loaded' ? model.model : undefined

  const frameProcessor = useFrameProcessor(
    (frame) => {
      'worklet'
      console.log(frame.toString())
      if (actualModel == null) {
        // model is still loading...
        return
      }

      const data = frame.toArrayBuffer()
      const result = actualModel.runSync([data])
      console.log('Result: ' + result.length)
    },
    [actualModel]
  )

  React.useEffect(() => {
    requestPermission()
  }, [requestPermission])

  console.log(`Model: ${model.state} (${model.model != null})`)

  return (
    <View style={styles.container}>
      {hasPermission && device != null ? (
        <Camera
          device={device}
          style={StyleSheet.absoluteFill}
          isActive={true}
          frameProcessor={frameProcessor}
        />
      ) : (
        <Text>No Camera available.</Text>
      )}

      {model.state === 'loading' && (
        <ActivityIndicator size="small" color="white" />
      )}

      {model.state === 'error' && (
        <Text>Failed to load model! {model.error}</Text>
      )}
    </View>
  )
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    alignItems: 'center',
    justifyContent: 'center',
  },
  box: {
    width: 60,
    height: 60,
    marginVertical: 20,
  },
})
