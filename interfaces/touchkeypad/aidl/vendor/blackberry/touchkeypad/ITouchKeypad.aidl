/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package vendor.blackberry.touchkeypad;

@VintfStability
interface ITouchKeypad {
    // Returns the current status of the Touch Keypad.
    boolean isEnabled();

    // Enables or disables Touch Keypad.
    void setEnabled(in boolean enable);
}
