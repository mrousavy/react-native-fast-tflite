#ifdef __cplusplus
#import "react-native-tflite.h"
#endif

#ifdef RCT_NEW_ARCH_ENABLED
#import "RNTfliteSpec.h"

@interface Tflite : NSObject <NativeTfliteSpec>
#else
#import <React/RCTBridgeModule.h>

@interface Tflite : NSObject <RCTBridgeModule>
#endif

@end
