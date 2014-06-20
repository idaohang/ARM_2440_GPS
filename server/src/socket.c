#include "socket.h"
#include "wrap.h"
#include "mytools.h"
#include "logc.h"
#include "prot.h"

#include <errno.h>		/*errno*/
#include <unistd.h>		/*close*/
#include <string.h>		/*bzero*/
#include <signal.h>		/*SIGALRM*/

extern fd_set allset;
extern int maxi;


/**
 * @brief 		服务端套接字初始化程序
 * @param type	套接口类型
 * @param addr	指向套接口地址结构的指针
 * @param alen	套接口地址结构的大小
 * @param qlen	同listen  的第二个参数
 * @return 		初始化后的套接字描述符
 */
int init_server_socket(int type, const struct sockaddr *addr, socklen_t alen, int qlen){
	int socketfd;
	int err = 0;
	int opt;

	opt = 1;
	
	LOGN("server", LOG_DEBUG, "create sokcetfd");

	if((socketfd = socket(addr->sa_family, type, 0)) < 0)
		return -1;
	/*避开TIME_WAIT  状态*/
	if(setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
		err = errno;
		goto errout;
	}

	if(setsockopt(socketfd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt)) < 0){
		err = errno;
		goto errout;
	}
	if(bind(socketfd, addr, alen) < 0){
		err = errno;
		goto errout;
	}
	if(type == SOCK_STREAM || type == SOCK_SEQPACKET){
		if(listen(socketfd, qlen) < 0){
			err = errno;
			goto errout;
		}
	}
	return socketfd;

errout:
	close(socketfd);
	errno = err;
	return -1;
}

/**
 * @brief		服务主机地址的初始化，并调用init_server_socket进行进一步的绑定等初始化工作
 * @param port	服务器要监听的端口号
 * @return 		套接字描述符
 */
int s_socket(int port){
	int listenfd;
	struct sockaddr_in servaddr;
	socklen_t addr_len;

	addr_len = sizeof(struct sockaddr_in);
	bzero(&servaddr, addr_len);
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);

	listenfd = init_server_socket(SOCK_STREAM, (SA *)&servaddr, addr_len, 20);

	return listenfd;
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
void signal_handler(union sigval v){
	int i;
	
	/*可能尚未有客户端连接过来，此时应先判断maxi  的值*/
	if(maxi >= 0){		
		for(i = maxi; i >= 0; i--){
			if(client[i].socketfd < 0)
				continue;
			/*添加互斥锁，保持数据的一致性，防止中途数据在其他地方更改，
			   造成数据无效*/
			Pthread_mutex_lock(&client[i].socketfd_mt);			
			if(client[i].heart_beat_cnt >= HEART_MAX_CNT){
				/*心跳包未成功次数超过给定界限*/
				printf("客户%s长时间未回复，现断开其连接...\n", client[i].client_id);
				LOGN("server", LOG_NOTICE,"heartbeat out %s", client[i].client_id);
				
				Close(client[i].socketfd);
				FD_CLR(client[i].socketfd, &allset);
				client[i].heart_beat_cnt = 0;
				client[i].socketfd = -1;
				client[i].socketfd_status = -1;
				bzero(client[i].client_id,CLIENT_ID_MAX);
				DestroyQueue(client[i].Q);		/*销毁循环队列，即循环队列只在连接建立的时候分配内存空间*/
				if(maxi == i){
					int k = i;
					maxi--;
					while(client[--k].socketfd < 0)
						maxi--;
				}
				Pthread_mutex_unlock(&client[i].socketfd_mt);
				continue;
				
				/*应该关闭客户端所占用的资源*/
			}
			if(client[i].socketfd_status == -1){
				/*连接异常，即未收到心跳包*/

				LOGN("server", LOG_DEBUG, "client %s not recv the heartbeat+1", i, client[i].client_id);

				client[i].heart_beat_cnt++;
			}else{
				LOGN("server", LOG_DEBUG, "the client %s is working", client[i].client_id);
				/*客户端保持存活*/
				client[i].socketfd_status = -1;
				client[i].heart_beat_cnt = 0;
			}
			Pthread_mutex_unlock(&client[i].socketfd_mt);
			
			send_data(client[i].socketfd, COMM_TYPE_HEARBEAT, 0, NULL, 0);
			
			LOGN("server", LOG_DEBUG, "write  client %s MSG_HEART_BEAT", i,client[i].client_id);
		}
	}
	
}


void heart_beat_server(){
	LOGN("server", LOG_DEBUG, "start heart_beat...");

	set_timer(HEART_BEAT_TIME, 0, signal_handler, 100);		//启动定时器
}

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

