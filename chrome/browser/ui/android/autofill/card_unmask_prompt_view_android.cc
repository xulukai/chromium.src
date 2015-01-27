// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/android/autofill/card_unmask_prompt_view_android.h"

#include "chrome/browser/ui/autofill/card_unmask_prompt_controller.h"
#include "content/public/browser/web_contents.h"
#include "jni/CardUnmaskBridge_jni.h"
#include "ui/base/android/view_android.h"
#include "ui/base/android/window_android.h"

namespace autofill {

CardUnmaskPromptView* CardUnmaskPromptView::CreateAndShow(
    CardUnmaskPromptController* controller) {
  CardUnmaskPromptViewAndroid* view =
      new CardUnmaskPromptViewAndroid(controller);
  view->Show();
  return view;
}

CardUnmaskPromptViewAndroid::CardUnmaskPromptViewAndroid(
    CardUnmaskPromptController* controller)
    : controller_(controller) {
}

CardUnmaskPromptViewAndroid::~CardUnmaskPromptViewAndroid() {
  if (controller_)
    controller_->OnUnmaskDialogClosed();
}

void CardUnmaskPromptViewAndroid::Show() {
  JNIEnv* env = base::android::AttachCurrentThread();
  ui::ViewAndroid* view_android =
      controller_->GetWebContents()->GetNativeView();

  ScopedJavaLocalRef<jstring> dialog_title =
      base::android::ConvertUTF16ToJavaString(env,
                                              controller_->GetWindowTitle());
  ScopedJavaLocalRef<jstring> instructions =
      base::android::ConvertUTF16ToJavaString(
          env, controller_->GetInstructionsMessage());
  java_object_.Reset(Java_CardUnmaskBridge_create(
      env, reinterpret_cast<intptr_t>(this), dialog_title.obj(),
      instructions.obj(),
      view_android->GetWindowAndroid()->GetJavaObject().obj()));

  Java_CardUnmaskBridge_show(env, java_object_.obj());
}

bool CardUnmaskPromptViewAndroid::CheckUserInputValidity(JNIEnv* env,
                                                         jobject obj,
                                                         jstring response) {
  return controller_->InputTextIsValid(
      base::android::ConvertJavaStringToUTF16(env, response));
}

void CardUnmaskPromptViewAndroid::OnUserInput(JNIEnv* env,
                                              jobject obj,
                                              jstring response) {
  controller_->OnUnmaskResponse(
      base::android::ConvertJavaStringToUTF16(env, response),
      base::string16(),
      base::string16());
}

void CardUnmaskPromptViewAndroid::PromptDismissed(JNIEnv* env, jobject obj) {
  delete this;
}

void CardUnmaskPromptViewAndroid::ControllerGone() {
  controller_ = nullptr;
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_CardUnmaskBridge_dismiss(env, java_object_.obj());
}

void CardUnmaskPromptViewAndroid::DisableAndWaitForVerification() {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_CardUnmaskBridge_disableAndWaitForVerification(env, java_object_.obj());
}

void CardUnmaskPromptViewAndroid::GotVerificationResult(bool success) {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_CardUnmaskBridge_verificationFinished(env, java_object_.obj(), success);
}

// static
bool CardUnmaskPromptViewAndroid::Register(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // namespace autofill
