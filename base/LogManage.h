#ifndef __LOG_MANAGE_INFO__
#define __LOG_MANAGE_INFO__
#include <unistd.h>
#define LOG_2_FILE
#ifndef LOG_2_FILE
bool OnInitLogInfo();
#define LOGOUT_FATAL printf
#define LOGOUT_ERROR printf
#define LOGOUT_NOTICE printf
#define LOGOUT_INFO printf
#define LOGOUT_DEBUG printf
#else
# if defined __GNUC__
#   define LOG_OUT_CHECK_PRINTF(m,n) __attribute__((format(printf,m,n)))
# else 
#   define LOG_OUT_CHECK_PRINTF(m,n)
# endif
typedef enum {
	LOGOUT_LEVEL_DEBUG = 20,
	LOGOUT_LEVEL_INFO = 40,
	LOGOUT_LEVEL_NOTICE = 60,
	LOGOUT_LEVEL_WARN = 80,
	LOGOUT_LEVEL_ERROR = 100,
	LOGOUT_LEVEL_FATAL = 120
} LOGOUT_LEVEL; 

bool OnInitLogInfo();
void OnLogOut(const char *fileName, size_t fileLen, const char *func, size_t funcLen,
	long line, const int level,
	const char *format, ...) LOG_OUT_CHECK_PRINTF(7,8);

void OnHLogOut(const char *file, size_t filelen,
	const char *func, size_t funclen,
	long line, int level,
	const void *buf, size_t buflen);

char * Hex2String (char buffer [], size_t length, void const * memory, size_t extent);

#define HEX2STRING(string, memory,iLen) Hex2String (string, sizeof (string), memory,iLen)
/* zlog macros */
#define LOGOUT_FATAL(format, args...) \
	OnLogOut(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	LOGOUT_LEVEL_FATAL, format, ##args)
#define  LOGOUT_ERROR(format, args...) \
	OnLogOut(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	LOGOUT_LEVEL_ERROR, format, ##args)
#define LOGOUT_WARN(format, args...) \
	OnLogOut(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	LOGOUT_LEVEL_WARN, format, ##args)
#define LOGOUT_NOTICE(format, args...) \
	OnLogOut(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	LOGOUT_LEVEL_NOTICE, format, ##args)
#define LOGOUT_INFO(format, args...) \
	OnLogOut(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	LOGOUT_LEVEL_INFO, format, ##args)
#define LOGOUT_DEBUG(format, args...) \
	OnLogOut(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	LOGOUT_LEVEL_DEBUG, format, ##args)

#define HLOGOUT_FATAL(buf, buf_len) \
	OnHLogOut(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	LOGOUT_LEVEL_FATAL, buf, buf_len)
#define  HLOGOUT_ERROR(buf, buf_len) \
	OnHLogOut(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	LOGOUT_LEVEL_ERROR, buf, buf_len)
#define HLOGOUT_WARN(buf, buf_len) \
	OnHLogOut(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	LOGOUT_LEVEL_WARN, buf, buf_len)
#define HLOGOUT_NOTICE(buf, buf_len) \
	OnHLogOut(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	LOGOUT_LEVEL_NOTICE, buf, buf_len)
#define HLOGOUT_INFO(buf, buf_len) \
	OnHLogOut(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	LOGOUT_LEVEL_INFO, buf, buf_len)
#define HLOGOUT_DEBUG(buf, buf_len) \
	OnHLogOut(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	LOGOUT_LEVEL_DEBUG, buf, buf_len)	
#endif
#endif
