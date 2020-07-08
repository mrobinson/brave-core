#ifndef BRAVE_IOS_BROWSER_BROWSER_STATE_BROWSER_STATE_MANAGER_H_
#define BRAVE_IOS_BROWSER_BROWSER_STATE_BROWSER_STATE_MANAGER_H_

#include <memory>
#include "base/macros.h"

class ChromeBrowserState;

class BrowserStateManager {
  public:
    static BrowserStateManager& instance();
    ~BrowserStateManager();

    ChromeBrowserState* getBrowserState();

  private:
    BrowserStateManager();
    DISALLOW_COPY_AND_ASSIGN(BrowserStateManager);

    std::unique_ptr<ChromeBrowserState> browser_state_;
};

#endif  // BRAVE_IOS_BROWSER_BROWSER_STATE_BROWSER_STATE_MANAGER_H_
