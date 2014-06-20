#ifndef __SOCKET_H__		/*对套接口处理的封装*/
#define __SOCKET_H__

#include "config.h"
#include "circleQueue.h"

#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct sockaddr SA;

typedef struct {
	//int is_connected;     			/*该client_id是否已经与服务端建立连接，1 表示已连接，0 表示未建立*/
	volatile int socketfd;					/*连接过来的套接字描述符*/
	int socketfd_status;			/*连接状态，0 表示运行正常，-1 表示异常*/
	int heart_beat_cnt;				/*心跳包未成功次数*/
	pthread_mutex_t socketfd_mt;	/*操作此socket 的锁*/
	char client_id[CLIENT_ID_MAX];	/*客户端ID*/
	Cqueue *Q;						/*每个客户端分配一个循环队列处理信息*/
}socket_fd_conn;
socket_fd_conn client[CONN_NUM_MAX];	/*存储客户端连接过来的信息*/

extern int s_socket(int port);
extern ssize_t	readn(int fd, void *vptr, size_t n);
extern ssize_t	writen(int fd, const void *vptr, size_t n);
extern void heart_beat_server();
extern int send_data(int fd, int commtype, int comm,unsigned char *data, int len);

#endif /* __SOCKET_H__ */
