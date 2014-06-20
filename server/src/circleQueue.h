#ifndef __CIRCLEQUEUE_H__		/*与循环队列有关*/
#define __CIRCLEQUEUE_H__

typedef struct {
	volatile int front;				/*队列头游标*/
	volatile int rear;				/*队列尾游标*/
	int maxsize;			/*队列的长度*/
	volatile int count;				/*指示当前队列之中的元素个数*/
	unsigned char *base;	/*队列的头指针*/
}Cqueue;

Cqueue* Cqueue_init(int size);
int QueueEmpty(Cqueue *Q);
int QueueFull(Cqueue *Q, int n);
int EnterQueue(Cqueue *Q, unsigned char *items, int  n;);
unsigned char  PopupQueue(Cqueue *Q);
int PopupnQueue(Cqueue *Q,int n);
void DestroyQueue(Cqueue *Q);

#endif /* __CIRCLEQUEUE_H__ */
