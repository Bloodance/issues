linkkit2.3如何编译出so
diff --git a/src/board/config.ubuntu.x86 b/src/board/config.ubuntu.x86

index adab66e..e82a992 100644

--- a/src/board/config.ubuntu.x86

+++ b/src/board/config.ubuntu.x86

@@ -1,6 +1,5 @@

CONFIG_ENV_CFLAGS   += \

-Os -Wall \

-    -g3 --coverage \

-D_PLATFORM_IS_LINUX_ \

-D__UBUNTU_SDK_DEMO__ \

@@ -32,3 +31,4 @@ endif

CONFIG_ENV_LDFLAGS  += -lpthread -lrt

OVERRIDE_STRIP      := strip

+CONFIG_LIB_EXPORT    = dynamic

diff --git a/src/ref-impl/hal/iot.mk b/src/ref-impl/hal/iot.mk

index 120a3a1..1e72044 100644

--- a/src/ref-impl/hal/iot.mk

+++ b/src/ref-impl/hal/iot.mk

@@ -1,4 +1,4 @@

-LIBA_TARGET := libiot_hal.a

+LIBSO_TARGET := libiot_hal.so

CFLAGS      := $(filter-out -ansi,$(CFLAGS))

HDR_REFS    += src/ref-impl/tls

diff --git a/src/ref-impl/tls/iot.mk b/src/ref-impl/tls/iot.mk

index 6bc7dfd..d5a50e8 100644

--- a/src/ref-impl/tls/iot.mk

+++ b/src/ref-impl/tls/iot.mk

@@ -1,4 +1,4 @@

-LIBA_TARGET := libiot_tls.a

+LIBSO_TARGET := libiot_tls.so

CFLAGS      := $(filter-out -ansi,$(CFLAGS))

HDR_REFS    += src/infra

网关：子设备注册失败

现象
注册函数等待了10秒返回注册失败。



可能原因
1、网关还没有初始化完成。需要在网关的gateway_register_complete成功后才能执行子设备create、register、login操作。

2、子设备的register、login等同步函数不能在回调中使用。具体使用方法请参考example。

注意新版本的api(sdk 2.3.0)，IOT_Linkkit_Connect、IOT_Linkkit_Report内部也是同步函数。

3、真的超时了

aos1.3.3透传云端下发数据异常

描述：云端下发的数据，云log与设备端log中payload不一致，经查是设备端解析错误，可通过以下补丁解决


dhcp 超时
aos2.0.0 
dhcp超时，lwip会分配一个假的ip地址，导致设备永远无法上线。
修改相应mcu下的配置文件中的宏MAX_DHCP_TRIES，如
platform/mcu/rtl8710bn/sdk/component/common/api/lwip_netconf.c:94:#define MAX_DHCP_TRIES 5   改为15

APP拿不到token的可能性（设备端只发送一次token）
格兰仕微波炉（二期）项目中又碰到客户配网只发了一次，调查原因在以下：
客户自己调用了一次awss_report_cloud这个函数，导致在该函数内部调用stop函数的时候前一次TIMER已经启动，这里直接把计数器赋值成了最大值。

在发送token的这个地方直接红框中的if语句没有进入，TIMER没创建，所以这里只发送了一次token设备端就不发了，有可能会导致在配网过程中APP拿不到token这样的问题。

交叉编译链工具相关问题
一、出现如下could not read symbols: Flie in wrong format提示，是由于交叉编译工具版本过低，导致在编译工程的时候出现无法识别库文件的现象。需要升级交叉编译工具到最新版本。

查看.a文件编译器版本
readelf ./output/release/lib/libiot_sdk.a -p .comment
linkkit 2.2.1 问题
以下问题必须要用户注意。
1、本地发现设备默认没有开启。
打上补丁 
https://github.com/kid2682/IOT-aos-linkkit/tree/fix_linkkit2.2.1_awss_not_enable
2、app发起子设备搜索，设备无法进入 LINKKIT_EVENT_SUBDEV_PERMITED  事件
./src/services/linkkit/dm/linkkit_gateway_legacy.h
#define LINKKIT_GATEWAY_LEGACY_KEY_PRODUCT_KEY "product_key"
#define LINKKIT_GATEWAY_LEGACY_KEY_TIMEOUT     "timeout"
改成
#define LINKKIT_GATEWAY_LEGACY_KEY_PRODUCT_KEY "productKey"
#define LINKKIT_GATEWAY_LEGACY_KEY_TIMEOUT     "time"
3、id.patch补丁
id.zip(1.83 kB)
解压后 在项目中输入 git am --signoff < id.patch
4、子设备动态注册
sdk不会维护子设备的ds，所以注册过的设备还是要调用 linkkit_gateway_subdev_register 来重新获取ds
5、NOT more @sub_handle space! 
问题原因，子设备注册过多，CONFIG_MQTT_SUBTOPIC_MAXNUM 默认只有100。
如果客户需要挂载多个子设备的话，可以调大这个数，这个版本1个子设备消耗内存1kb左右。
6、子设备的操作需要在gateway_register_complete回调之后执行。例子中使用了延迟，客户有可能会删除这段代码需要注意。
7、如果客户收到数据不完整，确认数据量是否很大。没有做特殊配置的情况下默认是 1024 byte。
通过编译时加上 CONFIG_ENV_CFLAGS   += -DCONFIG_MQTT_RX_MAXLEN=5000 来调整大小

sdk 编译问题
1、2.2.1 代码中直接调用 HAL层的函数

make src/infra/log
获取 .O/usr/lib/libiot_log.a
然后  -liot_sdk -liot_hal -liot_tls -liot_log

配网排查
1、passwd err 
可能情况，解密函数问题。验证一下解密函数。
验证函数
unsigned char iv[16] = {0};
unsigned char key[16] = {207,12,174,46,47,250,168,12,131,204,2,12,221,180,137,174};
unsigned char src[20] = {0xd7,0xf6,0x54,0xec,0xd6,0x19,0x50,0x75,0xff,0x4e,0x93,0xb4,0xab,0x36,0x9a,0x3,0x72,0xe7,0x59,0x9d};
unsigned char dst[20] = {0};
p_HAL_Aes128_t aes_d_h;
int i;
aes_d_h = HAL_Aes128_Init(key, iv, HAL_AES_DECRYPTION);
HAL_Aes128_Cfb_Decrypt(aes_d_h, src, sizeof(src), dst);
解密出来的数据
0x63  0x61  0x6F  0x68  0x61  0x69  0x62  0x6F
0x31  0x39  0x38  0x33  0x63  0x61  0x6F  0x68
0x61  0x69  0x62  0x6F
2、WARNING ON 706!!! p2p decrypt passwd content err 
app选的设备和配网设备不是同一个

date字段上报问题
问题sdk版本：aos 1.3.2（其他待测试）
描述：date字段上报会出现出现上报成功，但是云端无法解析，导致app端收不到的问题。
   云端下发的date类型的数据，getvalue也无法获取，这里可以通过以下补丁解决

OTA问题
aos1.3.1、aos 1.3.2的话，先请用户开启log宏。
AliOS-Things\include\aos\internal\log_impl.h
注释掉下面这个宏
#define CONFIG_LOGMACRO_SILENT
Linkkit版本号问题：
实现HAL_GetFirmwareVesion这个函数会让用户可以定义自己的版本号
使用OTA时要注意的问题：
1、上传云端的固件版本号填写要与common.mk中的版本一致，否则会出现无法升级的情况
设备离线状态下上线后，感知云端OTA的patch：
1.aos1.3.2
ota_request.zip(1.24 kB)
2.aos1.3.3
ota_patch.zip(1.44 kB)
CoAP问题
1、CoAP Network init failed
可能是端口冲突。Linux环境上运行的话，看看是否有两个sdk应用在运行。
socket栈小了
2、连接成功后，后台显示未激活。 2018年 9 -10月解决这个问题。
3、2.2.0之后的版本由于域名改变，代码这里没有改动导致，CoAP example无法运行。请进行如下修改
./src/protocol/coap/iotx_coap_api.c 
48行修改
#define IOTX_COAP_ONLINE_DTLS_SERVER_URL "coaps://%s.coap.cn-shanghai.link.aliyuncs.com:5684"

./examples/coap/coap-example.c
39行修改
#define IOTX_ONLINE_DTLS_SERVER_URL     "coaps://%s.coap.cn-shanghai.link.aliyuncs.com:5684"
4、ALCS Register Resource Failed, Code: 261 错误
[10-23 14:27:53][inf] alcs_resource_register_secure(439): alcs_resource_register_secure
[10-23 14:27:53][inf] CoAPResource_register(114): The resource count exceeds limit, cur 32, max 32
[10-23 14:27:53][err] ALCS Register Resource Failed, Code: 261
./middleware/linkkit/sdk-c/src/protocol/alcs/CoAPServer.c
116行
param.res_maxcount = 32;
32改大就行了。48


esp8266问题
1、启动后显示如下信息
[WiFi] event 2
Wifi disconnected (reason: 201)
reconnect
scandone
no TmallRouter_iottest02 found, reconnect after 1s
恢复出厂设置后正常
2、开发板配网按钮实现
platform\mcu\esp8266\bsp\key.c 这文件中的key_poll_func函数有用到一个gpio 14的口，并且代码里面调用了aos_post_event函数发送了CODE_BOOT。搜索了全部代码后发现linkkit_key_process这个函数里面会用到这个CODE_BOOT，可以看到里面根据按键时长调用了do_awss_active和do_awss_reset。那么可以得出gpio 14就是要的配网按钮。
可以看到的esp8266设计图，里面有红色圈出来的，找一根线，连接 io 14 和 22 。然后按22 右边那个按钮就可以了。

3、日志输出占用了2个uart口
由于配网的输出已经在.a文件里面了，无法进行修改。只能修改AliOS-Things/framework/protocol/linkkit/iotkit/hal-impl/rhino/HAL_OS_rhino.c文件中的Hal_printf函数，控制日志使用同一个uart输出。Hal层中调用 os_printf
4、开启CLI功能
修改entry.c代码中 kinit 变量的cli_ebable的值为1
static kinit_t kinit = {
    .argc = 0,
    .argv = NULL,
    .cli_enable = 1
};
entry.c文件位于 "xxx\AliOS-Things\platform\mcu\esp8266\bsp"。

常见问题记录
1、aos如何获取mac地址
HAL_Wifi_Get_Mac
2、解绑网关会自动解绑下面的子设备吗？
网关设备与子设备的关系是不会解绑掉的。 只会解绑网关设备与用户的关系
3、编译提示ERROR: not in AliOS-Things source code directory错误
       升级aos-cube版本 
4、dm_shw_get_property_data(472): Invalid Parameter
可能是tsl问题，查看tsl中是否有这个属性。
如果是动态tsl，查看tsl是否下载成功。tsl下载失败可能是buffer不够导致的。
write卡死
aos版本里，断网时write函数会卡住6分钟，修改宏之后，可以减小到10秒钟左右。
#ifndef TCP_MAXRTX
#define TCP_MAXRTX                      6
#endif
linkkit2.0.1，运行在linux平台上，断网时，write函数有可能一直卡住。在创建socket后，可以通过setsockopt设置超时。示例如下：
struct timeval sendtimeout;
sendtimeout.tv_sec = timeout;
sendtimeout.tv_usec = 0;
if (0 != setsockopt(ctx->fd, SOL_SOCKET, SO_SNDTIMEO, &sendtimeout, sizeof(sendtimeout))) {
     hal_err("setsockopt error");
}
hal_info("setsockopt SO_SNDTIMEO timeout: %ds", sendtimeout.tv_sec);
RAD5981 无任何错误日志输出 disconnect的问题
0001-mqtt-fix-disconect.zip(0.99 kB)
打上这个补丁
git am 补丁路径

飞燕平台设备开启动态注册
新建产品的时候，通讯方式要选择ZigBee。然后在量产管理里面可以设置烧录方式“一机一密”或者“一型一密”。
动态注册（一型一密）帮助文档：https://help.aliyun.com/document_detail/74006.html?spm=a2c4g.11186623.6.620.lO1yxY

Ssl_error 常见错误
概述
在使用IoT套件过程中会遇到一些ssl fail的case，本文档介绍出ssl fail的常见错误。
如何打开tls日志：
在iotx-sdk-c\src\tls\mbedtls-in-iotkit\include\mbedtls\configs.h中定义：MBEDTLS_DEBUG_C
在\iotx-sdk-c\src\platform\ssl\mbedtls\HAL_TLS_mbedtls.c 中定义debug 等级：#define DEBUG_LEVEL 10
重新 \iotx-sdk-c\src\platform\ssl\mbedtls\HAL_TLS_mbedtls.c 中_ssl_debug的实现
static void _ssl_debug(void *ctx, int level, const char file, int line, const char str)
{
((void) level);
printf( "%s:%04d: %s", file, line, str );​
}
 常见错误
错误定义在：base\tls\mbedtls\include\mbedtls\ssl.h
 MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY -0x7880 (-30848)
错误：云端把SSL连接断开 The peer notified us that the connection is going to be closed。
可能原因： 
设备端数据连接过于频繁，触发云端限流，断开设备。请关闭设备，等待一段时间(5分钟以后)再发起连接
有多个设备使用相同的product_key和device_name与云端建立连接，之前连接的设备被云端踢开。
设备端保活出错，没有及时发送MQTT ping packet （出错通常在连接建立1分钟以后）。
如果是保活问题，需要进一步分析。
如果从未连接成功过，可以考虑是不是大小端口不匹配。
目前SDK 默认是小端，如果在大端环境上工作，请添加定义全局编译条件   REVERSED。
MBEDTLS_ERR_SSL_PEER_VERIFY_FAILED -0x7800 (-30720)
错误：认证错误 Verification of our peer failed
可能原因： 
证书错误。
由于所有的环境使用的证书都是一样的，而且都是存放在套件内部的，因此这个可能性不大。
日常环境SSL域名校验错误。
如果出错时，连接的是日常环境，日常不支持SSL域名校验，请将FORCE_SSL_VERIFY的定义去掉。
 MBEDTLS_ERR_SSL_INVALID_RECORD -0x7200 (-29184)
错误：收到的数据出错
可能原因：
tcp/ip协议栈收到的数据包出错，需要协议栈排查问题。
ssl运行的线程栈过小，调整线程栈大小。
ssl自身内存配置太小，请调整ssl memory的配置。   调整#define MBEDTLS_SSL_MAX_CONTENT_LEN 为4096， 默认是16K。
MBEDTLS_ERR_NET_UNKNOWN_HOST -0X0052 (-82)
错误：dns 失败
可能原因： 大概率是设备端当前网络故障，无法访问。
 MBEDTLS_ERR_NET_BUFFER_TOO_SMALL -0x0043 (-67)
错误：ssl内存配置太小 调整#define MBEDTLS_SSL_MAX_CONTENT_LEN 为4096， 默认是16K
可能原因： ssl自身内存配置太小，请调整ssl memory的配置
MBEDTLS_ERR_NET_SOCKET_FAILED -0x0042 (-66)
错误：创建socket失败
可能原因： socket已经全部被使用。通常socket作为一个resource，都是有上限的。请检查当前的流程是否有socket 的内存泄漏，有些socket忘记close。如果流程正常，确实同一个时间内需要建立多个socket，请调整socket的个数上限。
MBEDTLS_ERR_NET_CONNECT_FAILED -0x0044 (-68)
错误：TCP connect fail
可能原因： tcp连接失败，请确认URL和port是否正确。
MBEDTLS_ERR_X509_CERT_VERIFY_FAILED -0x2700
问题：证书错误
可能原因： 检查证书的正确性。在日常和预发请把域名校验关闭。
MBEDTLS_ERR_SSL_WANT_READ -26880
问题：linuxhost error
如果在alios linuxhost上运行出现该错误，请忽略即可。


服务器地址切换
1.3.2 
linkkitapp.mk 
GLOBAL_DEFINES      += MQTT_DIRECT ALIOT_DEBUG IOTX_DEBUG USE_LPTHREAD HAL_ASYNC_API COAP_USE_PLATFORM_LOG
改成
GLOBAL_DEFINES      += SUPPORT_SINGAPORE_DOMAIN ALIOT_DEBUG IOTX_DEBUG USE_LPTHREAD HAL_ASYNC_API COAP_USE_PLATFORM_LOG

MQTT常见错误
错误定义在：base\utils\exports\iot_export_errno.h
 MQTT_CONNACK_BAD_USERDATA_ERROR -35
错误码：CONNACK中的错误对应的是4.  Connect request failed with the server returning a bad userdata error
可能原因：云端认为三元组错误而拒绝设备端的连接请求
使用了错误的设备，例如使用日常的设备连接线上等。
三元组传入的参数不正确。
修改过加密方式。
由于某种原因，设备三元组被禁用，如果还想使用该设备三元组，请在控制台或是使用服务端相应API解除禁用。
检查服务器连接是否正确（国内、国外）
是否是未充值欠费导致的被云端拒绝
MQTT_CONNACK_IDENTIFIER_REJECTED_ERROR -37 
错误码：CONNACK中的错误对应的是2.  Connect request failed with the server returning an identifier rejected error
可能原因：云端认为设备权限而拒绝设备端的连接请求。
创建设备的账号是否有效。
 MQTT_PUSH_TO_LIST_ERROR -42
错误码：订阅或是发布(QoS)太多太快，导致list 满了而出错
可能原因：
如果是订阅过程中发生的错误，说明当前订阅list太小。请调整IOTX_MC_SUB_NUM_MAX的大小。
如果是发布的时候发生的错误，可能是发送的频率太快，或是网络状态不佳。可以考虑使用QoS0来发布。
可以考虑调整IOTX_MC_REPUB_NUM_MAX的大小。
subscribe fail
调用IOT_MQTT_Subscribe后直接返回失败。
可能错误：
如果subscribe的topic较长，可能是topic的长度超过最大值。
请调整IOTX_MC_TOPIC_NAME_MAX_LEN的定义。
 NOT more @sub_handle space!
日志打印如下错误：
[err] iotx_mc_handle_recv_SUBACK(1034): NOT more @sub_handle space!
[err] iotx_mc_cycle(1275): recvSubAckProc error,result = -1
可能错误：订阅topic太多，导致list 满了而出错
请调整IOTX_MC_SUB_NUM_MAX的大小。
错误消息： ssl recv error: code = -29184
   修改这里会大量消耗资源，要客户自己按照模组资源来调整。
修改长度
security/mbedtls/include/mbedtls/ssl.h  MBEDTLS_SSL_MAX_CONTENT_LEN 
以上适用于1.3.3之前的版本（不包括1.3.3）。1.3.3版本之后
security/alicrypto/mbedtls/include/mbedtls/ssl.h 文件
错误消息：mqtt read buffer is too short, mqttReadBufLen，请修改
 修改这里会大量消耗资源，要客户自己按照模组资源来调整。
protocol/linkkit/cm/src/iotx_cloud_conn_mqtt.c 
#ifdef ESP8266
#define MQTT_MSGLEN (1024)
#else
#define MQTT_MSGLEN (8 * 1024)
#endif
以上适用于1.3.3之前的版本（不包括1.3.3）。1.3.3版本之后
linux环境：framework/protocol/linkkit/sdk/iotx-sdk-c_clone/src/board/config.rhino.make
扩大其中的 CONFIG_MQTT_TX_MAXLEN 和 CONFIG_MQTT_RX_MAXLEN, 分别表示扩大最大的MQTT上行/下行报文缓冲区长度, 如:
CONFIG_ENV_CFLAGS += \
-DCONFIG_HTTP_AUTH_TIMEOUT=500 \
-DCONFIG_MID_HTTP_TIMEOUT=500 \
-DCONFIG_GUIDER_AUTH_TIMEOUT=500 \
-DCONFIG_MQTT_TX_MAXLEN=640 \
-DCONFIG_MQTT_RX_MAXLEN=1200 \
改成
CONFIG_ENV_CFLAGS += \
-DCONFIG_HTTP_AUTH_TIMEOUT=500 \
-DCONFIG_MID_HTTP_TIMEOUT=500 \
-DCONFIG_GUIDER_AUTH_TIMEOUT=500 \
-DCONFIG_MQTT_TX_MAXLEN=10240 \
-DCONFIG_MQTT_RX_MAXLEN=12000 \
windows环境：/framework/protocol/linkkit/sdk/iotx-sdk-c_clone/aos.makefile
-DCONFIG_MQTT_TX_MAXLEN=640 -DCONFIG_MQTT_RX_MAXLEN=1200
改成
-DCONFIG_MQTT_TX_MAXLEN=10240 -DCONFIG_MQTT_RX_MAXLEN=12000
aos2.0.0：
Linux该宏的路径为：AliOS-Things\middleware\linkkit\sdk-c\src\board\config.rhino.make
Windows该宏的路径 ：  AliOS-Things\middleware\linkkit\sdk-c\sdk-c.mk
