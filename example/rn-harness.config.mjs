import {
    androidPlatform,
    androidEmulator,
  } from '@react-native-harness/platform-android';
  import {
    applePlatform,
    appleSimulator,
  } from '@react-native-harness/platform-apple';
  
  const config = {
    entryPoint: './index.js',
    appRegistryComponentName: 'TFLiteExample',
  
    runners: [
      androidPlatform({
        name: 'android',
        device: androidEmulator('Pixel_8_API_35'), // Your Android emulator name
        bundleId: 'com.margelo.nitro.tflite.example', // Your Android bundle ID
      }),
      applePlatform({
        name: 'ios',
        device: appleSimulator('iPhone 17 Pro', '26.2'), // Your iOS simulator name and version
        bundleId: 'com.margelo.nitro.tflite.example', // Your iOS bundle ID
      }),
    ],
    defaultRunner: 'android',
    bridgeTimeout: 180000,
  };
  
  export default config;
  
