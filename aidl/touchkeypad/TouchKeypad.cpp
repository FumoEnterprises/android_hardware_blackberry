/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "vendor.blackberry.touchkeypad-service"

#include "TouchKeypad.h"

#include <dirent.h>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/properties.h>

using ::android::base::GetBoolProperty;
using ::android::base::GetUintProperty;
using ::android::base::SetProperty;
using ::android::base::WriteStringToFile;

namespace {

constexpr const char* kTouchKeypadEnablePath = "/sys/devices/touch_keypad/turn_off";
constexpr const char* kTouchKeypadPersistProp = "persist.vendor.touchkeypad.enabled";
constexpr const char* kTouchKeypadTimeoutProp = "persist.vendor.touchkeypad.timeout_ms";
constexpr bool kTouchKeypadDefaultEnabled = false;
constexpr unsigned int kDefaultTimeoutMs = 400;
constexpr const char* kKeypadInputName = "stmpe_keypad";

}  // anonymous namespace

namespace aidl {
namespace vendor {
namespace blackberry {
namespace touchkeypad {

TouchKeypad::TouchKeypad() {
    mUserEnabled = GetBoolProperty(kTouchKeypadPersistProp, kTouchKeypadDefaultEnabled);
    mEffectiveEnabled = mUserEnabled;

    LOG(INFO) << "Restoring TouchKeypad state: " << (mUserEnabled ? "enabled" : "disabled");
    applySysfs(mUserEnabled);

    mMonitorThread = std::thread(&TouchKeypad::monitorKeypad, this);
}

TouchKeypad::~TouchKeypad() {
    mStopMonitor = true;
    if (mEpollFd >= 0) {
        close(mEpollFd);
    }
    if (mMonitorThread.joinable()) {
        mMonitorThread.join();
    }
}

int TouchKeypad::openInputDevice(const char* name) {
    const char* inputDir = "/dev/input";
    DIR* dir = opendir(inputDir);
    if (!dir) {
        LOG(ERROR) << "Failed to open " << inputDir;
        return -1;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (strncmp(entry->d_name, "event", 5) != 0) continue;

        std::string path = std::string(inputDir) + "/" + entry->d_name;
        int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
        if (fd < 0) continue;

        char deviceName[256] = {};
        if (ioctl(fd, EVIOCGNAME(sizeof(deviceName)), deviceName) >= 0) {
            if (strcmp(deviceName, name) == 0) {
                closedir(dir);
                LOG(INFO) << "Found input device '" << name << "' at " << path;
                return fd;
            }
        }
        close(fd);
    }

    closedir(dir);
    LOG(ERROR) << "Input device '" << name << "' not found";
    return -1;
}

void TouchKeypad::monitorKeypad() {
    int keypadFd = openInputDevice(kKeypadInputName);
    if (keypadFd < 0) {
        LOG(ERROR) << "Cannot monitor keypad, auto-suppress disabled";
        return;
    }

    mEpollFd = epoll_create1(0);
    if (mEpollFd < 0) {
        LOG(ERROR) << "Failed to create epoll fd";
        close(keypadFd);
        return;
    }

    struct epoll_event ev = {};
    ev.events = EPOLLIN;
    ev.data.fd = keypadFd;
    if (epoll_ctl(mEpollFd, EPOLL_CTL_ADD, keypadFd, &ev) < 0) {
        LOG(ERROR) << "Failed to add keypad fd to epoll";
        close(keypadFd);
        close(mEpollFd);
        mEpollFd = -1;
        return;
    }

    LOG(INFO) << "Keypad monitor thread started";

    bool suppressed = false;
    struct epoll_event events[1];

    while (!mStopMonitor) {
        unsigned int timeoutMs =
                GetUintProperty<unsigned int>(kTouchKeypadTimeoutProp, kDefaultTimeoutMs);

        int timeout = suppressed ? static_cast<int>(timeoutMs) : -1;
        int nfds = epoll_wait(mEpollFd, events, 1, timeout);

        if (mStopMonitor) break;

        if (nfds < 0) {
            if (errno == EINTR) continue;
            LOG(ERROR) << "epoll_wait failed: " << strerror(errno);
            break;
        }

        if (nfds == 0) {
            // Timeout expired — no typing for timeoutMs, re-enable if user wants it on
            std::lock_guard<std::mutex> lock(mLock);
            if (suppressed && mUserEnabled) {
                applySysfs(true);
                mEffectiveEnabled = true;
                suppressed = false;
                LOG(INFO) << "Typing idle, re-enabling touch keypad";
            }
            continue;
        }

        // Key event received — drain the fd and suppress if needed
        struct input_event inputEvents[64];
        ssize_t n = read(keypadFd, inputEvents, sizeof(inputEvents));
        if (n < 0) continue;

        bool gotKeypress = false;
        size_t count = n / sizeof(struct input_event);
        for (size_t i = 0; i < count; i++) {
            if (inputEvents[i].type == EV_KEY) {
                gotKeypress = true;
                break;
            }
        }

        if (gotKeypress) {
            std::lock_guard<std::mutex> lock(mLock);
            if (mUserEnabled && mEffectiveEnabled) {
                applySysfs(false);
                mEffectiveEnabled = false;
                LOG(INFO) << "Typing detected, suppressing touch keypad";
            }
            suppressed = true;  // reset the debounce timer even if already suppressed
        }
    }

    close(keypadFd);
    LOG(INFO) << "Keypad monitor thread exiting";
}

void TouchKeypad::applySysfs(bool enable) {
    if (!WriteStringToFile(enable ? "0" : "1", kTouchKeypadEnablePath, true)) {
        LOG(ERROR) << "Failed to write TouchKeypad sysfs state";
    }
}

ndk::ScopedAStatus TouchKeypad::isEnabled(bool* _aidl_return) {
    std::lock_guard<std::mutex> lock(mLock);
    // Report user's desired state, not the effective (possibly suppressed) state
    *_aidl_return = mUserEnabled;
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus TouchKeypad::setEnabled(bool enable) {
    std::lock_guard<std::mutex> lock(mLock);
    mUserEnabled = enable;
    mEffectiveEnabled = enable;
    applySysfs(enable);

    if (!SetProperty(kTouchKeypadPersistProp, enable ? "true" : "false")) {
        LOG(ERROR) << "Failed to persist TouchKeypad state";
    }

    return ndk::ScopedAStatus::ok();
}

}  // namespace touchkeypad
}  // namespace blackberry
}  // namespace vendor
}  // namespace aidl
