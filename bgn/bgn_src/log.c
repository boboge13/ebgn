/******************************************************************************
*
* Copyright (C) Chaoyong Zhou
* Email: bgnvendor@gmail.com 
* QQ: 312230917
*
*******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif/*__cplusplus*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

#include "type.h"
#include "bgnctrl.h"
#include "log.h"

#include "mm.h"

#include "cmisc.h"

#include "cmutex.h"
#include "cstring.h"

#include "task.h"

LOG g_default_log_tbl[DEFAULT_END_LOG_INDEX] = {
    {LOG_FILE_DEVICE, LOGD_SWITCH_OFF_ENABLE , LOGD_PID_INFO_ENABLE ,NULL_PTR, {SWITCH_OFF, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, LOGD_FILE_RECORD_LIMIT_DISABLED, LOGD_FILE_MAX_RECORDS_LIMIT, 0}},/*LOGSTDOUT*/
    {LOG_FILE_DEVICE, LOGD_SWITCH_OFF_DISABLE, LOGD_PID_INFO_ENABLE ,NULL_PTR, {SWITCH_OFF, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, LOGD_FILE_RECORD_LIMIT_DISABLED, LOGD_FILE_MAX_RECORDS_LIMIT, 0}},/*LOGSTDIN*/
    {LOG_FILE_DEVICE, LOGD_SWITCH_OFF_DISABLE, LOGD_PID_INFO_ENABLE ,NULL_PTR, {SWITCH_OFF, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, LOGD_FILE_RECORD_LIMIT_DISABLED, LOGD_FILE_MAX_RECORDS_LIMIT, 0}},/*LOGSTDERR*/
    {LOG_FILE_DEVICE, LOGD_SWITCH_OFF_ENABLE , LOGD_PID_INFO_ENABLE ,NULL_PTR, {SWITCH_OFF, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, LOGD_FILE_RECORD_LIMIT_DISABLED, LOGD_FILE_MAX_RECORDS_LIMIT, 0}},/*LOGSTDNULL*/
    {LOG_FILE_DEVICE, LOGD_SWITCH_OFF_ENABLE , LOGD_PID_INFO_ENABLE ,NULL_PTR, {SWITCH_OFF, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, LOGD_FILE_RECORD_LIMIT_DISABLED, LOGD_FILE_MAX_RECORDS_LIMIT, 0}},/*LOGCONSOLE*/
    
    {LOG_FILE_DEVICE, LOGD_SWITCH_OFF_ENABLE , LOGD_PID_INFO_ENABLE ,NULL_PTR, {SWITCH_OFF, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, LOGD_FILE_RECORD_LIMIT_DISABLED, LOGD_FILE_MAX_RECORDS_LIMIT, 0}},/*LOGUSER05*/    
    {LOG_FILE_DEVICE, LOGD_SWITCH_OFF_ENABLE , LOGD_PID_INFO_ENABLE ,NULL_PTR, {SWITCH_OFF, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, LOGD_FILE_RECORD_LIMIT_DISABLED, LOGD_FILE_MAX_RECORDS_LIMIT, 0}},/*LOGUSER06*/
    {LOG_FILE_DEVICE, LOGD_SWITCH_OFF_ENABLE , LOGD_PID_INFO_ENABLE ,NULL_PTR, {SWITCH_OFF, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, LOGD_FILE_RECORD_LIMIT_DISABLED, LOGD_FILE_MAX_RECORDS_LIMIT, 0}},/*LOGUSER07*/
    {LOG_FILE_DEVICE, LOGD_SWITCH_OFF_ENABLE , LOGD_PID_INFO_ENABLE ,NULL_PTR, {SWITCH_OFF, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, LOGD_FILE_RECORD_LIMIT_DISABLED, LOGD_FILE_MAX_RECORDS_LIMIT, 0}},/*LOGUSER08*/
    {LOG_FILE_DEVICE, LOGD_SWITCH_OFF_ENABLE , LOGD_PID_INFO_ENABLE ,NULL_PTR, {SWITCH_OFF, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, LOGD_FILE_RECORD_LIMIT_DISABLED, LOGD_FILE_MAX_RECORDS_LIMIT, 0}},/*LOGUSER09*/
};

static UINT32 g_log_switch = SWITCH_ON;

UINT32 g_log_level[ SEC_NONE_END ];

EC_BOOL log_start()
{   
    LOG_FILE_FP(LOGSTDOUT)  = stdout;
    LOG_FILE_FP(LOGSTDIN)   = stdin;
    LOG_FILE_FP(LOGSTDERR)  = stderr;
    LOG_FILE_FP(LOGSTDNULL) = stdnull;/*invalid value*/
    LOG_FILE_FP(LOGCONSOLE) = stdout;

    LOG_REDIRECT(LOGSTDOUT)  = NULL_PTR;
    LOG_REDIRECT(LOGSTDIN)   = NULL_PTR;
    LOG_REDIRECT(LOGSTDERR)  = NULL_PTR;
    LOG_REDIRECT(LOGSTDNULL) = NULL_PTR;
    LOG_REDIRECT(LOGCONSOLE) = NULL_PTR;

    /*user log device*/
    LOG_FILE_FP(LOGUSER05) = NULL_PTR;
    LOG_FILE_FP(LOGUSER06) = NULL_PTR;
    LOG_FILE_FP(LOGUSER07) = NULL_PTR;
    LOG_FILE_FP(LOGUSER08) = NULL_PTR;
    LOG_FILE_FP(LOGUSER09) = NULL_PTR;

    LOG_REDIRECT(LOGUSER05) = NULL_PTR;
    LOG_REDIRECT(LOGUSER06) = NULL_PTR;
    LOG_REDIRECT(LOGUSER07) = NULL_PTR;
    LOG_REDIRECT(LOGUSER08) = NULL_PTR;
    LOG_REDIRECT(LOGUSER09) = NULL_PTR;    

    log_level_tab_init(g_log_level, SEC_NONE_END, LOG_DEFAULT_DBG_LEVEL);

    return (EC_TRUE);
}

void log_end()
{
    LOG_FILE_FP(LOGSTDOUT)  = NULL_PTR;
    LOG_FILE_FP(LOGSTDIN)   = NULL_PTR;
    LOG_FILE_FP(LOGSTDERR)  = NULL_PTR;
    LOG_FILE_FP(LOGSTDNULL) = NULL_PTR;
    LOG_FILE_FP(LOGCONSOLE) = NULL_PTR;

    LOG_REDIRECT(LOGSTDOUT)  = NULL_PTR;
    LOG_REDIRECT(LOGSTDIN)   = NULL_PTR;
    LOG_REDIRECT(LOGSTDERR)  = NULL_PTR;
    LOG_REDIRECT(LOGSTDNULL) = NULL_PTR;
    LOG_REDIRECT(LOGCONSOLE) = NULL_PTR;

    /*user log device*/
    LOG_FILE_FP(LOGUSER05) = NULL_PTR;
    LOG_FILE_FP(LOGUSER06) = NULL_PTR;
    LOG_FILE_FP(LOGUSER07) = NULL_PTR;
    LOG_FILE_FP(LOGUSER08) = NULL_PTR;
    LOG_FILE_FP(LOGUSER09) = NULL_PTR;

    LOG_REDIRECT(LOGUSER05) = NULL_PTR;
    LOG_REDIRECT(LOGUSER06) = NULL_PTR;
    LOG_REDIRECT(LOGUSER07) = NULL_PTR;
    LOG_REDIRECT(LOGUSER08) = NULL_PTR;
    LOG_REDIRECT(LOGUSER09) = NULL_PTR;     

    return;
}

void log_level_set(UINT32 *log_level_tab, const UINT32 log_level_tab_size, const UINT32 log_sector, const UINT32 log_level)
{
    UINT32 aligned_log_level;

    aligned_log_level = DMIN(LOG_MAX_DBG_LEVEL, log_level);
    
    if(log_sector < log_level_tab_size)
    {   
        log_level_tab[ log_sector ] = aligned_log_level;
    }
    
    return;
}

UINT32 log_level_get(const UINT32 *log_level_tab, const UINT32 log_level_tab_size, const UINT32 log_sector)
{   
    if(log_sector < log_level_tab_size)
    {
        return (log_level_tab[ log_sector ]);
    }
    
    return (LOG_ERR_DBG_LEVEL);
}

void log_level_tab_init(UINT32 *log_level_tab, const UINT32 log_level_tab_size, const UINT32 log_level)
{
    UINT32 log_sector;
    
    for(log_sector = 0; log_sector < log_level_tab_size; log_sector ++)
    {
        log_level_tab[ log_sector ] = log_level;
    }
    return;
}

void log_level_tab_clean(UINT32 *log_level_tab, const UINT32 log_level_tab_size)
{
    UINT32 log_sector;
    
    for(log_sector = 0; log_sector < log_level_tab_size; log_sector ++)
    {
        log_level_tab[ log_sector ] = LOG_ERR_DBG_LEVEL;
    }
    return;
}

void log_level_tab_set_all(UINT32 *log_level_tab, const UINT32 log_level_tab_size, const UINT32 log_level)
{
    UINT32 log_sector;
    
    for(log_sector = 0; log_sector < log_level_tab_size; log_sector ++)
    {
        log_level_tab[ log_sector ] = log_level;
    }
    return;
}

void log_level_tab_clone(const UINT32 *log_level_tab_src, UINT32 *log_level_tab_des, const UINT32 log_level_tab_size)
{
    UINT32 log_sector;
    
    for(log_sector = 0; log_sector < log_level_tab_size; log_sector ++)
    {
        log_level_tab_des[ log_sector ] = log_level_tab_src[ log_sector ];
    }
    return;
}

void log_level_tab_print(LOG *log, const UINT32 *log_level_tab, const UINT32 log_level_tab_size)
{
    UINT32 log_sector;
    
    for(log_sector = 0; log_sector < log_level_tab_size; log_sector ++)
    {
        UINT32 log_level;

        log_level = log_level_get(log_level_tab, log_level_tab_size, log_sector);
        sys_log(log, "log_level_tab_print: log sector: %8ld, log level: %8ld\n", log_sector, log_level);
    }
    return;
}

void log_level_import_from(const UINT32 *log_level_tab_src, UINT32 *log_level_tab_des, const UINT32 log_level_tab_size)
{
    UINT32 log_sector;
    
    for(log_sector = 0; log_sector < DMIN(SEC_NONE_END, log_level_tab_size); log_sector ++)
    {
        if(LOG_MAX_DBG_LEVEL >= log_level_tab_src[ log_sector ])
        {
            log_level_tab_des[ log_sector ] = log_level_tab_src[ log_sector ];
        }
    }
    return;
}

void log_level_import(const UINT32 *log_level_tab, const UINT32 log_level_tab_size)
{
    UINT32 log_sector;
    
    for(log_sector = 0; log_sector < DMIN(SEC_NONE_END, log_level_tab_size); log_sector ++)
    {
        if(LOG_MAX_DBG_LEVEL >= log_level_tab[ log_sector ])
        {
            g_log_level[ log_sector ] = log_level_tab[ log_sector ];
        }
    }
    return;
}

void log_level_export(UINT32 *log_level_tab, const UINT32 log_level_tab_size)
{
    UINT32 log_sector;
    
    for(log_sector = 0; log_sector < DMIN(SEC_NONE_END, log_level_tab_size); log_sector ++)
    {
        log_level_tab[ log_sector ] = g_log_level[ log_sector ];
    }
    return;
}

EC_BOOL log_level_set_all(const UINT32 log_level)
{
    log_level_tab_set_all(g_log_level, SEC_NONE_END, log_level);
    return (EC_TRUE);
}

EC_BOOL log_level_set_sector(const UINT32 log_sector, const UINT32 log_level)
{
    if(SEC_NONE_END <= log_sector)
    {
        dbg_log(SEC_0104_LOG, 0)(LOGSTDOUT, "error:log_level_set_sector: sector %ld overflow\n", log_sector);    
        return (EC_FALSE);
    }
    g_log_level[ log_sector ] = log_level;
    return (EC_TRUE);
}

void log_level_print(LOG *log)
{
    log_level_tab_print(log, g_log_level, SEC_NONE_END);
    return;
}

static int __log_cur_time_str0(char *time_str, const int max_size)
{
    time_t timestamp;
    CTM   *cur_time;
    CTMV  *cur_timev;
    CTMV   ctmv;
    
    int tv_msec;
    int len;

    time(&timestamp);
    cur_time = c_localtime_r(&timestamp);   

    cur_timev = &ctmv;
    gettimeofday(cur_timev, NULL_PTR);
    tv_msec = (int)(cur_timev->tv_usec / 1000);

    len = snprintf(time_str, max_size, "[%4d-%02d-%02d %02d:%02d:%02d.%03d] ",
                    TIME_IN_YMDHMS(cur_time),
                    tv_msec);
    return (len);                    
}

static int __log_cur_time_str(char *time_str, const int max_size)
{
    CTM   *cur_time;
    CTMV  *cur_timev;
    CTMV   ctmv;
    
    int tv_msec;
    int tv_usec;
    int len;

    cur_timev = &ctmv;
    gettimeofday(cur_timev, NULL_PTR);
    
    cur_time = c_localtime_r(&(cur_timev->tv_sec));
    tv_msec = (int)(cur_timev->tv_usec / 1000);
    tv_usec = (int)(cur_timev->tv_usec % 1000);

#if (SWITCH_OFF == LOG_PTHREAD_ID_SWITCH)
    len = snprintf(time_str, max_size, "[%4d-%02d-%02d %02d:%02d:%02d.%03d.%03d] ",
                    TIME_IN_YMDHMS(cur_time),
                    tv_msec, tv_usec);
#endif/*(SWITCH_OFF == LOG_PTHREAD_ID_SWITCH)*/
#if (SWITCH_ON == LOG_PTHREAD_ID_SWITCH)
    len = snprintf(time_str, max_size, "[%4d-%02d-%02d %02d:%02d:%02d.%03d.%03d][tid %d] ",
                    TIME_IN_YMDHMS(cur_time),
                    tv_msec, tv_usec,
                    CTHREAD_GET_TID());
#endif/*(SWITCH_ON == LOG_PTHREAD_ID_SWITCH)*/
    return (len);                    
}

static int __log_time_str(char *time_str, const int max_size)
{
#if (SWITCH_OFF == LOG_ACCURATE_TIME_SWITCH)
    int  len;
    char *log_time_str;

    log_time_str = LOG_TIME_STR();
    if(NULL_PTR == time_str)
    {
        len = __log_cur_time_str(time_str, max_size);
    }
    else
    {
#if (SWITCH_OFF == LOG_PTHREAD_ID_SWITCH)    
        len = snprintf(time_str, max_size, "[%s] ", log_time_str);
#endif/*(SWITCH_OFF == LOG_PTHREAD_ID_SWITCH)*/
#if (SWITCH_ON == LOG_PTHREAD_ID_SWITCH)    
        len = snprintf(time_str, max_size, "[%s][tid %d] ", log_time_str, CTHREAD_GET_TID());
#endif/*(SWITCH_ON == LOG_PTHREAD_ID_SWITCH)*/
    }
    return (len);
#endif /*(SWITCH_OFF == LOG_ACCURATE_TIME_SWITCH)*/  
#if (SWITCH_ON == LOG_ACCURATE_TIME_SWITCH)
    return __log_cur_time_str(time_str, max_size);
#endif/*(SWITCH_ON == LOG_ACCURATE_TIME_SWITCH)*/
    
}

static EC_BOOL __log_reg(LOG *log, FILE *fp)
{
    if(NULL_PTR != LOG_FILE_FP(log))
    {
        dbg_log(SEC_0104_LOG, 0)(LOGSTDOUT, "error:__log_reg: log %p had already registered\n", log);
        return (EC_FALSE);
    }

    if(NULL_PTR == fp)
    {
        dbg_log(SEC_0104_LOG, 0)(LOGSTDOUT, "error:__log_reg: log %p reject null fp\n", log);
        return (EC_FALSE);
    }

    LOG_FILE_FP(log) = fp;

    return (EC_TRUE);
}

/*register user log only!*/
EC_BOOL log_reg(LOG *log, FILE *fp)
{
    if(LOGUSER05 == log
    || LOGUSER06 == log
    || LOGUSER07 == log
    || LOGUSER08 == log
    || LOGUSER09 == log
    )
    {
        return __log_reg(log, fp);
    }

    dbg_log(SEC_0104_LOG, 0)(LOGSTDOUT, "error:log_reg: log %p is not LOGUSERxx\n", log);
    return (EC_FALSE);
}

static FILE * __log_unreg(LOG *log)
{
    FILE *fp;
    if(NULL_PTR == LOG_FILE_FP(log))
    {
        dbg_log(SEC_0104_LOG, 0)(LOGSTDOUT, "error:__log_unreg: log %p was not registered\n", log);
        return (NULL_PTR);
    }

    fp = LOG_FILE_FP(log);
    LOG_FILE_FP(log) = NULL_PTR;

    return (fp);
}

/*unregister user log only!*/
FILE *log_unreg(LOG *log)
{
    if(LOGUSER05 == log
    || LOGUSER06 == log
    || LOGUSER07 == log
    || LOGUSER08 == log
    || LOGUSER09 == log
    )
    {
        return __log_unreg(log);
    }

    dbg_log(SEC_0104_LOG, 0)(LOGSTDOUT, "error:log_unreg: log %p is not LOGUSERxx\n", log);
    return (NULL_PTR);
}

EC_BOOL user_log_open(LOG *log, const char *fname,const char *mode)
{
    FILE *fp;

    fp = fopen(fname, mode);
    if(NULL_PTR == fp)
    {
        dbg_log(SEC_0104_LOG, 0)(LOGSTDOUT, "error:user_log_open: open %s with mode %s failed\n", fname, mode);
        return (EC_FALSE);
    }

    if(EC_FALSE == log_reg(log, fp))
    {
        dbg_log(SEC_0104_LOG, 0)(LOGSTDOUT, "error:user_log_open: register log %s failed\n", fname);
        fclose(fp);
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL user_log_close(LOG *log)
{
    FILE *fp;

    fp = log_unreg(log);
    if(NULL_PTR == fp)
    {
        dbg_log(SEC_0104_LOG, 0)(LOGSTDOUT, "error:user_log_close: close log failed\n");
        return (EC_FALSE);
    }

    fclose(fp);
    return (EC_TRUE);
}

LOG *log_get_by_fp(FILE *fp)
{
    UINT32 idx;
    LOG *log;

    for(idx = 0; idx < sizeof(g_default_log_tbl)/sizeof(g_default_log_tbl[0]); idx ++)
    {
        log = &(g_default_log_tbl[ idx ]);
        if(LOG_FILE_DEVICE == LOG_DEVICE_TYPE(log) && fp == LOG_FILE_FP(log))
        {
            return log;
        }
    }
    return &(g_default_log_tbl[ idx - 1 ]);
}

LOG *log_get_by_fd(const UINT32 fd)
{
    if( /*0 <= fd && */fd < sizeof(g_default_log_tbl)/sizeof(g_default_log_tbl[0]))
    {
        return &(g_default_log_tbl[ fd ]);
    }
    return LOGSTDNULL;
}

static int sys_log_to_buf(char *buf, const int buf_max_size, const char * format,va_list ap)
{
    int   len;

    len  = __log_time_str(buf, buf_max_size);
    len += vsnprintf((char *)(buf + len), buf_max_size - len, format, ap);

    return (len);
}

static int sys_log_to_fd_orig(FILE *fp, const char * format,va_list ap)
{
    char time_str[TASK_BRD_TIME_STR_SIZE];
    int ret;
    
    __log_time_str(time_str, TASK_BRD_TIME_STR_SIZE);
    
    fprintf(fp, "%s", time_str);
    fflush(fp);

    ret = vfprintf(fp, format, ap);
    fflush( fp );

    return (ret);
}

static EC_BOOL log_node_clean(LOG_NODE *log_node)
{
    if(NULL_PTR != LOG_NODE_BUF(log_node))
    {
        free_static_mem(MD_TASK, CMPI_ANY_MODI, MM_UINT8_064K, LOG_NODE_BUF(log_node), LOC_LOG_0001);
        LOG_NODE_BUF(log_node) = NULL_PTR;
    }

    LOG_NODE_LEN(log_node) = 0;

    return (EC_TRUE);
}

static int sys_log_to_node(LOG *log, const char * format,va_list ap, LOG_NODE *log_node)
{
    int   len;
    char *buf;

    alloc_static_mem(MD_TASK, CMPI_ANY_MODI, MM_UINT8_064K, &buf, LOC_LOG_0002);
    if(NULL_PTR == buf)
    {
        LOG_NODE_LEN(log_node) = 0;
        LOG_NODE_BUF(log_node) = NULL_PTR;      
        return (0);
    }

    len = sys_log_to_buf(buf, 64 * 1024, format, ap);

    LOG_NODE_LEN(log_node) = len;
    LOG_NODE_BUF(log_node) = buf;

    return (len);
}

static int sys_print_to_node(LOG *log, const char * format,va_list ap, LOG_NODE *log_node)
{
    int   len;
    char *buf;

    alloc_static_mem(MD_TASK, CMPI_ANY_MODI, MM_UINT8_064K, &buf, LOC_LOG_0003);
    if(NULL_PTR == buf)
    {
        LOG_NODE_LEN(log_node) = 0;
        LOG_NODE_BUF(log_node) = NULL_PTR;    
        return (0);
    }

    len = vsnprintf(buf, 64 * 1024, format, ap);

    LOG_NODE_LEN(log_node) = len;
    LOG_NODE_BUF(log_node) = buf;

    return (len);
}

static int sys_log_to_fd(LOG *log, const char * format,va_list ap)
{
    int   len;
    char *buf;

    alloc_static_mem(MD_TASK, CMPI_ANY_MODI, MM_UINT8_064K, &buf, LOC_LOG_0004);
    if(NULL_PTR == buf)
    {
        return (0);
    }

    len = sys_log_to_buf(buf, 64 * 1024, format, ap);

    LOG_FILE_LOCK(log, LOC_LOG_0005);
    fprintf(LOG_FILE_FP(log), "%.*s", len, buf);
    fflush(LOG_FILE_FP(log));
    LOG_FILE_UNLOCK(log, LOC_LOG_0006);
    
    free_static_mem(MD_TASK, CMPI_ANY_MODI, MM_UINT8_064K, buf, LOC_LOG_0007);

    return (len);
}

static int sys_log_to_fd_no_lock(LOG *log, const char * format,va_list ap)
{
    FILE *fp;
    int   len;
    char *buf;

    fp = LOG_FILE_FP(log);

    alloc_static_mem(MD_TASK, CMPI_ANY_MODI, MM_UINT8_064K, &buf, LOC_LOG_0008);
    if(NULL_PTR == buf)
    {
        return (0);
    }

    len = sys_log_to_buf(buf, 64 * 1024, format, ap);
    fprintf(fp, "%.*s", len, buf);
    fflush(fp);
    
    free_static_mem(MD_TASK, CMPI_ANY_MODI, MM_UINT8_064K, buf, LOC_LOG_0009);

    return (len);
}

static int sys_log_to_cstring(CSTRING *cstring, const char * format, va_list ap)
{
    char time_str[TASK_BRD_TIME_STR_SIZE];
    
    __log_time_str(time_str, TASK_BRD_TIME_STR_SIZE);
    
    cstring_format(cstring, "%s", time_str);
    cstring_vformat(cstring, format, ap);
    return (0);
}

static int sys_print_to_fd_orig(FILE *fp, const char * format,va_list ap)
{
    int ret;

    ret = vfprintf(fp, format, ap);
    fflush( fp );

    return (ret);
}

static int sys_print_to_fd(LOG *log, const char * format,va_list ap)
{
    int   len;
    char *buf;

    alloc_static_mem(MD_TASK, CMPI_ANY_MODI, MM_UINT8_064K, &buf, LOC_LOG_0010);
    if(NULL_PTR == buf)
    {
        return (0);
    }

    len = vsnprintf(buf, 64 * 1024, format, ap);

    LOG_FILE_LOCK(log, LOC_LOG_0011);
    fprintf(LOG_FILE_FP(log), "%.*s", len, buf);
    fflush(LOG_FILE_FP(log));
    LOG_FILE_UNLOCK(log, LOC_LOG_0012);
    
    free_static_mem(MD_TASK, CMPI_ANY_MODI, MM_UINT8_064K, buf, LOC_LOG_0013);

    return (len);
}

static int sys_print_to_fd_no_lock(LOG *log, const char * format,va_list ap)
{
    int   len;
    char *buf;

    alloc_static_mem(MD_TASK, CMPI_ANY_MODI, MM_UINT8_064K, &buf, LOC_LOG_0014);
    if(NULL_PTR == buf)
    {
        return (0);
    }

    len = vsnprintf(buf, 64 * 1024, format, ap);

    fprintf(LOG_FILE_FP(log), "%.*s", len, buf);
    fflush(LOG_FILE_FP(log));
    
    free_static_mem(MD_TASK, CMPI_ANY_MODI, MM_UINT8_064K, buf, LOC_LOG_0015);

    return (len);
}

static int sys_print_to_cstring(CSTRING *cstring, const char * format, va_list ap)
{
    cstring_vformat(cstring, format, ap);
    return (0);
}

int sys_log_no_lock(LOG *log, const char * format, ...)
{
    LOG *des_log;
    va_list ap;

    if( SWITCH_OFF == g_log_switch && NULL_PTR != log && LOGD_SWITCH_OFF_ENABLE == LOG_SWITCH_OFF_ENABLE(log) )
    {
        return (0);
    }

    des_log = log;
    while(NULL_PTR != des_log && NULL_PTR != des_log->redirect_log)
    {
        des_log = des_log->redirect_log;
    }

    if(LOGSTDNULL == des_log)
    {
        return (0);
    }

    if(LOG_FILE_DEVICE == LOG_DEVICE_TYPE(des_log) && NULL_PTR != LOG_FILE_FP(des_log))
    {
        int ret;

        va_start(ap, format);
        ret = sys_log_to_fd_no_lock(des_log, format, ap);
        va_end(ap);

        ++ LOG_FILE_CUR_RECORDS(des_log);
        
        if(LOGD_FILE_RECORD_LIMIT_ENABLED == LOG_FILE_LIMIT_ENABLED(des_log))
        {
            //WARNING: here need a lock!
            if(LOG_FILE_CUR_RECORDS(des_log) > LOG_FILE_RECORDS_LIMIT(des_log))
            {
                if(LOG_FILE_CUR_RECORDS(des_log) > LOG_FILE_RECORDS_LIMIT(des_log))/*double confirm!*/
                {
                    log_file_freopen(des_log);
                    LOG_FILE_CUR_RECORDS(des_log) = 0;
                }
            }
        }

        return (ret);
    }

    if(LOG_CSTR_DEVICE == LOG_DEVICE_TYPE(des_log) && NULL_PTR != LOG_CSTR(des_log))
    {
        int ret;
        va_start(ap, format);
        ret = sys_log_to_cstring(LOG_CSTR(des_log), format, ap);
        va_end(ap);

        return (ret);
    }

    return (-1);
}

int sys_log(LOG *log, const char * format, ...)
{
    LOG *des_log;
    va_list ap;

    if( SWITCH_OFF == g_log_switch && NULL_PTR != log && LOGD_SWITCH_OFF_ENABLE == LOG_SWITCH_OFF_ENABLE(log) )
    {
        return (0);
    }

    des_log = log;
    while(NULL_PTR != des_log && NULL_PTR != des_log->redirect_log)
    {
        des_log = des_log->redirect_log;
    }

    if(LOGSTDNULL == des_log)
    {
        return (0);
    }
    
    if(LOG_FILE_DEVICE == LOG_DEVICE_TYPE(des_log) && NULL_PTR != LOG_FILE_FP(des_log))
    {
        LOG_NODE log_node;
        int ret;

        va_start(ap, format);
        ret = sys_log_to_node(des_log, format, ap, &log_node);
        va_end(ap);
    
        LOG_FILE_LOCK(des_log, LOC_LOG_0016);

        fprintf(LOG_FILE_FP(des_log), "%.*s", LOG_NODE_LEN(&log_node), LOG_NODE_BUF(&log_node));
        fflush(LOG_FILE_FP(des_log));

        ++ LOG_FILE_CUR_RECORDS(des_log);
        
        if(LOGD_FILE_RECORD_LIMIT_ENABLED == LOG_FILE_LIMIT_ENABLED(des_log))
        {            
            if(LOG_FILE_CUR_RECORDS(des_log) > LOG_FILE_RECORDS_LIMIT(des_log))
            {
                log_file_freopen(des_log);
                LOG_FILE_CUR_RECORDS(des_log) = 0;
            }            
        }
      
        LOG_FILE_UNLOCK(des_log, LOC_LOG_0017);

        log_node_clean(&log_node);

        return (ret);
    }
    
    if(LOG_CSTR_DEVICE == LOG_DEVICE_TYPE(des_log) && NULL_PTR != LOG_CSTR(des_log))
    {
        int ret;
        va_start(ap, format);
        ret = sys_log_to_cstring(LOG_CSTR(des_log), format, ap);
        va_end(ap);

        return (ret);
    }

    return (-1);
}

int sys_print_no_lock(LOG *log, const char * format, ...)
{
    LOG *des_log;
    va_list ap;

    if( SWITCH_OFF == g_log_switch && NULL_PTR != log && LOGD_SWITCH_OFF_ENABLE == LOG_SWITCH_OFF_ENABLE(log) )
    {
        return (0);
    }

    des_log = log;
    while(NULL_PTR != des_log && NULL_PTR != des_log->redirect_log)
    {
        des_log = des_log->redirect_log;
    }

    if(LOGSTDNULL == des_log)
    {
        return (0);
    }

    if(LOG_FILE_DEVICE == LOG_DEVICE_TYPE(des_log) && NULL_PTR != LOG_FILE_FP(des_log))
    {
        int ret;

        va_start(ap, format);
        ret = sys_print_to_fd_no_lock(des_log, format, ap);
        va_end(ap);

        ++ LOG_FILE_CUR_RECORDS(des_log);
        if(LOGD_FILE_RECORD_LIMIT_ENABLED == LOG_FILE_LIMIT_ENABLED(des_log))
        {
            //WARNING: here need a lock!
            if(LOG_FILE_CUR_RECORDS(des_log) > LOG_FILE_RECORDS_LIMIT(des_log))
            {
                if(LOG_FILE_CUR_RECORDS(des_log) > LOG_FILE_RECORDS_LIMIT(des_log))/*double confirm!*/
                {
                    log_file_freopen(des_log);
                    LOG_FILE_CUR_RECORDS(des_log) = 0;
                }
            }            
        }

        return (ret);
    }

    if(LOG_CSTR_DEVICE == LOG_DEVICE_TYPE(des_log) && NULL_PTR != LOG_CSTR(des_log))
    {
        int ret;
        va_start(ap, format);
        ret = sys_print_to_cstring(LOG_CSTR(des_log), format, ap);
        va_end(ap);
        return (ret);
    }

    return (-1);
}

int sys_print(LOG *log, const char * format, ...)
{
    LOG *des_log;
    va_list ap;

    //return (0);/*debug*/

    if( SWITCH_OFF == g_log_switch && NULL_PTR != log && LOGD_SWITCH_OFF_ENABLE == LOG_SWITCH_OFF_ENABLE(log) )
    {
        return (0);
    }

    des_log = log;
    //if(LOGSTDNULL == log) des_log = LOGSTDOUT; /*debug!*/
    while(NULL_PTR != des_log && NULL_PTR != des_log->redirect_log)
    {
        des_log = des_log->redirect_log;
    }

    if(LOGSTDNULL == des_log)
    {
        return (0);
    }

    if(LOG_FILE_DEVICE == LOG_DEVICE_TYPE(des_log) && NULL_PTR != LOG_FILE_FP(des_log))
    {
        LOG_NODE log_node;
        int ret;

        va_start(ap, format);
        ret = sys_print_to_node(NULL_PTR, format, ap, &log_node);/*no timestamp insert ahead*/
        va_end(ap);
    
        LOG_FILE_LOCK(des_log, LOC_LOG_0018);

        fprintf(LOG_FILE_FP(des_log), "%.*s", LOG_NODE_LEN(&log_node), LOG_NODE_BUF(&log_node));
        fflush(LOG_FILE_FP(des_log));        

        ++ LOG_FILE_CUR_RECORDS(des_log);
        
        if(LOGD_FILE_RECORD_LIMIT_ENABLED == LOG_FILE_LIMIT_ENABLED(des_log))
        {            
            if(LOG_FILE_CUR_RECORDS(des_log) > LOG_FILE_RECORDS_LIMIT(des_log))
            {
                log_file_freopen(des_log);
                LOG_FILE_CUR_RECORDS(des_log) = 0;
            }            
        }
      
        LOG_FILE_UNLOCK(des_log, LOC_LOG_0019);

        log_node_clean(&log_node);

        return (ret);
    }

    if(LOG_CSTR_DEVICE == LOG_DEVICE_TYPE(des_log) && NULL_PTR != LOG_CSTR(des_log))
    {
        int ret;
        va_start(ap, format);
        ret = sys_print_to_cstring(LOG_CSTR(des_log), format, ap);
        va_end(ap);
        return (ret);
    }

    return (-1);
}

int sys_log_switch_on()
{
    g_log_switch = SWITCH_ON;
    return (0);
}

int sys_log_switch_off()
{
    g_log_switch = SWITCH_OFF;
    return (0);
}

int sys_log_redirect_setup(LOG *old_log, LOG *new_log)
{
    LOG_REDIRECT(old_log) = new_log;
    return (0);
}

LOG * sys_log_redirect_cancel(LOG *log)
{
    LOG *old_log;

    old_log = LOG_REDIRECT(log);
    LOG_REDIRECT(log) = NULL_PTR;

    return (old_log);
}

LOG *log_file_new(const char *fname, const char *mode, const UINT32 tcid, const UINT32 rank, const UINT32 record_limit_enabled, const UINT32 fname_with_date_switch, const UINT32 switch_off_enable, const UINT32 pid_info_enable)
{
    LOG *log;

    alloc_static_mem(MD_TASK, 0, MM_LOG, &log, LOC_LOG_0020);
    if(EC_FALSE == log_file_init(log, fname, mode, tcid, rank, record_limit_enabled, fname_with_date_switch, switch_off_enable, pid_info_enable))
    {
        dbg_log(SEC_0104_LOG, 0)(LOGSTDOUT, "error:log_file_new: log file %s init failed\n", fname);
        free_static_mem(MD_TASK, 0, MM_LOG, log, LOC_LOG_0021);
        return (NULL_PTR);
    }
    return (log);
}

EC_BOOL log_file_init(LOG *log, const char *fname, const char *mode, const UINT32 tcid, const UINT32 rank, const UINT32 record_limit_enabled, const UINT32 fname_with_date_switch, const UINT32 switch_off_enable, const UINT32 pid_info_enable)
{
    if(NULL_PTR == fname)
    {
        LOG_DEVICE_TYPE(log)         = LOG_FILE_DEVICE;
        LOG_SWITCH_OFF_ENABLE(log)   = switch_off_enable;
        LOG_PID_INFO_ENABLE(log)     = pid_info_enable;
        LOG_REDIRECT(log)            = NULL_PTR;

        LOG_FILE_NAME_WITH_DATE_SWITCH(log) = fname_with_date_switch;

        LOG_FILE_NAME(log)           = NULL_PTR;
        LOG_FILE_MODE(log)           = NULL_PTR;
        LOG_FILE_FP(log)             = NULL_PTR;
        LOG_FILE_CMUTEX(log)         = NULL_PTR;
        LOG_FILE_LIMIT_ENABLED(log)  = record_limit_enabled;
        LOG_FILE_RECORDS_LIMIT(log)  = LOGD_FILE_MAX_RECORDS_LIMIT;
        LOG_FILE_CUR_RECORDS(log)    = 0;

        LOG_FILE_TCID(log)           = tcid;
        LOG_FILE_RANK(log)           = rank;
        return (EC_TRUE);
    }

    LOG_DEVICE_TYPE(log)       = LOG_FILE_DEVICE;
    LOG_SWITCH_OFF_ENABLE(log) = switch_off_enable;
    LOG_PID_INFO_ENABLE(log)   = pid_info_enable;
    LOG_REDIRECT(log)          = NULL_PTR;

    LOG_FILE_TCID(log) = tcid;
    LOG_FILE_RANK(log) = rank;

    LOG_FILE_NAME_WITH_DATE_SWITCH(log) = fname_with_date_switch;

    LOG_FILE_NAME(log) = cstring_new((UINT8 *)fname, LOC_LOG_0022);
    LOG_FILE_MODE(log) = cstring_new((UINT8 *)mode, LOC_LOG_0023);

    LOG_FILE_CMUTEX(log) = c_mutex_new(CMUTEX_PROCESS_PRIVATE, LOC_LOG_0024);
    if(NULL_PTR == LOG_FILE_CMUTEX(log))
    {
        fprintf(stderr,"error:log_file_init: failed to new cmutex for %s\n", (char *)LOG_FILE_NAME_STR(log));
        fflush(stderr);

        cstring_free(LOG_FILE_NAME(log));
        LOG_FILE_NAME(log) = NULL_PTR;

        cstring_free(LOG_FILE_MODE(log));
        LOG_FILE_MODE(log) = NULL_PTR;

        LOG_DEVICE_TYPE(log) = LOG_NULL_DEVICE;

        return (EC_FALSE);
    }

    if(EC_FALSE == log_file_fopen(log))
    {
        fprintf(stderr,"error:log_file_init: failed to open %s to write\n", (char *)LOG_FILE_NAME_STR(log));
        fflush(stderr);

        cstring_free(LOG_FILE_NAME(log));
        LOG_FILE_NAME(log) = NULL_PTR;

        cstring_free(LOG_FILE_MODE(log));
        LOG_FILE_MODE(log) = NULL_PTR;

        LOG_DEVICE_TYPE(log) = LOG_NULL_DEVICE;

        c_mutex_free(LOG_FILE_CMUTEX(log), LOC_LOG_0025);
        LOG_FILE_CMUTEX(log) = NULL_PTR;

        return (EC_FALSE);
    }

    LOG_FILE_LIMIT_ENABLED(log) = record_limit_enabled;
    if(LOGD_FILE_RECORD_LIMIT_ENABLED == record_limit_enabled)
    {
        LOG_FILE_RECORDS_LIMIT(log) = FILE_LOG_MAX_RECORDS;
        LOG_FILE_CUR_RECORDS(log)   = 0;
    }
    else
    {
        LOG_FILE_RECORDS_LIMIT(log) = LOGD_FILE_MAX_RECORDS_LIMIT;
        LOG_FILE_CUR_RECORDS(log)   = 0;
    }

    return (EC_TRUE);
}

EC_BOOL log_file_clean(LOG *log)
{
    if(LOGSTDOUT == log || LOGSTDIN == log || LOGSTDERR == log)
    {
        return (EC_TRUE);
    }

    if(NULL_PTR != LOG_FILE_NAME(log))
    {
        cstring_free(LOG_FILE_NAME(log));
        LOG_FILE_NAME(log) = NULL_PTR;
    }

    if(NULL_PTR != LOG_FILE_MODE(log))
    {
        cstring_free(LOG_FILE_MODE(log));
        LOG_FILE_MODE(log) = NULL_PTR;
    }

    log_file_fclose(log);
    if(NULL_PTR != LOG_FILE_CMUTEX(log))
    {
        c_mutex_free(LOG_FILE_CMUTEX(log), LOC_LOG_0026);
        LOG_FILE_CMUTEX(log) = NULL_PTR;
    }

    LOG_FILE_LIMIT_ENABLED(log) = LOGD_FILE_RECORD_LIMIT_DISABLED;
    LOG_FILE_RECORDS_LIMIT(log) = LOGD_FILE_MAX_RECORDS_LIMIT;
    LOG_FILE_CUR_RECORDS(log)   = 0;

    LOG_DEVICE_TYPE(log) = LOG_NULL_DEVICE;
    LOG_REDIRECT(log)    = NULL_PTR;

    return (EC_TRUE);
}

EC_BOOL log_file_fopen(LOG *log)
{
    char fname[128];

    if(SWITCH_ON == LOG_FILE_NAME_WITH_DATE_SWITCH(log))
    {
        CTM *cur_time;
        cur_time = LOG_TM();

        snprintf(fname, sizeof(fname) - 1, "%s_%4d%02d%02d_%02d%02d%02d.log",
                (char *)LOG_FILE_NAME_STR(log),
                TIME_IN_YMDHMS(cur_time));
    }
    else
    {
        snprintf(fname, sizeof(fname) - 1, "%s.log", (char *)LOG_FILE_NAME_STR(log));
    }

    if(EC_FALSE == c_basedir_create(fname))
    {
        return (EC_FALSE);
    }

    LOG_FILE_FP(log) = fopen(fname, (char *)LOG_FILE_MODE_STR(log));
    if(NULL_PTR == LOG_FILE_FP(log))
    {
        return (EC_FALSE);
    }

#if 1
    if(LOGD_PID_INFO_ENABLE == LOG_PID_INFO_ENABLE(log))
    {
        char time_str[TASK_BRD_TIME_STR_SIZE];
        
        __log_time_str(time_str, TASK_BRD_TIME_STR_SIZE);    
        
    
        fprintf(LOG_FILE_FP(log), "%s", time_str);
        fprintf(LOG_FILE_FP(log), "my pid = %u, tcid = %s, rank = %ld\n",
                                  getpid(), c_word_to_ipv4(LOG_FILE_TCID(log)), LOG_FILE_RANK(log));
        fflush(LOG_FILE_FP(log));
    }
#endif
    return (EC_TRUE);
}

EC_BOOL log_file_fclose(LOG *log)
{
    if(LOGSTDOUT == log || LOGSTDIN == log || LOGSTDERR == log)
    {
        return (EC_TRUE);
    }

    if(NULL_PTR == LOG_FILE_FP(log))
    {
        return (EC_TRUE);
    }

    if(stdout != LOG_FILE_FP(log) && stderr != LOG_FILE_FP(log) && stdin != LOG_FILE_FP(log))
    {
        fprintf(LOG_FILE_FP(log), "log_file_fclose: ------- close log %p (fp %p) ------------\n", log, LOG_FILE_FP(log));
        fflush(LOG_FILE_FP(log));
    
        fclose(LOG_FILE_FP(log));
        LOG_FILE_FP(log) = NULL_PTR;
    }

    return (EC_TRUE);
}

EC_BOOL log_file_freopen(LOG *log)
{
    log_file_fclose(log);/*close old*/

    if(EC_FALSE == log_file_fopen(log)) /*open new*/
    {
        dbg_log(SEC_0104_LOG, 0)(LOGCONSOLE, "error:log_file_freopen: failed to reopen file %s with mode %s\n",
                           (char *)LOG_FILE_NAME_STR(log),
                           (char *)LOG_FILE_MODE_STR(log));
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

void log_file_free(LOG *log)
{
    if(NULL_PTR != log)
    {
        log_file_clean(log);
        free_static_mem(MD_TASK, 0, MM_LOG, log, LOC_LOG_0027);
    }
    return;
}

LOG * log_file_open(const char *fname, const char *mode, const UINT32 tcid, const UINT32 rank, const UINT32 record_limit_enabled, const UINT32 fname_with_date_switch, const UINT32 switch_off_enable, const UINT32 pid_info_enable)
{
    return log_file_new(fname, mode, tcid, rank, record_limit_enabled, fname_with_date_switch, switch_off_enable, pid_info_enable);
}

EC_BOOL log_file_close(LOG *log)
{
    return log_free(log);
}

LOG *log_cstr_new()
{
    LOG *log;

    alloc_static_mem(MD_TASK, 0, MM_LOG, &log, LOC_LOG_0028);
    LOG_CSTR(log) = NULL_PTR;
    log_cstr_init(log);

    return log;
}

EC_BOOL log_cstr_init(LOG *log)
{
    LOG_DEVICE_TYPE(log) = LOG_CSTR_DEVICE;
    LOG_REDIRECT(log)    = NULL_PTR;

    if(NULL_PTR == LOG_CSTR(log))
    {
        LOG_CSTR(log) = cstring_new(NULL_PTR, LOC_LOG_0029);
    }

    return (EC_TRUE);
}

EC_BOOL log_cstr_clean(LOG *log)
{
    cstring_free(LOG_CSTR(log));
    LOG_CSTR(log) = NULL_PTR;

    LOG_DEVICE_TYPE(log) = LOG_NULL_DEVICE;
    LOG_REDIRECT(log)    = NULL_PTR;
    return (EC_TRUE);
}

void log_cstr_free(LOG *log)
{
    if(NULL_PTR != log)
    {
        log_cstr_clean(log);
        free_static_mem(MD_TASK, 0, MM_LOG, log, LOC_LOG_0030);
    }
    return;
}

LOG * log_cstr_open()
{
    return log_cstr_new();
}

EC_BOOL log_cstr_close(LOG *log)
{
    return log_free(log);
}


EC_BOOL log_clean(LOG *log)
{
    if(LOGSTDOUT == log || LOGSTDIN == log || LOGSTDERR == log)
    {
        return (EC_TRUE);
    }

    if(LOG_FILE_DEVICE == LOG_DEVICE_TYPE(log))
    {
        return log_file_clean(log);
    }

    if(LOG_CSTR_DEVICE == LOG_DEVICE_TYPE(log))
    {
        return log_cstr_clean(log);
    }

    LOG_SWITCH_OFF_ENABLE(log) = LOGD_SWITCH_OFF_DISABLE;
    LOG_PID_INFO_ENABLE(log)   = LOGD_PID_INFO_DISABLE;

    return (EC_TRUE);
}

EC_BOOL log_free(LOG *log)
{
    if(NULL_PTR != log)
    {
        log_clean(log);
        free_static_mem(MD_TASK, 0, MM_LOG, log, LOC_LOG_0031);
    }
    return (EC_TRUE);
}

UINT32 log_init_0(const UINT32 md_id, LOG *log)
{
    LOG_DEVICE_TYPE(log)       = LOG_NULL_DEVICE;
    LOG_SWITCH_OFF_ENABLE(log) = LOGD_SWITCH_OFF_DISABLE;
    LOG_PID_INFO_ENABLE(log)   = LOGD_PID_INFO_DISABLE;
    LOG_REDIRECT(log)          = NULL_PTR;
    LOG_CSTR(log)              = NULL_PTR;

    return (0);
}
UINT32 log_clean_0(const UINT32 md_id, LOG *log)
{
    log_clean(log);
    return (0);
}

UINT32 log_free_0(const UINT32 md_id, LOG *log)
{
    log_free(log);
    return (0);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/
