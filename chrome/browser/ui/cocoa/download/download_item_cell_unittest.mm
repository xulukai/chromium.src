// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/mac/scoped_nsobject.h"
#include "chrome/browser/download/download_item_model.h"
#import "chrome/browser/ui/cocoa/cocoa_test_helper.h"
#import "chrome/browser/ui/cocoa/download/download_item_cell.h"
#include "content/public/test/mock_download_item.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/platform_test.h"

using ::testing::Return;



using ::testing::_;
using ::testing::ReturnRefOfCopy;


class DownloadItemCellTest : public CocoaTest {
 public:
  DownloadItemCellTest() : CocoaTest() {
  }

  void SetUp() override {
    CocoaTest::SetUp();
    button_.reset([[NSButton alloc] initWithFrame:NSMakeRect(0, 0, 100, 100)]);
    [[test_window() contentView] addSubview:button_];
    cell_.reset([[DownloadItemCell alloc] initTextCell:@""]);
    [button_ setCell:cell_];
  }

 protected:
  base::scoped_nsobject<DownloadItemCell> cell_;
  base::scoped_nsobject<NSButton> button_;

 private:
  DISALLOW_COPY_AND_ASSIGN(DownloadItemCellTest);
};

TEST_F(DownloadItemCellTest, ToggleStatusText) {
  EXPECT_FALSE([cell_ isStatusTextVisible]);
  EXPECT_FLOAT_EQ(0.0, [cell_ statusTextAlpha]);

  [cell_ showSecondaryTitle];
  [cell_ skipVisibilityAnimation];
  EXPECT_TRUE([cell_ isStatusTextVisible]);
  EXPECT_FLOAT_EQ(1.0, [cell_ statusTextAlpha]);

  [cell_ hideSecondaryTitle];
  [cell_ skipVisibilityAnimation];
  EXPECT_FALSE([cell_ isStatusTextVisible]);
  EXPECT_FLOAT_EQ(0.0, [cell_ statusTextAlpha]);
}

TEST_F(DownloadItemCellTest, IndeterminateProgress) {
  testing::NiceMock<content::MockDownloadItem> item;
  ON_CALL(item, IsPaused()).WillByDefault(Return(false));
  ON_CALL(item, PercentComplete()).WillByDefault(Return(-1));
  ON_CALL(item, GetState())
      .WillByDefault(Return(content::DownloadItem::IN_PROGRESS));
  ON_CALL(item, GetFileNameToReportUser())
      .WillByDefault(Return(base::FilePath("foo.bar")));
  DownloadItemModel model(&item);

  // Set indeterminate state.
  EXPECT_FALSE([cell_ indeterminateProgressTimer]);
  [cell_ setStateFromDownload:&model];
  EXPECT_TRUE([cell_ indeterminateProgressTimer]);

  // Draw.
  [button_ lockFocus];
  [cell_ drawWithFrame:[button_ bounds]
                inView:button_];
  [button_ unlockFocus];

  // Unset indeterminate state.
  ON_CALL(item, PercentComplete()).WillByDefault(Return(0));
  [cell_ setStateFromDownload:&model];
  EXPECT_FALSE([cell_ indeterminateProgressTimer]);
}
