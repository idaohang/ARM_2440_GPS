#ifndef __CONFIG_H__    /*配置文件定义一些常量*/
#define __CONFIG_H__

#define CLIEN_ID	  		"12345678901" 	/*本客户端ID*/
#define SERVE_PORT 	  		8000			/*服务端要绑定的端口号*/
#define CLIENT_ID_MAX 		12        		/*客户端ID  的最大长度，包含结束符*/
#define MAXSLEEP 			128				/*connect 超时重连的最长睡眠时间*/	
#define HEART_BEAT_TIME		5				/*心跳时间间隔，单位为s*/
#define HEART_MAX_CNT		3				/*允许心跳包未成功的最大次数*/
#define DATA_MAX_SIZE 		1024
#define DEBUG				1

#define GPS_UPLOAD_TIME		15				/*定时上传GPS  数据包的时间间隔*/
#define MSG_HEART_BEAT 		1
#define MSG_IMAGE_UPLOAD    2

#define DEV_BUTTON_CHAR_PATH	"/dev/buttons"
#define DEV_BUTTON_INPUT_PATH 	"/dev/input/event1"
#define DEV_PWN_PATH 			"/dev/mini2440_pwm"
#define DEV_LED_PATH 			"/dev/mini2440_led"

#define IS_MINI2440			1

#define PWM_IOCTL_SET_FREQ		1
#define CHAR_DEV_DRIVE				0					/*按键驱动是否是字符驱动*/
#define INPUT_DEV_DRIVE			1					/*按键驱动是否是输入子系统*/

#define SERVER_ADDR		"219.229.129.235"	
#define CQUEUE_SIZE		4096						/*消息队列的最大长度*/

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

#define BUFFER_SIZE 1024
#define NAME_MAX_SIZE	50

#endif /* __CONFIG_H__ */
