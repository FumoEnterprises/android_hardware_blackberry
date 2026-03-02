/*
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include "TouchKeypad.h"

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>

using aidl::vendor::blackberry::touchkeypad::TouchKeypad;

int main() {
    ABinderProcess_setThreadPoolMaxThreadCount(0);
    std::shared_ptr<TouchKeypad> touchkeypad = ndk::SharedRefBase::make<TouchKeypad>();

    const std::string instance = std::string(TouchKeypad::descriptor) + "/default";
    binder_status_t status =
    AServiceManager_addService(touchkeypad->asBinder().get(), instance.c_str());
    CHECK_EQ(status, STATUS_OK);

    LOG(ERROR) << "BlackBerry TouchKeypad service registered succesfully.";

    ABinderProcess_joinThreadPool();
    return EXIT_FAILURE;  // should not reach
}
