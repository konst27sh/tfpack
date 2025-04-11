//
// Created by sheverdin on 3/19/24.
//

#ifndef TF_HWSYS_GPIO_MODULE_H
#define TF_HWSYS_GPIO_MODULE_H

#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <stdint.h>
#include <string.h>

#define GPIO_PATH "/dev/gpiochip0"

void gpio_test(void);

#define MAX_GPIO            5
#define GPIO_IDLE           16

typedef enum {
    RTL_GPIO_SYS_LED 	    = 0,
    RTL_GPIO_SYS_RESET	    = 1,
    RTL_GPIO_IOMCU_RESET    = 11,
    RTL_GPIO_POE_RESET	    = 12,
    RTL_IDLE13              = 13,
    RTL_GPIO_BOOT0		    = 14
}GPIO_ADDR;


typedef struct {
    GPIO_ADDR offset;
    const char name[16];
    struct gpiod_chip *chip;
    struct gpiod_line *line;
    uint8_t value;
}gpio_param_t;

GPIO_ADDR get_gpio_addr_by_name(const char *gpio_reg);
struct gpiod_chip* gpio_chipOpen(char *path);
struct gpiod_line* gpio_getChipLine(struct gpiod_chip *chip, int offset);
int gpio_getHandler(gpio_param_t *gpio_param);
int gpio_setHandler(gpio_param_t *gpio_param);
int gpio_resetHandler(gpio_param_t *gpio_param);
void gpio_resetIoMcu();

void set_gpio_state(int gpio_offset, uint8_t value);


#endif //TF_HWSYS_GPIO_MODULE_H


