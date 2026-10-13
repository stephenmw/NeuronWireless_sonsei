"""Verify the actual compiled Sonsei USB/BLE descriptors, including Bazecor reports.

Usage: python3 tests/verify_descriptors.py build/ploverhid-release/obj/hid_report_descriptor.o
Only Python's standard library is required. Reads ELF32 little-endian objects.
"""
from pathlib import Path
import struct
import sys


def sections(path):
    data = Path(path).read_bytes()
    assert data[:6] == b'\x7fELF\x01\x01'
    header = struct.unpack_from('<16sHHIIIIIHHHHHH', data)
    offset, stride, count, names_index = header[6], header[11], header[12], header[13]
    entries = [struct.unpack_from('<IIIIIIIIII', data, offset + i * stride) for i in range(count)]
    names_entry = entries[names_index]
    names = data[names_entry[4]:names_entry[4] + names_entry[5]]
    return {names[e[0]:].split(b'\0', 1)[0].decode(): data[e[4]:e[4] + e[5]] for e in entries}


def reports(descriptor):
    page = usage = report_id = size = count = depth = 0
    bits = {}
    collections = []
    cursor = 0
    while cursor < len(descriptor):
        prefix = descriptor[cursor]
        cursor += 1
        assert prefix != 0xfe
        length = (prefix & 3) if (prefix & 3) != 3 else 4
        assert cursor + length <= len(descriptor)
        value = int.from_bytes(descriptor[cursor:cursor + length], 'little')
        cursor += length
        tag = prefix & 0xfc
        if tag == 0x04: page = value
        elif tag == 0x08: usage = value
        elif tag == 0x84: report_id = value
        elif tag == 0x74: size = value
        elif tag == 0x94: count = value
        elif tag == 0xa0:
            if depth == 0: collections.append((page, usage, value))
            depth += 1
        elif tag == 0xc0:
            assert depth > 0
            depth -= 1
        elif tag in (0x80, 0x90, 0xb0):
            bits[report_id, tag] = bits.get((report_id, tag), 0) + size * count
    assert depth == 0
    assert (0xff50, 0x4c56, 1) in collections, 'Plover must be a top-level Application collection'
    assert bits[0x50, 0x80] == 64
    assert {key[0] for key in bits} == {1, 2, 3, 4, 5, 0x50}
    return bits


compiled = sections(sys.argv[1])
usb = reports(compiled['.rodata._ZN12_GLOBAL__N_1L14usb_descriptorE'])
ble = reports(compiled['.rodata._ZN12_GLOBAL__N_1L14ble_descriptorE'])
for tag in (0x80, 0x90):
    assert usb[5, tag] == 63 * 8, 'USB configuration reports must fit a 64-byte packet including ID'
    assert ble[5, tag] == 200 * 8, 'BLE configuration reports must remain 200 bytes'
assert {k: v for k, v in usb.items() if k[0] != 5} == {k: v for k, v in ble.items() if k[0] != 5}
print('Compiled Sonsei USB/BLE Plover and configuration descriptors passed.')
