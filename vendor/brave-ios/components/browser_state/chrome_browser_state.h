//
//  chrome_browser_state.hpp
//  base/third_party/double_conversion:double_conversion
//
//  Created by brandon on 2020-05-07.
//

#ifndef chrome_browser_state_hpp
#define chrome_browser_state_hpp

#include "base/callback_forward.h"
#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "ios/web/public/browser_state.h"

class PrefService;

namespace base {
class SequencedTaskRunner;
}

namespace sync_preferences {
class PrefServiceSyncable;
}

namespace user_prefs {
class PrefRegistrySyncable;
}

class ChromeBrowserState : public web::BrowserState {
 public:
  ~ChromeBrowserState() override;

  static ChromeBrowserState* FromBrowserState(BrowserState* browser_state);

  net::URLRequestContextGetter* GetRequestContext() override;

  PrefService* GetPrefs();
  scoped_refptr<base::SequencedTaskRunner> GetIOTaskRunner();

 protected:
  explicit ChromeBrowserState(
      scoped_refptr<base::SequencedTaskRunner> io_task_runner,
      const base::FilePath& path);

 private:
  net::URLRequestContextGetter* CreateRequestContext();

  scoped_refptr<base::SequencedTaskRunner> io_task_runner_;
  base::FilePath state_path_;
  scoped_refptr<user_prefs::PrefRegistrySyncable> pref_registry_;
  std::unique_ptr<sync_preferences::PrefServiceSyncable> prefs_;
  scoped_refptr<net::URLRequestContextGetter> request_context_getter_;

  DISALLOW_COPY_AND_ASSIGN(ChromeBrowserState);
};

#endif /* chrome_browser_state_hpp */
