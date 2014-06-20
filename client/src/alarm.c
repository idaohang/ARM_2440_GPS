#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>			/*usleep*/
#include <linux/ioctl.h>
#include <linux/input.h>
#include <pthread.h>		/*pthread*/

#include "alarm.h"
#include "config.h"
#include "logc.h"
#include "wrap.h"
#include "socket.h"  /*writen*/

#define MINI2440_LED_CMD_0 _IOW('l', 0, int)
#define MINI2440_LED_CMD_1 _IOW('l', 1, int)
#define MINI2440_LED_CMD_2 _IOW('l', 2, int)
#define MINI2440_LED_CMD_3 _IOW('l', 3, int)

extern int led_fd;
extern int led1_status, led2_status;

void beyond_region_alarm();

void* init_alarm_func(void *param){
	int button_fd, pwn_fd;
	struct input_event ev_key;
	
#if	CHAR_DEV_DRIVE				/*字符设备*/
	button_fd = open(DEV_BUTTON_CHAR_PATH, 0);
#elif	INPUT_DEV_DRIVE		/*输入子系统*/
	button_fd = open(DEV_BUTTON_INPUT_PATH,0);
#endif

	if(button_fd < 0){
		LOG(LOG_ERROR, "按键%s无法正常工作:%s",
#if	CHAR_DEV_DRIVE
						DEV_BUTTON_CHAR_PATH, 
#elif	INPUT_DEV_DRIVE
						DEV_BUTTON_INPUT_PATH,
							strerror(errno));
#endif
		exit(-1);
	}

	pwn_fd = open(DEV_PWN_PATH, O_RDWR | O_NONBLOCK);
	if(pwn_fd < 0){
		LOG(LOG_ERROR, "蜂鸣器无法正常打开:%s", strerror(errno));
		exit(-1);
	}                  

	for(; ;){
		/*这些都需要在连接正常的情况下进行*/
		
		int  n;
		
#if 	INPUT_DEV_DRIVE								/*输入子系统*/
		n = sizeof(struct input_event);
		if(read(button_fd, &ev_key, n) != n){
			LOG(LOG_ERROR, "read %s:%s",DEV_BUTTON_INPUT_PATH, strerror(errno));
			exit(-1);
		}

		if(EV_KEY ==ev_key.type && ev_key.code){
			switch(ev_key.code){
				case 256:	/*劫车警报*/
					if(ev_key.value == 1){
						/*上传报警信息和GPS  信息*/
						
						LOG(LOG_DEBUG, "劫车报警");
						send_data(socket_fd_ctd.socketfd, COMM_TYPE_ALARM, COMM_ALARM_CARJACKING, NULL, 0);
					}else{
						/*解除报警*/
						LOG(LOG_DEBUG, "解除劫车报警");
						send_data(socket_fd_ctd.socketfd, COMM_TYPE_ALARM, COMM_ALARM_CARJACKING_C, NULL, 0);
					}
					break;
				case 257:	/*胎压报警*/
					if(ev_key.value == 1){
						/*上传报警信息同时打开蜂鸣器10s*/						
						LOG(LOG_DEBUG,"胎压报警");
						send_data(socket_fd_ctd.socketfd, COMM_TYPE_ALARM, COMM_ALARM_TIREPRESURE, NULL, 0);
						ioctl(pwn_fd, PWM_IOCTL_SET_FREQ, 2);			/*控制蜂鸣器*/
					}else{
						/*解除胎压报警*/
						LOG(LOG_DEBUG, "解除胎压报警");
						send_data(socket_fd_ctd.socketfd, COMM_TYPE_ALARM, COMM_ALARM_TIREPRESURE_C, NULL, 0);
					}
					break;
				case 258:	/*越出区域报警*/
					if(ev_key.value == 1){
						/*上传报警和GPS  信息并不断闪烁LED1*/
						LOG(LOG_DEBUG, "越出区域报警");
						send_data(socket_fd_ctd.socketfd, COMM_TYPE_ALARM, COMM_ALARM_BEYONGD_REGION, NULL, 0);
						beyond_region_alarm();
					}else{
						/*解除警报*/
						LOG(LOG_DEBUG, "解除报警");
						send_data(socket_fd_ctd.socketfd, COMM_TYPE_ALARM, COMM_ALARM_BEYONGD_REGION_C, NULL, 0);
						led1_status = 0;		/*使闪烁函数结束*/
					}
					break;
			}
		}
		
#elif 	CHAR_DEV_DRIVE							/*字符驱动*/
		char buttons[6] = {'0', '0', '0', '0', '0', '0'};
		char cur_buttons[6];
		int i;
		
		n = sizeof(cur_buttons);
		if(read(button_fd, &ev_key, n) != n){
			LOG(LOG_ERROR, "read %s:%s",DEV_BUTTON_CHAR_PATH, strerror(errno));
			exit(-1);
		}
		n /= sizeof(cur_buttons[0]);
		for(i = 0; i < n; i++){
			if(buttons[i] != cur_buttons[i]){
				switch(i){
					case 0:/*劫车报警*/
						if(cur_buttons[0] != '0'){
							/*上传报警信息和GPS  信息*/
							LOG(LOG_DEBUG, "劫车报警");
							send_data(socket_fd_ctd.socketfd, COMM_TYPE_ALARM, COMM_ALARM_CARJACKING, NULL, 0);
						}else{
							/*解除报警*/
							LOG(LOG_DEBUG, "解除劫车报警");
							send_data(socket_fd_ctd.socketfd, COMM_TYPE_ALARM, COMM_ALARM_CARJACKING_C, NULL, 0);
						}
						break;
					case 1:/*胎压报警*/
						if(cur_buttons[1] != '0'){
							/*上传报警信息同时打开蜂鸣器10s*/
							LOG(LOG_DEBUG,"胎压报警");
							send_data(socket_fd_ctd.socketfd, COMM_TYPE_ALARM, COMM_ALARM_TIREPRESURE, NULL, 0);
							ioctl(pwn_fd, PWM_IOCTL_SET_FREQ, 2);
						}else{
							/*解除胎压报警*/
							LOG(LOG_DEBUG, "解除胎压报警");
							send_data(socket_fd_ctd.socketfd, COMM_TYPE_ALARM, COMM_ALARM_TIREPRESURE_C, NULL, 0);
						}
						break;
					case 2:/*越出区域报警*/
						if(cur_buttons[2] != '0'){
							/*上传报警和GPS  信息并不断闪烁LED1*/
							LOG(LOG_DEBUG, "越出区域报警");
							send_data(socket_fd_ctd.socketfd, COMM_TYPE_ALARM, COMM_ALARM_BEYONGD_REGION, NULL, 0);
							beyond_region_alarm();
						}else{
							/*解除警报*/
							
							LOG(LOG_DEBUG, "解除报警");
							send_data(socket_fd_ctd.socketfd, COMM_TYPE_ALARM, COMM_ALARM_BEYONGD_REGION_C, NULL, 0);
							led1_status = 0;
						}
						break;
					case 3:
						break;
					case 4:
						break;
					case 5:
						break;
				}
				buttons[i] = cur_buttons[i];
			}
		}
#endif
	}	

	Close(button_fd);
	Close(pwn_fd);
}

/**
 * @brief 			控制LED	灯的状态
 * @param which		哪个LED  灯
 * @param value		LED灯的状态
 */
void led_cfg_func(int which, int value){
	if(led_fd < 0){
		/*LED  灯未打开*/
		if(led_fd < 0)
			led_fd = open(DEV_LED_PATH, O_RDWR | O_NONBLOCK);
		if(led_fd < 0){			//打不开程序还是可以运行，只是LED  灯功能不能用
			LOG(LOG_ERROR, "LED无法正常打开:%s", strerror(errno));
			return ;
		}
	}
	switch(which){
		case 1:
			ioctl(led_fd, MINI2440_LED_CMD_0, &value);
			break;
		case 2:
			ioctl(led_fd, MINI2440_LED_CMD_1, &value);
			break;
		case 3:
			ioctl(led_fd, MINI2440_LED_CMD_2, &value);
			break;
		case 4:
			ioctl(led_fd, MINI2440_LED_CMD_3, &value);
			break;
	}
}


/**
 * @brief	闪烁LED 灯,灯1,2有效
 */
void *led_blink(void *param){
	int which;

	which = (int)param;
	if(which == 1){
		while(led1_status){
			led_cfg_func(which,0);
			sleep(1);
			led_cfg_func(which, 1);
			sleep(1);
		}
	}else{
		while(led2_status){
			led_cfg_func(which,0);
			sleep(1);
			led_cfg_func(which, 1);
			sleep(1);
		}
	}
	led_cfg_func(which, 1);
}

/**
 * @brief 	越出区域报警,不断闪烁LED1
 */
void beyond_region_alarm(){
	pthread_t pid_led1_blink;
	pthread_attr_t attr;												
	led1_status = 1;

	pthread_attr_init (&attr);
	pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
	Pthread_create(&pid_led1_blink, &attr, led_blink, (void *)1);
	pthread_attr_destroy (&attr);
}

/**
 * @brief		锁住车辆，不断闪烁LED2
 */
void lock_car(void){
	pthread_t pid_led2;
	pthread_attr_t attr;
	led2_status = 1;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	Pthread_create(&pid_led2, &attr, led_blink, (void *)2);
	pthread_attr_destroy(&attr);
}
