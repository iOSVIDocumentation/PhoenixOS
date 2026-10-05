#ifndef LOGGER_H
#define LOGGER_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    LOG_LVL_TRACE = 0,
    LOG_LVL_DEBUG,
    LOG_LVL_INFO,
    LOG_LVL_WARN,
    LOG_LVL_ERROR,
    LOG_LVL_FATAL
} log_level_t;

typedef enum {
    LOG_SUB_SYS = 0,
    LOG_SUB_BOOT,
    LOG_SUB_CORE,
    LOG_SUB_WDT,
    LOG_SUB_THERMAL,
    LOG_SUB_CFG,
    LOG_SUB_SD,
    LOG_SUB_FILES,
    LOG_SUB_MEDIA,
    LOG_SUB_DISPLAY,
    LOG_SUB_SOUND,
    LOG_SUB_VIEWER,
    LOG_SUB_WALLPAPER,
    LOG_SUB_APP,
    LOG_SUB_INPUT,
    LOG_SUB_COUNT
} log_subsys_t;

uint8_t logger_core_id(void);
void logger_init(void);
void logger_tick(void);
void logger_flush_now(void);
bool logger_ready(void);
void logger_set_min_level(log_level_t level);
log_level_t logger_get_min_level(void);
void logger_suspend_sd(void);
void logger_resume_sd(void);
void logger_write(uint8_t core, log_level_t level, log_subsys_t subsys, const char *fmt, ...);
const char *log_level_name(log_level_t level);
const char *log_subsys_name(log_subsys_t subsys);

#define LOG_TRACE(sub, fmt, ...)  logger_write(logger_core_id(), LOG_LVL_TRACE, (sub), (fmt), ##__VA_ARGS__)
#define LOG_DEBUG(sub, fmt, ...)  logger_write(logger_core_id(), LOG_LVL_DEBUG, (sub), (fmt), ##__VA_ARGS__)
#define LOG_INFO(sub, fmt, ...)   logger_write(logger_core_id(), LOG_LVL_INFO,  (sub), (fmt), ##__VA_ARGS__)
#define LOG_WARN(sub, fmt, ...)   logger_write(logger_core_id(), LOG_LVL_WARN,  (sub), (fmt), ##__VA_ARGS__)
#define LOG_ERROR(sub, fmt, ...)  logger_write(logger_core_id(), LOG_LVL_ERROR, (sub), (fmt), ##__VA_ARGS__)
#define LOG_FATAL(sub, fmt, ...)  logger_write(logger_core_id(), LOG_LVL_FATAL, (sub), (fmt), ##__VA_ARGS__)

#endif // LOGGER_H
