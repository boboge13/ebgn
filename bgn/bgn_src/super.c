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

#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "type.h"
#include "moduleconst.h"

#include "mm.h"
#include "log.h"
#include "debug.h"
#include "cmutex.h"

#include "clist.h"
#include "cvector.h"
#include "cstring.h"

#include "cbc.h"

#include "rank.h"

#include "task.inc"
#include "task.h"
#include "taskcfg.h"
#include "tasks.h"

#include "cmpie.h"
#include "tcnode.h"
#include "super.h"
#include "cproc.h"
#include "cmisc.h"
#include "cload.h"
#include "cbtimer.h"
#include "license.h"

#include "findex.inc"


#define SUPER_MD_CAPACITY()          (cbc_md_capacity(MD_SUPER))

#define SUPER_MD_GET(super_md_id)     ((SUPER_MD *)cbc_md_get(MD_SUPER, (super_md_id)))

#define SUPER_MD_ID_CHECK_INVALID(super_md_id)  \
    ((CMPI_ANY_MODI != (super_md_id)) && ((NULL_PTR == SUPER_MD_GET(super_md_id)) || (0 == (SUPER_MD_GET(super_md_id)->usedcounter))))



static EC_BOOL __super_fnode_clean(SUPER_FNODE *super_fnode);

static EC_BOOL __super_fnode_free(SUPER_FNODE *super_fnode);

static EC_BOOL __super_fnode_match_fname(const SUPER_FNODE *super_fnode, const CSTRING *fname);

static int __super_make_open_flags(const UINT32 open_flags);

/**
*   for test only
*
*   to query the status of SUPER Module
*
**/
void super_print_module_status(const UINT32 super_md_id, LOG *log)
{
    SUPER_MD *super_md;
    UINT32 this_super_md_id;

    for( this_super_md_id = 0; this_super_md_id < SUPER_MD_CAPACITY(); this_super_md_id ++ )
    {
        super_md = SUPER_MD_GET(this_super_md_id);

        if ( NULL_PTR != super_md && 0 < super_md->usedcounter )
        {
            sys_log(log,"SUPER Module # %ld : %ld refered\n",
                    this_super_md_id,
                    super_md->usedcounter);
        }
    }

    return ;
}

/**
*
*   free all static memory occupied by the appointed SUPER module
*
*
**/
UINT32 super_free_module_static_mem(const UINT32 super_md_id)
{
    SUPER_MD  *super_md;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_free_module_static_mem: super module #0x%lx not started.\n",
                super_md_id);
        /*note: here do not exit but return only*/
        return ((UINT32)-1);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    super_md = SUPER_MD_GET(super_md_id);

    free_module_static_mem(MD_SUPER, super_md_id);

    return 0;
}

/**
*
* start super module
*
**/
UINT32 super_start()
{
    SUPER_MD *super_md;
    UINT32 super_md_id;

    super_md_id = cbc_md_new(MD_SUPER, sizeof(SUPER_MD));
    if(ERR_MODULE_ID == super_md_id)
    {
        return (ERR_MODULE_ID);
    }

    /* initilize new one SUPER module */
    super_md = SUPER_MD_GET(super_md_id);
    super_md->usedcounter   = 0;

    /* create a new module node */
    init_static_mem();

    clist_init(SUPER_MD_FNODE_LIST(super_md), MM_IGNORE, LOC_SUPER_0001);
    SUPER_MD_OBJ_ZONE(super_md) = NULL_PTR;
    SUPER_MD_OBJ_ZONE_SIZE(super_md) = 0;

    super_md->usedcounter ++;    

    dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "super_start: start SUPER module #%ld\n", super_md_id);

    return ( super_md_id );
}

/**
*
* end super module
*
**/
void super_end(const UINT32 super_md_id)
{
    SUPER_MD *super_md;

    super_md = SUPER_MD_GET(super_md_id);
    if(NULL_PTR == super_md)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT,"error:super_end: super_md_id = %ld not exist.\n", super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
    /* if the module is occupied by others,then decrease counter only */
    if ( 1 < super_md->usedcounter )
    {
        super_md->usedcounter --;
        return ;
    }

    if ( 0 == super_md->usedcounter )
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT,"error:super_end: super_md_id = %ld is not started.\n", super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }

    /* if nobody else occupied the module,then free its resource */
    clist_clean(SUPER_MD_FNODE_LIST(super_md), (CVECTOR_DATA_CLEANER)__super_fnode_free);
    if(NULL_PTR != SUPER_MD_OBJ_ZONE(super_md))
    {
        EC_BOOL ret;
        cvector_loop(SUPER_MD_OBJ_ZONE(super_md), 
                    (void *)&ret, 
                    CVECTOR_CHECKER_DEFAULT, 
                    2, 
                    0, 
                    (UINT32)cvector_free,
                    NULL_PTR, 
                    LOC_SUPER_0002);
        cvector_free(SUPER_MD_OBJ_ZONE(super_md), LOC_SUPER_0003);
        SUPER_MD_OBJ_ZONE(super_md) = NULL_PTR;
    }
    SUPER_MD_OBJ_ZONE_SIZE(super_md) = 0;

    /* free module : */
    //super_free_module_static_mem(super_md_id);
    super_md->usedcounter = 0;

    dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "super_end: stop SUPER module #%ld\n", super_md_id);
    cbc_md_free(MD_SUPER, super_md_id);

    breathing_static_mem();

    return ;
}

SUPER_FNODE *super_fnode_new(const UINT32 super_md_id)
{
    SUPER_FNODE *super_fnode;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_fnode_new: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    alloc_static_mem(MD_SUPER, super_md_id, MM_SUPER_FNODE, &super_fnode, LOC_SUPER_0004);
    if(NULL_PTR != super_fnode)
    {
        super_fnode_init(super_md_id, super_fnode);
    }
    return (super_fnode);
}

EC_BOOL super_fnode_init(const UINT32 super_md_id, SUPER_FNODE *super_fnode)
{
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_fnode_init: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    cstring_init(SUPER_FNODE_FNAME(super_fnode), NULL_PTR);
    SUPER_FNODE_FD(super_fnode) = ERR_FD;
    SUPER_FNODE_PROGRESS(super_fnode) = 0.0;
    SUPER_FNODE_CMUTEX_INIT(super_fnode, LOC_SUPER_0005);
    return (EC_TRUE);
}

EC_BOOL super_fnode_clean(const UINT32 super_md_id, SUPER_FNODE *super_fnode)
{
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_fnode_clean: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    cstring_clean(SUPER_FNODE_FNAME(super_fnode));

    if(ERR_FD != SUPER_FNODE_FD(super_fnode))
    {
        c_file_close(SUPER_FNODE_FD(super_fnode));
        SUPER_FNODE_FD(super_fnode) = ERR_FD;
    }

    SUPER_FNODE_PROGRESS(super_fnode) = 0.0;
    SUPER_FNODE_CMUTEX_CLEAN(super_fnode, LOC_SUPER_0006);
    return (EC_TRUE);
}

EC_BOOL super_fnode_free(const UINT32 super_md_id, SUPER_FNODE *super_fnode)
{
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_fnode_free: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    if(NULL_PTR != super_fnode)
    {
        super_fnode_clean(super_md_id, super_fnode);
        free_static_mem(MD_SUPER, super_md_id, MM_SUPER_FNODE, super_fnode, LOC_SUPER_0007);
    }
    return (EC_TRUE);
}

static EC_BOOL __super_fnode_clean(SUPER_FNODE *super_fnode)
{
    cstring_clean(SUPER_FNODE_FNAME(super_fnode));

    if(ERR_FD != SUPER_FNODE_FD(super_fnode))
    {
        c_file_close(SUPER_FNODE_FD(super_fnode));
        SUPER_FNODE_FD(super_fnode) = ERR_FD;
    }
    SUPER_FNODE_PROGRESS(super_fnode) = 0;
    SUPER_FNODE_CMUTEX_CLEAN(super_fnode, LOC_SUPER_0008);
    return (EC_TRUE);
}

static EC_BOOL __super_fnode_free(SUPER_FNODE *super_fnode)
{
    if(NULL_PTR != super_fnode)
    {
        __super_fnode_clean(super_fnode);
        free_static_mem(MD_SUPER, CMPI_ANY_MODI, MM_SUPER_FNODE, super_fnode, LOC_SUPER_0009);
    }
    return (EC_TRUE);
}

static EC_BOOL __super_fnode_match_fname(const SUPER_FNODE *super_fnode, const CSTRING *fname)
{
    return cstring_is_equal(SUPER_FNODE_FNAME(super_fnode), fname);
}

static int __super_make_open_flags(const UINT32 open_flags)
{
    int flags;

    flags = 0;

    if(open_flags & SUPER_O_RDONLY)
    {
        flags |= O_RDONLY;
    }

    if(open_flags & SUPER_O_WRONLY)
    {
        flags |= O_WRONLY;
    }

    if(open_flags & SUPER_O_RDWR)
    {
        flags |= O_RDWR;
    }

    if(open_flags & SUPER_O_CREAT)
    {
        flags |= O_CREAT;
    }

    return flags;
}

SUPER_FNODE *super_search_fnode_by_fname_no_lock(const UINT32 super_md_id, const CSTRING *fname)
{
    SUPER_MD *super_md;
    CLIST_DATA *clist_data;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_search_fnode_by_fname_no_lock: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    super_md = SUPER_MD_GET(super_md_id);

    clist_data = clist_search_front_no_lock(SUPER_MD_FNODE_LIST(super_md), (void *)fname, (CLIST_DATA_DATA_CMP)__super_fnode_match_fname);
    if(NULL_PTR == clist_data)
    {
        return (NULL_PTR);
    }
    return (SUPER_FNODE *)CLIST_DATA_DATA(clist_data);
}

SUPER_FNODE *super_search_fnode_by_fname(const UINT32 super_md_id, const CSTRING *fname)
{
    SUPER_MD *super_md;
    CLIST_DATA *clist_data;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_search_fnode_by_fname: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    super_md = SUPER_MD_GET(super_md_id);

    clist_data = clist_search_front_no_lock(SUPER_MD_FNODE_LIST(super_md), (void *)fname, (CLIST_DATA_DATA_CMP)__super_fnode_match_fname);
    if(NULL_PTR == clist_data)
    {
        return (NULL_PTR);
    }
    return (SUPER_FNODE *)CLIST_DATA_DATA(clist_data);
}

SUPER_FNODE *super_open_fnode_by_fname(const UINT32 super_md_id, const CSTRING *fname, const UINT32 open_flags)
{
    SUPER_MD *super_md;
    SUPER_FNODE *super_fnode;
    int fd;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_open_fnode_by_fname: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    super_md = SUPER_MD_GET(super_md_id);

    CLIST_LOCK(SUPER_MD_FNODE_LIST(super_md), LOC_SUPER_0010);

    /*search cached*/
    super_fnode = super_search_fnode_by_fname_no_lock(super_md_id, fname);
    if(NULL_PTR != super_fnode)
    {
        CLIST_UNLOCK(SUPER_MD_FNODE_LIST(super_md), LOC_SUPER_0011);
        return (super_fnode);
    }

    /*open or create*/
    super_fnode = super_fnode_new(super_md_id);
    if(NULL_PTR == super_fnode)
    {
        CLIST_UNLOCK(SUPER_MD_FNODE_LIST(super_md), LOC_SUPER_0012);
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_open_fnode_by_fname: new super fnode failed\n");
        return (NULL_PTR);
    }

    if(open_flags & (SUPER_O_WRONLY | SUPER_O_RDWR | SUPER_O_CREAT))
    {
        if(EC_FALSE == c_basedir_create((char *)cstring_get_str(fname)))
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_open_fnode_by_fname: create basedir of file %s failed\n",
                                (char *)cstring_get_str(fname));
            super_fnode_free(super_md_id, super_fnode);
            return (NULL_PTR);
        }
    }

    fd = c_file_open((char *)cstring_get_str(fname), __super_make_open_flags(open_flags), 0666);
    if(ERR_FD == fd)
    {
        CLIST_UNLOCK(SUPER_MD_FNODE_LIST(super_md), LOC_SUPER_0013);
        super_fnode_free(super_md_id, super_fnode);
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_open_fnode_by_fname: open file %s with flag %lx failed\n",
                            (char *)cstring_get_str(fname), open_flags);
        return (NULL_PTR);
    }

    cstring_clone(fname, SUPER_FNODE_FNAME(super_fnode));
    SUPER_FNODE_FD(super_fnode) = fd;

    clist_push_back_no_lock(SUPER_MD_FNODE_LIST(super_md), (void *)super_fnode);

    CLIST_UNLOCK(SUPER_MD_FNODE_LIST(super_md), LOC_SUPER_0014);

    return (super_fnode);
}

EC_BOOL super_close_fnode_by_fname(const UINT32 super_md_id, const CSTRING *fname)
{
    SUPER_MD *super_md;
    SUPER_FNODE *super_fnode;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_close_fnode_by_fname: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    super_md = SUPER_MD_GET(super_md_id);

    super_fnode = (SUPER_FNODE *)clist_del(SUPER_MD_FNODE_LIST(super_md), (void *)fname, (CLIST_DATA_DATA_CMP)__super_fnode_match_fname);
    if(NULL_PTR == super_fnode)
    {
        return (EC_TRUE);
    }
    return super_fnode_free(super_md_id, super_fnode);
}

/**
*
* include taskc node info to SUPER module
*
**/
UINT32 super_incl_taskc_node(const UINT32 super_md_id, const UINT32 ipaddr, const UINT32 port, const int sockfd, const UINT32 taskc_id, const UINT32 taskc_comm, const UINT32 taskc_size)
{
    TASK_BRD  *task_brd;
    TASKS_CFG *tasks_cfg;

    CSOCKET_CNODE *csocket_cnode;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_incl_taskc_node: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();

    tasks_cfg = TASK_BRD_TASKS_CFG(task_brd);

    if(do_log(SEC_0117_SUPER, 5))
    {
        sys_log(LOGSTDOUT, "============================== super_incl_taskc_node: before ==============================\n");
        super_show_work_client(super_md_id, LOGSTDOUT);
    }

    csocket_cnode = csocket_cnode_new(taskc_id, sockfd, ipaddr, port);
    CSOCKET_CNODE_COMM(csocket_cnode) = taskc_comm;
    CSOCKET_CNODE_SIZE(csocket_cnode) = taskc_size;

    tasks_work_add_csocket_cnode(TASKS_CFG_WORK(tasks_cfg), csocket_cnode);

    if(do_log(SEC_0117_SUPER, 5))
    {
        sys_log(LOGSTDOUT, "============================== super_incl_taskc_node: after ==============================\n");
        super_show_work_client(super_md_id, LOGSTDOUT);
    }

    return (0);
}

/**
*
* exclude taskc node info to SUPER module
*
**/
UINT32 super_excl_taskc_node(const UINT32 super_md_id, const UINT32 tcid, const UINT32 comm)
{
    TASK_BRD   *task_brd;
    TASKS_CFG  *tasks_cfg;
    CVECTOR    *tasks_node_work;
    UINT32      pos;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_excl_taskc_node: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();

    tasks_cfg = TASK_BRD_TASKS_CFG(task_brd);
    tasks_node_work = TASKS_CFG_WORK(tasks_cfg);

    if(do_log(SEC_0117_SUPER, 5))
    {
        sys_log(LOGSTDOUT, "============================== super_excl_taskc_node: before ==============================\n");
        //cvector_print(LOGSTDOUT, TASKS_WORK_CLIENTS(tasks_cfg), (CVECTOR_DATA_PRINT)csocket_cnode_print);
        super_show_work_client(super_md_id, LOGSTDOUT);
    }

    CVECTOR_LOCK(tasks_node_work, LOC_SUPER_0015);
    for(pos = 0; pos < cvector_size(tasks_node_work); /*pos ++*/)
    {
        TASKS_NODE *tasks_node;

        tasks_node = (TASKS_NODE *)cvector_get_no_lock(tasks_node_work, pos);
        if(NULL_PTR == tasks_node)
        {
            cvector_erase_no_lock(tasks_node_work, pos);
            continue;
        }

        if((CMPI_ANY_TCID == tcid || TASKS_NODE_TCID(tasks_node) == tcid)
         &&(CMPI_ANY_COMM == comm || TASKS_NODE_COMM(tasks_node) == comm))
        {
            cvector_erase_no_lock(tasks_node_work, pos);
            tasks_node_free(tasks_node);
            continue;
        }

        pos ++;
    }
    CVECTOR_UNLOCK(tasks_node_work, LOC_SUPER_0016);

    if(do_log(SEC_0117_SUPER, 5))
    {
        sys_log(LOGSTDOUT, "============================== super_excl_taskc_node: after ==============================\n");
        super_show_work_client(super_md_id, LOGSTDOUT);
    }

    return (0);
}

/**
*
* init taskc node mgr in SUPER module
*
**/
UINT32 super_init_taskc_mgr(const UINT32 super_md_id, TASKC_MGR *taskc_mgr)
{
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_init_taskc_mgr: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    taskc_mgr_init(taskc_mgr);
    return (0);
}

/**
*
* clean taskc node mgr in SUPER module
*
**/
UINT32 super_clean_taskc_mgr(const UINT32 super_md_id, TASKC_MGR *taskc_mgr)
{
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_clean_taskc_mgr: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    taskc_mgr_clean(taskc_mgr);
    return (0);
}

/**
*
* free taskc node mgr in SUPER module
*
**/
UINT32 super_free_taskc_mgr(const UINT32 super_md_id, TASKC_MGR *taskc_mgr)
{
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_free_taskc_mgr: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    taskc_mgr_free(taskc_mgr);
    return (0);
}


/**
*
* sync taskc node mgr info by SUPER module
*
**/
UINT32 super_sync_taskc_mgr(const UINT32 super_md_id, TASKC_MGR *des_taskc_mgr)
{
    TASK_BRD   *task_brd;
    TASKS_CFG  *tasks_cfg;
    CVECTOR    *tasks_node_work;
    UINT32      tasks_node_pos;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_sync_taskc_mgr: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    if(NULL_PTR == des_taskc_mgr)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_sync_taskc_mgr: des_taskc_mgr is null ptr\n");
        dbg_exit(MD_SUPER, super_md_id);
    }

    task_brd = task_brd_default_get();

    tasks_cfg  = TASK_BRD_TASKS_CFG(task_brd);
    tasks_node_work = TASKS_CFG_WORK(tasks_cfg);

    CVECTOR_LOCK(tasks_node_work, LOC_SUPER_0017);
    for(tasks_node_pos = 0; tasks_node_pos < cvector_size(tasks_node_work); tasks_node_pos ++)
    {
        TASKS_NODE *tasks_node;
        TASKC_NODE *taskc_node;

        tasks_node = (TASKS_NODE *)cvector_get_no_lock(tasks_node_work, tasks_node_pos);
        if(NULL_PTR == tasks_node)
        {
            continue;
        }

        dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "super_sync_taskc_mgr: tasks node: tcid %s, comm %ld, size %ld\n",
                            TASKS_NODE_TCID_STR(tasks_node), TASKS_NODE_COMM(tasks_node), TASKS_NODE_SIZE(tasks_node));

        if(0 == TASKS_NODE_COMM(tasks_node))
        {
            continue;
        }

        taskc_node_new(&taskc_node);

        TASKC_NODE_TCID(taskc_node) = TASKS_NODE_TCID(tasks_node);
        TASKC_NODE_COMM(taskc_node) = TASKS_NODE_COMM(tasks_node);
        TASKC_NODE_SIZE(taskc_node) = TASKS_NODE_SIZE(tasks_node);

        /*if duplicate, give up pushing to list*/
        if(NULL_PTR != clist_search_front(TASKC_MGR_NODE_LIST(des_taskc_mgr), (void *)taskc_node, (CLIST_DATA_DATA_CMP)taskc_node_cmp_tcid))
        {
            taskc_node_free(taskc_node);
            continue;/*give up*/
        }

        clist_push_back(TASKC_MGR_NODE_LIST(des_taskc_mgr), (void *)taskc_node);
    }
    CVECTOR_UNLOCK(tasks_node_work, LOC_SUPER_0018);

    return (0);
}

UINT32 super_sync_cload_mgr0(const UINT32 super_md_id, const UINT32 type, CLOAD_MGR *des_cload_mgr)
{
    TASK_BRD   *task_brd;
    TASKS_CFG  *tasks_cfg;
    CVECTOR    *tasks_node_work;
    UINT32      tasks_node_pos;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_sync_cload_mgr0: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    if(NULL_PTR == des_cload_mgr)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_sync_cload_mgr: des_taskc_mgr is null ptr\n");
        dbg_exit(MD_SUPER, super_md_id);
    }

    task_brd = task_brd_default_get();

    tasks_cfg  = TASK_BRD_TASKS_CFG(task_brd);
    tasks_node_work = TASKS_CFG_WORK(tasks_cfg);

    CVECTOR_LOCK(tasks_node_work, LOC_SUPER_0019);
    for(tasks_node_pos = 0; tasks_node_pos < cvector_size(tasks_node_work); tasks_node_pos ++)
    {
        TASKS_NODE *tasks_node;
        CLOAD_NODE *cload_node;

        tasks_node = (TASKS_NODE *)cvector_get_no_lock(tasks_node_work, tasks_node_pos);
        if(NULL_PTR == tasks_node)
        {
            continue;
        }

        dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "super_sync_cload_mgr: tasks node: tcid %s, comm %ld, size %ld\n",
                            TASKS_NODE_TCID_STR(tasks_node), TASKS_NODE_COMM(tasks_node), TASKS_NODE_SIZE(tasks_node));

        if(0 == TASKS_NODE_COMM(tasks_node))
        {
            continue;
        }

        cload_node = cload_node_new(TASKS_NODE_TCID(tasks_node), TASKS_NODE_COMM(tasks_node), TASKS_NODE_SIZE(tasks_node));
        if(do_log(SEC_0117_SUPER, 9))
        {
            sys_log(LOGSTDOUT, "[DEBUG] super_sync_cload_mgr: new cload_node is\n");
            cload_node_print(LOGSTDOUT, cload_node);
        }

        /*if duplicate, give up pushing to list*/
        if(NULL_PTR != clist_search_front(des_cload_mgr, (void *)cload_node, (CLIST_DATA_DATA_CMP)cload_node_cmp_tcid))
        {
            cload_node_free(cload_node);
            continue;/*give up*/
        }

        task_brd_sync_cload_node(task_brd, cload_node);
        clist_push_back(des_cload_mgr, (void *)cload_node);
    }
    CVECTOR_UNLOCK(tasks_node_work, LOC_SUPER_0020);

    if(do_log(SEC_0117_SUPER, 9))
    {
        sys_log(LOGSTDOUT, "[DEBUG] super_sync_cload_mgr: des_cload_mgr is\n");
        cload_mgr_print(LOGSTDOUT, des_cload_mgr);
    }
    return (0);
}

UINT32 super_sync_cload_mgr(const UINT32 super_md_id, const CVECTOR *tcid_vec, CLOAD_MGR *des_cload_mgr)
{
    TASK_BRD    *task_brd;
    TASKS_CFG   *tasks_cfg;
    CVECTOR     *tasks_node_work;
    UINT32       tasks_node_pos;

    TASK_MGR   *task_mgr;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_sync_cload_mgr: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    if(NULL_PTR == des_cload_mgr)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_sync_cload_mgr: des_taskc_mgr is null ptr\n");
        dbg_exit(MD_SUPER, super_md_id);
    }

    task_brd = task_brd_default_get();

    tasks_cfg  = TASK_BRD_TASKS_CFG(task_brd);
    tasks_node_work = TASKS_CFG_WORK(tasks_cfg);

    task_mgr = task_new(NULL, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);

    CVECTOR_LOCK(tasks_node_work, LOC_SUPER_0021);
    for(tasks_node_pos = 0; tasks_node_pos < cvector_size(tasks_node_work); tasks_node_pos ++)
    {
        TASKS_NODE *tasks_node;
        CLOAD_NODE *cload_node;
        MOD_NODE    recv_mod_node;

        tasks_node = (TASKS_NODE *)cvector_get_no_lock(tasks_node_work, tasks_node_pos);
        if(NULL_PTR == tasks_node)
        {
            continue;
        }

        if(0 == TASKS_NODE_COMM(tasks_node))
        {
            continue;
        }

        if(CVECTOR_ERR_POS == cvector_search_front_no_lock(tcid_vec, (void *)TASKS_NODE_TCID(tasks_node), NULL_PTR))
        {
            continue;
        }
        
        dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "super_sync_cload_mgr: tasks node: tcid %s, comm %ld, size %ld\n",
                            TASKS_NODE_TCID_STR(tasks_node), TASKS_NODE_COMM(tasks_node), TASKS_NODE_SIZE(tasks_node));
        
        cload_node = cload_node_new(TASKS_NODE_TCID(tasks_node), TASKS_NODE_COMM(tasks_node), TASKS_NODE_SIZE(tasks_node));
        //dbg_log(SEC_0117_SUPER, 9)(LOGSTDOUT, "[DEBUG] super_sync_cload_mgr: new cload_node is\n");
        //cload_node_print(LOGSTDOUT, cload_node);

        /*if duplicate, give up pushing to list*/
        if(NULL_PTR != clist_search_front(des_cload_mgr, (void *)cload_node, (CLIST_DATA_DATA_CMP)cload_node_cmp_tcid))
        {
            cload_node_free(cload_node);
            continue;/*give up*/
        }

        MOD_NODE_TCID(&recv_mod_node) = TASKS_NODE_TCID(tasks_node);
        MOD_NODE_COMM(&recv_mod_node) = TASKS_NODE_COMM(tasks_node);
        MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
        MOD_NODE_MODI(&recv_mod_node) = 0;

        task_p2p_inc(task_mgr, super_md_id, &recv_mod_node, NULL_PTR, FI_super_sync_cload_node, ERR_MODULE_ID, cload_node);

        //task_brd_sync_cload_node(task_brd, cload_node);
        clist_push_back(des_cload_mgr, (void *)cload_node);
    }
    CVECTOR_UNLOCK(tasks_node_work, LOC_SUPER_0022);

    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    //dbg_log(SEC_0117_SUPER, 9)(LOGSTDOUT, "[DEBUG] super_sync_cload_mgr: des_cload_mgr is\n");
    //cload_mgr_print(LOGSTDOUT, des_cload_mgr);

    return (0);
}

EC_BOOL super_register_hsbgt_cluster(const UINT32 super_md_id)
{
    dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_register_hsbgt_cluster: obsoleted!\n");
    return (EC_FALSE);
}

EC_BOOL super_register_hsdfs_cluster(const UINT32 super_md_id)
{
    dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_register_hsdfs_cluster: obsoleted!\n");
    return (EC_FALSE);
}

/**
*
* check taskc node connectivity by SUPER module
*
**/
EC_BOOL super_check_tcid_connected(const UINT32 super_md_id, const UINT32 tcid)
{
    TASK_BRD  *task_brd;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_check_tcid_connected: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();

    if(tcid == TASK_BRD_TCID(task_brd))
    {
        return (EC_TRUE);
    }

    if(CMPI_FWD_RANK == TASK_BRD_RANK(task_brd))
    {
        TASKS_CFG *tasks_cfg;
        tasks_cfg = TASK_BRD_TASKS_CFG(task_brd);
        return tasks_work_check_connected_by_tcid(TASKS_CFG_WORK(tasks_cfg), tcid);
    }

    return (EC_FALSE);
}

/**
*
* check taskc node connectivity by SUPER module
*
**/
EC_BOOL super_check_ipaddr_connected(const UINT32 super_md_id, const UINT32 ipaddr)
{
    TASK_BRD  *task_brd;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_check_ipaddr_connected: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();

    if(ipaddr == TASK_BRD_IPADDR(task_brd))
    {
        return (EC_TRUE);
    }

    if(CMPI_FWD_RANK == TASK_BRD_RANK(task_brd))
    {
        TASKS_CFG *tasks_cfg;
        tasks_cfg = TASK_BRD_TASKS_CFG(task_brd);
        return tasks_work_check_connected_by_ipaddr(TASKS_CFG_WORK(tasks_cfg), ipaddr);
    }

    return (EC_FALSE);
}


/**
*
* activate sysconfig
* import from config.xml
* note: only add new info but never delete or override the old ones
*
**/
void super_activate_sys_cfg(const UINT32 super_md_id)
{
    TASK_BRD *task_brd;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_activate_sys_cfg: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();

    dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "super_activate_sys_cfg: activate sysconfig from %s\n", (char *)task_brd_default_sys_cfg_xml());
    dbg_log(SEC_0117_SUPER, 9)(LOGSTDOUT, "[DEBUG] super_activate_sys_cfg: load sys cfg ---------------------------------------------------\n");
    sys_cfg_load(TASK_BRD_SYS_CFG(task_brd), (char *)task_brd_default_sys_cfg_xml());

    dbg_log(SEC_0117_SUPER, 9)(LOGSTDOUT, "[DEBUG] super_activate_sys_cfg: import cparacfg ---------------------------------------------------\n");
    log_level_import(CPARACFG_LOG_LEVEL_TAB(TASK_BRD_CPARACFG(task_brd)), SEC_NONE_END);
    //log_level_print(LOGSTDOUT);

    if(CMPI_FWD_RANK == TASK_BRD_RANK(task_brd))
    {
        task_brd_register_cluster(task_brd);
    }
    return;
}

/**
*
* show current sysconfig
*
**/
void super_show_sys_cfg(const UINT32 super_md_id, LOG *log)
{
    TASK_BRD *task_brd;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_show_sys_cfg: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();

    sys_cfg_print_xml(log, TASK_BRD_SYS_CFG(task_brd), 0);
    return;
}

/**
*
* print mem statistics info of current process
*
**/
void super_show_mem(const UINT32 super_md_id, LOG *log)
{
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_show_mem: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    //dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "===================================== memory statistics info beg: log %lx =====================================\n", log);
    print_static_mem_status(log);
    //print_static_mem_status(LOGSTDOUT);
    //dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "===================================== memory statistics info end: =====================================\n");
    return;
}

/**
*
* print mem statistics info of current process
*
**/
void super_show_mem_of_type(const UINT32 super_md_id, const UINT32 type, LOG *log)
{
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_show_mem_of_type: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    //dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "===================================== memory statistics info of type %ld beg: log %lx =====================================\n", type, log);
    print_static_mem_status_of_type(log, type);
    //dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "===================================== memory statistics info end: =====================================\n");
    return;
}


/**
*
* diagnostic mem of current process
*
**/
void super_diag_mem(const UINT32 super_md_id, LOG *log)
{
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_diag_mem: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    //dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "===================================== memory diagnostic info beg: =====================================\n");
    print_static_mem_diag_info(log);
    //dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "===================================== memory diagnostic info end: =====================================\n");
    return;
}

/**
*
* diagnostic mem of current process
*
**/
void super_diag_mem_of_type(const UINT32 super_md_id, const UINT32 type, LOG *log)
{
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_diag_mem_of_type: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    //dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "===================================== memory diagnostic info beg: =====================================\n");
    print_static_mem_diag_info_of_type(log, type);
    //dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "===================================== memory diagnostic info end: =====================================\n");
    return;
}

/**
*
* clean mem of current process
*
**/
void super_clean_mem(const UINT32 super_md_id)
{
    TASK_BRD *task_brd;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_clean_mem: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();
    task_brd_clean(task_brd);
    return;
}

/**
*
* breathe mem of current process
*
**/
void super_breathing_mem(const UINT32 super_md_id)
{
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_breathing_mem: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    breathing_static_mem();
    return;
}

/**
*
* show log level info
*
**/
void super_show_log_level_tab(const UINT32 super_md_id, LOG *log)
{
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_show_log_level_tab: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    log_level_print(log);
    return;
}

/**
*
* set log level
*
**/
EC_BOOL super_set_log_level_tab(const UINT32 super_md_id, const UINT32 level)
{
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_set_log_level_tab: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    return log_level_set_all(level);
}

/**
*
* set log level of sector
*
**/
EC_BOOL super_set_log_level_sector(const UINT32 super_md_id, const UINT32 sector, const UINT32 level)
{
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_set_log_level_sector: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    return log_level_set_sector(sector, level);
}


/**
*
* shutdown current taskComm
*
**/
void super_shutdown_taskcomm(const UINT32 super_md_id)
{
    TASK_BRD *task_brd;
    LOG *log_stdout;
    LOG *log_stderr;
    LOG *log_stdin;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_shutdown_taskcomm: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();

    //TASK_BRD_ENABLE_FLAG(task_brd) = EC_FALSE;
    TASK_BRD_RESET_FLAG(task_brd)     = EC_FALSE; /*disable do_slave reset*/
    TASK_BRD_ABORT_FLAG(task_brd)     = CPROC_IS_ABORTED;

    if(CMPI_FWD_RANK == TASK_BRD_RANK(task_brd))
    {
        /*when stop TASKC, all packets in forwarding process will be unreachable to remote*/
        tasks_srv_end(TASK_BRD_TASKS_CFG(task_brd));
    }

    cbc_free();/*cbc_new is called in task_brd_default_init*/

    if(NULL_PTR != TASK_BRD_CSRV(task_brd))
    {
        csrv_end(TASK_BRD_CSRV(task_brd));
        TASK_BRD_CSRV(task_brd) = NULL_PTR;
    }

    task_brd_free(task_brd);

    log_stdout = sys_log_redirect_cancel(LOGSTDOUT);
    log_stderr = sys_log_redirect_cancel(LOGSTDERR);
    log_stdin  = sys_log_redirect_cancel(LOGSTDIN);

    if(0 != log_stdout)
    {
        log_free(log_stdout);
    }

    if(0 != log_stderr && log_stderr != log_stdout)
    {
        log_free(log_stderr);
    }

    if(0 != log_stdin && log_stdin != log_stderr && log_stdin != log_stdout)
    {
        log_free(log_stdin);
    }

    task_brd_default_abort();

    return;
}

EC_BOOL super_cancel_task_req(const UINT32 super_md_id, const UINT32 seqno, const UINT32 subseqno, const MOD_NODE *recv_mod_node)
{
    TASK_BRD   *task_brd;
    TASK_MGR   *task_mgr;
    TASK_REQ   *task_req;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_cancel_task_req: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();

    task_mgr = (TASK_MGR *)clist_search_data_front(TASK_BRD_RECV_TASK_MGR_LIST(task_brd), (void *)seqno, (CLIST_DATA_DATA_CMP)task_mgr_match_seqno);
    if(NULL_PTR == task_mgr)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_cancel_task_req: not found task_mgr with seqno %lx\n", seqno);
        return (EC_FALSE);
    }

    task_req = task_mgr_search_task_req_by_recver(task_mgr, seqno, subseqno, recv_mod_node);
    if(NULL_PTR == task_req)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_cancel_task_req: not found task_req with seqno %lx, subseqno %lx to (tcid %s,comm %ld,rank %ld,modi %ld)\n",
                            seqno, subseqno,
                            MOD_NODE_TCID_STR(recv_mod_node), MOD_NODE_COMM(recv_mod_node), MOD_NODE_RANK(recv_mod_node), MOD_NODE_MODI(recv_mod_node));
        return (EC_FALSE);
    }

    TASK_NODE_CMUTEX_LOCK(TASK_REQ_NODE(task_req), LOC_SUPER_0023);
    TASK_NODE_STATUS(TASK_REQ_NODE(task_req)) = TASK_REQ_DISCARD;
    TASK_MGR_COUNTER_INC_BY_TASK_REQ(TASK_REQ_MGR(task_req), TASK_MGR_COUNTER_TASK_REQ_DISCARD, task_req, LOC_SUPER_0024);
    TASK_NODE_CMUTEX_UNLOCK(TASK_REQ_NODE(task_req), LOC_SUPER_0025);

    return (EC_TRUE);
}

/**
*
* sync load info of current rank
*
**/
void super_sync_cload_stat(const UINT32 super_md_id, CLOAD_STAT *cload_stat)
{
    TASK_BRD *task_brd;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_sync_cload_stat: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();
    task_brd_cload_stat_update_once(task_brd);
    cload_stat_clone(TASK_BRD_CLOAD_STAT(task_brd), cload_stat);

    return ;
}

/**
*
* sync load info of current comm
*
**/
void super_sync_cload_node(const UINT32 super_md_id, CLOAD_NODE *cload_node)
{
    TASK_BRD *task_brd;
    TASK_MGR *task_mgr;

    MOD_NODE  send_mod_node;
    MOD_NODE  recv_mod_node;

    UINT32    rank;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_sync_cload_node: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();
    if(CMPI_FWD_RANK != TASK_BRD_RANK(task_brd))
    {
        MOD_NODE_TCID(&recv_mod_node) = CMPI_LOCAL_TCID;
        MOD_NODE_COMM(&recv_mod_node) = CMPI_LOCAL_COMM;
        MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
        MOD_NODE_MODI(&recv_mod_node) = 0;

        task_p2p(super_md_id, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, &recv_mod_node,
                 NULL_PTR, FI_super_sync_cload_node, ERR_MODULE_ID, cload_node);
        return;
    }

    mod_node_init(&send_mod_node);
    MOD_NODE_TCID(&send_mod_node) = CMPI_LOCAL_TCID;
    MOD_NODE_COMM(&send_mod_node) = CMPI_LOCAL_COMM;
    MOD_NODE_RANK(&send_mod_node) = CMPI_LOCAL_RANK;
    MOD_NODE_MODI(&send_mod_node) = super_md_id;

    task_mgr = task_new(NULL, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    for(rank = 0; rank < TASK_BRD_SIZE(task_brd); rank ++)
    {
        CLOAD_STAT *cload_stat;

        MOD_NODE_TCID(&recv_mod_node) = CMPI_LOCAL_TCID;
        MOD_NODE_COMM(&recv_mod_node) = CMPI_LOCAL_COMM;
        MOD_NODE_RANK(&recv_mod_node) = rank;
        MOD_NODE_MODI(&recv_mod_node) = 0;

        cload_stat = cload_node_get(cload_node, rank);
        task_p2p_inc(task_mgr, super_md_id, &recv_mod_node, NULL_PTR, FI_super_sync_cload_stat, ERR_MODULE_ID, cload_stat);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    return ;
}

/*TODO: sync other ranks from fwd rank*/
static void super_sync_taskcomm_self(CVECTOR *collected_vec, TASK_MGR *task_mgr, MOD_NODE *send_mod_node)
{
    TASK_BRD *task_brd;
    CVECTOR  *new_mod_node_vec;
    UINT32 rank;

    task_brd = task_brd_default_get();

    if(CMPI_FWD_RANK != TASK_BRD_RANK(task_brd)
    || MOD_NODE_TCID(send_mod_node) != TASK_BRD_TCID(task_brd) || MOD_NODE_COMM(send_mod_node) != TASK_BRD_COMM(task_brd) || MOD_NODE_RANK(send_mod_node) || TASK_BRD_RANK(task_brd))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_sync_taskcomm_self: FOR LOCAL FWD RANK ONLY!\n");
        return;
    }

    new_mod_node_vec = cvector_new(0, MM_MOD_NODE, LOC_SUPER_0026);
    cvector_push_no_lock(collected_vec, (void *)new_mod_node_vec);

    for(rank = 0; rank < TASK_BRD_SIZE(task_brd); rank ++)
    {
        MOD_NODE *recv_mod_node;

        recv_mod_node = mod_node_new();
        cvector_push(new_mod_node_vec, (void *)recv_mod_node);

        MOD_NODE_TCID(recv_mod_node) = TASK_BRD_TCID(task_brd);
        MOD_NODE_COMM(recv_mod_node) = TASK_BRD_COMM(task_brd);
        MOD_NODE_RANK(recv_mod_node) = rank;/*des rank*/
        MOD_NODE_MODI(recv_mod_node) = 0;
        MOD_NODE_HOPS(recv_mod_node) = 0;
        MOD_NODE_LOAD(recv_mod_node) = 0;

        task_super_inc(task_mgr, send_mod_node, recv_mod_node,
                        NULL_PTR, FI_super_sync_cload_stat, ERR_MODULE_ID, MOD_NODE_CLOAD_STAT(recv_mod_node));
    }
    return;
}

/*TODO: search intranet of local_tasks_cfg with (local tcid, local maski, local maske)*/
static void super_sync_taskcomm_intranet(CVECTOR *collected_vec, CVECTOR *remote_tasks_cfg_vec, TASKS_CFG *local_tasks_cfg, const UINT32 max_hops, const UINT32 max_remotes, const UINT32 time_to_live, TASK_MGR *task_mgr, MOD_NODE *send_mod_node)
{
    UINT32 pos;

    CVECTOR_LOCK(remote_tasks_cfg_vec, LOC_SUPER_0027);
    for(pos = 0; pos < cvector_size(remote_tasks_cfg_vec); pos ++)
    {
        TASKS_CFG *remote_tasks_cfg;
        CVECTOR *new_mod_node_vec;
        MOD_NODE recv_mod_node;

        remote_tasks_cfg = (TASKS_CFG *)cvector_get_no_lock(remote_tasks_cfg_vec, pos);
        if(NULL_PTR == remote_tasks_cfg)
        {
            continue;
        }

        if(local_tasks_cfg == remote_tasks_cfg)
        {
            continue;
        }

        if(! DES_TCID_IS_INTRANET(TASKS_CFG_TCID(local_tasks_cfg), TASKS_CFG_MASKI(local_tasks_cfg), TASKS_CFG_TCID(remote_tasks_cfg), TASKS_CFG_MASKE(remote_tasks_cfg)))
        {
            continue;
        }

        if(EC_FALSE == tasks_work_check_connected_by_tcid(TASKS_CFG_WORK(local_tasks_cfg), TASKS_CFG_TCID(remote_tasks_cfg)))
        {
            continue;
        }


        new_mod_node_vec = cvector_new(0, MM_MOD_NODE, LOC_SUPER_0028);
        cvector_push_no_lock(collected_vec, (void *)new_mod_node_vec);

        MOD_NODE_TCID(&recv_mod_node) = TASKS_CFG_TCID(remote_tasks_cfg);
        MOD_NODE_COMM(&recv_mod_node) = CMPI_ANY_COMM;
        MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
        MOD_NODE_MODI(&recv_mod_node) = 0;
        MOD_NODE_HOPS(&recv_mod_node) = 0;
        MOD_NODE_LOAD(&recv_mod_node) = 0;

        task_super_inc(task_mgr, send_mod_node, &recv_mod_node,
                        NULL_PTR, FI_super_sync_taskcomm, ERR_MODULE_ID,
                        TASKS_CFG_TCID(local_tasks_cfg), TASKS_CFG_MASKI(local_tasks_cfg), TASKS_CFG_MASKE(local_tasks_cfg),
                        max_hops, max_remotes, time_to_live,
                        new_mod_node_vec);
    }
    CVECTOR_UNLOCK(remote_tasks_cfg_vec, LOC_SUPER_0029);

    return;
}

/*TODO: search intranet of loca_tasks_cfg with (local tcid, local maski, local maske)*/
static void super_sync_taskcomm_lannet(CVECTOR *collected_vec, CVECTOR *remote_tasks_cfg_vec, TASKS_CFG *local_tasks_cfg, const UINT32 max_hops, const UINT32 max_remotes, const UINT32 time_to_live, TASK_MGR *task_mgr, MOD_NODE *send_mod_node)
{
    UINT32 pos;

    CVECTOR_LOCK(remote_tasks_cfg_vec, LOC_SUPER_0030);
    for(pos = 0; pos < cvector_size(remote_tasks_cfg_vec); pos ++)
    {
        TASKS_CFG *remote_tasks_cfg;
        CVECTOR *new_mod_node_vec;
        MOD_NODE recv_mod_node;

        remote_tasks_cfg = (TASKS_CFG *)cvector_get_no_lock(remote_tasks_cfg_vec, pos);
        if(NULL_PTR == remote_tasks_cfg)
        {
            continue;
        }

        if(local_tasks_cfg == remote_tasks_cfg)
        {
            continue;
        }

        if(! DES_TCID_IS_INTRANET(TASKS_CFG_TCID(local_tasks_cfg), TASKS_CFG_MASKI(local_tasks_cfg), TASKS_CFG_TCID(remote_tasks_cfg), TASKS_CFG_MASKE(remote_tasks_cfg)))
        {
            continue;
        }

        if(EC_FALSE == tasks_work_check_connected_by_tcid(TASKS_CFG_WORK(local_tasks_cfg), TASKS_CFG_TCID(remote_tasks_cfg)))
        {
            continue;
        }

        new_mod_node_vec = cvector_new(0, MM_MOD_NODE, LOC_SUPER_0031);
        cvector_push_no_lock(collected_vec, (void *)new_mod_node_vec);

        MOD_NODE_TCID(&recv_mod_node) = TASKS_CFG_TCID(remote_tasks_cfg);
        MOD_NODE_COMM(&recv_mod_node) = CMPI_ANY_COMM;
        MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
        MOD_NODE_MODI(&recv_mod_node) = 0;
        MOD_NODE_HOPS(&recv_mod_node) = 0;
        MOD_NODE_LOAD(&recv_mod_node) = 0;

        task_super_inc(task_mgr, send_mod_node, &recv_mod_node,
                        NULL_PTR, FI_super_sync_taskcomm, ERR_MODULE_ID,
                        TASKS_CFG_TCID(local_tasks_cfg), TASKS_CFG_MASKI(local_tasks_cfg), TASKS_CFG_MASKE(local_tasks_cfg),
                        max_hops, max_remotes, time_to_live,
                        new_mod_node_vec);
    }
    CVECTOR_UNLOCK(remote_tasks_cfg_vec, LOC_SUPER_0032);
    return;
}

/*TODO: search lannet and extranet of local_tasks_cfg with (local tcid, local maski, local maske)*/
static void super_sync_taskcomm_externet(CVECTOR *collected_vec, CVECTOR *remote_tasks_cfg_vec, TASKS_CFG *local_tasks_cfg, const UINT32 max_hops, const UINT32 max_remotes, const UINT32 time_to_live, TASK_MGR *task_mgr, MOD_NODE *send_mod_node)
{
    UINT32 pos;

    CVECTOR_LOCK(remote_tasks_cfg_vec, LOC_SUPER_0033);
    for(pos = 0; pos < cvector_size(remote_tasks_cfg_vec); pos ++)
    {
        TASKS_CFG *remote_tasks_cfg;
        CVECTOR *new_mod_node_vec;
        MOD_NODE recv_mod_node;

        remote_tasks_cfg = (TASKS_CFG *)cvector_get_no_lock(remote_tasks_cfg_vec, pos);
        if(NULL_PTR == remote_tasks_cfg)
        {
            continue;
        }

        if(local_tasks_cfg == remote_tasks_cfg)
        {
            continue;
        }

        if(
            (! DES_TCID_IS_LANNET(TASKS_CFG_TCID(local_tasks_cfg), TASKS_CFG_MASKE(local_tasks_cfg), TASKS_CFG_TCID(remote_tasks_cfg), TASKS_CFG_MASKE(remote_tasks_cfg)))
        &&
            (! DES_TCID_IS_EXTERNET(TASKS_CFG_TCID(local_tasks_cfg), TASKS_CFG_MASKE(local_tasks_cfg), TASKS_CFG_TCID(remote_tasks_cfg), TASKS_CFG_MASKI(remote_tasks_cfg)))
        )
        {
            continue;
        }

        if(EC_FALSE == tasks_work_check_connected_by_tcid(TASKS_CFG_WORK(local_tasks_cfg), TASKS_CFG_TCID(remote_tasks_cfg)))
        {
            continue;
        }

        new_mod_node_vec = cvector_new(0, MM_MOD_NODE, LOC_SUPER_0034);
        cvector_push_no_lock(collected_vec, (void *)new_mod_node_vec);

        MOD_NODE_TCID(&recv_mod_node) = TASKS_CFG_TCID(remote_tasks_cfg);
        MOD_NODE_COMM(&recv_mod_node) = CMPI_ANY_COMM;
        MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
        MOD_NODE_MODI(&recv_mod_node) = 0;
        MOD_NODE_HOPS(&recv_mod_node) = 0;
        MOD_NODE_LOAD(&recv_mod_node) = 0;

        task_super_inc(task_mgr, send_mod_node, &recv_mod_node,
                        NULL_PTR, FI_super_sync_taskcomm, ERR_MODULE_ID,
                        TASKS_CFG_TCID(local_tasks_cfg), TASKS_CFG_MASKI(local_tasks_cfg), TASKS_CFG_MASKE(local_tasks_cfg),
                        max_hops, max_remotes, time_to_live,
                        new_mod_node_vec);
    }
    CVECTOR_UNLOCK(remote_tasks_cfg_vec, LOC_SUPER_0035);
    return;
}

/**
*
* sync from remote taskcomms and the load info
*
* note: here the last cvector para always IO, otherwise the remote peer would have no idea its elements mem type when encoding rsp
*
**/
void super_sync_taskcomm(const UINT32 super_md_id, const UINT32 src_tcid, const UINT32 src_maski, const UINT32 src_maske, const UINT32 max_hops, const UINT32 max_remotes, const UINT32 time_to_live, CVECTOR *mod_node_vec)
{
    TASK_BRD  *task_brd;
    TASK_CFG  *local_task_cfg;
    TASKS_CFG *local_tasks_cfg;

    CVECTOR  *remote_tasks_cfg_vec;
    CVECTOR  *collected_vec;
    TASK_MGR *task_mgr;

    MOD_NODE send_mod_node;

    UINT32   pos;
    UINT32   mod_node_pos_with_max_hops;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_sync_taskcomm: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();

    if(CMPI_FWD_RANK != TASK_BRD_RANK(task_brd))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_sync_taskcomm: FOR FWD RANK ONLY!\n");
        return;
    }

#if 0
    if(CMPI_FWD_RANK != TASK_BRD_RANK(task_brd))
    {
        MOD_NODE *mod_node;

        mod_node = mod_node_new();
        MOD_NODE_TCID(mod_node) = TASK_BRD_TCID(task_brd);
        MOD_NODE_COMM(mod_node) = TASK_BRD_COMM(task_brd);
        MOD_NODE_RANK(mod_node) = TASK_BRD_RANK(task_brd);
        MOD_NODE_MODI(mod_node) = ERR_MODULE_ID;
        MOD_NODE_HOPS(mod_node) = 0;
        MOD_NODE_LOAD(mod_node) = TASK_BRD_LOAD(task_brd);

        cvector_push(mod_node_vec, (void *)mod_node);
        return;
    }
    else/*for fwd*/
    {
        MOD_NODE *mod_node;

        mod_node = mod_node_new();
        MOD_NODE_TCID(mod_node) = TASK_BRD_TCID(task_brd);
        MOD_NODE_COMM(mod_node) = TASK_BRD_COMM(task_brd);
        MOD_NODE_RANK(mod_node) = TASK_BRD_RANK(task_brd);
        MOD_NODE_MODI(mod_node) = ERR_MODULE_ID;
        MOD_NODE_HOPS(mod_node) = 0;
        MOD_NODE_LOAD(mod_node) = TASK_BRD_LOAD(task_brd);

        cvector_push(mod_node_vec, (void *)mod_node);
    }
#endif
    if(1 >= max_hops)/*this is the last hop*/
    {
        return;
    }

    local_task_cfg = sys_cfg_filter_task_cfg(TASK_BRD_SYS_CFG(task_brd), TASK_BRD_TCID(task_brd));
    if(NULL_PTR == local_task_cfg)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_sync_taskcomm: filter task cfg of tcid %s from sys cfg failed\n", TASK_BRD_TCID_STR(task_brd));
        return;
    }

    local_tasks_cfg      = TASK_BRD_TASKS_CFG(task_brd);
    remote_tasks_cfg_vec = TASK_CFG_TASKS_CFG_VEC(local_task_cfg);

    task_mgr = task_new(NULL_PTR, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    collected_vec = cvector_new(0, MM_CVECTOR, LOC_SUPER_0036);

    MOD_NODE_TCID(&send_mod_node) = TASK_BRD_TCID(task_brd);
    MOD_NODE_COMM(&send_mod_node) = TASK_BRD_COMM(task_brd);
    MOD_NODE_RANK(&send_mod_node) = TASK_BRD_RANK(task_brd);
    MOD_NODE_MODI(&send_mod_node) = 0;
    MOD_NODE_HOPS(&send_mod_node) = 0;
    MOD_NODE_LOAD(&send_mod_node) = 0;

    /*TODO: search current taskcomm*/
    super_sync_taskcomm_self(collected_vec, task_mgr, &send_mod_node);

    /*when local_tasks_cfg belong to intranet of src tcid*/
    if(max_remotes > TASK_BRD_SIZE(task_brd)
    && DES_TCID_IS_INTRANET(src_tcid, src_maski, TASKS_CFG_TCID(local_tasks_cfg), TASKS_CFG_MASKE(local_tasks_cfg)))
    {
        /*TODO: search intranet of local_tasks_cfg with (local tcid, local maski, local maske)*/
        super_sync_taskcomm_intranet(collected_vec, remote_tasks_cfg_vec, local_tasks_cfg,
                                    max_hops - 1, max_remotes - TASK_BRD_SIZE(task_brd), time_to_live,
                                    task_mgr, &send_mod_node);
    }

    /*when local_tasks_cfg belong to lannet of src tcid*/
    if(max_remotes > TASK_BRD_SIZE(task_brd)
    && DES_TCID_IS_LANNET(src_tcid, src_maske, TASKS_CFG_TCID(local_tasks_cfg), TASKS_CFG_MASKE(local_tasks_cfg)))
    {
        /*TODO: search intranet of loca_tasks_cfg with (local tcid, local maski, local maske)*/
        super_sync_taskcomm_lannet(collected_vec, remote_tasks_cfg_vec, local_tasks_cfg,
                                    max_hops - 1, max_remotes - TASK_BRD_SIZE(task_brd), time_to_live,
                                    task_mgr, &send_mod_node);
    }

    /*when local_tasks_cfg belong to extranet of src tcid*/
    if(max_remotes > TASK_BRD_SIZE(task_brd)
    && DES_TCID_IS_EXTERNET(src_tcid, src_maske, TASKS_CFG_TCID(local_tasks_cfg), TASKS_CFG_MASKI(local_tasks_cfg)))
    {
        /*TODO: search lannet and extranet of local_tasks_cfg with (local tcid, local maski, local maske)*/
        super_sync_taskcomm_externet(collected_vec, remote_tasks_cfg_vec, local_tasks_cfg,
                                    max_hops - 1, max_remotes - TASK_BRD_SIZE(task_brd), time_to_live,
                                    task_mgr, &send_mod_node);
    }

    task_wait(task_mgr, time_to_live, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    /*TODO: merge result*/
    mod_node_pos_with_max_hops = cvector_vote_pos(mod_node_vec, (CVECTOR_DATA_VOTER)mod_node_vote_gt_hops);
    for(pos = 0; pos < cvector_size(collected_vec); pos ++)
    {
        CVECTOR *new_mod_node_vec;
        UINT32 new_mod_node_pos;

        new_mod_node_vec = (CVECTOR *)cvector_get_no_lock(collected_vec, pos);
        if(NULL_PTR == new_mod_node_vec)
        {
            continue;
        }

        if(0 == cvector_size(new_mod_node_vec))
        {
            cvector_free(new_mod_node_vec, LOC_SUPER_0037);
            continue;
        }

        for(new_mod_node_pos = 0; new_mod_node_pos < cvector_size(new_mod_node_vec); new_mod_node_pos ++)
        {
            MOD_NODE *new_mod_node;
            MOD_NODE *max_hops_mod_node;

            new_mod_node = (MOD_NODE *)cvector_get_no_lock(new_mod_node_vec, new_mod_node_pos);
            if(NULL_PTR == new_mod_node)
            {
                continue;
            }

            cvector_set_no_lock(new_mod_node_vec, new_mod_node_pos, NULL_PTR);/*umount*/

            if(CVECTOR_ERR_POS != cvector_search_back(mod_node_vec, (void *)new_mod_node, (CVECTOR_DATA_CMP)mod_node_cmp))
            {
                mod_node_free(new_mod_node);
                continue;
            }

            if(MOD_NODE_TCID(new_mod_node) != TASK_BRD_TCID(task_brd))
            {
                MOD_NODE_HOPS(new_mod_node) ++;/*adjust*/
            }

            if(max_remotes > cvector_size(mod_node_vec))
            {
                UINT32 pushed_mod_node_pos;

                pushed_mod_node_pos = cvector_push(mod_node_vec, new_mod_node);

                if(CVECTOR_ERR_POS == mod_node_pos_with_max_hops)
                {
                    mod_node_pos_with_max_hops = pushed_mod_node_pos;
                    continue;
                }

                max_hops_mod_node = (MOD_NODE *)cvector_get(mod_node_vec, mod_node_pos_with_max_hops);
                if(EC_TRUE == mod_node_vote_gt_hops(new_mod_node, max_hops_mod_node))
                {
                    mod_node_pos_with_max_hops = pushed_mod_node_pos;
                    continue;
                }

                /*otherwise, do not update mod_node_pos_with_max_hops*/
                continue;
            }

            /*now max_remotes <= cvector_size(mod_node_vec), i.e., mod_node_vec is full*/
            max_hops_mod_node = (MOD_NODE *)cvector_get(mod_node_vec, mod_node_pos_with_max_hops);
            if(EC_TRUE == mod_node_vote_lt_hops(new_mod_node, max_hops_mod_node))
            {
                cvector_set(mod_node_vec, mod_node_pos_with_max_hops, (void *)new_mod_node);/*replace*/
                mod_node_free(max_hops_mod_node);

                /*re-compute mod_node_pos_with_max_hops*/
                mod_node_pos_with_max_hops = cvector_vote_pos(mod_node_vec, (CVECTOR_DATA_VOTER)mod_node_vote_gt_hops);
                continue;
            }

            mod_node_free(new_mod_node);
        }

        cvector_set_no_lock(collected_vec, pos, NULL_PTR);
        //cvector_clean(new_mod_node_vec, (CVECTOR_DATA_CLEANER)mod_node_free, LOC_SUPER_0038);
        cvector_free(new_mod_node_vec, LOC_SUPER_0039);
    }

    /*when reach here, collected_vec should have no more element*/
    cvector_free(collected_vec, LOC_SUPER_0040);

    task_cfg_free(local_task_cfg);

    return;
}

void super_sync_taskcomm_from_local(const UINT32 super_md_id, const UINT32 max_hops, const UINT32 max_remotes, const UINT32 time_to_live, CVECTOR *mod_node_vec)
{
    TASK_BRD  *task_brd;
    TASKS_CFG *local_tasks_cfg;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_sync_taskcomm_from_local: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();

    local_tasks_cfg = TASK_BRD_TASKS_CFG(task_brd);

    super_sync_taskcomm(TASK_BRD_SUPER_MD_ID(task_brd),
                        TASKS_CFG_TCID(local_tasks_cfg), TASKS_CFG_MASKI(local_tasks_cfg), TASKS_CFG_MASKE(local_tasks_cfg),
                        max_hops, max_remotes, time_to_live, mod_node_vec);
    return;
}

/**
*
* ping remote taskcomm with timeout
*
* if ping ack in timeout, remote taskcomm is reachable, otherwise, it is unreachable
*
**/
EC_BOOL super_ping_taskcomm(const UINT32 super_md_id)
{
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_ping_taskcomm: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    return (EC_TRUE);
}

EC_BOOL super_ping_ipaddr_cstr(const UINT32 super_md_id, const CSTRING *ipaddr_cstr)
{
    TASK_BRD *task_brd;
    UINT32    ipaddr;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_ping_ipaddr_cstr: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    if(EC_TRUE == cstring_is_empty(ipaddr_cstr))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_ping_ipaddr_cstr:ipaddr cstr is empty\n");
        return (EC_FALSE);
    }

    task_brd = task_brd_default_get();
    ipaddr   = c_ipv4_to_word((char *)cstring_get_str(ipaddr_cstr));

    return super_check_ipaddr_connected(super_md_id, ipaddr);
}

/**
*
* list queues in current taskComm
*
**/
void super_show_queues(const UINT32 super_md_id, LOG *log)
{
    TASK_BRD  *task_brd;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_show_queues: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();

    sys_log(log, "===============================[rank_%s_%ld] queues info beg: ===============================\n",
                TASK_BRD_TCID_STR(task_brd), TASK_BRD_RANK(task_brd));

    sys_log(log, "[RECVING QUEUE]\n");
    task_queue_print(log, TASK_BRD_QUEUE(task_brd, TASK_RECVING_QUEUE));

    sys_log(log, "[IS_RECV QUEUE]\n");
    task_queue_print(log, TASK_BRD_QUEUE(task_brd, TASK_IS_RECV_QUEUE));

    sys_log(log, "[TO_SEND QUEUE]\n");
    task_queue_print(log, TASK_BRD_QUEUE(task_brd, TASK_TO_SEND_QUEUE));

    sys_log(log, "[SENDING QUEUE]\n");
    task_queue_print(log, TASK_BRD_QUEUE(task_brd, TASK_SENDING_QUEUE));

    sys_log(log, "[RECV TASK MGR LIST]\n");
    clist_print(log, TASK_BRD_RECV_TASK_MGR_LIST(task_brd), (CLIST_DATA_DATA_PRINT)task_mgr_print);

    sys_log(log, "[AGING TASK MGR LIST]\n");
    clist_print(log, TASK_BRD_AGING_TASK_MGR_LIST(task_brd), (CLIST_DATA_DATA_PRINT)task_mgr_print);

    sys_log(log, "[MOD MGR LIST]\n");
    task_brd_mod_mgr_list_print(log, task_brd);

    sys_log(log, "[CONTEXT LIST]\n");
    task_brd_context_list_print(log, task_brd);
#if 0
    if(CMPI_FWD_RANK == TASK_BRD_RANK(task_brd))
    {
        TASKS_CFG *tasks_cfg;

        tasks_cfg = taskc_get_local_tasks_cfg(TASK_BRD_TASKC_MD_ID(task_brd));

        sys_log(log, "[CSOCKET REQUEST INCOMING LIST]\n");
        //csocket_request_list_print(log, TASKS_WORK_INCOMING_REQUESTS(TASKS_CFG_WORK(tasks_cfg)));

        sys_log(log, "[CSOCKET REQUEST INCOMED LIST]\n");
        //csocket_request_list_print(log, TASKS_WORK_INCOMED_REQUESTS(TASKS_CFG_WORK(tasks_cfg)));

        sys_log(log, "[CSOCKET REQUEST RECVING LIST]\n");
        //csocket_request_list_print(log, TASKS_WORK_RECVING_REQUESTS(TASKS_CFG_WORK(tasks_cfg)));
    }
#endif
    sys_log(log, "===============================[rank_%s_%ld] queues info end: ===============================\n",
                 TASK_BRD_TCID_STR(task_brd), TASK_BRD_RANK(task_brd));

    return;
}

void super_handle_broken_tcid(const UINT32 super_md_id, const UINT32 broken_tcid)
{
    TASK_BRD *task_brd;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_handle_broken_tcid: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "============================== super_handle_broken_tcid beg: broken tcid %s ==============================\n",
                        c_word_to_ipv4(broken_tcid));
    //super_show_queues(super_md_id);/*debug only!*/

    task_brd = task_brd_default_get();

    /*keep recving packet on the road. does anyone keep sending to me without pause??*/
    //task_brd_keep_recving(task_brd);

    if(CMPI_FWD_RANK == TASK_BRD_RANK(task_brd))
    {
        //TASKS_CFG *tasks_cfg;

        /*clean TASKC_NODE list*/
        super_excl_taskc_node(super_md_id, broken_tcid, CMPI_ANY_COMM);

        /*clean up all mod_mgr in task_brd. if found tcid in mod_node of some mod_mgr, then delete the mod_node*/
        task_brd_mod_mgr_list_excl(task_brd, broken_tcid);

        /*the socket to the broken taskComm would be closed automatically*/
        //tasks_cfg = taskc_get_local_tasks_cfg(TASK_BRD_TASKC_MD_ID(task_brd));
        //csocket_discard_task_node_from(TASKS_CFG_WORK(tasks_cfg), broken_tcid);

        //csocket_discard_task_node_to(TASKS_CFG_WORK(tasks_cfg), broken_tcid);

        /**
        *
        *  FROM the broken taskComm
        *  ---------------------------------------------------------------
        *  (QUEUE, TAG)       | TAG_TASK_REQ | TAG_TASK_RSP | TAG_TASK_FWD
        *  ---------------------------------------------------------------
        *  TASK_SENDING_QUEUE |      X       |      X       |     X
        *  ---------------------------------------------------------------
        *  TASK_RECVING_QUEUE |      X       |      X       |     X
        *  ---------------------------------------------------------------
        *  TASK_IS_RECV_QUEUE |      X       |      -       |     X
        *  ---------------------------------------------------------------
        * note: X means to discard, - means no such tag in the queue
        * note: when TAG_TASK_REQ in TASK_SENDING_QUEUE, it must from the broken taskComm be to forward in current taskComm
        * note: when TAG_TASK_REQ in TASK_RECVING_QUEUE, it must from the broken taskComm be to FWD rank of current taskComm
        *
        **/

        task_queue_discard_from(task_brd, TASK_BRD_QUEUE(task_brd, TASK_IS_RECV_QUEUE), TAG_TASK_REQ, broken_tcid);
        task_queue_discard_from(task_brd, TASK_BRD_QUEUE(task_brd, TASK_IS_RECV_QUEUE), TAG_TASK_FWD, broken_tcid);

        task_queue_discard_from(task_brd, TASK_BRD_QUEUE(task_brd, TASK_TO_SEND_QUEUE), TAG_TASK_REQ, broken_tcid);/*new add*/
        task_queue_discard_from(task_brd, TASK_BRD_QUEUE(task_brd, TASK_TO_SEND_QUEUE), TAG_TASK_RSP, broken_tcid);/*new add*/
        task_queue_discard_from(task_brd, TASK_BRD_QUEUE(task_brd, TASK_TO_SEND_QUEUE), TAG_TASK_FWD, broken_tcid);/*new add*/

        /**
        *
        *  TO the broken taskComm
        *  ---------------------------------------------------------------
        *  (QUEUE, TAG)       | TAG_TASK_REQ | TAG_TASK_RSP | TAG_TASK_FWD
        *  ---------------------------------------------------------------
        *  TASK_SENDING_QUEUE |      X       |      X       |     X
        *  ---------------------------------------------------------------
        *  TASK_RECVING_QUEUE |      -       |      -       |     X
        *  ---------------------------------------------------------------
        *  TASK_IS_RECV_QUEUE |      -       |      -       |     X
        *  ---------------------------------------------------------------
        * note: X means to discard, - means no such tag in the queue
        *
        **/
        task_queue_discard_to(task_brd, TASK_BRD_QUEUE(task_brd, TASK_IS_RECV_QUEUE), TAG_TASK_FWD, broken_tcid);

        task_queue_discard_to(task_brd, TASK_BRD_QUEUE(task_brd, TASK_TO_SEND_QUEUE), TAG_TASK_REQ, broken_tcid);/*new add*/
        task_queue_discard_to(task_brd, TASK_BRD_QUEUE(task_brd, TASK_TO_SEND_QUEUE), TAG_TASK_RSP, broken_tcid);/*new add*/
        task_queue_discard_to(task_brd, TASK_BRD_QUEUE(task_brd, TASK_TO_SEND_QUEUE), TAG_TASK_FWD, broken_tcid);/*new add*/

        /*reschedule all TASK_REQ to the borken taskComm*/
        task_mgr_list_handle_broken_taskcomm(task_brd, broken_tcid);/*2014.09.14*/

        /*discard all contexts (end module) from the broken taskComm*/
        task_context_discard_from(task_brd, broken_tcid);

        task_brd_rank_load_tbl_pop_all(task_brd, broken_tcid);
    }

    else
    {
        /*clean up all mod_mgr in task_brd. if found tcid in mod_node of some mod_mgr, then delete the mod_node*/
        task_brd_mod_mgr_list_excl(task_brd, broken_tcid);

        /**
        *
        *  FROM the broken taskComm
        *  ---------------------------------------------------------------
        *  (QUEUE, TAG)       | TAG_TASK_REQ | TAG_TASK_RSP | TAG_TASK_FWD
        *  ---------------------------------------------------------------
        *  TASK_SENDING_QUEUE |      -       |      -       |     -
        *  ---------------------------------------------------------------
        *  TASK_RECVING_QUEUE |      X       |      X       |     -
        *  ---------------------------------------------------------------
        *  TASK_IS_RECV_QUEUE |      X       |      +       |     -
        *  ---------------------------------------------------------------
        * note: X means to discard, - means no such tag in the queue, + means keep this tag in the queue
        * note: when TAG_TASK_REQ in TASK_SENDING_QUEUE, it must from the broken taskComm be to forward in current taskComm
        * note: when TAG_TASK_REQ in TASK_RECVING_QUEUE, it must from the broken taskComm be to FWD rank of current taskComm
        *
        **/
        task_queue_discard_from(task_brd, TASK_BRD_QUEUE(task_brd, TASK_IS_RECV_QUEUE), TAG_TASK_REQ, broken_tcid);

        /**
        *
        *  TO the broken taskComm
        *  ---------------------------------------------------------------
        *  (QUEUE, TAG)       | TAG_TASK_REQ | TAG_TASK_RSP | TAG_TASK_FWD
        *  ---------------------------------------------------------------
        *  TASK_SENDING_QUEUE |      X       |      X       |     -
        *  ---------------------------------------------------------------
        *  TASK_RECVING_QUEUE |      -       |      -       |     -
        *  ---------------------------------------------------------------
        *  TASK_IS_RECV_QUEUE |      -       |      -       |     -
        *  ---------------------------------------------------------------
        * note: X means to discard, - means no such tag in the queue, + means keep this tag in the queue
        * note: when TAG_TASK_REQ in TASK_SENDING_QUEUE, it must from the broken taskComm be to forward in current taskComm
        * note: when TAG_TASK_REQ in TASK_RECVING_QUEUE, it must from the broken taskComm be to FWD rank of current taskComm
        *
        **/
        task_queue_discard_to(task_brd, TASK_BRD_QUEUE(task_brd, TASK_TO_SEND_QUEUE), TAG_TASK_REQ, broken_tcid);/*new add*/
        task_queue_discard_to(task_brd, TASK_BRD_QUEUE(task_brd, TASK_TO_SEND_QUEUE), TAG_TASK_RSP, broken_tcid);/*new add*/

        /*reschedule all TASK_REQ to the borken taskComm*/
        task_mgr_list_handle_broken_taskcomm(task_brd, broken_tcid);

        /*process all TASK_RSP from the broken taskComm*/
        /*task_queue_process_from(task_brd, TASK_BRD_QUEUE(task_brd, TASK_RECVING_QUEUE), TAG_TASK_RSP, broken_tcid);*/

        /*discard all contexts (end module) from the broken taskComm*/
        task_context_discard_from(task_brd, broken_tcid);

        task_brd_rank_load_tbl_pop_all(task_brd, broken_tcid);
    }

    dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "============================== super_handle_broken_tcid end: broken tcid %s ==============================\n",
                        c_word_to_ipv4(broken_tcid));
    //super_show_queues(super_md_id);/*debug only!*/
#if 0
    dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "============================== super_handle_broken_tcid end: register tcid %s beg ==============================\n",
                        c_word_to_ipv4(broken_tcid));

    task_brd_register_node(task_brd, broken_tcid);

    dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "============================== super_handle_broken_tcid end: register tcid %s end ==============================\n",
                        c_word_to_ipv4(broken_tcid));
#endif
    return;
}
/**
*
* when fwd rank found some broken taskcomm, then notify all ranks in current taskcomm
*
* note: here does not notify other taskcomm(s)
*
**/
void super_notify_broken_tcid(const UINT32 super_md_id, const UINT32 broken_tcid)
{
    TASK_BRD  *task_brd;

    CSET      *rank_set;
    MOD_MGR   *mod_mgr;
    TASK_MGR  *task_mgr;

    UINT32 mod_node_num;
    UINT32 mod_node_idx;

    UINT32 broken_tcid_pos;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_notify_broken_tcid: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();

    /*pre-checking*/
    CVECTOR_LOCK(TASK_BRD_BROKEN_TCID_TBL(task_brd), LOC_SUPER_0041);
    broken_tcid_pos = cvector_search_front_no_lock(TASK_BRD_BROKEN_TCID_TBL(task_brd), (void *)broken_tcid, NULL_PTR);
    if(CVECTOR_ERR_POS != broken_tcid_pos)
    {
        /*okay, someone is working on this broken tcid, terminate on-going*/
        CVECTOR_UNLOCK(TASK_BRD_BROKEN_TCID_TBL(task_brd), LOC_SUPER_0042);
        return;
    }
    cvector_push_no_lock(TASK_BRD_BROKEN_TCID_TBL(task_brd), (void *)broken_tcid);
    CVECTOR_UNLOCK(TASK_BRD_BROKEN_TCID_TBL(task_brd), LOC_SUPER_0043);

    dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "============================== super_notify_broken_tcid beg: broken tcid %s ==============================\n",
                        c_word_to_ipv4(broken_tcid));

    mod_mgr = mod_mgr_new(super_md_id, LOAD_BALANCING_LOOP);

    /*set mod_mgr*/
    rank_set_new(&rank_set);
    rank_set_init(rank_set, TASK_BRD_SIZE(task_brd));
    mod_mgr_set(TASK_BRD_TCID(task_brd), TASK_BRD_COMM(task_brd), 0, rank_set, mod_mgr);
    //mod_mgr_excl(CMPI_ANY_DBG_TCID, CMPI_ANY_COMM, CMPI_ANY_RANK, CMPI_ANY_MODI, mod_mgr);/*ignore dbg process in dbg taskcomm*/
    rank_set_free(rank_set);

#if 1
    if(do_log(SEC_0117_SUPER, 5))
    {
        sys_log(LOGSTDOUT, "------------------------------------ super_notify_broken_tcid beg ----------------------------------\n");
        mod_mgr_print(LOGSTDOUT, mod_mgr);
        sys_log(LOGSTDOUT, "------------------------------------ super_notify_broken_tcid end ----------------------------------\n");
    }
#endif

    /*set task_mgr*/
    task_mgr = task_new(mod_mgr, TASK_PRIO_PREEMPT, TASK_NOT_NEED_RSP_FLAG, TASK_NEED_NONE_RSP);

    mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(mod_node_idx = 0; mod_node_idx < mod_node_num; mod_node_idx ++)
    {
        task_pos_inc(task_mgr, mod_node_idx, NULL_PTR, FI_super_handle_broken_tcid, ERR_MODULE_ID, broken_tcid);
    }

    task_no_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    /*ok, complete the broken tcid handling, remove it from table*/
    CVECTOR_LOCK(TASK_BRD_BROKEN_TCID_TBL(task_brd), LOC_SUPER_0044);
    /*we have to search it again because the broken tcid position maybe changed under multiple thread environment*/
    broken_tcid_pos = cvector_search_front_no_lock(TASK_BRD_BROKEN_TCID_TBL(task_brd), (void *)broken_tcid, NULL_PTR);
    cvector_erase_no_lock(TASK_BRD_BROKEN_TCID_TBL(task_brd), broken_tcid_pos);
    CVECTOR_UNLOCK(TASK_BRD_BROKEN_TCID_TBL(task_brd), LOC_SUPER_0045);

    dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "============================== super_notify_broken_tcid end: broken tcid %s ==============================\n",
                        c_word_to_ipv4(broken_tcid));

    return;
}

/**
*
* when fwd rank found some broken route, then notify the src taskcomm
*
**/
void super_notify_broken_route(const UINT32 super_md_id, const UINT32 src_tcid, const UINT32 broken_tcid)
{
    TASK_BRD  *task_brd;

    MOD_MGR   *mod_mgr;
    TASK_MGR  *task_mgr;

    UINT32 local_mod_node_pos;
    UINT32 remote_mod_node_pos;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_notify_broken_route: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "============================== super_notify_broken_route beg: src tcid %s, broken tcid %s ==============================\n",
                        c_word_to_ipv4(src_tcid), c_word_to_ipv4(broken_tcid));

    task_brd = task_brd_default_get();

    mod_mgr = mod_mgr_new(super_md_id, LOAD_BALANCING_LOOP);

    /*set mod_mgr*/
    local_mod_node_pos  = mod_mgr_incl(TASK_BRD_TCID(task_brd), TASK_BRD_COMM(task_brd), CMPI_FWD_RANK, 0, mod_mgr);
    remote_mod_node_pos = mod_mgr_incl(src_tcid, CMPI_ANY_COMM, CMPI_FWD_RANK, 0, mod_mgr);

#if 1
    if(do_log(SEC_0117_SUPER, 5))
    {
        sys_log(LOGSTDOUT, "------------------------------------ super_notify_broken_route beg ----------------------------------\n");
        mod_mgr_print(LOGSTDOUT, mod_mgr);
        sys_log(LOGSTDOUT, "------------------------------------ super_notify_broken_route end ----------------------------------\n");
    }
#endif

    /*set task_mgr*/
    task_mgr = task_new(mod_mgr, TASK_PRIO_PREEMPT, TASK_NOT_NEED_RSP_FLAG, TASK_NEED_NONE_RSP);
    task_pos_inc(task_mgr, local_mod_node_pos , NULL_PTR, FI_super_handle_broken_tcid, ERR_MODULE_ID, broken_tcid);
    task_pos_inc(task_mgr, remote_mod_node_pos, NULL_PTR, FI_super_notify_broken_tcid, ERR_MODULE_ID, broken_tcid);
    task_no_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "============================== super_notify_broken_route end: src tcid %s, broken tcid %s ==============================\n",
                        c_word_to_ipv4(src_tcid), c_word_to_ipv4(broken_tcid));

    return;
}

/**
*
* show work clients of tasks_cfg of taskc_cfg of task_brd
*
**/
void super_show_work_client(const UINT32 super_md_id, LOG *log)
{
    TASK_BRD  *task_brd;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_show_work_client: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();

    if(CMPI_FWD_RANK == TASK_BRD_RANK(task_brd))
    {
        TASKS_CFG *tasks_cfg;
        UINT32 index;

        tasks_cfg = TASK_BRD_TASKS_CFG(task_brd);
        index     = 0;

        //dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "===================================== working clients beg: =====================================\n");
        tasks_work_print_csocket_cnode_list_in_plain(log, TASKS_CFG_WORK(tasks_cfg), &index);
        //dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "===================================== working clients end: =====================================\n");
    }
    return;
}

/**
*
* show work threads of tasks_cfg of taskc_cfg of task_brd
*
**/
void super_show_thread_num(const UINT32 super_md_id, LOG *log)
{
    TASK_BRD  *task_brd;
    UINT32     idle_thread_num;
    UINT32     busy_thread_num;
    UINT32     total_thread_num;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_show_thread_num: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();

    croutine_pool_num_info(TASK_REQ_CTHREAD_POOL(task_brd), &idle_thread_num, &busy_thread_num, &total_thread_num);
    sys_log(log, "total req thread %ld, busy %ld, idle %ld\n", total_thread_num, busy_thread_num, idle_thread_num);

    return;
}

/**
*
* show route table of tasks_cfg of taskc_cfg of task_brd
*
**/
void super_show_route_table(const UINT32 super_md_id, LOG *log)
{
    TASK_BRD  *task_brd;
    TASKS_CFG *tasks_cfg;

    UINT32 pos;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_show_route_table: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();
    tasks_cfg = TASK_BRD_TASKS_CFG(task_brd);

    CVECTOR_LOCK(TASKS_CFG_TASKR_CFG_VEC(tasks_cfg), LOC_SUPER_0046);
    if(EC_TRUE == cvector_is_empty(TASKS_CFG_TASKR_CFG_VEC(tasks_cfg)))
    {
        sys_log(log, "(no route)\n");
        CVECTOR_UNLOCK(TASKS_CFG_TASKR_CFG_VEC(tasks_cfg), LOC_SUPER_0047);
        return;
    }
    for(pos = 0; pos < cvector_size(TASKS_CFG_TASKR_CFG_VEC(tasks_cfg)); pos ++)
    {
        TASKR_CFG *taskr_cfg;

        taskr_cfg = (TASKR_CFG *)cvector_get_no_lock(TASKS_CFG_TASKR_CFG_VEC(tasks_cfg), pos);
        if(NULL_PTR == taskr_cfg)
        {
            sys_log(log, "No. %ld: (null route)\n", pos);
            continue;
        }

        sys_log(log, "No. %ld: des_tcid = %s, maskr = %s, next_tcid = %s\n", pos,
                        TASKR_CFG_DES_TCID_STR(taskr_cfg),
                        TASKR_CFG_MASKR_STR(taskr_cfg),
                        TASKR_CFG_NEXT_TCID_STR(taskr_cfg));
    }
    CVECTOR_UNLOCK(TASKS_CFG_TASKR_CFG_VEC(tasks_cfg), LOC_SUPER_0048);
    return;
}

/**
*
* show rank node status of the rank
*
**/
void super_show_rank_node(const UINT32 super_md_id, LOG *log)
{
    TASK_BRD  *task_brd;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_show_rank_node: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();

    task_rank_tbl_print(log, TASK_BRD_RANK_TBL(task_brd));
    return;
}


/**
*
* switch/enable rank node light to green
*
**/
void super_switch_rank_node_green(const UINT32 super_md_id, const UINT32 rank)
{
    TASK_BRD  *task_brd;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_switch_rank_node_green: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();

    if(CMPI_ANY_RANK == rank)
    {
        task_rank_tbl_enable_all(TASK_BRD_RANK_TBL(task_brd));
    }
    else
    {
        task_rank_tbl_enable(TASK_BRD_RANK_TBL(task_brd), rank);
    }
    return;
}

/**
*
* switch/disable rank node light to red
*
**/
void super_switch_rank_node_red(const UINT32 super_md_id, const UINT32 rank)
{
    TASK_BRD  *task_brd;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_switch_rank_node_red: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();

    if(CMPI_ANY_RANK == rank)
    {
        task_rank_tbl_disable_all(TASK_BRD_RANK_TBL(task_brd));
    }
    else
    {
        task_rank_tbl_disable(TASK_BRD_RANK_TBL(task_brd), rank);
    }
    return;
}

/**
*
* output log by SUPER module
*
**/
void super_show_cstring(const UINT32 super_md_id, const UINT32 tcid, const UINT32 rank, const CSTRING *cstring)
{
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_show_cstring: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/
    //fprintf(LOGSTDOUT, "[rank_%s_%ld] ", c_word_to_ipv4(tcid), rank);
    //fprintf(LOGSTDOUT, "%s", (char *)cstring_get_str(cstring));
    //fflush(LOGSTDOUT);
    fputs((char *)cstring_get_str(cstring), stdout);
    fflush(stdout);

    return;
}

/**
*
* switch log off
*
**/
void super_switch_log_off(const UINT32 super_md_id)
{
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_switch_log_off: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/
    sys_log_switch_off();
    return;
}

/**
*
* switch log on
*
**/
void super_switch_log_on(const UINT32 super_md_id)
{
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_switch_log_on: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/
    sys_log_switch_on();
    return;
}


/**
*
* wait until current process of current taskComm is ready
*
**/
void super_wait_me_ready(const UINT32 super_md_id)
{
    TASK_BRD  *task_brd;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_wait_me_ready: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/
    task_brd = task_brd_default_get();

    TASK_BRD_FWD_CCOND_WAIT(task_brd, LOC_SUPER_0049);
    return;
}

/**
*
* add route
*
**/
void super_add_route(const UINT32 super_md_id, const UINT32 des_tcid, const UINT32 maskr, const UINT32 next_tcid)
{
    TASK_BRD  *task_brd;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_add_route: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();

    if(CMPI_FWD_RANK == TASK_BRD_RANK(task_brd))
    {
        TASKS_CFG *local_tasks_cfg;
        TASKR_CFG *taskr_cfg;

        local_tasks_cfg = TASK_BRD_TASKS_CFG(task_brd);

        taskr_cfg = taskr_cfg_new();
        taskr_cfg_set(taskr_cfg, des_tcid, maskr, next_tcid);

        if(EC_FALSE == tasks_cfg_add_taskr(local_tasks_cfg, taskr_cfg))
        {
            taskr_cfg_free(taskr_cfg);
        }
    }

    return;
}

/**
*
* del route
*
**/
void super_del_route(const UINT32 super_md_id, const UINT32 des_tcid, const UINT32 maskr, const UINT32 next_tcid)
{
    TASK_BRD  *task_brd;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_del_route: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();

    if(CMPI_FWD_RANK == TASK_BRD_RANK(task_brd))
    {
        TASKS_CFG *local_tasks_cfg;
        TASKR_CFG *taskr_cfg;

        local_tasks_cfg = TASK_BRD_TASKS_CFG(task_brd);

        taskr_cfg = taskr_cfg_new();
        taskr_cfg_set(taskr_cfg, des_tcid, maskr, next_tcid);

        while(EC_TRUE == tasks_cfg_del_taskr(local_tasks_cfg, taskr_cfg))
        {
            /*do nothing*/
        }

        taskr_cfg_free(taskr_cfg);
    }

    return;
}

/**
*
* add socket connection
*
**/
void super_add_connection(const UINT32 super_md_id, const UINT32 des_tcid, const UINT32 des_srv_ipaddr, const UINT32 des_srv_port, const UINT32 conn_num)
{
    //UINT32     csocket_cnode_idx;
    TASK_BRD  *task_brd;
    //TASKS_CFG *local_tasks_cfg;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_add_connection: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();

    if(CMPI_FWD_RANK != TASK_BRD_RANK(task_brd))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_add_connection: current rank %ld is not fwd rank\n", TASK_BRD_RANK(task_brd));
        return;
    }

    if(des_tcid == TASK_BRD_TCID(task_brd))
    {
        dbg_log(SEC_0117_SUPER, 1)(LOGSTDOUT, "warn:super_add_connection: giveup connect to itself\n");
        return;
    }

    task_brd_register_one(task_brd, des_tcid, des_srv_ipaddr, des_srv_port, conn_num);
#if 0
    local_tasks_cfg = taskc_get_local_tasks_cfg(TASK_BRD_TASKC_MD_ID(task_brd));
    if(NULL_PTR == local_tasks_cfg)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_add_connection: local tasks cfg is null\n");
        return;
    }

    /*setup multi sockets to remote taskcomm*/
    for(csocket_cnode_idx = 0; csocket_cnode_idx < conn_num; csocket_cnode_idx ++)
    {
        if(EC_FALSE == tasks_monitor_open(TASKS_CFG_MONITOR(local_tasks_cfg), des_tcid, des_srv_ipaddr, des_srv_port))
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_add_connection: failed connect to des tcid %s, srvipaddr %s, srvport %ld\n",
                                c_word_to_ipv4(des_tcid), c_word_to_ipv4(des_srv_ipaddr), des_srv_port);
            break;
        }
    }
#endif
    return;
}

/**
*
* execute shell command and return output as CSTRING
*
**/
void super_run_shell(const UINT32 super_md_id, const CSTRING *cmd_line, LOG *log)
{
    FILE   *rstream;
    CSTRING *result;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_run_shell: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "super_run_shell: execute shell command: %s\n", (char *)cstring_get_str(cmd_line));

    rstream = popen((char *)cstring_get_str(cmd_line), "r");

    result = cstring_new(NULL_PTR, LOC_SUPER_0050);
    cstring_set_capacity(result, 4096);/*4KB*/

    cstring_fread(result, rstream);

    pclose( rstream );

    //dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "super_run_shell: shell command output:\n %s\n", (char *)cstring_get_str(result));/*debug*/
    sys_print(log, "%s", (char *)cstring_get_str(result));
    cstring_free(result);

    return;
}


/**
*
* execute shell command and return output as CBYTES
*
**/
EC_BOOL super_exec_shell(const UINT32 super_md_id, const CSTRING *cmd_line, CBYTES *cbytes)
{
    FILE   *rstream;
    CSTRING *cmd_line_fix;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_exec_shell: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    cmd_line_fix = cstring_new(cstring_get_str(cmd_line), LOC_SUPER_0051);
    cstring_append_str(cmd_line_fix, (UINT8 *)" 2>&1");/*fix*/

    dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "super_exec_shell: execute shell command: %s\n", (char *)cstring_get_str(cmd_line_fix));

    rstream = popen((char *)cstring_get_str(cmd_line_fix), "r");

    //cbytes_expand_to(cbytes, 4096);/*4KB*/
    cbytes_fread(cbytes, rstream);

    pclose( rstream );

    dbg_log(SEC_0117_SUPER, 5)(LOGSTDNULL, "super_exec_shell: execute shell command: \"%s\", output len %ld, content is\n%.*s\n",
                        (char *)cstring_get_str(cmd_line_fix), cbytes_len(cbytes),
                        cbytes_len(cbytes), cbytes_buf(cbytes));

    dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "super_exec_shell: execute shell command: \"%s\", output len %ld\n",
                        (char *)cstring_get_str(cmd_line_fix), cbytes_len(cbytes));

    cstring_free(cmd_line_fix);
    return (EC_TRUE);
}

EC_BOOL super_exec_shell_tcid_cstr(const UINT32 super_md_id, const CSTRING *tcid_cstr, const CSTRING *cmd_line, CBYTES *output_cbytes)
{
    UINT32    tcid;
    MOD_NODE  recv_mod_node;

    EC_BOOL ret;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_exec_shell_tcid_cstr: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "super_exec_shell_tcid_cstr: execute shell command on tcid %s: %s\n",
                        (char *)cstring_get_str(tcid_cstr),
                        (char *)cstring_get_str(cmd_line));

    tcid = c_ipv4_to_word((char *)cstring_get_str(tcid_cstr));
    if(CMPI_LOCAL_TCID == tcid)
    {
        return super_exec_shell(super_md_id, cmd_line, output_cbytes);
    }

    if(EC_FALSE == task_brd_check_tcid_connected(task_brd_default_get(), tcid))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_exec_shell_tcid_cstr: tcid %s not connected\n", (char *)cstring_get_str(tcid_cstr));
        return (EC_FALSE);
    }

    MOD_NODE_TCID(&recv_mod_node) = tcid;
    MOD_NODE_COMM(&recv_mod_node) = CMPI_ANY_COMM;
    MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
    MOD_NODE_MODI(&recv_mod_node) = 0;

    ret = EC_FALSE;
    task_p2p(super_md_id, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, &recv_mod_node,
                &ret, FI_super_exec_shell, ERR_MODULE_ID, cmd_line, output_cbytes);
    return (ret);
}

/**
*
* execute shell command on several taskcomm and return each output as CBYTES
*
**/
EC_BOOL super_exec_shell_vec(const UINT32 super_md_id, const CVECTOR *tcid_vec, const CVECTOR *cmd_line_vec, CVECTOR *output_cbytes_vec)
{
    UINT32 tcid_num;
    UINT32 cmd_line_num;
    UINT32 pos;

    TASK_MGR *task_mgr;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_exec_shell_vec: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    tcid_num = cvector_size(tcid_vec);
    cmd_line_num = cvector_size(cmd_line_vec);

    if(tcid_num != cmd_line_num)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_exec_shell_vec: mismatched tcid vec size %ld and cmd line vec size %ld\n", tcid_num, cmd_line_num);
        return (EC_FALSE);
    }

    task_mgr = task_new(NULL, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    for(pos = 0; pos < tcid_num; pos ++)
    {
        UINT32   tcid;
        CSTRING *cmd_line;
        CBYTES  *output_cbytes;

        MOD_NODE recv_mod_node;

        tcid = (UINT32)cvector_get_no_lock(tcid_vec, pos);
        cmd_line = (CSTRING *)cvector_get_no_lock(cmd_line_vec, pos);

        output_cbytes = cbytes_new(0);
        if(NULL_PTR == output_cbytes)
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_exec_shell_vec: new output cbytes failed\n");
            cvector_clean_with_location_no_lock(output_cbytes_vec, (CVECTOR_DATA_LOCATION_CLEANER)cbytes_free, LOC_SUPER_0052);
            return (EC_FALSE);
        }

        cvector_push_no_lock(output_cbytes_vec, (void *)output_cbytes);

        MOD_NODE_TCID(&recv_mod_node) = tcid;
        MOD_NODE_COMM(&recv_mod_node) = CMPI_ANY_COMM;
        MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
        MOD_NODE_MODI(&recv_mod_node) = 0;

        task_p2p_inc(task_mgr, super_md_id, &recv_mod_node, NULL_PTR, FI_super_exec_shell, ERR_MODULE_ID, cmd_line, output_cbytes);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    return (EC_TRUE);
}



/**
*
* execute shell command on several taskcomm and return each output as CBYTES
* note: where tcid_cstr_vec is tcid STRING vector
**/
EC_BOOL super_exec_shell_vec_tcid_cstr(const UINT32 super_md_id, const CVECTOR *tcid_cstr_vec, const CSTRING *cmd_line, CVECTOR *output_cbytes_vec)
{
    UINT32 tcid_pos;

    TASK_MGR *task_mgr;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_exec_shell_vec_tcid_cstr: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_mgr = task_new(NULL, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    for(tcid_pos = 0; tcid_pos < cvector_size(tcid_cstr_vec); tcid_pos ++)
    {
        CSTRING *tcid_cstr;
        UINT32   tcid;
        CBYTES  *output_cbytes;

        MOD_NODE recv_mod_node;

        tcid_cstr = (CSTRING *)cvector_get_no_lock(tcid_cstr_vec, tcid_pos);
        tcid      = c_ipv4_to_word((char *)cstring_get_str(tcid_cstr));

        output_cbytes = cbytes_new(0);
        if(NULL_PTR == output_cbytes)
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_exec_shell_vec_tcid_cstr: new output cbytes failed\n");
            cvector_clean_with_location_no_lock(output_cbytes_vec, (CVECTOR_DATA_LOCATION_CLEANER)cbytes_free, LOC_SUPER_0053);
            task_mgr_free(task_mgr);
            return (EC_FALSE);
        }

        /*if tcid not reachable, empty cbytes will return*/
        cvector_push_no_lock(output_cbytes_vec, (void *)output_cbytes);

        if(EC_FALSE == task_brd_check_tcid_connected(task_brd_default_get(), tcid))
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_exec_shell_vec_tcid_cstr: tcid %s not connected, skip it\n", (char *)cstring_get_str(tcid_cstr));
            continue;
        }

        MOD_NODE_TCID(&recv_mod_node) = tcid;
        MOD_NODE_COMM(&recv_mod_node) = CMPI_ANY_COMM;
        MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
        MOD_NODE_MODI(&recv_mod_node) = 0;

        task_p2p_inc(task_mgr, super_md_id, &recv_mod_node, NULL_PTR, FI_super_exec_shell, ERR_MODULE_ID, cmd_line, output_cbytes);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    //dbg_log(SEC_0117_SUPER, 9)(LOGSTDOUT, "[DEBUG] super_exec_shell_vec_tcid_cstr: result is\n");
    //cvector_print(LOGSTDOUT, output_cbytes_vec, (CVECTOR_DATA_PRINT)cbytes_print_str);

    return (EC_TRUE);
}

EC_BOOL super_exec_shell_ipaddr_cstr(const UINT32 super_md_id, const CSTRING *ipaddr_cstr, const CSTRING *cmd_line, CBYTES *output_cbytes)
{
    UINT32   ipaddr;
    UINT32   tcid;
    MOD_NODE recv_mod_node;

    EC_BOOL ret;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_exec_shell_ipaddr_cstr: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/
    if(EC_TRUE == cstring_is_empty(ipaddr_cstr))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_exec_shell_ipaddr_cstr:ipaddr cstr is empty\n");
        return (EC_FALSE);
    }

    if(EC_TRUE == cstring_is_empty(cmd_line))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_exec_shell_ipaddr_cstr:cmd_line is empty\n");
        return (EC_FALSE);
    }

    dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "super_exec_shell_ipaddr_cstr: execute shell command on ipaddr %s: %s\n",
                        (char *)cstring_get_str(ipaddr_cstr),
                        (char *)cstring_get_str(cmd_line));

    ipaddr = c_ipv4_to_word((char *)cstring_get_str(ipaddr_cstr));
    tcid = task_brd_get_tcid_by_ipaddr(task_brd_default_get(), ipaddr);
    if(CMPI_ERROR_TCID == tcid)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_exec_shell_ipaddr_cstr: no tcid for ipaddr %s failed\n", (char *)cstring_get_str(ipaddr_cstr));
        return (EC_FALSE);
    }

    if(CMPI_LOCAL_TCID == tcid)
    {
        return super_exec_shell(super_md_id, cmd_line, output_cbytes);
    }

    if(EC_FALSE == task_brd_check_tcid_connected(task_brd_default_get(), tcid))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_exec_shell_ipaddr_cstr: ipaddr %s not connected\n", (char *)cstring_get_str(ipaddr_cstr));
        return (EC_FALSE);
    }

    MOD_NODE_TCID(&recv_mod_node) = tcid;
    MOD_NODE_COMM(&recv_mod_node) = CMPI_ANY_COMM;
    MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
    MOD_NODE_MODI(&recv_mod_node) = 0;

    ret = EC_FALSE;
    task_p2p(super_md_id, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, &recv_mod_node,
                &ret, FI_super_exec_shell, ERR_MODULE_ID, cmd_line, output_cbytes);
    return (ret);
}

EC_BOOL super_exec_shell_vec_ipaddr_cstr(const UINT32 super_md_id, const CVECTOR *ipaddr_cstr_vec, const CSTRING *cmd_line, CVECTOR *output_cbytes_vec)
{
    UINT32 ipaddr_pos;

    TASK_BRD *task_brd;
    TASK_MGR *task_mgr;

    EC_BOOL ret;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_exec_shell_vec_ipaddr_cstr: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/
    if(EC_TRUE == cstring_is_empty(cmd_line))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_exec_shell_vec_ipaddr_cstr:cmd_line is empty\n");
        return (EC_FALSE);
    }

    task_brd = task_brd_default_get();

    task_mgr = task_new(NULL, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    for(ipaddr_pos = 0; ipaddr_pos < cvector_size(ipaddr_cstr_vec); ipaddr_pos ++)
    {
        CSTRING *ipaddr_cstr;
        UINT32   ipaddr;
        UINT32   tcid;
        CBYTES  *output_cbytes;

        MOD_NODE recv_mod_node;

        ipaddr_cstr = (CSTRING *)cvector_get_no_lock(ipaddr_cstr_vec, ipaddr_pos);
        ipaddr      = c_ipv4_to_word((char *)cstring_get_str(ipaddr_cstr));
        tcid        = task_brd_get_tcid_by_ipaddr(task_brd, ipaddr);

        output_cbytes = cbytes_new(0);
        if(NULL_PTR == output_cbytes)
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_exec_shell_vec_ipaddr_cstr: new output cbytes failed\n");
            cvector_clean_with_location_no_lock(output_cbytes_vec, (CVECTOR_DATA_LOCATION_CLEANER)cbytes_free, LOC_SUPER_0054);
            task_mgr_free(task_mgr);
            return (EC_FALSE);
        }

        /*if ipaddr not reachable, empty cbytes will return*/
        cvector_push_no_lock(output_cbytes_vec, (void *)output_cbytes);

        if(CMPI_ERROR_TCID == tcid)
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_exec_shell_vec_ipaddr_cstr: no tcid for ipaddr %s\n", (char *)cstring_get_str(ipaddr_cstr));
        }

        if(EC_FALSE == task_brd_check_tcid_connected(task_brd_default_get(), tcid))
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_exec_shell_vec_ipaddr_cstr: ipaddr %s not connected, skip it\n", (char *)cstring_get_str(ipaddr_cstr));
            continue;
        }

        MOD_NODE_TCID(&recv_mod_node) = tcid;
        MOD_NODE_COMM(&recv_mod_node) = CMPI_ANY_COMM;
        MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
        MOD_NODE_MODI(&recv_mod_node) = 0;

        ret = EC_FALSE;
        task_p2p_inc(task_mgr, super_md_id, &recv_mod_node, &ret, FI_super_exec_shell, ERR_MODULE_ID, cmd_line, output_cbytes);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    //dbg_log(SEC_0117_SUPER, 9)(LOGSTDOUT, "[DEBUG] super_exec_shell_vec_ipaddr_cstr: result is\n");
    //cvector_print(LOGSTDOUT, output_cbytes_vec, (CVECTOR_DATA_PRINT)cbytes_print_str);

    return (EC_TRUE);
}

EC_BOOL super_exec_shell_cbtimer_reset(const UINT32 super_md_id, const CSTRING *cbtimer_name, const CSTRING *cmd_line, const UINT32 timeout)
{
    UINT32 timeout_func_id;
    CBTIMER_NODE   *cbtimer_node;
    FUNC_ADDR_NODE *func_addr_node;
    TASK_FUNC *handler;
    TASK_BRD  *task_brd;

    UINT32 mod_type;
    UINT32 delta;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_exec_shell_cbtimer_reset: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    if(EC_TRUE == cstring_is_empty(cbtimer_name))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_exec_shell_cbtimer_reset:cbtimer_name is empty\n");
        return (EC_FALSE);
    }

    if(EC_TRUE == cstring_is_empty(cmd_line))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_exec_shell_cbtimer_reset:cmd_line is empty\n");
        return (EC_FALSE);
    }

    task_brd = task_brd_default_get();

    timeout_func_id = FI_super_exec_shell;
    mod_type = (timeout_func_id >> (WORDSIZE / 2));
    if( MD_END <= mod_type )
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDERR, "error:super_exec_shell_cbtimer_reset: invalid timeout_func_id %lx\n", timeout_func_id);
        return (EC_FALSE);
    }

    if(0 != dbg_fetch_func_addr_node_by_index(timeout_func_id, &func_addr_node))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_exec_shell_cbtimer_reset: failed to fetch func addr node by func id %lx\n", timeout_func_id);
        return (EC_FALSE);
    }

    cbtimer_node = cbtimer_search_by_name(TASK_BRD_CBTIMER_LIST(task_brd), cbtimer_name);
    if(NULL_PTR == cbtimer_node)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_exec_shell_cbtimer_reset: undefined cbtimer with name %s\n", (char *)cstring_get_str(cbtimer_name));
        return (EC_FALSE);
    }

    if(NULL_PTR != CBTIMER_NODE_NAME(cbtimer_node))
    {
        cstring_free(CBTIMER_NODE_NAME(cbtimer_node));
        CBTIMER_NODE_NAME(cbtimer_node) = NULL_PTR;
    }

    CBTIMER_NODE_NAME(cbtimer_node) = cstring_new(cstring_get_str(cbtimer_name), LOC_SUPER_0055);

    /*timeout < timeout + delta = expire_time < 2 * timeout*/
    /*delta should ensure timeout action must be executed once and only once, but not 100 percent :(*/
    delta = 60;
    if(delta >= timeout)
    {
        delta = (timeout / 2);
    }

    CBTIMER_NODE_EXPIRE_NSEC(cbtimer_node)   = timeout + delta;
    CBTIMER_NODE_TIMEOUT_NSEC(cbtimer_node)  = timeout;

    CBTIMER_NODE_EXPIRE_FUNC_ADDR_NODE(cbtimer_node)  = NULL_PTR;
    CBTIMER_NODE_TIMEOUT_FUNC_ADDR_NODE(cbtimer_node) = func_addr_node;

    handler = CBTIMER_NODE_TIMEOUT_HANDLER(cbtimer_node);

    handler->func_id       = timeout_func_id;
    handler->func_para_num = func_addr_node->func_para_num;
    handler->func_ret_val  = EC_TRUE;

    if(NULL_PTR != handler->func_para[ 1 ].para_val)
    {
        cstring_free((CSTRING *)(handler->func_para[ 1 ].para_val));
        handler->func_para[ 1 ].para_val = 0;
    }

    if(NULL_PTR != handler->func_para[ 2 ].para_val)
    {
        cbytes_free((CBYTES *)(handler->func_para[ 2 ].para_val), LOC_SUPER_0056);
        handler->func_para[ 2 ].para_val = 0;
    }

    handler->func_para[ 0 ].para_val = super_md_id;
    handler->func_para[ 1 ].para_val = (UINT32)cstring_new(cstring_get_str(cmd_line), LOC_SUPER_0057);
    handler->func_para[ 2 ].para_val = (UINT32)cbytes_new(0);

    CTIMET_GET(CBTIMER_NODE_START_TIME(cbtimer_node));
    CTIMET_GET(CBTIMER_NODE_LAST_TIME(cbtimer_node));
    return (EC_TRUE);
}

EC_BOOL super_exec_shell_cbtimer_set(const UINT32 super_md_id, const CSTRING *cbtimer_name, const CSTRING *cmd_line, const UINT32 timeout)
{
    UINT32 timeout_func_id;
    CBTIMER_NODE   *cbtimer_node;
    FUNC_ADDR_NODE *func_addr_node;
    TASK_FUNC *handler;
    TASK_BRD  *task_brd;

    UINT32 mod_type;
    UINT32 delta;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_exec_shell_cbtimer_set: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    if(EC_TRUE == cstring_is_empty(cbtimer_name))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_exec_shell_cbtimer_set:cbtimer_name is empty\n");
        return (EC_FALSE);
    }

    if(EC_TRUE == cstring_is_empty(cmd_line))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_exec_shell_cbtimer_set:cmd_line is empty\n");
        return (EC_FALSE);
    }

    task_brd = task_brd_default_get();

    cbtimer_node = cbtimer_search_by_name(TASK_BRD_CBTIMER_LIST(task_brd), cbtimer_name);
    if(NULL_PTR != cbtimer_node)
    {
        dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "super_exec_shell_cbtimer_set: found cbtimer with name %s, try to reset it\n", (char *)cstring_get_str(cbtimer_name));
        return super_exec_shell_cbtimer_reset(super_md_id, cbtimer_name, cmd_line, timeout);
    }

    timeout_func_id = FI_super_exec_shell;
    mod_type = (timeout_func_id >> (WORDSIZE / 2));
    if( MD_END <= mod_type )
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDERR, "error:super_exec_shell_cbtimer_set: invalid timeout_func_id %lx\n", timeout_func_id);
        return (EC_FALSE);
    }

    if(0 != dbg_fetch_func_addr_node_by_index(timeout_func_id, &func_addr_node))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_exec_shell_cbtimer_set: failed to fetch func addr node by func id %lx\n", timeout_func_id);
        return (EC_FALSE);
    }

    cbtimer_node = cbtimer_node_new();
    if(NULL_PTR == cbtimer_node)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_exec_shell_cbtimer_set: new cbtimer node failed\n");
        return (EC_FALSE);
    }

    CBTIMER_NODE_NAME(cbtimer_node) = cstring_new(cstring_get_str(cbtimer_name), LOC_SUPER_0058);

    /*timeout < timeout + delta = expire_time < 2 * timeout*/
    /*delta should ensure timeout action must be executed once and only once, but not 100 percent :(*/
    delta = 60;
    if(delta >= timeout)
    {
        delta = (timeout / 2);
    }

    CBTIMER_NODE_EXPIRE_NSEC(cbtimer_node)   = timeout + delta;
    CBTIMER_NODE_TIMEOUT_NSEC(cbtimer_node)  = timeout;

    CBTIMER_NODE_EXPIRE_FUNC_ADDR_NODE(cbtimer_node)  = NULL_PTR;
    CBTIMER_NODE_TIMEOUT_FUNC_ADDR_NODE(cbtimer_node) = func_addr_node;

    handler = CBTIMER_NODE_TIMEOUT_HANDLER(cbtimer_node);

    handler->func_id       = timeout_func_id;
    handler->func_para_num = func_addr_node->func_para_num;
    handler->func_ret_val  = EC_TRUE;

    handler->func_para[ 0 ].para_val = super_md_id;
    handler->func_para[ 1 ].para_val = (UINT32)cstring_new(cstring_get_str(cmd_line), LOC_SUPER_0059);
    handler->func_para[ 2 ].para_val = (UINT32)cbytes_new(0);

    CTIMET_GET(CBTIMER_NODE_START_TIME(cbtimer_node));
    CTIMET_GET(CBTIMER_NODE_LAST_TIME(cbtimer_node));

    cbtimer_register(TASK_BRD_CBTIMER_LIST(task_brd), cbtimer_node);
    return (EC_TRUE);
}

EC_BOOL super_exec_shell_cbtimer_unset(const UINT32 super_md_id, const CSTRING *cbtimer_name)
{
    TASK_BRD     *task_brd;
    CBTIMER_NODE *cbtimer_node;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_exec_shell_cbtimer_unset: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    if(EC_TRUE == cstring_is_empty(cbtimer_name))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_exec_shell_cbtimer_unset:cbtimer_name is empty\n");
        return (EC_FALSE);
    }

    task_brd = task_brd_default_get();
    cbtimer_node = cbtimer_search_by_name(TASK_BRD_CBTIMER_LIST(task_brd), cbtimer_name);


    if(NULL_PTR != cbtimer_node)
    {
        cbtimer_unregister(TASK_BRD_CBTIMER_LIST(task_brd), cbtimer_node);
    }
    return (EC_TRUE);
}

EC_BOOL super_exec_shell_ipaddr_cstr_cbtimer_set(const UINT32 super_md_id, const CSTRING *ipaddr_cstr, const CSTRING *cbtimer_name, const CSTRING *cmd_line, const UINT32 timeout)
{
    UINT32   ipaddr;
    UINT32   tcid;
    MOD_NODE recv_mod_node;

    EC_BOOL ret;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_exec_shell_ipaddr_cstr_cbtimer_set: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    if(EC_TRUE == cstring_is_empty(ipaddr_cstr))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_exec_shell_ipaddr_cstr_cbtimer_set:ipaddr cstr is empty\n");
        return (EC_FALSE);
    }

    if(EC_TRUE == cstring_is_empty(cmd_line))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_exec_shell_ipaddr_cstr_cbtimer_set:cmd_line is empty\n");
        return (EC_FALSE);
    }

    dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "super_exec_shell_ipaddr_cstr_cbtimer_set: execute shell command on ipaddr %s: %s with name %s and timeout %ld\n",
                        (char *)cstring_get_str(ipaddr_cstr),
                        (char *)cstring_get_str(cmd_line),
                        (char *)cstring_get_str(cbtimer_name),
                        timeout
                        );

    ipaddr = c_ipv4_to_word((char *)cstring_get_str(ipaddr_cstr));
    tcid = task_brd_get_tcid_by_ipaddr(task_brd_default_get(), ipaddr);
    if(CMPI_ERROR_TCID == tcid)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_exec_shell_ipaddr_cstr_cbtimer_set: no tcid for ipaddr %s failed\n", (char *)cstring_get_str(ipaddr_cstr));
        return (EC_FALSE);
    }

    if(CMPI_LOCAL_TCID == tcid)
    {
        return super_exec_shell_cbtimer_set(super_md_id, cbtimer_name, cmd_line, timeout);
    }

    if(EC_FALSE == task_brd_check_tcid_connected(task_brd_default_get(), tcid))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_exec_shell_ipaddr_cstr_cbtimer_set: ipaddr %s not connected\n", (char *)cstring_get_str(ipaddr_cstr));
        return (EC_FALSE);
    }

    MOD_NODE_TCID(&recv_mod_node) = tcid;
    MOD_NODE_COMM(&recv_mod_node) = CMPI_ANY_COMM;
    MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
    MOD_NODE_MODI(&recv_mod_node) = 0;

    ret = EC_FALSE;
    task_p2p(super_md_id, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, &recv_mod_node,
                &ret, FI_super_exec_shell_cbtimer_set, ERR_MODULE_ID, cbtimer_name, cmd_line, timeout);
    return (ret);
}

EC_BOOL super_exec_shell_ipaddr_cstr_cbtimer_unset(const UINT32 super_md_id, const CSTRING *ipaddr_cstr, const CSTRING *cbtimer_name)
{
    UINT32   ipaddr;
    UINT32   tcid;
    MOD_NODE recv_mod_node;

    EC_BOOL ret;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_exec_shell_ipaddr_cstr_cbtimer_unset: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    if(EC_TRUE == cstring_is_empty(ipaddr_cstr))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_exec_shell_ipaddr_cstr_cbtimer_unset:ipaddr cstr is empty\n");
        return (EC_FALSE);
    }

    dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "super_exec_shell_ipaddr_cstr_cbtimer_set: cancel cbtimer %s on ipaddr %s\n",
                        (char *)cstring_get_str(cbtimer_name),
                        (char *)cstring_get_str(ipaddr_cstr)
                        );

    ipaddr = c_ipv4_to_word((char *)cstring_get_str(ipaddr_cstr));
    tcid = task_brd_get_tcid_by_ipaddr(task_brd_default_get(), ipaddr);
    if(CMPI_ERROR_TCID == tcid)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_exec_shell_ipaddr_cstr_cbtimer_unset: no tcid for ipaddr %s failed\n", (char *)cstring_get_str(ipaddr_cstr));
        return (EC_FALSE);
    }

    if(CMPI_LOCAL_TCID == tcid)
    {
        return super_exec_shell_cbtimer_unset(super_md_id, cbtimer_name);
    }

    if(EC_FALSE == task_brd_check_tcid_connected(task_brd_default_get(), tcid))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_exec_shell_ipaddr_cstr_cbtimer_unset: ipaddr %s not connected\n", (char *)cstring_get_str(ipaddr_cstr));
        return (EC_FALSE);
    }

    MOD_NODE_TCID(&recv_mod_node) = tcid;
    MOD_NODE_COMM(&recv_mod_node) = CMPI_ANY_COMM;
    MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
    MOD_NODE_MODI(&recv_mod_node) = 0;

    ret = EC_FALSE;
    task_p2p(super_md_id, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, &recv_mod_node,
                &ret, FI_super_exec_shell_cbtimer_unset, ERR_MODULE_ID, cbtimer_name);
    return (ret);
}
/**
*
* show rank load which is used for LOAD_BALANCING_RANK
*
**/
void super_show_rank_load(const UINT32 super_md_id, LOG *log)
{
    TASK_BRD  *task_brd;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_show_rank_load: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();
    cload_mgr_print(log, TASK_BRD_CLOAD_MGR(task_brd));
    return;
}

/**
*
* sync rank load which is used for LOAD_BALANCING_RANK
*
**/
void super_sync_rank_load(const UINT32 super_md_id, const UINT32 tcid, const UINT32 rank)
{
    MOD_MGR   *mod_mgr;

    MOD_NODE   recv_mod_node;
    CLOAD_STAT cload_stat;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_sync_rank_load: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    mod_mgr = mod_mgr_new(super_md_id, LOAD_BALANCING_LOOP);

    /*set mod_mgr*/
    mod_mgr_incl(tcid, CMPI_ANY_COMM, rank, 0, mod_mgr);

#if 1
    if(do_log(SEC_0117_SUPER, 5))
    {
        sys_log(LOGSTDOUT, "------------------------------------ super_sync_rank_load beg ----------------------------------\n");
        mod_mgr_print(LOGSTDOUT, mod_mgr);
        sys_log(LOGSTDOUT, "------------------------------------ super_sync_rank_load end ----------------------------------\n");
    }
#endif

    MOD_NODE_TCID(&recv_mod_node) = tcid;
    MOD_NODE_COMM(&recv_mod_node) = CMPI_ANY_COMM;
    MOD_NODE_RANK(&recv_mod_node) = rank;
    MOD_NODE_MODI(&recv_mod_node) = 0;

    task_super_mono(mod_mgr, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP,
                    &recv_mod_node,
                    NULL_PTR, FI_super_sync_cload_stat, ERR_MODULE_ID, &cload_stat);

    task_brd_rank_load_set(task_brd_default_get(), tcid, rank, &cload_stat);

    return;
}

/**
*
* forcely set rank load which is used for LOAD_BALANCING_RANK
*
**/
void super_set_rank_load(const UINT32 super_md_id, const UINT32 tcid, const UINT32 rank, const CLOAD_STAT *cload_stat)
{
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_set_rank_load: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd_rank_load_set(task_brd_default_get(), tcid, rank, cload_stat);

    return;
}

/**
*
* enable task brd by setting its load to real load
*
**/
void super_enable_task_brd(const UINT32 super_md_id)
{
    TASK_BRD  *task_brd;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_enable_task_brd: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();

    TASK_BRD_ENABLE_FLAG(task_brd) = EC_TRUE;
    return;
}

/**
*
* disable task brd by setting its load to -1
*
**/
void super_disable_task_brd(const UINT32 super_md_id)
{
    TASK_BRD  *task_brd;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_disable_task_brd: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();

    TASK_BRD_ENABLE_FLAG(task_brd) = EC_FALSE;
    return;
}

/**
*
* heartbeat
*
**/
void super_heartbeat_on_node(const UINT32 super_md_id, const CLOAD_NODE *cload_node)
{
    TASK_BRD  *task_brd;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_heartbeat_on_node: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();
    cload_mgr_update(TASK_BRD_CLOAD_MGR(task_brd), cload_node);

    return;
}

void super_heartbeat_on_rank(const UINT32 super_md_id, const UINT32 tcid, const UINT32 rank, const CLOAD_STAT *cload_stat)
{
    TASK_BRD  *task_brd;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_heartbeat_on_rank: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();

    cload_mgr_set(TASK_BRD_CLOAD_MGR(task_brd), tcid, rank, cload_stat);
    return;
}

void super_heartbeat_all(const UINT32 super_md_id, const CLOAD_MGR *cload_mgr)
{
    TASK_BRD  *task_brd;
    CLIST_DATA *clist_data;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_heartbeat_all: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();

    CLIST_LOOP_NEXT(cload_mgr, clist_data)
    {
        CLOAD_NODE *cload_node;
        cload_node = (CLOAD_NODE *)CLIST_DATA_DATA(clist_data);
        cload_mgr_update(TASK_BRD_CLOAD_MGR(task_brd), cload_node);
    }

    return;
}

void super_heartbeat_none(const UINT32 super_md_id)
{
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_heartbeat_none: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    /*do nothing, only socket connection heartbeat was triggered*/
    return;
}

EC_BOOL super_make_license(const UINT32 super_md_id, const CSTRING *mac_cstr,
                                const CSTRING *start_date_str, const CSTRING *end_date_str,
                                const CSTRING *user_name_str , const CSTRING *user_email_str)
{
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_make_license: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    return lic_make((char *)cstring_get_str(mac_cstr),
                    (char *)cstring_get_str(start_date_str), (char *)cstring_get_str(end_date_str),
                    (char *)cstring_get_str(user_name_str), (char *)cstring_get_str(user_email_str)
                    );
}

/**
*
* license
*
**/

EC_BOOL super_check_license(const UINT32 super_md_id)
{
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_check_license: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/
    return lic_check();
}

void super_show_license(const UINT32 super_md_id, LOG *log)
{
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_show_license: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    lic_print(log);
    return;
}

void super_show_version(const UINT32 super_md_id, LOG *log)
{
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_show_version: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    sys_log(log, "version     : %s\n", lic_version());
//    sys_log(log, "vendor name : %s\n", lic_vendor_name());
//    sys_log(log, "vendor email: %s\n", lic_vendor_email());
    return;
}

void super_show_vendor(const UINT32 super_md_id, LOG *log)
{
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_show_vendor: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    sys_log(log, "vendor name : %s\n", lic_vendor_name());
    sys_log(log, "vendor email: %s\n", lic_vendor_email());
    return;
}

/**
*
* OS info
*
**/
UINT32 super_get_wordsize(const UINT32 super_md_id)/*wordsize in bits*/
{
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_get_wordsize: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/
    return ((UINT32)WORDSIZE);
}

void super_show_wordsize(const UINT32 super_md_id, LOG *log)
{
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_show_wordsize: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/
    sys_log(log, "wordsize = %ld\n", (UINT32)WORDSIZE);

    return;
}

/**
*
* download from local disk to remote
*
**/
EC_BOOL super_download(const UINT32 super_md_id, const CSTRING *fname, CBYTES *cbytes)
{
    int fd;
    UINT32 fsize;
    UINT8 *fbuf;
    UINT32 offset;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_download: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    if(0 != access((char *)cstring_get_str(fname), F_OK | R_OK))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_download: inaccessable file %s, errno = %d, errstr = %s\n",
                            (char *)cstring_get_str(fname), errno, strerror(errno));
        return (EC_FALSE);
    }

    fd = c_file_open((char *)cstring_get_str(fname), O_RDONLY, 0666);
    if(-1 == fd)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_download: open file %s to read failed, errno = %d, errstr = %s\n",
                            (char *)cstring_get_str(fname), errno, strerror(errno));
        return (EC_FALSE);
    }

    fsize = lseek(fd, 0, SEEK_END);
    if(ERR_FD == fsize)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_download: seek and get file size of %s failed\n",
                           (char *)cstring_get_str(fname));
        c_file_close(fd);
        return (EC_FALSE);
    }

    fbuf = (UINT8 *)SAFE_MALLOC(fsize, LOC_SUPER_0060);
    if(NULL_PTR == fbuf)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_download: alloc %ld bytes for file %s buffer failed\n",
                            fsize, (char *)cstring_get_str(fname));
        c_file_close(fd);
        return (EC_FALSE);
    }

    offset = 0;
    if(EC_FALSE == c_file_load(fd, &offset, fsize, fbuf))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_download: load %ld bytes from file %s to buffer failed\n",
                            fsize, (char *)cstring_get_str(fname));
        SAFE_FREE(fbuf, LOC_SUPER_0061);
        c_file_close(fd);
        return (EC_FALSE);
    }

    c_file_close(fd);

    cbytes_mount(cbytes, fsize, fbuf);

    return (EC_TRUE);
}

EC_BOOL super_download_tcid_cstr(const UINT32 super_md_id, const CSTRING *tcid_cstr, const CSTRING *fname, CBYTES *output_cbytes)
{
    UINT32    tcid;
    MOD_NODE  recv_mod_node;
    EC_BOOL   ret;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_download_tcid_cstr: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "super_download_tcid_cstr: execute download file %s on tcid %s\n",
                        (char *)cstring_get_str(fname),
                        (char *)cstring_get_str(tcid_cstr));

    tcid = c_ipv4_to_word((char *)cstring_get_str(tcid_cstr));
    if(CMPI_LOCAL_TCID == tcid)
    {
        return super_download(super_md_id, fname, output_cbytes);
    }

    if(EC_FALSE == task_brd_check_tcid_connected(task_brd_default_get(), tcid))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_download_tcid_cstr: tcid %s not connected\n", (char *)cstring_get_str(tcid_cstr));
        return (EC_FALSE);
    }

    MOD_NODE_TCID(&recv_mod_node) = tcid;
    MOD_NODE_COMM(&recv_mod_node) = CMPI_ANY_COMM;
    MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
    MOD_NODE_MODI(&recv_mod_node) = 0;

    ret = EC_FALSE;
    task_p2p(super_md_id, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, &recv_mod_node,
                &ret, FI_super_download, ERR_MODULE_ID, fname, output_cbytes);
    return (ret);
}

/**
*
* download from local disk to remote
*
**/
EC_BOOL super_download_vec_tcid_cstr(const UINT32 super_md_id, const CVECTOR *tcid_cstr_vec, const CSTRING *fname, CVECTOR *output_cbytes_vec)
{
    UINT32 tcid_pos;

    TASK_MGR *task_mgr;
    EC_BOOL   ret;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_download_vec_tcid_cstr: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_mgr = task_new(NULL, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    for(tcid_pos = 0; tcid_pos < cvector_size(tcid_cstr_vec); tcid_pos ++)
    {
        CSTRING *tcid_cstr;
        UINT32   tcid;
        CBYTES  *output_cbytes;

        MOD_NODE recv_mod_node;

        tcid_cstr = (CSTRING *)cvector_get_no_lock(tcid_cstr_vec, tcid_pos);
        tcid      = c_ipv4_to_word((char *)cstring_get_str(tcid_cstr));

        output_cbytes = cbytes_new(0);
        if(NULL_PTR == output_cbytes)
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_download_vec_tcid_cstr: new output cbytes failed\n");
            cvector_clean_with_location_no_lock(output_cbytes_vec, (CVECTOR_DATA_LOCATION_CLEANER)cbytes_free, LOC_SUPER_0062);
            task_mgr_free(task_mgr);
            return (EC_FALSE);
        }

        /*if tcid not reachable, empty cbytes will return*/
        cvector_push_no_lock(output_cbytes_vec, (void *)output_cbytes);

        if(EC_FALSE == task_brd_check_tcid_connected(task_brd_default_get(), tcid))
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_download_vec_tcid_cstr: tcid %s not connected, skip it\n", (char *)cstring_get_str(tcid_cstr));
            continue;
        }

        MOD_NODE_TCID(&recv_mod_node) = tcid;
        MOD_NODE_COMM(&recv_mod_node) = CMPI_ANY_COMM;
        MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
        MOD_NODE_MODI(&recv_mod_node) = 0;

        ret = EC_FALSE;
        task_p2p_inc(task_mgr, super_md_id, &recv_mod_node, &ret, FI_super_download, ERR_MODULE_ID, fname, output_cbytes);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    //dbg_log(SEC_0117_SUPER, 9)(LOGSTDOUT, "[DEBUG] super_download_vec_tcid_cstr: result is\n");
    //cvector_print(LOGSTDOUT, output_cbytes_vec, (CVECTOR_DATA_PRINT)cbytes_print_str);

    return (EC_TRUE);
}

EC_BOOL super_download_ipaddr_cstr(const UINT32 super_md_id, const CSTRING *ipaddr_cstr, const CSTRING *fname, CBYTES *output_cbytes)
{
    UINT32    ipaddr;
    UINT32    tcid;
    TASK_BRD *task_brd;
    MOD_NODE  recv_mod_node;
    EC_BOOL   ret;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_download_ipaddr_cstr: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    if(EC_TRUE == cstring_is_empty(ipaddr_cstr))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_download_ipaddr_cstr:ipaddr cstr is empty\n");
        return (EC_FALSE);
    }

    if(EC_TRUE == cstring_is_empty(fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_download_ipaddr_cstr:fname cstr is empty\n");
        return (EC_FALSE);
    }

    dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "super_download_ipaddr_cstr: execute download file %s on ipaddr %s\n",
                        (char *)cstring_get_str(fname),
                        (char *)cstring_get_str(ipaddr_cstr));

    task_brd = task_brd_default_get();

    ipaddr = c_ipv4_to_word((char *)cstring_get_str(ipaddr_cstr));
    tcid   = task_brd_get_tcid_by_ipaddr(task_brd, ipaddr);
    if(CMPI_ERROR_TCID == tcid)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_download_ipaddr_cstr: no tcid for ipaddr %s\n", (char *)cstring_get_str(ipaddr_cstr));
        return (EC_FALSE);
    }

    if(CMPI_LOCAL_TCID == tcid)
    {
        return super_download(super_md_id, fname, output_cbytes);
    }

    if(EC_FALSE == task_brd_check_tcid_connected(task_brd, tcid))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_download_ipaddr_cstr: ipaddr %s not connected\n", (char *)cstring_get_str(ipaddr_cstr));
        return (EC_FALSE);
    }

    MOD_NODE_TCID(&recv_mod_node) = tcid;
    MOD_NODE_COMM(&recv_mod_node) = CMPI_ANY_COMM;
    MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
    MOD_NODE_MODI(&recv_mod_node) = 0;

    ret = EC_FALSE;
    task_p2p(super_md_id, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, &recv_mod_node,
                &ret, FI_super_download, ERR_MODULE_ID, fname, output_cbytes);
    return (ret);
}

EC_BOOL super_download_vec_ipaddr_cstr(const UINT32 super_md_id, const CVECTOR *ipaddr_cstr_vec, const CSTRING *fname, CVECTOR *output_cbytes_vec)
{
    UINT32 ipaddr_pos;

    TASK_BRD *task_brd;
    TASK_MGR *task_mgr;
    EC_BOOL   ret;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_download_vec_ipaddr_cstr: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/
    if(EC_TRUE == cstring_is_empty(fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_download_vec_ipaddr_cstr:fname cstr is empty\n");
        return (EC_FALSE);
    }

    task_brd = task_brd_default_get();

    task_mgr = task_new(NULL, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    for(ipaddr_pos = 0; ipaddr_pos < cvector_size(ipaddr_cstr_vec); ipaddr_pos ++)
    {
        CSTRING *ipaddr_cstr;
        UINT32   ipaddr;
        UINT32   tcid;
        CBYTES  *output_cbytes;

        MOD_NODE recv_mod_node;

        ipaddr_cstr = (CSTRING *)cvector_get_no_lock(ipaddr_cstr_vec, ipaddr_pos);
        ipaddr      = c_ipv4_to_word((char *)cstring_get_str(ipaddr_cstr));
        tcid        = task_brd_get_tcid_by_ipaddr(task_brd, ipaddr);

        output_cbytes = cbytes_new(0);
        if(NULL_PTR == output_cbytes)
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_download_vec_ipaddr_cstr: new output cbytes failed\n");
            cvector_clean_with_location_no_lock(output_cbytes_vec, (CVECTOR_DATA_LOCATION_CLEANER)cbytes_free, LOC_SUPER_0063);
            task_mgr_free(task_mgr);
            return (EC_FALSE);
        }

        /*if ipaddr not reachable, empty cbytes will return*/
        cvector_push_no_lock(output_cbytes_vec, (void *)output_cbytes);

        if(CMPI_ERROR_TCID == tcid)
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_download_vec_ipaddr_cstr: no tcid for ipaddr %s\n", (char *)cstring_get_str(ipaddr_cstr));
            continue;
        }

        if(EC_FALSE == task_brd_check_tcid_connected(task_brd, tcid))
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_download_vec_ipaddr_cstr: ipaddr %s not connected, skip it\n", (char *)cstring_get_str(ipaddr_cstr));
            continue;
        }

        MOD_NODE_TCID(&recv_mod_node) = tcid;
        MOD_NODE_COMM(&recv_mod_node) = CMPI_ANY_COMM;
        MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
        MOD_NODE_MODI(&recv_mod_node) = 0;

        ret = EC_FALSE;
        task_p2p_inc(task_mgr, super_md_id, &recv_mod_node, &ret, FI_super_download, ERR_MODULE_ID, fname, output_cbytes);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    //dbg_log(SEC_0117_SUPER, 9)(LOGSTDOUT, "[DEBUG] super_download_vec_ipaddr_cstr: result is\n");
    //cvector_print(LOGSTDOUT, output_cbytes_vec, (CVECTOR_DATA_PRINT)cbytes_print_str);

    return (EC_TRUE);
}

EC_BOOL super_backup(const UINT32 super_md_id, const CSTRING *fname)
{
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_backup: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/
    if(EC_TRUE == cstring_is_empty(fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_backup:fname cstr is empty\n");
        return (EC_FALSE);
    }

    if(0 == access((char *)cstring_get_str(fname), F_OK))
    {
        CSTRING *cmd_line;

        cmd_line = cstring_new(NULL_PTR, LOC_SUPER_0064);
        if(NULL_PTR == cmd_line)
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_backup: new cmd line failed\n");
            return (EC_FALSE);
        }

        cstring_format(cmd_line, "cp -p %s %s.bk",
                        (char *)cstring_get_str(fname), (char *)cstring_get_str(fname));
        if(EC_FALSE == exec_shell((char *)cstring_get_str(cmd_line), NULL_PTR, 0))
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_backup: exec cmd %s failed\n", (char *)cstring_get_str(cmd_line));
            cstring_free(cmd_line);
            return (EC_FALSE);
        }
        cstring_free(cmd_line);
    }

    return (EC_TRUE);
}

/**
*
* upload from remote to local disk
*
**/
EC_BOOL super_upload(const UINT32 super_md_id, const CSTRING *fname, const CBYTES *cbytes, const UINT32 backup_flag)
{
    int fd;
    UINT32 fsize;
    UINT8 *fbuf;
    UINT32 offset;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_upload: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*( SWITCH_ON == SUPER_DEBUG_SWITCH )*/
    if(EC_TRUE == cstring_is_empty(fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_upload:fname cstr is empty\n");
        return (EC_FALSE);
    }

    if(EC_TRUE == backup_flag)
    {
        if(EC_FALSE == super_backup(super_md_id, fname))
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_upload: backup file %s failed\n", (char *)cstring_get_str(fname));
            return (EC_FALSE);
        }
    }

    if(EC_FALSE == super_rmv_file(super_md_id, fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_upload: rmv file %s failed\n", (char *)cstring_get_str(fname));
        return (EC_FALSE);
    }

    fsize = cbytes_len(cbytes);
    if(0 == fsize)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_upload: fsize is zero\n");
        return (EC_FALSE);
    }

    fbuf = cbytes_buf(cbytes);
    if(NULL_PTR == fbuf)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_upload: buf is null\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == c_basedir_create((char *)cstring_get_str(fname)))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_upload: create basedir of file %s failed\n", (char *)cstring_get_str(fname));
        return (EC_FALSE);
    }

    fd = c_file_open((char *)cstring_get_str(fname), O_WRONLY | O_CREAT, 0666);
    if(-1 == fd)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_upload: open file %s to write failed, errno = %d, errstr = %s\n",
                            (char *)cstring_get_str(fname), errno, strerror(errno));
        return (EC_FALSE);
    }

    offset = 0;
    if(EC_FALSE == c_file_flush(fd, &offset, fsize, fbuf))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_upload: flush %ld bytes to file %s failed\n",
                            fsize, (char *)cstring_get_str(fname));
        c_file_close(fd);
        super_rmv_file(super_md_id, fname);/*remove it*/
        return (EC_FALSE);
    }

    c_file_close(fd);

    return (EC_TRUE);
}

EC_BOOL super_upload_tcid_cstr(const UINT32 super_md_id, const CSTRING *tcid_cstr, const CSTRING *fname, const CBYTES *input_cbytes, const UINT32 backup_flag)
{
    UINT32    tcid;
    MOD_NODE  recv_mod_node;
    EC_BOOL   ret;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_upload_tcid_cstr: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "super_upload_tcid_cstr: execute upload command on tcid %s: %s\n",
                        (char *)cstring_get_str(tcid_cstr),
                        (char *)cstring_get_str(fname));

    tcid = c_ipv4_to_word((char *)cstring_get_str(tcid_cstr));
    if(CMPI_LOCAL_TCID == tcid)
    {
        return super_upload(super_md_id, fname, input_cbytes, backup_flag);
    }

    if(EC_FALSE == task_brd_check_tcid_connected(task_brd_default_get(), tcid))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_upload_tcid_cstr: tcid %s not connected\n", (char *)cstring_get_str(tcid_cstr));
        return (EC_FALSE);
    }

    MOD_NODE_TCID(&recv_mod_node) = tcid;
    MOD_NODE_COMM(&recv_mod_node) = CMPI_ANY_COMM;
    MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
    MOD_NODE_MODI(&recv_mod_node) = 0;

    ret = EC_FALSE;
    task_p2p(super_md_id, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, &recv_mod_node,
                &ret, FI_super_upload, ERR_MODULE_ID, fname, input_cbytes, backup_flag);
    return (ret);
}

EC_BOOL super_upload_vec_tcid_cstr(const UINT32 super_md_id, const CVECTOR *tcid_cstr_vec, const CSTRING *fname, const CBYTES *input_cbytes, const UINT32 backup_flag, CVECTOR *ret_vec)
{
    UINT32 pos;
    UINT32 tcid_pos;

    TASK_MGR *task_mgr;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_upload_vec_tcid_cstr: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/
    for(pos = 0; pos < cvector_size(ret_vec); pos ++)
    {
        cvector_set_no_lock(ret_vec, pos, (void *)EC_FALSE);
    }

    for(;pos < cvector_size(tcid_cstr_vec); pos ++)
    {
        cvector_push_no_lock(ret_vec, (void *)EC_FALSE);
    }

    task_mgr = task_new(NULL, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    for(tcid_pos = 0; tcid_pos < cvector_size(tcid_cstr_vec); tcid_pos ++)
    {
        CSTRING *tcid_cstr;
        UINT32   tcid;

        MOD_NODE recv_mod_node;
        UINT32  *ret;

        tcid_cstr = (CSTRING *)cvector_get_no_lock(tcid_cstr_vec, tcid_pos);
        tcid      = c_ipv4_to_word((char *)cstring_get_str(tcid_cstr));

        ret = (UINT32 *)cvector_get_addr_no_lock(ret_vec, tcid_pos);
        if(EC_FALSE == task_brd_check_tcid_connected(task_brd_default_get(), tcid))
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_upload_vec_tcid_cstr: tcid %s not connected, skip it\n", (char *)cstring_get_str(tcid_cstr));
            continue;
        }

        MOD_NODE_TCID(&recv_mod_node) = tcid;
        MOD_NODE_COMM(&recv_mod_node) = CMPI_ANY_COMM;
        MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
        MOD_NODE_MODI(&recv_mod_node) = 0;

        task_p2p_inc(task_mgr, super_md_id, &recv_mod_node, ret, FI_super_upload, ERR_MODULE_ID, fname, input_cbytes, backup_flag);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);


    return (EC_TRUE);
}

EC_BOOL super_upload_ipaddr_cstr(const UINT32 super_md_id, const CSTRING *ipaddr_cstr, const CSTRING *fname, const CBYTES *input_cbytes, const UINT32 backup_flag)
{
    UINT32    ipaddr;
    UINT32    tcid;
    TASK_BRD *task_brd;
    MOD_NODE  recv_mod_node;
    EC_BOOL   ret;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_upload_ipaddr_cstr: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/
    if(EC_TRUE == cstring_is_empty(ipaddr_cstr))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_upload_ipaddr_cstr:ipaddr cstr is empty\n");
        return (EC_FALSE);
    }

    if(EC_TRUE == cstring_is_empty(fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_upload_ipaddr_cstr:fname cstr is empty\n");
        return (EC_FALSE);
    }

    dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "super_upload_ipaddr_cstr: execute upload command on ipaddr %s: %s\n",
                        (char *)cstring_get_str(ipaddr_cstr),
                        (char *)cstring_get_str(fname));

    task_brd = task_brd_default_get();

    ipaddr = c_ipv4_to_word((char *)cstring_get_str(ipaddr_cstr));
    tcid   = task_brd_get_tcid_by_ipaddr(task_brd, ipaddr);

    if(CMPI_ERROR_TCID == tcid)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_upload_ipaddr_cstr: no tcid for ipaddr %s\n", (char *)cstring_get_str(ipaddr_cstr));
        return (EC_FALSE);
    }

    if(CMPI_LOCAL_TCID == tcid)
    {
        return super_upload(super_md_id, fname, input_cbytes, backup_flag);
    }

    if(EC_FALSE == task_brd_check_tcid_connected(task_brd, tcid))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_upload_ipaddr_cstr: ipaddr %s not connected\n", (char *)cstring_get_str(ipaddr_cstr));
        return (EC_FALSE);
    }

    MOD_NODE_TCID(&recv_mod_node) = tcid;
    MOD_NODE_COMM(&recv_mod_node) = CMPI_ANY_COMM;
    MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
    MOD_NODE_MODI(&recv_mod_node) = 0;

    ret = EC_FALSE;
    task_p2p(super_md_id, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, &recv_mod_node,
                &ret, FI_super_upload, ERR_MODULE_ID, fname, input_cbytes, backup_flag);
    return (ret);
}

EC_BOOL super_upload_vec_ipaddr_cstr(const UINT32 super_md_id, const CVECTOR *ipaddr_cstr_vec, const CSTRING *fname, const CBYTES *input_cbytes, const UINT32 backup_flag, CVECTOR *ret_vec)
{
    UINT32 pos;
    UINT32 ipaddr_pos;

    TASK_BRD *task_brd;
    TASK_MGR *task_mgr;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_upload_vec_ipaddr_cstr: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/
    if(EC_TRUE == cstring_is_empty(fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_upload_vec_ipaddr_cstr:fname cstr is empty\n");
        return (EC_FALSE);
    }

    for(pos = 0; pos < cvector_size(ret_vec); pos ++)
    {
        cvector_set_no_lock(ret_vec, pos, (void *)EC_FALSE);
    }

    for(;pos < cvector_size(ipaddr_cstr_vec); pos ++)
    {
        cvector_push_no_lock(ret_vec, (void *)EC_FALSE);
    }

    task_brd = task_brd_default_get();

    task_mgr = task_new(NULL, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    for(ipaddr_pos = 0; ipaddr_pos < cvector_size(ipaddr_cstr_vec); ipaddr_pos ++)
    {
        CSTRING *ipaddr_cstr;
        UINT32   ipaddr;
        UINT32   tcid;
        EC_BOOL *ret;

        MOD_NODE recv_mod_node;

        ipaddr_cstr = (CSTRING *)cvector_get_no_lock(ipaddr_cstr_vec, ipaddr_pos);
        ipaddr      = c_ipv4_to_word((char *)cstring_get_str(ipaddr_cstr));
        tcid        = task_brd_get_tcid_by_ipaddr(task_brd, ipaddr);

        ret = (UINT32 *)cvector_get_addr_no_lock(ret_vec, ipaddr_pos);
        //dbg_log(SEC_0117_SUPER, 9)(LOGSTDOUT, "[DEBUG] super_upload_vec_ipaddr_cstr: ret = %ld (%lx) <== %lx\n", (*ret), (*ret), ret);

        if(CMPI_ERROR_TCID == tcid)
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_upload_vec_ipaddr_cstr: no tcid for ipaddr %s\n", (char *)cstring_get_str(ipaddr_cstr));
            continue;
        }

        if(EC_FALSE == task_brd_check_tcid_connected(task_brd_default_get(), tcid))
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_upload_vec_ipaddr_cstr: ipaddr %s not connected, skip it\n", (char *)cstring_get_str(ipaddr_cstr));
            continue;
        }

        MOD_NODE_TCID(&recv_mod_node) = tcid;
        MOD_NODE_COMM(&recv_mod_node) = CMPI_ANY_COMM;
        MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
        MOD_NODE_MODI(&recv_mod_node) = 0;

        task_p2p_inc(task_mgr, super_md_id, &recv_mod_node, ret, FI_super_upload, ERR_MODULE_ID, fname, input_cbytes, backup_flag);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    return (EC_TRUE);
}

EC_BOOL super_collect_vec_ipaddr_cstr(const UINT32 super_md_id, CVECTOR *ipaddr_cstr_vec)
{
    TASK_BRD *task_brd;
    CVECTOR  *ipaddr_vec;

    UINT32 pos;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_collect_vec_ipaddr_cstr: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();
    ipaddr_vec = cvector_new(0, MM_UINT32, LOC_SUPER_0065);
    if(NULL_PTR == ipaddr_vec)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_collect_vec_ipaddr_cstr: new cvector failed\n");
        return (EC_FALSE);
    }

    task_brd_collect_ipaddr(task_brd, ipaddr_vec);

    for(pos = 0; pos < cvector_size(ipaddr_vec); pos ++)
    {
        UINT32 ipaddr;
        CSTRING *ipaddr_cstr;

        ipaddr = (UINT32)cvector_get_no_lock(ipaddr_vec, pos);
        ipaddr_cstr = (CSTRING *)cstring_new((UINT8 *)c_word_to_ipv4(ipaddr), LOC_SUPER_0066);
        if(NULL_PTR == ipaddr_cstr)
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_collect_vec_ipaddr_cstr: new cstring failed\n");
            cvector_free(ipaddr_vec, LOC_SUPER_0067);
            return (EC_FALSE);
        }
        cvector_push_no_lock(ipaddr_cstr_vec, (void *)ipaddr_cstr);
    }
    cvector_free_no_lock(ipaddr_vec, LOC_SUPER_0068);
    return (EC_TRUE);
}

EC_BOOL super_write_fdata(const UINT32 super_md_id, const CSTRING *fname, const UINT32 offset, const CBYTES *cbytes)
{
    SUPER_FNODE *super_fnode;
    UINT32 cur_fsize;
    UINT32 write_offset;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_write_fdata: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/
    if(EC_TRUE == cstring_is_empty(fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_write_fdata:fname cstr is empty\n");
        return (EC_FALSE);
    }

    super_fnode = super_search_fnode_by_fname(super_md_id, fname);
    if(NULL_PTR == super_fnode)
    {
        super_fnode = super_open_fnode_by_fname(super_md_id, fname, SUPER_O_WRONLY | SUPER_O_CREAT);
    }

    if(NULL_PTR == super_fnode)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_write_fdata: open file %s failed\n", (char *)cstring_get_str(fname));
        return (EC_FALSE);
    }

    SUPER_FNODE_CMUTEX_LOCK(super_fnode, LOC_SUPER_0069);

    if(EC_FALSE == c_file_size(SUPER_FNODE_FD(super_fnode), &cur_fsize))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_write_fdata: get size of file %s failed\n", (char *)cstring_get_str(fname));
        SUPER_FNODE_CMUTEX_UNLOCK(super_fnode, LOC_SUPER_0070);
        return (EC_FALSE);
    }

    /*warning:not support disordered data writting!*/
    if(cur_fsize != offset && EC_FALSE == c_file_truncate(SUPER_FNODE_FD(super_fnode), offset))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_write_fdata: truncate file %s to %ld bytes failed\n", (char *)cstring_get_str(fname), offset);
        SUPER_FNODE_CMUTEX_UNLOCK(super_fnode, LOC_SUPER_0071);
        return (EC_FALSE);
    }

    write_offset = offset;
    if(EC_FALSE == c_file_flush(SUPER_FNODE_FD(super_fnode), &write_offset, cbytes_len(cbytes), cbytes_buf(cbytes)))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_write_fdata: flush %ld bytes at offset %ld of file %s failed\n",
                            cbytes_len(cbytes), write_offset, (char *)cstring_get_str(fname));
        SUPER_FNODE_CMUTEX_UNLOCK(super_fnode, LOC_SUPER_0072);
        return (EC_FALSE);
    }

    SUPER_FNODE_CMUTEX_UNLOCK(super_fnode, LOC_SUPER_0073);

    return (EC_TRUE);
}

EC_BOOL super_read_fdata(const UINT32 super_md_id, const CSTRING *fname, const UINT32 offset, const UINT32 max_len, CBYTES *cbytes)
{
    SUPER_FNODE *super_fnode;
    UINT32 read_offset;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_read_fdata: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/
    if(EC_TRUE == cstring_is_empty(fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_read_fdata:fname cstr is empty\n");
        return (EC_FALSE);
    }

    super_fnode = super_search_fnode_by_fname(super_md_id, fname);
    if(NULL_PTR == super_fnode)
    {
        super_fnode = super_open_fnode_by_fname(super_md_id, fname, SUPER_O_RDONLY);
    }

    if(NULL_PTR == super_fnode)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_read_fdata: open file %s failed\n", (char *)cstring_get_str(fname));
        return (EC_FALSE);
    }

    if(EC_FALSE == cbytes_resize(cbytes, max_len))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_read_fdata: cbytes resize to len %ld failed\n", max_len);
        return (EC_FALSE);
    }

    read_offset = offset;
    SUPER_FNODE_CMUTEX_LOCK(super_fnode, LOC_SUPER_0074);
    if(EC_FALSE == c_file_load(SUPER_FNODE_FD(super_fnode), &read_offset, max_len, cbytes_buf(cbytes)))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_read_fdata: load %ld bytes at offset %ld of file %s failed\n",
                            max_len, read_offset, (char *)cstring_get_str(fname));
        SUPER_FNODE_CMUTEX_UNLOCK(super_fnode, LOC_SUPER_0075);
        return (EC_FALSE);
    }

    SUPER_FNODE_CMUTEX_UNLOCK(super_fnode, LOC_SUPER_0076);

    return (EC_TRUE);
}

EC_BOOL super_set_progress(const UINT32 super_md_id, const CSTRING *fname, const REAL *progress)
{
    SUPER_FNODE *super_fnode;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_set_progress: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/
    if(EC_TRUE == cstring_is_empty(fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_set_progress:fname cstr is empty\n");
        return (EC_FALSE);
    }

    super_fnode = super_search_fnode_by_fname(super_md_id, fname);
    if(NULL_PTR == super_fnode)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_set_progress: not searched file %s\n", (char *)cstring_get_str(fname));
        return (EC_FALSE);
    }
    SUPER_FNODE_PROGRESS(super_fnode) = (*progress);
    return (EC_TRUE);
}

EC_BOOL super_get_progress(const UINT32 super_md_id, const CSTRING *fname, REAL *progress)
{
    SUPER_FNODE *super_fnode;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_get_progress: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/
    if(EC_TRUE == cstring_is_empty(fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_get_progress:fname cstr is empty\n");
        return (EC_FALSE);
    }

    super_fnode = super_search_fnode_by_fname(super_md_id, fname);
    if(NULL_PTR == super_fnode)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_get_progress: not searched file %s\n", (char *)cstring_get_str(fname));
        /*when the file was not on transfering, regard its progress is 100% */
        (*progress) = 1.0;
        return (EC_FALSE);
    }

    (*progress) = SUPER_FNODE_PROGRESS(super_fnode);
    return (EC_TRUE);
}

EC_BOOL super_size_file(const UINT32 super_md_id, const CSTRING *fname, UINT32 *fsize)
{
    SUPER_FNODE *super_fnode;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_size_file: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/
    if(EC_TRUE == cstring_is_empty(fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_size_file:fname cstr is empty\n");
        return (EC_FALSE);
    }

    /*search cached*/
    super_fnode = super_search_fnode_by_fname(super_md_id, fname);
    if(NULL_PTR != super_fnode)
    {
        return c_file_size(SUPER_FNODE_FD(super_fnode), fsize);
    }

    return (EC_FALSE);
}

EC_BOOL super_open_file(const UINT32 super_md_id, const CSTRING *fname, const UINT32 open_flags)
{
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_open_file: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/
    if(EC_TRUE == cstring_is_empty(fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_open_file:fname cstr is empty\n");
        return (EC_FALSE);
    }

    if(NULL_PTR == super_open_fnode_by_fname(super_md_id, fname, open_flags))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_open_file:open file %s with flags %lx failed\n",
                            (char *)cstring_get_str(fname), open_flags);
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

EC_BOOL super_close_file(const UINT32 super_md_id, const CSTRING *fname)
{
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_close_file: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/
    if(EC_TRUE == cstring_is_empty(fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_close_file:fname cstr is empty\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == super_close_fnode_by_fname(super_md_id, fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_close_file:c_file_close file %s failed\n",
                            (char *)cstring_get_str(fname));
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

EC_BOOL super_rmv_file(const UINT32 super_md_id, const CSTRING *fname)
{
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_rmv_file: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    if(EC_TRUE == cstring_is_empty(fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_rmv_file:fname cstr is empty\n");
        return (EC_FALSE);
    }

    if(0 != access((char *)cstring_get_str(fname), F_OK))
    {
        dbg_log(SEC_0117_SUPER, 9)(LOGSTDOUT, "[DEBUG] super_rmv_file: not exist file %s\n", (char *)cstring_get_str(fname));
        return (EC_TRUE);
    }

    if(0 != access((char *)cstring_get_str(fname), R_OK))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_rmv_file: not readable file %s, errno = %d, errstr = %s\n",
                            (char *)cstring_get_str(fname), errno, strerror(errno));
        return (EC_FALSE);
    }

    if(0 != unlink((char *)cstring_get_str(fname)))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_rmv_file: unlink file %s failed, errno = %d, errstr = %s\n",
                            (char *)cstring_get_str(fname), errno, strerror(errno));
        return (EC_FALSE);
    }

    dbg_log(SEC_0117_SUPER, 9)(LOGSTDOUT, "[DEBUG] super_rmv_file: removed file %s\n", (char *)cstring_get_str(fname));
    return (EC_TRUE);
}

EC_BOOL super_transfer_start(const UINT32 super_md_id, const CSTRING *src_fname, const UINT32 des_tcid, const CSTRING *des_fname)
{
    MOD_NODE recv_mod_node;
    EC_BOOL  ret;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_transfer_start: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/
    if(EC_TRUE == cstring_is_empty(src_fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer_start:src_fname cstr is empty\n");
        return (EC_FALSE);
    }

    if(EC_TRUE == cstring_is_empty(des_fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer_start:des_fname cstr is empty\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == super_open_file(super_md_id, src_fname, SUPER_O_RDONLY))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer_start: open src file %s failed\n",
                           (char *)cstring_get_str(src_fname));
        return (EC_FALSE);
    }

    MOD_NODE_TCID(&recv_mod_node) = des_tcid;
    MOD_NODE_COMM(&recv_mod_node) = CMPI_LOCAL_COMM;
    MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
    MOD_NODE_MODI(&recv_mod_node) = 0;

    ret = EC_FALSE;
    task_p2p(super_md_id, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP,
                    &recv_mod_node,
                    &ret, FI_super_open_file, ERR_MODULE_ID, des_fname, SUPER_O_WRONLY | SUPER_O_CREAT);
    if(EC_FALSE == ret)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer_start: open des file %s failed\n",
                           (char *)cstring_get_str(des_fname));
        super_close_file(super_md_id, src_fname);
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL super_transfer_stop(const UINT32 super_md_id, const CSTRING *src_fname, const UINT32 des_tcid, const CSTRING *des_fname)
{
    MOD_NODE recv_mod_node;
    EC_BOOL  ret_src;
    EC_BOOL  ret_des;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_transfer_stop: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/
    if(EC_TRUE == cstring_is_empty(src_fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer_stop:src_fname cstr is empty\n");
        return (EC_FALSE);
    }

    if(EC_TRUE == cstring_is_empty(des_fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer_stop:des_fname cstr is empty\n");
        return (EC_FALSE);
    }

    ret_src = EC_TRUE;
    if(EC_FALSE == super_close_file(super_md_id, src_fname))
    {
        ret_src = EC_FALSE;
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer_stop: c_file_close src file %s failed\n",
                           (char *)cstring_get_str(src_fname));
    }

    MOD_NODE_TCID(&recv_mod_node) = des_tcid;
    MOD_NODE_COMM(&recv_mod_node) = CMPI_LOCAL_COMM;
    MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
    MOD_NODE_MODI(&recv_mod_node) = 0;

    ret_des = EC_FALSE;
    task_p2p(CMPI_ANY_MODI, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP,
                    &recv_mod_node,
                    &ret_des, FI_super_close_file, ERR_MODULE_ID, des_fname);
    if(EC_FALSE == ret_des)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer_stop: c_file_close des file %s failed\n",
                           (char *)cstring_get_str(des_fname));
    }

    if(EC_FALSE == ret_src || EC_FALSE == ret_des)
    {
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL super_transfer(const UINT32 super_md_id, const CSTRING *src_fname, const UINT32 des_tcid, const CSTRING *des_fname)
{
    UINT32 csize;/*read completed size*/
    UINT32 osize;/*read once size*/
    UINT32 rsize;
    MOD_NODE recv_mod_node;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_transfer: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/
    if(EC_TRUE == cstring_is_empty(src_fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer:src_fname cstr is empty\n");
        return (EC_FALSE);
    }

    if(EC_TRUE == cstring_is_empty(des_fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer:des_fname cstr is empty\n");
        return (EC_FALSE);
    }

    /*when transfer file on current host to the same same, ignore it*/
    if(CMPI_LOCAL_TCID == des_tcid && EC_TRUE == cstring_is_equal(src_fname, des_fname))
    {
        return (EC_TRUE);
    }

    if(EC_FALSE == super_transfer_start(super_md_id, src_fname, des_tcid, des_fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer: start transfer failed\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == super_size_file(super_md_id, src_fname, &rsize))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer: get size of file %s failed\n", (char *)cstring_get_str(src_fname));
        super_transfer_stop(super_md_id, src_fname, des_tcid, des_fname);
        return (EC_FALSE);
    }

    MOD_NODE_TCID(&recv_mod_node) = des_tcid;
    MOD_NODE_COMM(&recv_mod_node) = CMPI_LOCAL_COMM;
    MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
    MOD_NODE_MODI(&recv_mod_node) = 0;

    for(csize = 0, osize = SUPER_READ_ONCE_MAX_BYTES; csize < rsize; csize += osize)
    {
        CBYTES *cbytes;
        EC_BOOL ret;
        REAL    progress;

        if(csize + osize > rsize)
        {
            osize = rsize - csize;
        }

        cbytes = cbytes_new(osize);
        if(NULL_PTR == cbytes)
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer: new cbytes with len %ld failed\n", osize);
            super_transfer_stop(super_md_id, src_fname, des_tcid, des_fname);
            return (EC_FALSE);
        }

        if(EC_FALSE == super_read_fdata(super_md_id, src_fname, csize, osize, cbytes))
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer: read %ld bytes at offset %ld of src file %s failed\n",
                                osize, csize, (char *)cstring_get_str(src_fname));
            cbytes_free(cbytes, LOC_SUPER_0077);
            super_transfer_stop(super_md_id, src_fname, des_tcid, des_fname);
            return (EC_FALSE);
        }

        ret = EC_FALSE;
        task_p2p(CMPI_ANY_MODI, TASK_ALWAYS_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP,
                        &recv_mod_node,
                        &ret, FI_super_write_fdata, ERR_MODULE_ID, des_fname, csize, cbytes);
        if(EC_FALSE == ret)
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_write_fdata: write %ld bytes at offset %ld of des file %s failed\n",
                               osize, csize, (char *)cstring_get_str(des_fname));
            cbytes_free(cbytes, LOC_SUPER_0078);
            super_transfer_stop(super_md_id, src_fname, des_tcid, des_fname);
            return (EC_FALSE);
        }

        cbytes_free(cbytes, LOC_SUPER_0079);

        progress = (csize + 0.0) / (rsize + 0.0);
        super_set_progress(super_md_id, src_fname, &progress);
    }

    if(EC_FALSE == super_transfer_stop(super_md_id, src_fname, des_tcid, des_fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer: stop transfer failed\n");
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL super_transfer_ipaddr_cstr(const UINT32 super_md_id, const CSTRING *src_fname, const CSTRING *ipaddr_cstr, const CSTRING *des_fname)
{
    UINT32    ipaddr;
    UINT32    tcid;
    TASK_BRD *task_brd;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_transfer_ipaddr_cstr: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    if(EC_TRUE == cstring_is_empty(src_fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer_ipaddr_cstr:src_fname is empty\n");
        return (EC_FALSE);
    }

    if(EC_TRUE == cstring_is_empty(des_fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer_ipaddr_cstr:des_fname is empty\n");
        return (EC_FALSE);
    }

    if(EC_TRUE == cstring_is_empty(ipaddr_cstr))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer_ipaddr_cstr:ipaddr cstr is empty\n");
        return (EC_FALSE);
    }

    task_brd = task_brd_default_get();

    ipaddr = c_ipv4_to_word((char *)cstring_get_str(ipaddr_cstr));
    tcid   = task_brd_get_tcid_by_ipaddr(task_brd, ipaddr);

    if(CMPI_ERROR_TCID == tcid)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer_ipaddr_cstr: no tcid for ipaddr %s\n", (char *)cstring_get_str(ipaddr_cstr));
        return (EC_FALSE);
    }

    return super_transfer(super_md_id, src_fname, tcid, des_fname);
}

EC_BOOL super_transfer_vec_start(const UINT32 super_md_id, const CSTRING *src_fname, const CVECTOR *des_tcid_vec, const CSTRING *des_fname)
{
    UINT32   des_tcid_pos;
    EC_BOOL  ret;

    TASK_MGR *task_mgr;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_transfer_vec_start: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/
    if(EC_TRUE == cstring_is_empty(src_fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer_vec_start:src_fname is empty\n");
        return (EC_FALSE);
    }

    if(EC_TRUE == cstring_is_empty(des_fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer_vec_start:des_fname is empty\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == super_open_file(super_md_id, src_fname, SUPER_O_RDONLY))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer_vec_start: open src file %s failed\n",
                           (char *)cstring_get_str(src_fname));
        return (EC_FALSE);
    }

    task_mgr = task_new(NULL, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    for(des_tcid_pos = 0; des_tcid_pos < cvector_size(des_tcid_vec); des_tcid_pos ++)
    {
        UINT32 des_tcid;
        MOD_NODE recv_mod_node;

        des_tcid = (UINT32)cvector_get_no_lock(des_tcid_vec, des_tcid_pos);
        /*when transfer file on current host to the same same, ignore it*/
        if(CMPI_LOCAL_TCID == des_tcid && EC_TRUE == cstring_is_equal(src_fname, des_fname))
        {
            continue;
        }

        MOD_NODE_TCID(&recv_mod_node) = des_tcid;
        MOD_NODE_COMM(&recv_mod_node) = CMPI_LOCAL_COMM;
        MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
        MOD_NODE_MODI(&recv_mod_node) = 0;

        ret = EC_FALSE;
        task_p2p_inc(task_mgr, super_md_id,
                    &recv_mod_node,
                    &ret, FI_super_open_file, ERR_MODULE_ID, des_fname, SUPER_O_WRONLY | SUPER_O_CREAT);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    return (EC_TRUE);
}

EC_BOOL super_transfer_vec_stop(const UINT32 super_md_id, const CSTRING *src_fname, const CVECTOR *des_tcid_vec, const CSTRING *des_fname)
{
    UINT32   des_tcid_pos;
    EC_BOOL  ret;

    TASK_MGR *task_mgr;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_transfer_vec_stop: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    if(EC_TRUE == cstring_is_empty(src_fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer_vec_stop:src_fname is empty\n");
        return (EC_FALSE);
    }

    if(EC_TRUE == cstring_is_empty(des_fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer_vec_stop:des_fname is empty\n");
        return (EC_FALSE);
    }

    task_mgr = task_new(NULL, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    for(des_tcid_pos = 0; des_tcid_pos < cvector_size(des_tcid_vec); des_tcid_pos ++)
    {
        UINT32 des_tcid;
        MOD_NODE recv_mod_node;

        des_tcid = (UINT32)cvector_get_no_lock(des_tcid_vec, des_tcid_pos);
        /*when transfer file on current host to the same same, ignore it*/
        if(CMPI_LOCAL_TCID == des_tcid && EC_TRUE == cstring_is_equal(src_fname, des_fname))
        {
            continue;
        }

        MOD_NODE_TCID(&recv_mod_node) = des_tcid;
        MOD_NODE_COMM(&recv_mod_node) = CMPI_LOCAL_COMM;
        MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
        MOD_NODE_MODI(&recv_mod_node) = 0;

        ret = EC_FALSE;
        task_p2p_inc(task_mgr, super_md_id,
                    &recv_mod_node,
                    &ret, FI_super_close_file, ERR_MODULE_ID, des_fname);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);


    if(EC_FALSE == super_close_file(super_md_id, src_fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer_vec_stop: c_file_close src file %s failed\n",
                           (char *)cstring_get_str(src_fname));
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

static EC_BOOL __super_transfer_prepare(const CVECTOR *des_tcid_vec, CVECTOR *ret_vec)
{
    UINT32 pos;

    for(pos = 0; pos < cvector_size(ret_vec); pos ++)
    {
        UINT32 des_tcid;
        des_tcid = (UINT32)cvector_get_no_lock(des_tcid_vec, pos);
#if 0
        /*check connectivity*/
        if(EC_FALSE == task_brd_check_tcid_connected(task_brd_default_get(), des_tcid))
        {
            dbg_log(SEC_0117_SUPER, 1)(LOGSTDOUT, "warn:__super_transfer_prepare: tcid %s not connected, skip it\n", c_word_to_ipv4(des_tcid));
            cvector_set_no_lock(ret_vec, pos, (void *)EC_FALSE);
        }
        else
        {
            cvector_set_no_lock(ret_vec, pos, (void *)EC_TRUE);
        }
#endif

#if 1
        if(CMPI_ERROR_TCID == des_tcid)
        {
            cvector_set_no_lock(ret_vec, pos, (void *)EC_FALSE);
        }
        else
        {
            cvector_set_no_lock(ret_vec, pos, (void *)EC_TRUE);
        }
#endif
    }

    for(pos = cvector_size(ret_vec); pos < cvector_size(des_tcid_vec); pos ++)
    {
        UINT32 des_tcid;
        des_tcid = (UINT32)cvector_get_no_lock(des_tcid_vec, pos);
#if 0
        /*check connectivity*/
        if(EC_FALSE == task_brd_check_tcid_connected(task_brd_default_get(), des_tcid))
        {
            dbg_log(SEC_0117_SUPER, 1)(LOGSTDOUT, "warn:__super_transfer_prepare: tcid %s not connected, skip it\n", c_word_to_ipv4(des_tcid));
            cvector_push_no_lock(ret_vec, (void *)EC_FALSE);
        }
        else
        {
            cvector_push_no_lock(ret_vec, (void *)EC_TRUE);
        }
#endif
#if 1
        if(CMPI_ERROR_TCID == des_tcid)
        {
            cvector_push_no_lock(ret_vec, (void *)EC_FALSE);
        }
        else
        {
            cvector_push_no_lock(ret_vec, (void *)EC_TRUE);
        }
#endif
    }
    return (EC_TRUE);
}

EC_BOOL super_transfer_vec(const UINT32 super_md_id, const CSTRING *src_fname, const CVECTOR *des_tcid_vec, const CSTRING *des_fname, CVECTOR *ret_vec)
{
    UINT32 csize;/*read completed size*/
    UINT32 osize;/*read once size*/
    UINT32 rsize;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_transfer_vec: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    if(EC_TRUE == cstring_is_empty(src_fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer_vec:src_fname is empty\n");
        return (EC_FALSE);
    }

    if(EC_TRUE == cstring_is_empty(des_fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer_vec:des_fname is empty\n");
        return (EC_FALSE);
    }

    __super_transfer_prepare(des_tcid_vec, ret_vec);

    if(EC_FALSE == super_transfer_vec_start(super_md_id, src_fname, des_tcid_vec, des_fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer_vec: start transfer failed\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == super_size_file(super_md_id, src_fname, &rsize))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer_vec: get size of file %s failed\n", (char *)cstring_get_str(src_fname));
        super_transfer_vec_stop(super_md_id, src_fname, des_tcid_vec, des_fname);
        return (EC_FALSE);
    }

    for(csize = 0, osize = SUPER_READ_ONCE_MAX_BYTES; csize < rsize; csize += osize)
    {
        CBYTES   *cbytes;
        TASK_MGR *task_mgr;
        UINT32 des_tcid_pos;

        if(csize + osize > rsize)
        {
            osize = rsize - csize;
        }

        cbytes = cbytes_new(osize);
        if(NULL_PTR == cbytes)
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer_vec: new cbytes with len %ld failed\n", osize);
            super_transfer_vec_stop(super_md_id, src_fname, des_tcid_vec, des_fname);
            return (EC_FALSE);
        }

        if(EC_FALSE == super_read_fdata(super_md_id, src_fname, csize, osize, cbytes))
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer_vec: read %ld bytes at offset %ld of src file %s failed\n",
                                osize, csize, (char *)cstring_get_str(src_fname));
            cbytes_free(cbytes, LOC_SUPER_0080);
            super_transfer_vec_stop(super_md_id, src_fname, des_tcid_vec, des_fname);
            return (EC_FALSE);
        }

        task_mgr = task_new(NULL, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
        for(des_tcid_pos = 0; des_tcid_pos < cvector_size(des_tcid_vec); des_tcid_pos ++)
        {
            MOD_NODE recv_mod_node;
            UINT32   des_tcid;
            UINT32  *ret;

            ret = (UINT32 *)cvector_get_addr_no_lock(ret_vec, des_tcid_pos);/*get address!*/
            if(EC_FALSE == (*ret))/*give up if it failed before*/
            {
                continue;
            }

            des_tcid = (UINT32)cvector_get_no_lock(des_tcid_vec, des_tcid_pos);
            /*when transfer file on current host to the same same, ignore it*/
            if(CMPI_LOCAL_TCID == des_tcid && EC_TRUE == cstring_is_equal(src_fname, des_fname))
            {
                continue;
            }

            MOD_NODE_TCID(&recv_mod_node) = des_tcid;
            MOD_NODE_COMM(&recv_mod_node) = CMPI_LOCAL_COMM;
            MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
            MOD_NODE_MODI(&recv_mod_node) = 0;

            task_p2p_inc(task_mgr, super_md_id,
                        &recv_mod_node,
                        ret, FI_super_write_fdata, ERR_MODULE_ID, des_fname, csize, cbytes);
        }
        task_wait(task_mgr, TASK_ALWAYS_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

        cbytes_free(cbytes, LOC_SUPER_0081);
    }

    if(EC_FALSE == super_transfer_vec_stop(super_md_id, src_fname, des_tcid_vec, des_fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer_vec: stop transfer failed\n");
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL super_transfer_vec_ipaddr_cstr(const UINT32 super_md_id, const CSTRING *src_fname, const CVECTOR *ipaddr_cstr_vec, const CSTRING *des_fname, CVECTOR *ret_vec)
{
    UINT32    pos;

    TASK_BRD *task_brd;
    CVECTOR  *tcid_vec;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_transfer_vec_ipaddr_cstr: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/
    if(EC_TRUE == cstring_is_empty(src_fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer_vec_ipaddr_cstr:src_fname is empty\n");
        return (EC_FALSE);
    }

    if(EC_TRUE == cstring_is_empty(des_fname))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer_vec_ipaddr_cstr:des_fname is empty\n");
        return (EC_FALSE);
    }

    task_brd = task_brd_default_get();

    tcid_vec = cvector_new(0, MM_UINT32, LOC_SUPER_0082);
    if(NULL_PTR == tcid_vec)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer_vec_ipaddr_cstr: new tcid vec failed\n");
        return (EC_FALSE);
    }

    for(pos = 0; pos < cvector_size(ipaddr_cstr_vec); pos ++)
    {
        CSTRING * ipaddr_cstr;
        UINT32    ipaddr;
        UINT32    tcid;

        ipaddr_cstr = (CSTRING *)cvector_get_no_lock(ipaddr_cstr_vec, pos);
        if(NULL_PTR == ipaddr_cstr)
        {
            continue;
        }

        ipaddr = c_ipv4_to_word((char *)cstring_get_str(ipaddr_cstr));
        tcid   = task_brd_get_tcid_by_ipaddr(task_brd, ipaddr);

        if(CMPI_ERROR_TCID == tcid)
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer_vec_ipaddr_cstr: no tcid for ipaddr %s\n", (char *)cstring_get_str(ipaddr_cstr));
            continue;
        }

        cvector_push_no_lock(tcid_vec, (void *)tcid);
    }

    if(EC_FALSE == super_transfer_vec(super_md_id, src_fname, tcid_vec, des_fname, ret_vec))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_transfer_vec_ipaddr_cstr: transfer to tcid vec failed\n");
        cvector_free(tcid_vec, LOC_SUPER_0083);
        return (EC_FALSE);
    }
    cvector_free(tcid_vec, LOC_SUPER_0084);
    return (EC_TRUE);
}

EC_BOOL super_start_mcast_udp_server(const UINT32 super_md_id)
{
    TASK_BRD *task_brd;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_start_mcast_udp_server: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();

    if(EC_FALSE == task_brd_is_mcast_udp_server(task_brd))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_start_mcast_udp_server: I am not mcast udp server\n");
        return (EC_FALSE);
    }

    /*when not running, start it*/
    if(EC_FALSE == task_brd_status_mcast_udp_server(task_brd))
    {
        if(EC_FALSE == task_brd_start_mcast_udp_server(task_brd))
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_start_mcast_udp_server: start mcast udp server failed\n");
            return (EC_FALSE);
        }
    }

    /*patch: for auto-connection the broken nodes*/
    super_activate_sys_cfg(super_md_id);

    return (EC_TRUE);
}

EC_BOOL super_stop_mcast_udp_server(const UINT32 super_md_id)
{
    TASK_BRD *task_brd;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_stop_mcast_udp_server: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();

    if(EC_FALSE == task_brd_is_mcast_udp_server(task_brd))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_stop_mcast_udp_server: I am not mcast udp server\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == task_brd_stop_mcast_udp_server(task_brd))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_stop_mcast_udp_server: start mcast udp server failed\n");
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL super_status_mcast_udp_server(const UINT32 super_md_id)
{
    TASK_BRD *task_brd;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_status_mcast_udp_server: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();

    if(EC_FALSE == task_brd_is_mcast_udp_server(task_brd))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_status_mcast_udp_server: I am not mcast udp server\n");
        return (EC_FALSE);
    }

    return task_brd_status_mcast_udp_server(task_brd);
}

EC_BOOL super_set_hostname(const UINT32 super_md_id, const CSTRING *hostname_cstr)
{
    char set_hostname_cmd[SUPER_CMD_BUFF_MAX_SIZE];
    const char *network_fname   = "/etc/sysconfig/network";
    const char *gmond_cfg_fname = "/usr/local/etc/gmond.conf";

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_set_hostname: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    if(EC_TRUE == cstring_is_empty(hostname_cstr))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_set_hostname: hostname cstr is empty\n");
        return (EC_FALSE);
    }

    snprintf(set_hostname_cmd, SUPER_CMD_BUFF_MAX_SIZE, "hostname %s", (char *)cstring_get_str(hostname_cstr));
    if(EC_FALSE == exec_shell(set_hostname_cmd, NULL_PTR, 0))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_set_hostname: exec shell %s failed\n", set_hostname_cmd);
        return (EC_FALSE);
    }
    dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "super_set_hostname: exec shell %s\n", set_hostname_cmd);

    if(0 == access(network_fname, F_OK | W_OK))
    {
        snprintf(set_hostname_cmd, SUPER_CMD_BUFF_MAX_SIZE, "sed -i s/HOSTNAME=.*/HOSTNAME=%s/g %s",
                 (char *)cstring_get_str(hostname_cstr), network_fname);
        if(EC_FALSE == exec_shell(set_hostname_cmd, NULL_PTR, 0))
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_set_hostname: exec shell %s failed\n", set_hostname_cmd);
            return (EC_FALSE);
        }
        dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "super_set_hostname: exec shell %s\n", set_hostname_cmd);
    }
    else
    {
        dbg_log(SEC_0117_SUPER, 1)(LOGSTDOUT, "warn:super_set_hostname: %s not accessiable\n", network_fname);
    }

    if(0 == access(gmond_cfg_fname, F_OK | W_OK))
    {
        snprintf(set_hostname_cmd, SUPER_CMD_BUFF_MAX_SIZE, "sed -i \"/override_hostname/c\\  override_hostname = %s\" %s",
                (char *)cstring_get_str(hostname_cstr), gmond_cfg_fname);
        if(EC_FALSE == exec_shell(set_hostname_cmd, NULL_PTR, 0))
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_set_hostname: exec shell %s failed\n", set_hostname_cmd);
            return (EC_FALSE);
        }
        dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "super_set_hostname: exec shell %s\n", set_hostname_cmd);
    }
    else
    {
        dbg_log(SEC_0117_SUPER, 1)(LOGSTDOUT, "warn:super_set_hostname: %s not accessiable\n", gmond_cfg_fname);
    }

    if(0 != sethostname((char *)cstring_get_str(hostname_cstr), cstring_get_len(hostname_cstr)))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_set_hostname: sethostname %s failed\n", (char *)cstring_get_str(hostname_cstr));
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL super_get_hostname(const UINT32 super_md_id, CSTRING *hostname_cstr)
{
    //const char *get_hostname_cmd = "hostname";
    char hostname[SUPER_CMD_BUFF_MAX_SIZE];

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_get_hostname: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    if(NULL_PTR == hostname_cstr)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_get_hostname: hostname cstr is null\n");
        return (EC_FALSE);
    }

    BSET(hostname, 0, SUPER_CMD_BUFF_MAX_SIZE);
#if 0
    if(EC_FALSE == exec_shell(get_hostname_cmd, hostname, SUPER_CMD_BUFF_MAX_SIZE))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_get_hostname: exec shell %s failed\n", get_hostname_cmd);
        return (EC_FALSE);
    }

    hostname[ strlen(hostname) - 1 ] = '\0'; /*discard the \r\n*/
#endif

    if(0 != gethostname(hostname, SUPER_CMD_BUFF_MAX_SIZE))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_get_hostname: gethostname failed\n");
        return (EC_FALSE);
    }

    cstring_format(hostname_cstr, "%s", hostname);

    return (EC_TRUE);
}

EC_BOOL super_set_hostname_ipaddr_cstr(const UINT32 super_md_id, const CSTRING *ipaddr_cstr, const CSTRING *hostname_cstr)
{
    UINT32   ipaddr;
    UINT32   tcid;
    MOD_NODE recv_mod_node;

    EC_BOOL ret;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_set_hostname_ipaddr_cstr: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    if(EC_TRUE == cstring_is_empty(ipaddr_cstr))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_set_hostname_ipaddr_cstr:ipaddr cstr is empty\n");
        return (EC_FALSE);
    }

    if(EC_TRUE == cstring_is_empty(hostname_cstr))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_set_hostname_ipaddr_cstr:hostname cstr is empty\n");
        return (EC_FALSE);
    }

    dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "super_set_hostname_ipaddr_cstr: set hostname on ipaddr %s: %s\n",
                        (char *)cstring_get_str(ipaddr_cstr),
                        (char *)cstring_get_str(hostname_cstr));

    ipaddr = c_ipv4_to_word((char *)cstring_get_str(ipaddr_cstr));
    tcid = task_brd_get_tcid_by_ipaddr(task_brd_default_get(), ipaddr);
    if(CMPI_ERROR_TCID == tcid)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_set_hostname_ipaddr_cstr: no tcid for ipaddr %s failed\n", (char *)cstring_get_str(ipaddr_cstr));
        return (EC_FALSE);
    }

    if(CMPI_LOCAL_TCID == tcid)
    {
        return super_set_hostname(super_md_id, hostname_cstr);
    }

    if(EC_FALSE == task_brd_check_tcid_connected(task_brd_default_get(), tcid))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_set_hostname_ipaddr_cstr: ipaddr %s not connected\n", (char *)cstring_get_str(ipaddr_cstr));
        return (EC_FALSE);
    }

    MOD_NODE_TCID(&recv_mod_node) = tcid;
    MOD_NODE_COMM(&recv_mod_node) = CMPI_ANY_COMM;
    MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
    MOD_NODE_MODI(&recv_mod_node) = 0;

    ret = EC_FALSE;
    task_p2p(super_md_id, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, &recv_mod_node,
                &ret, FI_super_set_hostname, ERR_MODULE_ID, hostname_cstr);
    return (ret);
}

EC_BOOL super_get_hostname_ipaddr_cstr(const UINT32 super_md_id, const CSTRING *ipaddr_cstr, CSTRING *hostname_cstr)
{
    UINT32   ipaddr;
    UINT32   tcid;
    MOD_NODE recv_mod_node;

    EC_BOOL ret;

#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_get_hostname_ipaddr_cstr: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    if(EC_TRUE == cstring_is_empty(ipaddr_cstr))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_get_hostname_ipaddr_cstr:ipaddr cstr is empty\n");
        return (EC_FALSE);
    }

    if(NULL_PTR == hostname_cstr)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_get_hostname_ipaddr_cstr:hostname cstr is null\n");
        return (EC_FALSE);
    }

    dbg_log(SEC_0117_SUPER, 5)(LOGSTDOUT, "super_get_hostname_ipaddr_cstr: get hostname on ipaddr %s\n",
                        (char *)cstring_get_str(ipaddr_cstr));

    ipaddr = c_ipv4_to_word((char *)cstring_get_str(ipaddr_cstr));
    tcid = task_brd_get_tcid_by_ipaddr(task_brd_default_get(), ipaddr);
    if(CMPI_ERROR_TCID == tcid)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_get_hostname_ipaddr_cstr: no tcid for ipaddr %s failed\n", (char *)cstring_get_str(ipaddr_cstr));
        return (EC_FALSE);
    }

    if(CMPI_LOCAL_TCID == tcid)
    {
        return super_get_hostname(super_md_id, hostname_cstr);
    }

    if(EC_FALSE == task_brd_check_tcid_connected(task_brd_default_get(), tcid))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_get_hostname_ipaddr_cstr: ipaddr %s not connected\n", (char *)cstring_get_str(ipaddr_cstr));
        return (EC_FALSE);
    }

    MOD_NODE_TCID(&recv_mod_node) = tcid;
    MOD_NODE_COMM(&recv_mod_node) = CMPI_ANY_COMM;
    MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
    MOD_NODE_MODI(&recv_mod_node) = 0;

    ret = EC_FALSE;
    task_p2p(super_md_id, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, &recv_mod_node,
                &ret, FI_super_get_hostname, ERR_MODULE_ID, hostname_cstr);

    return (ret);
}

EC_BOOL super_say_hello(const UINT32 super_md_id, const UINT32 des_tcid, const UINT32 des_rank, CSTRING *cstring)
{
    MOD_NODE recv_mod_node;
    EC_BOOL ret;
        
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_say_hello: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    if(CMPI_LOCAL_TCID == des_tcid && CMPI_LOCAL_RANK == des_rank)
    {
        cstring_format(cstring, "[%s] say hello!", c_word_to_ipv4(des_tcid));
        return (EC_TRUE);
    }

    MOD_NODE_TCID(&recv_mod_node) = des_tcid;
    MOD_NODE_COMM(&recv_mod_node) = CMPI_ANY_COMM;
    MOD_NODE_RANK(&recv_mod_node) = des_rank;
    MOD_NODE_MODI(&recv_mod_node) = 0;    

    ret = EC_FALSE;
    task_p2p(super_md_id, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP,
            &recv_mod_node,
            &ret, FI_super_say_hello, ERR_MODULE_ID, des_tcid, des_rank, cstring);
    return (ret);
}

EC_BOOL super_say_hello_batch(const UINT32 super_md_id, const UINT32 num, const UINT32 des_tcid, const UINT32 des_rank)
{
    TASK_MGR *task_mgr;
    MOD_NODE  recv_mod_node;    
    UINT32    idx;

    CVECTOR  *report_vec;
    CVECTOR  *cstring_vec;
    EC_BOOL   result;
        
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_say_hello_batch: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    MOD_NODE_TCID(&recv_mod_node) = des_tcid;
    MOD_NODE_COMM(&recv_mod_node) = CMPI_ANY_COMM;
    MOD_NODE_RANK(&recv_mod_node) = des_rank;
    MOD_NODE_MODI(&recv_mod_node) = 0;  

    report_vec  = cvector_new(0, MM_UINT32, LOC_SUPER_0085);
    cstring_vec = cvector_new(0, MM_CSTRING, LOC_SUPER_0086);

    task_mgr = task_new(NULL_PTR, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    for(idx = 0; idx < num; idx ++)
    {
        UINT32 *ret;
        CSTRING *cstring;

        alloc_static_mem(MD_TASK, CMPI_ANY_MODI, MM_UINT32, &ret, LOC_SUPER_0087);
        cvector_push(report_vec, (void *)ret);

        cstring = cstring_new(NULL_PTR, LOC_SUPER_0088);
        cvector_push(cstring_vec, (void *)cstring);
        
        (*ret) = EC_FALSE;
        task_p2p_inc(task_mgr, super_md_id, &recv_mod_node, 
                     ret, FI_super_say_hello, ERR_MODULE_ID, des_tcid, des_rank, cstring);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    result = EC_TRUE;
    for(idx = 0; idx < num; idx ++)
    {
        UINT32 *ret;
        CSTRING *cstring;
        
        ret = (UINT32 *)cvector_get(report_vec, idx);
        cstring = (CSTRING *)cvector_get(cstring_vec, idx);

        if(EC_FALSE == (*ret))
        {
            result = EC_FALSE;
        }

        cvector_set(report_vec, idx, NULL_PTR);
        free_static_mem(MD_TASK, CMPI_ANY_MODI, MM_UINT32, ret, LOC_SUPER_0089);

        cvector_set(cstring_vec, idx, NULL_PTR);
        cstring_free(cstring);
    }    

    cvector_free(report_vec, LOC_SUPER_0090);
    cvector_free(cstring_vec, LOC_SUPER_0091);

    return (result);
}

EC_BOOL super_say_hello_loop(const UINT32 super_md_id, const UINT32 loops, const UINT32 des_tcid, const UINT32 des_rank)
{
    UINT32   count;
    UINT32   step;
        
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_say_hello_loop: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    for(count = 0, step = 1000; count + step < loops; count += step)
    {
        if(EC_FALSE == super_say_hello_batch(super_md_id, step, des_tcid, des_rank))
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_say_hello_loop: say hello failed where count = %ld, step = %ld, loop = %ld\n", count, step, loops);
            return (EC_FALSE);
        }

        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "[DEBUG] super_say_hello_loop: %ld - %ld done\n", count, count + step);
    }

    if(count < loops)
    {
        step = loops - count;
        if(EC_FALSE == super_say_hello_batch(super_md_id, step, des_tcid, des_rank))
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_say_hello_loop: say hello failed where count = %ld, step = %ld, loop = %ld\n", count, step, loops);
            return (EC_FALSE);
        }

        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "[DEBUG] super_say_hello_loop: %ld - %ld done\n", count, count + step);    
    }
    return (EC_TRUE);
}

EC_BOOL super_say_hello_loop0(const UINT32 super_md_id, const UINT32 loops, const UINT32 des_tcid, const UINT32 des_rank)
{
    UINT32   count;
        
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_say_hello_loop: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    for(count = 0; count < loops; count ++)
    {
        EC_BOOL  ret;
        CSTRING *cstring;

        cstring = cstring_new(NULL_PTR, LOC_SUPER_0092);
        ASSERT(NULL_PTR != cstring);
        ret = super_say_hello(super_md_id, des_tcid, des_rank, cstring);
        cstring_free(cstring);

        if(EC_FALSE == ret)
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_say_hello_loop: say hello failed where count = %ld, loop = %ld\n", count, loops);
            return (EC_FALSE);
        }

        if(0 == ((count + 1) % 1000))
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "[DEBUG] super_say_hello_loop: %ld - %ld done\n", count - 999, count);
        }
    }
    return (EC_TRUE);
}

/*------------------------------------------------------ test for ict -----------------------------------------------------------------------*/

#define __TCID_TO_ZONE_ID_MASK                         ((UINT32)0xFF)

#define __OBJ_DATA_BIT_OFFSET                          ((UINT32)(WORDSIZE / 2))

#define __GET_ZONE_ID_FROM_TCID(tcid)                  (((tcid) & __TCID_TO_ZONE_ID_MASK) - 1)

#define __MAKE_OBJ_ID(zone_id, zone_size, obj_idx)     ((zone_id) * (zone_size) + (obj_idx))

#define __MAKE_OBJ_DATA(obj_id, data_idx)              (((data_idx) << __OBJ_DATA_BIT_OFFSET) | (obj_id))

#define __MAKE_DES_TCID(tcid, zone_id)                 (((tcid) & (~__TCID_TO_ZONE_ID_MASK)) | ((zone_id) + 1))

#define __GET_ZONE_ID_FROM_OBJ_ID(obj_id, zone_size)   ((obj_id) / (zone_size))

#define __GET_OBJ_IDX_FROM_OBJ_ID(obj_id, zone_size)   ((obj_id) % (zone_size))

EC_BOOL super_set_zone_size(const UINT32 super_md_id, const UINT32 obj_zone_size)
{
    SUPER_MD  *super_md;
    
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_set_zone_size: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    super_md = SUPER_MD_GET(super_md_id);
    SUPER_MD_OBJ_ZONE_SIZE(super_md) = obj_zone_size;
    return (EC_TRUE);
}

EC_BOOL super_load_data(const UINT32 super_md_id)
{
    SUPER_MD  *super_md;
    
    TASK_BRD  *task_brd;
    UINT32     obj_zone_size;
    UINT32     obj_zone_id;
    UINT32     obj_idx;
    
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_load_data: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    super_md = SUPER_MD_GET(super_md_id);   
    obj_zone_size = SUPER_MD_OBJ_ZONE_SIZE(super_md);

    task_brd = task_brd_default_get();
    obj_zone_id = __GET_ZONE_ID_FROM_TCID(TASK_BRD_TCID(task_brd));

    SUPER_MD_OBJ_ZONE(super_md) = cvector_new(obj_zone_size, MM_CVECTOR, LOC_SUPER_0093);
    if(NULL_PTR == SUPER_MD_OBJ_ZONE(super_md))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_load_data: new obj zone with size %ld failed\n", obj_zone_size);
        return (EC_FALSE);
    }

    for(obj_idx = 0; obj_idx < obj_zone_size; obj_idx ++)
    {
        CVECTOR *obj_vec;
        UINT32   obj_id;        
        UINT32   obj_data_num;
        UINT32   obj_data_idx;        
        
        obj_data_num = /*50*/5;
        
        obj_vec = cvector_new(obj_data_num, MM_UINT32, LOC_SUPER_0094);
        if(NULL_PTR == obj_vec)
        {
            EC_BOOL ret;
            
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_load_data: new obj vec with size %ld failed\n", obj_data_num);
            
            cvector_loop(SUPER_MD_OBJ_ZONE(super_md), 
                        (void *)&ret, 
                        CVECTOR_CHECKER_DEFAULT, 
                        2, 
                        0, 
                        (UINT32)cvector_free,
                        NULL_PTR, 
                        LOC_SUPER_0095);
            cvector_free(SUPER_MD_OBJ_ZONE(super_md), LOC_SUPER_0096);            
            return (EC_FALSE);
        }

        obj_id = __MAKE_OBJ_ID(obj_zone_id, SUPER_MD_OBJ_ZONE_SIZE(super_md), obj_idx);
        for(obj_data_idx = 0; obj_data_idx < obj_data_num; obj_data_idx ++)
        {
            UINT32 obj_data;
            obj_data = __MAKE_OBJ_DATA(obj_id, obj_data_idx);
            cvector_push_no_lock(obj_vec, (void *)obj_data);
        }

        cvector_push_no_lock(SUPER_MD_OBJ_ZONE(super_md), (void *)obj_vec);
    }
    
    return (EC_TRUE);
}

EC_BOOL super_load_data_all(const UINT32 super_md_id, const UINT32 obj_zone_num)
{
    TASK_BRD *task_brd;
    TASK_MGR *task_mgr;
    UINT32    obj_zone_id;
    UINT32    ret;
    
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_load_data_all: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();
   
    task_mgr = task_new(NULL_PTR, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);    
    for(obj_zone_id = 0; obj_zone_id < obj_zone_num; obj_zone_id ++)
    {
        UINT32 tcid;
        
        MOD_NODE recv_mod_node;

        tcid = __MAKE_DES_TCID(tcid, obj_zone_id);
        MOD_NODE_TCID(&recv_mod_node) = tcid;
        MOD_NODE_COMM(&recv_mod_node) = CMPI_ANY_COMM;
        MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
        MOD_NODE_MODI(&recv_mod_node) = 0;

        task_p2p_inc(task_mgr, super_md_id, &recv_mod_node, &ret, FI_super_load_data, ERR_MODULE_ID);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    return (EC_TRUE);
}

EC_BOOL super_get_data(const UINT32 super_md_id, const UINT32 obj_id, CVECTOR *obj_data)
{
    SUPER_MD  *super_md;
    
    TASK_BRD  *task_brd;
    UINT32     obj_idx;
    CVECTOR   *obj_vec;    
    
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_get_data: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    super_md = SUPER_MD_GET(super_md_id);    

    task_brd = task_brd_default_get();

    /*check*/
    if(__GET_ZONE_ID_FROM_OBJ_ID(obj_id, SUPER_MD_OBJ_ZONE_SIZE(super_md)) !=  __GET_ZONE_ID_FROM_TCID(TASK_BRD_TCID(task_brd)))
    {
        //dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_get_data:mismatched obj_zone_id %lx and obj_id %lx\n", obj_zone_id, obj_id);
        //return (EC_FALSE);

        EC_BOOL ret;
        UINT32  tcid;
        MOD_NODE recv_mod_node;

        tcid = __MAKE_DES_TCID(TASK_BRD_TCID(task_brd), __GET_ZONE_ID_FROM_OBJ_ID(obj_id, SUPER_MD_OBJ_ZONE_SIZE(super_md)));
        MOD_NODE_TCID(&recv_mod_node) = tcid;
        MOD_NODE_COMM(&recv_mod_node) = CMPI_ANY_COMM;
        MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
        MOD_NODE_MODI(&recv_mod_node) = 0;        

        ret = EC_FALSE;
        task_p2p(super_md_id, TASK_DEFAULT_LIVE, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP, 
                &recv_mod_node, &ret, FI_super_get_data, ERR_MODULE_ID, obj_id, obj_data);
        return (ret);                
    }

    if(NULL_PTR == SUPER_MD_OBJ_ZONE(super_md))
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_get_data:obj zone is null\n");
        return (EC_FALSE);
    }

    //dbg_log(SEC_0117_SUPER, 0)(LOGCONSOLE, "[DEBUG] super_get_data: obj_data %lx, mm type %ld\n", obj_data, obj_data->data_mm_type);

    obj_idx = __GET_OBJ_IDX_FROM_OBJ_ID(obj_id, SUPER_MD_OBJ_ZONE_SIZE(super_md));

    obj_vec = (CVECTOR *)cvector_get_no_lock(SUPER_MD_OBJ_ZONE(super_md), obj_idx);
    if(NULL_PTR == obj_vec)
    {
        dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_get_data: obj vec of obj id %ld or obj idx %ld is null\n", obj_id, obj_idx);
        return (EC_FALSE);
    }

    cvector_clone_no_lock(obj_vec, obj_data, NULL_PTR, NULL_PTR);

    //dbg_log(SEC_0117_SUPER, 0)(LOGCONSOLE, "[DEBUG] super_get_data: obj_vec %lx, mm type %ld\n", obj_vec, obj_vec->data_mm_type);
    //super_print_obj_vec(super_md_id, obj_vec, LOGCONSOLE);
    //dbg_log(SEC_0117_SUPER, 0)(LOGCONSOLE, "[DEBUG] super_get_data: obj_data %lx, mm type %ld\n", obj_data, obj_data->data_mm_type);
    //super_print_obj_vec(super_md_id, obj_data, LOGCONSOLE);

    return (EC_TRUE);
}

EC_BOOL super_get_data_vec(const UINT32 super_md_id, const CVECTOR *obj_id_vec, CVECTOR *obj_data_vec)
{
    SUPER_MD  *super_md;
    
    TASK_BRD *task_brd;
    TASK_MGR *task_mgr;
    UINT32    obj_id_pos;
    EC_BOOL   ret;
    
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_get_data_vec: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    super_md = SUPER_MD_GET(super_md_id);
    
    task_brd = task_brd_default_get();
   
    task_mgr = task_new(NULL_PTR, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);    
    for(obj_id_pos = 0; obj_id_pos < cvector_size(obj_id_vec); obj_id_pos ++)
    {
        UINT32 obj_id;
        UINT32 tcid;

        CVECTOR *obj_data;
        
        MOD_NODE recv_mod_node;

        obj_data = cvector_new(0, MM_UINT32, LOC_SUPER_0097);
        if(NULL_PTR == obj_data)
        {
            dbg_log(SEC_0117_SUPER, 0)(LOGSTDOUT, "error:super_get_data_vec: new obj_data failed\n");
            task_mgr_free(task_mgr);

            cvector_loop_no_lock(obj_data_vec, 
                        (void *)&ret, 
                        CVECTOR_CHECKER_DEFAULT, 
                        2, 
                        0, 
                        (UINT32)cvector_free_no_lock,
                        NULL_PTR, 
                        LOC_SUPER_0098);
            cvector_clean_no_lock(obj_data_vec, NULL_PTR, LOC_SUPER_0099);
            return (EC_FALSE);
        }

        cvector_push_no_lock(obj_data_vec, (void *)obj_data);

        obj_id = (UINT32)cvector_get_no_lock(obj_id_vec, obj_id_pos);
        tcid = __MAKE_DES_TCID(TASK_BRD_TCID(task_brd), __GET_ZONE_ID_FROM_OBJ_ID(obj_id, SUPER_MD_OBJ_ZONE_SIZE(super_md)));
        
        MOD_NODE_TCID(&recv_mod_node) = tcid;
        MOD_NODE_COMM(&recv_mod_node) = CMPI_ANY_COMM;
        MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
        MOD_NODE_MODI(&recv_mod_node) = 0;

        task_p2p_inc(task_mgr, super_md_id, &recv_mod_node, &ret, FI_super_get_data, ERR_MODULE_ID, obj_id, obj_data);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    return (EC_TRUE);
}

EC_BOOL super_print_obj_vec(const UINT32 super_md_id, const CVECTOR *obj_vec, LOG *log)
{
    UINT32     obj_data_idx;
    
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_print_obj_vec: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    for(obj_data_idx = 0; obj_data_idx < cvector_size(obj_vec); obj_data_idx ++)
    {
        UINT32     obj_data;
        obj_data = (UINT32)cvector_get_no_lock(obj_vec, obj_data_idx);
        sys_print(log, "%lx,", obj_data);
    }
    sys_print(log, "\n");

    return (EC_TRUE);
}

EC_BOOL super_print_data(const UINT32 super_md_id, LOG *log)
{
    SUPER_MD  *super_md;
    
    UINT32     obj_idx;    
    
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_print_data: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    super_md = SUPER_MD_GET(super_md_id);  
    for(obj_idx = 0; obj_idx < cvector_size(SUPER_MD_OBJ_ZONE(super_md)); obj_idx ++)
    {
        CVECTOR   *obj_vec;   

        obj_vec = (CVECTOR *)cvector_get_no_lock(SUPER_MD_OBJ_ZONE(super_md), obj_idx);
        
        sys_print(log, "[%lx] ", obj_idx);
        super_print_obj_vec(super_md_id, obj_vec, log);
    }

    return (EC_TRUE);
}

EC_BOOL super_print_data_all(const UINT32 super_md_id, const UINT32 obj_zone_num, LOG *log)
{
    TASK_BRD *task_brd;
    TASK_MGR *task_mgr;
    UINT32    obj_zone_id;
    UINT32    ret;
    
#if ( SWITCH_ON == SUPER_DEBUG_SWITCH )
    if ( SUPER_MD_ID_CHECK_INVALID(super_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:super_print_data_all: super module #0x%lx not started.\n",
                super_md_id);
        dbg_exit(MD_SUPER, super_md_id);
    }
#endif/*SUPER_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();
   
    task_mgr = task_new(NULL_PTR, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);    
    for(obj_zone_id = 0; obj_zone_id < obj_zone_num; obj_zone_id ++)
    {
        UINT32 tcid;
        
        MOD_NODE recv_mod_node;

        tcid = __MAKE_DES_TCID(TASK_BRD_TCID(task_brd), obj_zone_id);
        MOD_NODE_TCID(&recv_mod_node) = tcid;
        MOD_NODE_COMM(&recv_mod_node) = CMPI_ANY_COMM;
        MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
        MOD_NODE_MODI(&recv_mod_node) = 0;

        task_p2p_inc(task_mgr, super_md_id, &recv_mod_node, &ret, FI_super_print_data, ERR_MODULE_ID, log);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    return (EC_TRUE);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

