//
// Created by sheverdin on 3/19/24.
//

#include "global_include.h"

#include "ubus_lib/ubus_module.h"
#include "utils_lib/utils_module.h"

#include <libubox/ustream.h>
#include <libubox/uloop.h>
#include <libubox/list.h>
#include <libubox/ulog.h>
#include <libubus.h>

#include <uci.h>
#include <uci_blob.h>

#include "config.h"


//ubus communication
struct ubus_auto_conn conn = {
        .cb = ubus_connect_handler,
};

static void mainHandler()
{
    static uint8_t errorCount = 0;
    static uint8_t prevResetCount = 0;
    gpio_param_t gpio_param;

    if(read_i2c() == EXIT_FAILURE)
        errorCount++;
    else
        errorCount = 0;

    if (errorCount > MAX_I2C_ERROR)
    {
        errorCount = 0;
        gpio_resetIoMcu();
    }
}
static void main_timeout_cb(struct uloop_timeout *t)
{
    LOG_DBG(DEBUG_VERBOSE,"main_timeout_cb\n");
    mainHandler();
    uloop_timeout_set(t, 1000);
}

static void event_timeout_cb(struct uloop_timeout *t)
{
    LOG_DBG(DEBUG_VERBOSE,"event_timeout_cb\n");
    i2c_events_handler();
    uloop_timeout_set(t, 3 * 1000);
}

struct uloop_timeout main_timeout = {
        .cb = main_timeout_cb,
};
struct uloop_timeout event_timeout = {
        .cb = event_timeout_cb,
};

int main(int argc, char **argv)
{
    if (argc >= 2){
        setTestMode(argv[1]);
    }
    LOG_DBG(DEBUG_NORM,"tf_hwsys_demon VERSION: %s\n", VERSION);
    open_i2c();
    if(is_port_opened()){
        LOG_DBG(DEBUG_NORM,"I2C port opened OK\n");
    }
    else{
        LOG_DBG(DEBUG_NORM,"I2C port opened FAIL\n");
    }

    config_load();

    uloop_init();
    ubus_auto_connect(&conn);
    uloop_timeout_set(&main_timeout, 1000);
    uloop_timeout_set(&event_timeout, 1000);
    uloop_run();
    uloop_done();
    close_i2c();
    return EXIT_SUCCESS;
}
