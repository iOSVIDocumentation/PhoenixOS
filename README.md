# PhoenixOS

[![Version](https://img.shields.io/badge/version-v0.9.6-blue)](../../releases)
[![MCU](https://img.shields.io/badge/MCU-RP2350%20%7C%20dual%20Cortex--M33-green)](https://www.raspberrypi.com/products/rp2350/)
[![Language](https://img.shields.io/badge/language-C99-orange)]()
[![License](https://img.shields.io/badge/license-MIT-lightgrey)](#license)

A retro-style workstation operating system for the **Raspberry Pi Pico 2**
(RP2350, dual-core ARM Cortex-M33). PhoenixOS turns a $6 microcontroller
board into a tiny desktop computer: a windowed UI with themes, a file
manager, a PVX media player, games, SD-backed configuration and a
safety-monitored dual-core kernel.

Current version: **v0.9.6**

---

## What's New in v0.9.6 (includes everything from v0.9.5)

- **Theme engine**: Phoenix, Windows XP and Mac OS Classic skins.
  Every window, menu and the boot screen follow the active theme;
  live switching in the Control Panel.
- **Per-theme desktop icons stored on the SD card**
  (/themes/<name>/icons.rgb, 5 icons 32x32, RGB565, magenta = transparent).
  Missing icon sets are generated automatically on first boot; user
  files are never overwritten.
- **Games menu** with Snake (Classic + Maze, persistent records, themed
  UI) and the new **Tetris** (Easy / Medium, hold-to-move, dual rotation:
  MENU button + joystick press, OK = hard drop).
- **Full SD auto-provisioning**: on an empty card the OS creates videos/,
  wallpapers/, themes/, config.txt, phoenix.cfg (stock 150 MHz, no
  overclock) and snake.hi by itself.
- **Reliability fixes**: SD statistics cached at boot (eliminates the
  watchdog timeout in "About System"); dynamic app registration fix;
  smooth row-based cursor/icon rendering.
- **Documentation**: professional English README, full GPIO pinout table,
  MIT license.
- **Packaging**: the release ships the complete source archive next to
  the UF2 firmware.

---

## Feature Highlights

- Dual-core architecture:
  - Core 0 (master): 100 Hz main loop, input, ST7789 rendering,
    applications, hardware watchdog (1.5 s), thermal guard (65 C).
  - Core 1 (service): PVX media stream (30 fps), PWM sound service,
    heartbeat counter.
  - IPC via multicore FIFO (media), lock-free queue (sound) and a FatFs
    re-entrancy mutex between cores.
- Applications: desktop with cursor and icons, Start menu, file manager,
  viewer, control panel, wallpaper picker, CPU frequency manager, system
  monitor, PVX media player, Snake, Tetris, Wolfenstein-style 3D demo.
- Safety: hardware watchdog, core-1 heartbeat supervision, thermal
  rollback to 150 MHz, safe mode (hold BACK at power-on), FatFs mutex.
- Performance: stock 150 MHz or user-selectable overclock up to 250 MHz,
  persisted in the config file.

## Hardware & Pinout

| Block | Interface | Notes |
|---|---|---|
| Display | ST7789 320x240, SPI0 | 10-48 MHz, PWM backlight |
| SD card | SPI1 | 12.5 MHz (25 MHz not stable on this wiring) |
| Joystick | ADC0 / ADC1 + GPIO | analog axes + press switch |
| Buttons | 3x GPIO | MENU / OK / BACK |
| Buzzer | PWM | owned exclusively by core 1 |
| Backlight | PWM | smooth 0-100% |

### GPIO Map (generated from drivers/board.h - source of truth)

| Signal | GPIO |
|---|---|
| PIN_LCD_SCK | GP18 |
| PIN_LCD_MOSI | GP19 |
| PIN_LCD_DC | GP16 |
| PIN_LCD_CS | GP17 |
| PIN_LCD_RST | GP20 |
| PIN_LCD_BLK | GP21 |
| PIN_SD_SCK | GP10 |
| PIN_SD_MOSI | GP11 |
| PIN_SD_MISO | GP12 |
| PIN_SD_CS | GP13 |
| JOY_X_PIN | GP26 |
| JOY_Y_PIN | GP27 |
| JOY_SW_PIN | GP5 |
| PIN_BUZZER | GP7 |

Wiring details and schematics notes: see PINOUT.md in this repository.

## Controls

| Input | Action |
|---|---|
| Joystick | cursor movement / list navigation / Snake and Tetris control |
| Joystick press (SW) | OK (and piece rotation in Tetris) |
| MENU | Start menu (piece rotation in Tetris) |
| OK | confirm / hard drop in Tetris |
| BACK | return / save settings (hold at power-on for safe mode) |

## SD Card Layout (created automatically)

    /
    |-- videos/          PVX movies for the media player
    |-- wallpapers/      *.rgb wallpapers (320x240, RGB565 BE)
    |-- themes/
    |   |-- phoenix/icons.rgb   per-theme desktop icon sets
    |   |-- xp/icons.rb         (5 icons, 32x32, RGB565 BE,
    |   |                       magenta = transparent)
    |   +-- macos/icons.rgb
    |-- config.txt       board marker ("System OK")
    |-- phoenix.cfg      system settings (safe defaults, auto-created)
    |-- snake.hi         Snake high scores (classic / maze)
    +-- reset.log        boot/reset journal (diagnostics)

phoenix.cfg keys: bright (25/50/75/100), cursor (1-3), sound (on/off),
cpu (150/200/225/250), theme (0/1/2), wallpaper (path or empty).

## Repository Structure

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

## Building from Source

Requirements: Linux (Fedora tested), cmake, arm-none-eabi-gcc,
Pico SDK 2.3.0.

    git clone --recurse-submodules https://github.com/iOSVIDocumentation/PhoenixOS
    cd PhoenixOS
    export PICO_SDK_PATH=$HOME/pico/pico-sdk
    cmake -S . -B build
    cmake --build build -j$(nproc)

The firmware image is build/PhoenixOS.uf2.

## Flashing

Hold BOOTSEL, plug the board in, then copy build/PhoenixOS.uf2 to the
RPI-RP2 drive.

## Changelog

### v0.9.6
- README rendering fix; badges, full GPIO pinout table, MIT license.
- Release ships the complete source archive alongside the UF2.
- Version housekeeping across UI, boot screen and docs.

### v0.9.5
- Theme engine: Phoenix / Windows XP / Mac OS Classic, live switching.
- Per-theme SD icons with automatic generation of missing sets.
- Games menu; Tetris with two difficulty modes and dual rotation buttons.
- Full SD auto-provisioning for empty cards.
- Boot-time cached SD statistics (watchdog-timeout fix in About).
- Dynamic registration fix; smoother rendering; themed boot screen.

### v0.9.3
- FatFs inter-core mutex, watchdog, core-1 heartbeat, thermal guard.
- Sound service on core 1 (no UI stutter).
- Snake: two modes + persistent records; media player 30 fps;
  real RAM/SD/clock readouts; smooth backlight; unified board.h;
  250 MHz overclock validated.

## License

PhoenixOS is released under the **MIT License** - see the [LICENSE](LICENSE)
file. Copyright (c) 2026 iOSVIDocumentation (PhoenixOS project).
