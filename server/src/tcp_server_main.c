#include "socket.h"
#include "my_pthread.h"
#include "wrap.h"
#include "logc.h"
#include "messageHandler.h"
#include "menu.h"

#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/select.h>    /*fd_set*/
#include <unistd.h>
#include <time.h>			/*struct timeval*/


pthread_mutex_t	mlock = PTHREAD_MUTEX_INITIALIZER;     /*使监听套接字线程池同步*/
volatile fd_set allset;
volatile int maxfd, maxi;   /*maxfd、最大文件描述符，maxi client数组最大下标*/

void server_init();
void server_uninit();
void server_accept_main(int socketfd);
void *server_recv_main(void *param);


int main(int argc, char **argv){
	int socketfd;
	int i;
	pthread_t server_recv_tid, message_handler_tid, menu_tid;
	
	server_init();
	if((socketfd = s_socket(SERVE_PORT)) < 0){
		exit(-1);
	}
	server_accept_main(socketfd);
	Pthread_create(&server_recv_tid, NULL, server_recv_main, NULL);
	Pthread_create(&message_handler_tid, NULL, messageHandler, NULL);
	heart_beat_server();
	Pthread_create(&menu_tid, NULL, showmenu, NULL);

	for(i = 0; i < THREAD_ACCEPT_MAX; i++)
		Pthread_join(tptr[i].thread_tid, NULL);
	Pthread_join(server_recv_tid, NULL);
	Pthread_join(message_handler_tid, NULL);
	Pthread_join(menu_tid, NULL);

	server_uninit();
	return 0;
}

/**
 * @brief	进行服务端一些初始化工作如文件描述符初始化等
 */
void server_init(){
	int i;
	
#if DEBUG
	printf("server init..\n");
	LOGN("server", LOG_DEBUG, "server init...");
#endif
	FD_ZERO(&allset);
	maxi = -1;
	maxfd = 0;
	for(i = 0; i < CONN_NUM_MAX; i++){
		client[i].socketfd = -1;
		client[i].socketfd_status = -1;	/*在初次定时器触发间隔内没有改变的话，就是没有收到心跳包*/
		client[i].heart_beat_cnt = 0;
		Pthread_mutex_init(&client[i].socketfd_mt, NULL);
		client[i].Q = NULL;
	}
}

void server_uninit(){
	int i;

	for(i = 0; i < CONN_NUM_MAX; i++){
		Pthread_mutex_destroy(&client[i].socketfd_mt);
		DestroyQueue(client[i].Q);
	}
}

/**
 * @brief 	创建监听线程池
 */
void server_accept_main(int socketfd){
	int i ;

	tptr = Malloc(THREAD_ACCEPT_MAX* sizeof(Thread));
	
	for(i = 0; i < ACCEPT_THREAD_MAX; i++){
		thread_make(i, socketfd);
	}
}

/**
 * @brief 	使用select  实时监测已准备好读的套接字描述符，从中接收信息
 			并分析相应的命令类型进行不同的处理
 */
void *server_recv_main(void *param){
	fd_set rset;
	int nready;		/*存储select  返回值的信息*/
	int socketfd, i, n;
	unsigned char data[DATA_MAX_SIZE];
	struct timeval rto;

	for(; ;){
		rset = allset;
		rto.tv_sec = 5;
		rto.tv_usec = 0;
		if((nready = select(maxfd+1, &rset, NULL, NULL, &rto)) < 0 ){
			if(errno == EINTR)
				continue;					/*被中断的话继续*/
			else{
				perror("select error");
				exit(-1);
			}
		}else if(nready == 0){					/*超时*/
				continue;
		}else{	
			for(i = maxi; i >= 0; i--){
				if((socketfd = client[i].socketfd) < 0)
					continue;

				if(FD_ISSET(socketfd, &rset)){			/*准备好读*/
					bzero(data, DATA_MAX_SIZE);		

					if((n = read(socketfd, data, DATA_MAX_SIZE)) == 0){		/*connection closed by client, recv FIN*/
						/*关闭套接字，从监听读字符集中移除
							同时,重新初始化其所占用单元
						*/
						printf("\n客户%s 关闭连接!\n", client[i].client_id);
						LOGN("server", LOG_DEBUG, "the client %s closed!",client[i].client_id);
						
						Pthread_mutex_lock(&client[i].socketfd_mt);
						Close(socketfd);
						FD_CLR(socketfd, &allset);
						client[i].socketfd = -1;
						client[i].heart_beat_cnt = 0;
						client[i].socketfd_status = -1;
						bzero(client[i].client_id,CLIENT_ID_MAX);	
						DestroyQueue(client[i].Q);
						if(maxi == i){
							int k = i;
							maxi--;
							while(client[--k].socketfd < 0)
								maxi--;
						}
						Pthread_mutex_unlock(&client[i].socketfd_mt);							
					}else if(n < 0){					/*出现错误*/
						
					}else{								/*有数据*/
						/*将接收到的数据存于响应的循环队列中*/
						int ret;
						
						while((ret = EnterQueue(client[i].Q, data, n)) == 0);						
					}
					if(--nready <= 0)/*之前不小心将这句话放到了if(FD_ISSET)之外，客户端大于3个就运行不正常了*/
						break;							/*没有可读的描述符*/
				}
			}
		}
	}     
}

