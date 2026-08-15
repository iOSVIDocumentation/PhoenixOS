<p align="center">
  <img src="https://img.shields.io/badge/PhoenixOS-v0.9.3-001F?style=for-the-badge&logo=raspberrypi&logoColor=white" />
  <img src="https://img.shields.io/badge/RP2350-Cortex--M33%20x2-A00000?style=for-the-badge" />
  <img src="https://img.shields.io/badge/250%20MHz-verified-07E0?style=for-the-badge" />
</p>

# 🦅 PhoenixOS — Retro Workstation для Raspberry Pi Pico 2

**Настоящая ОС** на микроконтроллере: рабочий стол в стиле Win95, файловый менеджер, медиаплеер 30 fps, игры, разгон до 250 МГц, двухъядерная архитектура.

## 🚀 Быстрый старт
1. Скачай `PhoenixOS.uf2` из [Releases](../../releases/latest)
2. Зажми **BOOTSEL**, подключи USB
3. Скопируй UF2 на диск `RPI-RP2`

SD-карта (FAT32), всё создаётся само:

    /
    ├── phoenix.cfg   <- настройки (авто-создание, cpu=250)
    ├── wallpapers/   <- обои .rgb
    ├── videos/       <- видео .pvx
    └── snake.hi      <- рекорды змейки

Конвертер видео:

    sudo dnf install -y ffmpeg
    ~/pvx video.mp4     # сам найдёт SD, положит в /videos/

## ✨ Возможности
- 🖥️ Рабочий стол: иконки My PC / Files / Settings / **Games** / Media, панель задач, обои
- 🎮 **Snake**: CLASSIC + MAZE (просторные лабиринты без замкнутых зон, flood-fill), рекорды на SD, ретро-графика
- 🎬 Медиаплеер v2: 30 fps, LCD 48 МГц, чиптюн-звук, точный темп
- ⚙️ Настройки: яркость 0–100%, курсор, звук, разгон 150–250 МГц, обои
- 📂 Файловый менеджер по SD (FAT32)
- 🔊 Звук-сервис на **core1** (PWM, без заиканий UI)

## 🔧 Железо
| Компонент | Интерфейс |
|---|---|
| Pico 2 (RP2350, M33 x2, 520 KB SRAM) | — |
| ST7789 320×240 | SPI0, 10 МГц (48 МГц в видео) |
| MicroSD | SPI1, 12.5 МГц |
| Джойстик + 3 кнопки | ADC + GPIO |
| Буззер | PWM |

📌 Полная распиновка — [`PINOUT.md`](PINOUT.md)

## ⚙️ Сборка

    export PICO_SDK_PATH=/path/to/pico-sdk
    git clone --recursive https://github.com/iOSVIDocumentation/PhoenixOS.git
    cd PhoenixOS && mkdir build && cd build
    cmake .. && make -j$(nproc)

На выходе `build/PhoenixOS.uf2`

## 🏗️ Архитектура
- **Core0**: ядро, ввод, UI, приложения, watchdog 1.5 c, thermal guard 65°C
- **Core1**: медиа-поток PVX + звук-сервис + heartbeat
- Связь: FIFO + mutex; FatFs `FF_FS_REENTRANT=1`
- Подробнее: [`ARCHITECTURE.md`](ARCHITECTURE.md)

## 🧩 Своё приложение за 5 минут
Скопируй `apps/_template.c`, в `main.c` одна строка:

    core_register_dyn(&app_myapp);

Открыть: `core_open(core_find("myapp"), 0);`

## 📊 Версии
| Версия | Главное |
|---|---|
| **v0.9.3** ⭐ | Snake + рекорды, звук на core1, медиа 30 fps, watchdog, sysinfo, автоконфиг |
| v0.9 | Первая публичная |

## 📄 Лицензия
**Apache License 2.0** — открытый код: используй и модифицируй свободно, но с сохранением авторства. Встроенная защита от патентных троллей. Полный текст — в [LICENSE](LICENSE).
