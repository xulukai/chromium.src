// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/app_list/views/app_list_view.h"

#include "base/command_line.h"
#include "base/run_loop.h"
#include "base/strings/string_util.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/app_list/app_list_switches.h"
#include "ui/app_list/pagination_model.h"
#include "ui/app_list/test/app_list_test_model.h"
#include "ui/app_list/test/app_list_test_view_delegate.h"
#include "ui/app_list/views/app_list_folder_view.h"
#include "ui/app_list/views/app_list_main_view.h"
#include "ui/app_list/views/apps_container_view.h"
#include "ui/app_list/views/apps_grid_view.h"
#include "ui/app_list/views/contents_view.h"
#include "ui/app_list/views/search_box_view.h"
#include "ui/app_list/views/start_page_view.h"
#include "ui/app_list/views/test/apps_grid_view_test_api.h"
#include "ui/app_list/views/tile_item_view.h"
#include "ui/aura/test/aura_test_base.h"
#include "ui/aura/window.h"
#include "ui/views/test/views_test_base.h"
#include "ui/views/views_delegate.h"
#include "ui/views/widget/desktop_aura/desktop_native_widget_aura.h"

namespace app_list {
namespace test {

namespace {

enum TestType {
  TEST_TYPE_START = 0,
  NORMAL = TEST_TYPE_START,
  LANDSCAPE,
  EXPERIMENTAL,
  TEST_TYPE_END,
};

bool IsViewAtOrigin(views::View* view) {
  return view->bounds().origin().IsOrigin();
}

size_t GetVisibleTileItemViews(const std::vector<TileItemView*>& tiles) {
  size_t count = 0;
  for (std::vector<TileItemView*>::const_iterator it = tiles.begin();
       it != tiles.end();
       ++it) {
    if ((*it)->visible())
      count++;
  }
  return count;
}

// Choose a set that is 3 regular app list pages and 2 landscape app list pages.
const int kInitialItems = 34;

// Allows the same tests to run with different contexts: either an Ash-style
// root window or a desktop window tree host.
class AppListViewTestContext {
 public:
  AppListViewTestContext(int test_type, aura::Window* parent);
  ~AppListViewTestContext();

  // Test displaying the app list and performs a standard set of checks on its
  // top level views. Then closes the window.
  void RunDisplayTest();

  // Hides and reshows the app list with a folder open, expecting the main grid
  // view to be shown.
  void RunReshowWithOpenFolderTest();

  // Tests displaying of the experimental app list and shows the start page.
  void RunStartPageTest();

  // Tests that changing the App List profile.
  void RunProfileChangeTest();

  // A standard set of checks on a view, e.g., ensuring it is drawn and visible.
  static void CheckView(views::View* subview);

  // Invoked when the Widget is closing, and the view it contains is about to
  // be torn down. This only occurs in a run loop and will be used as a signal
  // to quit.
  void NativeWidgetClosing() {
    view_ = NULL;
    run_loop_->Quit();
  }

  // Whether the experimental "landscape" app launcher UI is being tested.
  bool is_landscape() const {
    return test_type_ == LANDSCAPE || test_type_ == EXPERIMENTAL;
  }

 private:
  // Shows the app list and waits until a paint occurs.
  void Show();

  // Closes the app list. This sets |view_| to NULL.
  void Close();

  // Gets the PaginationModel owned by |view_|.
  PaginationModel* GetPaginationModel();

  const TestType test_type_;
  scoped_ptr<base::RunLoop> run_loop_;
  app_list::AppListView* view_;  // Owned by native widget.
  app_list::test::AppListTestViewDelegate* delegate_;  // Owned by |view_|;

  DISALLOW_COPY_AND_ASSIGN(AppListViewTestContext);
};

// Extend the regular AppListTestViewDelegate to communicate back to the test
// context. Note the test context doesn't simply inherit this, because the
// delegate is owned by the view.
class UnitTestViewDelegate : public app_list::test::AppListTestViewDelegate {
 public:
  UnitTestViewDelegate(AppListViewTestContext* parent) : parent_(parent) {}

  // Overridden from app_list::AppListViewDelegate:
  virtual bool ShouldCenterWindow() const OVERRIDE {
    return app_list::switches::IsCenteredAppListEnabled();
  }

  // Overridden from app_list::test::AppListTestViewDelegate:
  virtual void ViewClosing() OVERRIDE { parent_->NativeWidgetClosing(); }

 private:
  AppListViewTestContext* parent_;

  DISALLOW_COPY_AND_ASSIGN(UnitTestViewDelegate);
};

AppListViewTestContext::AppListViewTestContext(int test_type,
                                               aura::Window* parent)
    : test_type_(static_cast<TestType>(test_type)) {
  switch (test_type_) {
    case NORMAL:
      break;
    case LANDSCAPE:
      base::CommandLine::ForCurrentProcess()->AppendSwitch(
          switches::kEnableCenteredAppList);
      break;
    case EXPERIMENTAL:
      base::CommandLine::ForCurrentProcess()->AppendSwitch(
          switches::kEnableExperimentalAppList);
      break;
    default:
      NOTREACHED();
      break;
  }

  delegate_ = new UnitTestViewDelegate(this);
  view_ = new app_list::AppListView(delegate_);

  // Initialize centered around a point that ensures the window is wholly shown.
  view_->InitAsBubbleAtFixedLocation(parent,
                                     0,
                                     gfx::Point(300, 300),
                                     views::BubbleBorder::FLOAT,
                                     false /* border_accepts_events */);
}

AppListViewTestContext::~AppListViewTestContext() {
  // The view observes the PaginationModel which is about to get destroyed, so
  // if the view is not already deleted by the time this destructor is called,
  // there will be problems.
  EXPECT_FALSE(view_);
}

// static
void AppListViewTestContext::CheckView(views::View* subview) {
  ASSERT_TRUE(subview);
  EXPECT_TRUE(subview->parent());
  EXPECT_TRUE(subview->visible());
  EXPECT_TRUE(subview->IsDrawn());
}

void AppListViewTestContext::Show() {
  view_->GetWidget()->Show();
  run_loop_.reset(new base::RunLoop);
  view_->SetNextPaintCallback(run_loop_->QuitClosure());
  run_loop_->Run();

  EXPECT_TRUE(view_->GetWidget()->IsVisible());
}

void AppListViewTestContext::Close() {
  view_->GetWidget()->Close();
  run_loop_.reset(new base::RunLoop);
  run_loop_->Run();

  // |view_| should have been deleted and set to NULL via ViewClosing().
  EXPECT_FALSE(view_);
}

PaginationModel* AppListViewTestContext::GetPaginationModel() {
  return view_->GetAppsPaginationModel();
}

void AppListViewTestContext::RunDisplayTest() {
  EXPECT_FALSE(view_->GetWidget()->IsVisible());
  EXPECT_EQ(-1, GetPaginationModel()->total_pages());
  delegate_->GetTestModel()->PopulateApps(kInitialItems);

  Show();
  if (is_landscape())
    EXPECT_EQ(2, GetPaginationModel()->total_pages());
  else
    EXPECT_EQ(3, GetPaginationModel()->total_pages());
  EXPECT_EQ(0, GetPaginationModel()->selected_page());

  // Checks on the main view.
  AppListMainView* main_view = view_->app_list_main_view();
  EXPECT_NO_FATAL_FAILURE(CheckView(main_view));
  EXPECT_NO_FATAL_FAILURE(CheckView(main_view->search_box_view()));
  EXPECT_NO_FATAL_FAILURE(CheckView(main_view->contents_view()));

  Close();
}

void AppListViewTestContext::RunReshowWithOpenFolderTest() {
  EXPECT_FALSE(view_->GetWidget()->IsVisible());
  EXPECT_EQ(-1, GetPaginationModel()->total_pages());

  AppListTestModel* model = delegate_->GetTestModel();
  model->PopulateApps(kInitialItems);
  const std::string folder_id =
      model->MergeItems(model->top_level_item_list()->item_at(0)->id(),
                        model->top_level_item_list()->item_at(1)->id());

  AppListFolderItem* folder_item = model->FindFolderItem(folder_id);
  EXPECT_TRUE(folder_item);

  Show();

  // The main grid view should be showing initially.
  AppListMainView* main_view = view_->app_list_main_view();
  AppsContainerView* container_view =
      main_view->contents_view()->apps_container_view();
  EXPECT_NO_FATAL_FAILURE(CheckView(main_view));
  EXPECT_NO_FATAL_FAILURE(CheckView(container_view->apps_grid_view()));
  EXPECT_FALSE(container_view->app_list_folder_view()->visible());

  AppsGridViewTestApi test_api(container_view->apps_grid_view());
  test_api.PressItemAt(0);

  // After pressing the folder item, the folder view should be showing.
  EXPECT_NO_FATAL_FAILURE(CheckView(main_view));
  EXPECT_NO_FATAL_FAILURE(CheckView(container_view->app_list_folder_view()));
  EXPECT_FALSE(container_view->apps_grid_view()->visible());

  view_->GetWidget()->Hide();
  EXPECT_FALSE(view_->GetWidget()->IsVisible());

  Show();

  // The main grid view should be showing after a reshow.
  EXPECT_NO_FATAL_FAILURE(CheckView(main_view));
  EXPECT_NO_FATAL_FAILURE(CheckView(container_view->apps_grid_view()));
  EXPECT_FALSE(container_view->app_list_folder_view()->visible());

  Close();
}

void AppListViewTestContext::RunStartPageTest() {
  EXPECT_FALSE(view_->GetWidget()->IsVisible());
  EXPECT_EQ(-1, GetPaginationModel()->total_pages());
  AppListTestModel* model = delegate_->GetTestModel();
  model->PopulateApps(3);

  Show();

  AppListMainView* main_view = view_->app_list_main_view();
  StartPageView* start_page_view =
      main_view->contents_view()->start_page_view();
  // Checks on the main view.
  EXPECT_NO_FATAL_FAILURE(CheckView(main_view));
  EXPECT_NO_FATAL_FAILURE(CheckView(main_view->contents_view()));
  if (test_type_ == EXPERIMENTAL) {
    EXPECT_NO_FATAL_FAILURE(CheckView(start_page_view));

    ContentsView* contents_view = main_view->contents_view();
    contents_view->SetActivePage(contents_view->GetPageIndexForNamedPage(
        ContentsView::NAMED_PAGE_START));
    contents_view->Layout();
    EXPECT_FALSE(main_view->search_box_view()->visible());
    EXPECT_TRUE(IsViewAtOrigin(start_page_view));
    EXPECT_FALSE(IsViewAtOrigin(contents_view->apps_container_view()));
    EXPECT_EQ(3u, GetVisibleTileItemViews(start_page_view->tile_views()));

    contents_view->SetActivePage(
        contents_view->GetPageIndexForNamedPage(ContentsView::NAMED_PAGE_APPS));
    contents_view->Layout();
    EXPECT_TRUE(main_view->search_box_view()->visible());
    EXPECT_FALSE(IsViewAtOrigin(start_page_view));
    EXPECT_TRUE(IsViewAtOrigin(contents_view->apps_container_view()));

    // Check tiles hide and show on deletion and addition.
    model->CreateAndAddItem("Test app");
    EXPECT_EQ(4u, GetVisibleTileItemViews(start_page_view->tile_views()));
    model->DeleteItem(model->GetItemName(0));
    EXPECT_EQ(3u, GetVisibleTileItemViews(start_page_view->tile_views()));
  } else {
    EXPECT_EQ(NULL, start_page_view);
  }

  Close();
}

void AppListViewTestContext::RunProfileChangeTest() {
  EXPECT_FALSE(view_->GetWidget()->IsVisible());
  EXPECT_EQ(-1, GetPaginationModel()->total_pages());
  delegate_->GetTestModel()->PopulateApps(kInitialItems);

  Show();

  if (is_landscape())
    EXPECT_EQ(2, GetPaginationModel()->total_pages());
  else
    EXPECT_EQ(3, GetPaginationModel()->total_pages());

  // Change the profile. The original model needs to be kept alive for
  // observers to unregister themselves.
  scoped_ptr<AppListTestModel> original_test_model(
      delegate_->ReleaseTestModel());
  delegate_->set_next_profile_app_count(1);

  // The original ContentsView is destroyed here.
  view_->SetProfileByPath(base::FilePath());
  EXPECT_EQ(1, GetPaginationModel()->total_pages());

  StartPageView* start_page_view =
      view_->app_list_main_view()->contents_view()->start_page_view();
  if (test_type_ == EXPERIMENTAL) {
    EXPECT_NO_FATAL_FAILURE(CheckView(start_page_view));
    EXPECT_EQ(1u, GetVisibleTileItemViews(start_page_view->tile_views()));
  } else {
    EXPECT_EQ(NULL, start_page_view);
  }

  // New model updates should be processed by the start page view.
  delegate_->GetTestModel()->CreateAndAddItem("Test App");
  if (test_type_ == EXPERIMENTAL)
    EXPECT_EQ(2u, GetVisibleTileItemViews(start_page_view->tile_views()));

  // Old model updates should be ignored.
  original_test_model->CreateAndAddItem("Test App 2");
  if (test_type_ == EXPERIMENTAL)
    EXPECT_EQ(2u, GetVisibleTileItemViews(start_page_view->tile_views()));

  Close();
}

class AppListViewTestAura : public views::ViewsTestBase,
                            public ::testing::WithParamInterface<int> {
 public:
  AppListViewTestAura() {}
  virtual ~AppListViewTestAura() {}

  // testing::Test overrides:
  virtual void SetUp() OVERRIDE {
    views::ViewsTestBase::SetUp();
    test_context_.reset(new AppListViewTestContext(GetParam(), GetContext()));
  }

  virtual void TearDown() OVERRIDE {
    test_context_.reset();
    views::ViewsTestBase::TearDown();
  }

 protected:
  scoped_ptr<AppListViewTestContext> test_context_;

 private:
  DISALLOW_COPY_AND_ASSIGN(AppListViewTestAura);
};

class AppListViewTestDesktop : public views::ViewsTestBase,
                               public ::testing::WithParamInterface<int> {
 public:
  AppListViewTestDesktop() {}
  virtual ~AppListViewTestDesktop() {}

  // testing::Test overrides:
  virtual void SetUp() OVERRIDE {
    set_views_delegate(new AppListViewTestViewsDelegate(this));
    views::ViewsTestBase::SetUp();
    test_context_.reset(new AppListViewTestContext(GetParam(), NULL));
  }

  virtual void TearDown() OVERRIDE {
    test_context_.reset();
    views::ViewsTestBase::TearDown();
  }

 protected:
  scoped_ptr<AppListViewTestContext> test_context_;

 private:
  class AppListViewTestViewsDelegate : public views::TestViewsDelegate {
   public:
    AppListViewTestViewsDelegate(AppListViewTestDesktop* parent)
        : parent_(parent) {}

    // Overridden from views::ViewsDelegate:
    virtual void OnBeforeWidgetInit(
        views::Widget::InitParams* params,
        views::internal::NativeWidgetDelegate* delegate) OVERRIDE;

   private:
    AppListViewTestDesktop* parent_;

    DISALLOW_COPY_AND_ASSIGN(AppListViewTestViewsDelegate);
  };

  DISALLOW_COPY_AND_ASSIGN(AppListViewTestDesktop);
};

void AppListViewTestDesktop::AppListViewTestViewsDelegate::OnBeforeWidgetInit(
    views::Widget::InitParams* params,
    views::internal::NativeWidgetDelegate* delegate) {
// Mimic the logic in ChromeViewsDelegate::OnBeforeWidgetInit(). Except, for
// ChromeOS, use the root window from the AuraTestHelper rather than depending
// on ash::Shell:GetPrimaryRootWindow(). Also assume non-ChromeOS is never the
// Ash desktop, as that is covered by AppListViewTestAura.
#if defined(OS_CHROMEOS)
  if (!params->parent && !params->context)
    params->context = parent_->GetContext();
#elif defined(USE_AURA)
  if (params->parent == NULL && params->context == NULL && !params->child)
    params->native_widget = new views::DesktopNativeWidgetAura(delegate);
#endif
}

}  // namespace

// Tests showing the app list with basic test model in an ash-style root window.
TEST_P(AppListViewTestAura, Display) {
  EXPECT_NO_FATAL_FAILURE(test_context_->RunDisplayTest());
}

// Tests showing the app list on the desktop. Note on ChromeOS, this will still
// use the regular root window.
TEST_P(AppListViewTestDesktop, Display) {
  EXPECT_NO_FATAL_FAILURE(test_context_->RunDisplayTest());
}

// Tests that the main grid view is shown after hiding and reshowing the app
// list with a folder view open. This is a regression test for crbug.com/357058.
TEST_P(AppListViewTestAura, ReshowWithOpenFolder) {
  EXPECT_NO_FATAL_FAILURE(test_context_->RunReshowWithOpenFolderTest());
}

TEST_P(AppListViewTestDesktop, ReshowWithOpenFolder) {
  EXPECT_NO_FATAL_FAILURE(test_context_->RunReshowWithOpenFolderTest());
}

// Tests that the start page view operates correctly.
TEST_P(AppListViewTestAura, StartPageTest) {
  EXPECT_NO_FATAL_FAILURE(test_context_->RunStartPageTest());
}

TEST_P(AppListViewTestDesktop, StartPageTest) {
  EXPECT_NO_FATAL_FAILURE(test_context_->RunStartPageTest());
}

// Tests that the profile changes operate correctly.
TEST_P(AppListViewTestAura, ProfileChangeTest) {
  EXPECT_NO_FATAL_FAILURE(test_context_->RunProfileChangeTest());
}

TEST_P(AppListViewTestDesktop, ProfileChangeTest) {
  EXPECT_NO_FATAL_FAILURE(test_context_->RunProfileChangeTest());
}

INSTANTIATE_TEST_CASE_P(AppListViewTestAuraInstance,
                        AppListViewTestAura,
                        ::testing::Range<int>(TEST_TYPE_START, TEST_TYPE_END));

INSTANTIATE_TEST_CASE_P(AppListViewTestDesktopInstance,
                        AppListViewTestDesktop,
                        ::testing::Range<int>(TEST_TYPE_START, TEST_TYPE_END));

}  // namespace test
}  // namespace app_list
