import { describe, it, expect, beforeAll } from 'react-native-harness';
import {
  loadTensorflowModel,
  type TfliteModel,
  type Tensor,
} from 'react-native-fast-tflite';


const MODEL_ASSET = require('../assets/efficientdet.tflite') as number;


const CPU_DELEGATES: [] = [];

function bytesPerElement(dataType: string): number {
  switch (dataType) {
    case 'uint8':
    case 'int8':
      return 1;
    case 'float16':
    case 'bfloat16':
    case 'int16':
    case 'uint16':
      return 2;
    case 'float32':
    case 'int32':
    case 'uint32':
      return 4;
    case 'float64':
    case 'int64':
    case 'uint64':
      return 8;
    default:
      throw new Error(
        `Unsupported tensor data type for buffer sizing: ${dataType}`,
      );
  }
}

function tensorElementCount(tensor: Tensor): number {
  const dims = tensor.shape;
  if (dims.length === 0) {
    return 1;
  }
  return dims.reduce((a, b) => a * b, 1);
}

function tensorByteLength(tensor: Tensor): number {
  const dims = tensor.shape;
  if (dims.length === 0) {
    return bytesPerElement(tensor.dataType);
  }
  const count = dims.reduce((a, b) => a * b, 1);
  return count * bytesPerElement(tensor.dataType);
}

function zeroedInputBuffer(input: Tensor): ArrayBuffer {
  const len = tensorByteLength(input);
  const buf = new ArrayBuffer(len);
  new Uint8Array(buf).fill(0);
  return buf;
}

function filledInputBuffer(input: Tensor, byte: number): ArrayBuffer {
  const len = tensorByteLength(input);
  const buf = new ArrayBuffer(len);
  new Uint8Array(buf).fill(byte & 0xff);
  return buf;
}

function buffersEqual(a: ArrayBuffer, b: ArrayBuffer): boolean {
  if (a.byteLength !== b.byteLength) {
    return false;
  }
  const ua = new Uint8Array(a);
  const ub = new Uint8Array(b);
  for (let i = 0; i < ua.length; i++) {
    if (ua[i] !== ub[i]) {
      return false;
    }
  }
  return true;
}

function firstInputTensor(model: TfliteModel): Tensor {
  const tensor = model.inputs[0];
  if (tensor == null) {
    throw new Error('Expected model to declare at least one input tensor');
  }
  return tensor;
}

function tensorSpecsMatch(a: Tensor[], b: Tensor[]): boolean {
  if (a.length !== b.length) {
    return false;
  }
  for (let i = 0; i < a.length; i++) {
    const u = a[i];
    const v = b[i];
    if (u == null || v == null) {
      return false;
    }
    if (
      u.name !== v.name ||
      u.dataType !== v.dataType ||
      u.shape.length !== v.shape.length
    ) {
      return false;
    }
    for (let j = 0; j < u.shape.length; j++) {
      if (u.shape[j] !== v.shape[j]) {
        return false;
      }
    }
  }
  return true;
}

function expectAllFloat32Finite(buf: ArrayBuffer, tensor: Tensor): void {
  if (tensor.dataType !== 'float32') {
    return;
  }
  const n = tensorElementCount(tensor);
  const floats = new Float32Array(buf, 0, n);
  for (let i = 0; i < floats.length; i++) {
    expect(Number.isFinite(floats[i])).toBe(true);
  }
}

describe('react-native-fast-tflite (harness)', () => {
  describe('bundled EfficientDet model', () => {
    let model: TfliteModel;

    beforeAll(async () => {
      model = await loadTensorflowModel(MODEL_ASSET, CPU_DELEGATES);
    });

    it('uses no hardware delegates when an empty list is passed', () => {
      expect(model.delegates).toEqual([]);
    });

    it('exposes non-empty input and output tensor metadata', () => {
      expect(model.inputs.length).toBeGreaterThanOrEqual(1);
      expect(model.outputs.length).toBeGreaterThanOrEqual(1);
      for (const t of [...model.inputs, ...model.outputs]) {
        expect(t.name.length).toBeGreaterThan(0);
        expect(t.dataType).not.toBe('none');
        expect(t.shape.length).toBeGreaterThan(0);
        expect(t.shape.every((d) => d > 0)).toBe(true);
      }
    });

    it('runSync returns one buffer per output with expected byte sizes', () => {
      const inputBuf = zeroedInputBuffer(firstInputTensor(model));
      const outputs = model.runSync([inputBuf]);
      expect(outputs).toHaveLength(model.outputs.length);
      for (let i = 0; i < outputs.length; i++) {
        const out = outputs[i];
        const spec = model.outputs[i];
        expect(out).toBeDefined();
        expect(spec).toBeDefined();
        expect(out!.byteLength).toBe(tensorByteLength(spec!));
      }
    });

    it('runSync rejects the wrong number of input buffers', () => {
      expect(() => model.runSync([])).toThrow(/input array size/i);
    });

    it('every float32 output has only finite values for a zero input', () => {
      const inputBuf = zeroedInputBuffer(firstInputTensor(model));
      const outputs = model.runSync([inputBuf]);
      for (let i = 0; i < model.outputs.length; i++) {
        const buf = outputs[i];
        const spec = model.outputs[i];
        expect(buf).toBeDefined();
        expect(spec).toBeDefined();
        expectAllFloat32Finite(buf!, spec!);
      }
    });

    it('runSync is deterministic for a fixed zero input', () => {
      const inputBuf = zeroedInputBuffer(firstInputTensor(model));
      const first = model.runSync([inputBuf]);
      const second = model.runSync([inputBuf]);
      expect(first).toHaveLength(second.length);
      for (let i = 0; i < first.length; i++) {
        const a = first[i];
        const b = second[i];
        expect(a).toBeDefined();
        expect(b).toBeDefined();
        expect(buffersEqual(a!, b!)).toBe(true);
      }
    });

    it('is deterministic for a fixed non-zero uint8 input when the tensor is uint8', () => {
      const input = firstInputTensor(model);
      if (input.dataType !== 'uint8') {
        return;
      }
      const buf = filledInputBuffer(input, 0x7f);
      const a = model.runSync([buf]);
      const b = model.runSync([buf]);
      expect(a).toHaveLength(b.length);
      for (let i = 0; i < a.length; i++) {
        expect(buffersEqual(a[i]!, b[i]!)).toBe(true);
      }
    });

    it('loading the same asset again yields the same tensor layout', async () => {
      const other = await loadTensorflowModel(MODEL_ASSET, CPU_DELEGATES);
      expect(tensorSpecsMatch(model.inputs, other.inputs)).toBe(true);
      expect(tensorSpecsMatch(model.outputs, other.outputs)).toBe(true);
      expect(other.delegates).toEqual([]);
    });

    it('two independently loaded models produce identical outputs for the same input', async () => {
      const other = await loadTensorflowModel(MODEL_ASSET, CPU_DELEGATES);
      const inputBuf = zeroedInputBuffer(firstInputTensor(model));
      const a = model.runSync([inputBuf]);
      const b = other.runSync([inputBuf]);
      expect(a).toHaveLength(b.length);
      for (let i = 0; i < a.length; i++) {
        expect(buffersEqual(a[i]!, b[i]!)).toBe(true);
      }
    });

    it('matches the num_detections read when layout is unchanged', () => {
      if (model.outputs.length <= 3) {
        return;
      }
      const meta = model.outputs[3];
      if (meta == null || meta.dataType !== 'float32') {
        return;
      }
      const inputBuf = zeroedInputBuffer(firstInputTensor(model));
      const result = model.runSync([inputBuf]);
      const detBuf = result[3];
      expect(detBuf).toBeDefined();
      const numDetections = new Float32Array(detBuf!)[0] ?? 0;
      expect(Number.isFinite(numDetections)).toBe(true);
      expect(numDetections).toBeGreaterThanOrEqual(0);
    });
  });
});
