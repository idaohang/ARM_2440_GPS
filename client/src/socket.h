#ifndef __SOCKET_H__		/*对套接口处理的封装*/
#define __SOCKET_H__

#include "config.h"
#include "circleQueue.h"  /*struct Cqueue*/

#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


typedef struct sockaddr SA;

typedef struct {
	int socketfd;					/*已连接的套接字，-1表示未连接*/
	int socketfd_status;			/*表示心跳包是否丢失，0  表示运行正常，-1  表示异常*/
	pthread_mutex_t socketfd_mt;	/*操作此socket  的锁*/
	int heartbeat_cnt;				/*心跳包未成功次数*/
	int client_regisered;			/*此客户端ID  是否成功注册*/
	volatile int status;						/*连接状态，0  表示运行正常，-1  表示异常*/
	char client_id[CLIENT_ID_MAX];	/*客户端ID*/
	Cqueue *Q;
}socket_fd_conn;
socket_fd_conn socket_fd_ctd;

extern ssize_t	writen(int fd, const void *vptr, size_t n);
extern ssize_t	readn(int fd, void *vptr, size_t n);
extern int c_socket(int port);
extern void heart_beat_client(int socketfd);
extern int connect_retry(int sockfd, const struct sockaddr *addr, socklen_t alen);
extern void client_gps_main();
extern int send_data(int fd, int commtype, int comm, unsigned char *data, int len);

#endif /* __SOCKET_H__ */
