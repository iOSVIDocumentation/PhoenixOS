# PhoenixOS 

[![Version](https://img.shields.io/badge/version-v0.9.6-blue)](../../releases)
[![MCU](https://img.shields.io/badge/MCU-RP2350%20%7C%20dual%20Cortex--M33-green)](https://www.raspberrypi.com/products/rp2350/)
[![Language](https://img.shields.io/badge/language-C99-orange)]()
[![License](https://img.shields.io/badge/license-MIT-lightgrey)](#license)

**A retro-style workstation OS for the Raspberry Pi Pico 2.**
Windows, themes, games, a media player and a file manager on a $6 board —
flash one UF2 file and get a tiny desktop computer.

> **Quick start:** download `PhoenixOS.uf2` from
> [Releases](../../releases) → hold **BOOTSEL** → copy the file to the
> `RPI-RP2` drive → done. On an empty SD card the OS creates all folders
> and configs by itself.

**Contents**

- [What's New in v0.9.6](#whats-new-in-v096)
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

## What's New in v0.9.6

*Includes everything from v0.9.5.*

- 🎨 **Theme engine** — Phoenix / Windows XP / Mac OS Classic. Every window,
  menu and the boot screen follow the active theme; live switching.
- 🖼️ **Per-theme desktop icons** stored on the SD card
  (`/themes/<name>/icons.rgb`); missing sets are generated automatically,
  your files are never overwritten.
- 🎮 **Games menu** — themed Snake (Classic + Maze, records) and the new
  **Tetris** (Easy / Medium, hold-to-move, dual rotation, hard drop).
- 💾 **SD auto-provisioning** — empty card? The OS creates videos/,
  wallpapers/, themes/, config.txt, phoenix.cfg (stock 150 MHz) and
  snake.hi by itself.
- 🛡️ **Reliability** — cached SD statistics (no more watchdog timeout in
  "About System"), dynamic registration fix, smooth rendering.
- 📖 **Docs & packaging** — English README, pinout table, MIT license,
  source archive in the release.

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
  player, Snake, Tetris, Wolfenstein-style 3D demo.
- **Safety:** watchdog, core-1 heartbeat supervision, thermal rollback to
  150 MHz, safe mode (hold BACK at power-on).
- **Performance:** stock 150 MHz or overclock 200 / 225 / 250 MHz,
  persisted in config.

---

## Hardware & Pinout

| Block | Interface | Notes |
|---|---|---|
| Display | ST7789 320×240 | SPI0, 10–48 MHz, PWM backlight |
| SD card | SPI1 | 12.5 MHz (25 MHz is not stable on this wiring) |
| Joystick | ADC + GPIO | analog X/Y + press switch |
| Buttons | 3 × GPIO | MENU / OK / BACK |
| Buzzer | PWM | owned exclusively by core 1 |

<details>
<summary>📌 <b>Pinout — who connects where (open table)</b></summary>

| Component | Signal (board.h) | Pico pin |
|---|---|---|
| ST7789 display (SPI0) | `PIN_LCD_DC` | **GP16** |
| ST7789 display (SPI0) | `PIN_LCD_CS` | **GP17** |
| ST7789 display (SPI0) | `PIN_LCD_SCK` | **GP18** |
| ST7789 display (SPI0) | `PIN_LCD_MOSI` | **GP19** |
| ST7789 display (SPI0) | `PIN_LCD_RST` | **GP20** |
| ST7789 display (SPI0) | `PIN_LCD_BLK` | **GP21** |
| SD card (SPI1) | `PIN_SD_SCK` | **GP10** |
| SD card (SPI1) | `PIN_SD_MOSI` | **GP11** |
| SD card (SPI1) | `PIN_SD_MISO` | **GP12** |
| SD card (SPI1) | `PIN_SD_CS` | **GP13** |
| Joystick | `JOY_SW_PIN` | **GP5** |
| Joystick | `JOY_X_PIN` | **GP26** |
| Joystick | `JOY_Y_PIN` | **GP27** |
| Buzzer | `PIN_BUZZER` | **GP7** |

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
    |   +-- macos/icons.rgb
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
                   wallpaper, cpu, media, snake, tetris, wolf3d,
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
<summary>🕰️ <b>v0.9.6 / v0.9.5 / v0.9.3 (open)</b></summary>

### v0.9.6
- README hardening: badges, TOC, collapsible sections, pinout table,
  MIT license.
- Release ships the complete source archive alongside the UF2.

### v0.9.5
- Theme engine (Phoenix / Windows XP / Mac OS Classic), live switching.
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

PhoenixOS is open-source software released under the **MIT License** —
see the [LICENSE](LICENSE) file.

Copyright (c) 2026 iOSVIDocumentation (PhoenixOS project).
