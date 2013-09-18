/******************************************************************************
*
* Copyright (C) Chaoyong Zhou
* Email: bgnvendor@gmail.com 
* QQ: 2796796
*
*******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif/*__cplusplus*/

#ifndef _LOG_H
#define _LOG_H

#include <stdio.h>
#include <stdlib.h>

#include "type.h"
#include "cstring.h"

#define LOG_FILE_DEVICE      ((UINT32) 1)
#define LOG_CSTR_DEVICE      ((UINT32) 2)
#define LOG_NULL_DEVICE      ((UINT32) 3)

#define stdnull ((FILE *)-1)

#define DEFAULT_STDOUT_LOG_INDEX    0
#define DEFAULT_STDIN_LOG_INDEX     1
#define DEFAULT_STDERR_LOG_INDEX    2
#define DEFAULT_STDNULL_LOG_INDEX   3
#define DEFAULT_CONSOLE_LOG_INDEX   4
#define DEFAULT_END_LOG_INDEX       5

extern LOG g_default_log_tbl[];
#define LOGSTDOUT  (&g_default_log_tbl[ DEFAULT_STDOUT_LOG_INDEX  ])
#define LOGSTDIN   (&g_default_log_tbl[ DEFAULT_STDIN_LOG_INDEX   ])
#define LOGSTDERR  (&g_default_log_tbl[ DEFAULT_STDERR_LOG_INDEX  ])
#define LOGSTDNULL (&g_default_log_tbl[ DEFAULT_STDNULL_LOG_INDEX ])
#define LOGCONSOLE (&g_default_log_tbl[ DEFAULT_CONSOLE_LOG_INDEX ])

#define LOG_DEVICE_TYPE(this_log)                   ((this_log)->type)
#define LOG_SWITCH_OFF_ENABLE(this_log)             ((this_log)->switch_off_enable)
#define LOG_PID_INFO_ENABLE(this_log)               ((this_log)->pid_info_enable)
#define LOG_REDIRECT(this_log)                      ((this_log)->redirect_log)

#define LOG_FILE_NAME_WITH_DATE_SWITCH(file_log)    ((file_log)->logd.file.fname_with_date_switch)
#define LOG_FILE_NAME(file_log)                     ((file_log)->logd.file.fname)
#define LOG_FILE_NAME_STR(file_log)                 (cstring_get_str(LOG_FILE_NAME(file_log)))
#define LOG_FILE_MODE(file_log)                     ((file_log)->logd.file.mode)
#define LOG_FILE_MODE_STR(file_log)                 (cstring_get_str(LOG_FILE_MODE(file_log)))
#define LOG_FILE_FP(file_log)                       ((file_log)->logd.file.fp)
#define LOG_FILE_CMUTEX(file_log)                   ((file_log)->logd.file.mutex)
#define LOG_FILE_LIMIT_ENABLED(file_log)            ((file_log)->logd.file.record_limit_enabled)
#define LOG_FILE_RECORDS_LIMIT(file_log)            ((file_log)->logd.file.max_records)
#define LOG_FILE_CUR_RECORDS(file_log)              ((file_log)->logd.file.cur_records)
#define LOG_FILE_TCID(file_log)                     ((file_log)->logd.file.tcid)
#define LOG_FILE_RANK(file_log)                     ((file_log)->logd.file.rank)
                                               
#define LOG_CSTR(cstr_log)                          ((cstr_log)->logd.cstring)

#if 1
#define LOG_FILE_LOCK(file_log, location) do{\
if(NULL_PTR != (file_log) && LOG_FILE_DEVICE == LOG_DEVICE_TYPE(file_log) && NULL_PTR != LOG_FILE_FP(file_log) && NULL_PTR != LOG_FILE_CMUTEX(file_log)) {\
    c_mutex_lock(LOG_FILE_CMUTEX(file_log), location);\
}\
}while(0)

#define LOG_FILE_UNLOCK(file_log, location) do{\
if(NULL_PTR != (file_log) && LOG_FILE_DEVICE == LOG_DEVICE_TYPE(file_log) && NULL_PTR != LOG_FILE_FP(file_log) && NULL_PTR != LOG_FILE_CMUTEX(file_log)) {\
    c_mutex_unlock(LOG_FILE_CMUTEX(file_log), location);\
}\
}while(0)
#else
#define LOG_FILE_LOCK(file_log, location) do{}while(0)
#define LOG_FILE_UNLOCK(file_log, location) do{}while(0)
#endif

EC_BOOL log_start();

void log_end();

LOG *log_get_by_fp(FILE *fp);

LOG *log_get_by_fd(const UINT32 fd);

int sys_log_no_lock(LOG *log, const char * format, ...);

int sys_log(LOG *log, const char * format, ...);

int sys_print_no_lock(LOG *log, const char * format, ...);

int sys_print(LOG *log, const char * format, ...);

int sys_log_switch_on();

int sys_log_switch_off();

int sys_log_redirect_setup(LOG *old_log, LOG *new_log);

LOG * sys_log_redirect_cancel(LOG *log);

LOG *log_file_new(const char *fname, const char *mode, const UINT32 tcid, const UINT32 rank, const UINT32 record_limit_enabled, const UINT32 fname_with_date_switch, const UINT32 switch_off_enable, const UINT32 pid_info_enable);

EC_BOOL log_file_init(LOG *log, const char *fname, const char *mode, const UINT32 tcid, const UINT32 rank, const UINT32 record_limit_enabled, const UINT32 fname_with_date_switch, const UINT32 switch_off_enable, const UINT32 pid_info_enable);

EC_BOOL log_file_clean(LOG *log);

EC_BOOL log_file_fopen(LOG *log);

EC_BOOL log_file_fclose(LOG *log);

EC_BOOL log_file_freopen(LOG *log);

void log_file_free(LOG *log);

LOG * log_file_open(const char *fname, const char *mode, const UINT32 tcid, const UINT32 rank, const UINT32 record_limit_enabled, const UINT32 fname_with_date_switch, const UINT32 switch_off_enable, const UINT32 pid_info_enable);

EC_BOOL log_file_close(LOG *log);

LOG *log_cstr_new();

EC_BOOL log_cstr_init(LOG *log);

EC_BOOL log_cstr_clean(LOG *log);

void log_cstr_free(LOG *log);

LOG * log_cstr_open();

EC_BOOL log_cstr_close(LOG *log);

EC_BOOL log_clean(LOG *log);

EC_BOOL log_free(LOG *log);

UINT32 log_init_0(const UINT32 md_id, LOG *log);

UINT32 log_clean_0(const UINT32 md_id, LOG *log);

UINT32 log_free_0(const UINT32 md_id, LOG *log);

#endif/* _LOG_H */

#ifdef __cplusplus
}
#endif/*__cplusplus*/

