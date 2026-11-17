# nrflog

A log viewer for the nRF52840 CDC-ACM port, with auto-reconnect.

```
uv run main.py
```

## Why not `cat /dev/ttyACM0`

Three reasons, all of which you have already hit:

- **Replug changes nothing.** The device is located by USB **VID:PID**, not by
  path, so it is found again even if it comes back as `ttyACM1`. No retyping.
- **Raw mode.** `cat` leaves the tty in canonical mode, where the kernel
  buffers until it sees a newline. Output without a trailing `\n` is invisible.
  This opens the port raw.
- **Unterminated output is still shown.** If bytes arrive and no newline
  follows — within 300 ms, or within 160 bytes — the line is displayed anyway,
  in yellow with a `⋯`. Firmware that truncates before its own newline is a
  bug you want to *see*, not one that looks like silence.

## Keys

| Key | Does |
|-----|------|
| `q` | quit |
| `c` | clear the buffer and counters |
| `s` | save scrollback to `nrflog-YYYYmmdd-HHMMSS.log` |
| `f` | toggle follow (auto-scroll) |

## Options

```
--vid 1209      USB vendor ID, hex without 0x   (pid.codes test VID)
--pid 0001      USB product ID                  (matches usb.c descriptor)
--device PATH   pin to one path, skipping VID:PID discovery
--baud 115200   set on the tty; CDC ignores it, the line discipline does not
```

## Layout

`serial_port.py` has no Textual dependency on purpose — it is the transport
layer, and the protocol client for the sniffer will want the same discovery and
the same raw reads.

| Piece | Does |
|-------|------|
| `find_devices()` | scans `/sys/class/tty/ttyACM*` and matches the USB parent's `idVendor`/`idProduct` |
| `SerialPort` | raw termios setup, non-blocking reads, `Disconnected` on unplug |
| `LineAssembler` | reassembles lines across read boundaries — a USB packet boundary is not a line boundary |
| `main.py` | the Textual UI, and a threaded worker that reconnects forever |

Standard library only below the UI; no pyserial.

## Notes

The firmware gates `usb_write` on DTR, set from `SET_CONTROL_LINE_STATE`.
Opening the port raises DTR on Linux anyway, but `serial_port.py` asserts it
explicitly so the dependency is visible rather than inherited from tty
defaults. Closing drops it, which stops the firmware transmitting — so nothing
accumulates in the ring while nobody is listening.
