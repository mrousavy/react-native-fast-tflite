import { ConfigPlugin, createRunOncePlugin } from '@expo/config-plugins'
import { ConfigProps } from './@types'
import { withCoreMLDelegate } from './withCoreMLDelegate'
// eslint-disable-next-line @typescript-eslint/no-unsafe-assignment, @typescript-eslint/no-var-requires
const pkg = require('../../../package.json') // from the lib directory, the package.json is three levels up

const withFastTFLite: ConfigPlugin<ConfigProps> = (config, props) => {
  if (props?.enableCoreMLDelegate) config = withCoreMLDelegate(config)

  return config
}

export default createRunOncePlugin(withFastTFLite, pkg.name, pkg.version)
