#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>

#include "linkkit_gateway_export.h"
#include "iot_import.h"
#include "iot_export_linkkit.h"
#include "app_entry.h"

#include "cJSON.h"
#include "tcpserver.h"

#define EXAMPLE_TRACE(...)                                      \
    do {                                                     \
        printf("\033[1;31;40m%s.%d: ", __func__, __LINE__);  \
        printf(__VA_ARGS__);                                 \
        printf("\033[0m\n");                                   \
    } while (0)

typedef struct {
    char *pk;
    char *dn;
    char *ds;
    int connected;
    int register_completed;
    int lk_dev;
} gateway_t;

typedef enum {
    OPT_MSG_POST,
    OPT_MSG_EVENT,
    OPT_MSG_SERVICE,
    OPT_MSG_UPDATE,
    OPT_MSG_DELETE
} upstream_opt_type_t;


/* callback function */



/*
 * the handler property get
 * alink method: thing.service.property.get
 */


/*
 * the handler property set
 * alink method: thing.service.property.set
 */



/*
 * the handler of service which is defined by identifier, not property
 * alink method: thing.service.{tsl.service.identifier}
 */



int sub_all(iotx_linkkit_dev_meta_info_t *meta_info){

    int res = 0, devid = -1;
    devid = IOT_Linkkit_Open(IOTX_LINKKIT_DEV_TYPE_SLAVE, meta_info);
    if (devid == FAIL_RETURN) {
        EXAMPLE_TRACE("subdev open Failed\n");
        return FAIL_RETURN;
    }
    EXAMPLE_TRACE("subdev open susseed, devid = %d\n", devid);

    res = IOT_Linkkit_Connect(devid);
    if (res == FAIL_RETURN) {
        EXAMPLE_TRACE("subdev connect Failed\n");
        return res;
    }
    EXAMPLE_TRACE("subdev connect success: devid = %d\n", devid);

    res = IOT_Linkkit_Report(devid, ITM_MSG_LOGIN, NULL, 0);
    if (res == FAIL_RETURN) {
        EXAMPLE_TRACE("subdev login Failed\n");
        return res;
    }
    EXAMPLE_TRACE("subdev login success: devid = %d\n", devid);
    return res;
}

int sub_open(iotx_linkkit_dev_meta_info_t *meta_info){
    iotx_linkkit_dev_meta_info_t *subdev = meta_info;
    int devid = -1;
    devid = IOT_Linkkit_Open(IOTX_LINKKIT_DEV_TYPE_SLAVE, subdev);
    if (devid == FAIL_RETURN) {
        EXAMPLE_TRACE("subdev open Failed\n");
        return FAIL_RETURN;
    }
    EXAMPLE_TRACE("subdev open susseed, devid = %d\n", devid);
    return devid;
}

int sub_connect(int devid){
    int res = -1;
    res = IOT_Linkkit_Connect(devid);
    if (res == FAIL_RETURN) {
        EXAMPLE_TRACE("subdev connect Failed\n");
        return res;
    }
    EXAMPLE_TRACE("subdev connect success: devid = %d\n", devid);

    return 0;
}

int sub_login(int devid){
    int res = -1;
    res = IOT_Linkkit_Report(devid, ITM_MSG_LOGIN, NULL, 0);
    if (res == FAIL_RETURN) {
        EXAMPLE_TRACE("subdev login Failed\n");
        return res;
    }
    EXAMPLE_TRACE("subdev login success: devid = %d\n", devid);

    return 0;
}

int sub_logout(int devid){
    int res = -1;

    res = IOT_Linkkit_Report(devid, ITM_MSG_LOGOUT, NULL, 0);
    if (res == FAIL_RETURN) {
        EXAMPLE_TRACE("subdev logout Failed\n");
        return res;
    }
    EXAMPLE_TRACE("subdev logout success: devid = %d\n", devid);

    return 0;
}

int sub_unconnect(int devid){
    int res = -1;

    res = IOT_Linkkit_Report(devid, ITM_MSG_DELETE_TOPO, NULL, 0);
    if (res == FAIL_RETURN) {
        EXAMPLE_TRACE("subdev unregister Failed\n");
        return res;
    }
    EXAMPLE_TRACE("subdev unregister success: devid = %d\n", devid);

    return 0;
}

int sub_close(int devid){
    int res = -1;

    res = IOT_Linkkit_Close(devid);
    if (res == FAIL_RETURN) {
        EXAMPLE_TRACE("subdev close Failed\n");
        return res;
    }
    EXAMPLE_TRACE("subdev close success: devid = %d\n", devid);

    return 0;
}

void sub_upstream(int devid, int opt_type, char *identifier, char **valuestr){
    int res = 0;
    const unsigned char *prop_payload_format = "{\"%s\":\"%s\"}";
    unsigned char *prop_payload = malloc(sizeof(char) * 2048);

    const unsigned char *event_payload_format = "{\"%s\":\"%s\"}";
    unsigned char *event_payload = malloc(sizeof(char) * 2048);
    
    const unsigned char *update_payload_format = "[{\"attrKey\":\"%s\",\"attrValue\":\"%s\"}]";
    unsigned char *update_payload = malloc(sizeof(char) * 2048);

    const unsigned char *delete_payload_format = "[{\"attrKey\":\"%s\"}]";
    unsigned char *delete_payload = malloc(sizeof(char) * 2048);

    switch(opt_type) {
        case OPT_MSG_POST:
            sprintf(prop_payload, prop_payload_format, identifier, valuestr[0]);
            res = IOT_Linkkit_Report(devid, ITM_MSG_POST_PROPERTY, prop_payload, strlen(prop_payload));
            if(res == FAIL_RETURN) {
                EXAMPLE_TRACE("Post Property Failed\n");
            }
            EXAMPLE_TRACE("Post Property Message ID: %d", res);
            break;
        case OPT_MSG_EVENT:
            sprintf(event_payload, event_payload_format, valuestr[0], valuestr[1]);
            res = IOT_Linkkit_TriggerEvent(devid, identifier, strlen(identifier), event_payload, strlen(event_payload));
            if(res == FAIL_RETURN) {
                EXAMPLE_TRACE("Trigger Event Failed\n");
            }
            EXAMPLE_TRACE("Trigger Event Message ID: %d", res);
            break;
        case OPT_MSG_SERVICE:
            EXAMPLE_TRACE("Not Support Yet\n");
            break;
        case OPT_MSG_UPDATE:
            sprintf(update_payload, update_payload_format, valuestr[0], valuestr[1]);
            res = IOT_Linkkit_Report(devid, ITM_MSG_DEVICEINFO_UPDATE, update_payload, strlen(update_payload));
            if(res == FAIL_RETURN) {
                EXAMPLE_TRACE("Device Info Update Failed\n");
            }
            EXAMPLE_TRACE("Device Info Update Message ID: %d", res);
            break;
        case OPT_MSG_DELETE:
            sprintf(delete_payload, delete_payload_format, valuestr[0], valuestr[1]);
            res = IOT_Linkkit_Report(devid, ITM_MSG_DEVICEINFO_DELETE, delete_payload, strlen(delete_payload));
            if(res == FAIL_RETURN) {
                EXAMPLE_TRACE("Device Info Delete Failed\n");
            }
            EXAMPLE_TRACE("Device Info Delete Message ID: %d", res);
            break;
        default :
            EXAMPLE_TRACE("Invalid Parameter!");
    }
    free(prop_payload);
    free(event_payload);
}

int g_devid = -1;

void *subdev(void *arg)
{
    EXAMPLE_TRACE("Entered");
    int sockfd = *(int *)arg;
    char buffer[BUFFER_SIZE];
    int ret = -1;
    
    iotx_linkkit_dev_meta_info_t *subdev = malloc(sizeof(iotx_linkkit_dev_meta_info_t));

    do
    {   
        memset(buffer, 0, sizeof(buffer));
        ret = HAL_TCP_Read(sockfd, buffer, BUFFER_SIZE, 200);
        if(ret == -1){
            // sub_logout(g_devid);
            close(sockfd);
            EXAMPLE_TRACE("close socket");
            break;
        }else if(ret > 0){
            EXAMPLE_TRACE("msg : %s", buffer);
            cJSON *user = cJSON_Parse(buffer);

            cJSON *ot = cJSON_GetObjectItem(user, "operation_type");
            cJSON *pk = cJSON_GetObjectItem(user, "product_key");
            cJSON *dn = cJSON_GetObjectItem(user, "device_name");
            cJSON *ds = cJSON_GetObjectItem(user, "device_secret");
            int csize = cJSON_GetArraySize(user);
            printf("csize : %d \r\n", csize);

            cJSON **ext = malloc(sizeof(cJSON*) * 1024);
            char *extstr = malloc(sizeof(char) * 5);
            int cycle = 0;
            
            if(csize > 4)
            {
                for(int i = 0; i < csize - 4; i++)
                {
                    sprintf(extstr, "ext%d", i);
                    printf("extstr : %s \r\n", extstr);
                    ext[i] = cJSON_GetObjectItem(user, extstr);
                    printf("ext%d : %s \r\n", i, ext[i]->valuestring);
                    cycle = i + 1;
                }
            }
            
            char *operation_type = ot->valuestring;
            char *product_key = pk->valuestring;
            char *device_name = "";
            char *identifier = {0};
            char **valuestr = malloc(sizeof(char*) * 2048);
            if(!strcmp(operation_type, "open"))
            {
                device_name = dn->valuestring;
            } else if(!strcmp(operation_type, "post") || !strcmp(operation_type, "event") || !strcmp(operation_type, "service")){
                g_devid = atoi(dn->valuestring);
                identifier = ds->valuestring;
                for(int j = 0; j < cycle; j++)
                {
                    valuestr[j] = ext[j]->valuestring;
                }
                
            } else {
                g_devid = atoi(dn->valuestring);
            }
            
            char* device_secret = ds->valuestring;
            free(ext);
            
            strcpy(subdev->product_key, product_key);
            strcpy(subdev->device_name, device_name);
            strcpy(subdev->device_secret, device_secret);

            EXAMPLE_TRACE("product_key : %s", product_key);
            EXAMPLE_TRACE("device_name : %s", device_name);
            EXAMPLE_TRACE("device_secret : %s", device_secret);
            EXAMPLE_TRACE("operation_type : %s", operation_type);
            if (strcmp(device_secret, "null") == 0)
            {
                device_secret = NULL;
            }
            if (strcmp(operation_type, "open") == 0) {
                g_devid = sub_open(subdev);
                if (g_devid < 0) {
                    EXAMPLE_TRACE("open error close socket");
                    close(sockfd);
                    break;
                }
                close(sockfd);
                break;
            } else if (strcmp(operation_type, "connect") == 0) {
                if(sub_connect(g_devid) != 0){
                    EXAMPLE_TRACE("connect error close socket");
                    close(sockfd);
                    break;
                }
                close(sockfd);
                break;
            } else if (strcmp(operation_type, "login") == 0) {
                if(sub_login(g_devid) != 0){
                    EXAMPLE_TRACE("login error close socket");
                    close(sockfd);
                    break;
                }
                close(sockfd);
                break;
            } else if (strcmp(operation_type, "logout") == 0) {
                if(sub_logout(g_devid) != 0){
                    EXAMPLE_TRACE("logout error close socket");
                    close(sockfd);
                    break;
                }
                close(sockfd);
                break;
            } else if (strcmp(operation_type, "unconnect") == 0) {
                if(sub_unconnect(g_devid) != 0){
                    EXAMPLE_TRACE("unconnect error close socket");
                    close(sockfd);
                    break;
                }
                close(sockfd);
                break;
            } else if (strcmp(operation_type, "close") == 0) {
                if(sub_close(g_devid) != 0){
                    EXAMPLE_TRACE("close error close socket");
                    close(sockfd);
                    break;
                }
                close(sockfd);
                break;
            } else if (strcmp(operation_type, "all") == 0) {
                if(sub_all(subdev) != 0){
                    EXAMPLE_TRACE("all error close socket");
                    close(sockfd);
                    break;
                }
                close(sockfd);
                break;
            } else if (strcmp(operation_type, "post") == 0) {
                sub_upstream(g_devid, OPT_MSG_POST, identifier, valuestr);
                close(sockfd);
                free(valuestr);
                break;
            } else if (strcmp(operation_type, "event") == 0) {
                sub_upstream(g_devid, OPT_MSG_EVENT, identifier, valuestr);
                close(sockfd);
                free(valuestr);
                break;
            } else {
                EXAMPLE_TRACE("Invalid Paramter!");
                close(sockfd);
                break;
            }
        }
    } while(1);
   	return 0;
}

void *start_socket_server()
{
    EXAMPLE_TRACE("start_socket_server.");
    //定义IPV4的TCP连接的套接字描述符
    int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    //定义sockaddr_in
    struct sockaddr_in server_sockaddr;
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_sockaddr.sin_port = htons(SERVERPORT);
    //bind成功返回0，出错返回-1
    if(bind(server_sockfd, (struct sockaddr *)&server_sockaddr, sizeof(server_sockaddr))==-1)
    {
        perror("bind error");
    }
    EXAMPLE_TRACE("bind success.");
    //listen成功返回0，出错返回-1，允许同时帧听的连接数为QUEUE_SIZE
    if(listen(server_sockfd, QUEUE_SIZE) == -1)
    {
        perror("listen error");
    }
    EXAMPLE_TRACE("listen success.");
    for(;;)
    {
        struct sockaddr_in client_addr;
        socklen_t length = sizeof(client_addr);
        //进程阻塞在accept上，成功返回非负描述字，出错返回-1
        int conn = accept(server_sockfd, (struct sockaddr*)&client_addr, &length);
        if(conn < 0)
        {
            perror("connect error! \r\n");
        }
        EXAMPLE_TRACE("new client accepted.");
        void *sub_ctx = {0};
        int stack_used = 0;
        int res = -1;
        res = HAL_ThreadCreate(&sub_ctx, subdev, &conn, NULL, &stack_used);
        if(res == -1){
            EXAMPLE_TRACE("creact tcp server error rc: %i", res);
        }
    }
    EXAMPLE_TRACE("close server_socket!");
    close(server_sockfd);

    return 0;
}
