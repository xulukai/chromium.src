// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_PROFILES_PROFILE_INFO_CACHE_UNITTEST_H_
#define CHROME_BROWSER_PROFILES_PROFILE_INFO_CACHE_UNITTEST_H_

#include <set>

#include "base/message_loop.h"
#include "chrome/browser/profiles/profile_info_cache_observer.h"
#include "chrome/test/base/testing_profile_manager.h"
#include "content/public/test/test_browser_thread.h"
#include "testing/gtest/include/gtest/gtest.h"

class FilePath;
class ProfileInfoCache;

// Class used to test that ProfileInfoCache does not try to access any
// unexpected profile names.
class ProfileNameVerifierObserver : public ProfileInfoCacheObserver {
 public:
  explicit ProfileNameVerifierObserver(
      TestingProfileManager* testing_profile_manager);
  virtual ~ProfileNameVerifierObserver();

  // ProfileInfoCacheObserver overrides:
  virtual void OnProfileAdded(const FilePath& profile_path) OVERRIDE;
  virtual void OnProfileWillBeRemoved(
      const FilePath& profile_path) OVERRIDE;
  virtual void OnProfileWasRemoved(
      const FilePath& profile_path,
      const string16& profile_name) OVERRIDE;
  virtual void OnProfileNameChanged(
      const FilePath& profile_path,
      const string16& old_profile_name) OVERRIDE;
  virtual void OnProfileAvatarChanged(const FilePath& profile_path) OVERRIDE;

 private:
  ProfileInfoCache* GetCache();
  std::set<string16> profile_names_;
  TestingProfileManager* testing_profile_manager_;
  DISALLOW_COPY_AND_ASSIGN(ProfileNameVerifierObserver);
};

class ProfileInfoCacheTest : public testing::Test {
 protected:
  ProfileInfoCacheTest();
  virtual ~ProfileInfoCacheTest();

  virtual void SetUp() OVERRIDE;
  virtual void TearDown() OVERRIDE;

  ProfileInfoCache* GetCache();
  FilePath GetProfilePath(const std::string& base_name);
  void ResetCache();

 protected:
  TestingProfileManager testing_profile_manager_;

 private:
  MessageLoopForUI ui_loop_;
  content::TestBrowserThread ui_thread_;
  content::TestBrowserThread file_thread_;
  ProfileNameVerifierObserver name_observer_;
};

#endif  // CHROME_BROWSER_PROFILES_PROFILE_INFO_CACHE_UNITTEST_H_
