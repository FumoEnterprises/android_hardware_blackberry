/*
 * Copyright (C) 2021-2022 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "GloveModeService"

#include "GloveMode.h"

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android-base/strings.h>
#include <fstream>

namespace vendor {
namespace lineage {
namespace touch {
namespace V1_0 {
namespace implementation {

const std::string kGloveModePath = "/sys/devices/virtual/tp_device/tp_glove/glove_enable";

Return<bool> GloveMode::isEnabled() {
    std::string val;

    if (!android::base::ReadFileToString(kGloveModePath, &val)) {
        PLOG(ERROR) << "Failed to read glove_mode";
        return false;
    }

    android::base::Trim(val);
    LOG(INFO) << "Enabled state raw: " << val;

    // not sure if this is needed but it seemed to fail without it
    if (val == "0x1") {
        return true;
    }

    return false;
}

Return<bool> GloveMode::setEnabled(bool enabled) {
    std::string val = enabled ? "1" : "0";

    bool ok = android::base::WriteStringToFile(val, kGloveModePath);
    if (!ok) {
        PLOG(ERROR) << "Failed to write glove_mode";
    }

    LOG(INFO) << "NewEnabled state: " << enabled << " write_ok=" << ok;
    return ok;
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace touch
}  // namespace lineage
}  // namespace vendor
