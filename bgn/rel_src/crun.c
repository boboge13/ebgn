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

#include <stdio.h>
#include <stdlib.h>

#include "lib_type.h"
#include "lib_cstring.h"
#include "lib_crun.h"
#include "lib_log.h"

UINT32 usr_run_01(const CSTRING *cstring)
{
    sys_log(LOGSTDOUT, "usr_run_01: cstring: %s\n", (char *)cstring_get_str(cstring));
    return (0);
}

UINT32 usr_run_02(const CSTRING *cstring_01, const CSTRING *cstring_02, CSTRING *cstring_03)
{
    sys_log(LOGSTDOUT, "usr_run_02: cstring_01: %s\n", (char *)cstring_get_str(cstring_01));
    sys_log(LOGSTDOUT, "usr_run_02: cstring_02: %s\n", (char *)cstring_get_str(cstring_02));

    cstring_append_cstr(cstring_01, cstring_03);
    cstring_append_cstr(cstring_02, cstring_03);

    return (0);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

