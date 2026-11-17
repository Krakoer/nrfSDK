"""Device discovery and raw serial reads for the nRF52840 CDC-ACM port.

Deliberately free of any TUI dependency: this is the transport layer, and the
protocol client for the sniffer will want it too. Standard library only --
pyserial would work but adds a dependency for about forty lines of termios.
"""

from __future__ import annotations

import errno
import fcntl
import os
import select
import struct
import termios
from dataclasses import dataclass
from pathlib import Path

# pid.codes test VID/PID, matching the device descriptor in mysdk/src/usb.c.
DEFAULT_VID = "1209"
DEFAULT_PID = "0001"

TIOCM_DTR = 0x002
TIOCM_RTS = 0x004


@dataclass(frozen=True)
class FoundDevice:
    path: str
    vid: str
    pid: str

    def __str__(self) -> str:
        return f"{self.path} ({self.vid}:{self.pid})"


def _read_sysfs(path: Path) -> str | None:
    try:
        return path.read_text().strip()
    except OSError:
        return None


def find_devices(vid: str = DEFAULT_VID, pid: str = DEFAULT_PID) -> list[FoundDevice]:
    """Every /dev/ttyACM* whose USB parent matches vid:pid, lowest number first.

    Matching on VID/PID rather than a fixed path is what makes reconnect work
    when the device comes back as ttyACM1 instead of ttyACM0.
    """
    found: list[FoundDevice] = []
    for tty in sorted(Path("/sys/class/tty").glob("ttyACM*")):
        # .../ttyACMn/device is the USB *interface*; its parent is the device.
        usb_device = tty / "device" / ".."
        got_vid = _read_sysfs(usb_device / "idVendor")
        got_pid = _read_sysfs(usb_device / "idProduct")
        if got_vid is None or got_pid is None:
            continue
        if got_vid.lower() == vid.lower() and got_pid.lower() == pid.lower():
            dev_path = f"/dev/{tty.name}"
            if os.path.exists(dev_path):
                found.append(FoundDevice(dev_path, got_vid, got_pid))
    return found


def _configure_raw(fd: int, baud: int) -> None:
    """Put the tty in raw mode.

    Without this the line discipline buffers until a newline and mangles CR,
    which is why `cat` on the port looks like it is dropping output that has
    no trailing \\n.
    """
    speed = getattr(termios, f"B{baud}", termios.B115200)
    iflag, oflag, cflag, lflag, _ispeed, _ospeed, cc = termios.tcgetattr(fd)

    iflag = 0  # no IXON/IXOFF/ICRNL/INLCR/ISTRIP
    oflag = 0  # no OPOST: do not touch bytes on the way out
    lflag = 0  # no ICANON/ECHO/ISIG: deliver bytes as they arrive
    cflag = termios.CS8 | termios.CREAD | termios.CLOCAL

    cc = list(cc)
    cc[termios.VMIN] = 0  # never block in read(); select() does the waiting
    cc[termios.VTIME] = 0

    termios.tcsetattr(
        fd, termios.TCSANOW, [iflag, oflag, cflag, lflag, speed, speed, cc]
    )
    termios.tcflush(fd, termios.TCIFLUSH)


def _assert_dtr(fd: int) -> None:
    """Raise DTR (and RTS) explicitly.

    The firmware gates usb_write on dtr_latch, set from SET_CONTROL_LINE_STATE.
    Linux raises DTR on open anyway, but saying so here means the dependency is
    visible rather than inherited from tty defaults.
    """
    try:
        fcntl.ioctl(fd, termios.TIOCMBIS, struct.pack("I", TIOCM_DTR | TIOCM_RTS))
    except (OSError, AttributeError):
        pass  # not fatal; the open already did it on Linux


class SerialPort:
    """A raw, non-blocking reader for one tty.

    Usage is open() / read() until read() raises Disconnected / close().
    """

    class Disconnected(Exception):
        pass

    def __init__(self, path: str, baud: int = 115200) -> None:
        self.path = path
        self.baud = baud
        self._fd: int | None = None

    def open(self) -> None:
        fd = os.open(self.path, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
        try:
            _configure_raw(fd, self.baud)
            _assert_dtr(fd)
        except Exception:
            os.close(fd)
            raise
        self._fd = fd

    def close(self) -> None:
        if self._fd is not None:
            try:
                os.close(self._fd)
            finally:
                self._fd = None

    def read(self, timeout: float = 0.2) -> bytes:
        """Return whatever has arrived, or b"" on timeout.

        Raises Disconnected when the device goes away. Unplugging shows up as
        EIO or ENODEV from read(), or as a zero-length read -- all three mean
        the same thing here.
        """
        if self._fd is None:
            raise self.Disconnected("port is not open")
        try:
            ready, _, _ = select.select([self._fd], [], [], timeout)
        except OSError as exc:
            raise self.Disconnected(str(exc)) from exc
        if not ready:
            return b""
        try:
            chunk = os.read(self._fd, 4096)
        except OSError as exc:
            if exc.errno in (errno.EAGAIN, errno.EWOULDBLOCK):
                return b""
            raise self.Disconnected(os.strerror(exc.errno)) from exc
        if not chunk:
            raise self.Disconnected("end of stream")
        return chunk

    def __enter__(self) -> "SerialPort":
        self.open()
        return self

    def __exit__(self, *_exc: object) -> None:
        self.close()


class LineAssembler:
    """Turns arbitrary read chunks into complete lines.

    A USB bulk packet boundary has nothing to do with a line boundary, so a
    log line routinely arrives split across two reads.
    """

    def __init__(self, max_pending: int = 64 * 1024) -> None:
        self._buf = bytearray()
        self._max_pending = max_pending

    def feed(self, chunk: bytes) -> list[str]:
        self._buf.extend(chunk)
        if len(self._buf) > self._max_pending:
            # A firmware that never emits a newline must not grow this forever.
            del self._buf[: len(self._buf) - self._max_pending]
        lines: list[str] = []
        while True:
            idx = self._buf.find(b"\n")
            if idx < 0:
                break
            raw = bytes(self._buf[:idx])
            del self._buf[: idx + 1]
            lines.append(raw.rstrip(b"\r").decode("utf-8", errors="replace"))
        return lines

    def has_pending(self) -> bool:
        return bool(self._buf)

    def pending_len(self) -> int:
        return len(self._buf)

    def flush(self) -> str | None:
        """Take whatever is buffered without waiting for a newline.

        Firmware that prints without a trailing \\n -- or that truncates its
        own output before the newline -- would otherwise be invisible, which
        is the exact failure `cat` gives you and the reason this tool exists.
        """
        if not self._buf:
            return None
        text = bytes(self._buf).rstrip(b"\r").decode("utf-8", errors="replace")
        self._buf.clear()
        return text or None
