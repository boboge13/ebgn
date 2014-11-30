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
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <stdarg.h>

#include <sys/epoll.h>
#include <errno.h>

#include "type.h"
#include "log.h"

#include "cxml.h"
#include "task.h"
#include "csocket.h"
#include "task.inc"
#include "task.h"

#include "cmpic.inc"
#include "cmpie.h"
#include "cmisc.h"

#include "cepoll.h"
#include "csocket.h"
#include "taskcfg.inc"
#include "tasks.h"
#include "cmutex.h"
#include "crb.h"
#include "croutine.h"


#define CEPOLL_RD_EVENT_CHAR(events)        (((events) & CEPOLL_RD_EVENT)  ? 'R':'-')
#define CEPOLL_WR_EVENT_CHAR(events)        (((events) & CEPOLL_WR_EVENT)  ? 'W':'-')
#define CEPOLL_ERR_EVENT_CHAR(events)       (((events) & CEPOLL_ERR_EVENT) ? 'E':'-')
#define CEPOLL_HUP_EVENT_CHAR(events)       (((events) & CEPOLL_HUP_EVENT) ? 'U':'-')
#define CEPOLL_ALL_EVENT_CHARS(events)      CEPOLL_RD_EVENT_CHAR(events), CEPOLL_WR_EVENT_CHAR(events), CEPOLL_ERR_EVENT_CHAR(events), CEPOLL_HUP_EVENT_CHAR(events)

EC_BOOL cepoll_node_init(CEPOLL_NODE *cepoll_node)
{
    CEPOLL_NODE_USED_FLAG(cepoll_node)        = CEPOLL_NODE_NOT_USED_FLAG;
    CEPOLL_NODE_SOCKFD(cepoll_node)           = ERR_FD;
    /*note: not init sockfd*/
    CEPOLL_NODE_RD_ARG(cepoll_node)           = NULL_PTR;
    CEPOLL_NODE_WR_ARG(cepoll_node)           = NULL_PTR;
    CEPOLL_NODE_TIMEOUT_ARG(cepoll_node)      = NULL_PTR;
    CEPOLL_NODE_SHUTDOWN_ARG(cepoll_node)     = NULL_PTR;
    
    CEPOLL_NODE_RD_HANDLER(cepoll_node)       = NULL_PTR;
    CEPOLL_NODE_WR_HANDLER(cepoll_node)       = NULL_PTR;
    CEPOLL_NODE_TIMEOUT_HANDLER(cepoll_node)  = NULL_PTR;
    CEPOLL_NODE_SHUTDOWN_HANDLER(cepoll_node) = NULL_PTR;
    
    CEPOLL_NODE_EVENTS(cepoll_node)           = 0;
    CEPOLL_NODE_ATIME_TS(cepoll_node)         = 0;
    CEPOLL_NODE_TIMEOUT_NSEC(cepoll_node)     = 0;   
    CEPOLL_NODE_COUNTER(cepoll_node)          = 0;   

    CEPOLL_NODE_CRB_NODE(cepoll_node)         = NULL_PTR;
    

    return (EC_TRUE);
}

EC_BOOL cepoll_node_clean(CEPOLL_NODE *cepoll_node)
{
    CEPOLL_NODE_USED_FLAG(cepoll_node)        = CEPOLL_NODE_NOT_USED_FLAG;
    CEPOLL_NODE_SOCKFD(cepoll_node)           = ERR_FD;    
    
    CEPOLL_NODE_RD_ARG(cepoll_node)           = NULL_PTR;
    CEPOLL_NODE_WR_ARG(cepoll_node)           = NULL_PTR;
    CEPOLL_NODE_TIMEOUT_ARG(cepoll_node)      = NULL_PTR;
    CEPOLL_NODE_SHUTDOWN_ARG(cepoll_node)     = NULL_PTR;
    
    CEPOLL_NODE_RD_HANDLER(cepoll_node)       = NULL_PTR;
    CEPOLL_NODE_WR_HANDLER(cepoll_node)       = NULL_PTR;
    CEPOLL_NODE_TIMEOUT_HANDLER(cepoll_node)  = NULL_PTR;
    CEPOLL_NODE_SHUTDOWN_HANDLER(cepoll_node) = NULL_PTR;
    
    CEPOLL_NODE_EVENTS(cepoll_node)           = 0;
    CEPOLL_NODE_ATIME_TS(cepoll_node)         = 0;
    CEPOLL_NODE_TIMEOUT_NSEC(cepoll_node)     = 0;   
    CEPOLL_NODE_COUNTER(cepoll_node)          = 0;   

    CEPOLL_NODE_CRB_NODE(cepoll_node)         = NULL_PTR;

    return (EC_TRUE);
}

EC_BOOL cepoll_node_clear(CEPOLL_NODE *cepoll_node)
{
    //CEPOLL_NODE_USED_FLAG(cepoll_node)        = CEPOLL_NODE_NOT_USED_FLAG;
    //CEPOLL_NODE_SOCKFD(cepoll_node)           = ERR_FD;   
    
    CEPOLL_NODE_RD_ARG(cepoll_node)           = NULL_PTR;
    CEPOLL_NODE_WR_ARG(cepoll_node)           = NULL_PTR;
    CEPOLL_NODE_TIMEOUT_ARG(cepoll_node)      = NULL_PTR;
    CEPOLL_NODE_SHUTDOWN_ARG(cepoll_node)     = NULL_PTR;
    
    CEPOLL_NODE_RD_HANDLER(cepoll_node)       = NULL_PTR;
    CEPOLL_NODE_WR_HANDLER(cepoll_node)       = NULL_PTR;
    CEPOLL_NODE_TIMEOUT_HANDLER(cepoll_node)  = NULL_PTR;
    CEPOLL_NODE_SHUTDOWN_HANDLER(cepoll_node) = NULL_PTR;
    
    CEPOLL_NODE_EVENTS(cepoll_node)           = 0;
    CEPOLL_NODE_ATIME_TS(cepoll_node)         = 0;
    CEPOLL_NODE_TIMEOUT_NSEC(cepoll_node)     = 0;   
    CEPOLL_NODE_COUNTER(cepoll_node)          = 0;   

    CEPOLL_NODE_CRB_NODE(cepoll_node)         = NULL_PTR;

    return (EC_TRUE);
}

EC_BOOL cepoll_node_set_used(CEPOLL_NODE *cepoll_node)
{
    CEPOLL_NODE_USED_FLAG(cepoll_node)        = CEPOLL_NODE_IS_USED_FLAG;
    return (EC_TRUE);    
}

EC_BOOL cepoll_node_set_not_used(CEPOLL_NODE *cepoll_node)
{
    CEPOLL_NODE_USED_FLAG(cepoll_node)        = CEPOLL_NODE_NOT_USED_FLAG;
    return (EC_TRUE);    
}

/*expire time = last access time + timeout*/
int cepoll_node_cmp_expire_time(const CEPOLL_NODE *cepoll_node_1st, const CEPOLL_NODE *cepoll_node_2nd)
{
    UINT32 expire_time_1st;
    UINT32 expire_time_2nd;

    expire_time_1st = CEPOLL_NODE_ATIME_TS(cepoll_node_1st) + CEPOLL_NODE_TIMEOUT_NSEC(cepoll_node_1st);
    expire_time_2nd = CEPOLL_NODE_ATIME_TS(cepoll_node_2nd) + CEPOLL_NODE_TIMEOUT_NSEC(cepoll_node_2nd);
    
    if(expire_time_1st < expire_time_2nd)
    {
        return (-1);
    }

    if(expire_time_1st > expire_time_2nd)
    {
        return (1);
    }

    if(CEPOLL_NODE_COUNTER(cepoll_node_1st) < CEPOLL_NODE_COUNTER(cepoll_node_2nd))
    {
        return (-1);
    }

    if(CEPOLL_NODE_COUNTER(cepoll_node_1st) > CEPOLL_NODE_COUNTER(cepoll_node_2nd))
    {
        return (1);
    }    

    return (0);
}

void cepoll_node_print(LOG *log, const CEPOLL_NODE *cepoll_node)
{
    dbg_log(SEC_0072_CEPOLL, 5)(LOGSTDOUT, "cepoll_node_print: cepoll_node %p, sockfd %d, events %u, timeout %u, last access time %u, counter %u\n",
                        cepoll_node, 
                        CEPOLL_NODE_SOCKFD(cepoll_node),
                        CEPOLL_NODE_EVENTS(cepoll_node), 
                        CEPOLL_NODE_TIMEOUT_NSEC(cepoll_node),
                        CEPOLL_NODE_ATIME_TS(cepoll_node),
                        CEPOLL_NODE_COUNTER(cepoll_node));
    return;
}

CEPOLL *cepoll_new(const int epoll_max_event_num)
{
    CEPOLL *cepoll;

    alloc_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CEPOLL, &cepoll, LOC_CEPOLL_0001);
    if(NULL_PTR == cepoll)
    {
        dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:cepoll_new: new cepoll failed\n");
        return (NULL_PTR);
    }

    if(EC_FALSE == cepoll_init(cepoll, epoll_max_event_num))
    {
        dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:cepoll_new: init cepoll with max event num %d failed\n", epoll_max_event_num);
        free_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CEPOLL, cepoll, LOC_CEPOLL_0002);
        return (NULL_PTR);
    }
    return (cepoll);
}

EC_BOOL cepoll_init(CEPOLL *cepoll, const int epoll_max_event_num)
{
    CEPOLL_EVENT  *epoll_event_tab;
    CEPOLL_NODE   *epoll_node_tab;
    UINT32 size;
    int epoll_fd;
    int sockfd;

    CEPOLL_INIT_LOCK(cepoll, LOC_CEPOLL_0003);
    CEPOLL_COUNTER(cepoll) = 0;
    crb_tree_init(CEPOLL_TIMEOUT_TREE(cepoll), 
                  (CRB_DATA_CMP)cepoll_node_cmp_expire_time, 
                  (CRB_DATA_FREE)NULL_PTR, 
                  (CRB_DATA_PRINT)cepoll_node_print);    
    
    size = ((UINT32)1) * sizeof(CEPOLL_EVENT) * epoll_max_event_num;
    epoll_event_tab = (CEPOLL_EVENT *)safe_malloc(size, LOC_CEPOLL_0004);
    if(NULL_PTR == epoll_event_tab)
    {
        dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:cepoll_init: malloc %d cepoll events failed\n", epoll_max_event_num);
        return (EC_FALSE);        
    }

    size = ((UINT32)1) * sizeof(CEPOLL_NODE) * CEPOLL_MAX_FD_NUM;
    epoll_node_tab = (CEPOLL_NODE *)safe_malloc(size, LOC_CEPOLL_0005);
    if(NULL_PTR == epoll_node_tab)
    {
        safe_free(epoll_event_tab, LOC_CEPOLL_0006);
        dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:cepoll_init: malloc %d cepoll nodes failed\n", CEPOLL_MAX_FD_NUM);
        return (EC_FALSE);        
    }

    for(sockfd = 0; sockfd < CEPOLL_MAX_FD_NUM; sockfd ++)
    {
        CEPOLL_NODE *cepoll_node;

        cepoll_node = &(epoll_node_tab[ sockfd ]);
        
        cepoll_node_init(cepoll_node);
        CEPOLL_NODE_SOCKFD(cepoll_node)    = sockfd;
        CEPOLL_NODE_USED_FLAG(cepoll_node) = CEPOLL_NODE_IS_USED_FLAG;

        //dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_init: sockfd %d, events %d\n", sockfd, CEPOLL_NODE_EVENTS(cepoll_node));
    }
    dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_init: init %d epoll_nodes done\n", sockfd);
    //BSET(epoll_node_tab, 0, size);

    epoll_fd = epoll_create(epoll_max_event_num);
    if(0 > epoll_fd)
    {
        safe_free(epoll_event_tab, LOC_CEPOLL_0007);
        safe_free(epoll_node_tab, LOC_CEPOLL_0008);
        dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:cepoll_init: create epoll with max event num %d failed, errno = %d, errstr = %s\n", 
                           epoll_max_event_num, errno, strerror(errno));
        return (EC_FALSE);
    }

    CEPOLL_FD(cepoll)        = epoll_fd;
    CEPOLL_EVENT_NUM(cepoll) = epoll_max_event_num;
    CEPOLL_EVENT_TAB(cepoll) = epoll_event_tab;
    CEPOLL_NODE_TAB(cepoll)  = epoll_node_tab;

    CEPOLL_LOOP_HANDLER(cepoll) = NULL_PTR;
    CEPOLL_LOOP_ARG(cepoll)     = NULL_PTR;
    
    return (EC_TRUE);
}

EC_BOOL cepoll_clean(CEPOLL *cepoll)
{
    if(ERR_FD != CEPOLL_FD(cepoll))
    {
        close(CEPOLL_FD(cepoll));
        CEPOLL_FD(cepoll) = ERR_FD;
    }

    if(NULL_PTR != CEPOLL_EVENT_TAB(cepoll))
    {
        safe_free(CEPOLL_EVENT_TAB(cepoll), LOC_CEPOLL_0009);
        CEPOLL_EVENT_TAB(cepoll) = NULL_PTR;
    }

    if(NULL_PTR != CEPOLL_NODE_TAB(cepoll))
    {
        safe_free(CEPOLL_NODE_TAB(cepoll), LOC_CEPOLL_0010);
        CEPOLL_NODE_TAB(cepoll) = NULL_PTR;
    }    

    CEPOLL_EVENT_NUM(cepoll) = 0;

    CEPOLL_LOOP_HANDLER(cepoll) = NULL_PTR;
    CEPOLL_LOOP_ARG(cepoll)     = NULL_PTR;    

    crb_tree_clean(CEPOLL_TIMEOUT_TREE(cepoll));
    CEPOLL_CLEAN_LOCK(cepoll, LOC_CEPOLL_0011);
    CEPOLL_COUNTER(cepoll) = 0;
    
    return (EC_TRUE);
}

EC_BOOL cepoll_free(CEPOLL *cepoll)
{
    if(NULL_PTR != cepoll)
    {
        cepoll_clean(cepoll);
        free_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CEPOLL, cepoll, LOC_CEPOLL_0012);
    }
    return (EC_TRUE);
}

static CEPOLL_EVENT *cepoll_fetch_event(const CEPOLL *cepoll, const int pos)
{
    if(0 > pos || pos >= CEPOLL_EVENT_NUM(cepoll))
    {
        return (NULL_PTR);
    }

    return CEPOLL_FETCH_EVENT(cepoll, pos);
}

static CEPOLL_NODE *cepoll_fetch_node(const CEPOLL *cepoll, const int fd)
{
    if(0 > fd || fd >= CEPOLL_MAX_FD_NUM)
    {
        return (NULL_PTR);
    }

    return CEPOLL_FETCH_NODE(cepoll, fd);
}

static int cepoll_fetch_sockfd(const CEPOLL *cepoll, const CEPOLL_NODE *cepoll_node)
{
    UINT32 offset;
    UINT32 remain;

    CEPOLL_NODE *cepoll_node_first;
    CEPOLL_NODE *cepoll_node_last;

    int sockfd;

    cepoll_node_first = CEPOLL_FETCH_NODE(cepoll, 0);
    cepoll_node_last  = CEPOLL_FETCH_NODE(cepoll, CEPOLL_MAX_FD_NUM - 1);

    if(cepoll_node < cepoll_node_first || cepoll_node > cepoll_node_last)
    {
        dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:cepoll_fetch_sockfd: invalid cepoll_node addr %lx\n", cepoll_node);
        return (ERR_FD);
    }

    offset = cepoll_node - cepoll_node_first;
    remain = (offset % sizeof(CEPOLL_NODE));

    if(0 != remain)
    {
        dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:cepoll_fetch_sockfd: invalid cepoll_node addr %p, offset %ld mod %d != 0\n", 
                            cepoll_node, offset, sizeof(CEPOLL_NODE));
        return (ERR_FD);
    }

    sockfd = (int)(offset / sizeof(CEPOLL_NODE));
    return (sockfd);    
}

EC_BOOL cepoll_add(CEPOLL *cepoll, const int sockfd, const uint32_t events)
{ 
    CEPOLL_EVENT cepoll_event;

    CEPOLL_EVENT_TYPE(&cepoll_event) = events;
    CEPOLL_EVENT_FD(&cepoll_event)   = sockfd;

    dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDNULL, "[DEBUG] cepoll_add: sockfd %d add event:%c%c%c%c\n", sockfd, CEPOLL_ALL_EVENT_CHARS(events));    
    
    if(0 != epoll_ctl(CEPOLL_FD(cepoll), EPOLL_CTL_ADD, sockfd, &cepoll_event)) 
    {
        dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:cepoll_add: EPOLL_CTL_ADD failed, sockfd %d, errno = %d, errstr = %s\n", 
                            sockfd, errno, strerror(errno));
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL cepoll_del(CEPOLL *cepoll, const int sockfd, const uint32_t events)
{
    CEPOLL_EVENT cepoll_event;

    CEPOLL_EVENT_TYPE(&cepoll_event) = events;
    CEPOLL_EVENT_FD(&cepoll_event)   = sockfd;

    dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDNULL, "[DEBUG] cepoll_del: sockfd %d del event:%c%c%c%c\n", sockfd, CEPOLL_ALL_EVENT_CHARS(events));    
    
    if(0 != epoll_ctl(CEPOLL_FD(cepoll), EPOLL_CTL_DEL, sockfd, &cepoll_event))
    {
        dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:cepoll_del: EPOLL_CTL_DEL failed, sockfd %d, errno = %d, errstr = %s\n", 
                            sockfd, errno, strerror(errno));
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL cepoll_mod(CEPOLL *cepoll, const int sockfd, const uint32_t events)
{ 
    CEPOLL_EVENT cepoll_event;

    CEPOLL_EVENT_TYPE(&cepoll_event) = events;/*modify to events*/
    CEPOLL_EVENT_FD(&cepoll_event)   = sockfd;

    dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDNULL, "[DEBUG] cepoll_mod: sockfd %d mod to event:%c%c%c%c\n", sockfd, CEPOLL_ALL_EVENT_CHARS(events));    
    
    if(0 != epoll_ctl(CEPOLL_FD(cepoll), EPOLL_CTL_MOD, sockfd, &cepoll_event))
    {
        dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:cepoll_mod: EPOLL_CTL_MOD failed, sockfd %d, errno = %d, errstr = %s\n", 
                            sockfd, errno, strerror(errno));
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL cepoll_set_reader(CEPOLL *cepoll, const int sockfd, CEPOLL_EVENT_HANDLER rd_handler, void *arg)
{
    CEPOLL_NODE *cepoll_node;

    if(0 > sockfd || CEPOLL_MAX_FD_NUM <= sockfd)
    {
        dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:cepoll_set_reader:invalid sockfd %d\n", sockfd);
        return (EC_FALSE);
    }

    cepoll_node = CEPOLL_FETCH_NODE(cepoll, sockfd);

    if(NULL_PTR != CEPOLL_NODE_RD_ARG(cepoll_node))
    {
        dbg_log(SEC_0072_CEPOLL, 2)(LOGSTDOUT, "warn:cepoll_set_reader:sockfd %d change RD_ARG from %p to %p\n", 
                            sockfd, CEPOLL_NODE_RD_ARG(cepoll_node), rd_handler);
        //return (EC_FALSE);
    }    

    CEPOLL_NODE_RD_HANDLER(cepoll_node) = rd_handler;
    CEPOLL_NODE_RD_ARG(cepoll_node)     = arg;
    CEPOLL_NODE_USED_FLAG(cepoll_node)  = CEPOLL_NODE_IS_USED_FLAG;
    
    dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_set_reader: sockfd %d done\n", sockfd);
    return (EC_TRUE);
}

EC_BOOL cepoll_set_writer(CEPOLL *cepoll, const int sockfd, CEPOLL_EVENT_HANDLER wr_handler, void *arg)
{
    CEPOLL_NODE *cepoll_node;

    if(0 > sockfd || CEPOLL_MAX_FD_NUM <= sockfd)
    {
        dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:cepoll_set_writer:invalid sockfd %d\n", sockfd);
        return (EC_FALSE);
    }

    cepoll_node = CEPOLL_FETCH_NODE(cepoll, sockfd);

    if(NULL_PTR != CEPOLL_NODE_WR_ARG(cepoll_node))
    {
        dbg_log(SEC_0072_CEPOLL, 2)(LOGSTDOUT, "warn:cepoll_set_writer:sockfd %d change WR_ARG from %p to %p\n", 
                            sockfd, CEPOLL_NODE_WR_ARG(cepoll_node), wr_handler);
        //return (EC_FALSE);
    }    

    CEPOLL_NODE_WR_HANDLER(cepoll_node) = wr_handler;
    CEPOLL_NODE_WR_ARG(cepoll_node)     = arg;
    CEPOLL_NODE_USED_FLAG(cepoll_node)  = CEPOLL_NODE_IS_USED_FLAG;
    
    dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_set_writer: sockfd %d done\n", sockfd);
    return (EC_TRUE);
}

EC_BOOL cepoll_set_shutdown(CEPOLL *cepoll, const int sockfd, CEPOLL_EVENT_HANDLER shutdown_handler, void *arg)
{
    CEPOLL_NODE *cepoll_node;

    if(0 > sockfd || CEPOLL_MAX_FD_NUM <= sockfd)
    {
        dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:cepoll_set_shutdown:invalid sockfd %d\n", sockfd);
        return (EC_FALSE);
    }

    cepoll_node = CEPOLL_FETCH_NODE(cepoll, sockfd);

    if(NULL_PTR != CEPOLL_NODE_SHUTDOWN_ARG(cepoll_node))
    {
        dbg_log(SEC_0072_CEPOLL, 2)(LOGSTDOUT, "warn:cepoll_set_shutdown:sockfd %d change SHUTDOWN_ARG from %p to %p\n", 
                            sockfd, CEPOLL_NODE_SHUTDOWN_ARG(cepoll_node), arg);
        //return (EC_FALSE);
    }    

    CEPOLL_NODE_SHUTDOWN_HANDLER(cepoll_node) = shutdown_handler;
    CEPOLL_NODE_SHUTDOWN_ARG(cepoll_node)     = arg;
    CEPOLL_NODE_USED_FLAG(cepoll_node)        = CEPOLL_NODE_IS_USED_FLAG;
    
    dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_set_shutdown: sockfd %d done\n", sockfd);
    return (EC_TRUE);
}

EC_BOOL cepoll_set_timeout(CEPOLL *cepoll, const int sockfd, const uint32_t timeout_nsec, CEPOLL_EVENT_HANDLER timeout_handler, void *arg)
{
    CEPOLL_NODE *cepoll_node;

    if(0 > sockfd || CEPOLL_MAX_FD_NUM <= sockfd)
    {
        dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:cepoll_set_timeout:invalid sockfd %d\n", sockfd);
        return (EC_FALSE);
    }

    cepoll_node = CEPOLL_FETCH_NODE(cepoll, sockfd);

    if(NULL_PTR != CEPOLL_NODE_TIMEOUT_ARG(cepoll_node))
    {
        dbg_log(SEC_0072_CEPOLL, 2)(LOGSTDOUT, "warn:cepoll_set_timeout:sockfd %d change TIMEOUT_ARG from %p to %p\n", 
                            sockfd, CEPOLL_NODE_TIMEOUT_ARG(cepoll_node), timeout_handler);
        //return (EC_FALSE);
    }    

    CEPOLL_NODE_ATIME_TS(cepoll_node)        = task_brd_default_get_time();
    CEPOLL_NODE_TIMEOUT_NSEC(cepoll_node)    = timeout_nsec;
    CEPOLL_NODE_TIMEOUT_HANDLER(cepoll_node) = timeout_handler;
    CEPOLL_NODE_TIMEOUT_ARG(cepoll_node)     = arg;
    CEPOLL_NODE_USED_FLAG(cepoll_node)       = CEPOLL_NODE_IS_USED_FLAG;

    if(0 < CEPOLL_NODE_TIMEOUT_NSEC(cepoll_node))
    {
        CRB_NODE *crb_node;
        CEPOLL_LOCK(cepoll, LOC_CEPOLL_0013);        
        if(NULL_PTR != CEPOLL_NODE_CRB_NODE(cepoll_node))/*prevent from set timeout again*/
        {
            crb_tree_delete(CEPOLL_TIMEOUT_TREE(cepoll), CEPOLL_NODE_CRB_NODE(cepoll_node));
            CEPOLL_NODE_CRB_NODE(cepoll_node) = NULL_PTR;
        }
        CEPOLL_NODE_COUNTER(cepoll_node) = ++ CEPOLL_COUNTER(cepoll);
        crb_node = crb_tree_insert_data(CEPOLL_TIMEOUT_TREE(cepoll), (void *)cepoll_node);
        CEPOLL_UNLOCK(cepoll, LOC_CEPOLL_0014);
        
        if(NULL_PTR == crb_node)
        {
            dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:cepoll_set_timeout:sockfd %d cepll_node %p insert to tree %p failed\n", 
                                sockfd, cepoll_node, CEPOLL_TIMEOUT_TREE(cepoll));
            CEPOLL_NODE_CRB_NODE(cepoll_node) = NULL_PTR;
            return (EC_FALSE);
        }

        if(CRB_NODE_DATA(crb_node) != cepoll_node)
        {
            dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:cepoll_set_timeout:sockfd %d cepll_node %p insert to tree %p but find duplicate\n", 
                                sockfd, cepoll_node, CEPOLL_TIMEOUT_TREE(cepoll));
            CEPOLL_NODE_CRB_NODE(cepoll_node) = NULL_PTR;
            return (EC_FALSE);
        }

        CEPOLL_NODE_CRB_NODE(cepoll_node) = crb_node;
    }
    
    dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_set_timeout: sockfd %d, start %d, timeout %d, handler %p, arg %p done\n", 
                       sockfd, 
                       CEPOLL_NODE_ATIME_TS(cepoll_node), CEPOLL_NODE_TIMEOUT_NSEC(cepoll_node),
                       CEPOLL_NODE_TIMEOUT_HANDLER(cepoll_node), CEPOLL_NODE_TIMEOUT_ARG(cepoll_node));

    //dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_set_timeout: cepoll timeout list is\n");
    //crb_tree_print(LOGSTDOUT, CEPOLL_TIMEOUT_TREE(cepoll));
    return (EC_TRUE);
}

EC_BOOL cepoll_set_used(CEPOLL *cepoll, const int sockfd)
{
    CEPOLL_NODE *cepoll_node;

    if(0 > sockfd || CEPOLL_MAX_FD_NUM <= sockfd)
    {
        dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:cepoll_set_used:invalid sockfd %d\n", sockfd);
        return (EC_FALSE);
    }

    cepoll_node = CEPOLL_FETCH_NODE(cepoll, sockfd);

    cepoll_node_set_used(cepoll_node);
    return (EC_TRUE);
}

EC_BOOL cepoll_set_not_used(CEPOLL *cepoll, const int sockfd)
{
    CEPOLL_NODE *cepoll_node;

    if(0 > sockfd || CEPOLL_MAX_FD_NUM <= sockfd)
    {
        dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:cepoll_set_not_used:invalid sockfd %d\n", sockfd);
        return (EC_FALSE);
    }

    cepoll_node = CEPOLL_FETCH_NODE(cepoll, sockfd);

    cepoll_node_set_not_used(cepoll_node);
    return (EC_TRUE);
}

EC_BOOL cepoll_del_event(CEPOLL *cepoll, const int sockfd, const uint32_t event)
{
    CEPOLL_NODE *cepoll_node;
    uint32_t events_src;
    uint32_t events_des;    

    if(0 > sockfd || CEPOLL_MAX_FD_NUM <= sockfd)
    {
        dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:cepoll_del_event:invalid sockfd %d\n", sockfd);
        return (EC_FALSE);
    }

    cepoll_node = CEPOLL_FETCH_NODE(cepoll, sockfd);
    events_src  = CEPOLL_NODE_EVENTS(cepoll_node);

    if(CEPOLL_RD_EVENT == event)
    {
        dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_del_event: sockfd %d del reader (func %p, arg %p)\n", 
                            sockfd, CEPOLL_NODE_RD_HANDLER(cepoll_node), CEPOLL_NODE_RD_ARG(cepoll_node));        

        /*del reader*/
        CEPOLL_NODE_RD_HANDLER(cepoll_node) = NULL_PTR;
        CEPOLL_NODE_RD_ARG(cepoll_node)     = NULL_PTR;        
    }
    else if(CEPOLL_WR_EVENT == event)
    {
        dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_del_event: sockfd %d del writer (func %p, arg %p)\n", 
                            sockfd, CEPOLL_NODE_WR_HANDLER(cepoll_node), CEPOLL_NODE_WR_ARG(cepoll_node));        

        /*del writer*/    
        CEPOLL_NODE_WR_HANDLER(cepoll_node) = NULL_PTR;
        CEPOLL_NODE_WR_ARG(cepoll_node)     = NULL_PTR;
    }
    else
    {
        dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:cepoll_del_event:sockfd %d reject del invalid event:%c%c%c%c [%x]\n", 
                            sockfd, CEPOLL_ALL_EVENT_CHARS(event), event);
        return (EC_FALSE);
    }

    if(0 == (events_src & event))
    {
        return (EC_TRUE);
    }

    events_des = (events_src & (~event));    
    CEPOLL_NODE_EVENTS(cepoll_node) = events_des;

    if(events_des & (CEPOLL_RD_EVENT | CEPOLL_WR_EVENT))/*fuck trap!*/
    {
        dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_del_event: sockfd %d modify events %c%c%c%c => %c%c%c%c\n", 
                            sockfd, CEPOLL_ALL_EVENT_CHARS(events_src), CEPOLL_ALL_EVENT_CHARS(events_des));        
        return cepoll_mod(cepoll, sockfd, events_des);
    }

    dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_del_event: sockfd %d del events %c%c%c%c => %c%c%c%c\n", 
                        sockfd, CEPOLL_ALL_EVENT_CHARS(events_src), CEPOLL_ALL_EVENT_CHARS(events_des));
    return cepoll_del(cepoll, sockfd, event);
}

EC_BOOL cepoll_del_events(CEPOLL *cepoll, const int sockfd, const uint32_t events)
{
    CEPOLL_NODE *cepoll_node;
    uint32_t events_src;
    uint32_t events_des;
    uint32_t events_t;

    if(0 > sockfd || CEPOLL_MAX_FD_NUM <= sockfd)
    {
        dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:cepoll_del_events:invalid sockfd %d\n", sockfd);
        return (EC_FALSE);
    }

    cepoll_node = CEPOLL_FETCH_NODE(cepoll, sockfd);
    events_src  = CEPOLL_NODE_EVENTS(cepoll_node);
    events_des  = events_src & (~events);

    dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_del_events: sockfd %d: %c%c%c%c -> %c%c%c%c\n", 
                        sockfd, 
                        CEPOLL_ALL_EVENT_CHARS(events_src),
                        CEPOLL_ALL_EVENT_CHARS(events_des));
    
    events_t = (events_src & events);
    CEPOLL_NODE_EVENTS(cepoll_node) = events_des;
    dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDNULL, "[DEBUG] cepoll_del_events: CEPOLL_NODE_EVENTS of sockfd %d changed to %d: %c%c%c%c\n", 
                       sockfd, CEPOLL_NODE_EVENTS(cepoll_node), CEPOLL_ALL_EVENT_CHARS(events_des));
    
    dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDNULL, "[DEBUG] cepoll_del_events: sockfd %d del epoll events %c%c%c%c\n", 
                        sockfd, CEPOLL_ALL_EVENT_CHARS(events_t));

    if(events_t & (CEPOLL_IN | CEPOLL_OUT))
    {
        return cepoll_del(cepoll, sockfd, events_t);
    }
                        
    return (EC_TRUE);
}

EC_BOOL cepoll_del_all(CEPOLL *cepoll, const int sockfd)
{
    CEPOLL_NODE *cepoll_node;
    uint32_t     events;

    if(0 > sockfd || CEPOLL_MAX_FD_NUM <= sockfd)
    {
        dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:cepoll_del_all:invalid sockfd %d\n", sockfd);
        return (EC_FALSE);
    }

    cepoll_node = CEPOLL_FETCH_NODE(cepoll, sockfd);
    events      = CEPOLL_NODE_EVENTS(cepoll_node);
    dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_del_all: sockfd %d del all epoll events %c%c%c%c\n", 
                       sockfd, CEPOLL_ALL_EVENT_CHARS(events));    

    if(NULL_PTR != CEPOLL_NODE_CRB_NODE(cepoll_node))
    {
        CEPOLL_LOCK(cepoll, LOC_CEPOLL_0015);
        crb_tree_delete(CEPOLL_TIMEOUT_TREE(cepoll), CEPOLL_NODE_CRB_NODE(cepoll_node));
        CEPOLL_NODE_CRB_NODE(cepoll_node) = NULL_PTR;
        CEPOLL_UNLOCK(cepoll, LOC_CEPOLL_0016);        
    }
    cepoll_node_clear(cepoll_node);
        
    if((CEPOLL_IN | CEPOLL_OUT) & events)
    {
        return cepoll_del(cepoll, sockfd, events);
    }
    return (EC_TRUE);
}

EC_BOOL cepoll_set_event(CEPOLL *cepoll, const int sockfd, const uint32_t event, CEPOLL_EVENT_HANDLER handler, void *arg)
{
    CEPOLL_NODE *cepoll_node;
    uint32_t events_src;
    uint32_t events_des;

    if(0 > sockfd || CEPOLL_MAX_FD_NUM <= sockfd)
    {
        dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:cepoll_set_event:invalid sockfd %d\n", sockfd);
        return (EC_FALSE);
    }

    cepoll_node = CEPOLL_FETCH_NODE(cepoll, sockfd);
    events_src  = CEPOLL_NODE_EVENTS(cepoll_node);

    if(CEPOLL_RD_EVENT == event)
    {
        /*set reader*/
        CEPOLL_NODE_RD_HANDLER(cepoll_node) = handler;
        CEPOLL_NODE_RD_ARG(cepoll_node)     = arg;
        CEPOLL_NODE_USED_FLAG(cepoll_node)  = CEPOLL_NODE_IS_USED_FLAG;  

        dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_set_event: sockfd %d set reader (func %p, arg %p)\n", 
                            sockfd, handler, arg);        
    }
    else if(CEPOLL_WR_EVENT == event)
    {
        CEPOLL_NODE_WR_HANDLER(cepoll_node) = handler;
        CEPOLL_NODE_WR_ARG(cepoll_node)     = arg;
        CEPOLL_NODE_USED_FLAG(cepoll_node)  = CEPOLL_NODE_IS_USED_FLAG;  
        
        dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_set_event: sockfd %d set writer (func %p, arg %p)\n", 
                            sockfd, handler, arg);                
    }
    else
    {
        dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:cepoll_set_event:sockfd %d reject set to invalid event:%c%c%c%c [%x]\n", 
                            sockfd, CEPOLL_ALL_EVENT_CHARS(event), event);
        return (EC_FALSE);
    }

    if(0 != (events_src & event))/*event already exist*/
    {
        return (EC_TRUE);
    }

    events_des = (events_src | event | CEPOLL_HUP | CEPOLL_ERR);
    CEPOLL_NODE_EVENTS(cepoll_node) = events_des;
    
    if(0 != (events_src & (~(CEPOLL_HUP | CEPOLL_ERR))))/*if some event exist, modify*/
    {
        dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_set_event: sockfd %d modify events %c%c%c%c => %c%c%c%c\n", 
                            sockfd, CEPOLL_ALL_EVENT_CHARS(events_src), CEPOLL_ALL_EVENT_CHARS(events_des));
        return cepoll_mod(cepoll, sockfd, events_des);    
    }

    /*if no event exist, add*/
    dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_set_event: sockfd %d add events %c%c%c%c => %c%c%c%c\n", 
                        sockfd, CEPOLL_ALL_EVENT_CHARS(events_src), CEPOLL_ALL_EVENT_CHARS(events_des));
                        
    return cepoll_add(cepoll, sockfd, events_des); 
}

EC_BOOL cepoll_set_events(CEPOLL *cepoll, const int sockfd, const uint32_t events)
{
    CEPOLL_NODE *cepoll_node;
    uint32_t events_src;
    uint32_t events_des;

    if(0 > sockfd || CEPOLL_MAX_FD_NUM <= sockfd)
    {
        dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:cepoll_set_events:invalid sockfd %d\n", sockfd);
        return (EC_FALSE);
    }

    cepoll_node = CEPOLL_FETCH_NODE(cepoll, sockfd);
    events_src  = CEPOLL_NODE_EVENTS(cepoll_node);
    events_des  = ((0 == events) ? events : (events | CEPOLL_HUP | CEPOLL_ERR));

    if(events_src == events_des)
    {
        dbg_log(SEC_0072_CEPOLL, 2)(LOGSTDOUT, "warn:cepoll_set_events:sockfd %d not need update event:%c%c%c%c\n", 
                            sockfd, CEPOLL_ALL_EVENT_CHARS(events_src));
        return (EC_TRUE);
    }

    if(0 == events_des)
    {
        CEPOLL_NODE_EVENTS(cepoll_node) = events_des;
        //dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_set_events: CEPOLL_NODE_EVENTS of sockfd %d changed to %d [1]\n", sockfd, CEPOLL_NODE_EVENTS(cepoll_node));
        return cepoll_del(cepoll, sockfd, events_des);
    }

    if(0 < (events_src & (~(CEPOLL_HUP | CEPOLL_ERR))))
    {
        dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_set_events:sockfd %d mod event:%c%c%c%c --> %c%c%c%c\n", 
                            sockfd, CEPOLL_ALL_EVENT_CHARS(events_src), CEPOLL_ALL_EVENT_CHARS(events_des));    
        CEPOLL_NODE_EVENTS(cepoll_node) = events_des;
        dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_set_events: CEPOLL_NODE_EVENTS of sockfd %d changed to %c%c%c%c [2]\n", sockfd, CEPOLL_ALL_EVENT_CHARS(CEPOLL_NODE_EVENTS(cepoll_node)));
        return cepoll_mod(cepoll, sockfd, events_des);
    }

    /*else, add events*/
    CEPOLL_NODE_EVENTS(cepoll_node) = events_des;
    dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_set_events: CEPOLL_NODE_EVENTS of sockfd %d changed to %c%c%c%c [3]\n", sockfd, CEPOLL_ALL_EVENT_CHARS(CEPOLL_NODE_EVENTS(cepoll_node)));
    return cepoll_add(cepoll, sockfd, events_des);
}

EC_BOOL cepoll_set_loop_handler(CEPOLL *cepoll, CEPOLL_LOOP_HANDLER handler, void *arg)
{
    if(NULL_PTR != CEPOLL_LOOP_HANDLER(cepoll))
    {
        dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:cepoll_set_loop_handler: loop handler was set already\n");
        return (EC_FALSE);
    }

    CEPOLL_LOOP_HANDLER(cepoll) = handler;
    CEPOLL_LOOP_ARG(cepoll)     = arg;

    dbg_log(SEC_0072_CEPOLL, 5)(LOGSTDOUT, "cepoll_set_loop_handler: loop handler set done\n");
    return (EC_TRUE);
}

EC_BOOL cepoll_update_atime(CEPOLL *cepoll, const int sockfd)
{
    CEPOLL_NODE  *cepoll_node;
    CTIMET        atime_old;
    
    cepoll_node  = CEPOLL_FETCH_NODE(cepoll, sockfd);        
    if(NULL_PTR == cepoll_node)
    {
        dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_update_atime: sockfd %d not register cepoll node\n", sockfd);
        return (EC_FALSE);                                
    }

    atime_old = CEPOLL_NODE_ATIME_TS(cepoll_node);
    
    /*update last access time*/
    CEPOLL_NODE_ATIME_TS(cepoll_node) = task_brd_default_get_time();

    if(atime_old == CEPOLL_NODE_ATIME_TS(cepoll_node))
    {
        return (EC_TRUE);
    }

    if(NULL_PTR != CEPOLL_NODE_CRB_NODE(cepoll_node))
    {
        CRB_NODE *crb_node;
        CEPOLL_LOCK(cepoll, LOC_CEPOLL_0017);
        crb_tree_delete(CEPOLL_TIMEOUT_TREE(cepoll), CEPOLL_NODE_CRB_NODE(cepoll_node));
        CEPOLL_NODE_CRB_NODE(cepoll_node) = NULL_PTR;
        CEPOLL_NODE_COUNTER(cepoll_node) = ++ CEPOLL_COUNTER(cepoll);
        crb_node = crb_tree_insert_data(CEPOLL_TIMEOUT_TREE(cepoll), (void *)cepoll_node);
        CEPOLL_UNLOCK(cepoll, LOC_CEPOLL_0018);

        if(NULL_PTR == crb_node)
        {
            dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:cepoll_update_atime: sockfd %d insert cepoll_node %p to tree %p failed\n", 
                               sockfd, cepoll_node, CEPOLL_TIMEOUT_TREE(cepoll));
            return (EC_FALSE);
        }

        if(CRB_NODE_DATA(crb_node) != cepoll_node)
        {
            dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:cepoll_update_atime:sockfd %d cepll_node %p insert to tree %p but find duplicate\n", 
                                sockfd, cepoll_node, CEPOLL_TIMEOUT_TREE(cepoll));
            CEPOLL_NODE_CRB_NODE(cepoll_node) = NULL_PTR;
            return (EC_FALSE);
        }
        
        CEPOLL_NODE_CRB_NODE(cepoll_node) = crb_node;
    }

    return (EC_TRUE);
}

static EC_BOOL __cepoll_timeout_handle(CEPOLL *cepoll, const int sockfd, CEPOLL_NODE  *cepoll_node)
{
    if(NULL_PTR != CEPOLL_NODE_TIMEOUT_HANDLER(cepoll_node))
    {
        dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] __cepoll_timeout_handle: sockfd %d handle TIMEOUT \n", sockfd); 
    
        CEPOLL_NODE_TIMEOUT_HANDLER(cepoll_node)(CEPOLL_NODE_TIMEOUT_ARG(cepoll_node));
    }
    return (EC_TRUE);
}

static EC_BOOL __cepoll_shutdown_handle(CEPOLL *cepoll, const int sockfd, CEPOLL_NODE  *cepoll_node)
{
    if(NULL_PTR != CEPOLL_NODE_SHUTDOWN_HANDLER(cepoll_node))
    {
        dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] __cepoll_shutdown_handle: sockfd %d handle SHUTDOWN \n", sockfd); 
    
        CEPOLL_NODE_SHUTDOWN_HANDLER(cepoll_node)(CEPOLL_NODE_SHUTDOWN_ARG(cepoll_node));
    }
    return (EC_TRUE);
}

static EC_BOOL __cepoll_read_handle(CEPOLL *cepoll, const int sockfd, CEPOLL_NODE  *cepoll_node)
{
    if(EC_FALSE == CEPOLL_NODE_RD_HANDLER(cepoll_node)(CEPOLL_NODE_RD_ARG(cepoll_node)))
    {
        //dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:__cepoll_read_handle: sockfd %d handle RD event failed, trigger shutdown\n", sockfd); 
        //__cepoll_shutdown_handle(cepoll, sockfd, cepoll_node);
        return (EC_FALSE);
    }
    
    return (EC_TRUE);
}

static EC_BOOL __cepoll_write_handle(CEPOLL *cepoll, const int sockfd, CEPOLL_NODE  *cepoll_node)
{
    if(EC_FALSE == CEPOLL_NODE_WR_HANDLER(cepoll_node)(CEPOLL_NODE_WR_ARG(cepoll_node)))
    {
        //dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:__cepoll_write_handle: sockfd %d handle WR event failed, trigger shutdown\n", sockfd); 
        //__cepoll_shutdown_handle(cepoll, sockfd, cepoll_node);
        return (EC_FALSE);
    }
    
    return (EC_TRUE);
}

EC_BOOL cepoll_handle(CEPOLL *cepoll,  const int sockfd, const uint32_t events, CEPOLL_NODE  *cepoll_node)
{
    dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_handle: enter, sockfd %d, events %c%c%c%c, rd handler %p, wr handler %p\n", 
                       sockfd, 
                       CEPOLL_ALL_EVENT_CHARS(events), 
                       CEPOLL_NODE_RD_HANDLER(cepoll_node), 
                       CEPOLL_NODE_WR_HANDLER(cepoll_node));
                       
    if(events & (CEPOLL_HUP | CEPOLL_ERR))
    {    
        dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:cepoll_handle: sockfd %d trigger event HP or ERR\n", sockfd);
        return (EC_FALSE);
    }

    if((events & CEPOLL_IN) && (NULL_PTR != CEPOLL_NODE_RD_HANDLER(cepoll_node)))
    {
        dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_handle: try to read on sockfd %d\n", sockfd);
        if(EC_FALSE == __cepoll_read_handle(cepoll, sockfd, cepoll_node))
        {
            dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:cepoll_handle: sockfd %d read failed\n", sockfd);         
            return (EC_FALSE);
        }
    }

    if((events & CEPOLL_OUT) && (NULL_PTR != CEPOLL_NODE_WR_HANDLER(cepoll_node)))
    {
        dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_handle: try to write on sockfd %d\n", sockfd);
        if(EC_FALSE == __cepoll_write_handle(cepoll, sockfd, cepoll_node))
        {
            dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:cepoll_handle: sockfd %d write failed\n", sockfd);         
            return (EC_FALSE);
        }
    }    

    //dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_handle: try nothing on sockfd %d\n", sockfd);
    
    return (EC_TRUE);
}

EC_BOOL cepoll_timeout(CEPOLL *cepoll)
{   
    CTIMET cur_ts;
    UINT32 timeout_num;

    //dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_timeout: nodes num %ld\n", crb_tree_node_num(CEPOLL_TIMEOUT_TREE(cepoll)));
    //dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_timeout: beg\n");

    cur_ts = task_brd_default_get_time();

    timeout_num = 0;

    CEPOLL_LOCK(cepoll, LOC_CEPOLL_0019);
    while(EC_FALSE == crb_tree_is_empty(CEPOLL_TIMEOUT_TREE(cepoll)) && CRFSCHTTP_TIMEOUT_MAX_NUM > timeout_num)
    {
        CRB_NODE    *crb_node;
        CEPOLL_NODE *cepoll_node;
        int sockfd;

        crb_node = (CRB_NODE *)crb_tree_first_node(CEPOLL_TIMEOUT_TREE(cepoll));
        if(NULL_PTR == crb_node)
        {
            break;
        }
        
        cepoll_node = (CEPOLL_NODE *)CRB_NODE_DATA(crb_node);
        if(NULL_PTR == cepoll_node)
        {
            break;
        }

        if(cur_ts < CEPOLL_NODE_ATIME_TS(cepoll_node) + CEPOLL_NODE_TIMEOUT_NSEC(cepoll_node))
        {
            break;
        }

        /*handle one timeout node*/
        timeout_num ++;
        
        sockfd = CEPOLL_NODE_SOCKFD(cepoll_node);

        dbg_log(SEC_0072_CEPOLL, 1)(LOGSTDOUT, "cepoll_timeout: sockfd %d cur_ts %u, start_ts %u, timeout_nsec %u\n", 
                           sockfd, cur_ts,  CEPOLL_NODE_ATIME_TS(cepoll_node), CEPOLL_NODE_TIMEOUT_NSEC(cepoll_node));

        ASSERT(crb_node == CEPOLL_NODE_CRB_NODE(cepoll_node));
        crb_tree_delete(CEPOLL_TIMEOUT_TREE(cepoll), crb_node);
        /*clean it and then cepoll_del_all will not call clist_rmv which cause dead loop in locking*/
        CEPOLL_NODE_CRB_NODE(cepoll_node) = NULL_PTR;

        cepoll_del_all(cepoll, sockfd);
        __cepoll_timeout_handle(cepoll, sockfd, cepoll_node);
    }

    CEPOLL_UNLOCK(cepoll, LOC_CEPOLL_0020);
    //dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_timeout: end\n");
    return (EC_TRUE);
}

EC_BOOL cepoll_del_timeout_event(CEPOLL *cepoll, const int sockfd)
{
    CEPOLL_NODE *cepoll_node;

    if(0 > sockfd || CEPOLL_MAX_FD_NUM <= sockfd)
    {
        dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:cepoll_del_timeout_event:invalid sockfd %d\n", sockfd);
        return (EC_FALSE);
    }

    cepoll_node = CEPOLL_FETCH_NODE(cepoll, sockfd);

    CEPOLL_NODE_TIMEOUT_HANDLER(cepoll_node) = NULL_PTR;
    CEPOLL_NODE_TIMEOUT_ARG(cepoll_node)     = NULL_PTR;

    if(NULL_PTR == CEPOLL_NODE_CRB_NODE(cepoll_node))
    {
        return (EC_TRUE);
    }

    CEPOLL_LOCK(cepoll, LOC_CEPOLL_0021);
    crb_tree_delete(CEPOLL_TIMEOUT_TREE(cepoll), CEPOLL_NODE_CRB_NODE(cepoll_node));
    CEPOLL_NODE_CRB_NODE(cepoll_node) = NULL_PTR;
    CEPOLL_UNLOCK(cepoll, LOC_CEPOLL_0022);

    return (EC_TRUE);
}

static EC_BOOL __cepoll_no_error(const int ierrno)
{
    switch (ierrno)
    {
        case EINPROGRESS:
        case EWOULDBLOCK:
#if (EAGAIN != EWOULDBLOCK)
        case EAGAIN:
#endif/*(EAGAIN != EWOULDBLOCK)*/
        case EALREADY:
        case EINTR:
#ifdef ERESTART        
        case ERESTART:
#endif/*ERESTART*/        
            return (EC_TRUE);
        default:
            dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "warn:__cepoll_no_error: errno = %d, errstr = %s\n", ierrno, strerror(ierrno));
            return (EC_FALSE);
    }

    dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error: __cepoll_no_error: should never reach here\n");
    return (EC_FALSE);
}

EC_BOOL cepoll_wait(CEPOLL *cepoll, int timeout_ms)
{
    int sockfd_idx;
    int sockfd_num;

    dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDNULL, "[DEBUG] cepoll_wait: epoll_wait beg where timeout in %d ms\n", timeout_ms);

#if 0
    do
    {
        sockfd_num = epoll_wait(CEPOLL_FD(cepoll), CEPOLL_EVENT_TAB(cepoll), CEPOLL_EVENT_NUM(cepoll), timeout_ms);
    }while(0 > sockfd_num && EINTR == errno);

    if(0 > sockfd_num)
    {
        dbg_log(SEC_0072_CEPOLL, 0)(LOGSTDOUT, "error:cepoll_wait: errno = %d, errstr = %s\n", errno, strerror(errno));
        return (EC_FALSE);    
    }
#endif

#if 1
    sockfd_num = epoll_wait(CEPOLL_FD(cepoll), CEPOLL_EVENT_TAB(cepoll), CEPOLL_EVENT_NUM(cepoll), timeout_ms);
    
    if(0 > sockfd_num)
    {
        int errcode;

        errcode = errno;
        dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_wait: epoll_wait end and sockfd_num %d < 0, errno = %d, errstr = %s\n", 
                            sockfd_num, errcode, strerror(errcode));
        cepoll_timeout(cepoll);                            
        return __cepoll_no_error(errcode);
    }
#endif
    //dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_wait: epoll_wait end and sockfd_num %d >= 0\n", sockfd_num);
    
    /*no file descriptor became ready during the requested timeout milliseconds*/
    if (0 == sockfd_num)/*due to timeout*/
    {
        cepoll_timeout(cepoll);
        return (EC_TRUE);
    }

    dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDNULL, "[DEBUG] cepoll_wait: return sockfd_num %d\n", sockfd_num);

    /*debug*/
    if(0)
    {
        CSTRING *info;

        info = cstring_new(NULL_PTR, LOC_CEPOLL_0023);
        ASSERT(NULL_PTR != info);
        
        for(sockfd_idx = 0 ; sockfd_idx < sockfd_num; sockfd_idx ++)
        {
            CEPOLL_EVENT *cepoll_event;
            CEPOLL_NODE  *cepoll_node;
            int      sockfd;
            uint32_t events_t;
            
            cepoll_event = CEPOLL_FETCH_EVENT(cepoll, sockfd_idx);
            sockfd       = CEPOLL_EVENT_FD(cepoll_event);
            events_t     = CEPOLL_EVENT_TYPE(cepoll_event);
            
            cepoll_node  = CEPOLL_FETCH_NODE(cepoll, sockfd);        
            if(NULL_PTR == cepoll_node)
            {
                dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_wait: sockfd_idx %d: sockfd %d not register cepoll node\n", sockfd_idx, sockfd);
                cstring_free(info);
                continue;                                
            }

            if(CEPOLL_NODE_NOT_USED_FLAG == CEPOLL_NODE_USED_FLAG(cepoll_node))
            {
                dbg_log(SEC_0072_CEPOLL, 2)(LOGSTDOUT, "warn:cepoll_wait: sockfd_idx %d: sockfd %d not used\n", sockfd_idx, sockfd);
                cstring_free(info);
                continue;                                
            }

            cstring_format(info, "[INFO] cepoll_wait: sockfd %d trigger %c%c%c%c of %c%c%c%c\n", 
                                sockfd, 
                                CEPOLL_ALL_EVENT_CHARS(events_t),
                                CEPOLL_ALL_EVENT_CHARS(CEPOLL_NODE_EVENTS(cepoll_node)));
        }        

        dbg_log(SEC_0072_CEPOLL, 1)(LOGSTDOUT, "%s\n", cstring_get_str(info));
        cstring_free(info);
    }

    for(sockfd_idx = 0 ; sockfd_idx < sockfd_num; sockfd_idx ++)
    {
        CEPOLL_EVENT *cepoll_event;
        CEPOLL_NODE  *cepoll_node;
        int      sockfd;
        uint32_t events_t;
        
        cepoll_event = CEPOLL_FETCH_EVENT(cepoll, sockfd_idx);
        sockfd       = CEPOLL_EVENT_FD(cepoll_event);
        events_t     = CEPOLL_EVENT_TYPE(cepoll_event);
        
        cepoll_node  = CEPOLL_FETCH_NODE(cepoll, sockfd);        
        if(NULL_PTR == cepoll_node)
        {
            dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_wait: sockfd_idx %d: sockfd %d not register cepoll node\n", sockfd_idx, sockfd);
            continue;                                
        }

        if(CEPOLL_NODE_NOT_USED_FLAG == CEPOLL_NODE_USED_FLAG(cepoll_node))
        {
            dbg_log(SEC_0072_CEPOLL, 2)(LOGSTDOUT, "warn:cepoll_wait: sockfd_idx %d: sockfd %d not used\n", sockfd_idx, sockfd);
            continue;                                
        }

        dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_wait: sockfd %d trigger %c%c%c%c of %c%c%c%c\n", 
                            sockfd, 
                            CEPOLL_ALL_EVENT_CHARS(events_t),
                            CEPOLL_ALL_EVENT_CHARS(CEPOLL_NODE_EVENTS(cepoll_node)));

        cepoll_update_atime(cepoll, sockfd);
        
        if(EC_FALSE == cepoll_handle(cepoll, sockfd, events_t, cepoll_node))
        {
            dbg_log(SEC_0072_CEPOLL, 9)(LOGSTDOUT, "[DEBUG] cepoll_wait: sockfd %d trigger shutdown and epoll events remove\n", sockfd);    
            __cepoll_shutdown_handle(cepoll, sockfd, cepoll_node);            
            cepoll_del_all(cepoll, sockfd);
        }
    }

    cepoll_timeout(cepoll);

    if(NULL_PTR != CEPOLL_LOOP_HANDLER(cepoll))
    {
        CEPOLL_LOOP_HANDLER(cepoll)(CEPOLL_LOOP_ARG(cepoll));
    }

    return (EC_TRUE);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/
