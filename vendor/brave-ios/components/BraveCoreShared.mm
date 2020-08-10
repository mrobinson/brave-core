#include "brave/vendor/brave-ios/components/BraveCoreShared.h"
#include "brave/ios/web/brave_webmain.h"

#include "base/files/file_path.h"
#include "base/strings/utf_string_conversions.h"
#include "base/strings/sys_string_conversions.h"
#include "brave/ios/app/brave_main_delegate.h"
#include "brave/vendor/brave-ios/components/browser_state/chrome_browser_state.h"
#include "ios/web/public/init/web_main.h"

@interface BraveCoreShared()
@property (nonatomic, assign) NSString*(^fetchUserAgent)();
@end

@interface BraveCoreShared()
{
    std::unique_ptr<BraveMainDelegate> delegate_;
    std::unique_ptr<web::BraveWebMain> web_main_;
    std::unique_ptr<ChromeBrowserState> browser_state_;
}
@end

@implementation BraveCoreShared

+ (instancetype)shared {
    static BraveCoreShared* instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[BraveCoreShared alloc] init];
    });
    return instance;
}

- (instancetype)init {
    if ((self = [super init])) {
        delegate_.reset(new BraveMainDelegate());

        web::WebMainParams params(delegate_.get());
        params.register_exit_manager = false;
        
        web_main_ = std::make_unique<web::BraveWebMain>(std::move(params));

        browser_state_ = std::make_unique<ChromeBrowserState>(
            base::FilePath(kIOSChromeInitialBrowserState));
    }
    return self;
}

- (void)dealloc {
    browser_state_.reset();
    web_main_.reset();
    delegate_.reset();
}

- (NSString *)getUserAgent {
    if (self.fetchUserAgent) {
        return self.fetchUserAgent();
    }
    return @"";
}

- (void)setUserAgentCallback:(NSString*(^)())userAgentCallback {
    self.fetchUserAgent = userAgentCallback;
}

@end
