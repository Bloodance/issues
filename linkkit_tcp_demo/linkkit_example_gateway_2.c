/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */
#ifdef DEPRECATED_LINKKIT
#include "deprecated/gateway.c"
#else
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "cJSON.h"
#include "iot_import.h"
#include "iot_export_linkkit.h"
#include "app_entry.h"
#include "tcpserver/tcpserver.h"

#ifdef LINKKIT_GATEWAY_TEST_CMD
    #include "simulate_subdev/testcmd.h"
#endif

#if defined(OTA_ENABLED) && defined(BUILD_AOS)
    #include "ota_service.h"
#endif

// for demo only
#define PRODUCT_KEY      "a1uwOsxYY64"
#define PRODUCT_SECRET   "raTTjtDq7GwWhr4X"
#define DEVICE_NAME      "test1"
#define DEVICE_SECRET    "s7eOumSmP51dszhawrDuT1BbCQgbPZ0u"

#define USER_EXAMPLE_YIELD_TIMEOUT_MS (200)

#define EXAMPLE_TRACE(...) \
    do { \
        HAL_Printf("\033[1;32;40m%s.%d: ", __func__, __LINE__); \
        HAL_Printf(__VA_ARGS__); \
        HAL_Printf("\033[0m\r\n"); \
    } while (0)

#define EXAMPLE_SUBDEV_ADD_NUM          3
#define EXAMPLE_SUBDEV_MAX_NUM          20
const iotx_linkkit_dev_meta_info_t subdevArr[EXAMPLE_SUBDEV_MAX_NUM] = {
    {
        "a11obENsIK5",
        "7t8sboFlAxl5DHuR",
        "test1",
        "OvZijBgvxMSIUxj2hbhVIVlwVEwq6YLG"
    },
    {
        "a11obENsIK5",
        "7t8sboFlAxl5DHuR",
        "test2",
        "JDOtNriLZ86G91TztwHqtZxZKh5GD9gA"
    },
    {
        "a11obENsIK5",
        "7t8sboFlAxl5DHuR",
        "test3",
        "ZfmjO59j7Em7v7iRhxqVglt3W8qigBv5"
    },
    {
        "a11obENsIK5",
        "7t8sboFlAxl5DHuR",
        "test4",
        "2yNi06mZCovhokSfUwEVuCWgE2yO2swA"
    },
    {
        "a11obENsIK5",
        "7t8sboFlAxl5DHuR",
        "test5",
        "mH7bk9wLzWFPVITt0NdAlO8RDldBtNLy"
    },
    {
        "a1YRfb9bepk",
        "PKbZL7baK8pBso94",
        "test_06",
        "QTobiz1BdGW5XNgLGIgNSylH0btVvvGS"
    },
    {
        "a1YRfb9bepk",
        "PKbZL7baK8pBso94",
        "test_07",
        "IX7ol50rRS2uP8V74jt0DKfmYn8iC6h1"
    },
    {
        "a1YRfb9bepk",
        "PKbZL7baK8pBso94",
        "test_08",
        "sbFxx62evXVoVgJ5gL2oCLcz1pX9d6K2"
    },
    {
        "a1YRfb9bepk",
        "PKbZL7baK8pBso94",
        "test_09",
        "S0CpOl54GZxEO7Gz5DWQa5YxgUMfT4xA"
    },
    {
        "a1YRfb9bepk",
        "PKbZL7baK8pBso94",
        "test_10",
        "MowJJjiNIkTdUcX5fCNUDu39Yz02KADL"
    },
    {
        "a1YRfb9bepk",
        "PKbZL7baK8pBso94",
        "test_11",
        "BrJNdAihVznMWTpdRfe8HIiI95ubSYdN"
    },
    {
        "a1YRfb9bepk",
        "PKbZL7baK8pBso94",
        "test_12",
        "Yd3ZHK8D6cAKKRQb9rUevCfwPf7atoQ4"
    },
    {
        "a1YRfb9bepk",
        "PKbZL7baK8pBso94",
        "test_13",
        "d3HKvu2eBR5ytcgDaBEt0gpvJZlu9W0g"
    },
    {
        "a1YRfb9bepk",
        "PKbZL7baK8pBso94",
        "test_14",
        "qAJYUpQ1tGmAINQBzMiZwwbyjY6YXDGc"
    },
    {
        "a1YRfb9bepk",
        "PKbZL7baK8pBso94",
        "test_15",
        "GxgVknnAmUmwjjdHJf9dbEBDoqyDaUfp"
    },
    {
        "a1YRfb9bepk",
        "PKbZL7baK8pBso94",
        "test_16",
        "9d17Sv05j1XeTYOs80UBpBU1OYTTJ58X"
    },
    {
        "a1YRfb9bepk",
        "PKbZL7baK8pBso94",
        "test_17",
        "FTUm4HAfhZ5wH2u0pPn7PWcCLGDrgEfn"
    },
    {
        "a1YRfb9bepk",
        "PKbZL7baK8pBso94",
        "test_18",
        "mF7a2ptc3PRi7jWLE92t0GElhGdPnAe3"
    },
    {
        "a1YRfb9bepk",
        "PKbZL7baK8pBso94",
        "test_19",
        "Vrh8HuNvu3jtTEwSzulAjTqgOseAsVgz"
    },
    {
        "a1YRfb9bepk",
        "PKbZL7baK8pBso94",
        "test_20",
        "8Wxrxnjch6SW0s2HR5JkIBtgjt3BOUo7"
    }
};

typedef struct {
    int auto_add_subdev;
    int master_devid;
    int cloud_connected;
    int master_initialized;
    int subdev_index;
    int permit_join;
    void *g_user_dispatch_thread;
    int g_user_dispatch_thread_running;
} user_example_ctx_t;

static user_example_ctx_t g_user_example_ctx;

void *example_malloc(size_t size)
{
    return HAL_Malloc(size);
}

void example_free(void *ptr)
{
    HAL_Free(ptr);
}

static user_example_ctx_t *user_example_get_ctx(void)
{
    return &g_user_example_ctx;
}

static int user_connected_event_handler(void)
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();

    EXAMPLE_TRACE("Cloud Connected");

    user_example_ctx->cloud_connected = 1;
#if defined(OTA_ENABLED) && defined(BUILD_AOS)
    ota_service_init(NULL);
#endif
    return 0;
}

static int user_disconnected_event_handler(void)
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();

    EXAMPLE_TRACE("Cloud Disconnected");

    user_example_ctx->cloud_connected = 0;

    return 0;
}

static int user_service_request_event_handler(const int devid, const char *serviceid, const int serviceid_len,
        const char *request, const int request_len,
        char **response, int *response_len)
{
    int contrastratio = 0, to_cloud = 0;
    cJSON *root = NULL, *item_transparency = NULL, *item_from_cloud = NULL;
    EXAMPLE_TRACE("Service Request Received, Devid: %d, Service ID: %.*s, Payload: %s", devid, serviceid_len,
                  serviceid,
                  request);

    /* Parse Root */
    root = cJSON_CreateObject();
    if (root == NULL || !cJSON_IsObject(root)) {
        EXAMPLE_TRACE("JSON Parse Error");
        return -1;
    }

    // cJSON_AddItemToObject(root, "LightSwitch", "1");

    // char *str1 = cJSON_Print(root);
    // EXAMPLE_TRACE("Test cJSON Print : %s", str1);

    // int res = IOT_Linkkit_Report(0, ITM_MSG_POST_PROPERTY, str1, strlen(str1));
    // EXAMPLE_TRACE("Post Property Message ID: %d", res);
    // if (strlen("AddKey") == serviceid_len && memcmp("AddKey", serviceid, serviceid_len) == 0) {
    //     /* Parse Item */
    //     const char *response_fmt = "{\"UserLimit\":%d}";
    //     item_transparency = cJSON_GetObjectItem(root, "transparency");
    //     if (item_transparency == NULL || !cJSON_IsNumber(item_transparency)) {
    //         cJSON_Delete(root);
    //         return -1;
    //     }
    //     EXAMPLE_TRACE("transparency: %d", item_transparency->valueint);
    //     contrastratio = item_transparency->valueint + 1;

    //     /* Send Service Response To Cloud */
    //     *response_len = strlen(response_fmt) + 10 + 1;
    //     *response = (char *)HAL_Malloc(*response_len);
    //     if (*response == NULL) {
    //         EXAMPLE_TRACE("Memory Not Enough");
    //         return -1;
    //     }
    //     memset(*response, 0, *response_len);
    //     HAL_Snprintf(*response, *response_len, response_fmt, contrastratio);
    //     *response_len = strlen(*response);
    // } else if (strlen("SyncService") == serviceid_len && memcmp("SyncService", serviceid, serviceid_len) == 0) {
    //     /* Parse Item */
    //     const char *response_fmt = "{\"ToCloud\":%d}";
    //     item_from_cloud = cJSON_GetObjectItem(root, "FromCloud");
    //     if (item_from_cloud == NULL || !cJSON_IsNumber(item_from_cloud)) {
    //         cJSON_Delete(root);
    //         return -1;
    //     }
    //     EXAMPLE_TRACE("FromCloud: %d", item_from_cloud->valueint);
    //     to_cloud = item_from_cloud->valueint + 1;

    //     /* Send Service Response To Cloud */
    //     *response_len = strlen(response_fmt) + 10 + 1;
    //     *response = (char *)HAL_Malloc(*response_len);
    //     if (*response == NULL) {
    //         EXAMPLE_TRACE("Memory Not Enough");
    //         return -1;
    //     }
    //     memset(*response, 0, *response_len);
    //     HAL_Snprintf(*response, *response_len, response_fmt, to_cloud);
    //     *response_len = strlen(*response);
    // }
    cJSON_Delete(root);

    return 0;
}

static int user_property_set_event_handler(const int devid, const char *request, const int request_len)
{
    int res = 0;
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    EXAMPLE_TRACE("Property Set Received, Devid: %d, Request: %s", devid, request);

    res = IOT_Linkkit_Report(devid, ITM_MSG_POST_PROPERTY,
                             (unsigned char *)request, request_len);
    EXAMPLE_TRACE("Post Property Message ID: %d", res);

    return 0;
}

static int user_report_reply_event_handler(const int devid, const int msgid, const int code, const char *reply,
        const int reply_len)
{
    const char *reply_value = (reply == NULL) ? ("NULL") : (reply);
    const int reply_value_len = (reply_len == 0) ? (strlen("NULL")) : (reply_len);

    EXAMPLE_TRACE("Message Post Reply Received, Devid: %d, Message ID: %d, Code: %d, Reply: %.*s", devid, msgid, code,
                  reply_value_len,
                  reply_value);
    return 0;
}

static int user_trigger_event_reply_event_handler(const int devid, const int msgid, const int code, const char *eventid,
        const int eventid_len, const char *message, const int message_len)
{
    EXAMPLE_TRACE("Trigger Event Reply Received, Devid: %d, Message ID: %d, Code: %d, EventID: %.*s, Message: %.*s", devid,
                  msgid, code,
                  eventid_len,
                  eventid, message_len, message);

    return 0;
}

static int user_timestamp_reply_event_handler(const char *timestamp)
{
    EXAMPLE_TRACE("Current Timestamp: %s", timestamp);

    return 0;
}

static int user_initialized(const int devid)
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    EXAMPLE_TRACE("Device Initialized, Devid: %d", devid);

    if (user_example_ctx->master_devid == devid) {
        user_example_ctx->master_initialized = 1;
        user_example_ctx->subdev_index++;
    }

    return 0;
}

static uint64_t user_update_sec(void)
{
    static uint64_t time_start_ms = 0;

    if (time_start_ms == 0) {
        time_start_ms = HAL_UptimeMs();
    }

    return (HAL_UptimeMs() - time_start_ms) / 1000;
}

void user_post_property(void)
{
    int res = 0;
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    char *property_payload = "{\"LightSwitch\":1}";

    res = IOT_Linkkit_Report(user_example_ctx->master_devid, ITM_MSG_POST_PROPERTY,
                             (unsigned char *)property_payload, strlen(property_payload));
    EXAMPLE_TRACE("Post Property Message ID: %d", res);
}

void user_deviceinfo_update(void)
{
    int res = 0;
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    char *device_info_update = "[{\"attrKey\":\"abc\",\"attrValue\":\"hello,world\"}]";

    res = IOT_Linkkit_Report(user_example_ctx->master_devid, ITM_MSG_DEVICEINFO_UPDATE,
                             (unsigned char *)device_info_update, strlen(device_info_update));
    EXAMPLE_TRACE("Device Info Update Message ID: %d", res);
}

void user_deviceinfo_delete(void)
{
    int res = 0;
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    char *device_info_delete = "[{\"attrKey\":\"abc\"}]";

    res = IOT_Linkkit_Report(user_example_ctx->master_devid, ITM_MSG_DEVICEINFO_DELETE,
                             (unsigned char *)device_info_delete, strlen(device_info_delete));
    EXAMPLE_TRACE("Device Info Delete Message ID: %d", res);
}

static int user_master_dev_available(void)
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();

    if (user_example_ctx->cloud_connected && user_example_ctx->master_initialized) {
        return 1;
    }

    return 0;
}

void set_iotx_info()
{
    HAL_SetProductKey(PRODUCT_KEY);
    HAL_SetProductSecret(PRODUCT_SECRET);
    HAL_SetDeviceName(DEVICE_NAME);
    HAL_SetDeviceSecret(DEVICE_SECRET);
}

int user_permit_join_event_handler(const char *product_key, const int time)
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();

    EXAMPLE_TRACE("Product Key: %s, Time: %d", product_key, time);

    user_example_ctx->permit_join = 1;

    return 0;
}

void *user_dispatch_yield(void *args)
{
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();

    while (user_example_ctx->g_user_dispatch_thread_running) {
        IOT_Linkkit_Yield(USER_EXAMPLE_YIELD_TIMEOUT_MS);
    }

    return NULL;
}

static int max_running_seconds = 0;
int linkkit_main(void *paras)
{
    int res = 0;
    uint64_t time_prev_sec = 0, time_now_sec = 0, time_begin_sec = 0;
    user_example_ctx_t *user_example_ctx = user_example_get_ctx();
    iotx_linkkit_dev_meta_info_t master_meta_info;

    memset(user_example_ctx, 0, sizeof(user_example_ctx_t));

#if defined(__UBUNTU_SDK_DEMO__)
    int                             argc = ((app_main_paras_t *)paras)->argc;
    char                          **argv = ((app_main_paras_t *)paras)->argv;

    if (argc > 1) {
        int tmp = atoi(argv[1]);

        if (tmp >= 60) {
            max_running_seconds = tmp;
            EXAMPLE_TRACE("set [max_running_seconds] = %d seconds\n", max_running_seconds);
        }
    }

    if (argc > 2) {
        if (strlen("auto") == strlen(argv[2]) &&
            memcmp("auto", argv[2], strlen(argv[2])) == 0) {
            user_example_ctx->auto_add_subdev = 1;
        }
    }
#endif

#if !defined(WIFI_PROVISION_ENABLED) || !defined(BUILD_AOS)
    set_iotx_info();
#endif

    user_example_ctx->subdev_index = -1;

    IOT_SetLogLevel(IOT_LOG_DEBUG);

    /* Register Callback */
    IOT_RegisterCallback(ITE_CONNECT_SUCC, user_connected_event_handler);
    IOT_RegisterCallback(ITE_DISCONNECTED, user_disconnected_event_handler);
    IOT_RegisterCallback(ITE_SERVICE_REQUST, user_service_request_event_handler);
    IOT_RegisterCallback(ITE_PROPERTY_SET, user_property_set_event_handler);
    IOT_RegisterCallback(ITE_REPORT_REPLY, user_report_reply_event_handler);
    IOT_RegisterCallback(ITE_TRIGGER_EVENT_REPLY, user_trigger_event_reply_event_handler);
    IOT_RegisterCallback(ITE_TIMESTAMP_REPLY, user_timestamp_reply_event_handler);
    IOT_RegisterCallback(ITE_INITIALIZE_COMPLETED, user_initialized);
    IOT_RegisterCallback(ITE_PERMIT_JOIN, user_permit_join_event_handler);

    memset(&master_meta_info, 0, sizeof(iotx_linkkit_dev_meta_info_t));
    memcpy(master_meta_info.product_key, PRODUCT_KEY, strlen(PRODUCT_KEY));
    memcpy(master_meta_info.product_secret, PRODUCT_SECRET, strlen(PRODUCT_SECRET));
    memcpy(master_meta_info.device_name, DEVICE_NAME, strlen(DEVICE_NAME));
    memcpy(master_meta_info.device_secret, DEVICE_SECRET, strlen(DEVICE_SECRET));

    /* Create Master Device Resources */
    user_example_ctx->master_devid = IOT_Linkkit_Open(IOTX_LINKKIT_DEV_TYPE_MASTER, &master_meta_info);
    if (user_example_ctx->master_devid < 0) {
        EXAMPLE_TRACE("IOT_Linkkit_Open Failed\n");
        return -1;
    }

    /* Choose Login Server */
    int domain_type = IOTX_CLOUD_REGION_SHANGHAI;
    IOT_Ioctl(IOTX_IOCTL_SET_DOMAIN, (void *)&domain_type);

    /* Choose Login Method */
    int dynamic_register = 0;
    IOT_Ioctl(IOTX_IOCTL_SET_DYNAMIC_REGISTER, (void *)&dynamic_register);

    /* Choose Whether You Need Post Property/Event Reply */
    int post_event_reply = 0;
    IOT_Ioctl(IOTX_IOCTL_RECV_EVENT_REPLY, (void *)&post_event_reply);
    // HAL_SleepMs(2000);
    /* Start Connect Aliyun Server */
    res = IOT_Linkkit_Connect(user_example_ctx->master_devid);
    if (res < 0) {
        EXAMPLE_TRACE("IOT_Linkkit_Connect Failed\n");
        return -1;
    }
    // HAL_SleepMs(2000);

    user_example_ctx->g_user_dispatch_thread_running = 1;
    res = HAL_ThreadCreate(&user_example_ctx->g_user_dispatch_thread, user_dispatch_yield, NULL, NULL, NULL);
    if (res < 0) {
        EXAMPLE_TRACE("HAL_ThreadCreate Failed\n");
        IOT_Linkkit_Close(user_example_ctx->master_devid);
        return -1;
    }
    int stack_used = 0;

    res = HAL_ThreadCreate(&user_example_ctx->g_user_dispatch_thread, start_socket_server, NULL, NULL, &stack_used);
    if(res == -1){
        EXAMPLE_TRACE("creact tcp server error rc: %i", res);
    }

    time_begin_sec = user_update_sec();

    HAL_SleepMs(2000);

    cJSON *root = cJSON_CreateObject();
    cJSON *sn = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "RouterName", "YYF");
    cJSON_AddItemToObject(root, "SensorName", sn);
    cJSON_AddStringToObject(sn, "SensorLocation", "Longdd");
    cJSON_AddStringToObject(sn, "SensorSerialNumber", "123456");
    // char *str2 = cJSON_Print(root);
    char *str2 = "{\"SensorName.SensorLocation\":\"Longdd\"}";
    EXAMPLE_TRACE("str2: %s", str2);
    cJSON_Delete(root);

    res = IOT_Linkkit_Report(user_example_ctx->master_devid, ITM_MSG_POST_PROPERTY, (unsigned char *)str2, strlen(str2));
    EXAMPLE_TRACE("Post Property Message ID: %d", res);
    
    while (1) {
        HAL_SleepMs(200);

        time_now_sec = user_update_sec();
        if (time_prev_sec == time_now_sec) {
            continue;
        }
        if (max_running_seconds && (time_now_sec - time_begin_sec > max_running_seconds)) {
            EXAMPLE_TRACE("Example Run for Over %d Seconds, Break Loop!\n", max_running_seconds);
            break;
        }

        /* Add subdev */
        // if (user_example_ctx->master_initialized && user_example_ctx->subdev_index >= 0 &&
        //     (user_example_ctx->auto_add_subdev == 1 || user_example_ctx->permit_join != 0)) {
        //     if (user_example_ctx->subdev_index < EXAMPLE_SUBDEV_ADD_NUM) {
        //         /* Add next subdev */
        //         if (example_add_subdev((iotx_linkkit_dev_meta_info_t *)&subdevArr[user_example_ctx->subdev_index]) == SUCCESS_RETURN) {
        //             EXAMPLE_TRACE("subdev %s add succeed", subdevArr[user_example_ctx->subdev_index].device_name);
        //         } else {
        //             EXAMPLE_TRACE("subdev %s add failed", subdevArr[user_example_ctx->subdev_index].device_name);
        //         }
        //         user_example_ctx->subdev_index++;
        //         user_example_ctx->permit_join = 0;
        //     }
        // }

        /* Post Proprety Example */
        if (time_now_sec % 11 == 0 && user_master_dev_available()) {
            // user_post_property();
        }

        /* Device Info Update Example */
        if (time_now_sec % 23 == 0 && user_master_dev_available()) {
            // user_deviceinfo_update();
        }

        /* Device Info Delete Example */
        if (time_now_sec % 29 == 0 && user_master_dev_available()) {
            // user_deviceinfo_delete();
        }

        time_prev_sec = time_now_sec;
    }

    user_example_ctx->g_user_dispatch_thread_running = 0;
    IOT_Linkkit_Close(user_example_ctx->master_devid);
    HAL_ThreadDelete(user_example_ctx->g_user_dispatch_thread);

    IOT_DumpMemoryStats(IOT_LOG_DEBUG);
    IOT_SetLogLevel(IOT_LOG_NONE);
    return 0;
}
#endif
