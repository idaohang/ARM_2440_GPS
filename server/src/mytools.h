#ifndef __MY_TOOLS_H__
#define __MY_TOOLS_H__

#include <signal.h> /*union sigval*/

typedef void Sigfunc(int);
typedef void Timerfunc(union sigval v);

extern Sigfunc *signal(int signo, Sigfunc *func);
extern void set_timer(long sec, long usec, Timerfunc *func, int sigval_int);

#endif /* __MY_TOOLS_H__ */
