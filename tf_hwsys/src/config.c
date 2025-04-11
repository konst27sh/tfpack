//
// Created by BelyaevAlex on 18.12.2024.
//

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>

#include <libubox/ustream.h>
#include <libubox/uloop.h>
#include <libubox/list.h>
#include <libubox/ulog.h>
#include <libubus.h>

#include <uci.h>
#include <uci_blob.h>

#include <syslog.h>
#include "config.h"

#include "utils_lib/utils_module.h"
#include "i2c_lib/i2c_module.h"
extern struct config config;

void load_io_config(){
    struct uci_context *uci = uci_alloc_context();
    struct uci_package *package = NULL;
    const char *state;
    const char *name;
    i2c_data_t i2CDataWrite;
    REGISTER_ADDR addr;
    int32_t i32val = 0;

	 if (!uci_load(uci, "tfortis_io", &package)) {
         struct uci_element *e;

         uci_foreach_element(&package->sections, e)
         {
             struct uci_section *s = uci_to_section(e);
             if (!strcmp(s->type, "output")) {
                 state = uci_lookup_option_string(uci, s, "state");
                 if (state) {
                     name = s->e.name;
                     LOG_DBG(DEBUG_NORM,"setParam name = %s value=%s\n",name,state);

                     if (!strcmp(state, "open"))
                        i32val = 0;
                     else  if (!strcmp(state, "short"))
                         i32val = 1;

                     //i2c handler
                     addr = get_i2c_addr_by_name(name);
                     if(addr) {
                         i2CDataWrite.opcode = I2C_OPCODE_WRITE;
                         i2CDataWrite.addr = addr;
                         i2CDataWrite.value[0] = (uint8_t) i32val;
                         i2CDataWrite.value[1] = (uint8_t)(i32val << 8);
                         i2CDataWrite.value[2] = (uint8_t)(i32val << 16);
                         i2CDataWrite.value[3] = (uint8_t)(i32val << 24);
                         i2CDataWrite.lenData = 1;
                         i2c_setData(&i2CDataWrite);
                     }

                 }
             }
         }
     }
    uci_unload(uci, package);
    uci_free_context(uci);
}


void config_load()
{
    load_io_config();
}

