const { getDefaultConfig, mergeConfig } = require('@react-native/metro-config');
const path = require('path');
const pak = require('../package.json');

const root = path.resolve(__dirname, '..');
const modules = Object.keys({ ...pak.peerDependencies });

/**
 * Metro configuration
 * https://reactnative.dev/docs/metro
 *
 * @type {import('@react-native/metro-config').MetroConfig}
 */
const config = {
  watchFolders: [root],

  resolver: {
    assetExts: ['tflite', 'png', 'jpg'],
    blockList: modules.map(
      (m) =>
        new RegExp(`^${path.join(root, 'node_modules', m).replace(/[.*+?^${}()|[\]\\]/g, '\\$&')}\\/.*$`)
    ),
    extraNodeModules: modules.reduce((acc, name) => {
      acc[name] = path.join(__dirname, 'node_modules', name);
      return acc;
    }, {}),
  },
};

module.exports = mergeConfig(getDefaultConfig(__dirname), config);
