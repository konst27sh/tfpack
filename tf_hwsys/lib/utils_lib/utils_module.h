//
// Created by sheverdin on 3/21/24.
//


#ifndef TF_HWSYS_UTILS_MODULE_H
#define TF_HWSYS_UTILS_MODULE_H

#include <stdint.h>
#include <libubox/ulog.h>

typedef unsigned long clock_time_t;
struct timer {
  clock_time_t start;
  clock_time_t interval;
};

typedef enum{
    ERROR = 0,
    DEBUG_NORM,
    DEBUG_VERBOSE
}event_verbose_t;



void setTestMode(char *testMode);
uint8_t isDebugMode(void);
void printf_array(uint8_t *buff,uint32_t size);
void runTimer(uint32_t *timeNow, uint8_t delay_sec);

void timer_set(struct timer *t, clock_time_t interval);
void timer_reset(struct timer *t);
int timer_expired(struct timer *t);

FILE *openPipe(const char *cmdStr);
void closePipe(FILE *pipe);

#define LOG_DBG(level,fmt, ...) if(isDebugMode() >= level) {ulog(LOG_DEBUG, fmt, ## __VA_ARGS__);}


#endif //TF_HWSYS_UTILS_MODULE_H
