#include "brave/vendor/brave-ios/components/Passphrase.h"

#include <string>
#include <vector>

#include "brave/components/brave_sync/crypto/crypto.h"
#include "base/strings/sys_string_conversions.h"

@interface PassPhrase()
{

}
@end

@implementation PassPhrase
- (NSString *)getSyncCode {
	std::vector<uint8_t> seed = brave_sync::crypto::GetSeed();
    std::string sync_code = brave_sync::crypto::PassphraseFromBytes32(seed);
    return base::SysUTF8ToNSString(sync_code.c_str());
}
@end