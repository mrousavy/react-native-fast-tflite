# vision-camera-tflite

High-performance TensorFlow Lite library for React Native

## Installation

```sh
npm install vision-camera-tflite
```

## Usage

```js
import { multiply } from 'vision-camera-tflite';

// ...

const result = await multiply(3, 7);
```

### Using GPU Delegates

GPU Delegates offer faster, GPU accelerated computation. There's multiple different GPU delegates which you can enable:

#### CoreML (iOS)

To enable the CoreML Delegate, you need to include the CoreML/Metal code in your project:

1. Set `$EnableCoreMLDelegate` to true in your `Podfile`:
    ```ruby
    $EnableCoreMLDelegate=true

    # rest of your podfile...
    ```
2. Open your iOS project in Xcode and add the `CoreML` framework to your project:
    ![Xcode > xcodeproj > General > Frameworks, Libraries and Embedded Content > CoreML](ios/../img/ios-coreml-guide.png)


## Contributing

See the [contributing guide](CONTRIBUTING.md) to learn how to contribute to the repository and the development workflow.

## License

MIT

---

Made with [create-react-native-library](https://github.com/callstack/react-native-builder-bob)
