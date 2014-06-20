#include "messageHandler.h"
#include "socket.h"
#include "logc.h"
#include "alarm.h"
#include "prot.h"
#include "fileo.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern int led2_status;

int get_message_from_lq(Cqueue *Q,unsigned char *desc);

void *messageHandler(void *param){

	/*循环处理队列*/
	while(1){
		/*开始的时候这边没有等号，结果死活通不过。。*/
			while(socket_fd_ctd.status != -1 && !QueueEmpty(socket_fd_ctd.Q)){				//连接建立并且队列不为空的情况下

				/*处理队列中的信息*/
				unsigned char m_data[1024];
				int len;

				memset(m_data, 0x00, 1024);
				len = get_message_from_lq(socket_fd_ctd.Q, m_data);
				if(len == 0)
					continue;

				int comm_type = m_data[0];
				int comm ;
				switch(comm_type){
					case COMM_TYPE_HEARBEAT:
						socket_fd_ctd.socketfd_status = 0;
						break;
						
					case COMM_TYPE_SERVE:
						comm = m_data[1];
						
						if(comm == COMM_SERV_GPS){
							/*上传GPS 信息*/
							LOG(LOG_DEBUG, "服务端要求上传GPS数据信息");
#if IS_MINI2440
						}else if(comm == COMM_SERV_LOCK){
							/*锁车*/
							lock_car();
							LOG(LOG_DEBUG, "服务端下达锁车指令，锁车...");
						}else if(comm == COMM_SERV_UNLOCK){
							/*解锁*/
							led2_status = 0;
							LOG(LOG_DEBUG, "服务端下达解锁指令，已解锁...");
#endif							
						}else if(comm ==  COMM_SERV_BITMAP){
							/*上传图片*/
							static int count = 1;
							unsigned char file_name[6];
							snprintf((char *)file_name,6,"%d.jpg", count++);
							send_file((char *)file_name, socket_fd_ctd.socketfd);
							LOG(LOG_DEBUG, "服务端要求上传图片");
						}
						break;
					default:
						printf("无法识别的指令\n");
						break;
				}
			}
	}
}

/*
 * @brief	从循环队列中取出一段完整的数据包
 */
int get_message_from_lq(Cqueue *Q, unsigned char *desc){
	char temp;
	int count = 0;
	int len = 0, n;;
	
	while(Q->count && Q->base[Q->front] != HEAD){		//取得包头
		Q->front = (Q->front + 1) % Q->maxsize; 			//不是包头的话就丢弃
		Q->count--;
	}
		
	count = 1;
	while(Q->count && (count + 1 <= Q->count) && (temp = Q->base[(Q->front + count)%Q->maxsize]) != END){				//取得包尾
		if(temp == HEAD){
			Q->front = (Q->front + count) % Q->maxsize;
			Q->count -= count;		
			count = 1;
			continue;
		}else
			count++;
	}
	if(count + 1 <= Q->count){		//队列有完整的数据包完整
		if((n = (Q->maxsize - Q->front)) >= (count + 1)){
			len = unpacket(desc,&Q->base[Q->front], count+1); //这边也必须谨防数据回到头部
		}
		else{
			unsigned char *temp_data;		/*存放循环队列中的一段数据*/
			temp_data = (unsigned char *)malloc(sizeof(unsigned char) * (count+1));
	
			memcpy(temp_data, Q->base + Q->front, n);
			memcpy(temp_data + n, Q->base, count + 1 - n);
			len = unpacket(desc, temp_data, count+1);
			free(temp_data);
		}
		PopupnQueue(Q ,count + 1);
	}
	
	return len;
}


