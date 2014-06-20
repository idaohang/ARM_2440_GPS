#include "wrap.h"
#include "logc.h"

#include <errno.h>
#include <unistd.h>			/*close*/
#include <pthread.h>		/*pthread*/

void *Malloc(size_t size){
	void *ptr;

	if((ptr = malloc(size)) == NULL){
		perror("malloc error");
		exit(-1);
	}
	return ptr;
}
void Close(int fd){	
	if (close(fd) == -1){
		perror("close error");
		LOGN("server", LOG_ERROR, "close error");
		exit(-1);
	}
}

void Pthread_mutex_init(pthread_mutex_t *mptr, pthread_mutexattr_t *attr){	
	int		n;
	if ( (n = pthread_mutex_init(mptr, attr)) == 0)		
		return;
	errno = n;
	perror("pthread_mutex_init error");
	exit(-1);
}

void Pthread_mutex_destroy(pthread_mutex_t *mptr){
	int n;
	if((n = pthread_mutex_destroy(mptr)) == 0)
		return;
	errno = n;
	perror("pthread_mutex_init error");
	exit(-1);
}

void Pthread_mutex_lock(pthread_mutex_t *mptr){	
	int		n;
	if ( (n = pthread_mutex_lock(mptr)) == 0)		
		return;
	errno = n;
	perror("pthread_mutex_lock error");
	exit(-1);
}


void Pthread_mutex_unlock(pthread_mutex_t *mptr){	
	int		n;
	if ( (n = pthread_mutex_unlock(mptr)) == 0)		
		return;
	errno = n;
	perror("pthread_mutex_unlock error");
	exit(-1);
}

void Pthread_create(pthread_t *tid, const pthread_attr_t *attr,void * (*func)(void *), void *arg){
	int		n;	
	if ( (n = pthread_create(tid, attr, func, arg)) == 0)	
		return;
	errno = n;	
	perror("pthread_create error");
	exit(-1);
}

void Pthread_join(pthread_t tid, void **status){
	int		n;	
	
	if ((n = pthread_join(tid, status)) == 0)	
		return;	errno = n;
	perror("pthread_join error");
	exit(-1);
}

