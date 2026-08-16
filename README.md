# PhoenixOS

A retro-style workstation operating system for the Raspberry Pi Pico 2
(RP2350, dual-core ARM Cortex-M33). PhoenixOS turns a $6 microcontroller
board into a tiny desktop computer: a windowed UI with themes, a file
manager, a PVX media player, games, SD-backed configuration and a
safety-monitored dual-core kernel.

Current version: **v0.9.6**

---

## What's New in v0.9.6

- Documentation hardening: the GitHub page now renders correctly
  (indented code blocks, no broken fences).
- Release packaging: v0.9.6 ships the complete source archive next to
  the UF2 firmware.
- All v0.9.5 features listed below remain included.

## What's New in v0.9.5

- **Theme engine** with three skins: Phoenix, Windows XP and Mac OS
  Classic. Every window, menu and the boot screen follow the active theme.
- **Per-theme desktop icons stored on the SD card**
  (/themes/<name>/icons.rgb, 5 icons 32x32, RGB565, magenta = transparent).
  Missing icon sets are generated automatically on first boot; user files
  are never overwritten.
- **Games menu** with Snake (Classic + Maze, persistent records) and the
  new **Tetris** (Easy / Medium, hold-to-move, two rotation buttons).
- **Full SD auto-provisioning**: on an empty card the OS creates videos/,
  wallpapers/, themes/, config.txt, phoenix.cfg and snake.hi with safe
  defaults (stock 150 MHz, no overclock).
- **Reliability fix**: SD statistics are cached at boot, eliminating the
  watchdog timeout in "About System".
- Fixed dynamic application registration; smoothed cursor and icon
  rendering; themed boot screen.
- Professional English documentation; release ships with full sources.

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

## Hardware Reference

| Block | Details |
|---|---|
| MCU | Raspberry Pi Pico 2 (RP2350), 520 KB SRAM |
| Display | ST7789 320x240, SPI0 @ 10-48 MHz (DC 16, CS 17, SCK 18, MOSI 19, RST 20, backlight 21 / PWM) |
| SD card | SPI1 @ 12.5 MHz (full pin map: PINOUT.md) |
| Joystick | X = GP26 (ADC0), Y = GP27 (ADC1), press = GP5 |
| Buttons | MENU = GP2, OK = GP3, BACK = GP4 |
| Buzzer | GP7 (PWM, owned exclusively by core 1) |
| Backlight | GP21, smooth 0-100% PWM |

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
    |   |-- xp/icons.rgb        (5 icons, 32x32, RGB565 BE,
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
- README rendering fix on GitHub.
- Release now includes the full source archive.
- Version housekeeping across UI, boot screen and documentation.

### v0.9.5
- Theme engine: Phoenix / Windows XP / Mac OS Classic, live switching.
- Per-theme SD icons with automatic generation of missing sets.
- Games menu; Tetris with two difficulty modes and dual rotation buttons.
- Full SD auto-provisioning for empty cards.
- Boot-time cached SD statistics (watchdog-timeout fix in About).
- Dynamic registration fix; smoother rendering; themed boot screen.
- English documentation; release includes full sources.

### v0.9.3
- FatFs inter-core mutex, watchdog, core-1 heartbeat, thermal guard.
- Sound service on core 1 (no UI stutter).
- Snake: two modes + persistent records; media player 30 fps;
  real RAM/SD/clock readouts; smooth backlight; unified board.h;
  250 MHz overclock validated.

## License

See the repository. (c) PhoenixOS project.
