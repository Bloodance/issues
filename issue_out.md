# HF-AliOS-Things1.3.2和HF-AliOS-Things1.3.3纯净SDK可能会碰到的问题

## 一、1.3.2可能出现的问题：

###	1.上报事件和属性必须在linkkit_action里面 否则会失败(aos1.3.3是否有这个bug未知)

###	2.飞燕后台在设备端离线状态OTA，设备端没有反应(aos1.3.2bug)

###	3.date字段无法上报和下发的值有问题(aos1.3.2bug)

###	4.ntp获取到的时间不正确:更新awss新的开源代码，编译即可(照理说)

###	5.新版汉枫SDK为了支持动态拉取TSL，默认的修改了MQTT_MSGLEN和MBEDTLS_SLL_MAX_CONTENT_LEN的倍数 这样会导致内存可能不够用



## 二、1.3.3可能出现的问题：
	
###	1.透传下来的数据payload会解析不正确,尚不清楚HF是否打补丁，可以通过补丁解决(aos1.3.3bug)
