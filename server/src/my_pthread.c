#include "my_pthread.h"
#include "wrap.h"
#include "types.h"
#include "db_controller.h"
#include "socket.h"   	/*readn()*/
#include "wrap.h"		/*Malloc()*/
#include "logc.h"

#include <errno.h>
#include <unistd.h>		/*close(),*/
#include <string.h>		/*strncpy(), bzero()*/

extern fd_set allset;
extern int maxi,maxfd;

void read_client_id(int listenfd, char *buf, int len,int index){
	ssize_t nread;
	if((nread = readn(listenfd, buf, len)) < 0){		/*read error*/
		LOGN("server", LOG_ERROR, "read error,%s",strerror(errno));
		exit(-1);
	} 
#if DEBUG
	LOGN("server", LOG_DEBUG, "pthread %d read %s", index, buf);	
#endif
	return ;

}

/**
 * @brief 	我们的工作线程所要做的主要工作
 */
void *thread_main(void *arg){
	int connfd, listenfd, index;
	socklen_t clilen, addrlen;
	struct sockaddr cliaddr;
	char client_id[CLIENT_ID_MAX];

	index = LOWORD((int)arg);
	listenfd = HIWORD((int)arg);
	addrlen = sizeof(struct sockaddr);
#if DEBUG
	LOGN("server", LOG_DEBUG, "thread %d starting listening %d", index, listenfd);
#endif	
	for(; ;){
		clilen = addrlen;
		Pthread_mutex_lock(&mlock);
#if DEBUG		
		LOGN("server", LOG_DEBUG, "pthread %d accepting...", index);
#endif
again:
		if((connfd = accept(listenfd, &cliaddr, &clilen)) < 0){		/*accept  出错*/
#ifdef EPROTO   		/*协议出错*/
			if(errno == EPROTO || errno == ECONNABORTED || errno == EINTR)
#else 					/*连接被终止*/
			if(errno == ECONNABORTED || errno == EINTR)
#endif
				goto again;
			else{
				LOGN("server", LOG_ERROR, "accept error");
				exit(-1);
			}	
		}
		
		Pthread_mutex_unlock(&mlock);

		/*判断客户终端的ID  是否合法，如果合法则将connfd  加入TCP  连接*/
		read_client_id(connfd, client_id, CLIENT_ID_MAX, index);
		if(false == check_client_id(client_id)){		/*不是合法ID*/
			writen(connfd, "no", 3);
			Close(connfd);
		}else{											/*合法ID*/
			int i;
			for(i = 0; i < CONN_NUM_MAX; i++){			/*将其存入已连接套接字数组*/
				if(client[i].socketfd < 0){				/*这里是初次将其添加进来，所以 理论上不用和其他地方互斥*/
					client[i].socketfd = connfd;
					break;
				}
			}
			if(i == CONN_NUM_MAX){						/*超过服务器所能够承载的数目，告诉客户端稍后再试*/
				LOGN("server", LOG_DEBUG, "too many clients");
				client[i].socketfd = -1;
				Close(connfd);
				continue;
			}

			client[i].socketfd_status = 0;
			bzero(client[i].client_id, CLIENT_ID_MAX);
			strncpy(client[i].client_id, client_id, CLIENT_ID_MAX);
			FD_SET(connfd, &allset);					/*将其添加进select监听的套接字*/
			client[i].Q = Cqueue_init(CQUEUE_SIZE);		/*初始化循环队列*/
			writen(connfd, "ok", 3);
			
			if(connfd > maxfd)
				maxfd = connfd;
			if(i > maxi)
				maxi = i;
		}
	}
	
}

/**
 * @brief 			创建我们的工作线程，监听服务套接字
 * @param index 		表示是监听线程池中的第几个线程
 * @param listenfd 	要监听的套接字描述符
 */
int thread_make(int index, int listenfd){
	int n;

	if((n = pthread_create(&(tptr[index].thread_tid), NULL, &thread_main, (void *)(MAKEWORD(index, listenfd)))) == 0 ){
		return 0;
	}
	errno = n;				/*创建线程失败*/
#if DEBUG
	LOGN("server", LOG_DEBUG, "pthread_create %d error", index);
#endif
	return -1;			/*一个线程出错，其他线程可以继续产生，所以不直接退出*/
}

