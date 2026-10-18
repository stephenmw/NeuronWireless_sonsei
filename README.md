# Sonsei firmware with PloverHID

Based on Dygmalab's stable `Sonsei_v1.1.0` tag (`72e33d3`), on the
`ploverhid-support` branch. Adds direct PloverHID key assignments over USB and
Bluetooth LE, using the same `0xE000`–`0xE03F` keycodes as the Defy/Bazecor
implementation in this workspace. Existing layers provide access to steno keys;
no separate steno mode or preset layout is installed.

Assign keys using the PloverHID picker in a compatible Bazecor build. In Plover,
select **Plover HID** as the machine. The firmware advertises
`ploverhid.version` (`1`) through Focus. `ploverhid.faults` counts delivery faults.
See [the protocol catalog](libraries/PloverHID/contract.json) for all 64 identities.

The plugin captures each physical assignment through release, handles duplicate
assignments, and queues ordered eight-byte snapshots. Connection changes,
suspend/resume, BLE subscription changes, and delivery faults clear pending
input and require held steno keys to release before resuming. Reports already
received by Plover cannot be retracted.

USB/BLE expose usage page `0xFF50`, usage `0x4C56`, report ID `0x50` as an
Application collection. Sonsei's existing configuration reports retain their
63-byte USB and 200-byte BLE payloads. The plugin is registered in the
Kaleidoscope adapter immediately after KbdInterface so LEDControl and other
Dygma handlers can observe PloverHID key activity before it is consumed.
KeyRoleManager passes
Plover key events through without tap/hold queuing; OneShot processes them
normally, so a steno press can use up a pending one-shot modifier or layer.
Ordinary typing remains on the existing report path; synthetic macro/Superkey
Plover assignments are consumed without steno output.

## Build

Initialize recursive submodules, install a complete GNU Arm embedded toolchain
(including newlib), GNU Make, and Python with `numpy` and `intelhex`. Ensure
`python` and `arm-none-eabi-*` are on `PATH`. Extract Nordic's
`nRF5_SDK_17.1.0_ddde560` into `libraries/SDK/nRF5_SDK_17.1.0_ddde560`.
This local checkout uses the SDK already installed in the adjacent Defy tree.

```sh
cd build
gmake -j8 release MODE_RELEASE_DIR_NAME=ploverhid-release
```

Output: `build/ploverhid-release/NeuronWireless_sonsei_sealed.hex` and
`NeuronWireless_sonsei_sealed.bin`. These are Sonsei application artifacts, not a
combined bootloader/keyscanner/SoftDevice update package. Upstream build-version
metadata is retained. The local library changes are committed on
`ploverhid-support` branches in their respective submodules; publishing this
branch also requires publishing those dependency commits.

## Verification

From the repository root:

```sh
c++ -std=c++11 -I libraries/PloverHID libraries/PloverHID/tests/state_test.cpp -o /tmp/sonsei-plover-state
/tmp/sonsei-plover-state
c++ -std=c++11 -I libraries/PloverHID libraries/PloverHID/tests/descriptor_test.cpp -o /tmp/sonsei-plover-descriptor
/tmp/sonsei-plover-descriptor
python3 tests/verify_descriptors.py build/ploverhid-release/obj/hid_report_descriptor.o
c++ -std=c++11 -I libraries/PloverHID libraries/PloverHID/tests/report_dump.cpp -o /tmp/sonsei-plover-reports
# Run with the Python shipped in your Plover installation:
python libraries/PloverHID/tests/verify_with_plover.py /tmp/sonsei-plover-reports
```

Verified locally with GNU Arm 13.2.Rel1 and the installed Plover 5.4.1 decoder:
release build and sealing, Sonsei's 60-key state model, all 64 protocol identities,
Application collection discovery metadata, and compiled USB/BLE configuration
report sizes. Native tests cover duplicates, repeated events, retry ordering,
connection changes, held-key release barriers, invalid inputs, and queue overflow.

Physical Sonsei testing remains required for USB/BLE discovery and input,
reconnect/suspend behavior, simultaneous Bazecor access, and mid-stroke connection
loss. Re-pair Bluetooth if the host caches the previous report map. No firmware
has been flashed as part of this change.
