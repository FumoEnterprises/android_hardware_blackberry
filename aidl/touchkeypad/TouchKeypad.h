/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <aidl/vendor/blackberry/touchkeypad/BnTouchKeypad.h>

#include <atomic>
#include <mutex>
#include <thread>

namespace aidl {
namespace vendor {
namespace blackberry {
namespace touchkeypad {

class TouchKeypad : public BnTouchKeypad {
  public:
    TouchKeypad();
    ~TouchKeypad();

    ndk::ScopedAStatus isEnabled(bool* _aidl_return) override;
    ndk::ScopedAStatus setEnabled(bool enable) override;

  private:
    void monitorKeypad();
    void applySysfs(bool enable);
    int openInputDevice(const char* name);

    std::mutex mLock;
    bool mUserEnabled;       // user's desired state (persisted, shown on QS tile)
    bool mEffectiveEnabled;  // actual sysfs state (suppressed during typing)

    std::thread mMonitorThread;
    std::atomic<bool> mStopMonitor{false};
    int mEpollFd = -1;
};

}  // namespace touchkeypad
}  // namespace blackberry
}  // namespace vendor
}  // namespace aidl
