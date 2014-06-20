#ifndef __MY_PTHREAD_H__     /*自定义线程处理函数*/
#define __MY_PTHREAD_H__

#include "socket.h" 	/*socket_fd_conn*/

#include <pthread.h>

typedef struct{
	pthread_t thread_tid; 	/*thread ID*/
}Thread;

Thread *tptr;				/*线程池*/
extern pthread_mutex_t mlock;

extern int thread_make(int index, int listenfd);

#endif /* __MY_PTHREAD_H__ */
