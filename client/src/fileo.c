#include "fileo.h"
#include "types.h"
#include "socket.h"

#include <unistd.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief	发送文件
 * @param 	src_path		要发送的文件的路径
 * @param 	desc_fd		要发送的文件描述符
 */
int send_file(const char *src_path, int desc_fd){
	int iRet = -1, src_fd;
	byte buffer[BUFFER_SIZE];
	ssize_t read_size;
	

	if(!src_path || (desc_fd < 0)){
		errno = EBADF;
		goto ERR;
	}
	if((src_fd = open(src_path, O_RDONLY)) == -1){
		goto ERR;
	}
	

	while((read_size = read(src_fd, buffer, BUFFER_SIZE))){
		if(read_size == -1 && errno != EINTR){
			goto READ_ERR;
		}else if(read_size > 0){				/*读到数据，就将其封包发送出去*/
			byte buf[BUFFER_SIZE + NAME_MAX_SIZE + 4];
			
			memset(buf, 0x00, sizeof(buf));
			*(int *)buf = strlen(src_path) + 1;			//文件名长度
			memcpy(buf+4, src_path, strlen(src_path) + 1);	//文件名
			memcpy(buf + 4 + strlen(src_path) + 1, buffer, read_size);
			send_data(desc_fd, COMM_TYPE_SERVE, COMM_SERV_BITMAP, buf, read_size + 4 + strlen(src_path) + 1);
		}
	}
READ_ERR:
	close(src_fd);
ERR:
	return iRet;
}


