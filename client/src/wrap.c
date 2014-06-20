#include "wrap.h"
#include "logc.h"

#include <errno.h>
#include <unistd.h>			/*close*/
#include <pthread.h>		/*pthread*/
#include <string.h>			/*strerror*/

void *Malloc(size_t size){
	void *ptr;

	if((ptr = malloc(size)) == NULL){
		LOG(LOG_ERROR, "malloc error:%s", strerror(errno));
		exit(-1);
	}
	return ptr;
}
void Close(int fd){	
	if (close(fd) == -1){
		LOG(LOG_ERROR, "close error:%s" ,strerror(errno));
		exit(-1);
	}
}

void Pthread_mutex_init(pthread_mutex_t *mptr, pthread_mutexattr_t *attr){	
	int		n;
	if ( (n = pthread_mutex_init(mptr, attr)) == 0)		
		return;
	errno = n;
	LOG(LOG_ERROR, "pthread_mutex_init error:%s", strerror(errno));
	exit(-1);
}

void Pthread_mutex_destroy(pthread_mutex_t *mptr){
	int n;
	if((n = pthread_mutex_destroy(mptr)) == 0)
		return;
	errno = n;
	LOG(LOG_ERROR, "pthread_mutex_init error:%s", strerror(errno));
	exit(-1);
}

void Pthread_mutex_lock(pthread_mutex_t *mptr){	
	int		n;
	if ( (n = pthread_mutex_lock(mptr)) == 0)		
		return;
	errno = n;
	LOG(LOG_ERROR, "pthread_mutex_lock error:%s", strerror(errno));
	exit(-1);
}


void Pthread_mutex_unlock(pthread_mutex_t *mptr){	
	int		n;
	if ( (n = pthread_mutex_unlock(mptr)) == 0)		
		return;
	errno = n;
	LOG(LOG_ERROR, "pthread_mutex_unlock error:%s", strerror(errno));
	exit(-1);
}

void Pthread_create(pthread_t *tid, const pthread_attr_t *attr,void * (*func)(void *), void *arg){
	int		n;	
	if ( (n = pthread_create(tid, attr, func, arg)) == 0)	
		return;
	errno = n;	
	LOG(LOG_ERROR, "pthread_create error:%s",strerror(errno));
	exit(-1);
}

void Pthread_join(pthread_t tid, void **status){
	int		n;	
	
	if ((n = pthread_join(tid, status)) == 0)	
		return;	errno = n;
	LOG(LOG_ERROR, "pthread_join error:%s", strerror(errno));
	exit(-1);
}

