/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <aidl/vendor/blackberry/touchkeypad/BnTouchKeypad.h>

namespace aidl {
namespace vendor {
namespace blackberry {
namespace touchkeypad {

class TouchKeypad : public BnTouchKeypad {
  public:
    ndk::ScopedAStatus isEnabled(bool* _aidl_return) override;
    ndk::ScopedAStatus setEnabled(bool enable) override;
};

}  // namespace touchkeypad
}  // namespace blackberry
}  // namespace vendor
}  // namespace aidl
