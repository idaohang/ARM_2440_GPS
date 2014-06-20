/**
 * @file logc.c
 * @brief 简单日志接口。
 *
 * 直接输出日志请用LOG接口；需要指定日志文件名请用LOGN接口；需要扩展日志接口请用vLOGN接口。
 */
#include <stdio.h>
#include <string.h>
#include "timeo.h"
#include "logc.h"

/**
 * @brief 日志接口。
 * 
 * @param name 日志文件名称。
 * @param level 日志等级。
 * @param format 日志数据格式。
 * @param args 格式参数。
 */
void vLOGN(const char *name, const int level, const char *file, const int line, const char *format, va_list args)
{
	FILE *fp;
	char buf[LOG_SIZE + 1], FilePath[LOG_PATH_MAX + 1], file_name[LOG_NAME_MAX + 1];
	char ts[50];
	
	if (name == NULL) {
		strcpy(file_name, "log");
	} else {
		strcpy(file_name, name);
	}
	memset(ts, 0, sizeof(ts));
	sftime(ts, sizeof(ts), "yyyy-mm-dd");
	sprintf(FilePath, "%s%s.%s.log", LOG_PATH, file_name, ts);
	vsprintf(buf, format, args);
	fp = fopen(FilePath, "a+");
	if (fp == null) {
		return;
	}
	memset(ts, 0, sizeof(ts));
	sftime(ts, sizeof(ts), "yyyy-mm-dd hh:mi:ss.ms");
	fprintf(fp, "[%s] %s (%s:%d) - %s\n", priorities[level], ts, file, line, buf);
	fflush(fp);
	fclose(fp);
}

/**
 * @brief 写日志。
 * 
 * @param name 日志文件名称。
 * @param level 日志等级。
 * @param format 日志数据格式。
 * @param ... 格式参数。
 */
void _LOGN(const char *name, const int level, const char *file, const int line, const char *format, ...)
{
	va_list args;
	
	va_start(args, format);
	vLOGN(name, level, file, line, format, args);
	va_end(args);
}

/**
 * @brief 写日志。
 * 
 * @param level 日志等级。
 * @param format 日志数据格式。
 * @param ... 格式参数。
 */
void _LOG(const int level, const char *file, const int line, const char *format, ...)
{
	va_list args;
	
	va_start(args, format);
	vLOGN(NULL, level, file, line, format, args);
	va_end(args);
}

