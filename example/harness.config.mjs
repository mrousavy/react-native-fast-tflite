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
    appRegistryComponentName: 'TfliteExample',
  
    runners: [
      androidPlatform({
        name: 'android',
        device: androidEmulator('Pixel_6_Pro'), // Your Android emulator name
        bundleId: 'com.tfliteexamplenew', // Your Android bundle ID
      }),
      applePlatform({
        name: 'ios',
        device: appleSimulator('iPhone 17 Pro', '26.2'), // Your iOS simulator name and version
        bundleId: 'org.reactjs.native.example.TfliteExample', // Your iOS bundle ID
      }),
    ],
    defaultRunner: 'android',
    bridgeTimeout: 180000,
  };
  
  export default config;
  