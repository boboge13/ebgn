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

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

#include "type.h"
#include "cqueue.h"
#include "mm.h"
#include "log.h"

#include "bgnctrl.h"

CQUEUE *cqueue_new(const UINT32 mm_type, const UINT32 location)
{
    return clist_new(mm_type, location);
}

void cqueue_free(CQUEUE *cqueue, const UINT32 location)
{
    clist_free(cqueue, location);
    return;
}

void cqueue_init(CQUEUE *cqueue, const UINT32 mm_type, const UINT32 location)
{
    clist_init(cqueue, mm_type, location);
    return;
}

EC_BOOL cqueue_is_empty(const CQUEUE *cqueue)
{
    return clist_is_empty(cqueue);
}

CQUEUE_DATA * cqueue_search(const CQUEUE *cqueue, const void *data, EC_BOOL (*cmp)(const void *, const void *))
{
    return clist_search_front(cqueue, data, cmp);
}

CQUEUE_DATA * cqueue_push(CQUEUE *cqueue, void *data)
{
    return clist_push_back(cqueue, data);
}

void * cqueue_pop(CQUEUE *cqueue)
{
    return clist_pop_front(cqueue);
}

UINT32 cqueue_size(const CQUEUE *cqueue)
{
    return clist_size(cqueue);
}

void *cqueue_front(const CQUEUE *cqueue)
{
    return clist_front(cqueue);
}

void *cqueue_back(const CQUEUE *cqueue)
{
    return clist_back(cqueue);
}

void cqueue_loop_front(const CQUEUE *cqueue, void (*handler)(void *))
{
    clist_loop_front(cqueue, handler);
    return;
}

void cqueue_loop_back(const CQUEUE *cqueue, void (*handler)(void *))
{
    clist_loop_back(cqueue, handler);
    return;
}

void cqueue_clean(CQUEUE *cqueue, void (*cleaner)(void *))
{
    clist_clean(cqueue, cleaner);
    return;
}

void cqueue_print(LOG *log, const CQUEUE *cqueue, void (*print)(LOG *, const void *))
{
    clist_print(log, cqueue, print);
}
#ifdef __cplusplus
}
#endif/*__cplusplus*/
