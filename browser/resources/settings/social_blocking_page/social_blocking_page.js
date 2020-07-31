/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

/**
 * 'settings-social-blocking-page' is the settings page containing brave's
 * social blocking options
 */
Polymer({
  is: 'settings-social-blocking-page',

  properties: {},

  /** @private {?settings.SocialBlockingPageProxy} */
  browserProxy_: null,

  /** @override */
  created: function() {
    this.browserProxy_ = settings.SocialBlockingPageProxyImpl.getInstance();
  },

  /** @override */
  ready: function() {
    this.setGoogleLoginEnabled_ = this.setGoogleLoginEnabled_.bind(this)
    this.restartBrowser_ = this.restartBrowser_.bind(this)
  },

  setGoogleLoginEnabled_: function() {
    this.browserProxy_.setGoogleLoginEnabled(this.$.googleLoginControlType.checked)
  },

  shouldShowRestart_: function(enabled) {
    return enabled != this.browserProxy_.wasGoogleLoginEnabledAtStartup();
  },

  restartBrowser_: function(e) {
    e.stopPropagation();
    window.open("chrome://restart", "_self");
  },
});
