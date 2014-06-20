#include "socket.h"
#include "wrap.h"
#include "logc.h"
#include "alarm.h"
#include "messageHandler.h"
#include "circleQueue.h"
#include "fileo.h"

#include <string.h> 		/*strncpy*/
#include <stdlib.h>			/*exit*/
#include <errno.h>			/*errno*/
#include <stdio.h>			/*perror*/
#include <unistd.h>			/*read*/
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>


void client_init();
void client_uninit();
void client_connect_main(int *socketfd);
void *client_recv_main(void *param);

struct sockaddr_in servaddr;
volatile fd_set allset;
int led_fd = -1;
volatile int led1_status, led2_status;

int main(int argc, char *argv[]){	
	pthread_t pid_recv, pid_messgeHandler;
	pthread_t pid_alarm;
	
	client_init();
	client_connect_main(&socket_fd_ctd.socketfd);
	heart_beat_client(socket_fd_ctd.socketfd);     					/*启动心跳包*/
	client_gps_main();							   					/*启动GPS  数据包定时上传*/
	
#if IS_MINI2440
	
	Pthread_create(&pid_alarm, NULL, init_alarm_func, NULL);		/*创建线程监听各种警报信息*/
#endif
	Pthread_create(&pid_recv, NULL, client_recv_main, NULL);		/*创建线程接收信息*/
	Pthread_create(&pid_messgeHandler, NULL, messageHandler, NULL);

	Pthread_join(pid_recv, NULL);
	Pthread_join(pid_messgeHandler, NULL);
	
#if IS_MINI2440
	Pthread_join(pid_alarm, NULL);
#endif
	client_uninit();
	return 0;
}

/**
 * @brief	进行客户端一些初始化工作如文件描述符初始化等
 */
void client_init(){
	LOG(LOG_DEBUG, "client init...\n");
#if	IS_MINI2440
	if(led_fd < 0)
		led_fd = open(DEV_LED_PATH, O_RDWR | O_NONBLOCK);
	if(led_fd < 0){			//打不开程序还是可以运行，只是LED  灯功能不能用
		LOG(LOG_ERROR, "LED灯不正常:%s", strerror(errno));
	}
#endif
	strncpy(socket_fd_ctd.client_id, CLIEN_ID, CLIENT_ID_MAX);
	socket_fd_ctd.client_regisered = 0;
	socket_fd_ctd.heartbeat_cnt = 0;
	socket_fd_ctd.socketfd_status = -1;
	socket_fd_ctd.status = 0;
	Pthread_mutex_init(&(socket_fd_ctd.socketfd_mt), NULL);

	socket_fd_ctd.Q = Cqueue_init(CQUEUE_SIZE);			/*初始化消息队列*/
}

void client_uninit(){
	Pthread_mutex_destroy(&socket_fd_ctd.socketfd_mt);
	Close(socket_fd_ctd.socketfd);
	socket_fd_ctd.status = -1;

	DestroyQueue(socket_fd_ctd.Q);		/*销毁消息队列*/
	
#if	IS_MINI2440
	led_fd = -1;
#endif
}

void client_connect_main(int *socketfd){
	int n;
	char buf[5];
	
	if((*socketfd = c_socket(SERVE_PORT)) < 0){
		LOG(LOG_ERROR, "fail to create socket:%s",strerror(errno));
		exit(-1);
	}
	LOG(LOG_DEBUG, "send the client ID...\n");
	writen(*socketfd, socket_fd_ctd.client_id, CLIENT_ID_MAX);
	/*
		接收来自服务端的确认数据
	*/
	if((n = read(*socketfd, buf, 5)) < 0){			/*发生错误*/
		LOG(LOG_ERROR,"read error:%s", strerror(errno));
		exit(-1);
	}else{
		if(strncmp(buf, "busy", 5) == 0){					/*服务器繁忙*/
			
		}else if(strncmp(buf, "ok", 5) == 0){			/*注册成功*/
			LOG(LOG_DEBUG, "register success...\n");
		}else{											/*未注册*/
			printf("您的车牌号还未注册，不能使用这款服务!\n");
			client_uninit();
			exit(-1);									
		}
	}
}

void *client_recv_main(void *param){
	fd_set rset;
	int nready, socketfd, n;
	char data[DATA_MAX_SIZE];
	struct timeval rto;

	socketfd = socket_fd_ctd.socketfd;
	FD_ZERO(&allset);
	FD_SET(socketfd, &allset);
	
	for(; ;){
		rset = allset;
		rto.tv_sec = 5;
		rto.tv_usec = 0;

		if(socket_fd_ctd.status == -1){     //服务器断开连接，重新连接服务器
			
			client_connect_main(&socket_fd_ctd.socketfd);
			socketfd = socket_fd_ctd.socketfd;
			client_init();
			FD_SET(socketfd,&allset);
		}
		if((nready = select(socketfd+1, &rset, NULL, NULL, &rto)) < 0){
			if(errno == EINTR)
				continue;
			else{
				LOG(LOG_ERROR,"select error:%s", strerror(errno));
				exit(-1);
			}	
		}else if(nready == 0){						/*超时*/
			
		}else{
			if(FD_ISSET(socketfd, &rset)){
				bzero(data, DATA_MAX_SIZE);
				if((n = read(socketfd, data, DATA_MAX_SIZE)) == 0){
					/*服务器关闭了套接口，重新连接服务器*/
					LOG(LOG_DEBUG, "the server has been closed the sock,we will reconnect again...\n");
					printf("服务端断开连接，将会重新连接...\n");
					
					client_uninit();
					FD_ZERO(&allset);
				}else if(n < 0){		/*出现错误*/
					
				}else{
					int ret = 0;
					/*将消息存入消息队列*/
					while((ret = EnterQueue(socket_fd_ctd.Q, data, n)) == 0);		/**/
				}
			}
		}
	}
}