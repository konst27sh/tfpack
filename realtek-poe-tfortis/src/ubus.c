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
#include "ubus.h"
#include "main.h"
#include "config.h"

static struct blob_buf b;
extern struct state state;
extern struct config config;

static int ubus_poe_info_cb(struct ubus_context *ctx, struct ubus_object *obj,
                 struct ubus_request_data *req, const char *method,
                 struct blob_attr *msg)
{
    char tmp[16];
    size_t i;
    void *c;

    blob_buf_init(&b, 0);

    snprintf(tmp, sizeof(tmp), "v%u.%u",
             state.sys_version, state.sys_ext_version);
    blobmsg_add_string(&b, "firmware", tmp);
    if (state.sys_mcu)
        blobmsg_add_string(&b, "mcu", state.sys_mcu);
    blobmsg_add_double(&b, "budget", config.budget);
    blobmsg_add_double(&b, "consumption", state.power_consumption);

    c = blobmsg_open_table(&b, "ports");
    for (i = 0; i < config.port_count; i++) {
        void *p;

        if (!config.ports[i].valid)
            continue;

        p = blobmsg_open_table(&b, config.ports[i].name);

        blobmsg_add_u32(&b, "priority", config.ports[i].priority);

        if (state.ports[i].poe_mode)
            blobmsg_add_string(&b, "mode", state.ports[i].poe_mode);
        if (state.ports[i].status_str)
            blobmsg_add_string(&b, "status", state.ports[i].status_str);
        else
            blobmsg_add_string(&b, "status", "unknown");

        if (state.ports[i].status == STAT_DELIVERING) {
            if (state.ports[i].conn_status_str)
                blobmsg_add_string(&b, "connection", state.ports[i].conn_status_str);

            if (state.ports[i].power)
                blobmsg_add_double(&b, "power", state.ports[i].power);
        }
        else if(state.ports[i].status == STAT_FAULT || state.ports[i].status == STAT_OTHER_FAULT){
            if (state.ports[i].faults_str)
                blobmsg_add_string(&b, "faults", state.ports[i].faults_str);
        }


        if (state.ports[i].status_a_str)
            blobmsg_add_string(&b, "status_a", state.ports[i].status_a_str);
        if (state.ports[i].status_b_str)
            blobmsg_add_string(&b, "status_b", state.ports[i].status_b_str);
        if (state.ports[i].det_status_a_str)
            blobmsg_add_string(&b, "detection_a", state.ports[i].det_status_a_str);
        if(state.ports[i].status_a == STAT_DELIVERING) {
            if (state.ports[i].cls_status_a_str)
                blobmsg_add_string(&b, "classification_a", state.ports[i].cls_status_a_str);
            blobmsg_add_u16(&b, "current_a", state.ports[i].current_a);
            blobmsg_add_double(&b, "voltage_a", state.ports[i].voltage_a);
        }
        else if(state.ports[i].status_a == STAT_FAULT || state.ports[i].status_a == STAT_OTHER_FAULT){
            if (state.ports[i].faults_a_str)
                blobmsg_add_string(&b, "faults_a", state.ports[i].faults_a_str);
        }

        if (state.ports[i].det_status_b_str)
            blobmsg_add_string(&b, "detection_b", state.ports[i].det_status_b_str);
        if(state.ports[i].status_b == STAT_DELIVERING) {
            if (state.ports[i].cls_status_b_str)
                blobmsg_add_string(&b, "classification_b", state.ports[i].cls_status_b_str);

            blobmsg_add_u16(&b, "current_b", state.ports[i].current_b);
            blobmsg_add_double(&b, "voltage_b", state.ports[i].voltage_b);
        }
        else if(state.ports[i].status_b == STAT_FAULT || state.ports[i].status_b == STAT_OTHER_FAULT){
            if (state.ports[i].faults_b_str)
                blobmsg_add_string(&b, "faults_b", state.ports[i].faults_b_str);
        }

        if (state.ports[i].temperature)
            blobmsg_add_double(&b, "temperature", state.ports[i].temperature);


        blobmsg_close_table(&b, p);
    }
    blobmsg_close_table(&b, c);

    ubus_send_reply(ctx, req, b.head);

    return UBUS_STATUS_OK;
}

static const struct blobmsg_policy ubus_poe_sendframe_policy[] = {
        { "frame", BLOBMSG_TYPE_STRING },
};

static int
ubus_poe_sendframe_cb(struct ubus_context *ctx, struct ubus_object *obj,
                      struct ubus_request_data *req, const char *method,
                      struct blob_attr *msg)
{
    struct blob_attr *tb[ARRAY_SIZE(ubus_poe_sendframe_policy)];
    char *frame, *next, *end;
    size_t cmd_len = 0;
    unsigned long byte_val;
    uint8_t cmd[9];

    blobmsg_parse(ubus_poe_sendframe_policy,
                  ARRAY_SIZE(ubus_poe_sendframe_policy),
                  tb, blob_data(msg), blob_len(msg));
    if (!*tb)
        return UBUS_STATUS_INVALID_ARGUMENT;

    frame = blobmsg_get_string(*tb);
    end = frame + strlen(frame);
    next = frame;

    while ((next < end) && (cmd_len < sizeof(cmd))) {
        errno = 0;
        byte_val = strtoul(frame, &next, 16);
        if (errno || (frame == next) || (byte_val > 0xff))
            return UBUS_STATUS_INVALID_ARGUMENT;

        cmd[cmd_len++] = byte_val;
        frame = next;
    }

    return poe_cmd_queue(cmd, cmd_len);
}

static int
ubus_poe_reload_cb(struct ubus_context *ctx, struct ubus_object *obj,
                   struct ubus_request_data *req, const char *method,
                   struct blob_attr *msg)
{
    config_load(0);
    poe_port_setup();

    return UBUS_STATUS_OK;
}

static const struct blobmsg_policy ubus_poe_manage_policy[] = {
        { "port", BLOBMSG_TYPE_STRING },
        { "enable", BLOBMSG_TYPE_BOOL },
};

static int ubus_poe_manage_cb(struct ubus_context *ctx, struct ubus_object *obj,
                              struct ubus_request_data *req, const char *method,
                              struct blob_attr *msg)
{
    struct blob_attr *tb[ARRAY_SIZE(ubus_poe_manage_policy)];
    const struct port_config *port;
    const char *port_name;
    size_t i;

    blobmsg_parse(ubus_poe_manage_policy,
                  ARRAY_SIZE(ubus_poe_manage_policy),
                  tb, blob_data(msg), blob_len(msg));
    if (!tb[0] || !tb[1])
        return UBUS_STATUS_INVALID_ARGUMENT;

    port_name = blobmsg_get_string(tb[0]);
    for (i = 0; i < config.port_count; i++) {
        port = &config.ports[i];
        if (port->enable && !strcmp(port_name, port->name))
            return poe_cmd_port_enable(i, blobmsg_get_bool(tb[1]));
    }
    return UBUS_STATUS_INVALID_ARGUMENT;
}

static const struct ubus_method ubus_poe_methods[] = {
        UBUS_METHOD_NOARG("info", ubus_poe_info_cb),
        UBUS_METHOD_NOARG("reload", ubus_poe_reload_cb),
        UBUS_METHOD("sendframe", ubus_poe_sendframe_cb, ubus_poe_sendframe_policy),
        UBUS_METHOD("manage", ubus_poe_manage_cb, ubus_poe_manage_policy),
};

static struct ubus_object_type ubus_poe_object_type =
        UBUS_OBJECT_TYPE("poe", ubus_poe_methods);

static struct ubus_object ubus_poe_object = {
        .name = "poe",
        .type = &ubus_poe_object_type,
        .methods = ubus_poe_methods,
        .n_methods = ARRAY_SIZE(ubus_poe_methods),
};

void ubus_connect_handler(struct ubus_context *ctx)
{
    int ret;
    ULOG_DBG("ubus_connect_handler\n");
    ret = ubus_add_object(ctx, &ubus_poe_object);
    if (ret)
        ULOG_ERR("Failed to add object: %s\n", ubus_strerror(ret));
}
