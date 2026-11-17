"""nrflog -- a log viewer for the nRF52840 CDC-ACM port, with auto-reconnect.

Finds the dongle by USB VID:PID rather than a fixed device path, so replugging
is a no-op even when it comes back as a different ttyACM number.
"""

from __future__ import annotations

import argparse
import time
from datetime import datetime

from rich.text import Text
from textual import work
from textual.app import App, ComposeResult
from textual.binding import Binding
from textual.widgets import Footer, Header, RichLog, Static
from textual.worker import get_current_worker

from serial_port import DEFAULT_PID, DEFAULT_VID, LineAssembler, SerialPort, find_devices

RECONNECT_DELAY = 0.5
PARTIAL_FLUSH_AFTER = 0.3   # seconds of silence before showing an unterminated line
PARTIAL_FLUSH_BYTES = 160   # ...or this many bytes with still no newline in sight


class StatusBar(Static):
    """One line: where we are connected, and how much has come through."""

    def set_state(self, *, connected: bool, detail: str, lines: int, bytes_: int) -> None:
        text = Text()
        if connected:
            text.append(" CONNECTED ", style="bold white on dark_green")
        else:
            text.append(" WAITING   ", style="bold white on dark_red")
        text.append("  ")
        text.append(detail, style="bold" if connected else "dim")
        text.append(f"   {lines} lines / {bytes_} bytes", style="dim")
        self.update(text)


class NrfLogApp(App):
    CSS = """
    Screen { layout: vertical; }
    RichLog { height: 1fr; border: none; padding: 0 1; }
    StatusBar { height: 1; background: $panel; padding: 0 1; }
    """

    BINDINGS = [
        Binding("q", "quit", "Quit"),
        Binding("c", "clear", "Clear"),
        Binding("s", "save", "Save"),
        Binding("f", "toggle_follow", "Follow"),
    ]

    def __init__(self, vid: str, pid: str, baud: int, device: str | None) -> None:
        super().__init__()
        self.vid = vid
        self.pid = pid
        self.baud = baud
        self.device = device
        self.lines_seen = 0
        self.bytes_seen = 0
        self.follow = True
        self._history: list[str] = []
        self._connected = False
        self._detail = "searching…"

    def compose(self) -> ComposeResult:
        yield Header(show_clock=True)
        yield RichLog(highlight=False, markup=False, wrap=True, auto_scroll=True)
        yield StatusBar()
        yield Footer()

    def on_mount(self) -> None:
        self.title = "nrflog"
        self.sub_title = f"{self.vid}:{self.pid}"
        self._set_status(connected=False, detail="searching…")
        self.reader()

    # ---------------------------------------------------------------- UI ----

    def _set_status(self, *, connected: bool | None = None, detail: str | None = None) -> None:
        if connected is not None:
            self._connected = connected
        if detail is not None:
            self._detail = detail
        self.query_one(StatusBar).set_state(
            connected=self._connected,
            detail=self._detail,
            lines=self.lines_seen,
            bytes_=self.bytes_seen,
        )

    def _stamp(self) -> Text:
        return Text(datetime.now().strftime("%H:%M:%S.%f")[:-3] + " ", style="dim cyan")

    def append_line(self, line: str, partial: bool = False) -> None:
        self.lines_seen += 1
        self._history.append(line)
        text = self._stamp()
        if partial:
            # No newline arrived; shown anyway so output is never invisible.
            text.append(line, style="yellow")
            text.append(" ⋯", style="dim yellow")
        else:
            text.append(line)
        self.query_one(RichLog).write(text)

    def append_notice(self, message: str, style: str) -> None:
        text = self._stamp()
        text.append(f"── {message}", style=style)
        self.query_one(RichLog).write(text)

    def note_bytes(self, count: int) -> None:
        self.bytes_seen += count
        self._set_status()

    def set_connected(self, path: str) -> None:
        self._set_status(connected=True, detail=path)
        self.append_notice(f"connected to {path}", "bold green")

    def set_waiting(self, reason: str) -> None:
        self._set_status(connected=False, detail=reason)

    # ----------------------------------------------------------- actions ----

    def action_clear(self) -> None:
        self.query_one(RichLog).clear()
        self._history.clear()
        self.lines_seen = 0
        self.bytes_seen = 0

    def action_toggle_follow(self) -> None:
        self.follow = not self.follow
        log = self.query_one(RichLog)
        log.auto_scroll = self.follow
        if self.follow:
            log.scroll_end(animate=False)
        self.append_notice(
            f"follow {'on' if self.follow else 'off'}", "bold yellow"
        )

    def action_save(self) -> None:
        name = datetime.now().strftime("nrflog-%Y%m%d-%H%M%S.log")
        try:
            with open(name, "w", encoding="utf-8") as handle:
                handle.write("\n".join(self._history))
                handle.write("\n")
        except OSError as exc:
            self.append_notice(f"save failed: {exc}", "bold red")
        else:
            self.append_notice(f"saved {len(self._history)} lines to {name}", "bold green")

    # ------------------------------------------------------------ reader ----

    @work(thread=True, exclusive=True, group="serial")
    def reader(self) -> None:
        """Connect, read until the device goes away, repeat.

        Runs on a worker thread, so every touch of the UI goes through
        call_from_thread.
        """
        worker = get_current_worker()
        assembler = LineAssembler()
        announced_wait = False

        while not worker.is_cancelled:
            path = self.device
            if path is None:
                devices = find_devices(self.vid, self.pid)
                path = devices[0].path if devices else None

            if path is None:
                if not announced_wait:
                    self.call_from_thread(
                        self.set_waiting, f"no {self.vid}:{self.pid} device"
                    )
                    announced_wait = True
                time.sleep(RECONNECT_DELAY)
                continue

            port = SerialPort(path, self.baud)
            try:
                port.open()
            except OSError as exc:
                # Typically a race with udev: the node exists but permissions
                # or the driver are not ready yet. Retrying is the whole point.
                if not announced_wait:
                    self.call_from_thread(self.set_waiting, f"{path}: {exc.strerror}")
                    announced_wait = True
                time.sleep(RECONNECT_DELAY)
                continue

            announced_wait = False
            self.call_from_thread(self.set_connected, path)

            last_data = time.monotonic()
            try:
                while not worker.is_cancelled:
                    try:
                        chunk = port.read(timeout=0.1)
                    except SerialPort.Disconnected as exc:
                        tail = assembler.flush()
                        if tail:
                            self.call_from_thread(self.append_line, tail, True)
                        self.call_from_thread(
                            self.append_notice, f"disconnected ({exc})", "bold red"
                        )
                        self.call_from_thread(self.set_waiting, "reconnecting…")
                        break

                    if chunk:
                        last_data = time.monotonic()
                        self.call_from_thread(self.note_bytes, len(chunk))
                        for line in assembler.feed(chunk):
                            self.call_from_thread(self.append_line, line)

                    # Show unterminated output rather than holding it hostage
                    # the way cat does. Two triggers, because a device can be
                    # quiet (idle) or chatty but newline-free (over length).
                    idle = time.monotonic() - last_data > PARTIAL_FLUSH_AFTER
                    too_long = assembler.pending_len() >= PARTIAL_FLUSH_BYTES
                    if assembler.has_pending() and (idle or too_long):
                        tail = assembler.flush()
                        if tail:
                            self.call_from_thread(self.append_line, tail, True)
            finally:
                port.close()

            time.sleep(RECONNECT_DELAY)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="nRF52840 log viewer with auto-reconnect")
    parser.add_argument("--vid", default=DEFAULT_VID, help="USB vendor ID (hex, no 0x)")
    parser.add_argument("--pid", default=DEFAULT_PID, help="USB product ID (hex, no 0x)")
    parser.add_argument("--baud", type=int, default=115200, help="ignored by CDC, set anyway")
    parser.add_argument(
        "--device",
        default=None,
        help="pin to one path (e.g. /dev/ttyACM0) instead of matching on VID:PID",
    )
    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()
    NrfLogApp(vid=args.vid, pid=args.pid, baud=args.baud, device=args.device).run()
