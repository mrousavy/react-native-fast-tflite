import * as React from 'react';

import { StyleSheet, View, Text } from 'react-native';
import {
  loadTensorflowModel,
  useTensorflowModel,
} from '@mrousavy/react-native-tflite';

export default function App() {
  const [result, setResult] = React.useState<number | undefined>();

  const model = useTensorflowModel(
    require('../assets/smartreply_1_default_1.tflite')
  );

  React.useEffect(() => {
    if (model.model == null) return;

    console.log(`Running Model...`);
    const result = model.model.run([new Uint8Array([5])]);
    console.log(`Successfully ran Model!`, result);
  }, [model.model]);

  return (
    <View style={styles.container}>
      <Text>Result: {result}</Text>
    </View>
  );
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
});
