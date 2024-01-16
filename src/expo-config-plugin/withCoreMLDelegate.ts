import { ConfigPlugin, withPodfileProperties } from '@expo/config-plugins'

export const withCoreMLDelegate: ConfigPlugin = (c) => {
  return withPodfileProperties(c, (config) => {
    // TODO: Implement Podfile writing
    config.ios = config.ios
    return config
  })
}
