#include "TfliteHelpers.hpp"

typedef float float32_t;
typedef double float64_t;

std::string tfLiteStatusToString(TfLiteStatus status) {
  switch (status) {
    case kTfLiteOk:
      return "ok";
    case kTfLiteError:
      return "error";
    case kTfLiteDelegateError:
      return "delegate-error";
    case kTfLiteApplicationError:
      return "application-error";
    case kTfLiteDelegateDataNotFound:
      return "delegate-data-not-found";
    case kTfLiteDelegateDataWriteError:
      return "delegate-data-write-error";
    case kTfLiteDelegateDataReadError:
      return "delegate-data-read-error";
    case kTfLiteUnresolvedOps:
      return "unresolved-ops";
    case kTfLiteCancelled:
      return "cancelled";
  }
  return "unknown";
}

std::string dataTypeToString(TfLiteType dataType) {
  switch (dataType) {
    case kTfLiteFloat32:
      return "float32";
    case kTfLiteFloat64:
      return "float64";
    case kTfLiteInt4:
      return "int4";
    case kTfLiteInt8:
      return "int8";
    case kTfLiteInt16:
      return "int16";
    case kTfLiteInt32:
      return "int32";
    case kTfLiteInt64:
      return "int64";
    case kTfLiteUInt8:
      return "uint8";
    case kTfLiteUInt16:
      return "uint16";
    case kTfLiteUInt32:
      return "uint32";
    case kTfLiteUInt64:
      return "uint64";
    case kTfLiteNoType:
      return "none";
    case kTfLiteString:
      return "string";
    case kTfLiteBool:
      return "bool";
    case kTfLiteComplex64:
      return "complex64";
    case kTfLiteComplex128:
      return "complex128";
    case kTfLiteResource:
      return "resource";
    case kTfLiteVariant:
      return "variant";
  }
  return "invalid";
}

size_t getTFLTensorDataTypeSize(TfLiteType dataType) {
  switch (dataType) {
    case kTfLiteFloat32:
      return sizeof(float32_t);
    case kTfLiteInt32:
      return sizeof(int32_t);
    case kTfLiteUInt8:
      return sizeof(uint8_t);
    case kTfLiteInt64:
      return sizeof(int64_t);
    case kTfLiteInt16:
      return sizeof(int16_t);
    case kTfLiteInt8:
      return sizeof(int8_t);
    case kTfLiteFloat64:
      return sizeof(float64_t);
    case kTfLiteUInt64:
      return sizeof(uint64_t);
    case kTfLiteUInt32:
      return sizeof(uint32_t);
    case kTfLiteUInt16:
      return sizeof(uint16_t);
  }
  throw std::runtime_error("TFLite: Unsupported output data type! " + dataTypeToString(dataType));
}

int getTensorTotalLength(const TfLiteTensor* tensor) {
  int dimensions = TfLiteTensorNumDims(tensor);
  if (dimensions < 1)
    return 0;
  int size = 1;
  for (size_t i = 0; i < dimensions; i++) {
    size *= TfLiteTensorDim(tensor, i);
  }
  return size;
}
