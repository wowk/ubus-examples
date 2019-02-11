#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>
#include <libubus.h>
#include <libubox/uloop.h>
#include <libubox/blobmsg.h>
#include <libubox/blobmsg_json.h>


static struct ubus_context* ubus_ctx;
static struct ubus_subscriber subscriber;
static struct ubus_event_handler event;

int subscriber_handler(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
    printf("got notify\n");
}

void subscriber_remove_handler(struct ubus_context *ctx, struct ubus_subscriber *obj, uint32_t id)
{
    printf("object removed\n");
}

void event_handler(struct ubus_context *ctx, struct ubus_event_handler *ev, const char *type, struct blob_attr *msg)
{
    printf("got event\n");
}

int main()
{
    uloop_init();

    ubus_ctx = ubus_connect("/tmp/ubus.sock");
    if(!ubus_ctx){
        printf("failed to connect to ubusd\n");
        return -1;
    }
    ubus_add_uloop(ubus_ctx);

    subscriber.cb = subscriber_handler;
    subscriber.remove_cb = subscriber_remove_handler;
    ubus_register_subscriber(ubus_ctx, &subscriber);

    int status_obj_id;
    ubus_lookup_id(ubus_ctx, "status", &status_obj_id);
    ubus_subscribe(ubus_ctx, &subscriber, status_obj_id);

    event.cb = event_handler;
    ubus_register_event_handler(ubus_ctx, &event, "event test");
    ubus_handle_event(ubus_ctx);

    uloop_run();

    ubus_free(ubus_ctx);
    uloop_done();

    return 0;
}
