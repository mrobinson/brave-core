/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/logging_util.h"

#include <map>
#include <vector>

#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "brave/base/containers/utils.h"
#include "bat/ads/internal/url_util.h"

namespace ads {

namespace {

bool ShouldAllowHeader(const std::string& header) {
  const std::vector<std::string> allowed_headers {
    "digest",
    "signature",
    "accept",
    "content-type"
  };

  for (const auto& allowed_header : allowed_headers) {
    if (base::StartsWith(header, allowed_header,
        base::CompareCase::INSENSITIVE_ASCII)) {
      return true;
    }
  }

  return false;
}

std::string HeadersToString(
    const std::vector<std::string>& headers,
    const int indent = 4) {
  std::vector<std::string> formatted_headers;

  for (const auto& header : request->headers) {
    if (!ShouldAllowHeader(header)) {
      continue;
    }

    const std::string formatted_header =
        base::StringPrintf("%s%s", std::string(' ', indent), header.c_str());

    formatted_headers.push_back(formatted_header);
  }

  return base::JoinString(formatted_headers, "\n");
}

std::string HeadersToString(
    const std::map<std::string, std::string>& headers,
    const int indent = 4) {
  std::vector<std::string> formatted_headers;

  for (const auto& header : headers) {
    const std::string key = header.first;
    const std::string value = header.second;

    const std::string formatted_header = base::StringPrintf("%s%s: %s",
        std::string(' ', indent), key.c_str(), value.c_str());

    formatted_headers.push_back(formatted_header);
  }

  return base::JoinString(formatted_headers, "\n");
}

}  // namespace

std::string UrlRequestToString(
    const UrlRequestPtr& request) {
  std::string log = "URL Request:\n";

  log += base::StringPrintf("  URL: %s\n", request->url.c_str());

  if (!request->content.empty()) {
    log += base::StringPrintf("  Content: %s\n", request->content.c_str());
  }

  if (!request->content_type.empty()) {
    log += base::StringPrintf("  Content Type: %s\n",
        request->content_type.c_str());
  }

  std::ostringstream ss;
  ss << request->method;

  log += base::StringPrintf("  Method: %s", ss.str().c_str());

  return log;
}

std::string UrlRequestHeadersToString(
    const UrlRequestPtr& request) {
  std::string log = "  Headers:\n";

  if (!request->headers.empty()) {
    log += HeadersToString(request->headers);
  }

  return log;
}

std::string UrlResponseToString(
    const UrlResponse& response) {
  const std::string formatted_headers =
      HeadersToString(base::FlatMapToMap(response.headers));

  return base::StringPrintf("URL Response:\n  URL: %s\n  Response "
      "Status Code: %d\n  Response: %s", response.url.c_str(),
          response.status_code, response.body.c_str());
}

std::string UrlResponseHeadersToString(
    const UrlResponse& response) {
  const std::string formatted_headers =
      HeadersToString(base::FlatMapToMap(response.headers));

  return base::StringPrintf("  Headers:\n%s", formatted_headers.c_str());
}

}  // namespace ads
