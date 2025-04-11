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


extern struct config config;

void load_port_config(struct uci_context *uci, struct uci_section *s)
{
    const char * name, *id_str, *enable, *priority, *poe_plus;
    unsigned long id;

    id_str = uci_lookup_option_string(uci, s, "id");
    name = uci_lookup_option_string(uci, s, "name");
    priority = uci_lookup_option_string(uci, s, "priority");
    poe_plus = uci_lookup_option_string(uci, s, "poe_plus");

    if (!id_str || !name) {
        ULOG_ERR("invalid port with missing name and id");
        return;
    }

    id = strtoul(id_str, NULL, 0);
    if (!id || id > MAX_PORT) {
        ULOG_ERR("invalid port id=%lu for %s", id, name);
        return;
    }
    config.port_count = MAX(config.port_count, id);
    id--;

    strncpy(config.ports[id].name, name, sizeof(config.ports[id].name));
    config.ports[id].valid = 1;
    config.ports[id].priority = priority ? strtoul(priority, NULL, 0) : 0;
    config.ports[id].enable = 1;

    if (config.ports[id].priority > 3)
        config.ports[id].priority = 3;

    if (poe_plus && !strcmp(poe_plus, "1"))
        config.ports[id].power_up_mode = 3;
}

void load_port_state_config(struct uci_context *uci, struct uci_section *s, int port)
{
    const char *enable;
    enable = uci_lookup_option_string(uci, s, "poe");
    config.ports[port].enable = enable ? !strcmp(enable, "enable") : 0;
}

void load_global_config(struct uci_context *uci, struct uci_section *s)
{
    const char *budget, *guardband;

    budget = uci_lookup_option_string(uci, s, "budget");
    guardband = uci_lookup_option_string(uci, s, "guard");

    config.budget = budget ? strtof(budget, NULL) : 31.0;
    config.budget_guard = config.budget / 10;
    if (guardband)
        config.budget_guard = strtof(guardband, NULL);
}
void config_load(int init)
{
    struct uci_context *uci = uci_alloc_context();
    struct uci_package *package = NULL;

    memset(config.ports, 0, sizeof(config.ports));
    //poe sections
    if (!uci_load(uci, "poe", &package)) {
        struct uci_element *e;

        if (init)
            uci_foreach_element(&package->sections, e) {
            struct uci_section *s = uci_to_section(e);

            if (!strcmp(s->type, "global"))
                load_global_config(uci, s);
        }
        uci_foreach_element(&package->sections, e) {
            struct uci_section *s = uci_to_section(e);

            if (!strcmp(s->type, "port"))
                load_port_config(uci, s);
        }
    }
    uci_unload(uci, package);
    //port sections (poe state)
    if (!uci_load(uci, "port", &package)) {
        struct uci_element *e;
        int port=0;
        uci_foreach_element(&package->sections, e) {
            struct uci_section *s = uci_to_section(e);

            if (!strcmp(s->type, "lan")){
                load_port_state_config(uci, s, port);
                port ++;
            }
        }
    }
    uci_unload(uci, package);
    uci_free_context(uci);
}

static char *get_board_compatible(void)
{
    char name[128];
    int fd, ret;

    fd = open("/sys/firmware/devicetree/base/compatible", O_RDONLY);
    if (fd < 0)
        return NULL;

    ret = read(fd, name, sizeof(name));
    if (ret < 0)
        return NULL;

    close(fd);

    return strndup(name, ret);
}

void config_apply_quirks(struct config *config)
{
    char *compatible;

    compatible = get_board_compatible();
    if (!compatible) {
        ULOG_ERR("Can't get 'compatible': %s\n", strerror(errno));
        return;
    }

    if (!strcmp(compatible, "zyxel,gs1900-24hp-v1")) {
        /* Send budget command to first 8 PSE IDs */
        config->pse_id_set_budget_mask = 0xff;
    }

    free(compatible);
}