//
// Created by sheverdin on 3/21/24.
//
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include "utils_module.h"
uint8_t globalDebugMode = ERROR;

void setTestMode(char *mode)
{
    if (strcmp(mode,"-dd")==0){
        printf("testMode = %s\n", mode);
        globalDebugMode = DEBUG_VERBOSE;
    }
    if (strcmp(mode,"-d")==0){
        printf("testMode = %s\n", mode);
        globalDebugMode = DEBUG_NORM;
    }
}

void printf_array(uint8_t *buff,uint32_t size){
char temp[16];
char printf_buff[128];
	//печатаем по 16 символов в строке
	for(uint8_t i=0;i<(size/16);i++){
		printf_buff[0] = 0;
		for(uint8_t j=0;j<16;j++){
			sprintf(temp,"%02X ",buff[i*16+j]);
			strcat(printf_buff,temp);
		}
		printf("%s\r\n",printf_buff);
	}
	//для остатка
	printf_buff[0] = 0;
	for(uint8_t i=0;i<size%16;i++){
		sprintf(temp,"%02X ",buff[(size/16)*16+i]);
		strcat(printf_buff,temp);
	}
	printf("%s\r\n",printf_buff);
}


uint8_t isDebugMode(void)
{
    return globalDebugMode;
}

void runTimer(uint32_t *timeAlarm, uint8_t delay_sec)
{
    *timeAlarm = time(NULL) + delay_sec;
}

void timer_set(struct timer *t, clock_time_t interval)
{
  t->interval = interval;
  t->start = time(NULL);
}

void timer_reset(struct timer *t)
{
  t->start += t->interval;
}

int timer_expired(struct timer *t)
{
  return (clock_time_t)(time(NULL) - t->start) >= (clock_time_t)t->interval;
}

FILE *openPipe(const char *cmdStr)
{
    FILE *pipe = popen(cmdStr, "r");
    if (!pipe) {
        openlog("tfortis", LOG_PID | LOG_PERROR, LOG_USER);
        syslog(LOG_ERR, "Error to run command %s", cmdStr);
        closelog();
    }
    return pipe;
}

void closePipe(FILE *pipe)
{
    int status = pclose(pipe);

    if (!status) {
        return;
    } else{
        //printf("error str = %s\n", str);
        openlog("tfortis", LOG_PID | LOG_PERROR, LOG_USER);
        //syslog(LOG_ERR, "Error close pipe, Status =  %d", status);
        syslog(LOG_INFO, "Error close pipe, status %d\n",  status);
        //printf("Error close pipe, cmd  %s, status %d\n", str, status);
        closelog();
    }
}