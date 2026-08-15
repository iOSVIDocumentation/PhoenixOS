# PhoenixOS — архитектура ядра v1.1

## Ядра
- **Core0 (master):** главный цикл (~100 Гц, sleep 10 мс), ввод (джойстик+кнопки),
  UI-рендер (ST7789), приложения, FS для интерфейса, thermal-guard, watchdog.
- **Core1 (service):** медиа-поток PVX (чтение кадров в двойной буфер),
  звуковой движок (PWM буззер), heartbeat.

## Связь между ядрами (единственные каналы)
1. **multicore FIFO** — медиа-протокол: CMD_OPEN / CMD_READ / CMD_STOP,
   строгий запрос-ответ (app_media.c).
2. **Очередь sound_svc + mutex** — fire-and-forget звуки с core0;
   PWM буззера принадлежит ТОЛЬКО core1. Приоритет: звук видео глушит UI-звуки.
3. **g_core1_heartbeat** — счётчик живости; core0 проверяет 1 раз/сек,
   3 сек простоя = перезагрузка.

## Файловая система
FatFs (FF_FS_REENTRANT=1) + ff_mutex_* на pico mutex (core/fs_mutex.c).
Безопасны одновременные вызовы f_* с обоих ядер.

## Защита от зависаний
- watchdog 1500 мс, кормится в главном цикле.
- Перегрев >65C: cpu_mhz=150 + settings_save + reboot.
- Валидация регистрации/переключения приложений (NULL и кривые id игнорируются).

## Приложения
Интерфейс: phoenix_app_t { name, on_enter(arg), on_tick(in, delta_ms), on_exit }.
Ввод: core_input_t (nav_*, ok/back/start, sw). Время: core_now_ms().
Регистрация: системные — core_register(APP_x, &app_x);
пользовательские — core_register_dyn(&app); поиск — core_find("name").
Открытие: core_open(id, arg).

## Как добавить приложение (5 минут)
1. cp apps/_template.c apps/app_myapp.c, правь под себя.
2. Добавь apps/app_myapp.c в add_executable(CMakeLists.txt).
3. В main.c: extern const phoenix_app_t app_myapp; core_register_dyn(&app_myapp);
4. Открыть: core_open(core_find("myapp"), 0);

## Конфиг /phoenix.cfg (SD, FAT32)
bright=0..100 | cursor=1..3 | sound=on/off | cpu=100..300 | wallpaper=/path

## Железо
Смотри PINOUT.md. Дисплей SPI0 10 МГц (в видео 24 МГц), SD SPI1 12.5 МГц.
