#include "messageHandler.h"
#include "socket.h"
#include "logc.h"
#include "prot.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


extern int maxi;

int get_message_from_lq(Cqueue *Q, unsigned char *desc);

void *messageHandler(void *param){
	int i;

	/*循环处理队列*/
	while(1){
		/*开始的时候这边没有等号，结果死活通不过。(没有往下运行)*/
		for(i = 0; i <= maxi; i++){		/*只处理已连接的*/
			if(client[i].socketfd < 0)
				continue;
			while(!QueueEmpty(client[i].Q)){
				/*处理队列中的信息*/
				unsigned char m_data[BUFFER_SIZE + NAME_MAX_SIZE + 4];
				int len;
				
				memset(m_data, 0x00, 1024);
				len = get_message_from_lq(client[i].Q, m_data);
				
				if(len == 0)
					continue;
				
				int comm_type = m_data[0];
				int comm ;

				switch(comm_type){
					case COMM_TYPE_ALARM:
						comm = m_data[1];

						if(comm == COMM_ALARM_BEYONGD_REGION){
							/*越出区域报警*/
							printf("客户%s越出了指定区域!", client[i].client_id);
							LOGN("server", LOG_DEBUG,"客户%s越出了指定区域!", client[i].client_id);
						}else if(comm == COMM_ALARM_CARJACKING){
							/*劫车报警*/
							printf("客户%s被劫车了!", client[i].client_id);
							LOGN("server", LOG_DEBUG,"客户%s被劫车了!", client[i].client_id);
						}else if(comm == COMM_ALARM_TIREPRESURE){
							/*胎压报警*/
							printf("客户%s胎压不正常!", client[i].client_id);
							LOGN("server", LOG_DEBUG,"客户%s胎压不正常!", client[i].client_id);
						}else if(comm == COMM_ALARM_BEYONGD_REGION_C){
							/*解除越出区域报警*/
							printf("客户%s已返回指定区域!", client[i].client_id);
							LOGN("server", LOG_DEBUG,"客户%s已返回指定区域!", client[i].client_id);
						}else if(comm == COMM_ALARM_CARJACKING_C){
							/*解除劫车报警*/
							printf("客户%s解除了劫车报警!", client[i].client_id);
							LOGN("server", LOG_DEBUG,"客户%s解除了劫车报警!", client[i].client_id);
						}else if(comm == COMM_ALARM_TIREPRESURE_C){
							/*解除胎压报警*/
							printf("客户%s胎压已恢复正常!", client[i].client_id);
							LOGN("server", LOG_DEBUG,"客户%s胎压已恢复正常!", client[i].client_id);
						}
						break;
					case COMM_TYPE_HEARBEAT:
						client[i].socketfd_status = 0;
						break;
					case COMM_TYPE_SERVE:
						comm = m_data[1];
						
						if(comm == COMM_SERV_GPS){
							//printf("收到GPS包\n");
							
						}else if(comm == COMM_SERV_BITMAP){
							/*收到图片*/
							//int length = *(int *)(m_data + 2);
							int fd;
							unsigned char *file_name;
							int file_name_len;

							file_name_len = *(int *)(m_data + FILE_NAME_LEN_START);
							file_name = (unsigned char *)malloc(sizeof(unsigned char) * file_name_len);
							memcpy(file_name, m_data + FILE_NAME_START, file_name_len);
							fd = open((char *)file_name, O_CREAT | O_RDWR | O_APPEND, 0777);
							if(fd < 0){
								perror("can't open 1.jpg");
								continue;
							}
								
							writen(fd, m_data + FILE_NAME_START + file_name_len, len - (FILE_NAME_START + file_name_len ));
							free(file_name);
							close(fd);
						}
						break;
					default:
						printf("不知道这是什么东西\n");
						break;
				}
			}
		}
	}
}

/*
 * @brief	从循环队列中取出一段完整的数据包
 * @return	取出的数据的长度(解包之后)
 */
int get_message_from_lq(Cqueue *Q, unsigned char *desc){
	char temp;
	int count = 0;
	int len = 0, n;;

	while(Q->count && Q->base[Q->front] != HEAD){ 		//取得包头
		Q->front = (Q->front + 1) % Q->maxsize;				//不是包头的话就丢弃
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
	if(count + 1 <= Q->count){		//队列有完整的数据包完整,才取出数据处理
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

