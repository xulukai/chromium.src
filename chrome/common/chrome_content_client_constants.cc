// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/common/chrome_content_client.h"

#if defined(GOOGLE_CHROME_BUILD)
const char* const ChromeContentClient::kPDFPluginName = "Chrome PDF Viewer";
#else
const char* const ChromeContentClient::kPDFPluginName = "Chromium PDF Viewer";
#endif
const char* const ChromeContentClient::kPDFPluginPath =
    "internal-pdf-viewer";
const char* const ChromeContentClient::kRemotingViewerPluginPath =
    "internal-remoting-viewer";
