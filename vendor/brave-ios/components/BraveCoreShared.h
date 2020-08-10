#import <Foundation/Foundation.h>


@interface BraveCoreShared: NSObject
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)shared;

- (NSString *)getUserAgent;
- (void)setUserAgentCallback:(NSString*(^)())userAgentCallback;
@end
