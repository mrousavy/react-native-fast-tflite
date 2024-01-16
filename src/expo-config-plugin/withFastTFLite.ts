import {
  withPlugins,
  ConfigPlugin,
  createRunOncePlugin,
} from '@expo/config-plugins'
import { ConfigProps } from './@types'
import { withCoreMLDelegate } from './withCoreMLDelegate'
const pkg = require('../../package.json')

const withCamera: ConfigPlugin<ConfigProps> = (config, props) => {
  if (props.enableCoreMLDelegate) {
    config = withCoreMLDelegate(config)
  }

  return withPlugins(config, [])
}

export default createRunOncePlugin(withCamera, pkg.name, pkg.version)
