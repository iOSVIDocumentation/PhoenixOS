# PhoenixOS

[![Version](https://img.shields.io/badge/version-v1.0--beta1-blueviolet)](../../releases)
[![MCU](https://img.shields.io/badge/MCU-RP2350%20%7C%20dual%20Cortex--M33-green)](https://www.raspberrypi.com/products/rp2350/)
[![Language](https://img.shields.io/badge/language-C99-orange)]()
[![License](https://img.shields.io/badge/license-Apache--2.0-blue)](#license)

**A retro-style workstation OS for the Raspberry Pi Pico 2.**
Windows, themes, games, a media player and a file manager on a $6 board —
flash one UF2 file and get a tiny desktop computer.

> **Quick start:** download `PhoenixOS.uf2` from
> [Releases](../../releases) → hold **BOOTSEL** → copy the file to the
> `RPI-RP2` drive → done. On an empty SD card the OS creates all folders
> and configs by itself.

**Contents**

- [What's New in v1.0-beta1](#whats-new-in-v10-beta1)
- [Roadmap / Future Plans](#roadmap--future-plans)
- [Feature Highlights](#feature-highlights)
- [Hardware & Pinout](#hardware--pinout)
- [Controls](#controls)
- [SD Card Layout](#sd-card-layout)
- [Repository Structure](#repository-structure)
- [Building from Source](#building-from-source)
- [Flashing](#flashing)
- [Changelog](#changelog)
- [License](#license)

---

## Download

| Asset | Link |
| --- | --- |
| Firmware (UF2, flash-ready) | https://github.com/iOSVIDocumentation/PhoenixOS/releases/latest/download/PhoenixOS.uf2 |
| Full source code (ZIP, v1.0-beta1) | https://github.com/iOSVIDocumentation/PhoenixOS/archive/refs/tags/v1.0-beta1.zip |
| Full source code (ZIP, main branch) | https://github.com/iOSVIDocumentation/PhoenixOS/archive/refs/heads/main.zip |
| All releases | https://github.com/iOSVIDocumentation/PhoenixOS/releases |

## Building from source

### 1. Clone with submodules

```bash
git clone --recursive https://github.com/iOSVIDocumentation/PhoenixOS.git
cd PhoenixOS
```

### 2. Apply RP2350 compatibility patches

Upstream FatFs targets RP2040 and pulls in hardware RTC, which RP2350 does not have.
Apply the bundled patch once after cloning:

```bash
./tools/apply_patches.sh
```

### 3. Build

```bash
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

Firmware: build/PhoenixOS.uf2

## What's New in v1.0-beta1

**Major milestone** — first beta of the 1.0 line with a new 3D demo app, display performance boost and gameplay fixes.

### New Apps
- 🎲 **3D Test** — rotating wireframe cube with real-time FPS counter.
  - Full 320×240 software framebuffer.
  - Dynamic RAM allocation (`malloc`/`free`) — memory is held only while the app is active.
  - Perspective projection, 3-axis rotation, Bresenham line drawing.
  - Stable on-screen text (rendered into the framebuffer, no flicker).

### Display & Drivers
- **SPI0 baudrate raised from 10 MHz to 40 MHz** (stable) — display writes are ~4× faster.
- Eliminated the white flash on boot and on app exit (fixed `st7789_init()` ordering: framebuffer is filled with black **before** the display command `0x29` turns the panel on).
- New driver helper `st7789_write_full_frame()` for single bulk SPI transfers (used by future apps).

### Games
- 🐍 **Snake**: fixed impossible deaths near food — eating now requires an exact head-food overlap instead of a 1-cell radius. Tail-exclusion logic for self-collision is now correct again.

### System
- **Version bumped to v1.0-beta1** (displayed in the About window).
- All local commits are now pushed to origin and a full source archive ships with the release.

---

## Roadmap / Future Plans

What we want to tackle in upcoming releases (post-1.0 GA):

- 🏎 **Pseudo-3D racing game** — OutRun-style segment-based road rendering with scaling roadside sprites, targeting a stable 30 fps in the 160×120 doubled mode.
- 🎬 **Media player overhaul** — RLE-compressed PVX v2 frames, core1 read-ahead buffer, and a proper `tools/pvx_pack.py` converter so users don't have to run a custom pipeline.
- 🕹 **Raycaster engine (Wolf3D-style)** — textured walls in 160×120 at 20–30 fps; brings back the spirit of early 90s 3D shooters within our hardware limits.
- 🔧 **DMA-driven SPI** for the display — offload framebuffer flush to DMA, freeing core 0 for more work.
- 📦 **Improved SD handling and caching** — further reduce UI stalls during media/file-manager use.

---

## Feature Highlights

- **Dual-core kernel**
  - *Core 0 (master):* 100 Hz loop, input, ST7789 rendering, apps,
    hardware watchdog (1.5 s), thermal guard (65 °C).
  - *Core 1 (service):* PVX media stream (30 fps), PWM sound service,
    heartbeat.
  - *IPC:* multicore FIFO (media), lock-free queue (sound), FatFs mutex.
- **Apps:** desktop with cursor & icons, Start menu, file manager, viewer,
  control panel, wallpaper picker, CPU manager, system monitor, PVX media
  player, Snake and Tetris.
- **Safety:** watchdog, core-1 heartbeat supervision, thermal rollback to
  150 MHz, safe mode (hold BACK at power-on).
- **Performance:** stock 150 MHz or overclock 200 / 225 / 250 MHz,
  persisted in config.

---

## Hardware & Pinout

Target display module: **2.4" TFT SPI 240×320 v1.3** (with on-board SD slot).
Pico leg numbers assume USB up (left column 1–20 top-down, right column
21–40 bottom-up).

<details>
<summary>📌 <b>Pinout — who connects where (open table)</b></summary>

**Display module 2.4" TFT SPI 240x320 v1.3 - main header, pin order as silkscreened
("SPI" is a side marking, not a pin). 8 pins connected, touch pins unused:**

| # | Module pin (silkscreen) | Pico leg | GPIO | Status |
|---|---|---|---|---|
| 1 | VCC | 36 | 3V3 | connected |
| 2 | GND | 38 | GND | connected |
| 3 | CS | 22 | GP17 | connected |
| 4 | RESET | 26 | GP20 | connected |
| 5 | DC | 21 | GP16 | connected |
| 6 | SDI (MOSI) | 25 | GP19 | connected |
| 7 | SCK | 24 | GP18 | connected |
| 8 | LED | 27 | GP21 | connected, PWM backlight |
| 9 | SDO (MISO) | - | - | NOT connected |
| 10 | T_CLK | - | - | NOT connected (touch) |
| 11 | T_CS | - | - | NOT connected (touch) |
| 12 | T_DIN | - | - | NOT connected (touch) |
| 13 | T_DO | - | - | NOT connected (touch) |
| 14 | T_IRQ | - | - | NOT connected (touch) |

**SD slot on the same module (silkscreen at the bottom):**

| Module pin | Pico leg | GPIO | Status |
|---|---|---|---|
| SD_SCK | 14 | GP10 | connected |
| SD_MOSI | 15 | GP11 | connected |
| SD_MISO | 16 | GP12 | connected |
| SD_CS | 17 | GP13 | connected |

**Other modules:**

| Module | Pin | Pico leg | GPIO | Notes |
|---|---|---|---|---|
| Joystick | VCC | 36 | 3V3 | |
| | GND | 23 | GND | |
| | VRx | 31 | GP26 | ADC0 |
| | VRy | 32 | GP27 | ADC1 |
| | SW | 7 | GP5 | press switch |
| Buttons | MENU | 4 | GP2 | second pin = GND |
| | OK | 5 | GP3 | second pin = GND |
| | BACK | 6 | GP4 | second pin = GND |
| Buzzer | + | 10 | GP7 | PWM |
| | - | - | GND | |
| Power | +5V | 39 | VSYS | or USB |
| | GND | 38 | GND | common ground |

Full wiring notes: `PINOUT.md`.

</details>

---

## Controls

| Input | Action |
|---|---|
| Joystick | cursor / lists / Snake & Tetris movement |
| Joystick press (SW) | OK (+ piece rotation in Tetris) |
| MENU | Start menu (+ piece rotation in Tetris) |
| OK | confirm / hard drop in Tetris |
| BACK | return / save (hold at power-on = safe mode) |

---

## SD Card Layout

<details>
<summary>📁 <b>What the OS creates on the card (open)</b></summary>

    /
    |-- videos/          PVX movies for the media player
    |-- wallpapers/      *.rgb wallpapers (320x240, RGB565 BE)
    |-- themes/
    |   |-- phoenix/icons.rgb   per-theme desktop icon sets
    |   |-- xp/icons.rgb        (5 icons, 32x32, RGB565 BE,
    |   |                       magenta = transparent)
    |   +-- terminal/icons.rgb
    |-- config.txt       board marker ("System OK")
    |-- phoenix.cfg      settings (safe defaults, auto-created)
    |-- snake.hi         Snake high scores (classic / maze)
    +-- reset.log        boot/reset journal (diagnostics)

`phoenix.cfg` keys: `bright` (25/50/75/100), `cursor` (1–3),
`sound` (on/off), `cpu` (150/200/225/250), `theme` (0/1/2),
`wallpaper` (path or empty).

</details>

---

## Repository Structure

<details>
<summary>🗂️ <b>Source tree (open)</b></summary>

    main.c         boot screen, init, app registration
    core/          kernel: registry, main loop, watchdog, heartbeat,
                   FatFs mutex, sound service, SD provisioning
    apps/          desktop, start menu, files, viewer, settings,
                   wallpaper, cpu, media, snake, tetris,
                   games menu
    drivers/       board.h (single source of truth for pins),
                   st7789, buzzer, joystick, buttons
    modules/       ui (themed renderer), settings, themes, theme icons,
                   wallpaper, files, viewer, sysinfo, sound
    fatfs_lib/     FatFs_SPI submodule

</details>

---

## Building from Source

Requirements: Linux (Fedora tested), `cmake`, `arm-none-eabi-gcc`,
Pico SDK 2.3.0.

    git clone --recurse-submodules https://github.com/iOSVIDocumentation/PhoenixOS
    cd PhoenixOS
    export PICO_SDK_PATH=$HOME/pico/pico-sdk
    cmake -S . -B build
    cmake --build build -j$(nproc)

Firmware image: `build/PhoenixOS.uf2`.

## Flashing

Hold **BOOTSEL**, plug the board in, copy `build/PhoenixOS.uf2` to the
`RPI-RP2` drive.

---

## Changelog

<details>
<summary>🕰️ <b>v1.0-beta1 / v0.9.8 / v0.9.7 / v0.9.6 / v0.9.5 / v0.9.3 (open)</b></summary>

### v1.0-beta1

- New app: **3D Test** (rotating wireframe cube, 320×240 framebuffer, dynamic RAM, FPS counter).
- SPI0 baudrate raised 10 → 40 MHz: ~4× faster display writes.
- White flash on boot and on app exit eliminated (init order fix).
- Snake: impossible deaths near food fixed (exact head-food overlap required).
- Version bumped to v1.0-beta1 (About window).

### v0.9.8

- Stability & safety pass: 18 surgical fixes after a full source audit (kernel, drivers, apps, build system).
- RP2350 FatFs compatibility patch + tools/apply_patches.sh; build instructions added to README.
- Cursor: subpixel frame-rate-independent movement; joystick SW pin debounce 20 ms.
- Sound: UI clicks keep priority during PVX playback (ducking); snd_success() non-blocking.
- Dead code removed: Wolf3D raycaster app, snd_shoot(), sd_card.c.
- Full list: see "What's New in v0.9.8" above.

### v0.9.7
- Snake field fully black; maze walls unchanged.

### v0.9.6
- Exact pinout table matching the real module silkscreen
  (2.4" TFT v1.3, 8 connected display pins + on-board SD slot).
- README hardening: badges, TOC, collapsible sections.
- License changed to Apache 2.0.
- Release ships the complete source archive alongside the UF2.

### v0.9.5
- Theme engine (Phoenix / Windows XP / CRT Terminal), live switching.
- Per-theme SD icons, auto-generated missing sets.
- Games menu + Tetris (2 modes, dual rotation, hold-to-move).
- Full SD auto-provisioning for empty cards.
- Cached SD statistics (About watchdog fix); dyn-registration fix;
  smoother rendering; themed boot screen.

### v0.9.3
- FatFs inter-core mutex, watchdog, core-1 heartbeat, thermal guard.
- Sound service on core 1 (no UI stutter).
- Snake: 2 modes + records; media player 30 fps; real RAM/SD/clock
  readouts; smooth backlight; unified board.h; 250 MHz validated.

</details>

---

## License

PhoenixOS is released under the **Apache License, Version 2.0** —
see the [LICENSE](LICENSE) file.

Copyright 2026 iOSVIDocumentation (PhoenixOS project).

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this work except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. See the License for the specific language governing
permissions and limitations under the License.
