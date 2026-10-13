/*
 * Copyright (C) 2025  Dygma Lab S.L.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * The purpose of this module is for defining the HID descriptors and functions for
 * getting the USB and BLE HID descriptor versions in hidDefy module
 */

#include "hidDefy.h"
#include "PloverHIDProtocol.h"

#define RAW_USAGE_SONSEI 0x01
#define USB_RAW_HID_REPORT_SIZE 63

namespace {
const uint8_t usb_descriptor[] = {
    HID_DEFY_COMMON_REPORT_DESCRIPTOR, PLOVER_HID_DESCRIPTOR,
    TUD_HID_REPORT_DESC_GENERIC_INOUT_DYGMA(USB_RAW_HID_REPORT_SIZE, RAW_USAGE_SONSEI,
                                           HID_REPORT_ID(REPORT_ID_RAW))
};
const uint8_t ble_descriptor[] = {
    HID_DEFY_COMMON_REPORT_DESCRIPTOR, PLOVER_HID_DESCRIPTOR,
    TUD_HID_REPORT_DESC_GENERIC_INOUT_DYGMA(OUTPUT_REPORT_LEN_RAW, RAW_USAGE_SONSEI,
                                           HID_REPORT_ID(REPORT_ID_RAW))
};
}

void hid_report_descriptor_usb_get(const uint8_t **data, uint32_t *length) {
    *data = usb_descriptor;
    *length = sizeof(usb_descriptor);
}
void hid_report_descriptor_ble_get(const uint8_t **data, uint32_t *length) {
    *data = ble_descriptor;
    *length = sizeof(ble_descriptor);
}
void hid_report_descriptor_get(const uint8_t **data, uint32_t *length) {
    hid_report_descriptor_ble_get(data, length);
}
