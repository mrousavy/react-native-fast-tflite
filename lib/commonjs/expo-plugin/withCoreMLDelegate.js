"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.withCoreMLDelegate = void 0;
var _fs = _interopRequireDefault(require("fs"));
var _path = _interopRequireDefault(require("path"));
var _configPlugins = require("@expo/config-plugins");
var _commonCodeMod = require("@expo/config-plugins/build/utils/commonCodeMod");
function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }
const withCoreMLDelegate = config => (0, _configPlugins.withPlugins)(config, [
// Add $EnableCoreMLDelegate = true to the top of the Podfile
[_configPlugins.withDangerousMod, ['ios', currentConfig => {
  const podfilePath = _path.default.join(currentConfig.modRequest.platformProjectRoot, 'Podfile');
  const prevPodfileContent = _fs.default.readFileSync(podfilePath, 'utf-8');
  const newPodfileContent = (0, _commonCodeMod.insertContentsAtOffset)(prevPodfileContent, '$EnableCoreMLDelegate=true\n', 0);
  _fs.default.writeFileSync(podfilePath, newPodfileContent);
  return currentConfig;
}]],
// Add the CoreML framework to the Xcode project
[_configPlugins.withXcodeProject, currentConfig => {
  // `XcodeProject` isn't correctly exported from @expo/config-plugins, so TypeScript sees it as `any`
  // see https://github.com/expo/expo/issues/16470
  // eslint-disable-next-line @typescript-eslint/no-unsafe-assignment
  const xcodeProject = currentConfig.modResults;
  xcodeProject.addFramework('CoreML.framework');
  return currentConfig;
}]]);
exports.withCoreMLDelegate = withCoreMLDelegate;
//# sourceMappingURL=withCoreMLDelegate.js.map