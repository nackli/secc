#include <stdint.h>
#include "LogManage.h"
#include "zlog.h"
#define DIGITS_HEX_STRING                  "0123456789ABCDEF"
#define HEX_EXTENDER_CHAR           ':'
#define HEX_DIGITS_NUM              2
zlog_category_t *g_pZlogCategory =  NULL;
bool OnInitLogInfo()
{
#ifdef LOG_2_FILE    
    int rc;
   
    rc = zlog_init("LogFile_Config.conf");
    if (rc) 
    { 
        printf("init failed\n");
        return false;
    }
    
    g_pZlogCategory = zlog_get_category("Secc_rule");
    if (!g_pZlogCategory) 
    {
        printf("get cat fail\n");
        zlog_fini();
        return false;
    }
    if(dzlog_set_category("Secc_rule"))
        printf("Set Dlog Info Fail\n");
    //if(!dzlog_init("LogFile_Config.conf", "Secc_rule"))
    //    printf("Init dzlog fail\n");
   
#endif    
    return true;
}

void OnLogOut(const char *fileName, size_t fileLen, const char *func, size_t funcLen,
	long line, const int level,
	const char *format, ...)
{
    if(!g_pZlogCategory)
    {
        printf("zlog category fail\r\n");
        return;  
    }
    va_list arg_ptr;
    va_start(arg_ptr, format);
    vzlog(g_pZlogCategory, fileName, fileLen, func, funcLen, line, level, format,arg_ptr); 
    va_end(arg_ptr);
}

void OnHLogOut(const char *file, size_t filelen,
	const char *func, size_t funclen,
	long line, int level,
	const void *buf, size_t buflen)
{
	hdzlog(file, filelen, func, funclen, line, level, buf, buflen);   
}


static size_t OnHexString (void const * memory, register size_t extent, char buffer [], register size_t length)

{
	register char * string = (char *)(buffer);
	register uint8_t * offset = (uint8_t *)(memory);
	if (length)
	{
		length /= HEX_DIGITS_NUM + 1;
		while ((length--) && (extent--))
		{
			*string++ = DIGITS_HEX_STRING [(*offset >> 4) & 0x0F];
			*string++ = DIGITS_HEX_STRING [(*offset >> 0) & 0x0F];
			// if ((length) && (extent))
			// {
			// 	*string++ = HEX_EXTENDER_CHAR;
			// }
			offset++;
		}
		*string = (char) (0);
	}
	return (string - buffer);
}


char * Hex2String (char buffer [], size_t length, void const * memory, size_t extent)

{
	OnHexString (memory, extent, buffer, length);
	return ((char *)(buffer));
}
