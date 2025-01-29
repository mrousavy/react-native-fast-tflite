"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.default = void 0;
var _configPlugins = require("@expo/config-plugins");
var _withCoreMLDelegate = require("./withCoreMLDelegate");
// eslint-disable-next-line @typescript-eslint/no-unsafe-assignment, @typescript-eslint/no-var-requires
const pkg = require('../../../package.json'); // from the lib directory, the package.json is three levels up

const withFastTFLite = (config, props) => {
  if (props?.enableCoreMLDelegate) config = (0, _withCoreMLDelegate.withCoreMLDelegate)(config);
  return config;
};
var _default = (0, _configPlugins.createRunOncePlugin)(withFastTFLite, pkg.name, pkg.version);
exports.default = _default;
//# sourceMappingURL=withFastTFLite.js.map