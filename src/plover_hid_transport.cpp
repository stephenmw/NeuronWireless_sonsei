// SPDX-License-Identifier: GPL-3.0-or-later
#include "PloverHIDTransport.h"
#include "PloverHIDProtocol.h"
#include "hidDefy.h"
#include "Ble_composite_dev.h"

namespace {
uint32_t usb_generation = 1;
uint32_t generation = 1;
uint32_t previous_epoch = 0;
uint8_t previous_transport = 0;
uint8_t transport() {
  if (ble_connected()) return 2;
  return TinyUSBDevice.mounted() && !TinyUSBDevice.suspended() ? 1 : 0;
}
}
extern "C" void tud_mount_cb() { ++usb_generation; }
extern "C" void tud_umount_cb() { ++usb_generation; }
extern "C" void tud_suspend_cb(bool) { ++usb_generation; }
extern "C" void tud_resume_cb() { ++usb_generation; }
namespace ploverhid {
uint32_t connectionGeneration() {
  const uint8_t selected = transport();
  const uint32_t epoch = selected == 2 ? ble_hid_connection_generation() : usb_generation;
  if (selected != previous_transport || epoch != previous_epoch) {
    ++generation;
    if (!generation) ++generation;
    previous_transport = selected;
    previous_epoch = epoch;
  }
  return selected ? generation : 0;
}
SendResult sendReport(const uint8_t *report, uint32_t expected_generation) {
  if (connectionGeneration() != expected_generation) return SendResult::Retry;
  // Bypass the common FIFO: PloverHID owns ordering and connection recovery.
  if (transport() == 2) {
    const int8_t result = ble_send_plover_report(report);
    return result > 0 ? SendResult::Accepted : result < 0 ? SendResult::Fault : SendResult::Retry;
  }
  if (transport() != 1 || !HID().usb_hid.ready()) return SendResult::Retry;
  return HID().usb_hid.sendReport(PLOVER_HID_REPORT_ID, report, PLOVER_HID_REPORT_SIZE)
    ? SendResult::Accepted : SendResult::Retry;
}
}
