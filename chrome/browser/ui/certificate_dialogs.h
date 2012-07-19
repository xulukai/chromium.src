// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_CERTIFICATE_DIALOGS_H_
#define CHROME_BROWSER_UI_CERTIFICATE_DIALOGS_H_

#include "chrome/browser/ui/select_file_dialog.h"
#include "net/base/x509_certificate.h"

namespace content {
class WebContents;
}

void ShowCertSelectFileDialog(SelectFileDialog* select_file_dialog,
                              SelectFileDialog::Type type,
                              const FilePath& suggested_path,
                              gfx::NativeWindow parent,
                              void* params);

void ShowCertExportDialog(content::WebContents* web_contents,
                          gfx::NativeWindow parent,
                          net::X509Certificate::OSCertHandle cert);

#endif  // CHROME_BROWSER_UI_CERTIFICATE_DIALOGS_H_
