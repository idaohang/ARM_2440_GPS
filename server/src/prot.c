#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "prot.h"
#include "logc.h"

#define COMMTYPE_BYTENUM 1   //命令类型字节数
#define COMM_BYTENUM 1		//命令码字节数
#define HEAD_BYTENUM 1		//包头字节数
#define END_BYTENUM 1		//包尾字节数
#define DATALENTH_BYTENUM 4	//数据长度字节数
#define CHECKSUM_BYTENUM 1	//校验和字节数

unsigned char getXORSum(unsigned char *src, int checkSize);

int packet(unsigned char *p, unsigned char *data, int dataLenth, unsigned char commType, unsigned char comm)  
{
	unsigned char HEAD = 0x7E;
	unsigned char END = 0x7F;
	unsigned char ESCAPE_7D = 0x7d;
	unsigned char ESCAPE_5D = 0x5d;
	unsigned char ESCAPE_5E = 0x5e;
	unsigned char ESCAPE_5F = 0x5f;

	unsigned char ESCAPE_X;

	int checkSize = 0;
	int escapeCount = 0;	//封包后的数据的长度
	int escapeSize = 0;		
	int tempCount = 0;   //用来终止转义的条件！
	unsigned char checkSum;		//校验和
	unsigned char *checkData = NULL;
	unsigned char *escapeData = NULL;
	unsigned char *escapeTemp = NULL;
	unsigned char *escapeData_t = NULL;
	unsigned char *checkData_t = NULL;
	unsigned char escapeByte;

	//checkSize为命令类型——信息域的数据总长度
	checkSize = COMMTYPE_BYTENUM + COMM_BYTENUM +  DATALENTH_BYTENUM + dataLenth;
	if( NULL == (checkData = (unsigned char *)malloc(sizeof(unsigned char) * checkSize)) )
	{
		LOG(LOG_NOTICE, "申请内存失败！");
		return 0;
	}

	memset(checkData, 0, checkSize);
	//将命令类型——信息域的数据填充到checkData中
	checkData_t = checkData;
	memcpy(checkData_t, &commType, COMMTYPE_BYTENUM);
	checkData_t += COMMTYPE_BYTENUM;
	memcpy(checkData_t, &comm, COMM_BYTENUM);
	checkData_t += COMM_BYTENUM;
	memcpy(checkData_t, &dataLenth,  DATALENTH_BYTENUM);
	checkData_t +=  DATALENTH_BYTENUM;
	memcpy(checkData_t, data, dataLenth);

	//checkSum为命令类型——信息域数据的字节异或和
	checkSum = getXORSum(checkData, checkSize);
	


//进行转义的判断和处理
	escapeSize = checkSize + 1;  //escapeSize为命令类型——校验和的数据的长度
	if( NULL == (escapeData = (unsigned char *)malloc(sizeof(char) * escapeSize)) )
	{
		LOG(LOG_NOTICE, "申请内存失败！");
		return 0;
	}
	//将命令类型——校验和数据填充到escapeData中，方便后续的转义处理
	memcpy(escapeData, checkData, checkSize);
	memcpy(escapeData + checkSize, &checkSum, 1);

	//首先将HEAD值0x7E传入p中
	memcpy(p, &HEAD, 1);
	escapeCount = 1;


//然后将escapeData的内容一一进行转义检测并填充到p中
	escapeTemp = escapeData;
	escapeByte = *escapeTemp;
	escapeData_t = p + 1;

	while(tempCount < escapeSize)//tempCount为escapeData中已经被转义检测的数据长度
	{
		//0x7f转义
		if(END == escapeByte)
		{
			ESCAPE_X = ESCAPE_5F;
		}
		//0x7e转义
		else if(HEAD == escapeByte)
		{
			ESCAPE_X = ESCAPE_5E;
		}
		//0x7d转义
		else if(ESCAPE_7D == escapeByte)
		{
			ESCAPE_X = ESCAPE_5D;
		}
		else
		{	//检测出不需要转义，直接填充到p缓存中
			memcpy(escapeData_t, &escapeByte, 1);
			escapeData_t++;

			tempCount++;

			escapeTemp++;
			escapeByte = *escapeTemp;
			escapeCount++;
			continue;
		}
		//检测出需要转义的操作
		memcpy(escapeData_t, &ESCAPE_7D, 1);
		escapeData_t++;
		memcpy(escapeData_t, &ESCAPE_X, 1);
		escapeData_t++;

		tempCount++;

		escapeTemp++;
		escapeByte = *escapeTemp;
		escapeCount += 2;
	}

	//最后将END值0x7F传入p中
	memcpy(escapeData_t, &END, 1);
	escapeCount++;

	free(escapeData);
	free(checkData);
	
	return escapeCount;
}


//参数：p是解包后数据存放的缓存地址，data是要进行解包操作的包的地址
//		dataLenth为data数据的长度
//返回：成功返回解包后数据的长度checkSize，失败返回0.

int unpacket(unsigned char *p, unsigned char *data, int dataLenth) 
{
	unsigned char HEAD = 0x7E;
	unsigned char END = 0x7F;
	unsigned char ESCAPE_7D = 0x7d;
	
	unsigned char *escapeData = NULL;
	unsigned char *escapeTemp = NULL;
	unsigned char *dataTemp = NULL;
	unsigned char *escapeData_t = NULL;
	unsigned char *checkData = NULL;
	unsigned char *checkTemp = NULL;
	unsigned char checkSum;
	unsigned char escapeByte;
	unsigned char ESCAPE_X;

	int escapeCount = 0;
	int checkSize = 0;
	int tempCount = 0;
	
	tempCount = dataLenth-HEAD_BYTENUM-END_BYTENUM;

	if( NULL == ( escapeData=(unsigned char*)malloc(sizeof(unsigned char) * tempCount) ) )
	{
			LOG(LOG_NOTICE, "申请内存失败！");
			return 0;
	}


	dataTemp = data;
	dataTemp++;//直接忽略包头HEAD
	/*去除包头包尾*/
	memcpy(escapeData, dataTemp, tempCount);

	/*反转义处理*/	
	escapeTemp = escapeData;
	escapeByte = *escapeTemp;
	escapeData_t = p;
	while(tempCount > 0)
	{
		if(0x7d == escapeByte)
		{
			escapeTemp++;
			escapeByte = *escapeTemp;
			if(0x5d == escapeByte)
			{
				ESCAPE_X = ESCAPE_7D;
			}
			else if(0x5e == escapeByte)
			{
				ESCAPE_X = HEAD;
			}
			else if(0x5f == escapeByte)
			{
				ESCAPE_X = END;
			}
			else
			{
				LOG(LOG_NOTICE, "反转义处理中，发现封包转义出现错误！");
				return 0;
			}
			memcpy(escapeData_t, &ESCAPE_X, 1);
			escapeData_t++;
			escapeCount++;	
			tempCount--;	
		}
		else
		{
			memcpy(escapeData_t, &escapeByte, 1);
			escapeData_t++;
			escapeCount++;
		}
		
		tempCount--;
		escapeTemp++;
		escapeByte = *escapeTemp;
	}

	checkTemp = p;
	checkSize = escapeCount - CHECKSUM_BYTENUM;

	if( NULL == (checkData = (unsigned char *)malloc(sizeof(unsigned char) * checkSize)) )
	{
		LOG(LOG_NOTICE, "申请内存失败！");
		return 0;
	}

	memcpy(checkData, checkTemp, checkSize);

	checkTemp += checkSize;
	checkSum = *checkTemp;

	if( checkSum != getXORSum(checkData, checkSize) )
	{
		LOG(LOG_NOTICE, "校验和不一致！");
		return 0;
	}

	free(escapeData);
	free(checkData);

	return checkSize;

}

/*计算异或和*/
unsigned char getXORSum(unsigned char *src, int checkSize)
{
  unsigned char UcRc;
  unsigned char *srcTemp;
	int count = 0;
	 
  UcRc = 0;
  srcTemp = src;

  while(checkSize > count)      
  {
       UcRc ^= *srcTemp;
       srcTemp++;
	   count++;
   }

   return UcRc;
}
