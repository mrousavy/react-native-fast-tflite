const { getDefaultConfig, mergeConfig } = require('@react-native/metro-config');
const path = require('path');
const pak = require('../package.json');

const root = path.resolve(__dirname, '..');
const modules = Object.keys({ ...pak.peerDependencies });

/** Always resolve from the example app — required when React Native Harness wraps Metro (it clears blockList). */
const RESOLVE_FROM_EXAMPLE = new Set([
  'react',
  'react-native',
  'react-native-nitro-modules',
]);

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
    resolveRequest(context, moduleName, platform) {
      if (RESOLVE_FROM_EXAMPLE.has(moduleName)) {
        return {
          type: 'sourceFile',
          filePath: require.resolve(moduleName, { paths: [__dirname] }),
        };
      }
      // Library sources live under ../src; Babel emits @babel/runtime/* imports that
      // must resolve from the example app (watchFolders alone does not fix this).
      if (
        moduleName === '@babel/runtime' ||
        moduleName.startsWith('@babel/runtime/')
      ) {
        return {
          type: 'sourceFile',
          filePath: require.resolve(moduleName, { paths: [__dirname] }),
        };
      }
      return context.resolveRequest(context, moduleName, platform);
    },
  },
};

module.exports = mergeConfig(getDefaultConfig(__dirname), config);
