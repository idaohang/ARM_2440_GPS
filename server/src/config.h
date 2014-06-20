#ifndef __CONFIG_H__    /*配置文件定义一些常量*/
#define __CONFIG_H__

#define SERVE_PORT 	  		8000	/*服务端要绑定的端口号*/
#define THREAD_ACCEPT_MAX	5		/*创建的线程池中的线程数目*/
#define CLIENT_ID_MAX 		12      /*客户端ID  的最大长度，包含结束符*/
#define CONN_NUM_MAX  		256		/*可以同时建立连接的最大客户端个数*/
#define ACCEPT_THREAD_MAX 	3		/*创建的监听套接字线程池中所包含的最大线程数*/
#define DATA_MAX_SIZE 		1024	/*数据缓冲区的最大字节数*/
#define MSG_HEART_BEAT 		3		/*命令类型是心跳包*/
#define HEART_BEAT_TIME		5		/*心跳时间间隔，单位为s*/
#define HEART_MAX_CNT		3		/*允许心跳包未成功的最大次数*/
#define DEBUG				1   
#define CQUEUE_SIZE			4096	/*为每个客户端分配的循环队列大小*/

/*命令类型*/
#define COMM_TYPE_ALARM		0x00				/*报警命令*/
#define COMM_TYPE_SERVE		0x10				/*服务命令*/
#define COMM_TYPE_HEARBEAT	0x30				/*心跳包*/
/*报警命令字*/
#define COMM_ALARM_CARJACKING		0x01		/*劫车报警*/
#define COMM_ALARM_TIREPRESURE		0x02		/*胎压报警*/
#define COMM_ALARM_BEYONGD_REGION	0x03		/*越出区域报警*/
#define COMM_ALARM_CARJACKING_C		0x04		/*解除劫车报警*/
#define COMM_ALARM_TIREPRESURE_C	0x05		/*解除胎压报警*/
#define COMM_ALARM_BEYONGD_REGION_C	0x06		/*解除越出区域报警*/

/*服务命令字*/
#define COMM_SERV_GPS				0x11		/*上传GPS  数据包*/
#define COMM_SERV_LOCK				0x12		/*锁车指令*/
#define	COMM_SERV_UNLOCK			0x13		/*解锁指令*/
#define COMM_SERV_BITMAP			0x14		/*上传图片*/

#define HEAD  0x7E				/*包头*/
#define END   0x7F				/*包尾*/

#define DATA_START				6	/*普通数据包数据起始位置*/
#define FILE_NAME_LEN_START		6	/*图片数据区域，文件名长度的数据起始区*/
#define FILE_NAME_START			10	/*图片数据区域，存放文件名起始位置*/

#define BUFFER_SIZE 			1024
#define NAME_MAX_SIZE			50

#endif /* __CONFIG_H__ */
