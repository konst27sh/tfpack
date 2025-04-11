//
// Created by BelyaevAlex on 05.12.2024.
//
#include <signal.h>

#include <libubox/ustream.h>
#include <libubox/uloop.h>
#include <libubox/list.h>
#include <libubox/ulog.h>
#include <libubus.h>

#include <syslog.h>
#include "utils_lib/utils_module.h"
#include "ubus_lib/ubus_module.h"
#include "i2c_lib/i2c_module.h"
#include "gpio_lib/gpio_module.h"
static struct blob_buf b;

extern i2c_data_t sock_msgArr[MAX_SENSORS];


//dump all registers
static int ubus_hwsys_dump_cb(struct ubus_context *ctx, struct ubus_object *obj,
                   struct ubus_request_data *req, const char *method,
                   struct blob_attr *msg) {
    char tmp[64];
    size_t i;
    void *c;
    blob_buf_init(&b, 0);
    for (i = 0; i < sizeof(sock_msgArr) / sizeof(i2c_data_t); i++) {
        if(sock_msgArr[i].addr) {
            i2c_parsingData(sock_msgArr[i].addr, tmp);
            blobmsg_add_string(&b, sock_msgArr[i].name, tmp);
        }
    }
    ubus_send_reply(ctx, req, b.head);
    return UBUS_STATUS_OK;
}

//status of running hwsys
static int ubus_hwsys_info_cb(struct ubus_context *ctx, struct ubus_object *obj,
                   struct ubus_request_data *req, const char *method,
                   struct blob_attr *msg)
{
    char tmp[16];
    size_t i;
    void *c;
    blob_buf_init(&b, 0);

    blobmsg_add_u8(&b, "running", 1);
    blobmsg_add_u8(&b, "debug", isDebugMode());
    blobmsg_add_u8(&b, "i2c_opened", is_port_opened());

    ubus_send_reply(ctx, req, b.head);
    return UBUS_STATUS_OK;
}

static const struct blobmsg_policy ubus_hwsys_param_policy[] = {
        { "name", BLOBMSG_TYPE_STRING },
};

static const struct blobmsg_policy ubus_hwsys_set_param_policy[] = {
        { "name",  BLOBMSG_TYPE_STRING },
        { "value", BLOBMSG_TYPE_STRING },
};


static int ubus_hwsys_param_cb(struct ubus_context *ctx, struct ubus_object *obj,
                                struct ubus_request_data *req, const char *method,
                                struct blob_attr *msg)
{
    struct blob_attr *tb[ARRAY_SIZE(ubus_hwsys_param_policy)];
    const struct port_config *port;
    const char *name;
    size_t i;
    char tmp[64];

    if (!msg)
        return UBUS_STATUS_INVALID_ARGUMENT;

    blobmsg_parse(ubus_hwsys_param_policy,
                  ARRAY_SIZE(ubus_hwsys_param_policy),
                  tb, blob_data(msg), blob_len(msg));
    if (!*tb)
        return UBUS_STATUS_INVALID_ARGUMENT;

    name = blobmsg_get_string(*tb);
    LOG_DBG(DEBUG_NORM,"getParam name = %s",name);
    blob_buf_init(&b, 0);
    //i2c handler
    for (i = 0; i < sizeof(sock_msgArr) / sizeof(i2c_data_t); i++) {
        if(sock_msgArr[i].addr) {
            if(strcmp(name,sock_msgArr[i].name) == 0) {
                i2c_parsingData(sock_msgArr[i].addr, tmp);
                blobmsg_add_string(&b, sock_msgArr[i].name, tmp);
                ubus_send_reply(ctx, req, b.head);
                return UBUS_STATUS_OK;
            }
        }
    }
    //gpio handler
    //nothing
    return UBUS_STATUS_INVALID_ARGUMENT;
}

static int ubus_hwsys_category_cb(struct ubus_context *ctx, struct ubus_object *obj,
                               struct ubus_request_data *req, const char *method,
                               struct blob_attr *msg)
{
    struct blob_attr *tb[ARRAY_SIZE(ubus_hwsys_param_policy)];
    const struct port_config *port;
    const char *name;
    size_t i;
    register_category_t category;
    char tmp[64];

    blobmsg_parse(ubus_hwsys_param_policy,
                  ARRAY_SIZE(ubus_hwsys_param_policy),
                  tb, blob_data(msg), blob_len(msg));
    if (!*tb)
        return UBUS_STATUS_INVALID_ARGUMENT;

    name = blobmsg_get_string(tb[0]);
    category = get_param_category(name);

    LOG_DBG(DEBUG_NORM,"getCategory name = %s",name);
    blob_buf_init(&b, 0);
    for (i = 0; i < sizeof(sock_msgArr) / sizeof(i2c_data_t); i++) {
        if(sock_msgArr[i].addr) {
            if(category == sock_msgArr[i].category) {
                i2c_parsingData(sock_msgArr[i].addr, tmp);
                blobmsg_add_string(&b, sock_msgArr[i].name, tmp);
            }
        }
    }
    ubus_send_reply(ctx, req, b.head);
    return UBUS_STATUS_OK;
}


static int ubus_hwsys_set_param_cb(struct ubus_context *ctx, struct ubus_object *obj,
                               struct ubus_request_data *req, const char *method,
                               struct blob_attr *msg)
{
    struct blob_attr *tb[ARRAY_SIZE(ubus_hwsys_set_param_policy)];
    const struct port_config *port;
    const char *name,*value;
    size_t i;
    i2c_data_t i2CDataWrite;
    REGISTER_ADDR addr;
    GPIO_ADDR gpio_offset;
    int32_t i32val;

    char *end = NULL;


    blobmsg_parse(ubus_hwsys_set_param_policy,
                  ARRAY_SIZE(ubus_hwsys_set_param_policy),
                  tb, blob_data(msg), blob_len(msg));
    if (!tb[0] || !tb[1])
        return UBUS_STATUS_INVALID_ARGUMENT;

    name = blobmsg_get_string(tb[0]);
    value = blobmsg_get_string(tb[1]);

    LOG_DBG(DEBUG_NORM,"setParam name = %s value=%s\n",name,value);
    i32val = strtol(value, &end, 10);

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

    //gpio handler
    gpio_offset = get_gpio_addr_by_name(name);
    if(gpio_offset != GPIO_IDLE) {
        set_gpio_state(gpio_offset,(uint8_t)i32val);
    }

    return UBUS_STATUS_OK;
}

static int ubus_hwsys_set_config_cb(struct ubus_context *ctx, struct ubus_object *obj,
                                   struct ubus_request_data *req, const char *method,
                                   struct blob_attr *msg)
{
    struct blob_attr *tb[ARRAY_SIZE(ubus_hwsys_set_param_policy)];
    const struct port_config *port;
    const char *name,*value;
    size_t i;
    i2c_data_t i2CDataWrite;
    REGISTER_ADDR addr;
    GPIO_ADDR gpio_offset;
    int32_t i32val;

    char *end = NULL;


    blobmsg_parse(ubus_hwsys_set_param_policy,
                  ARRAY_SIZE(ubus_hwsys_set_param_policy),
                  tb, blob_data(msg), blob_len(msg));
    if (!tb[0] || !tb[1])
        return UBUS_STATUS_INVALID_ARGUMENT;

    name = blobmsg_get_string(tb[0]);
    value = blobmsg_get_string(tb[1]);

    i32val = strtol(value, &end, 10);

    if(strcmp(name,"close_port")==0){
        close_i2c();
    }
    else if(strcmp(name,"open_port")==0){
        open_i2c();
    }


    return UBUS_STATUS_OK;
}

static const struct ubus_method ubus_hwsys_methods[] = {
        UBUS_METHOD_NOARG("dump", ubus_hwsys_dump_cb),
        UBUS_METHOD_NOARG("info", ubus_hwsys_info_cb),
        UBUS_METHOD("getParam", ubus_hwsys_param_cb,ubus_hwsys_param_policy),
        UBUS_METHOD("getCategory", ubus_hwsys_category_cb,ubus_hwsys_param_policy),
        UBUS_METHOD("setParam", ubus_hwsys_set_param_cb,ubus_hwsys_set_param_policy),
        UBUS_METHOD("config", ubus_hwsys_set_config_cb,ubus_hwsys_set_param_policy),
};

static struct ubus_object_type ubus_hwsys_object_type =
        UBUS_OBJECT_TYPE("tf_hwsys", ubus_hwsys_methods);

static struct ubus_object ubus_hwsys_object = {
        .name = "tf_hwsys",
        .type = &ubus_hwsys_object_type,
        .methods = ubus_hwsys_methods,
        .n_methods = ARRAY_SIZE(ubus_hwsys_methods),
};

void ubus_connect_handler(struct ubus_context *ctx)
{
    int ret;
    LOG_DBG(DEBUG_NORM,"ubus_connect_handler\n");
    ret = ubus_add_object(ctx, &ubus_hwsys_object);
    if (ret)
        LOG_DBG(ERROR,"Failed to add object: %s\n", ubus_strerror(ret));
}