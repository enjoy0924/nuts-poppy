/*
C语言风格日志库封装
*/
#ifndef __NPP_LOG_H__
#define __NPP_LOG_H__

#ifdef ALOG_EXPORTS
#define ALOG_API __declspec(dllexport)
#else
#define ALOG_API __declspec(dllimport)
#pragma comment(lib, "alog.lib")
#endif

#pragma comment(lib, "log4cplus.lib")

#define FILE_NAME "NPP"

typedef enum _ALOG_LEVEL
{
    ALOG_LEVEL_ERROR,
    ALOG_LEVEL_WARN,
    ALOG_LEVEL_INFO,
    ALOG_LEVEL_DEBUG,
    ALOG_LEVEL_TRACE
}ALOG_LEVEL;

#ifdef __GNUC__
#define LOG_CHECK_FMT(a, b) __attribute__((format(printf, a, b)))
#define CALLBACK
#else
#define LOG_CHECK_FMT(a, b)
#endif


#ifdef __cplusplus
extern "C" {
#endif
    //初始化日志配置文件
    ALOG_API int __stdcall AlogInit(const char* s_config_filename);
    ALOG_API void __stdcall alog_format(ALOG_LEVEL level, const char* module, const char* format, ...) LOG_CHECK_FMT(3, 4);
    ALOG_API void __stdcall AlogFini();
#ifdef __cplusplus
}

// 考虑兼容性，特增加log_printf_def和log_printf_dst
#ifdef _WIN32
#define log_printf_def(level, format, ...)      alog_format(level, "", format, ##__VA_ARGS__)
#else
#define log_printf_def(level, format, args...)  alog_format(level, "", format, ##args)
#endif
#define log_printf_dst alog_format

// for defualt module
#ifdef _WIN32
#define LOG_ERROR(fmt, ...)         alog_format(ALOG_LEVEL_ERROR, FILE_NAME, "<%d>\t<%s>,"fmt, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)          alog_format(ALOG_LEVEL_WARN, FILE_NAME, "<%d>\t<%s>,"fmt,  __LINE__, __FUNCTION__, ##__VA_ARGS__)    
#define LOG_INFO(fmt, ...)          alog_format(ALOG_LEVEL_INFO, FILE_NAME, "<%d>\t<%s>,"fmt,  __LINE__, __FUNCTION__, ##__VA_ARGS__)    
#define LOG_DEBUG(fmt, ...)         alog_format(ALOG_LEVEL_DEBUG, FILE_NAME, "<%d>\t<%s>,"fmt, __LINE__, __FUNCTION__, ##__VA_ARGS__)    
#define LOG_TRACE(fmt, ...)         alog_format(ALOG_LEVEL_TRACE, FILE_NAME, "<%d>\t<%s>,"fmt, __LINE__, __FUNCTION__, ##__VA_ARGS__) 
#else
#define LOG_ERROR(fmt, args...)     alog_format(ALOG_LEVEL_ERROR, "", "<%s>|<%d>\t<%s>,"fmt, __FILE__, __LINE__, __FUNCTION__, ##args)
#define LOG_WARN(fmt, args...)      alog_format(ALOG_LEVEL_WARN, "", "<%s>|<%d>\t<%s>,"fmt,  __FILE__, __LINE__, __FUNCTION__, ##args)
#define LOG_INFO(fmt, args...)      alog_format(ALOG_LEVEL_INFO, "", "<%s>|<%d>\t<%s>,"fmt,  __FILE__, __LINE__, __FUNCTION__, ##args)
#define LOG_DEBUG(fmt, args...)     alog_format(ALOG_LEVEL_DEBUG, "", "<%s>|<%d>\t<%s>,"fmt, __FILE__, __LINE__, __FUNCTION__, ##args)
#define LOG_TRACE(fmt, args...)     alog_format(ALOG_LEVEL_TRACE, "", "<%s>|<%d>\t<%s>,"fmt, __FILE__, __LINE__, __FUNCTION__, ##args)
#endif

#endif

#endif