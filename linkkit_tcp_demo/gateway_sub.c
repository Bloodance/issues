#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include "cJSON.h"
#include "iot_import.h"
#include "iot_export.h"
#include "iotx_log.h"
#define PORT  8890
#define BUFFER_SIZE 2048
#define HOST "127.0.0.1"

#define SUB_PRODUCT_KEY              "a1ZFauqf6M8"

cJSON *SUB_MSG;

void sendMessage(int sockfd, char *input)
{
    printf("sendMessage input : %s \r\n", input);
    int i;
    int cycle = 0;
    char **command = malloc(sizeof(char*) * 2048);
    command[0] = strtok(input, ",");
    
    for(i = 1; command[i-1] != NULL; i++)
    {
        command[i] = strtok(NULL, ",");
        printf("command[%d] : %s \r\n", i, command[i]);
        cycle = i;
    }
    
    char *null = "null";
    
    
    cJSON *user = cJSON_CreateObject();
    cJSON *product_key = NULL;
    cJSON *device_name = NULL;
    cJSON *device_secret = NULL;
    cJSON *operation_type = NULL;
    cJSON **ext = malloc(sizeof(cJSON*) * 10);
    
    product_key = cJSON_CreateString(SUB_PRODUCT_KEY);
    device_name = cJSON_CreateString(command[1]);
    if(command[2] == NULL)
    {
        device_secret = cJSON_CreateString(null);
    } else {
        device_secret = cJSON_CreateString(command[2]);
    }
    operation_type = cJSON_CreateString(command[0]);

    if(cycle > 3)
    {
        for(int j = 3; j < cycle; j++)
        {
            ext[j - 3] =  cJSON_CreateString(command[j]);
        }
    }

    cJSON_AddItemToObject(user, "product_key", product_key);
    cJSON_AddItemToObject(user, "device_name", device_name);
    cJSON_AddItemToObject(user, "device_secret", device_secret);
    cJSON_AddItemToObject(user, "operation_type", operation_type);

    char *cjsonstr = malloc(sizeof(char) * 2048);

    for(int k = 0; k < (cycle - 3); k++)
    {
        sprintf(cjsonstr, "ext%d", k);
        cJSON_AddItemToObject(user, cjsonstr, ext[k]);
    }

    // char* temp = type;

    char * u = cJSON_Print(user);
    send(sockfd, u, strlen(u),0); ///发送
    cJSON_Delete(user);
    free(command);
    free(cjsonstr);
    free(ext);
}

 
int linkkit_main(int argc, char **argv)
{
    while(1)
    {
        char *input;
        setbuf(stdin, NULL);

        input = malloc(sizeof(char) * 2048);
        printf("enter subdev information! \r\n");
        int scanfret = -1;
        
        scanfret = scanf("%[^\n]", input);
        printf("input: %s \r\n", input);
        if(scanfret < 0)
        {
            printf("scanf error! \r\n");
        }

        int sock_cli;
        // char buffer[BUFFER_SIZE];
        // int ret;
        SUB_MSG = cJSON_CreateObject();
        //定义IPV4的TCP连接的套接字描述符
        sock_cli = socket(AF_INET, SOCK_STREAM, 0);
        
        //定义sockaddr_in
        struct sockaddr_in servaddr;
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = inet_addr(HOST);
        servaddr.sin_port = htons(PORT);  //服务器端口

        //连接服务器，成功返回0，错误返回-1
        if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        {
            perror("connect");
            exit(1);
        }
        
        sendMessage(sock_cli, input);

        // while(1){
        //     memset(buffer, 0, sizeof(buffer));
        //     ret = HAL_TCP_Read(sock_cli, buffer, BUFFER_SIZE, 2000);
        //     if(ret == -1){
        //         close(sock_cli);
        //         break;
        //     } else if(ret > 0) {
        //         cJSON * user = cJSON_Parse(buffer);
        //         cJSON * actionJSON = cJSON_GetObjectItem(user, "action");
        //         const char* action = actionJSON->valuestring;
        //         // if(action == "register"){

        //         // }
        //         printf("msg : %s\n", action);
        //         close(sock_cli);
        //         break;
        //     }
        // }
        HAL_SleepMs(1000);
        close(sock_cli);
        printf("send done! \r\n");
    }
    return 0;
}

