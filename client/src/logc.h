/**
 * @file logc.h
 * @brief 简单日志接口。
 */
#ifndef __LOGC_H__
#define __LOGC_H__

#include "types.h"
#include <stdarg.h>

/** @brief 日志文件路径 */
#define LOG_PATH "../logs"
/** @brief 单条日志缓存大小(20K) */
#define LOG_SIZE (1024 * 20)
/** @brief 日志文件名最大长度 */
#define LOG_NAME_MAX 50
/** @brief 日志文件路径最大长度 */
#define LOG_PATH_MAX 256

/** @brief 日志等级(LOG_LEVEL) */
typedef enum {
	LOG_FATAL,
	LOG_ALERT,
	LOG_CRIT, 
	LOG_ERROR, 
	LOG_WARN, 
	LOG_NOTICE, 
	LOG_INFO, 
	LOG_DEBUG,
	LOG_TRACE,
	LOG_NOTSET,
	LOG_UNKNOWN
}LOG_LEVEL;

static const char * const priorities[] = {
	"FATAL", 
	"ALERT",
	"CRIT",
	"ERROR",
	"WARN",
	"NOTICE",
	"INFO",
	"DEBUG",
	"TRACE",
	"NOTSET",
	"UNKNOWN"
};

#define LOG(level, format, ...) _LOG(level, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOGN(name, level, format, ...) _LOGN(name, level, __FILE__, __LINE__, format, ##__VA_ARGS__)

void vLOGN(const char *name, const int level, const char *file, const int line, const char *format, va_list args);
void _LOGN(const char *name, const int level, const char *file, const int line, const char *format, ...);
void _LOG(const int level, const char *file, const int line, const char *format, ...);

#endif /*__LOGC_H__*/

