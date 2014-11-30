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

/*Random File System Data Node*/

#ifndef _CRFSDN_H
#define _CRFSDN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>

#include "type.h"
#include "clist.h"
#include "cmutex.h"
#include "crb.h"
#include "cpgrb.h"
#include "cpgb.h"
#include "cpgd.h"
#include "cpgv.h"

#define CRFSDN_DB_NAME      ((const char *)"dn_cfg.dat")

#define CRFSDN_NODE_NAME_MAX_SIZE       (256)

#define CRFSDN_NODE_CACHED_MAX_NUM      (1024)

//#define CRFSDN_DISK_FIXED_NUM           (16)    /*virtual disk, fix its number*/
#define CRFSDN_DISK_FIXED_NUM           (10)    /*virtual disk number*/

#define CRFSDN_NODE_O_RDONLY           ((UINT32)O_RDONLY)
#define CRFSDN_NODE_O_WRONLY           ((UINT32)O_WRONLY)
#define CRFSDN_NODE_O_RDWR             ((UINT32)O_RDWR  )
#define CRFSDN_NODE_O_CREATE           ((UINT32)O_CREAT )

#define CRFSDN_PATH_LAYOUT_DIR0_NBITS    ( 16)
#define CRFSDN_PATH_LAYOUT_DIR1_NBITS    ( 16)

#define CRFSDN_PATH_LAYOUT_DIR0_ABITS    (16) /*bit alignment*/
#define CRFSDN_PATH_LAYOUT_DIR1_ABITS    (0) /*bit alignment*/

#define CRFSDN_PATH_LAYOUT_DIR0_MASK     (((UINT32)(UINT32_ONE << CRFSDN_PATH_LAYOUT_DIR0_NBITS)) - 1)
#define CRFSDN_PATH_LAYOUT_DIR1_MASK     (((UINT32)(UINT32_ONE << CRFSDN_PATH_LAYOUT_DIR1_NBITS)) - 1)

#define CRFSDN_PATH_LAYOUT_DIR0_NO(path_id)     (((path_id) >> CRFSDN_PATH_LAYOUT_DIR0_ABITS) & CRFSDN_PATH_LAYOUT_DIR0_MASK)
#define CRFSDN_PATH_LAYOUT_DIR1_NO(path_id)     (((path_id) >> CRFSDN_PATH_LAYOUT_DIR1_ABITS) & CRFSDN_PATH_LAYOUT_DIR1_MASK)

typedef struct
{
    UINT32  crfsdn_tcid;
    UINT32  crfsdn_full;
}CRFSDN_STAT;

#define CRFSDN_STAT_TCID(crfsdn_stat)       ((crfsdn_stat)->crfsdn_tcid)
#define CRFSDN_STAT_FULL(crfsdn_stat)       ((crfsdn_stat)->crfsdn_full)

#define CRFSDN_NODE_FLAG_UPDATED_BIT         ((UINT32) 0x2)
#define CRFSDN_NODE_FLAG_WRITE_BIT           ((UINT32) 0x4)
#define CRFSDN_NODE_FLAG_MASK                ((UINT32) 0xF)/*4 bits*/

#define CRFSDN_NODE_FLAG_READ_BIT            ((UINT32)0x10)/*trick!*/
#define CRFSDN_NODE_FLAG_UNDEF               ((UINT32)0x00)

#define CRFSDN_EXPIRED_IN_NSEC               ((uint32_t) 5 * 60) /*300 seconds*/

/*memory cached block info*/
typedef struct
{  
    UINT32          id;        /*id = disk_no | block_no*/
    UINT32          flags;

    CMUTEX          cmutex;     /*cmutex for node read/write*/
    ctime_t         atime;      /*last access time (in seconds)*/
#if (32 == WORDSIZE)
    uint32_t        rsvd;
#endif
    
    int             block_fd;   /* block fd */
    uint16_t        reader_num;
    uint16_t        access_num;
}CRFSDN_NODE;

#define CRFSDN_NODE_ID(crfsdn_node)                       ((crfsdn_node)->id)
#define CRFSDN_NODE_FLAGS(crfsdn_node)                    ((crfsdn_node)->flags)
#define CRFSDN_NODE_CMUTEX(crfsdn_node)                   (&((crfsdn_node)->cmutex))
#define CRFSDN_NODE_ATIME(crfsdn_node)                    ((crfsdn_node)->atime)
#define CRFSDN_NODE_FD(crfsdn_node)                       ((crfsdn_node)->block_fd)
#define CRFSDN_NODE_READER_NUM(crfsdn_node)               ((crfsdn_node)->reader_num)
#define CRFSDN_NODE_ACCESS_NUM(crfsdn_node)               ((crfsdn_node)->access_num)

#define CRFSDN_NODE_ID_MAKE(disk_no, block_no)            ((((UINT32)(disk_no)) << 16) | (((UINT32)(block_no)) << 0))
#define CRFSDN_NODE_ID_GET_DISK_NO(node_id)               ((uint16_t)(((node_id) >> 16) & 0xFFFF))
#define CRFSDN_NODE_ID_GET_BLOCK_NO(node_id)              ((uint16_t)(((node_id) >>  0) & 0xFFFF))
#define CRFSDN_NODE_ERR_ID                                (CRFSDN_NODE_ID_MAKE(CPGRB_ERR_POS, CPGRB_ERR_POS))

#define CRFSDN_NODE_DISK_NO(crfsdn_node)                  (CRFSDN_NODE_ID_GET_DISK_NO(CRFSDN_NODE_ID(crfsdn_node)))
#define CRFSDN_NODE_BLOCK_NO(crfsdn_node)                 (CRFSDN_NODE_ID_GET_BLOCK_NO(CRFSDN_NODE_ID(crfsdn_node)))

//#define CRFSDN_PATH_LAYOUT(disk_no, block_no)             ((((UINT32)(block_no)) << 16) | (((UINT32)(disk_no)) << 0)) /*disorder*/
#define CRFSDN_PATH_LAYOUT(disk_no, block_no)             ((((UINT32)(disk_no)) << 16) | (((UINT32)(block_no)) << 0))
#define CRFSDN_NODE_PATH_LAYOUT(crfsdn_node)              (CRFSDN_PATH_LAYOUT(CRFSDN_NODE_DISK_NO(crfsdn_node), CRFSDN_NODE_BLOCK_NO(crfsdn_node)))


#if 1
#define CRFSDN_NODE_CMUTEX_INIT(crfsdn_node, location)    (cmutex_init(CRFSDN_NODE_CMUTEX(crfsdn_node), CMUTEX_PROCESS_PRIVATE, location))
#define CRFSDN_NODE_CMUTEX_CLEAN(crfsdn_node, location)   (cmutex_clean(CRFSDN_NODE_CMUTEX(crfsdn_node), location))
#define CRFSDN_NODE_CMUTEX_LOCK(crfsdn_node, location)    (cmutex_lock(CRFSDN_NODE_CMUTEX(crfsdn_node), location))
#define CRFSDN_NODE_CMUTEX_UNLOCK(crfsdn_node, location)  (cmutex_unlock(CRFSDN_NODE_CMUTEX(crfsdn_node), location))
#endif

typedef struct
{
    uint16_t        disk_no;
    uint16_t        block_no;
    uint16_t        page_no;
    uint16_t        rsvd1;
    UINT32          data_size;
    uint8_t        *data_buff;
}CRFSDN_CACHE_NODE;

#define CRFSDN_CACHE_NODE_DISK_NO(crfsdn_cache_node)        ((crfsdn_cache_node)->disk_no)
#define CRFSDN_CACHE_NODE_BLOCK_NO(crfsdn_cache_node)       ((crfsdn_cache_node)->block_no)
#define CRFSDN_CACHE_NODE_PAGE_NO(crfsdn_cache_node)        ((crfsdn_cache_node)->page_no)
#define CRFSDN_CACHE_NODE_DATA_SIZE(crfsdn_cache_node)      ((crfsdn_cache_node)->data_size)
#define CRFSDN_CACHE_NODE_DATA_BUFF(crfsdn_cache_node)      ((crfsdn_cache_node)->data_buff)

typedef struct
{
    CRWLOCK            rwlock;
    CRB_TREE           open_nodes;  /*open nodes to read or write, item is CRFSDN_NODE*/
    CMUTEX             cmutex;      /*cmutex for open nodes which was accessed by do_slave thread and task_brd_cbtimer_do thread*/
    CLIST              cached_nodes;/*cached nodes to read or write, item is CRFSDN_NODE*/

    uint8_t           *root_dname;
    CPGV              *cpgv;
}CRFSDN;

#define CRFSDN_CRWLOCK(crfsdn)                             (&((crfsdn)->rwlock))
#define CRFSDN_CMUTEX(crfsdn)                              (&((crfsdn)->cmutex))
#define CRFSDN_OPEN_NODES(crfsdn)                          (&((crfsdn)->open_nodes))
#define CRFSDN_CACHED_NODES(crfsdn)                        (&((crfsdn)->cached_nodes))
#define CRFSDN_ROOT_DNAME(crfsdn)                          ((crfsdn)->root_dname)
#define CRFSDN_CPGV(crfsdn)                                ((crfsdn)->cpgv)

#define CRFSDN_OPEN_NODE(_crfsdn, node_id)                 (crfsdn_node_fetch((_crfsdn), (node_id)))

#if 1
#define CRFSDN_CRWLOCK_INIT(crfsdn, location)       (crwlock_init(CRFSDN_CRWLOCK(crfsdn), CMUTEX_PROCESS_PRIVATE, location))
#define CRFSDN_CRWLOCK_CLEAN(crfsdn, location)      (crwlock_clean(CRFSDN_CRWLOCK(crfsdn), location))

#define CRFSDN_CRWLOCK_RDLOCK(crfsdn, location)     (crwlock_rdlock(CRFSDN_CRWLOCK(crfsdn), location))
#define CRFSDN_CRWLOCK_WRLOCK(crfsdn, location)     (crwlock_wrlock(CRFSDN_CRWLOCK(crfsdn), location))
#define CRFSDN_CRWLOCK_UNLOCK(crfsdn, location)     (crwlock_unlock(CRFSDN_CRWLOCK(crfsdn), location))
#endif

#if 0
#define CRFSDN_CRWLOCK_INIT(crfsdn, location)  do{\
    sys_log(LOGSTDOUT, "[DEBUG] CRFSDN_CRWLOCK_INIT: CRFSDN_CRWLOCK %p, at %s:%ld\n", CRFSDN_CRWLOCK(crfsdn), MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));\
    croutine_rwlock_init(CRFSDN_CRWLOCK(crfsdn), CMUTEX_PROCESS_PRIVATE, location);\
}while(0)

#define CRFSDN_CRWLOCK_CLEAN(crfsdn, location) do{\
    sys_log(LOGSTDOUT, "[DEBUG] CRFSDN_CRWLOCK_CLEAN: CRFSDN_CRWLOCK %p, at %s:%ld\n", CRFSDN_CRWLOCK(crfsdn), MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));\
    croutine_rwlock_clean(CRFSDN_CRWLOCK(crfsdn), location);\
}while(0)    

#define CRFSDN_CRWLOCK_RDLOCK(crfsdn, location)     do{\
    sys_log(LOGSTDOUT, "[DEBUG] CRFSDN_CRWLOCK_RDLOCK: CRFSDN_CRWLOCK %p, at %s:%ld\n", CRFSDN_CRWLOCK(crfsdn), MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));\
    croutine_rwlock_rdlock(CRFSDN_CRWLOCK(crfsdn), location);\
    sys_log(LOGSTDOUT, "[DEBUG] CRFSDN_CRWLOCK_RDLOCK: CRFSDN_CRWLOCK %p, at %s:%ld done\n", CRFSDN_CRWLOCK(crfsdn), MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));\
}while(0)

#define CRFSDN_CRWLOCK_WRLOCK(crfsdn, location)     do{\
    sys_log(LOGSTDOUT, "[DEBUG] CRFSDN_CRWLOCK_WRLOCK: CRFSDN_CRWLOCK %p, at %s:%ld\n", CRFSDN_CRWLOCK(crfsdn), MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));\
    croutine_rwlock_wrlock(CRFSDN_CRWLOCK(crfsdn), location);\
    sys_log(LOGSTDOUT, "[DEBUG] CRFSDN_CRWLOCK_WRLOCK: CRFSDN_CRWLOCK %p, at %s:%ld done\n", CRFSDN_CRWLOCK(crfsdn), MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));\
}while(0)
#define CRFSDN_CRWLOCK_UNLOCK(crfsdn, location)     do{\
    sys_log(LOGSTDOUT, "[DEBUG] CRFSDN_CRWLOCK_UNLOCK: CRFSDN_CRWLOCK %p, at %s:%ld\n", CRFSDN_CRWLOCK(crfsdn), MM_LOC_FILE_NAME(location),MM_LOC_LINE_NO(location));\
    croutine_rwlock_unlock(CRFSDN_CRWLOCK(crfsdn), location);\
}while(0)
#endif

#if 0
#define CRFSDN_CMUTEX_INIT(crfsdn, location)         (croutine_mutex_init(CRFSDN_CMUTEX(crfsdn), CMUTEX_PROCESS_PRIVATE, location))
#define CRFSDN_CMUTEX_CLEAN(crfsdn, location)        (croutine_mutex_clean(CRFSDN_CMUTEX(crfsdn), location))
#define CRFSDN_CMUTEX_LOCK(crfsdn, location)         (croutine_mutex_lock(CRFSDN_CMUTEX(crfsdn), location))
#define CRFSDN_CMUTEX_UNLOCK(crfsdn, location)       (croutine_mutex_unlock(CRFSDN_CMUTEX(crfsdn), location))
#endif

#if 1
#define CRFSDN_CMUTEX_INIT(crfsdn, location)         (cmutex_init(CRFSDN_CMUTEX(crfsdn), CMUTEX_PROCESS_PRIVATE, location))
#define CRFSDN_CMUTEX_CLEAN(crfsdn, location)        (cmutex_clean(CRFSDN_CMUTEX(crfsdn), location))
#define CRFSDN_CMUTEX_LOCK(crfsdn, location)         (cmutex_lock(CRFSDN_CMUTEX(crfsdn), location))
#define CRFSDN_CMUTEX_UNLOCK(crfsdn, location)       (cmutex_unlock(CRFSDN_CMUTEX(crfsdn), location))
#endif


CRFSDN_NODE *crfsdn_node_new();

EC_BOOL crfsdn_node_init(CRFSDN_NODE *crfsdn_node);

EC_BOOL crfsdn_node_clean(CRFSDN_NODE *crfsdn_node);

EC_BOOL crfsdn_node_free(CRFSDN_NODE *crfsdn_node);

int crfsdn_node_cmp(const CRFSDN_NODE *crfsdn_node_1st, const CRFSDN_NODE *crfsdn_node_2nd);

void crfsdn_node_print(LOG *log, const CRFSDN_NODE *crfsdn_node);

/*for debug only*/
void crfsdn_node_fname_print(LOG *log, const CRFSDN *crfsdn, const UINT32 node_id);

CRFSDN_NODE *crfsdn_node_fetch(const CRFSDN *crfsdn, const UINT32 node_id);

EC_BOOL crfsdn_node_delete(CRFSDN *crfsdn, const UINT32 node_id);

CRFSDN_NODE *crfsdn_node_create(CRFSDN *crfsdn, const UINT32 node_id);

CRFSDN_NODE *crfsdn_node_open(CRFSDN *crfsdn, const UINT32 node_id, const UINT32 open_flags);

EC_BOOL crfsdn_node_unlink(CRFSDN *crfsdn, const UINT32 node_id);

EC_BOOL crfsdn_node_write(CRFSDN *crfsdn, const UINT32 node_id, const UINT32 data_max_len, const UINT8 *data_buff, UINT32 *offset);

EC_BOOL crfsdn_node_read(CRFSDN *crfsdn, const UINT32 node_id, const UINT32 data_max_len, UINT8 *data_buff, UINT32 *offset);

CRFSDN_CACHE_NODE *crfsdn_cache_node_new();

EC_BOOL crfsdn_cache_node_init(CRFSDN_CACHE_NODE *crfsdn_cache_node);

EC_BOOL crfsdn_cache_node_clean(CRFSDN_CACHE_NODE *crfsdn_cache_node);

EC_BOOL crfsdn_cache_node_free(CRFSDN_CACHE_NODE *crfsdn_cache_node);

EC_BOOL crfsdn_cache_node_init_0(const UINT32 md_id, CRFSDN_CACHE_NODE *crfsdn_cache_node);

EC_BOOL crfsdn_cache_node_clean_0(const UINT32 md_id, CRFSDN_CACHE_NODE *crfsdn_cache_node);

EC_BOOL crfsdn_cache_node_free_0(const UINT32 md_id, CRFSDN_CACHE_NODE *crfsdn_cache_node);

EC_BOOL crfsdn_has_no_cache_node(const CRFSDN *crfsdn);

EC_BOOL crfsdn_push_cache_node(CRFSDN *crfsdn, CRFSDN_CACHE_NODE *crfsdn_cache_node);

CRFSDN_CACHE_NODE *crfsdn_pop_cache_node(CRFSDN *crfsdn);

EC_BOOL crfsdn_flush_cache_node(CRFSDN *crfsdn, CRFSDN_CACHE_NODE *crfsdn_cache_node);

void crfsdn_flush_cache_nodes(CRFSDN **crfsdn, EC_BOOL *terminate_flag);

EC_BOOL crfsdn_expire_open_nodes(CRFSDN *crfsdn);

CRFSDN *crfsdn_create(const char *root_dname);

EC_BOOL crfsdn_add_disk(CRFSDN *crfsdn, const uint16_t disk_no);

EC_BOOL crfsdn_del_disk(CRFSDN *crfsdn, const uint16_t disk_no);

EC_BOOL crfsdn_mount_disk(CRFSDN *crfsdn, const uint16_t disk_no);

EC_BOOL crfsdn_umount_disk(CRFSDN *crfsdn, const uint16_t disk_no);

CRFSDN *crfsdn_new();

EC_BOOL crfsdn_init(CRFSDN *crfsdn);

EC_BOOL crfsdn_clean(CRFSDN *crfsdn);

EC_BOOL crfsdn_free(CRFSDN *crfsdn);

void crfsdn_print(LOG *log, const CRFSDN *crfsdn);

EC_BOOL crfsdn_is_full(CRFSDN *crfsdn);

EC_BOOL crfsdn_flush(CRFSDN *crfsdn);

EC_BOOL crfsdn_load(CRFSDN *crfsdn, const char *root_dname);

EC_BOOL crfsdn_exist(const char *root_dname);

CRFSDN *crfsdn_open(const char *root_dir);

EC_BOOL crfsdn_close(CRFSDN *crfsdn);

EC_BOOL crfsdn_fetch_block_fd(CRFSDN *crfsdn, const uint16_t disk_no, const uint16_t block_no, int *block_fd);

/*random access for reading, the offset is for the whole 64M page-block */
EC_BOOL crfsdn_read_o(CRFSDN *crfsdn, const uint16_t disk_no, const uint16_t block_no, const UINT32 offset, const UINT32 data_max_len, UINT8 *data_buff, UINT32 *data_len);

/*random access for writting */
/*offset: IN/OUT, the offset is for the whole 64M page-block*/
EC_BOOL crfsdn_write_o(CRFSDN *crfsdn, const UINT32 data_max_len, const UINT8 *data_buff, const uint16_t disk_no, const uint16_t block_no, UINT32 *offset);

EC_BOOL crfsdn_read_b(CRFSDN *crfsdn, const uint16_t disk_no, const uint16_t block_no, const uint16_t page_no, const UINT32 data_max_len, UINT8 *data_buff, UINT32 *data_len);

EC_BOOL crfsdn_write_b(CRFSDN *crfsdn, const UINT32 data_max_len, const UINT8 *data_buff, uint16_t *disk_no, uint16_t *block_no, UINT32 *offset);

EC_BOOL crfsdn_update_b(CRFSDN *crfsdn, const UINT32 data_max_len, const UINT8 *data_buff, const uint16_t disk_no, const uint16_t block_no, UINT32 *offset);

EC_BOOL crfsdn_read_p(CRFSDN *crfsdn, const uint16_t disk_no, const uint16_t block_no, const uint16_t page_no, const UINT32 data_max_len, UINT8 *data_buff, UINT32 *data_len);

EC_BOOL crfsdn_write_p(CRFSDN *crfsdn, const UINT32 data_max_len, const UINT8 *data_buff, uint16_t *disk_no, uint16_t *block_no, uint16_t *page_no);

EC_BOOL crfsdn_write_p_cache(CRFSDN *crfsdn, const UINT32 data_max_len, const UINT8 *data_buff, uint16_t *disk_no, uint16_t *block_no, uint16_t *page_no);

/*random access for reading, the offset is for the user file but not for the whole 64M page-block */
EC_BOOL crfsdn_read_e(CRFSDN *crfsdn, const uint16_t disk_no, const uint16_t block_no, const uint16_t page_no, const uint32_t offset, const UINT32 data_max_len, UINT8 *data_buff, UINT32 *data_len);

/*random access for writting, the offset is for the user file but not for the whole 64M page-block */
EC_BOOL crfsdn_write_e(CRFSDN *crfsdn, const UINT32 data_max_len, const UINT8 *data_buff, const uint16_t disk_no, const uint16_t block_no, const uint16_t page_no, const uint32_t offset);

EC_BOOL crfsdn_remove(CRFSDN *crfsdn, const uint16_t disk_no, const uint16_t block_no, const uint16_t page_no, const UINT32 data_max_len);

EC_BOOL crfsdn_show(LOG *log, const char *root_dir);

EC_BOOL crfsdn_rdlock(CRFSDN *crfsdn, const UINT32 location);

EC_BOOL crfsdn_wrlock(CRFSDN *crfsdn, const UINT32 location);

EC_BOOL crfsdn_unlock(CRFSDN *crfsdn, const UINT32 location);

#endif/* _CRFSDN_H */

#ifdef __cplusplus
}
#endif/*__cplusplus*/

