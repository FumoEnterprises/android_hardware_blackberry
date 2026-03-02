/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "vendor.blackberry.touchkeypad-service"

#include "TouchKeypad.h"

#include <android-base/file.h>
#include <android-base/logging.h>

using ::android::base::ReadFileToString;
using ::android::base::WriteStringToFile;

namespace {

constexpr const char* kTouchKeypadEnablePath = "/sys/devices/touch_keypad/turn_off";

}  // anonymous namespace

namespace aidl {
namespace vendor {
namespace blackberry {
namespace touchkeypad {

ndk::ScopedAStatus TouchKeypad::isEnabled(bool* _aidl_return) {
    std::string value;
    if (!ReadFileToString(kTouchKeypadEnablePath, &value)) {
        LOG(ERROR) << "Failed to read current TouchKeypad state";
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    *_aidl_return = value != "0\n";
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus TouchKeypad::setEnabled(bool enable) {
    if (!WriteStringToFile(enable ? "1" : "0", kTouchKeypadEnablePath, true)) {
        LOG(ERROR) << "Failed to write TouchKeypad state";
        return ndk::ScopedAStatus::fromExceptionCode(EX_SERVICE_SPECIFIC);
    }

    return ndk::ScopedAStatus::ok();
}

}  // namespace touchkeypad
}  // namespace blackberry
}  // namespace vendor
}  // namespace aidl
