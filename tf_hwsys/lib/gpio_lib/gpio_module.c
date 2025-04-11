//
// Created by sheverdin on 3/19/24.
//

#include "gpio_module.h"

#include "utils_lib/utils_module.h"


gpio_param_t gpio_addrArr[MAX_GPIO] =
{
    {.offset = RTL_GPIO_SYS_LED, .name = "SYS_LED"},
    {.offset = RTL_GPIO_SYS_RESET, .name = "SYS_RESET"},
    {.offset = RTL_GPIO_IOMCU_RESET, .name = "IOMCU_RESET"},
    {.offset = RTL_GPIO_POE_RESET, .name = "POE_RESET"},
    {.offset = RTL_GPIO_BOOT0, .name = "BOOT0"},
};

void gpio_test(void)
{
    printf("Hello from GPIO\n");
}

GPIO_ADDR get_gpio_addr_by_name(const char *gpio_reg)
{
    for(int i = 0; i < MAX_GPIO; i++)
    {
        if(strcmp(gpio_reg, gpio_addrArr[i].name) == 0)
        {
            return gpio_addrArr[i].offset;
        }
    }
    return GPIO_IDLE;
}

struct gpiod_chip* gpio_chipOpen(char *path)
{
    struct gpiod_chip *chip = gpiod_chip_open(path);
    if (!chip) {
        openlog("gpio_ctrl", LOG_PID, LOG_USER);
        syslog(LOG_ERR, "ERROR gpiod_chip_open" );
        closelog();
        exit (EXIT_FAILURE);
    }
    return chip;
}

struct gpiod_line* gpio_getChipLine(struct gpiod_chip *chip, int offset)
{
    //LOG_DBG(DEBUG_NORM,"gpiod_chip_get_line before\n");
    struct gpiod_line *line = gpiod_chip_get_line(chip, offset);
    if (!line) {
        openlog("gpio_ctrl", LOG_PID, LOG_USER);
        syslog(LOG_ERR, "ERROR gpiod_chip_get_line %d\n", offset);
        closelog();
        gpiod_chip_close(chip);
        exit (EXIT_FAILURE);
    }
    return line;
}

int gpio_getHandler(gpio_param_t *gpio_param)
{
    int value, req;
    req = gpiod_line_request_input(gpio_param->line, "gpio_lib");
    if (req) {
        openlog("gpio_ctrl", LOG_PID, LOG_USER);
        syslog(LOG_ERR, "ERROR gpiod_line_request_input %d\n", gpio_param->offset);
        closelog();
        gpiod_chip_close(gpio_param->chip);
        return -1;
    }
    value = gpiod_line_get_value(gpio_param->line);
    req = gpiod_line_request_output(gpio_param->line, "gpio_lib", value);
    printf("%d\n",value);
    gpiod_chip_close(gpio_param->chip);
    return 0;
}

int gpio_setHandler(gpio_param_t *gpio_param)
{
    int req;
    req = gpiod_line_request_output(gpio_param->line, "gpio_lib", 0);
    if (req) {
        openlog("gpio_ctrl", LOG_PID, LOG_USER);
        syslog(LOG_ERR, "ERROR gpiod_line_request_output %d\n",gpio_param->offset);
        closelog();
        gpiod_chip_close(gpio_param->chip);
        return -1;
    }
    gpiod_line_set_value(gpio_param->line, gpio_param->value);
    //printf("ok\n");
    gpiod_chip_close(gpio_param->chip);
    return 0;
}

int gpio_resetHandler(gpio_param_t *gpio_param) {
    int req;
    req = gpiod_line_request_output(gpio_param->line, "gpio", 0);
    if (req) {
        openlog("gpio_ctrl", LOG_PID, LOG_USER);
        syslog(LOG_ERR, "ERROR gpiod_line_request_output %d\n", gpio_param->offset);
        closelog();
        gpiod_chip_close(gpio_param->chip);
        return -1;
    }
    gpiod_line_set_value(gpio_param->line, 0);
    sleep(1);
    gpiod_line_set_value(gpio_param->line, 1);
    //printf("OK \n");
    gpiod_chip_close(gpio_param->chip);
    return 0;
}

void gpio_resetIoMcu()
{
    gpio_param_t gpioParam;
    GPIO_ADDR gpioAddr = RTL_GPIO_IOMCU_RESET;
    struct gpiod_chip *chip = NULL;
    struct gpiod_line *line = NULL;
    chip = gpio_chipOpen(GPIO_PATH);
    //printf("RESET event gpio_chipOpen - gpio_offset = %d\n", gpioAddr);
    line = gpio_getChipLine(chip, gpioAddr);
    gpioParam.chip = chip;
    gpioParam.line = line;
    gpioParam.offset = gpioAddr;
    gpioParam.value = 0;

    int req = gpiod_line_request_output(gpioParam.line, "gpio", 0);
    if (req) {
        openlog("gpio_ctrl", LOG_PID, LOG_USER);
        syslog(LOG_ERR, "ERROR gpiod_line_request_output %d\n", gpioParam.offset);
        closelog();
        gpiod_chip_close(gpioParam.chip);
    }
    else
    {
        gpiod_line_set_value(gpioParam.line, 0);
        sleep(1);
        gpiod_line_set_value(gpioParam.line, 1);
        printf("io_mcu reset OK \n");
        gpiod_chip_close(gpioParam.chip);
    }
}

void set_gpio_state(int gpio_offset, uint8_t value) {
//printf("GPIO HANDLER\n");
    gpio_param_t gpioParam;
    struct gpiod_chip *chip = NULL;
    struct gpiod_line *line = NULL;
    chip = gpio_chipOpen(GPIO_PATH);
    line = gpio_getChipLine(chip, gpio_offset);

    gpioParam.chip = chip;
    gpioParam.line = line;
    gpioParam.offset = gpio_offset;
    gpioParam.value = value;
    if(value == 0 || value == 1)
        gpio_setHandler(&gpioParam);
    else if(value == 2)
        gpio_resetHandler(&gpioParam);
}