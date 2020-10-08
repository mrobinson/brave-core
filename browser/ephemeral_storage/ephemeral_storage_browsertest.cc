/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <string>

#include "base/path_service.h"
#include "brave/browser/ephemeral_storage/ephemeral_storage_tab_helper.h"
#include "brave/common/brave_paths.h"
#include "brave/components/brave_shields/browser/brave_shields_util.h"
#include "brave/components/brave_shields/common/brave_shield_constants.h"
#include "chrome/browser/content_settings/host_content_settings_map_factory.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chrome/test/base/ui_test_utils.h"
#include "components/network_session_configurator/common/network_switches.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/browser_test.h"
#include "content/public/test/test_navigation_observer.h"
#include "net/dns/mock_host_resolver.h"
#include "net/test/embedded_test_server/default_handlers.h"
#include "third_party/blink/public/common/features.h"
#include "url/gurl.h"

using content::RenderFrameHost;
using content::WebContents;
using ephemeral_storage::EphemeralStorageTabHelper;
using net::test_server::EmbeddedTestServer;

namespace {

enum StorageType { Session, Local };

const char* ToString(StorageType storage_type) {
  switch (storage_type) {
    case StorageType::Session:
      return "session";
    case StorageType::Local:
      return "local";
  }
}

void SetStorageValueInFrame(RenderFrameHost* host,
                            std::string value,
                            StorageType storage_type) {
  std::string script =
      base::StringPrintf("%sStorage.setItem('storage_key', '%s');",
                         ToString(storage_type), value.c_str());
  ASSERT_TRUE(content::ExecuteScript(host, script));
}

content::EvalJsResult GetStorageValueInFrame(RenderFrameHost* host,
                                             StorageType storage_type) {
  std::string script = base::StringPrintf("%sStorage.getItem('storage_key');",
                                          ToString(storage_type));
  return content::EvalJs(host, script);
}

}  // namespace

class EphemeralStorageBrowserTest : public InProcessBrowserTest {
 public:
  EphemeralStorageBrowserTest()
      : https_server_(net::EmbeddedTestServer::TYPE_HTTPS) {
    scoped_feature_list_.InitAndEnableFeature(
        blink::features::kBraveEphemeralStorage);
  }

  void SetUpOnMainThread() override {
    InProcessBrowserTest::SetUpOnMainThread();

    host_resolver()->AddRule("*", "127.0.0.1");

    brave::RegisterPathProvider();
    base::FilePath test_data_dir;
    base::PathService::Get(brave::DIR_TEST_DATA, &test_data_dir);

    https_server_.ServeFilesFromDirectory(test_data_dir);
    https_server_.AddDefaultHandlers(GetChromeTestDataDir());
    content::SetupCrossSiteRedirector(&https_server_);

    ASSERT_TRUE(https_server_.Start());
    a_site_ephemeral_storage_url_ =
        https_server_.GetURL("a.com", "/ephemeral_storage.html");
    b_site_ephemeral_storage_url_ =
        https_server_.GetURL("b.com", "/ephemeral_storage.html");
    c_site_ephemeral_storage_url_ =
        https_server_.GetURL("c.com", "/ephemeral_storage.html");
    b_site_simple_url_ = https_server_.GetURL("b.com", "/simple.html");
    c_site_simple_url_ = https_server_.GetURL("c.com", "/simple.html");
  }

  void SetUpCommandLine(base::CommandLine* command_line) override {
    InProcessBrowserTest::SetUpCommandLine(command_line);

    // This is needed to load pages from "domain.com" without an interstitial.
    command_line->AppendSwitch(switches::kIgnoreCertificateErrors);
  }

  void AllowAllCookies() {
    auto* content_settings =
        HostContentSettingsMapFactory::GetForProfile(browser()->profile());
    brave_shields::SetCookieControlType(
        content_settings, brave_shields::ControlType::ALLOW, GURL());
  }

  void SetValuesInFrames(WebContents* web_contents,
                         std::string storage_value,
                         std::string cookie_value) {
    std::string frame_info;
    auto set_values_in_frame = [&](RenderFrameHost* frame) {
      SetStorageValueInFrame(frame,
                             storage_value + frame_info + ", local storage",
                             StorageType::Local);
      SetStorageValueInFrame(frame,
                             storage_value + frame_info + ", session storage",
                             StorageType::Session);
    };

    RenderFrameHost* main_frame = web_contents->GetMainFrame();
    frame_info = ", mainframe";
    set_values_in_frame(main_frame);
    frame_info = ", iframe";
    set_values_in_frame(content::ChildFrameAt(main_frame, 0));
    set_values_in_frame(content::ChildFrameAt(main_frame, 1));
  }

  struct ValuesFromFrame {
    content::EvalJsResult local_storage;
    content::EvalJsResult session_storage;
  };

  ValuesFromFrame GetValuesFromFrame(RenderFrameHost* frame) {
    return {
        GetStorageValueInFrame(frame, StorageType::Local),
        GetStorageValueInFrame(frame, StorageType::Session),
    };
  }

  struct ValuesFromFrames {
    ValuesFromFrame main_frame;
    ValuesFromFrame iframe_1;
    ValuesFromFrame iframe_2;
  };

  ValuesFromFrames GetValuesFromFrames(WebContents* web_contents) {
    RenderFrameHost* main_frame = web_contents->GetMainFrame();
    return ValuesFromFrames{
        GetValuesFromFrame(main_frame),
        GetValuesFromFrame(content::ChildFrameAt(main_frame, 0)),
        GetValuesFromFrame(content::ChildFrameAt(main_frame, 1)),
    };
  }

  WebContents* LoadURLInNewTab(GURL url) {
    ui_test_utils::AllBrowserTabAddedWaiter add_tab;
    ui_test_utils::NavigateToURLWithDisposition(
        browser(), url, WindowOpenDisposition::NEW_FOREGROUND_TAB,
        ui_test_utils::BROWSER_TEST_WAIT_FOR_LOAD_STOP);
    return add_tab.Wait();
  }

 protected:
  net::test_server::EmbeddedTestServer https_server_;
  base::test::ScopedFeatureList scoped_feature_list_;
  GURL a_site_ephemeral_storage_url_;
  GURL b_site_ephemeral_storage_url_;
  GURL c_site_ephemeral_storage_url_;
  GURL b_site_simple_url_;
  GURL c_site_simple_url_;

 private:
  DISALLOW_COPY_AND_ASSIGN(EphemeralStorageBrowserTest);
};

IN_PROC_BROWSER_TEST_F(EphemeralStorageBrowserTest, StorageIsPartitioned) {
  AllowAllCookies();

  WebContents* first_party_tab = LoadURLInNewTab(b_site_ephemeral_storage_url_);
  WebContents* site_a_tab1 = LoadURLInNewTab(a_site_ephemeral_storage_url_);
  WebContents* site_a_tab2 = LoadURLInNewTab(a_site_ephemeral_storage_url_);
  WebContents* site_c_tab = LoadURLInNewTab(c_site_ephemeral_storage_url_);

  EXPECT_EQ(browser()->tab_strip_model()->count(), 5);

  // We set a value in the page where all the frames are first-party.
  SetValuesInFrames(first_party_tab, "b.com - first party", "from=b.com");

  // The page this tab is loaded via a.com and has two b.com third-party
  // iframes. The third-party iframes should have ephemeral storage. That means
  // that their values should be shared by third-party b.com iframes loaded from
  // a.com.
  SetValuesInFrames(site_a_tab1, "a.com", "from=a.com");
  ValuesFromFrames site_a_tab1_values = GetValuesFromFrames(site_a_tab1);
  EXPECT_EQ("a.com, mainframe, local storage",
            site_a_tab1_values.main_frame.local_storage);
  EXPECT_EQ("a.com, iframe, local storage",
            site_a_tab1_values.iframe_1.local_storage);
  EXPECT_EQ("a.com, iframe, local storage",
            site_a_tab1_values.iframe_2.local_storage);

  EXPECT_EQ("a.com, mainframe, session storage",
            site_a_tab1_values.main_frame.session_storage);
  EXPECT_EQ("a.com, iframe, session storage",
            site_a_tab1_values.iframe_1.session_storage);
  EXPECT_EQ("a.com, iframe, session storage",
            site_a_tab1_values.iframe_2.session_storage);

  // The second tab is loaded on the same domain, so should see the same
  // storage for the third-party iframes.
  ValuesFromFrames site_a_tab2_values = GetValuesFromFrames(site_a_tab2);
  EXPECT_EQ("a.com, mainframe, local storage",
            site_a_tab2_values.main_frame.local_storage);
  EXPECT_EQ("a.com, iframe, local storage",
            site_a_tab2_values.iframe_1.local_storage);
  EXPECT_EQ("a.com, iframe, local storage",
            site_a_tab2_values.iframe_2.local_storage);

  EXPECT_EQ(nullptr, site_a_tab2_values.main_frame.session_storage);
  EXPECT_EQ(nullptr, site_a_tab2_values.iframe_1.session_storage);
  EXPECT_EQ(nullptr, site_a_tab2_values.iframe_2.session_storage);

  // The storage in the first-party iframes should still reflect the
  // original value that was written in the non-ephemeral storage area.
  // The value in mainframe is rewrited by iframes.
  ValuesFromFrames first_party_values = GetValuesFromFrames(first_party_tab);
  EXPECT_EQ("b.com - first party, iframe, local storage",
            first_party_values.main_frame.local_storage);
  EXPECT_EQ("b.com - first party, iframe, local storage",
            first_party_values.iframe_1.local_storage);
  EXPECT_EQ("b.com - first party, iframe, local storage",
            first_party_values.iframe_2.local_storage);

  EXPECT_EQ("b.com - first party, iframe, session storage",
            first_party_values.main_frame.session_storage);
  EXPECT_EQ("b.com - first party, iframe, session storage",
            first_party_values.iframe_1.session_storage);
  EXPECT_EQ("b.com - first party, iframe, session storage",
            first_party_values.iframe_2.session_storage);

  // Even though this page loads b.com iframes as third-party iframes, the TLD
  // differs, so it should get an entirely different ephemeral storage area.
  ValuesFromFrames site_c_tab_values = GetValuesFromFrames(site_c_tab);
  EXPECT_EQ(nullptr, site_c_tab_values.main_frame.local_storage);
  EXPECT_EQ(nullptr, site_c_tab_values.iframe_1.local_storage);
  EXPECT_EQ(nullptr, site_c_tab_values.iframe_2.local_storage);

  EXPECT_EQ(nullptr, site_c_tab_values.main_frame.session_storage);
  EXPECT_EQ(nullptr, site_c_tab_values.iframe_1.session_storage);
  EXPECT_EQ(nullptr, site_c_tab_values.iframe_2.session_storage);
}

IN_PROC_BROWSER_TEST_F(EphemeralStorageBrowserTest,
                       NavigatingClearsEphemeralStorage) {
  AllowAllCookies();

  ui_test_utils::NavigateToURL(browser(), a_site_ephemeral_storage_url_);
  auto* web_contents = browser()->tab_strip_model()->GetActiveWebContents();

  SetValuesInFrames(web_contents, "a.com value", "from=a.com");

  ValuesFromFrames values_before = GetValuesFromFrames(web_contents);
  EXPECT_EQ("a.com value, mainframe, local storage",
            values_before.main_frame.local_storage);
  EXPECT_EQ("a.com value, iframe, local storage",
            values_before.iframe_1.local_storage);
  EXPECT_EQ("a.com value, iframe, local storage",
            values_before.iframe_2.local_storage);

  EXPECT_EQ("a.com value, mainframe, session storage",
            values_before.main_frame.session_storage);
  EXPECT_EQ("a.com value, iframe, session storage",
            values_before.iframe_1.session_storage);
  EXPECT_EQ("a.com value, iframe, session storage",
            values_before.iframe_2.session_storage);

  // Navigate away and then navigate back to the original site.
  ui_test_utils::NavigateToURL(browser(), b_site_ephemeral_storage_url_);
  ui_test_utils::NavigateToURL(browser(), a_site_ephemeral_storage_url_);

  ValuesFromFrames values_after = GetValuesFromFrames(web_contents);
  EXPECT_EQ("a.com value, mainframe, local storage",
            values_after.main_frame.local_storage);
  EXPECT_EQ(nullptr, values_after.iframe_1.local_storage);
  EXPECT_EQ(nullptr, values_after.iframe_2.local_storage);

  EXPECT_EQ("a.com value, mainframe, session storage",
            values_after.main_frame.session_storage);
  EXPECT_EQ(nullptr, values_after.iframe_1.session_storage);
  EXPECT_EQ(nullptr, values_after.iframe_2.session_storage);
}

IN_PROC_BROWSER_TEST_F(EphemeralStorageBrowserTest, TwoTabsAndOneNavigateAway) {
  AllowAllCookies();

  WebContents* site_a_tab1 = LoadURLInNewTab(a_site_ephemeral_storage_url_);
  WebContents* site_a_tab2 = LoadURLInNewTab(a_site_ephemeral_storage_url_);

  SetValuesInFrames(site_a_tab2, "a.com value", "from=a.com");

  ValuesFromFrames values_tab2 = GetValuesFromFrames(site_a_tab2);
  EXPECT_EQ("a.com value, mainframe, local storage",
            values_tab2.main_frame.local_storage);
  EXPECT_EQ("a.com value, iframe, local storage",
            values_tab2.iframe_1.local_storage);
  EXPECT_EQ("a.com value, iframe, local storage",
            values_tab2.iframe_2.local_storage);

  EXPECT_EQ("a.com value, mainframe, session storage",
            values_tab2.main_frame.session_storage);
  EXPECT_EQ("a.com value, iframe, session storage",
            values_tab2.iframe_1.session_storage);
  EXPECT_EQ("a.com value, iframe, session storage",
            values_tab2.iframe_2.session_storage);

  // Tab2 navigates away.
  ui_test_utils::NavigateToURL(browser(), b_site_ephemeral_storage_url_);
  // Tab1 should still keep ephemeral storage.
  ValuesFromFrames values_tb1 = GetValuesFromFrames(site_a_tab1);
  EXPECT_EQ("a.com value, mainframe, local storage",
            values_tb1.main_frame.local_storage);
  EXPECT_EQ("a.com value, iframe, local storage",
            values_tb1.iframe_1.local_storage);
  EXPECT_EQ("a.com value, iframe, local storage",
            values_tb1.iframe_2.local_storage);

  EXPECT_EQ(nullptr, values_tb1.main_frame.session_storage);
  EXPECT_EQ(nullptr, values_tb1.iframe_1.session_storage);
  EXPECT_EQ(nullptr, values_tb1.iframe_2.session_storage);
}

IN_PROC_BROWSER_TEST_F(EphemeralStorageBrowserTest, IframeNavigateAwayAndBack) {
  AllowAllCookies();

  WebContents* web_contents = LoadURLInNewTab(a_site_ephemeral_storage_url_);

  SetValuesInFrames(web_contents, "a.com value", "from=a.com");

  ValuesFromFrames values_before = GetValuesFromFrames(web_contents);
  EXPECT_EQ("a.com value, mainframe, local storage",
            values_before.main_frame.local_storage);
  EXPECT_EQ("a.com value, iframe, local storage",
            values_before.iframe_1.local_storage);
  EXPECT_EQ("a.com value, iframe, local storage",
            values_before.iframe_2.local_storage);

  EXPECT_EQ("a.com value, mainframe, session storage",
            values_before.main_frame.session_storage);
  EXPECT_EQ("a.com value, iframe, session storage",
            values_before.iframe_1.session_storage);
  EXPECT_EQ("a.com value, iframe, session storage",
            values_before.iframe_2.session_storage);

  // Iframe_a navigate away and set a new value.
  content::NavigateIframeToURL(web_contents, "third_party_iframe_a",
                               c_site_simple_url_);
  content::RenderFrameHost* iframe_a =
      content::ChildFrameAt(web_contents->GetMainFrame(), 0);
  SetStorageValueInFrame(iframe_a, "c.com value, iframe, session storage",
                         StorageType::Session);
  SetStorageValueInFrame(iframe_a, "c.com value, iframe, local storage",
                         StorageType::Local);

  ValuesFromFrames new_iframe_value = GetValuesFromFrames(web_contents);
  EXPECT_EQ("a.com value, mainframe, local storage",
            new_iframe_value.main_frame.local_storage);
  EXPECT_EQ("c.com value, iframe, local storage",
            new_iframe_value.iframe_1.local_storage);
  EXPECT_EQ("a.com value, iframe, local storage",
            new_iframe_value.iframe_2.local_storage);

  EXPECT_EQ("a.com value, mainframe, session storage",
            new_iframe_value.main_frame.session_storage);
  EXPECT_EQ("c.com value, iframe, session storage",
            new_iframe_value.iframe_1.session_storage);
  EXPECT_EQ("a.com value, iframe, session storage",
            new_iframe_value.iframe_2.session_storage);

  // All iframes navigate away and back.
  content::NavigateIframeToURL(web_contents, "third_party_iframe_b",
                               c_site_simple_url_);
  content::NavigateIframeToURL(web_contents, "third_party_iframe_a",
                               b_site_simple_url_);
  content::NavigateIframeToURL(web_contents, "third_party_iframe_b",
                               b_site_simple_url_);

  // Main frame is not changed, the ephemeral values of b_site_simple_url_
  // should be kept.
  ValuesFromFrames values_after = GetValuesFromFrames(web_contents);
  EXPECT_EQ("a.com value, mainframe, local storage",
            values_after.main_frame.local_storage);
  EXPECT_EQ("a.com value, iframe, local storage",
            values_after.iframe_1.local_storage);
  EXPECT_EQ("a.com value, iframe, local storage",
            values_after.iframe_2.local_storage);

  EXPECT_EQ("a.com value, mainframe, session storage",
            values_after.main_frame.session_storage);
  EXPECT_EQ("a.com value, iframe, session storage",
            values_after.iframe_1.session_storage);
  EXPECT_EQ("a.com value, iframe, session storage",
            values_after.iframe_2.session_storage);
}

IN_PROC_BROWSER_TEST_F(EphemeralStorageBrowserTest,
                       ClosingTabClearsEphemeralStorage) {
  AllowAllCookies();

  WebContents* site_a_tab = LoadURLInNewTab(a_site_ephemeral_storage_url_);
  EXPECT_EQ(browser()->tab_strip_model()->count(), 2);

  SetValuesInFrames(site_a_tab, "a.com value", "from=a.com");

  ValuesFromFrames values_before = GetValuesFromFrames(site_a_tab);
  EXPECT_EQ("a.com value, mainframe, local storage",
            values_before.main_frame.local_storage);
  EXPECT_EQ("a.com value, iframe, local storage",
            values_before.iframe_1.local_storage);
  EXPECT_EQ("a.com value, iframe, local storage",
            values_before.iframe_2.local_storage);

  EXPECT_EQ("a.com value, mainframe, session storage",
            values_before.main_frame.session_storage);
  EXPECT_EQ("a.com value, iframe, session storage",
            values_before.iframe_1.session_storage);
  EXPECT_EQ("a.com value, iframe, session storage",
            values_before.iframe_2.session_storage);

  // Close the new tab which we set ephemeral storage value in. This should
  // clear the ephemeral storage since this is the last tab which has a.com as
  // an eTLD.
  int tab_index =
      browser()->tab_strip_model()->GetIndexOfWebContents(site_a_tab);
  bool was_closed = browser()->tab_strip_model()->CloseWebContentsAt(
      tab_index, TabStripModel::CloseTypes::CLOSE_NONE);
  EXPECT_TRUE(was_closed);

  // Navigate the main tab to the same site.
  ui_test_utils::NavigateToURL(browser(), a_site_ephemeral_storage_url_);
  auto* web_contents = browser()->tab_strip_model()->GetActiveWebContents();

  // Closing the tab earlier should have cleared the ephemeral storage area.
  ValuesFromFrames values_after = GetValuesFromFrames(web_contents);
  EXPECT_EQ("a.com value, mainframe, local storage",
            values_after.main_frame.local_storage);
  EXPECT_EQ(nullptr, values_after.iframe_1.local_storage);
  EXPECT_EQ(nullptr, values_after.iframe_2.local_storage);

  EXPECT_EQ(nullptr, values_after.main_frame.session_storage);
  EXPECT_EQ(nullptr, values_after.iframe_1.session_storage);
  EXPECT_EQ(nullptr, values_after.iframe_2.session_storage);
}

IN_PROC_BROWSER_TEST_F(EphemeralStorageBrowserTest,
                       BrowserSideEphemeralStorageExistenceCheck) {
  AllowAllCookies();

  ui_test_utils::NavigateToURL(browser(), b_site_simple_url_);
  auto* web_contents = browser()->tab_strip_model()->GetActiveWebContents();

  EXPECT_EQ(true,
            EphemeralStorageTabHelper::URLHasEphemeralLocalStorageForTesting(
                b_site_simple_url_));
  EXPECT_EQ(true,
            EphemeralStorageTabHelper::
                WebContentsHasEphemeralSessionStorageForTesting(web_contents));

  ui_test_utils::NavigateToURL(browser(), a_site_ephemeral_storage_url_);

  EXPECT_EQ(false,
            EphemeralStorageTabHelper::URLHasEphemeralLocalStorageForTesting(
                b_site_simple_url_));
  EXPECT_EQ(true,
            EphemeralStorageTabHelper::URLHasEphemeralLocalStorageForTesting(
                a_site_ephemeral_storage_url_));
  EXPECT_EQ(true,
            EphemeralStorageTabHelper::
                WebContentsHasEphemeralSessionStorageForTesting(web_contents));
}
