// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_APP_LIST_APP_LIST_PREFS_FACTORY_H_
#define CHROME_BROWSER_UI_APP_LIST_APP_LIST_PREFS_FACTORY_H_

#include "base/memory/scoped_ptr.h"
#include "base/memory/singleton.h"
#include "components/keyed_service/content/browser_context_keyed_service_factory.h"

namespace app_list {

class AppListPrefs;

class AppListPrefsFactory : public BrowserContextKeyedServiceFactory {
 public:
  static AppListPrefs* GetForBrowserContext(content::BrowserContext* context);

  static AppListPrefsFactory* GetInstance();

  void SetInstanceForTesting(content::BrowserContext* context,
                             AppListPrefs* prefs);

 private:
  friend struct DefaultSingletonTraits<AppListPrefsFactory>;

  AppListPrefsFactory();
  virtual ~AppListPrefsFactory();

  virtual KeyedService* BuildServiceInstanceFor(
      content::BrowserContext* context) const OVERRIDE;
  virtual content::BrowserContext* GetBrowserContextToUse(
      content::BrowserContext* context) const OVERRIDE;
};

}  // namespace app_list

#endif  // CHROME_BROWSER_UI_APP_LIST_APP_LIST_PREFS_FACTORY_H_
