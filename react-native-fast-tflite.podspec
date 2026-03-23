require "json"

package = JSON.parse(File.read(File.join(__dir__, "package.json")))

enableCoreMLDelegate = false
if defined?($EnableCoreMLDelegate)
  enableCoreMLDelegate = $EnableCoreMLDelegate
end
Pod::UI.puts "[TFLite] CoreML Delegate is set to #{enableCoreMLDelegate}! ($EnableCoreMLDelegate setting in Podfile)"

Pod::Spec.new do |s|
  s.name         = "react-native-fast-tflite"
  s.version      = package["version"]
  s.summary      = package["description"]
  s.homepage     = package["homepage"]
  s.license      = package["license"]
  s.authors      = package["author"]

  s.platforms    = { :ios => min_ios_version_supported, :visionos => 1.0 }
  s.module_name  = 'NitroTflite'
  s.source       = { :git => "https://github.com/mrousavy/react-native-fast-tflite.git", :tag => "#{s.version}" }

  s.source_files = [
    "ios/**/*.{swift}",
    "ios/**/*.{h,m,mm}",
    "cpp/**/*.{hpp,cpp,c,h}",
  ]

  s.pod_target_xcconfig = {
    'GCC_PREPROCESSOR_DEFINITIONS' => "$(inherited) FAST_TFLITE_ENABLE_CORE_ML=#{enableCoreMLDelegate}",
  }

  s.dependency "TensorFlowLiteC", "2.17.0"
  if enableCoreMLDelegate then
    s.dependency "TensorFlowLiteC/CoreML", "2.17.0"
  end

  load 'nitrogen/generated/ios/NitroTflite+autolinking.rb'
  add_nitrogen_files(s)

  s.dependency 'React-jsi'
  s.dependency 'React-callinvoker'
  install_modules_dependencies(s)
end
