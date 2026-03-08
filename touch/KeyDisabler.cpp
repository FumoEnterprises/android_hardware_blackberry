/*
 * Copyright (C) 2019 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/strings.h>

#include "KeyDisabler.h"

namespace vendor {
namespace lineage {
namespace touch {
namespace V1_0 {
namespace implementation {

constexpr const char kControlPath[] = "/sys/devices/virtual/tp_device/tp_button/button_enable";

KeyDisabler::KeyDisabler() {
    mHasKeyDisabler = !access(kControlPath, F_OK);
}

// Methods from ::vendor::lineage::touch::V1_0::IKeyDisabler follow.
Return<bool> KeyDisabler::isEnabled() {
    std::string val;

    if (!android::base::ReadFileToString(kControlPath, &val)) {
        PLOG(ERROR) << "Failed to read button_enable";
        return false;
    }

    android::base::Trim(val);
    LOG(INFO) << "Enabled state raw: " << val;

    // not sure if this is needed but it seemed to fail without it
    if (val == "0x0") {
        return true;
    }

    return false;
}

Return<bool> KeyDisabler::setEnabled(bool enabled) {
    std::string val = enabled ? "0" : "1";

    bool ok = android::base::WriteStringToFile(val, kControlPath);
    if (!ok) {
        PLOG(ERROR) << "Failed to write button_enable";
    }

    LOG(INFO) << "NewEnabled state: " << enabled << " write_ok=" << ok;
    return ok;
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace touch
}  // namespace lineage
}  // namespace vendor
