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
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>

#include "type.h"
#include "mm.h"
#include "log.h"
#include "cmpic.inc"
#include "cmutex.h"
#include "cmisc.h"

#include "real.h"

#include "clist.h"

#include "task.h"

#include "cdsk.h"
#include "crb.h"
#include "crfsdn.h"
#include "cpgrb.h"
#include "cpgb.h"
#include "cpgd.h"
#include "cpgv.h"


/*Random File System Data Node*/
CRFSDN_NODE *crfsdn_node_new()
{
    CRFSDN_NODE *crfsdn_node;

    alloc_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CRFSDN_NODE, &crfsdn_node, LOC_CRFSDN_0001);
    if(NULL_PTR != crfsdn_node)
    {
        crfsdn_node_init(crfsdn_node);
    }
    return (crfsdn_node);
}

EC_BOOL crfsdn_node_init(CRFSDN_NODE *crfsdn_node)
{
    CRFSDN_NODE_ID(crfsdn_node)             = CRFSDN_NODE_ERR_ID;
    CRFSDN_NODE_FLAGS(crfsdn_node)          = CRFSDN_NODE_FLAG_UNDEF;
    CRFSDN_NODE_FD(crfsdn_node)             = ERR_FD;
    CRFSDN_NODE_READER_NUM(crfsdn_node)     = (uint16_t)0;
    CRFSDN_NODE_ACCESS_NUM(crfsdn_node)     = (uint16_t)0;

    CRFSDN_NODE_CMUTEX_INIT(crfsdn_node, LOC_CRFSDN_0002);

    return (EC_TRUE);
}

EC_BOOL crfsdn_node_clean(CRFSDN_NODE *crfsdn_node)
{  
    if(ERR_FD != CRFSDN_NODE_FD(crfsdn_node))
    {
        c_file_close(CRFSDN_NODE_FD(crfsdn_node));
        CRFSDN_NODE_FD(crfsdn_node) = ERR_FD;
    }

    CRFSDN_NODE_ID(crfsdn_node)             = CRFSDN_NODE_ERR_ID;
    CRFSDN_NODE_FLAGS(crfsdn_node)          = CRFSDN_NODE_FLAG_UNDEF;
    CRFSDN_NODE_READER_NUM(crfsdn_node)     = (uint16_t)0;
    CRFSDN_NODE_ACCESS_NUM(crfsdn_node)     = (uint16_t)0;  

    CRFSDN_NODE_CMUTEX_CLEAN(crfsdn_node, LOC_CRFSDN_0003);
    
    return (EC_TRUE);
}

EC_BOOL crfsdn_node_free(CRFSDN_NODE *crfsdn_node)
{
    if(NULL_PTR != crfsdn_node)
    {
        crfsdn_node_clean(crfsdn_node);
        free_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CRFSDN_NODE, crfsdn_node, LOC_CRFSDN_0004);
    }
    return (EC_TRUE);
}

int crfsdn_node_cmp(const CRFSDN_NODE *crfsdn_node_1st, const CRFSDN_NODE *crfsdn_node_2nd)
{
    UINT32 node_id_1st;
    UINT32 node_id_2nd;

    node_id_1st = CRFSDN_NODE_ID(crfsdn_node_1st);
    node_id_2nd = CRFSDN_NODE_ID(crfsdn_node_2nd);

    if(node_id_1st > node_id_2nd)
    {
        return (1);
    }

    if(node_id_1st < node_id_2nd)
    {
        return (-1);
    }

    return (0);
}

void crfsdn_node_print(LOG *log, const CRFSDN_NODE *crfsdn_node)
{
    if(NULL_PTR != crfsdn_node)
    {
        sys_log(log, "crfsdn_node %p: disk %u, block %u, fd %d, flag %x, readers %u, access %u\n",
                        crfsdn_node,
                        CRFSDN_NODE_DISK_NO(crfsdn_node),
                        CRFSDN_NODE_BLOCK_NO(crfsdn_node),
                        CRFSDN_NODE_FD(crfsdn_node),
                        CRFSDN_NODE_FLAGS(crfsdn_node),
                        CRFSDN_NODE_READER_NUM(crfsdn_node),
                        CRFSDN_NODE_ACCESS_NUM(crfsdn_node)
                        );
    }

    return;
}

/*for debug only*/
void crfsdn_node_fname_print(LOG *log, const CRFSDN *crfsdn, const UINT32 node_id)
{
    UINT32       path_layout;
    CDSK_SHARD   cdsk_shard;

    path_layout = CRFSDN_PATH_LAYOUT(CRFSDN_NODE_ID_GET_DISK_NO(node_id), CRFSDN_NODE_ID_GET_BLOCK_NO(node_id));

    cdsk_pathlayout_to_shard(path_layout, CRFSDN_DISK_FIXED_NUM, &cdsk_shard);

    sys_log(log, "${ROOT}/dsk%ld/%ld/%ld/%ld/%ld\n",
                CDSK_SHARD_DISK_ID(&cdsk_shard),
                CRFSDN_PATH_LAYOUT_DIR0_NO(CDSK_SHARD_PATH_ID(&cdsk_shard)),
                CRFSDN_PATH_LAYOUT_DIR1_NO(CDSK_SHARD_PATH_ID(&cdsk_shard)),
                CRFSDN_PATH_LAYOUT_DIR2_NO(CDSK_SHARD_PATH_ID(&cdsk_shard)),
                CRFSDN_PATH_LAYOUT_DIR3_NO(CDSK_SHARD_PATH_ID(&cdsk_shard))
                );
    return;
}

static EC_BOOL __crfsdn_node_fname_gen(const CRFSDN *crfsdn, const UINT32 node_id, char *path, const UINT32 max_len)
{
    UINT32       path_layout;
    CDSK_SHARD   cdsk_shard;

    path_layout = CRFSDN_PATH_LAYOUT(CRFSDN_NODE_ID_GET_DISK_NO(node_id), CRFSDN_NODE_ID_GET_BLOCK_NO(node_id));

    cdsk_pathlayout_to_shard(path_layout, CRFSDN_DISK_FIXED_NUM, &cdsk_shard);

    snprintf(path, max_len, "%s/dsk%ld/%ld/%ld/%ld/%ld",
                (char *)CRFSDN_ROOT_DNAME(crfsdn),
                CDSK_SHARD_DISK_ID(&cdsk_shard),
                CRFSDN_PATH_LAYOUT_DIR0_NO(CDSK_SHARD_PATH_ID(&cdsk_shard)),
                CRFSDN_PATH_LAYOUT_DIR1_NO(CDSK_SHARD_PATH_ID(&cdsk_shard)),
                CRFSDN_PATH_LAYOUT_DIR2_NO(CDSK_SHARD_PATH_ID(&cdsk_shard)),
                CRFSDN_PATH_LAYOUT_DIR3_NO(CDSK_SHARD_PATH_ID(&cdsk_shard))
                );
    sys_log(LOGSTDOUT, "[DEBUG] __crfsdn_node_fname_gen: node %u ==> path %s\n", node_id, path);
    return (EC_TRUE);
}

static EC_BOOL __crfsdn_node_dname_gen(const CRFSDN *crfsdn, const UINT32 node_id, char *path, const UINT32 max_len)
{
    UINT32       path_layout;
    CDSK_SHARD   cdsk_shard;

    path_layout = CRFSDN_PATH_LAYOUT(CRFSDN_NODE_ID_GET_DISK_NO(node_id), CRFSDN_NODE_ID_GET_BLOCK_NO(node_id));

    cdsk_pathlayout_to_shard(path_layout, CRFSDN_DISK_FIXED_NUM, &cdsk_shard);

    snprintf(path, max_len, "%s/dsk%ld/%ld/%ld/%ld/",
                (char *)CRFSDN_ROOT_DNAME(crfsdn),
                CDSK_SHARD_DISK_ID(&cdsk_shard),
                CRFSDN_PATH_LAYOUT_DIR0_NO(CDSK_SHARD_PATH_ID(&cdsk_shard)),
                CRFSDN_PATH_LAYOUT_DIR1_NO(CDSK_SHARD_PATH_ID(&cdsk_shard)),
                CRFSDN_PATH_LAYOUT_DIR2_NO(CDSK_SHARD_PATH_ID(&cdsk_shard))
                );
    return (EC_TRUE);
}

CRFSDN_NODE *crfsdn_node_fetch(const CRFSDN *crfsdn, const UINT32 node_id)
{
    CRFSDN_NODE crfsdn_node;
    CRB_NODE   *crb_node;

    CRFSDN_NODE_ID(&crfsdn_node) = node_id;
    
    crb_node = crb_tree_search_data(CRFSDN_OPEN_NODES(crfsdn), (void *)&crfsdn_node);
    if(NULL_PTR == crb_node)
    {
        return (NULL_PTR);
    }

    return ((CRFSDN_NODE *)CRB_NODE_DATA(crb_node));
}

EC_BOOL crfsdn_node_delete(CRFSDN *crfsdn, const UINT32 node_id)
{
    CRFSDN_NODE crfsdn_node;

    CRFSDN_NODE_ID(&crfsdn_node) = node_id;
    return crb_tree_delete_data(CRFSDN_OPEN_NODES(crfsdn), (void *)&crfsdn_node); 
}

CRFSDN_NODE *crfsdn_node_create(CRFSDN *crfsdn, const UINT32 node_id)
{
    CRFSDN_NODE *crfsdn_node;
    CRB_NODE   *crb_node;

    char path[ CRFSDN_NODE_NAME_MAX_SIZE ];

    crfsdn_node = CRFSDN_OPEN_NODE(crfsdn, node_id);
    if(NULL_PTR != crfsdn_node)
    {
        return (crfsdn_node);
    }

    /*create dir if needed*/
    __crfsdn_node_dname_gen(crfsdn, node_id, path, CRFSDN_NODE_NAME_MAX_SIZE);
    if(EC_FALSE == c_dir_create(path))
    {
        sys_log(LOGSTDOUT, "error:crfsdn_node_create: create dir %s failed\n", path);
        return (NULL_PTR);
    }

    /*if file exist, do nothing*/
    __crfsdn_node_fname_gen(crfsdn, node_id, path, CRFSDN_NODE_NAME_MAX_SIZE);
    if(EC_TRUE == c_file_access(path, F_OK))
    {
        sys_log(LOGSTDOUT, "warn:crfsdn_node_create: node file %s already exist\n", path);
        return (NULL_PTR);
    }

    crfsdn_node = crfsdn_node_new();
    if(NULL_PTR == crfsdn_node)
    {
        sys_log(LOGSTDOUT, "warn:crfsdn_node_create: new crfsdn_node failed\n");
        return (NULL_PTR);
    }
    CRFSDN_NODE_ID(crfsdn_node) = node_id;

    /*creat file*/
    CRFSDN_NODE_FD(crfsdn_node) = c_file_open(path, O_RDWR | O_CREAT, 0666);
    if(ERR_FD == CRFSDN_NODE_FD(crfsdn_node))
    {
        sys_log(LOGSTDOUT, "error:crfsdn_node_create: open node file %s failed\n", path);
        crfsdn_node_free(crfsdn_node);
        return (NULL_PTR);
    }
    sys_log(LOGSTDOUT, "[DEBUG] crfsdn_node_create: create file %s done\n", path);
    
    /*optimize*/
    if(EC_FALSE == c_file_truncate(CRFSDN_NODE_FD(crfsdn_node), CPGB_CACHE_MAX_BYTE_SIZE))
    {
        sys_log(LOGSTDOUT, "error:crfsdn_node_create: truncate file %s failed\n", path);
        crfsdn_node_free(crfsdn_node);
        return (NULL_PTR);
    }
    sys_log(LOGSTDOUT, "[DEBUG] crfsdn_node_create: truncate file %s to %ld bytes done\n", path, CPGB_CACHE_MAX_BYTE_SIZE);

    crb_node = crb_tree_insert_data(CRFSDN_OPEN_NODES(crfsdn), (void *)crfsdn_node);
    if(NULL_PTR == crb_node)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_node_create: insert new crfsdn_node into cached nodes failed\n");
        crfsdn_node_free(crfsdn_node);
        return (NULL_PTR);
    }

    if(CRB_NODE_DATA(crb_node) != (void *)crfsdn_node)
    {
        sys_log(LOGSTDOUT, "warn:crfsdn_node_create: inserted but crfsdn_node is not the newest one\n");
        crfsdn_node_free(crfsdn_node);
        return ((CRFSDN_NODE *)CRB_NODE_DATA(crb_node));
    }

    return (crfsdn_node);
}

CRFSDN_NODE *crfsdn_node_open(CRFSDN *crfsdn, const UINT32 node_id, const UINT32 open_flags)
{
    CRFSDN_NODE *crfsdn_node;
    CRB_NODE    *crb_node;
    char path[ CRFSDN_NODE_NAME_MAX_SIZE ];

    CRFSDN_CMUTEX_LOCK(crfsdn, LOC_CRFSDN_0005);
    crfsdn_node = CRFSDN_OPEN_NODE(crfsdn, node_id);
    if(NULL_PTR != crfsdn_node)
    {
        CRFSDN_CMUTEX_UNLOCK(crfsdn, LOC_CRFSDN_0006);
        return (crfsdn_node);
    }

    __crfsdn_node_fname_gen(crfsdn, node_id, path, CRFSDN_NODE_NAME_MAX_SIZE);
    sys_log(LOGSTDOUT, "[DEBUG] crfsdn_node_open: path is %s\n", path);
    
    /*when node file not exit, then create it and return*/
    if(EC_FALSE == c_file_access(path, F_OK))
    {
        if(open_flags & CRFSDN_NODE_O_CREATE)
        {
            sys_log(LOGSTDOUT, "warn:crfsdn_node_open: node file %s not exist, try to create it\n", path);
            crfsdn_node = crfsdn_node_create(crfsdn, node_id);
            if(NULL_PTR != crfsdn_node)
            {
                CRFSDN_CMUTEX_UNLOCK(crfsdn, LOC_CRFSDN_0007);
                return (crfsdn_node);
            }
        }

        CRFSDN_CMUTEX_UNLOCK(crfsdn, LOC_CRFSDN_0008);
        sys_log(LOGSTDOUT, "warn:crfsdn_node_open: node file %s not exist\n", path);        
        return (NULL_PTR);
    }

    crfsdn_node = crfsdn_node_new();
    if(NULL_PTR == crfsdn_node)
    {
        CRFSDN_CMUTEX_UNLOCK(crfsdn, LOC_CRFSDN_0009);
        sys_log(LOGSTDOUT, "warn:crfsdn_node_open: new crfsdn_node failed\n");
        return (NULL_PTR);
    }
    CRFSDN_NODE_ID(crfsdn_node) = node_id;
    
    /*when node file exit, then open it*/
    CRFSDN_NODE_FD(crfsdn_node) = c_file_open(path, O_RDWR, 0666);
    if(ERR_FD == CRFSDN_NODE_FD(crfsdn_node))
    {
        CRFSDN_CMUTEX_UNLOCK(crfsdn, LOC_CRFSDN_0010);
        sys_log(LOGSTDOUT, "error:crfsdn_node_open: open node file %s failed\n", path);
        crfsdn_node_free(crfsdn_node);
        return (NULL_PTR);
    }

    sys_log(LOGSTDOUT, "[DEBUG] crfsdn_node_open: insert node %u with path %s to cached nodes(rbtree)\n", node_id, path);
    crb_node = crb_tree_insert_data(CRFSDN_OPEN_NODES(crfsdn), (void *)crfsdn_node);
    if(NULL_PTR == crb_node)
    {
        CRFSDN_CMUTEX_UNLOCK(crfsdn, LOC_CRFSDN_0011);
        sys_log(LOGSTDOUT, "error:crfsdn_node_open: insert new crfsdn_node into cached nodes failed\n");
        crfsdn_node_free(crfsdn_node);
        return (NULL_PTR);
    }

    if(CRB_NODE_DATA(crb_node) != (void *)crfsdn_node)
    {
        CRFSDN_CMUTEX_UNLOCK(crfsdn, LOC_CRFSDN_0012);
        sys_log(LOGSTDOUT, "warn:crfsdn_node_open: inserted but crfsdn_node is not the newest one\n");
        crfsdn_node_free(crfsdn_node);
        return ((CRFSDN_NODE *)CRB_NODE_DATA(crb_node));
    }
    CRFSDN_CMUTEX_UNLOCK(crfsdn, LOC_CRFSDN_0013);
    return (crfsdn_node);
}

EC_BOOL crfsdn_node_unlink(CRFSDN *crfsdn, const UINT32 node_id)
{
    char path[ CRFSDN_NODE_NAME_MAX_SIZE ];

    __crfsdn_node_fname_gen(crfsdn, node_id, path, CRFSDN_NODE_NAME_MAX_SIZE);
    if(EC_FALSE == c_file_access(path, F_OK))
    {
        sys_log(LOGSTDOUT, "error:crfsdn_node_unlink: node file %s not exist\n", path);
        return (EC_FALSE);
    }

    if( EC_FALSE == c_file_unlink(path))
    {
        sys_log(LOGSTDOUT, "error:crfsdn_node_unlink: unlink node file %s failed\n", path);
        return (EC_FALSE);
    }

    if(EC_FALSE == crfsdn_node_delete(crfsdn, node_id))
    {
        sys_log(LOGSTDOUT, "error:crfsdn_node_unlink: delete node from cache failed\n");
        return (EC_FALSE);
    }

    sys_log(LOGSTDOUT, "[DEBUG] crfsdn_node_unlink: unlink node file %s successfully\n", path);
    return (EC_TRUE);
}


EC_BOOL crfsdn_node_write(CRFSDN *crfsdn, const UINT32 node_id, const UINT32 data_max_len, const UINT8 *data_buff, UINT32 *offset)
{
    CRFSDN_NODE *crfsdn_node;

    crfsdn_node = crfsdn_node_open(crfsdn, node_id, CRFSDN_NODE_O_CREATE | CRFSDN_NODE_O_RDWR);
    if(NULL_PTR == crfsdn_node)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_node_write: open node %ld failed\n", node_id);
        return (EC_FALSE);
    }

    CRFSDN_NODE_CMUTEX_LOCK(crfsdn_node, LOC_CRFSDN_0014);
    if(EC_FALSE == c_file_flush(CRFSDN_NODE_FD(crfsdn_node), offset, data_max_len, data_buff))
    {
        CRFSDN_NODE_CMUTEX_UNLOCK(crfsdn_node, LOC_CRFSDN_0015);
        sys_log(LOGSTDOUT, "error:crfsdn_node_write: flush %ld bytes to node %ld at offset %ld failed\n",
                            data_max_len, node_id, (*offset));
        return (EC_FALSE);
    }
    CRFSDN_NODE_CMUTEX_UNLOCK(crfsdn_node, LOC_CRFSDN_0016);
    return (EC_TRUE);
}

EC_BOOL crfsdn_node_read(CRFSDN *crfsdn, const UINT32 node_id, const UINT32 data_max_len, UINT8 *data_buff, UINT32 *offset)
{    
    CRFSDN_NODE *crfsdn_node;

    crfsdn_node = crfsdn_node_open(crfsdn, node_id, /*CRFSDN_NODE_O_CREATE | */CRFSDN_NODE_O_RDWR);
    if(NULL_PTR == crfsdn_node)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_node_read: open node %ld failed\n", node_id);
        return (EC_FALSE);
    }

    CRFSDN_NODE_CMUTEX_LOCK(crfsdn_node, LOC_CRFSDN_0017);
    if(EC_FALSE == c_file_load(CRFSDN_NODE_FD(crfsdn_node), offset, data_max_len, data_buff))
    {
        CRFSDN_NODE_CMUTEX_UNLOCK(crfsdn_node, LOC_CRFSDN_0018);
        sys_log(LOGSTDOUT, "error:crfsdn_node_read: load %ld bytes from node %ld at offset %ld failed\n",
                            data_max_len, node_id, (*offset));        
        return (EC_FALSE);
    }
    CRFSDN_NODE_CMUTEX_UNLOCK(crfsdn_node, LOC_CRFSDN_0019);
    return (EC_TRUE);
}

CRFSDN_CACHE_NODE *crfsdn_cache_node_new()
{
    CRFSDN_CACHE_NODE *crfsdn_cache_node;

    alloc_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CRFSDN_CACHE_NODE, &crfsdn_cache_node, LOC_CRFSDN_0020);
    if(NULL_PTR != crfsdn_cache_node)
    {
        crfsdn_cache_node_init(crfsdn_cache_node);
    }
    return (crfsdn_cache_node);
}

EC_BOOL crfsdn_cache_node_init(CRFSDN_CACHE_NODE *crfsdn_cache_node)
{
    CRFSDN_CACHE_NODE_DISK_NO(crfsdn_cache_node)   = CPGRB_ERR_POS;
    CRFSDN_CACHE_NODE_BLOCK_NO(crfsdn_cache_node)  = CPGRB_ERR_POS;
    CRFSDN_CACHE_NODE_PAGE_NO(crfsdn_cache_node)   = CPGRB_ERR_POS;
    CRFSDN_CACHE_NODE_DATA_SIZE(crfsdn_cache_node) = 0;
    CRFSDN_CACHE_NODE_DATA_BUFF(crfsdn_cache_node) = NULL_PTR;

    return (EC_TRUE);
}

EC_BOOL crfsdn_cache_node_clean(CRFSDN_CACHE_NODE *crfsdn_cache_node)
{
    CRFSDN_CACHE_NODE_DISK_NO(crfsdn_cache_node)   = CPGRB_ERR_POS;
    CRFSDN_CACHE_NODE_BLOCK_NO(crfsdn_cache_node)  = CPGRB_ERR_POS;
    CRFSDN_CACHE_NODE_PAGE_NO(crfsdn_cache_node)   = CPGRB_ERR_POS;

    if(NULL_PTR != CRFSDN_CACHE_NODE_DATA_BUFF(crfsdn_cache_node))
    {
        safe_free(CRFSDN_CACHE_NODE_DATA_BUFF(crfsdn_cache_node), LOC_CRFSDN_0021);
        CRFSDN_CACHE_NODE_DATA_BUFF(crfsdn_cache_node) = NULL_PTR;
    }
    CRFSDN_CACHE_NODE_DATA_SIZE(crfsdn_cache_node) = 0;

    return (EC_TRUE);
}

EC_BOOL crfsdn_cache_node_free(CRFSDN_CACHE_NODE *crfsdn_cache_node)
{
    if(NULL_PTR != crfsdn_cache_node)
    {
        crfsdn_cache_node_clean(crfsdn_cache_node);
        free_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CRFSDN_CACHE_NODE, crfsdn_cache_node, LOC_CRFSDN_0022);
    }
    return (EC_TRUE);
}

EC_BOOL crfsdn_cache_node_init_0(const UINT32 md_id, CRFSDN_CACHE_NODE *crfsdn_cache_node)
{
    return crfsdn_cache_node_init(crfsdn_cache_node);
}

EC_BOOL crfsdn_cache_node_clean_0(const UINT32 md_id, CRFSDN_CACHE_NODE *crfsdn_cache_node)
{
    return crfsdn_cache_node_clean(crfsdn_cache_node);
}

EC_BOOL crfsdn_cache_node_free_0(const UINT32 md_id, CRFSDN_CACHE_NODE *crfsdn_cache_node)
{
    return crfsdn_cache_node_free(crfsdn_cache_node);
}

EC_BOOL crfsdn_has_no_cache_node(const CRFSDN *crfsdn)
{
    return clist_is_empty(CRFSDN_CACHED_NODES(crfsdn));
}

EC_BOOL crfsdn_push_cache_node(CRFSDN *crfsdn, CRFSDN_CACHE_NODE *crfsdn_cache_node)
{
    clist_push_back(CRFSDN_CACHED_NODES(crfsdn), (void *)crfsdn_cache_node);
    return (EC_TRUE);
}

CRFSDN_CACHE_NODE *crfsdn_pop_cache_node(CRFSDN *crfsdn)
{
    CRFSDN_CACHE_NODE *crfsdn_cache_node;
    crfsdn_cache_node = (CRFSDN_CACHE_NODE *)clist_pop_front(CRFSDN_CACHED_NODES(crfsdn));
    return (crfsdn_cache_node);
}

EC_BOOL crfsdn_flush_cache_node(CRFSDN *crfsdn, CRFSDN_CACHE_NODE *crfsdn_cache_node)
{
    UINT32 offset;
    uint16_t        disk_no;
    uint16_t        block_no;
    uint16_t        page_no;
    uint32_t        size;
    UINT32          data_size;
    uint8_t        *data_buff;

    disk_no   = CRFSDN_CACHE_NODE_DISK_NO(crfsdn_cache_node);
    block_no  = CRFSDN_CACHE_NODE_BLOCK_NO(crfsdn_cache_node);
    page_no   = CRFSDN_CACHE_NODE_PAGE_NO(crfsdn_cache_node);
    data_size = CRFSDN_CACHE_NODE_DATA_SIZE(crfsdn_cache_node);
    data_buff = CRFSDN_CACHE_NODE_DATA_BUFF(crfsdn_cache_node);

    size = (uint32_t)data_size;
    offset  = (((UINT32)(page_no)) << (CPGB_PAGE_4K_BIT_SIZE));
    
    if(EC_FALSE == crfsdn_write_o(crfsdn, data_size, data_buff, disk_no, block_no, &offset))
    {
        sys_log(LOGSTDOUT, "error:crfsdn_flush_cache_node: write %u bytes to disk %u block %u page %u failed\n", 
                            data_size, (disk_no), (block_no), (page_no));
                            
        cpgv_free_space(CRFSDN_CPGV(crfsdn), disk_no, block_no, page_no, size);
        return (EC_FALSE);
    }    
    sys_log(LOGSTDOUT, "[DEBUG] crfsdn_flush_cache_node: write %u bytes to disk %u block %u page %u done\n", 
                        data_size, (disk_no), (block_no), (page_no));
    return (EC_TRUE);
}

void crfsdn_flush_cache_nodes(CRFSDN **crfsdn, EC_BOOL *terminate_flag)
{
    while(EC_FALSE == (*terminate_flag) && NULL_PTR == (*crfsdn))
    {
        c_usleep(200);
    }

    sys_log(LOGSTDOUT, "[DEBUG] crfsdn_flush_cache_nodes: [1] terminate_flag %ld, crfsdn %p\n", (*terminate_flag), (*crfsdn));

    while(EC_FALSE == (*terminate_flag) && NULL_PTR != (*crfsdn))
    {
        CRFSDN_CACHE_NODE *crfsdn_cache_node;
        
        if(EC_TRUE == crfsdn_has_no_cache_node(*crfsdn))
        {
            c_usleep(200);
        }

        crfsdn_cache_node = crfsdn_pop_cache_node(*crfsdn);
        if(NULL_PTR != crfsdn_cache_node)
        {
            sys_log(LOGSTDOUT, "[DEBUG] crfsdn_flush_cache_nodes: [2] terminate_flag %ld, crfsdn %p, crfsdn_cache_node %p\n", 
                                (*terminate_flag), (*crfsdn), crfsdn_cache_node);
            crfsdn_flush_cache_node(*crfsdn, crfsdn_cache_node);
            crfsdn_cache_node_free(crfsdn_cache_node);
        }
    }

    sys_log(LOGSTDOUT, "[DEBUG] crfsdn_flush_cache_nodes: [3] terminate_flag %ld, crfsdn %p\n", (*terminate_flag), (*crfsdn));

    return;
}

static char * __crfsdn_vol_fname_gen(const char *root_dname)
{
    const static char *vol_basename = (const char *)"/vol/vol.dat";
    const char *field[ 2 ];
    char       *vol_fname;  

    field[ 0 ] = root_dname;
    field[ 1 ] = vol_basename;

    vol_fname = c_str_join("/", field, 2);/*${root_dname}/${vol_basename}*/
    if(NULL_PTR == vol_fname)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_vol_fname_gen: make vol_fname %s/%s failed\n", root_dname, vol_basename);
        return (NULL_PTR);
    }

    return (vol_fname);
}

static uint16_t __crfsdn_count_disk_num_from_disk_space(const uint16_t max_gb_num_of_disk_space)
{
#if (CPGD_TEST_SCENARIO_001T_DISK == CPGD_DEBUG_CHOICE)
    uint16_t max_tb_num_of_disk_space;
    uint16_t disk_num;

    max_tb_num_of_disk_space = (max_gb_num_of_disk_space + 1024 - 1) / 1024;
    disk_num = max_tb_num_of_disk_space; /*one disk = 1 TB*/
#endif/*(CPGD_TEST_SCENARIO_001T_DISK == CPGD_DEBUG_CHOICE)*/

#if (CPGD_TEST_SCENARIO_256M_DISK == CPGD_DEBUG_CHOICE)
    uint16_t max_mb_num_of_disk_space;
    uint16_t disk_num;

    sys_log(LOGSTDOUT, "[DEBUG] __crfsdn_count_disk_num_from_disk_space: ### set 1 disk = %u MB for debug purpose \n", CPGD_DEBUG_MB_PER_DISK);
    max_mb_num_of_disk_space = (max_gb_num_of_disk_space) * (1024 / CPGD_DEBUG_MB_PER_DISK);
    disk_num = max_mb_num_of_disk_space;
    
#endif/*(CPGD_TEST_SCENARIO_256M_DISK == CPGD_DEBUG_CHOICE)*/

#if (CPGD_TEST_SCENARIO_512M_DISK == CPGD_DEBUG_CHOICE)
    uint16_t max_mb_num_of_disk_space;
    uint16_t disk_num;

    sys_log(LOGSTDOUT, "[DEBUG] __crfsdn_count_disk_num_from_disk_space: ### set 1 disk = %u MB for debug purpose \n", CPGD_DEBUG_MB_PER_DISK);
    max_mb_num_of_disk_space = (max_gb_num_of_disk_space) * (1024 / CPGD_DEBUG_MB_PER_DISK);
    disk_num = max_mb_num_of_disk_space;
    
#endif/*(CPGD_TEST_SCENARIO_512M_DISK == CPGD_DEBUG_CHOICE)*/

#if (CPGD_TEST_SCENARIO_032G_DISK == CPGD_DEBUG_CHOICE)
    uint16_t disk_num;

    sys_log(LOGSTDOUT, "[DEBUG] __crfsdn_count_disk_num_from_disk_space: ### set 1 disk = %u GB for debug purpose \n", CPGD_DEBUG_GB_PER_DISK);
    disk_num = (max_gb_num_of_disk_space + CPGD_DEBUG_GB_PER_DISK - 1) / CPGD_DEBUG_GB_PER_DISK;
    
#endif/*(CPGD_TEST_SCENARIO_032G_DISK == CPGD_DEBUG_CHOICE)*/

    return (disk_num);
}

CRFSDN *crfsdn_create(const char *root_dname)
{
    CRFSDN *crfsdn;
    uint8_t *vol_fname;
    
    crfsdn = crfsdn_new();
    if(NULL_PTR == crfsdn)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_create: new crfsdn failed\n");
        return (NULL_PTR);
    }

    CRFSDN_ROOT_DNAME(crfsdn) = (uint8_t *)c_str_dup(root_dname);
    if(NULL_PTR == CRFSDN_ROOT_DNAME(crfsdn))
    {
        sys_log(LOGSTDOUT, "error:crfsdn_create: dup root_dname %s failed\n", root_dname);
        crfsdn_free(crfsdn);
        return (NULL_PTR);
    }

    vol_fname = (uint8_t *)__crfsdn_vol_fname_gen((char *)CRFSDN_ROOT_DNAME(crfsdn));
    if(NULL_PTR == vol_fname)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_create: make vol_fname from root_dname %s failed\n", root_dname);
        crfsdn_free(crfsdn);
        return (NULL_PTR);
    }

    CRFSDN_CPGV(crfsdn) = cpgv_new((uint8_t *)vol_fname);
    if(NULL_PTR == CRFSDN_CPGV(crfsdn))
    {
        sys_log(LOGSTDOUT, "error:crfsdn_create: new vol %s failed\n", vol_fname);
        crfsdn_free(crfsdn);
        safe_free(vol_fname, LOC_CRFSDN_0023);
        return (NULL_PTR);
    }

    sys_log(LOGSTDOUT, "[DEBUG] crfsdn_create: vol %s was created\n", vol_fname);
    safe_free(vol_fname, LOC_CRFSDN_0024);

    if(EC_FALSE == crfsdn_flush(crfsdn))/*xxx*/
    {
        sys_log(LOGSTDOUT, "error:crfsdn_create:flush dn failed\n");
        crfsdn_free(crfsdn);
        return (NULL_PTR);
    }

    return (crfsdn);
}

EC_BOOL crfsdn_add_disk(CRFSDN *crfsdn, const uint16_t disk_no)
{
    if(EC_FALSE == cpgv_add_disk(CRFSDN_CPGV(crfsdn), disk_no))
    {
        sys_log(LOGSTDOUT, "error:crfsdn_add_disk: cpgv add disk %u failed\n", disk_no);
        return (EC_FALSE);
    }
    
    if(EC_FALSE == crfsdn_flush(crfsdn))/*xxx*/
    {
        sys_log(LOGSTDOUT, "error:crfsdn_add_disk: flush dn failed after add disk %u \n", disk_no);
        cpgv_del_disk(CRFSDN_CPGV(crfsdn), disk_no);
        return (EC_FALSE);
    }    
    return (EC_TRUE);
}

EC_BOOL crfsdn_del_disk(CRFSDN *crfsdn, const uint16_t disk_no)
{
    if(EC_FALSE == cpgv_del_disk(CRFSDN_CPGV(crfsdn), disk_no))
    {
        sys_log(LOGSTDOUT, "error:crfsdn_del_disk: cpgv del disk %u failed\n", disk_no);
        return (EC_FALSE);
    }
    
    if(EC_FALSE == crfsdn_flush(crfsdn))/*xxx*/
    {
        sys_log(LOGSTDOUT, "error:crfsdn_del_disk: flush dn failed after del disk %u \n", disk_no);
        return (EC_FALSE);
    }    
    return (EC_TRUE);
}

EC_BOOL crfsdn_mount_disk(CRFSDN *crfsdn, const uint16_t disk_no)
{
    if(EC_FALSE == cpgv_mount_disk(CRFSDN_CPGV(crfsdn), disk_no))
    {
        sys_log(LOGSTDOUT, "error:crfsdn_mount_disk: cpgv mount disk %u failed\n", disk_no);
        return (EC_FALSE);
    }
    
    if(EC_FALSE == crfsdn_flush(crfsdn))/*xxx*/
    {
        sys_log(LOGSTDOUT, "error:crfsdn_mount_disk: flush dn failed after mount disk %u \n", disk_no);
        return (EC_FALSE);
    }    
    return (EC_TRUE);
}

EC_BOOL crfsdn_umount_disk(CRFSDN *crfsdn, const uint16_t disk_no)
{
    if(EC_FALSE == cpgv_umount_disk(CRFSDN_CPGV(crfsdn), disk_no))
    {
        sys_log(LOGSTDOUT, "error:crfsdn_umount_disk: cpgv umount disk %u failed\n", disk_no);
        return (EC_FALSE);
    }
    
    if(EC_FALSE == crfsdn_flush(crfsdn))/*xxx*/
    {
        sys_log(LOGSTDOUT, "error:crfsdn_umount_disk: flush dn failed after umount disk %u \n", disk_no);
        return (EC_FALSE);
    }    
    return (EC_TRUE);
}


CRFSDN *crfsdn_new()
{
    CRFSDN *crfsdn;  

    alloc_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CRFSDN, &crfsdn, LOC_CRFSDN_0025);
    if(NULL_PTR != crfsdn)
    {
        crfsdn_init(crfsdn);
        return (crfsdn);
    }
    return (crfsdn);
}

EC_BOOL crfsdn_init(CRFSDN *crfsdn)
{
    CRFSDN_CRWLOCK_INIT(crfsdn, LOC_CRFSDN_0026);
    CRFSDN_CMUTEX_INIT(crfsdn, LOC_CRFSDN_0027);
    
    crb_tree_init(CRFSDN_OPEN_NODES(crfsdn), 
                  (CRB_DATA_CMP  )crfsdn_node_cmp,  
                  (CRB_DATA_FREE )crfsdn_node_free, 
                  (CRB_DATA_PRINT)crfsdn_node_print);
    clist_init(CRFSDN_CACHED_NODES(crfsdn), MM_CRFSDN_CACHE_NODE, LOC_CRFSDN_0028);

    CRFSDN_ROOT_DNAME(crfsdn)  = NULL_PTR;
    CRFSDN_CPGV(crfsdn)        = NULL_PTR;
    
    return (EC_TRUE);
}

EC_BOOL crfsdn_clean(CRFSDN *crfsdn)
{
    CRFSDN_CRWLOCK_CLEAN(crfsdn, LOC_CRFSDN_0029);
    CRFSDN_CMUTEX_CLEAN(crfsdn, LOC_CRFSDN_0030);
    
    crb_tree_clean(CRFSDN_OPEN_NODES(crfsdn));

    if(NULL_PTR != CRFSDN_ROOT_DNAME(crfsdn))
    {
        safe_free(CRFSDN_ROOT_DNAME(crfsdn), LOC_CRFSDN_0031);
        CRFSDN_ROOT_DNAME(crfsdn) = NULL_PTR;
    }

    if(NULL_PTR != CRFSDN_CPGV(crfsdn))
    {
        cpgv_close(CRFSDN_CPGV(crfsdn));
        CRFSDN_CPGV(crfsdn) = NULL_PTR;
    }

    return (EC_TRUE);
}

EC_BOOL crfsdn_free(CRFSDN *crfsdn)
{
    if(NULL_PTR != crfsdn)
    {
        crfsdn_clean(crfsdn);
        free_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CRFSDN, crfsdn, LOC_CRFSDN_0032);
    }
    return (EC_TRUE);
}

void crfsdn_print(LOG *log, const CRFSDN *crfsdn)
{
    if(NULL_PTR != crfsdn)
    {       
        sys_log(log, "crfsdn %p: root dname: %s\n", crfsdn, (char *)CRFSDN_ROOT_DNAME(crfsdn));

        cpgv_print(log, CRFSDN_CPGV(crfsdn));
        if(0)
        {
            sys_log(log, "crfsdn %p: cached nodes: \n", crfsdn);
            crb_tree_print(log, CRFSDN_OPEN_NODES(crfsdn));
        }     
    }
    return;
}

EC_BOOL crfsdn_is_full(CRFSDN *crfsdn)
{
    return cpgv_is_full(CRFSDN_CPGV(crfsdn));
}

EC_BOOL crfsdn_flush(CRFSDN *crfsdn)
{
    if(NULL_PTR == crfsdn)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_flush: crfsdn is null\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == cpgv_sync(CRFSDN_CPGV(crfsdn)))
    {
        sys_log(LOGSTDOUT, "error:crfsdn_flush: sync cpgv failed\n");
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

EC_BOOL crfsdn_load(CRFSDN *crfsdn, const char *root_dname)
{
    uint8_t *vol_fname;

    if(NULL_PTR == crfsdn)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_load: crfsdn is null\n");
        return (EC_FALSE);
    }

    if(NULL_PTR != CRFSDN_CPGV(crfsdn))
    {
        sys_log(LOGSTDOUT, "error:crfsdn_load: CRFSDN_CPGV is not null\n");
        return (EC_FALSE);
    }

    CRFSDN_ROOT_DNAME(crfsdn) = (uint8_t *)c_str_dup(root_dname);
    if(NULL_PTR == CRFSDN_ROOT_DNAME(crfsdn))
    {
        sys_log(LOGSTDOUT, "error:crfsdn_load: dup root_dname %s failed\n", root_dname);
        return (EC_FALSE);
    }     

    vol_fname = (uint8_t *)__crfsdn_vol_fname_gen((char *)CRFSDN_ROOT_DNAME(crfsdn));
    if(NULL_PTR == vol_fname)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_load: make vol_fname from root_dname %s failed\n", (char *)CRFSDN_ROOT_DNAME(crfsdn));
        return (EC_FALSE);
    }    

    CRFSDN_CPGV(crfsdn) = cpgv_open(vol_fname);
    if(NULL_PTR == CRFSDN_CPGV(crfsdn))
    {
        sys_log(LOGSTDOUT, "error:crfsdn_load: load/open vol from %s failed\n", (char *)vol_fname);
        safe_free(vol_fname, LOC_CRFSDN_0033);
        return (EC_FALSE);
    }
    
    sys_log(LOGSTDOUT, "[DEBUG] crfsdn_load: load/open vol from %s done\n", (char *)vol_fname);
    safe_free(vol_fname, LOC_CRFSDN_0034);

    return (EC_TRUE);
}

EC_BOOL crfsdn_exist(const char *root_dname)
{
    char *vol_fname;
    
    vol_fname = __crfsdn_vol_fname_gen(root_dname);
    if(NULL_PTR == vol_fname)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_exist: make vol_fname from root_dname %s failed\n", root_dname);
        return (EC_FALSE);
    }  

    if(EC_FALSE == c_file_access(vol_fname, F_OK))
    {
        safe_free(vol_fname, LOC_CRFSDN_0035);
        return (EC_FALSE);
    }    

    safe_free(vol_fname, LOC_CRFSDN_0036);
    return (EC_TRUE);
}

CRFSDN *crfsdn_open(const char *root_dname)
{
    CRFSDN *crfsdn;

    if(NULL_PTR == root_dname)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_open: root dir is null\n");
        return (NULL_PTR);
    }

    if(EC_FALSE == c_file_access(root_dname, F_OK))
    {
        sys_log(LOGSTDOUT, "error:crfsdn_open: root dir %s not exist\n", root_dname);
        return (NULL_PTR);
    }

    crfsdn = crfsdn_new();
    if(NULL_PTR == crfsdn)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_open: new crfsdn with root dir %s failed\n", root_dname);
        return (NULL_PTR);
    }
    
    if(EC_FALSE == crfsdn_load(crfsdn, root_dname))
    {
        sys_log(LOGSTDOUT, "error:crfsdn_open: load crfsdn from root dir %s failed\n", root_dname);
        crfsdn_free(crfsdn);
        return (NULL_PTR);
    }
    sys_log(LOGSTDOUT, "[DEBUG] crfsdn_open: load crfsdn from root dir %s done\n", root_dname);
    
    return (crfsdn);
}

EC_BOOL crfsdn_close(CRFSDN *crfsdn)
{
    if(NULL_PTR != crfsdn)
    {
        CRFSDN_CRWLOCK_WRLOCK(crfsdn, LOC_CRFSDN_0037);
        crfsdn_flush(crfsdn);
        CRFSDN_CRWLOCK_UNLOCK(crfsdn, LOC_CRFSDN_0038);
        crfsdn_free(crfsdn);
    }
    return (EC_TRUE);
}

/*random access for reading, the offset is for the whole 64M page-block */
EC_BOOL crfsdn_read_o(CRFSDN *crfsdn, const uint16_t disk_no, const uint16_t block_no, const UINT32 offset, const UINT32 data_max_len, UINT8 *data_buff, UINT32 *data_len)
{
    UINT32 node_id;
    UINT32 offset_t;

    if(NULL_PTR == crfsdn)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_read_o: crfsdn is null\n");
        return (EC_FALSE);
    }

    if(NULL_PTR == data_buff)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_read_o: data_buff is null\n");
        return (EC_FALSE);
    } 

    if(CPGB_CACHE_MAX_BYTE_SIZE < data_max_len)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_read_o: data max len %ld overflow\n", data_max_len);
        return (EC_FALSE);
    }   

    if(CPGB_CACHE_MAX_BYTE_SIZE <= offset)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_read_o: offset %ld overflow\n", offset);
        return (EC_FALSE);
    }   

    if(CPGB_CACHE_MAX_BYTE_SIZE < offset + data_max_len)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_read_o: offset %ld + data_max_len %ld = %ld overflow\n", 
                            offset, data_max_len, offset + data_max_len);
        return (EC_FALSE);
    }     

    node_id = CRFSDN_NODE_ID_MAKE(disk_no, block_no);
    offset_t = offset;
    sys_log(LOGSTDOUT, "[DEBUG] crfsdn_read_o: disk %u, block %u  ==> node %u\n", disk_no, block_no, node_id);
    if(EC_FALSE == crfsdn_node_read(crfsdn, node_id, data_max_len, data_buff, &offset_t))
    {
        sys_log(LOGSTDOUT, "error:crfsdn_read_o: read %u bytes at offset %u from node %u failed\n", 
                           data_max_len, offset, node_id);
        return (EC_FALSE);
    }

    if(NULL_PTR != data_len)
    {
        (*data_len) = offset_t - offset;
    }

    return (EC_TRUE);
}

/*random access for writting */
/*offset: IN/OUT, the offset is for the whole 64M page-block*/
EC_BOOL crfsdn_write_o(CRFSDN *crfsdn, const UINT32 data_max_len, const UINT8 *data_buff, const uint16_t disk_no, const uint16_t block_no, UINT32 *offset)
{
    UINT32 node_id;
    UINT32 offset_t;

    if(NULL_PTR == crfsdn)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_write_o: crfsdn is null\n");
        return (EC_FALSE);
    }

    if(NULL_PTR == data_buff)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_write_o: data_buff is null\n");
        return (EC_FALSE);
    }    

    if(CPGB_CACHE_MAX_BYTE_SIZE < data_max_len)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_write_o: data max len %u overflow\n", data_max_len);
        return (EC_FALSE);
    }

    node_id  = CRFSDN_NODE_ID_MAKE(disk_no, block_no);
    offset_t = (*offset);

    if(EC_FALSE == crfsdn_node_write(crfsdn, node_id, data_max_len, data_buff, offset))
    {
        sys_log(LOGSTDOUT, "error:crfsdn_write_o: write %u bytes to disk %u block %u offset %u failed\n", 
                            data_max_len, disk_no, block_no, offset_t);
                            
        return (EC_FALSE);
    }

    sys_log(LOGSTDOUT, "[DEBUG] crfsdn_write_o: write %u bytes to disk %u block %u offset %u done\n", 
                        data_max_len, disk_no, block_no, offset_t);

    return (EC_TRUE);
}

EC_BOOL crfsdn_read_b(CRFSDN *crfsdn, const uint16_t disk_no, const uint16_t block_no, const uint16_t page_no, const UINT32 data_max_len, UINT8 *data_buff, UINT32 *data_len)
{
    UINT32 offset;

    if(NULL_PTR == crfsdn)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_read_b: crfsdn is null\n");
        return (EC_FALSE);
    }

    if(NULL_PTR == data_buff)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_read_b: data_buff is null\n");
        return (EC_FALSE);
    } 

    if(CPGB_CACHE_MAX_BYTE_SIZE < data_max_len)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_read_b: data max len %ld overflow\n", data_max_len);
        return (EC_FALSE);
    }   

    ASSERT(0 == page_no);

    offset  = (((UINT32)page_no) << (CPGB_PAGE_4K_BIT_SIZE));
    if(EC_FALSE == crfsdn_read_o(crfsdn, disk_no, block_no, offset, data_max_len, data_buff, data_len))
    {
        sys_log(LOGSTDOUT, "error:crfsdn_read_b: read %u bytes from disk %u block %u page %u failed\n", 
                           data_max_len, disk_no, block_no, page_no);
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

EC_BOOL crfsdn_write_b(CRFSDN *crfsdn, const UINT32 data_max_len, const UINT8 *data_buff, uint16_t *disk_no, uint16_t *block_no, UINT32 *offset)
{
    uint32_t size;
    uint16_t page_no;

    if(NULL_PTR == crfsdn)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_write_b: crfsdn is null\n");
        return (EC_FALSE);
    }

    if(NULL_PTR == data_buff)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_write_b: data_buff is null\n");
        return (EC_FALSE);
    }    

    if(CPGB_CACHE_MAX_BYTE_SIZE < data_max_len + (*offset))
    {
        sys_log(LOGSTDOUT, "error:crfsdn_write_b: data max len %u + offset %u = %u overflow\n", 
                           data_max_len, (*offset), data_max_len + (*offset));
        return (EC_FALSE);
    }

    size = CPGB_CACHE_MAX_BYTE_SIZE;

    if(EC_FALSE == cpgv_new_space(CRFSDN_CPGV(crfsdn), size, disk_no, block_no, &page_no))
    {
        sys_log(LOGSTDOUT, "error:crfsdn_write_b: new %u bytes space from vol failed\n", data_max_len);
        return (EC_FALSE);
    }
    ASSERT(0 == page_no);

    if(EC_FALSE == crfsdn_write_o(crfsdn, data_max_len, data_buff, *disk_no, *block_no, offset))
    {
        sys_log(LOGSTDOUT, "error:crfsdn_write_b: write %u bytes to disk %u block %u page %u failed\n", 
                            data_max_len, (*disk_no), (*block_no), (page_no));
                            
        cpgv_free_space(CRFSDN_CPGV(crfsdn), *disk_no, *block_no, page_no, size);
        return (EC_FALSE);
    }
    sys_log(LOGSTDOUT, "[DEBUG] crfsdn_write_b: write %u bytes to disk %u block %u page %u done\n", 
                        data_max_len, (*disk_no), (*block_no), (page_no));

    return (EC_TRUE);
}

EC_BOOL crfsdn_update_b(CRFSDN *crfsdn, const UINT32 data_max_len, const UINT8 *data_buff, const uint16_t disk_no, const uint16_t block_no, UINT32 *offset)
{
    if(NULL_PTR == crfsdn)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_update_b: crfsdn is null\n");
        return (EC_FALSE);
    }

    if(NULL_PTR == data_buff)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_update_b: data_buff is null\n");
        return (EC_FALSE);
    }    

    if(CPGB_CACHE_MAX_BYTE_SIZE < data_max_len + (*offset))
    {
        sys_log(LOGSTDOUT, "error:crfsdn_update_b: data max len %u + offset %u = %u overflow\n", 
                           data_max_len, (*offset), data_max_len + (*offset));
        return (EC_FALSE);
    }

    if(EC_FALSE == crfsdn_write_o(crfsdn, data_max_len, data_buff, disk_no, block_no, offset))
    {
        sys_log(LOGSTDOUT, "error:crfsdn_update_b: write %u bytes to disk %u block %u failed\n", 
                            data_max_len, disk_no, block_no);                            
        return (EC_FALSE);
    }
    
    sys_log(LOGSTDOUT, "[DEBUG] crfsdn_update_b: write %u bytes to disk %u block %u done\n", 
                        data_max_len, disk_no, block_no);

    return (EC_TRUE);
}

EC_BOOL crfsdn_read_p(CRFSDN *crfsdn, const uint16_t disk_no, const uint16_t block_no, const uint16_t page_no, const UINT32 data_max_len, UINT8 *data_buff, UINT32 *data_len)
{
    UINT32 offset;

    if(NULL_PTR == crfsdn)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_read_p: crfsdn is null\n");
        return (EC_FALSE);
    }

    if(NULL_PTR == data_buff)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_read_p: data_buff is null\n");
        return (EC_FALSE);
    } 

    if(CPGB_CACHE_MAX_BYTE_SIZE < data_max_len)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_read_p: data max len %ld overflow\n", data_max_len);
        return (EC_FALSE);
    }   

    offset  = (((UINT32)page_no) << (CPGB_PAGE_4K_BIT_SIZE));
    sys_log(LOGSTDOUT, "[DEBUG] crfsdn_read_p: disk %u, block %u, page %u ==> offset %u\n", disk_no, block_no, page_no, offset);
    if(EC_FALSE == crfsdn_read_o(crfsdn, disk_no, block_no, offset, data_max_len, data_buff, data_len))
    {
        sys_log(LOGSTDOUT, "error:crfsdn_read_p: read %u bytes from disk %u block %u page %u failed\n", 
                           data_max_len, disk_no, block_no, page_no);
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

EC_BOOL crfsdn_write_p(CRFSDN *crfsdn, const UINT32 data_max_len, const UINT8 *data_buff, uint16_t *disk_no, uint16_t *block_no, uint16_t *page_no)
{
    UINT32   offset;
    uint32_t size;

    if(NULL_PTR == crfsdn)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_write_p: crfsdn is null\n");
        return (EC_FALSE);
    }

    if(NULL_PTR == data_buff)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_write_p: data_buff is null\n");
        return (EC_FALSE);
    }    

    if(CPGB_CACHE_MAX_BYTE_SIZE < data_max_len)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_write_p: data max len %u overflow\n", data_max_len);
        return (EC_FALSE);
    }

    size = (uint32_t)(data_max_len);

    if(EC_FALSE == cpgv_new_space(CRFSDN_CPGV(crfsdn), size, disk_no, block_no,  page_no))
    {
        sys_log(LOGSTDOUT, "error:crfsdn_write_p: new %u bytes space from vol failed\n", data_max_len);
        return (EC_FALSE);
    }

    offset  = (((UINT32)(*page_no)) << (CPGB_PAGE_4K_BIT_SIZE));

    if(EC_FALSE == crfsdn_write_o(crfsdn, data_max_len, data_buff, *disk_no, *block_no, &offset))
    {
        sys_log(LOGSTDOUT, "error:crfsdn_write_p: write %u bytes to disk %u block %u page %u failed\n", 
                            data_max_len, (*disk_no), (*block_no), (*page_no));
                            
        cpgv_free_space(CRFSDN_CPGV(crfsdn), *disk_no, *block_no, *page_no, size);
        return (EC_FALSE);
    }
    sys_log(LOGSTDOUT, "[DEBUG] crfsdn_write_p: write %u bytes to disk %u block %u page %u done\n", 
                        data_max_len, (*disk_no), (*block_no), (*page_no));

    return (EC_TRUE);
}

EC_BOOL crfsdn_write_p_cache(CRFSDN *crfsdn, const UINT32 data_max_len, const UINT8 *data_buff, uint16_t *disk_no, uint16_t *block_no, uint16_t *page_no)
{
    CRFSDN_CACHE_NODE *crfsdn_cache_node;
    uint8_t           *data_buff_t;
    uint32_t size;    

    if(NULL_PTR == crfsdn)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_write_p_cache: crfsdn is null\n");
        return (EC_FALSE);
    }

    if(CPGB_CACHE_MAX_BYTE_SIZE < data_max_len)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_write_p_cache: data max len %u overflow\n", data_max_len);
        return (EC_FALSE);
    }

    size = (uint32_t)(data_max_len);

    crfsdn_wrlock(crfsdn, LOC_CRFSDN_0039);
    if(EC_FALSE == cpgv_new_space(CRFSDN_CPGV(crfsdn), size, disk_no, block_no,  page_no))
    {
        crfsdn_unlock(crfsdn, LOC_CRFSDN_0040);
        sys_log(LOGSTDOUT, "error:crfsdn_write_p_cache: new %u bytes space from vol failed\n", data_max_len);
        return (EC_FALSE);
    }
    crfsdn_unlock(crfsdn, LOC_CRFSDN_0041);

    crfsdn_cache_node = crfsdn_cache_node_new();
    if(NULL_PTR == crfsdn_cache_node)/*try all best*/
    {
        UINT32 offset;

        offset  = (((UINT32)(*page_no)) << (CPGB_PAGE_4K_BIT_SIZE));
        
        if(EC_FALSE == crfsdn_write_o(crfsdn, data_max_len, data_buff, *disk_no, *block_no, &offset))
        {
            sys_log(LOGSTDOUT, "error:crfsdn_write_p_cache: write %u bytes to disk %u block %u page %u failed\n", 
                                data_max_len, (*disk_no), (*block_no), (*page_no));
                                
            cpgv_free_space(CRFSDN_CPGV(crfsdn), *disk_no, *block_no, *page_no, size);
            return (EC_FALSE);
        }    
        sys_log(LOGSTDOUT, "[DEBUG] crfsdn_write_p_cache: write %u bytes to disk %u block %u page %u done\n", 
                            data_max_len, (*disk_no), (*block_no), (*page_no));
        return (EC_TRUE);
    }

    data_buff_t = safe_malloc(data_max_len, LOC_CRFSDN_0042);
    if(NULL_PTR == data_buff_t)/*try all best*/
    {
        UINT32 offset;
        
        crfsdn_cache_node_free(crfsdn_cache_node);

        offset  = (((UINT32)(*page_no)) << (CPGB_PAGE_4K_BIT_SIZE));
        
        if(EC_FALSE == crfsdn_write_o(crfsdn, data_max_len, data_buff, *disk_no, *block_no, &offset))
        {
            sys_log(LOGSTDOUT, "error:crfsdn_write_p_cache: write %u bytes to disk %u block %u page %u failed\n", 
                                data_max_len, (*disk_no), (*block_no), (*page_no));
                                
            cpgv_free_space(CRFSDN_CPGV(crfsdn), *disk_no, *block_no, *page_no, size);
            return (EC_FALSE);
        }    
        sys_log(LOGSTDOUT, "[DEBUG] crfsdn_write_p_cache: write %u bytes to disk %u block %u page %u done\n", 
                            data_max_len, (*disk_no), (*block_no), (*page_no));
        return (EC_TRUE);
    }

    /*clone data*/
    BCOPY(data_buff, data_buff_t, data_max_len);
   
    sys_log(LOGSTDOUT, "[DEBUG] crfsdn_write_p_cache: write %u bytes to disk %u block %u page %u done\n", 
                        data_max_len, (*disk_no), (*block_no), (*page_no));

    CRFSDN_CACHE_NODE_DISK_NO(crfsdn_cache_node)   = (*disk_no);
    CRFSDN_CACHE_NODE_BLOCK_NO(crfsdn_cache_node)  = (*block_no);
    CRFSDN_CACHE_NODE_PAGE_NO(crfsdn_cache_node)   = (*page_no);
    CRFSDN_CACHE_NODE_DATA_SIZE(crfsdn_cache_node) = data_max_len;
    CRFSDN_CACHE_NODE_DATA_BUFF(crfsdn_cache_node) = data_buff_t;    
    
    crfsdn_push_cache_node(crfsdn, crfsdn_cache_node);

    return (EC_TRUE);
}

/*random access for reading, the offset is for the user file but not for the whole 64M page-block */
EC_BOOL crfsdn_read_e(CRFSDN *crfsdn, const uint16_t disk_no, const uint16_t block_no, const uint16_t page_no, const uint32_t offset, const UINT32 data_max_len, UINT8 *data_buff, UINT32 *data_len)
{
    UINT32 offset_t;

    offset_t  = (((UINT32)page_no) << (CPGB_PAGE_4K_BIT_SIZE)) + offset;
    if(EC_FALSE == crfsdn_read_o(crfsdn, disk_no, block_no, offset_t, data_max_len, data_buff, data_len))
    {
        sys_log(LOGSTDOUT, "error:crfsdn_read_e: read %u bytes from disk %u block %u page %u offset %u failed\n", 
                           data_max_len, disk_no, block_no, page_no, offset);
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

/*random access for writting, the offset is for the user file but not for the whole 64M page-block */
EC_BOOL crfsdn_write_e(CRFSDN *crfsdn, const UINT32 data_max_len, const UINT8 *data_buff, const uint16_t disk_no, const uint16_t block_no, const uint16_t page_no, const uint32_t offset)
{
    UINT32 offset_t;

    offset_t  = (((UINT32)page_no) << (CPGB_PAGE_4K_BIT_SIZE)) + offset;
    if(EC_FALSE == crfsdn_write_o(crfsdn, data_max_len, data_buff, disk_no, block_no, &offset_t))
    {
        sys_log(LOGSTDOUT, "error:crfsdn_write_e: write %u bytes to disk %u block %u page %u offset %u failed\n", 
                           data_max_len, disk_no, block_no, page_no, offset_t);
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

EC_BOOL crfsdn_remove(CRFSDN *crfsdn, const uint16_t disk_no, const uint16_t block_no, const uint16_t page_no, const UINT32 data_max_len)
{   
    uint32_t size;

    if(NULL_PTR == crfsdn)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_remove: crfsdn is null\n");
        return (EC_FALSE);
    }

    if(CPGB_CACHE_MAX_BYTE_SIZE < data_max_len)
    {
        sys_log(LOGSTDOUT, "error:crfsdn_remove: data max len %u overflow\n", data_max_len);
        return (EC_FALSE);
    }

    size = (uint32_t)(data_max_len);

    if(EC_FALSE == cpgv_free_space(CRFSDN_CPGV(crfsdn), disk_no, block_no, page_no, size))
    {
        sys_log(LOGSTDOUT, "error:crfsdn_remove: free %u bytes space to vol failed\n", data_max_len);
        return (EC_FALSE);
    } 
    sys_log(LOGSTDOUT, "[DEBUG] crfsdn_remove: free %u bytes to disk %u block %u page %u done\n", 
                        data_max_len, disk_no, block_no, page_no);

    return (EC_TRUE);
}


EC_BOOL crfsdn_show(LOG *log, const char *root_dir)
{
    CRFSDN *crfsdn;

    crfsdn = crfsdn_open(root_dir);
    if(NULL_PTR == crfsdn)
    {
        sys_log(log, "error:crfsdn_show: open crfsdn %s failed\n", root_dir);
        return (EC_FALSE);
    }

    crfsdn_print(log, crfsdn);

    crfsdn_close(crfsdn);

    return (EC_TRUE);
}

EC_BOOL crfsdn_rdlock(CRFSDN *crfsdn, const UINT32 location)
{
    CRFSDN_CRWLOCK_RDLOCK(crfsdn, location);
    return (EC_TRUE);
}

EC_BOOL crfsdn_wrlock(CRFSDN *crfsdn, const UINT32 location)
{
    CRFSDN_CRWLOCK_WRLOCK(crfsdn, location);
    return (EC_TRUE);
}

EC_BOOL crfsdn_unlock(CRFSDN *crfsdn, const UINT32 location)
{
    CRFSDN_CRWLOCK_UNLOCK(crfsdn, location);
    return (EC_TRUE);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

