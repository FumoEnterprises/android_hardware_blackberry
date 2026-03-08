/*
 * Copyright (C) 2021 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "vendor.lineage.touch@1.0-service.blackberry"

#include <android-base/logging.h>
#include <hidl/HidlTransportSupport.h>

#include "GloveMode.h"
#include "KeyDisabler.h"

using ::vendor::lineage::touch::V1_0::IGloveMode;
using ::vendor::lineage::touch::V1_0::implementation::GloveMode;

using ::vendor::lineage::touch::V1_0::IKeyDisabler;
using ::vendor::lineage::touch::V1_0::implementation::KeyDisabler;

int main() {
    android::sp<IGloveMode> gloveMode = new GloveMode();
    android::sp<IKeyDisabler> keyDisabler = new KeyDisabler();

    android::hardware::configureRpcThreadpool(1, true /*callerWillJoin*/);

    if (gloveMode->registerAsService() != android::OK) {
        LOG(ERROR) << "Cannot register touchscreen glove HAL service.";
        return 1;
    }

    if (keyDisabler->registerAsService() != android::OK) {
        LOG(ERROR) << "Cannot register touchscreen KeyDisabler HAL service.";
        return 1;
    }

    LOG(INFO) << "Touchscreen HAL service ready.";

    android::hardware::joinRpcThreadpool();

    LOG(ERROR) << "Touchscreen HAL service failed to join thread pool.";
    return 1;
}
