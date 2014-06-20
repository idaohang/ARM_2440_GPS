#ifndef __WRAP_H__		/*一些常用包裹函数，即封装了错误处理的函数*/
#define __WRAP_H__

#include <stdio.h>
#include <stdlib.h>

extern void *Malloc(size_t size);
extern void Close(int fd);
extern void Pthread_mutex_init(pthread_mutex_t *mptr, pthread_mutexattr_t *attr);
extern void Pthread_mutex_destroy(pthread_mutex_t *mptr);
extern void Pthread_mutex_lock(pthread_mutex_t *mptr);
extern void Pthread_mutex_unlock(pthread_mutex_t *mptr);
extern void Pthread_create(pthread_t *tid, const pthread_attr_t *attr,void * (*func)(void *), void *arg);
void Pthread_join(pthread_t tid, void **status);

#endif 	/* __WRAP_H__ */
