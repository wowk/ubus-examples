#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>
#include <libubus.h>
#include <libubox/uloop.h>
#include <libubox/blobmsg.h>
#include <libubox/blobmsg_json.h>


static struct ubus_context* ubus_ctx;
static struct blob_buf blobbuf;

enum {
    PORT_WAN,
    PORT_LAN_ETH1,
    PORT_LAN_ETH2,
    PORT_LAN_ETH3,
    PORT_LAN_ETH4,
    PORT_WLAN_2G,
    PORT_WLAN_5G,
};
int port[] = {
    [PORT_WAN] = 1,
    [PORT_LAN_ETH1] = 0,
    [PORT_LAN_ETH1] = 1,
    [PORT_LAN_ETH1] = 0,
    [PORT_LAN_ETH1] = 1,
    [PORT_WLAN_2G] = 0,
    [PORT_WLAN_5G] = 1,
};

/**************************************************************************
 * get network port's status
 * ************************************************************************/
static int status_obj_cb(struct ubus_context *ctx, struct ubus_object *obj,
        struct ubus_request_data *req, const char *method, struct blob_attr *msg);

struct ubus_method status_method[] = {
    UBUS_METHOD_NOARG("network", status_obj_cb),
    UBUS_METHOD_NOARG("network.wan", status_obj_cb),
    UBUS_METHOD_NOARG("network.lan", status_obj_cb),
    UBUS_METHOD_NOARG("network.lan.eth1", status_obj_cb),
    UBUS_METHOD_NOARG("network.lan.eth2", status_obj_cb),
    UBUS_METHOD_NOARG("network.lan.eth3", status_obj_cb),
    UBUS_METHOD_NOARG("network.lan.eth4", status_obj_cb),
    UBUS_METHOD_NOARG("network.lan.wl2g", status_obj_cb),
    UBUS_METHOD_NOARG("network.lan.wl5g", status_obj_cb),
};

struct ubus_object_type status_obj_type =
        UBUS_OBJECT_TYPE("status", status_method);

struct ubus_object status_obj = {
    .name       = "status",
    .type       = &status_obj_type,
    .methods    = status_method,
    .n_methods  = ARRAY_SIZE(status_method)
};


/**************************************************************************
 * config network port
 * ************************************************************************/
enum {
    NETWORK_PORT_LINK,
    NETWORK_PORT_LINK_MAX,
};

struct blobmsg_policy network_port_policy[NETWORK_PORT_LINK_MAX] = {
    {.name = "link",   .type = BLOBMSG_TYPE_BOOL},
};

static int config_obj_cb(struct ubus_context *ctx, struct ubus_object *obj,
        struct ubus_request_data *req, const char *method, struct blob_attr *msg);

struct ubus_method config_method[] = {
    UBUS_METHOD("network.wan", config_obj_cb, network_port_policy),
    UBUS_METHOD("network.lan.eth1", config_obj_cb, network_port_policy),
    UBUS_METHOD("network.lan.eth2", config_obj_cb, network_port_policy),
    UBUS_METHOD("network.lan.eth3", config_obj_cb, network_port_policy),
    UBUS_METHOD("network.lan.eth4", config_obj_cb, network_port_policy),
    UBUS_METHOD("network.lan.wl2g", config_obj_cb, network_port_policy),
    UBUS_METHOD("network.lan.wl5g", config_obj_cb, network_port_policy),
};

struct ubus_object_type config_obj_type =
        UBUS_OBJECT_TYPE("config", config_method);

struct ubus_object config_obj = {
    .name       = "config",
    .type       = &config_obj_type,
    .methods    = config_method,
    .n_methods  = ARRAY_SIZE(config_method),
};

void timeout_handler(struct uloop_timeout *t)
{
    blob_buf_init(&blobbuf, 2);
    blobmsg_add_json_from_string(&blobbuf, "{'type':'notify'}");
    ubus_notify(ubus_ctx, &status_obj, "subscriber/notify test", blobbuf.head, 1000);
    ubus_send_event(ubus_ctx, "test", blobbuf.head);
    blob_buf_free(&blobbuf);

    blob_buf_init(&blobbuf, 3);
    blobmsg_add_json_from_string(&blobbuf, "{'type':'event'}");
    ubus_send_event(ubus_ctx, "event test", blobbuf.head);
    blob_buf_free(&blobbuf);

    uloop_timeout_set(t, 1000);
}

int main(int argc, char *argv[])
{
    uloop_init();

    ubus_ctx = ubus_connect("/tmp/ubus.sock");
    if(!ubus_ctx){
        printf("failed to connect to ubusd: %s", ubus_strerror(errno));
        return -1;
    }

    int errcode;
    errcode = ubus_add_object(ubus_ctx, &status_obj);
    if(errcode){
        printf("failed to register object: %s\n", ubus_strerror(errcode));
    }

    errcode = ubus_add_object(ubus_ctx, &config_obj);
    if(errcode){
        printf("failed to register object: %s\n", ubus_strerror(errcode));
    }

    struct uloop_timeout timeout = {
        .cb          = timeout_handler,
        .pending     = 0,
    };
    uloop_timeout_set(&timeout, 1000);

    //ubus_monitor_start(ubus_ctx);
    ubus_add_uloop(ubus_ctx);
    uloop_run();

    ubus_free(ubus_ctx);
    uloop_done();
}

int status_obj_cb(
        struct ubus_context *ctx,
        struct ubus_object *obj,
        struct ubus_request_data *req,
        const char *method,
        struct blob_attr *msg)
{
    blob_buf_init(&blobbuf, 1);
    json_object* result = json_object_new_object();

    if(!strcmp(method, "network")){
        json_object* wan = json_object_new_boolean(port[PORT_WAN] ? TRUE : FALSE);
        json_object_object_add(result, "wan", wan);

        json_object* lan = json_object_new_object();
        json_object_object_add(result, "lan", lan);

        json_object* lan1 = json_object_new_boolean(port[PORT_LAN_ETH1] ? TRUE : FALSE);
        json_object_object_add(lan, "lan1", lan1);

        json_object* lan2 = json_object_new_boolean(port[PORT_LAN_ETH2] ? TRUE : FALSE);
        json_object_object_add(lan, "lan2", lan2);

        json_object* lan3 = json_object_new_boolean(port[PORT_LAN_ETH3] ? TRUE : FALSE);
        json_object_object_add(lan, "lan3", lan3);

        json_object* lan4 = json_object_new_boolean(port[PORT_LAN_ETH4] ? TRUE : FALSE);
        json_object_object_add(lan, "lan4", lan4);

        json_object* wl2g = json_object_new_boolean(port[PORT_WLAN_2G] ? TRUE : FALSE);
        json_object_object_add(lan, "wl2g", wl2g);

        json_object* wl5g = json_object_new_boolean(port[PORT_WLAN_5G] ? TRUE : FALSE);
        json_object_object_add(lan, "wl5g", wl5g);
    }else if(!strcmp(method, "network.wan")){
        json_object* wan= json_object_new_boolean(port[PORT_WAN] ? TRUE : FALSE);
        json_object_object_add(result, "wan", wan);
    }else if(!strcmp(method, "network.lan")){
        json_object* lan = json_object_new_object();
        json_object_object_add(result, "lan", lan);

        json_object* lan1 = json_object_new_boolean(port[PORT_LAN_ETH1] ? TRUE : FALSE);
        json_object_object_add(lan, "lan1", lan1);

        json_object* lan2 = json_object_new_boolean(port[PORT_LAN_ETH2] ? TRUE : FALSE);
        json_object_object_add(lan, "lan2", lan2);

        json_object* lan3 = json_object_new_boolean(port[PORT_LAN_ETH3] ? TRUE : FALSE);
        json_object_object_add(lan, "lan3", lan3);

        json_object* lan4 = json_object_new_boolean(port[PORT_LAN_ETH4] ? TRUE : FALSE);
        json_object_object_add(lan, "lan4", lan4);

        json_object* wl2g = json_object_new_boolean(port[PORT_WLAN_2G] ? TRUE : FALSE);
        json_object_object_add(lan, "wl2g", wl2g);

        json_object* wl5g = json_object_new_boolean(port[PORT_WLAN_5G] ? TRUE : FALSE);
        json_object_object_add(lan, "wl5g", wl5g);
    }else if(!strcmp(method, "network.lan.eth1")){
        json_object* lan = json_object_new_object();
        json_object_object_add(result, "lan", lan);
        json_object* lan1 = json_object_new_boolean(port[PORT_LAN_ETH1] ? TRUE : FALSE);
        json_object_object_add(lan, "lan1", lan1);
    }else if(!strcmp(method, "network.lan.eth2")){
        json_object* lan = json_object_new_object();
        json_object_object_add(result, "lan", lan);
        json_object* lan2 = json_object_new_boolean(port[PORT_LAN_ETH2] ? TRUE : FALSE);
        json_object_object_add(lan, "lan2", lan2);
    }else if(!strcmp(method, "network.lan.eth3")){
        json_object* lan = json_object_new_object();
        json_object_object_add(result, "lan", lan);
        json_object* lan3 = json_object_new_boolean(port[PORT_LAN_ETH3] ? TRUE : FALSE);
        json_object_object_add(lan, "lan3", lan3);
    }else if(!strcmp(method, "network.lan.eth4")){
        json_object* lan = json_object_new_object();
        json_object_object_add(result, "lan", lan);
        json_object* lan4 = json_object_new_boolean(port[PORT_LAN_ETH4] ? TRUE : FALSE);
        json_object_object_add(lan, "lan4", lan4);
    }else if(!strcmp(method, "network.lan.wl2g")){
        json_object* lan = json_object_new_object();
        json_object_object_add(result, "lan", lan);
        json_object* wl2g = json_object_new_boolean(port[PORT_WLAN_2G] ? TRUE : FALSE);
        json_object_object_add(lan, "wl2g", wl2g);
    }else if(!strcmp(method, "network.lan.wl5g")){
        json_object* lan = json_object_new_object();
        json_object_object_add(result, "lan", lan);
        json_object* wl5g = json_object_new_boolean(port[PORT_WLAN_5G] ? TRUE : FALSE);
        json_object_object_add(lan, "wl5g", wl5g);
    }

    blobmsg_add_json_from_string(&blobbuf, strdup(json_object_to_json_string_ext(result, JSON_C_TO_STRING_PRETTY)));

    ubus_send_reply(ctx, req, blobbuf.head);
    json_object_put(result);

    blob_buf_free(&blobbuf);

    return 0;
}


int config_obj_cb(
        struct ubus_context *ctx,
        struct ubus_object *obj,
        struct ubus_request_data *req,
        const char *method,
        struct blob_attr *msg)
{
    struct blob_attr* tb[NETWORK_PORT_LINK_MAX];
    blobmsg_parse(&network_port_policy, ARRAY_SIZE(network_port_policy), &tb, blob_data(msg), blob_len(msg));
    blob_buf_init(&blobbuf, 1);
    json_object* result = json_object_new_object();

    if(!tb[NETWORK_PORT_LINK]){
        json_object* is_success = json_object_new_boolean(FALSE);
        json_object_object_add(result, "success", is_success);
    }else{
        int link_st = blobmsg_get_bool(tb[NETWORK_PORT_LINK]);\
        if(!strcmp(method, "network.wan")){
            port[PORT_WAN] = link_st;
        }else if(!strcmp(method, "network.lan.eth1")){
            port[PORT_LAN_ETH1] = link_st;
        }else if(!strcmp(method, "network.lan.eth2")){
            port[PORT_LAN_ETH2] = link_st;
        }else if(!strcmp(method, "network.lan.eth4")){
            port[PORT_LAN_ETH3] = link_st;
        }else if(!strcmp(method, "network.lan.eth4")){
            port[PORT_LAN_ETH4] = link_st;
        }else if(!strcmp(method, "network.lan.wl2g")){
            port[PORT_WLAN_2G] = link_st;
        }else if(!strcmp(method, "network.lan.wl5g")){
            port[PORT_WLAN_5G] = link_st;
        }
        json_object* is_success = json_object_new_boolean(TRUE);
        json_object_object_add(result, "success", is_success);
    }

    blobmsg_add_json_from_string(&blobbuf, strdup(json_object_to_json_string_ext(result, JSON_C_TO_STRING_PRETTY)));
    ubus_send_reply(ctx, req, blobbuf.head);
    json_object_put(result);

    blob_buf_free(&blobbuf);

    return 0;
}
