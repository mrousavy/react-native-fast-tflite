#pragma once

#include "HybridTfliteModel.hpp"
#include "HybridTfliteModuleSpec.hpp"

namespace margelo::nitro::nitrotflite {

class HybridTfliteModule : public HybridTfliteModuleSpec {
public:
  HybridTfliteModule() : HybridObject(TAG) {}

  // Methods (from HybridTfliteModuleSpec)
  std::shared_ptr<HybridTfliteModelSpec> createModel(const std::shared_ptr<ArrayBuffer>& data,
                                                     TensorflowModelDelegate delegate) override;
};

} // namespace margelo::nitro::nitrotflite
