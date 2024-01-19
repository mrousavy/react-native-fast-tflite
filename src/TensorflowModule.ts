import { NativeModules, Platform } from 'react-native'

const LINKING_ERROR =
  "The package 'react-native-fast-tflite' doesn't seem to be linked. Make sure: \n\n" +
  Platform.select({ ios: "- You have run 'pod install'\n", default: '' }) +
  '- You rebuilt the app after installing the package\n' +
  '- You are not using Expo Go\n'

// eslint-disable-next-line @typescript-eslint/no-unsafe-assignment
const Tflite =
  NativeModules.Tflite != null
    ? NativeModules.Tflite
    : new Proxy(
        {},
        {
          get() {
            throw new Error(LINKING_ERROR)
          },
        }
      )

// eslint-disable-next-line @typescript-eslint/no-unsafe-assignment
export const TensorflowModule = Tflite
