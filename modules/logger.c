#include "logger.h"
#include "ff.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define LOG_RING_BYTES 16384u
#define LOG_RING_MASK (LOG_RING_BYTES - 1u)
#define LOG_CHUNK_BYTES 2048u
#define LOG_RECORD_BYTES 256u
#define LOG_MSG_BYTES 176u
#define LOG_MAX_FILE_BYTES (128u * 1024u)
#define LOG_FLUSH_INTERVAL_MS 250u
#define LOG_HIGH_WATER (LOG_RING_BYTES * 3u / 4u)

typedef struct {
    volatile uint32_t head;
    volatile uint32_t tail;
    volatile uint32_t dropped;
    char buf[LOG_RING_BYTES];
} log_ring_t;

static log_ring_t g_rings[2];
static bool g_initialized = false;
static bool g_file_open = false;
static volatile bool g_urgent = false;
static volatile bool g_suspended = false;
static FIL g_file;
static uint32_t g_bytes_written = 0;
static uint32_t g_last_flush = 0;
static log_level_t g_min_level = LOG_LVL_DEBUG;

static const char *g_level_names[] = {"TRACE", "DEBUG", "INFO ", "WARN ", "ERROR", "FATAL"};
static const char *g_sub_names[] = {
    "SYS",
    "BOOT",
    "CORE",
    "WDT",
    "THERMAL",
    "CFG",
    "SD",
    "FILES",
    "MEDIA",
    "DISPLAY",
    "SOUND",
    "VIEWER",
    "WALLPAP",
    "APP",
    "INPUT"
};

uint8_t logger_core_id(void) {
    return (uint8_t)get_core_num();
}

const char *log_level_name(log_level_t level) {
    if ((int)level < 0 || level >= LOG_LVL_FATAL + 1) return "?";
    return g_level_names[level];
}

const char *log_subsys_name(log_subsys_t subsys) {
    if ((int)subsys < 0 || subsys >= LOG_SUB_COUNT) return "?";
    return g_sub_names[subsys];
}

static void ring_write(log_ring_t *r, uint32_t pos, const void *src, uint16_t n) {
    if (n == 0) return;
    pos &= LOG_RING_MASK;
    uint16_t first = (uint16_t)(LOG_RING_BYTES - pos);
    const uint8_t *s = (const uint8_t *)src;
    uint8_t *d = (uint8_t *)r->buf;

    if (first >= n) {
        memcpy(d + pos, s, n);
    } else {
        memcpy(d + pos, s, first);
        memcpy(d, s + first, n - first);
    }
}

static void ring_read(log_ring_t *r, uint32_t pos, void *dst, uint16_t n) {
    if (n == 0) return;
    pos &= LOG_RING_MASK;
    uint16_t first = (uint16_t)(LOG_RING_BYTES - pos);
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)r->buf;

    if (first >= n) {
        memcpy(d, s + pos, n);
    } else {
        memcpy(d, s + pos, first);
        memcpy(d + first, s, n - first);
    }
}

static bool ring_enqueue(log_ring_t *r, const char *data, uint16_t len) {
    uint16_t total = (uint16_t)(len + 2u);
    uint32_t h = r->head;
    uint32_t used = h - r->tail;

    if (LOG_RING_BYTES - used < total) {
        r->dropped++;
        return false;
    }

    uint8_t hdr[2] = {
        (uint8_t)(len & 0xFFu),
        (uint8_t)((len >> 8) & 0xFFu)
    };

    ring_write(r, h, hdr, 2);
    ring_write(r, h + 2u, data, len);
    r->head = h + total;
    return true;
}

static bool ring_dequeue(log_ring_t *r, char *out, uint16_t out_size, uint16_t *len_out) {
    uint32_t h = r->head;
    uint32_t t = r->tail;
    if (h == t) return false;

    uint8_t hdr[2];
    ring_read(r, t, hdr, 2);
    uint16_t len = (uint16_t)(hdr[0] | (hdr[1] << 8));

    if (len == 0 || len > out_size) {
        // Corrupted record: drop the whole ring to stay alive.
        r->tail = r->head;
        return false;
    }

    ring_read(r, t + 2u, out, len);
    *len_out = len;
    r->tail = t + 2u + len;
    return true;
}

static void logger_rotate_locked(void) {
    if (g_file_open) {
        f_close(&g_file);
        g_file_open = false;
    }

    f_rename("/logs/system.log.2", "/logs/system.log.3");
    f_rename("/logs/system.log.1", "/logs/system.log.2");
    f_rename("/logs/system.log", "/logs/system.log.1");

    g_bytes_written = 0;
}

static bool logger_try_open(void) {
    if (g_file_open) return true;

    f_mkdir("/logs");

    FILINFO fi;
    bool exists = (f_stat("/logs/system.log", &fi) == FR_OK);

    if (exists && fi.fsize >= LOG_MAX_FILE_BYTES) {
        logger_rotate_locked();
        exists = false;
    }

    FRESULT fr = f_open(&g_file, "/logs/system.log", FA_WRITE | FA_OPEN_APPEND);
    if (fr != FR_OK) {
        return false;
    }

    g_file_open = true;
    g_bytes_written = exists ? (uint32_t)fi.fsize : 0u;
    return true;
}

void logger_init(void) {
    memset(g_rings, 0, sizeof(g_rings));
    g_initialized = true;
    g_file_open = false;
    g_urgent = false;
    g_suspended = false;
    g_bytes_written = 0;
    g_last_flush = 0;
    g_min_level = LOG_LVL_DEBUG;
    logger_try_open();
}

bool logger_ready(void) {
    return g_initialized && g_file_open;
}

void logger_set_min_level(log_level_t level) {
    g_min_level = level;
}

log_level_t logger_get_min_level(void) {
    return g_min_level;
}

void logger_suspend_sd(void) {
    g_suspended = true;
}

void logger_resume_sd(void) {
    g_suspended = false;
    g_urgent = true;
}

void logger_write(uint8_t core, log_level_t level, log_subsys_t subsys, const char *fmt, ...) {
    if (!g_initialized) return;
    if (level < g_min_level) return;

    char msg[LOG_MSG_BYTES];
    va_list ap;
    va_start(ap, fmt);
    int m = vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);

    if (m < 0) return;
    if (m >= (int)sizeof(msg)) m = (int)sizeof(msg) - 1;
    msg[m] = 0;

    char rec[LOG_RECORD_BYTES];
    int n = snprintf(
        rec,
        sizeof(rec),
        "[%lu ms][c%u][%s][%s] %s",
        (unsigned long)to_ms_since_boot(get_absolute_time()),
        (unsigned)(core & 1u),
        log_level_name(level),
        log_subsys_name(subsys),
        msg
    );

    if (n < 0) return;
    if (n >= (int)sizeof(rec)) n = (int)sizeof(rec) - 1;
    rec[n] = 0;

    ring_enqueue(&g_rings[core & 1u], rec, (uint16_t)n);

    if (level >= LOG_LVL_WARN) {
        g_urgent = true;
    }
}

void logger_tick(void) {
    if (!g_initialized || g_suspended) return;

    uint32_t now = to_ms_since_boot(get_absolute_time());
    uint32_t used0 = g_rings[0].head - g_rings[0].tail;
    uint32_t used1 = g_rings[1].head - g_rings[1].tail;
    bool high = (used0 >= LOG_HIGH_WATER) || (used1 >= LOG_HIGH_WATER);
    bool do_sync = g_urgent;

    if (!g_urgent && !high && (now - g_last_flush < LOG_FLUSH_INTERVAL_MS)) {
        return;
    }

    g_last_flush = now;
    g_urgent = false;

    if (!logger_try_open()) {
        if (high) {
            // No SD: drop buffered logs instead of filling RAM forever.
            g_rings[0].tail = g_rings[0].head;
            g_rings[1].tail = g_rings[1].head;
        }
        return;
    }

    char chunk[LOG_CHUNK_BYTES];
    uint16_t clen = 0;

    while (clen + LOG_RECORD_BYTES + 1u <= LOG_CHUNK_BYTES) {
        char rec[LOG_RECORD_BYTES];
        uint16_t rlen = 0;

        bool got = ring_dequeue(&g_rings[0], rec, sizeof(rec), &rlen);
        if (!got) got = ring_dequeue(&g_rings[1], rec, sizeof(rec), &rlen);
        if (!got) break;

        memcpy(chunk + clen, rec, rlen);
        clen = (uint16_t)(clen + rlen);
        chunk[clen++] = '\n';
    }

    if (clen > 0) {
        UINT bw = 0;
        FRESULT fr = f_write(&g_file, chunk, clen, &bw);

        if (fr != FR_OK || bw != clen) {
            f_close(&g_file);
            g_file_open = false;
            g_bytes_written = 0;
        } else {
            g_bytes_written += (uint32_t)bw;

            if (do_sync) {
                f_sync(&g_file);
            }

            if (g_bytes_written >= LOG_MAX_FILE_BYTES) {
                logger_rotate_locked();
                logger_try_open();
            }
        }
    }

    uint32_t d0 = g_rings[0].dropped;
    uint32_t d1 = g_rings[1].dropped;

    if (d0 || d1) {
        g_rings[0].dropped = 0;
        g_rings[1].dropped = 0;
        logger_write(
            0,
            LOG_LVL_WARN,
            LOG_SUB_SYS,
            "dropped_logs c0=%lu c1=%lu",
            (unsigned long)d0,
            (unsigned long)d1
        );
    }
}

void logger_flush_now(void) {
    g_urgent = true;
    if (logger_core_id() == 0) {
        logger_tick();
    }
}
