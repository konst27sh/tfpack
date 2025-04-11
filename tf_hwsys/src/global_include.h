//
// Created by sheverdin on 3/19/24.
//

#ifndef TF_HWSYS_GLOBAL_INCLUDE_H
#define TF_HWSYS_GLOBAL_INCLUDE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>

#include "gpio_lib/gpio_module.h"
#include "i2c_lib/i2c_module.h"
#include "utils_lib/utils_module.h"

#define VERSION "0.0.4"
#define MAX_I2C_ERROR (5)


#endif //TF_HWSYS_GLOBAL_INCLUDE_H
