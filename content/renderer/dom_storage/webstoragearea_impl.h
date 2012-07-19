// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_RENDERER_DOM_STORAGE_WEBSTORAGEAREA_IMPL_H_
#define CONTENT_RENDERER_DOM_STORAGE_WEBSTORAGEAREA_IMPL_H_

#include "base/basictypes.h"
#include "base/memory/ref_counted.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebStorageArea.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/platform/WebString.h"

class GURL;

namespace dom_storage {
class DomStorageCachedArea;
}

class WebStorageAreaImpl : public WebKit::WebStorageArea {
 public:
  static WebStorageAreaImpl* FromConnectionId(int id);

  WebStorageAreaImpl(int64 namespace_id, const GURL& origin);
  virtual ~WebStorageAreaImpl();

  // See WebStorageArea.h for documentation on these functions.
  virtual unsigned length();
  virtual WebKit::WebString key(unsigned index);
  virtual WebKit::WebString getItem(const WebKit::WebString& key);
  virtual void setItem(
      const WebKit::WebString& key, const WebKit::WebString& value,
      const WebKit::WebURL& page_url, WebStorageArea::Result& result);
  virtual void removeItem(
      const WebKit::WebString& key, const WebKit::WebURL& page_url);
  virtual void clear(const WebKit::WebURL& url);

 private:
  int connection_id_;
  scoped_refptr<dom_storage::DomStorageCachedArea> cached_area_;
};

#endif  // CONTENT_RENDERER_DOM_STORAGE_WEBSTORAGEAREA_IMPL_H_
