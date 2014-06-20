#include "socket.h"
#include "config.h"
#include "mytools.h"	/*set_timer*/
#include "logc.h"
#include "prot.h"

#include <unistd.h> 	/*sleep*/
#include <sys/socket.h>		
#include <netinet/in.h>
#include <arpa/inet.h>	/*inet_addr*/
#include <stdio.h>		/*perror*/
#include <errno.h>		/*errno*/
#include <stdlib.h>		/*exit*/
#include <signal.h>
#include <string.h>		/*strerror*/

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

extern void client_uninit();

extern struct sockaddr_in servaddr;
extern fd_set allset;

int connect_retry(int sockfd, const struct sockaddr *addr, socklen_t alen){
	int nsec;

	for(nsec = 1; nsec <= MAXSLEEP; nsec <<= 1){
		if(connect(sockfd, addr, alen) == 0)
			return 0;
		if(nsec <= MAXSLEEP/2)
			sleep(nsec);
	}
	
	return -1;
}

/**
 * @brief 	客户端套接口的初始化包括与服务器的超时连接
 * 			包括创建套接字，绑定连接服务器
 */
int c_socket(int port){

	int sockfd;
	socklen_t len;
#if DEBUG
	LOG(LOG_DEBUG, "create socket...\n");
#endif
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		return -1;
	}
	servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    servaddr.sin_port = htons(SERVE_PORT);
    len = sizeof(servaddr);
	LOG(LOG_DEBUG, "connecting...\n");
again:
	if(connect_retry(sockfd, (struct sockaddr *)&servaddr, len) < 0){
		LOG(LOG_ERROR, "connect error:%s", strerror(errno));
		LOG(LOG_DEBUG, "we will connect 3 minutes after again!\n");
		printf("we will connect 3 minutes after again!\n");
		sleep(180);
		goto again;
	}
    return sockfd;
}


/**
 * @brief 		确保从一个文件描述符中读取n  个字节内容
 */
ssize_t	readn(int fd, void *vptr, size_t n){					
	size_t	nleft;
	ssize_t	nread;
	char	*ptr;
	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nread = read(fd, ptr, nleft)) < 0) {
			if (errno == EINTR)				
				nread = 0;		/* and call read() again */			
			else				
				return(-1);		
		} else if (nread == 0)
		break;				/* EOF */		
		nleft -= nread;		
		ptr   += nread;	
	}	
	return(n - nleft);		/* return >= 0 */
}/* end readn */

/**
 * @brief 			确保写n  个字节到描述符中
 */
ssize_t	writen(int fd, const void *vptr, size_t n){	
	size_t		nleft;	
	ssize_t		nwritten;	
	const char	*ptr;	
	ptr = vptr;	nleft = n;	
	while (nleft > 0) {		
		if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
			if (nwritten < 0 && errno == EINTR)	
				nwritten = 0;		/* and call write() again */			
			else				
				return(-1);			/* error */		
		}		
		nleft -= nwritten;		
		ptr   += nwritten;	
	}
	return(n);
}/* end writen */

/*
 * @brief	自定义的信号超时处理程序
 			当心跳包
 */
void heartbeat_handler(union sigval v){
	if(socket_fd_ctd.status == 0){
		if(socket_fd_ctd.heartbeat_cnt >= HEART_MAX_CNT){
			/*心跳包未成功次数超过给定界限*/
			LOG(LOG_DEBUG,"heartbeat out\n");
			printf("hearbeat out again, we will connect again\n");
			
			client_uninit();
			FD_ZERO(&allset);
			return ;
		}
		if(socket_fd_ctd.socketfd_status == -1){		/*连接异常，即心跳包未得到响应*/
			LOG(LOG_DEBUG, "the server not respons +1\n");
			socket_fd_ctd.heartbeat_cnt++;
		}else{	
			LOG(LOG_DEBUG, "the server is working\n");
			socket_fd_ctd.socketfd_status = -1;			/*下次接收到应答的时候置0*/
			socket_fd_ctd.heartbeat_cnt = 0;			/*心跳包未成功次数回置*/
		}
		//send_heart_beat(socket_fd_ctd.socketfd);
		send_data(socket_fd_ctd.socketfd, COMM_TYPE_HEARBEAT, 0, NULL, 0);
	}
}

/*int send_heart_beat(int fd){
	char hearbeat_data[9];
	int len;
	
	len = packet(hearbeat_data, NULL, 0,COMM_TYPE_HEARBEAT, 0);
	if(len > 0)
		writen(fd, hearbeat_data, len);
}*/

/*
 * @brief 	传送数据
 * @return	成功返回发送的字节数，否则为0
 */
int send_data(int fd, int commtype, int comm, unsigned char *data, int len){
	int length;
	unsigned char *s_data;
		
	if(data == NULL)
		s_data = (unsigned char *)malloc(sizeof(unsigned char) * 9);
			
	else
		s_data = (unsigned char *)malloc(sizeof(unsigned char) * 2 *len);
	
	length = packet(s_data, (len == 0 ? NULL:data), len, commtype, comm);
	if(length > 0)
		writen(fd, s_data, length);
	
	free(s_data);
	return len;
}



/**
 * @brief		客户端心跳包函数启动定时器定时向服务端发送心跳包并处理异常情况
 */
void heart_beat_client(int socketfd){
	send_data(socketfd, COMM_TYPE_HEARBEAT, 0, NULL, 0);
	LOG(LOG_DEBUG, "start the heartbeat...\n");
	set_timer(HEART_BEAT_TIME, 0, heartbeat_handler, 100);	//启动定时器
}

/*
 * @brief 	传输GPS  数据包
 */
void transmit_gps(union sigval v){
	if(socket_fd_ctd.status == 0){	
		LOG(LOG_DEBUG, "send gps data...\n");
		send_data(socket_fd_ctd.socketfd, COMM_TYPE_SERVE, COMM_SERV_GPS,NULL, 0);
	}
}

/*
 * @brief	启动定时器定时发送GPS  数据包
 */
void client_gps_main(){
	LOG(LOG_DEBUG,"start transmit gps data func\n");
	set_timer(GPS_UPLOAD_TIME, 0, transmit_gps, 101);
	
}

