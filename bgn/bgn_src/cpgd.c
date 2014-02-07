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
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "type.h"
#include "mm.h"
#include "log.h"

#include "cmisc.h"
#include "real.h"

#include "db_internal.h"

#include "cpgrb.h"
#include "cpgb.h"
#include "cpgd.h"

/*page-cache disk:1TB = 2^14 page-cache block*/

/************************************************************************************************
  comment:
  ========
   1. if one block can assign max pages with page model, then put the block into page model 
      RB tree of disk
   2. one block was in at most one RB tree
************************************************************************************************/

#if (SWITCH_ON == CRFS_ASSERT_SWITCH)
#define CPGD_ASSERT(cond)   ASSERT(cond)
#endif/*(SWITCH_ON == CRFS_ASSERT_SWITCH)*/

#if (SWITCH_OFF == CRFS_ASSERT_SWITCH)
#define CPGD_ASSERT(cond)   do{}while(0)
#endif/*(SWITCH_OFF == CRFS_ASSERT_SWITCH)*/

#define DEBUG_COUNT_CPGD_HDR_PAD_SIZE() \
                                (sizeof(CPGD_HDR) \
                                 - sizeof(CPGRB_POOL) \
                                 - CPGB_MODEL_NUM *sizeof(uint16_t) \
                                 - 3 * sizeof(uint16_t) \
                                 - 1 * sizeof(uint32_t) \
                                 - 2 * sizeof(uint32_t) \
                                 - sizeof(uint64_t))


#define ASSERT_CPGD_HDR_PAD_SIZE() \
    CPGD_ASSERT( CPGD_HDR_PAD_SIZE == DEBUG_COUNT_CPGD_HDR_PAD_SIZE() )

static uint16_t __cpgd_page_model_first_block(const CPGD *cpgd, const uint16_t page_model)
{
    uint16_t node_pos;
    const CPGRB_NODE *node;
    
    node_pos = cpgrb_tree_first_node(CPGD_PAGE_BLOCK_CPGRB_POOL(cpgd), CPGD_PAGE_MODEL_BLOCK_CPGRB_ROOT_POS(cpgd, page_model));
    if(CPGRB_ERR_POS == node_pos)
    {
        sys_log(LOGSTDERR, "error:__cpgd_page_model_first_block: no free page in page model %u\n", page_model);
        return (CPGRB_ERR_POS);
    }

    node = CPGRB_POOL_NODE(CPGD_PAGE_BLOCK_CPGRB_POOL(cpgd), node_pos);
    return (CPGRB_NODE_DATA(node));
}

static uint16_t __cpgd_page_model_get(const CPGD *cpgd, const uint16_t assign_bitmap)
{
    uint16_t page_model;
    uint16_t e;

    for(page_model = 0, e = 1; CPGB_MODEL_NUM > page_model && (assign_bitmap & e); page_model ++, e <<= 1)
    {
      /*do nothing*/
    }
    return (page_model);
}

static void __cpgd_hdr_size_info_print()
{
    sys_log(LOGSTDOUT, "[DEBUG] __cpgd_hdr_size_info_print: sizeof(CPGD_HDR)   = %u\n", sizeof(CPGD_HDR));
    sys_log(LOGSTDOUT, "[DEBUG] __cpgd_hdr_size_info_print: sizeof(CPGRB_POOL) = %u\n", sizeof(CPGRB_POOL));
    sys_log(LOGSTDOUT, "[DEBUG] __cpgd_hdr_size_info_print: CPGB_MODEL_NUM     = %u\n", CPGB_MODEL_NUM);
    sys_log(LOGSTDOUT, "[DEBUG] __cpgd_hdr_size_info_print: sizeof(uint64_t)   = %u\n", sizeof(uint64_t));
    sys_log(LOGSTDOUT, "[DEBUG] __cpgd_hdr_size_info_print: CPGD_HDR_PAD_SIZE  = %u\n", CPGD_HDR_PAD_SIZE);
    sys_log(LOGSTDOUT, "[DEBUG] __cpgd_hdr_size_info_print: sizeof(CPGD_HDR) "
                                 "- sizeof(CPGRB_POOL) "
                                 "- CPGB_MODEL_NUM *sizeof(uint16_t) "
                                 "- 3 * sizeof(uint16_t) "
                                 "- 1 * sizeof(uint32_t) "
                                 "- 2 * sizeof(uint32_t) "
                                 "- 1 * sizeof(uint64_t) = %u\n",
                                 DEBUG_COUNT_CPGD_HDR_PAD_SIZE());   
    return;
}

CPGD_HDR *cpgd_hdr_new(CPGD *cpgd, const uint16_t block_num)
{
    CPGD_HDR *cpgd_hdr;
    uint16_t  page_model;

    __cpgd_hdr_size_info_print();

    ASSERT_CPGD_HDR_PAD_SIZE();

    cpgd_hdr = (CPGD_HDR *)mmap(NULL_PTR, CPGD_FSIZE(cpgd), PROT_READ | PROT_WRITE, MAP_SHARED, CPGD_FD(cpgd), 0);
    if(MAP_FAILED == cpgd_hdr)
    {
        sys_log(LOGSTDOUT, "error:cpgd_hdr_new: mmap file %s failed, errno = %d, errorstr = %s\n", 
                           (char *)CPGD_FNAME(cpgd), errno, strerror(errno));
        return (NULL_PTR);
    } 

    if(EC_FALSE == cpgrb_pool_init(CPGD_HDR_CPGRB_POOL(cpgd_hdr), block_num))
    {
        sys_log(LOGSTDERR, "error:cpgd_hdr_new: init cpgrb pool failed where block_num = %u\n", block_num);
        munmap(cpgd_hdr, CPGD_FSIZE(cpgd));
        return (NULL_PTR);
    }

    for(page_model = 0; CPGB_MODEL_NUM > page_model; page_model ++)
    {
        CPGD_HDR_BLOCK_CPGRB_ROOT_POS(cpgd_hdr, page_model) = CPGRB_ERR_POS;
    }

    CPGD_HDR_ASSIGN_BITMAP(cpgd_hdr) = 0;

    CPGD_HDR_PAGE_BLOCK_MAX_NUM(cpgd_hdr) = block_num;

    /*statistics*/    
    CPGD_HDR_PAGE_4K_MAX_NUM(cpgd_hdr)       = block_num * CPGD_BLOCK_PAGE_NUM;
    CPGD_HDR_PAGE_4K_USED_NUM(cpgd_hdr)      = 0;
    CPGD_HDR_PAGE_ACTUAL_USED_SIZE(cpgd_hdr) = 0;    
    
    return (cpgd_hdr);
}

EC_BOOL cpgd_hdr_free(CPGD *cpgd)
{   
    if(NULL_PTR != CPGD_HEADER(cpgd))
    {        
        if(0 != msync(CPGD_HEADER(cpgd), CPGD_FSIZE(cpgd), MS_SYNC))
        {
            sys_log(LOGSTDOUT, "warn:cpgd_hdr_free: sync cpgd_hdr of %s with size %u failed\n", 
                               CPGD_FNAME(cpgd), CPGD_FSIZE(cpgd));
        }
        
        if(0 != munmap(CPGD_HEADER(cpgd), CPGD_FSIZE(cpgd)))
        {
            sys_log(LOGSTDOUT, "warn:cpgd_hdr_free: munmap cpgd of %s with size %u failed\n", 
                               CPGD_FNAME(cpgd), CPGD_FSIZE(cpgd));
        }
        
        CPGD_HEADER(cpgd) = NULL_PTR;        
    }

    return (EC_TRUE);
}

CPGD_HDR *cpgd_hdr_open(CPGD *cpgd)
{
    CPGD_HDR *cpgd_hdr;

    ASSERT_CPGD_HDR_PAD_SIZE();

    sys_log(LOGSTDOUT, "[DEBUG] cpgd_hdr_open: fsize %u\n", CPGD_FSIZE(cpgd));

    cpgd_hdr = (CPGD_HDR *)mmap(NULL_PTR, CPGD_FSIZE(cpgd), PROT_READ | PROT_WRITE, MAP_SHARED, CPGD_FD(cpgd), 0);
    if(MAP_FAILED == cpgd_hdr)
    {
        sys_log(LOGSTDOUT, "error:cpgd_hdr_open: mmap file %s with fd %d failed, errno = %d, errorstr = %s\n", 
                           (char *)CPGD_FNAME(cpgd), CPGD_FD(cpgd), errno, strerror(errno));
        return (NULL_PTR);
    } 
    
    return (cpgd_hdr);
}

EC_BOOL cpgd_hdr_close(CPGD *cpgd)
{   
    if(NULL_PTR != CPGD_HEADER(cpgd))
    {   
        if(0 != msync(CPGD_HEADER(cpgd), CPGD_FSIZE(cpgd), MS_SYNC))
        {
            sys_log(LOGSTDOUT, "warn:cpgd_hdr_close: sync cpgd_hdr of %s with size %u failed\n", 
                               CPGD_FNAME(cpgd), CPGD_FSIZE(cpgd));
        }
        
        if(0 != munmap(CPGD_HEADER(cpgd), CPGD_FSIZE(cpgd)))
        {
            sys_log(LOGSTDOUT, "warn:cpgd_hdr_close: munmap cpgd of %s with size %u failed\n", 
                               CPGD_FNAME(cpgd), CPGD_FSIZE(cpgd));
        }
       
        CPGD_HEADER(cpgd) = NULL_PTR;        
    }

    return (EC_TRUE);
}

EC_BOOL cpgd_hdr_sync(CPGD *cpgd)
{   
    if(NULL_PTR != CPGD_HEADER(cpgd))
    {   
        if(0 != msync(CPGD_HEADER(cpgd), CPGD_FSIZE(cpgd), MS_SYNC))
        {
            sys_log(LOGSTDOUT, "warn:cpgd_hdr_sync: sync cpgd_hdr of %s with size %u failed\n", 
                               CPGD_FNAME(cpgd), CPGD_FSIZE(cpgd));
        }             
    }

    return (EC_TRUE);
}

EC_BOOL cpgd_hdr_flush_size(const CPGD_HDR *cpgd_hdr, UINT32 *size)
{
    (*size) += sizeof(CPGD_HDR);
    return (EC_TRUE);
}

EC_BOOL cpgd_hdr_flush(const CPGD_HDR *cpgd_hdr, int fd, UINT32 *offset)
{
    UINT32 osize;/*flush once size*/
    DEBUG(UINT32 offset_saved = *offset;);

    /*flush rbtree pool*/
    if(EC_FALSE == cpgrb_flush(CPGD_HDR_CPGRB_POOL(cpgd_hdr), fd, offset))
    {
        sys_log(LOGSTDOUT, "error:cpgd_hdr_flush: flush CPGD_HDR_CPGRB_POOL at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }

    /*flush CPGD_PAGE_MODEL_BLOCK_CPGRB_ROOT_POS_TBL*/
    osize = CPGB_MODEL_NUM * sizeof(uint16_t);
    if(EC_FALSE == c_file_flush(fd, offset, osize, (uint8_t *)CPGD_HDR_BLOCK_CPGRB_ROOT_POS_TBL(cpgd_hdr)))
    {
        sys_log(LOGSTDOUT, "error:cpgd_hdr_flush: flush CPGD_HDR_BLOCK_CPGRB_ROOT_POS_TBL at offset %u of fd %d failed\n", 
                            (*offset), fd);
        return (EC_FALSE);
    }  

    /*skip rsvd1*/
    osize = sizeof(uint16_t);
    if(EC_FALSE == c_file_pad(fd, offset, osize, FILE_PAD_CHAR))
    {
        sys_log(LOGSTDOUT, "error:cpgd_hdr_flush: pad %u bytes at offset %u of fd %d failed\n", osize, (*offset), fd);
        return (EC_FALSE);
    }    

    /*flush CPGD_HDR_ASSIGN_BITMAP*/
    osize = sizeof(uint16_t);
    if(EC_FALSE == c_file_flush(fd, offset, osize, (uint8_t *)&(CPGD_HDR_ASSIGN_BITMAP(cpgd_hdr))))
    {
        sys_log(LOGSTDOUT, "error:cpgd_hdr_flush: flush CPGD_HDR_ASSIGN_BITMAP at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }    
    
    /*flush CPGD_HDR_PAGE_BLOCK_MAX_NUM*/
    osize = sizeof(uint16_t);
    if(EC_FALSE == c_file_flush(fd, offset, osize, (uint8_t *)&(CPGD_HDR_PAGE_BLOCK_MAX_NUM(cpgd_hdr))))
    {
        sys_log(LOGSTDOUT, "error:cpgd_hdr_flush: flush CPGD_HDR_PAGE_BLOCK_MAX_NUM at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }

    /*flush CPGD_HDR_PAGE_4K_MAX_NUM*/
    osize = sizeof(uint32_t);
    if(EC_FALSE == c_file_flush(fd, offset, osize, (uint8_t *)&(CPGD_HDR_PAGE_4K_MAX_NUM(cpgd_hdr))))
    {
        sys_log(LOGSTDOUT, "error:cpgd_hdr_flush: flush CPGD_HDR_PAGE_4K_MAX_NUM at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }

    /*flush CPGD_HDR_PAGE_4K_USED_NUM*/
    osize = sizeof(uint32_t);
    if(EC_FALSE == c_file_flush(fd, offset, osize, (uint8_t *)&(CPGD_HDR_PAGE_4K_USED_NUM(cpgd_hdr))))
    {
        sys_log(LOGSTDOUT, "error:cpgd_hdr_flush: flush CPGD_HDR_PAGE_4K_USED_NUM at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }    

    /*flush CPGD_HDR_PAGE_ACTUAL_USED_SIZE*/
    osize = sizeof(uint64_t);
    if(EC_FALSE == c_file_flush(fd, offset, osize, (uint8_t *)&(CPGD_HDR_PAGE_ACTUAL_USED_SIZE(cpgd_hdr))))
    {
        sys_log(LOGSTDOUT, "error:cpgd_hdr_flush: flush CPGD_HDR_PAGE_ACTUAL_USED_SIZE at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }

    /*skip rsvd2*/
    if(EC_FALSE == c_file_pad(fd, offset, CPGD_HDR_PAD_SIZE, FILE_PAD_CHAR))
    {
        sys_log(LOGSTDOUT, "error:cpgd_hdr_flush: flush CPGD_HDR_PAD at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }

    DEBUG(CPGD_ASSERT(sizeof(CPGD_HDR) == (*offset) - offset_saved));
   
    return (EC_TRUE);
}

EC_BOOL cpgd_hdr_load(CPGD_HDR *cpgd_hdr, int fd, UINT32 *offset)
{
    UINT32 osize;/*load once size*/

    /*load rbtree pool*/
    if(EC_FALSE == cpgrb_load(CPGD_HDR_CPGRB_POOL(cpgd_hdr), fd, offset))
    {
        sys_log(LOGSTDOUT, "error:cpgd_hdr_load: load CPGD_HDR_CPGRB_POOL at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }

    /*load CPGD_HDR_BLOCK_CPGRB_ROOT_POS_TBL*/
    osize = CPGB_MODEL_NUM * sizeof(uint16_t);
    if(EC_FALSE == c_file_load(fd, offset, osize, (uint8_t *)CPGD_HDR_BLOCK_CPGRB_ROOT_POS_TBL(cpgd_hdr)))
    {
        sys_log(LOGSTDOUT, "error:cpgd_hdr_load: load CPGD_HDR_BLOCK_CPGRB_ROOT_POS_TBL at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }    

    /*skip rsvd1*/
    (*offset) += sizeof(uint16_t);

    /*load CPGD_HDR_ASSIGN_BITMAP*/
    osize = sizeof(uint16_t);
    if(EC_FALSE == c_file_load(fd, offset, osize, (uint8_t *)&(CPGD_HDR_ASSIGN_BITMAP(cpgd_hdr))))
    {
        sys_log(LOGSTDOUT, "error:cpgd_hdr_load: load CPGD_HDR_ASSIGN_BITMAP at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }    

    /*load CPGD_HDR_PAGE_BLOCK_MAX_NUM*/
    osize = sizeof(uint16_t);
    if(EC_FALSE == c_file_load(fd, offset, osize, (uint8_t *)&(CPGD_HDR_PAGE_BLOCK_MAX_NUM(cpgd_hdr))))
    {
        sys_log(LOGSTDOUT, "error:cpgd_hdr_load: load CPGD_HDR_PAGE_BLOCK_MAX_NUM at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }

    /*load CPGD_HDR_PAGE_4K_MAX_NUM*/
    osize = sizeof(uint32_t);
    if(EC_FALSE == c_file_load(fd, offset, osize, (uint8_t *)&(CPGD_HDR_PAGE_4K_MAX_NUM(cpgd_hdr))))
    {
        sys_log(LOGSTDOUT, "error:cpgd_hdr_load: load CPGD_HDR_PAGE_4K_MAX_NUM at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }    

    /*load CPGD_HDR_PAGE_4K_USED_NUM*/
    osize = sizeof(uint32_t);
    if(EC_FALSE == c_file_load(fd, offset, osize, (uint8_t *)&(CPGD_HDR_PAGE_4K_USED_NUM(cpgd_hdr))))
    {
        sys_log(LOGSTDOUT, "error:cpgd_hdr_load: load CPGD_HDR_PAGE_4K_USED_NUM at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }

    /*load CPGD_HDR_PAGE_ACTUAL_USED_SIZE*/
    osize = sizeof(uint64_t);
    if(EC_FALSE == c_file_load(fd, offset, osize, (uint8_t *)&(CPGD_HDR_PAGE_ACTUAL_USED_SIZE(cpgd_hdr))))
    {
        sys_log(LOGSTDOUT, "error:cpgd_hdr_load: load CPGD_HDR_PAGE_ACTUAL_USED_SIZE at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }

    /*skip rsvd2*/
    (*offset) += CPGD_HDR_PAD_SIZE;

    return (EC_TRUE);
}

CPGD *cpgd_new(const uint8_t *cpgd_fname, const uint16_t block_num)
{
    CPGD      *cpgd;
    uint16_t   block_no;

    if(CPGD_MAX_BLOCK_NUM < block_num)
    {
        sys_log(LOGSTDOUT, "error:cpgd_new: block_num %u overflow\n", block_num);
        return (NULL_PTR);
    }
    
    if(EC_TRUE == c_file_access((const char *)cpgd_fname, F_OK))
    {
        sys_log(LOGSTDOUT, "error:cpgd_new: %s already exist\n", cpgd_fname);
        return (NULL_PTR);
    }

    alloc_static_mem(MD_TBD, 0, MM_CPGD, &cpgd, LOC_CPGD_0001);
    if(NULL_PTR == cpgd)
    {
        sys_log(LOGSTDOUT, "error:cpgd_new:malloc cpgd failed\n");
        return (NULL_PTR);
    }

    cpgd_init(cpgd);

    CPGD_FNAME(cpgd) = (uint8_t *)c_str_dup((char *)cpgd_fname);
    if(NULL_PTR == CPGD_FNAME(cpgd))
    {
        sys_log(LOGSTDOUT, "error:cpgd_new:str dup %s failed\n", cpgd_fname);
        cpgd_free(cpgd);
        return (NULL_PTR);
    }

    CPGD_FD(cpgd) = c_file_open((const char *)cpgd_fname, O_RDWR | O_SYNC | O_CREAT, 0666);
    if(ERR_FD == CPGD_FD(cpgd))
    {
        sys_log(LOGSTDOUT, "error:cpgd_new: create %s failed\n", cpgd_fname);
        cpgd_free(cpgd);
        return (NULL_PTR);
    }

    sys_log(LOGSTDOUT, "[DEBUG] cpgd_new: sizeof(CPGD_HDR) %u, block_num %u, sizeof(CPGB) %u, sizeof(off_t) = %u\n", 
                        sizeof(CPGD_HDR), block_num, sizeof(CPGB), sizeof(off_t));

    CPGD_FSIZE(cpgd) = sizeof(CPGD_HDR) + block_num * sizeof(CPGB);
    if(EC_FALSE == c_file_truncate(CPGD_FD(cpgd), CPGD_FSIZE(cpgd)))
    {
        sys_log(LOGSTDOUT, "error:cpgd_new: truncate %s to %u bytes failed\n", cpgd_fname, CPGD_FSIZE(cpgd));
        cpgd_free(cpgd);
        return (NULL_PTR);
    }

    CPGD_HEADER(cpgd) = cpgd_hdr_new(cpgd, block_num);
    if(NULL_PTR == CPGD_HEADER(cpgd))
    {
        sys_log(LOGSTDOUT, "error:cpgd_new: new cpgd header of file %s failed\n", cpgd_fname);
        cpgd_free(cpgd);
        return (NULL_PTR);
    }

    /*init blocks*/
    for(block_no = 0; block_no < block_num; block_no ++)
    {
        CPGD_BLOCK_CPGB(cpgd, block_no) = (CPGB *)(((void *)CPGD_HEADER(cpgd)) + sizeof(CPGD_HDR) + block_no * sizeof(CPGB));
        cpgb_init(CPGD_BLOCK_CPGB(cpgd, block_no), CPGD_BLOCK_PAGE_MODEL);
        cpgd_add_block(cpgd, block_no, CPGD_BLOCK_PAGE_MODEL);

        if(0 == ((block_no + 1) % 1000))
        {
            sys_log(LOGSTDOUT, "info:cpgd_new: init block %u - %u of file %s done\n", block_no - 999, block_no, cpgd_fname);
        }
    }
    sys_log(LOGSTDOUT, "info:cpgd_new: init %u blocks of file %s done\n", block_num, cpgd_fname);

    return (cpgd);
}

EC_BOOL cpgd_free(CPGD *cpgd)
{
    if(NULL_PTR != cpgd)
    {
        UINT32 block_num;
        UINT32 block_no;
        
        /*clean blocks*/
        block_num = CPGD_PAGE_BLOCK_MAX_NUM(cpgd);
        for(block_no = 0; block_no < block_num; block_no ++)
        {
            CPGD_BLOCK_CPGB(cpgd, block_no) = NULL_PTR;
        } 
    
        cpgd_hdr_free(cpgd);

        if(ERR_FD != CPGD_FD(cpgd))
        {
            c_file_close(CPGD_FD(cpgd));
            CPGD_FD(cpgd) = ERR_FD;
        }

        if(NULL_PTR != CPGD_FNAME(cpgd))
        {
            safe_free(CPGD_FNAME(cpgd), LOC_CPGD_0002);
            CPGD_FNAME(cpgd) = NULL_PTR;
        }

        free_static_mem(MD_TBD, 0, MM_CPGD, cpgd, LOC_CPGD_0003);
    }

    return (EC_TRUE);
}

EC_BOOL cpgd_exist(const uint8_t *cpgd_fname)
{
    return c_file_access((const char *)cpgd_fname, F_OK);
}

EC_BOOL cpgd_rmv(const uint8_t *cpgd_fname)
{
    return c_file_unlink((const char *)cpgd_fname);
}

CPGD *cpgd_open(const uint8_t *cpgd_fname)
{
    CPGD      *cpgd;
    
    uint16_t  block_num;
    uint16_t  block_no;

    UINT32    fsize;
    
    alloc_static_mem(MD_TBD, 0, MM_CPGD, &cpgd, LOC_CPGD_0004);
    if(NULL_PTR == cpgd)
    {
        sys_log(LOGSTDOUT, "error:cpgd_open:malloc cpgd failed\n");
        return (NULL_PTR);
    }

    cpgd_init(cpgd);

    CPGD_FNAME(cpgd) = (uint8_t *)c_str_dup((const char *)cpgd_fname);
    if(NULL_PTR == CPGD_FNAME(cpgd))
    {
        sys_log(LOGSTDOUT, "error:cpgd_open:str dup %s failed\n", cpgd_fname);
        cpgd_close(cpgd);
        return (NULL_PTR);
    }

    CPGD_FD(cpgd) = c_file_open((const char *)cpgd_fname, O_RDWR | O_SYNC , 0666);
    if(ERR_FD == CPGD_FD(cpgd))
    {
        sys_log(LOGSTDOUT, "error:cpgd_open: open %s failed\n", cpgd_fname);
        cpgd_close(cpgd);
        return (NULL_PTR);
    }

    if(EC_FALSE == c_file_size(CPGD_FD(cpgd), &fsize))
    {
        sys_log(LOGSTDOUT, "error:cpgd_open: get size of %s failed\n", cpgd_fname);
        cpgd_close(cpgd);
        return (NULL_PTR);
    }
    CPGD_FSIZE(cpgd) = (uint32_t)fsize;

    CPGD_HEADER(cpgd) = cpgd_hdr_open(cpgd);
    if(NULL_PTR == CPGD_HEADER(cpgd))
    {
        sys_log(LOGSTDOUT, "error:cpgd_open: open cpgd header of file %s failed\n", cpgd_fname);
        cpgd_close(cpgd);
        return (NULL_PTR);
    }

    /*init blocks*/
    block_num = CPGD_PAGE_BLOCK_MAX_NUM(cpgd);
    for(block_no = 0; block_no < block_num; block_no ++)
    {
        CPGD_BLOCK_CPGB(cpgd, block_no) = (CPGB *)(((void *)CPGD_HEADER(cpgd)) + sizeof(CPGD_HDR) + block_no * sizeof(CPGB));
    }

    return (cpgd);
}

EC_BOOL cpgd_close(CPGD *cpgd)
{
    if(NULL_PTR != cpgd)
    {
        UINT32 block_num;
        UINT32 block_no;
        
        /*clean blocks*/
        if(NULL_PTR != CPGD_HEADER(cpgd))
        {
            block_num = CPGD_PAGE_BLOCK_MAX_NUM(cpgd);
            for(block_no = 0; block_no < block_num; block_no ++)
            {
                CPGD_BLOCK_CPGB(cpgd, block_no) = NULL_PTR;
            } 
        }
    
        cpgd_hdr_close(cpgd);

        if(ERR_FD != CPGD_FD(cpgd))
        {
            c_file_close(CPGD_FD(cpgd));
            CPGD_FD(cpgd) = ERR_FD;
        }

        if(NULL_PTR != CPGD_FNAME(cpgd))
        {
            safe_free(CPGD_FNAME(cpgd), LOC_CPGD_0005);
            CPGD_FNAME(cpgd) = NULL_PTR;
        }

        free_static_mem(MD_TBD, 0, MM_CPGD, cpgd, LOC_CPGD_0006);
    }
    return (EC_TRUE);
}

EC_BOOL cpgd_sync(CPGD *cpgd)
{
    if(NULL_PTR != cpgd)
    {        
        cpgd_hdr_sync(cpgd);
    }
    return (EC_TRUE);
}
 
/* one disk = 1TB */
EC_BOOL cpgd_init(CPGD *cpgd)
{
    uint16_t block_no;

    CPGD_FD(cpgd)    = ERR_FD;
    CPGD_FNAME(cpgd) = NULL_PTR;
    CPGD_FSIZE(cpgd) = 0;
    CPGD_HEADER(cpgd)= NULL_PTR;

    for(block_no = 0; block_no < CPGD_MAX_BLOCK_NUM; block_no ++)
    {
        CPGD_BLOCK_CPGB(cpgd, block_no) = NULL_PTR;
    }
    return (EC_TRUE);
}

/*note: cpgd_clean is for not applying mmap*/
void cpgd_clean(CPGD *cpgd)
{
    uint16_t page_model;
    uint16_t block_no;

    if(ERR_FD != CPGD_FD(cpgd))
    {
        c_file_close(CPGD_FD(cpgd));
        CPGD_FD(cpgd) = ERR_FD;
    }

    if(NULL_PTR != CPGD_FNAME(cpgd))
    {
        safe_free(CPGD_FNAME(cpgd), LOC_CPGD_0007);
        CPGD_FNAME(cpgd) = NULL_PTR;
    }

    if(NULL_PTR == CPGD_HEADER(cpgd))
    {
        return;
    }

    cpgrb_pool_clean(CPGD_PAGE_BLOCK_CPGRB_POOL(cpgd));

    for(page_model = 0; CPGB_MODEL_NUM > page_model; page_model ++)
    {
        CPGD_PAGE_MODEL_BLOCK_CPGRB_ROOT_POS(cpgd, page_model) = CPGRB_ERR_POS;
    }

    for(block_no = 0; block_no < CPGD_PAGE_BLOCK_MAX_NUM(cpgd); block_no ++)
    {
        if(NULL_PTR != CPGD_BLOCK_CPGB(cpgd, block_no))
        {           
            safe_free(CPGD_BLOCK_CPGB(cpgd, block_no), LOC_CPGD_0008);
            CPGD_BLOCK_CPGB(cpgd, block_no) = NULL_PTR;
        }
    }  
    CPGD_PAGE_BLOCK_MAX_NUM(cpgd)           = 0;

    CPGD_PAGE_MODEL_ASSIGN_BITMAP(cpgd)     = 0;    
    CPGD_PAGE_4K_MAX_NUM(cpgd)              = 0;
    CPGD_PAGE_4K_USED_NUM(cpgd)             = 0;
    CPGD_PAGE_ACTUAL_USED_SIZE(cpgd)        = 0;

    safe_free(CPGD_HEADER(cpgd), LOC_CPGD_0009);
    CPGD_HEADER(cpgd) = NULL_PTR;
    
    return;    
}

/*add one free block into pool*/
EC_BOOL cpgd_add_block(CPGD *cpgd, const uint16_t block_no, const uint16_t page_model)
{    
    if(CPGD_PAGE_BLOCK_MAX_NUM(cpgd) <= block_no)
    {
        sys_log(LOGSTDOUT, "error:cpgd_add_block: block_no %u overflow where block max num is %u\n", block_no, CPGD_PAGE_BLOCK_MAX_NUM(cpgd));
        return (EC_FALSE);
    }

    /*insert block_no to rbtree*/
    if(CPGRB_ERR_POS == cpgrb_tree_insert_data(CPGD_PAGE_BLOCK_CPGRB_POOL(cpgd), &(CPGD_PAGE_MODEL_BLOCK_CPGRB_ROOT_POS(cpgd, page_model)), block_no))
    {
        sys_log(LOGSTDERR, "error:cpgd_add_block: add block_no %u to rbtree of page model %u failed\n", block_no, page_model);
        return (EC_FALSE);
    }

    /*set assignment bitmap*/
    /*set bits of page_model, page_model + 1, ... page_4k_model, the highest bit is for 2k-page which is not supported,clear it!*/
    CPGD_PAGE_MODEL_ASSIGN_BITMAP(cpgd) |= (uint16_t)(~((1 << page_model) - 1)) & CPGB_MODEL_MASK_ALL;

    return (EC_TRUE);
}

/*del one free block from pool*/
EC_BOOL cpgd_del_block(CPGD *cpgd, const uint16_t block_no, const uint16_t page_model)
{
    /*del block_no from rbtree*/
    if(CPGRB_ERR_POS == cpgrb_tree_delete_data(CPGD_PAGE_BLOCK_CPGRB_POOL(cpgd), &(CPGD_PAGE_MODEL_BLOCK_CPGRB_ROOT_POS(cpgd, page_model)), block_no))
    {
        sys_log(LOGSTDERR, "error:cpgd_del_block: del block_no %u from rbtree of page model %u failed\n", block_no, page_model);
        return (EC_FALSE);
    }

    /*clear assignment bitmap if necessary*/
    if(0 == (CPGD_PAGE_MODEL_ASSIGN_BITMAP(cpgd) & (uint16_t)((1 << page_model) - 1)))/*upper page-model has no page*/
    {
        uint16_t page_model_t;
        
        page_model_t = page_model;            
        while(CPGB_MODEL_NUM > page_model_t
           && EC_TRUE == cpgrb_tree_is_empty(CPGD_PAGE_BLOCK_CPGRB_POOL(cpgd), CPGD_PAGE_MODEL_BLOCK_CPGRB_ROOT_POS(cpgd, page_model_t))/*this page-model is empty*/
        )
        {
            CPGD_PAGE_MODEL_ASSIGN_BITMAP(cpgd) &= (uint16_t)~(1 << page_model_t);/*clear bit*/
            page_model_t ++;
        }
    }

    return (EC_TRUE);
}

/*page_model is IN & OUT parameter*/
static EC_BOOL __cpgd_assign_block(CPGD *cpgd, uint16_t *page_model, uint16_t *block_no)
{
    uint16_t block_no_t;
    uint16_t page_model_t;
    uint16_t mask;

    page_model_t = *page_model;

    mask = (uint16_t)((1 << (page_model_t + 1)) - 1);
    if(0 == (CPGD_PAGE_MODEL_ASSIGN_BITMAP(cpgd) & mask))
    {
        sys_log(LOGSTDERR, "error:__cpgd_assign_block: page_model = %u where 0 == bitmap %x & mask %u indicates page is not available\n", 
                           page_model_t, CPGD_PAGE_MODEL_ASSIGN_BITMAP(cpgd), mask);
        return (EC_FALSE);
    }
    
    while(CPGB_MODEL_NUM > page_model_t 
       && EC_TRUE == cpgrb_tree_is_empty(CPGD_PAGE_BLOCK_CPGRB_POOL(cpgd), CPGD_PAGE_MODEL_BLOCK_CPGRB_ROOT_POS(cpgd, page_model_t))
       )
    {
        page_model_t --;
    }

    if(CPGB_MODEL_NUM <= page_model_t)
    {
        sys_log(LOGSTDERR, "error:__cpgd_assign_block: no free block available from page model %u\n", *page_model);
        return (EC_FALSE);
    }

    block_no_t = __cpgd_page_model_first_block(cpgd, page_model_t);
    if(CPGRB_ERR_POS == block_no_t)
    {
        sys_log(LOGSTDERR, "error:__cpgd_assign_block: no free block in page model %u\n", page_model_t);
        return (EC_FALSE);
    }  

    (*page_model) = page_model_t;
    (*block_no)   = block_no_t;
    
    return (EC_TRUE);
}

EC_BOOL cpgd_new_space(CPGD *cpgd, const uint32_t size, uint16_t *block_no, uint16_t *page_4k_no)
{
    CPGB    *cpgb;
    
    uint16_t page_4k_num_need;
    uint16_t page_model;
    uint16_t page_model_t;
    uint16_t e;
    uint16_t t;
    uint16_t page_4k_no_t;/*the page No. in certain page model*/
  
    uint16_t block_no_t;
    
    uint16_t pgb_assign_bitmap_old;
    uint16_t pgb_assign_bitmap_new;    

    CPGD_ASSERT(0 < size);

    if(CPGB_CACHE_MAX_BYTE_SIZE < size)
    {
        sys_log(LOGSTDERR, "error:cpgd_new_space: the expected size %u overflow\n", size);
        return (EC_FALSE);
    }

    page_4k_num_need = (uint16_t)((size + CPGB_PAGE_4K_BYTE_SIZE - 1) >> CPGB_PAGE_4K_BIT_SIZE);
    sys_log(LOGSTDNULL, "[DEBUG] cpgd_new_space: size = %u ==> page_4k_num_need = %u\n", size, page_4k_num_need);

    /*find a page model which can accept the page_4k_num_need 4k-pages */
    /*and then split the left space into page model with smaller size  */ 

    CPGD_ASSERT(0 == (CPGB_PAGE_4K_ERR_HI_BITS_MASK & page_4k_num_need));
    
    /*check bits of page_4k_num_need and determine the page_model*/
    e = CPGB_PAGE_4K_HI_BIT_MASK;
    for(t = page_4k_num_need, page_model = 0; 0 == (t & e); t <<= 1, page_model ++)
    {
        /*do nothing*/
    }
    sys_log(LOGSTDNULL, "[DEBUG] cpgd_new_space: t = 0x%x, page_model = %u, e = 0x%x, t << 1 is 0x%x\n", t, page_model, e, (t << 1));

    if(CPGB_PAGE_4K_LO_BITS_MASK & t)
    {
        page_model --;/*upgrade page_model one level*/
    }

    sys_log(LOGSTDNULL, "[DEBUG] cpgd_new_space: page_4k_num_need = %u ==> page_model = %u (has %u 4k-pages )\n", 
                       page_4k_num_need, page_model, (uint16_t)(1 << (CPGB_MODEL_NUM - 1 - page_model)));


    if(EC_FALSE == __cpgd_assign_block(cpgd, &page_model, &block_no_t))
    {
        sys_log(LOGSTDERR, "error:cpgd_new_space: assign one block from page model %u failed\n", page_model);
        return (EC_FALSE);
    }

    cpgb = CPGD_BLOCK_NODE(cpgd, block_no_t);
    pgb_assign_bitmap_old = CPGB_PAGE_MODEL_ASSIGN_BITMAP(cpgb);    
    
    if(EC_FALSE == cpgb_new_space(cpgb, size, &page_4k_no_t))
    {
        sys_log(LOGSTDERR, "error:cpgd_new_space: free one page from page model %u block %u failed\n", page_model, block_no_t);
        return (EC_FALSE);
    }

    pgb_assign_bitmap_new = CPGB_PAGE_MODEL_ASSIGN_BITMAP(cpgb);

    sys_log(LOGSTDOUT, "[DEBUG] cpgd_new_space: block_no_t %u: pgb bitmap %x => %x\n", block_no_t, pgb_assign_bitmap_old, pgb_assign_bitmap_new);

    /*pgb_assign_bitmap changes may make pgd_assign_bitmap changes*/
    if(pgb_assign_bitmap_new != pgb_assign_bitmap_old)
    {        
        sys_log(LOGSTDOUT, "[DEBUG] cpgd_new_space: before delete block_no_t %u: pgb bitmap %s, pgd assign bitmap %s\n", 
                            block_no_t,
                            c_uint16_to_bin_str(CPGB_PAGE_MODEL_ASSIGN_BITMAP(cpgb)), 
                            c_uint16_to_bin_str(CPGD_PAGE_MODEL_ASSIGN_BITMAP(cpgd)));

        cpgd_del_block(cpgd, block_no_t, page_model);
        
        sys_log(LOGSTDOUT, "[DEBUG] cpgd_new_space: after  delete block_no_t %u: pgb bitmap %s, pgd assign bitmap %s\n", 
                            block_no_t, 
                            c_uint16_to_bin_str(CPGB_PAGE_MODEL_ASSIGN_BITMAP(cpgb)), 
                            c_uint16_to_bin_str(CPGD_PAGE_MODEL_ASSIGN_BITMAP(cpgd)));

        if(EC_FALSE == cpgb_is_full(cpgb))
        {
            page_model_t = page_model;
            while(CPGB_MODEL_NUM > page_model_t 
               && 0 == (pgb_assign_bitmap_new & (uint16_t)(1 << page_model_t))
               )
            {
                 page_model_t ++;
            }
            CPGD_ASSERT(CPGB_MODEL_NUM > page_model_t);
            sys_log(LOGSTDOUT, "[DEBUG] cpgd_new_space: page_model %u, page_model_t %u\n", page_model, page_model_t);
            cpgd_add_block(cpgd, block_no_t, page_model_t);
            sys_log(LOGSTDOUT, "[DEBUG] cpgd_new_space: block_no_t %u: pgb bitmap %s, pgd assign bitmap %s\n", 
                                block_no_t, 
                                c_uint16_to_bin_str(CPGB_PAGE_MODEL_ASSIGN_BITMAP(cpgb)), 
                                c_uint16_to_bin_str(CPGD_PAGE_MODEL_ASSIGN_BITMAP(cpgd)));
        }
        else
        {
            /*do nothing*/
        }
    }
    
    (*block_no)   = block_no_t;
    (*page_4k_no) = page_4k_no_t;

    CPGD_PAGE_4K_USED_NUM(cpgd)      += page_4k_num_need;
    CPGD_PAGE_ACTUAL_USED_SIZE(cpgd) += size;

    CPGD_ASSERT(EC_TRUE == cpgd_check(cpgd));    
    
    sys_log(LOGSTDNULL, "[DEBUG] cpgd_new_space: pgd_page_4k_used_num %u due to increment %u\n", 
                        CPGD_PAGE_4K_USED_NUM(cpgd), page_4k_num_need);
    sys_log(LOGSTDNULL, "[DEBUG] cpgd_new_space: pgd_actual_used_size %u due to increment %u\n", 
                        CPGD_PAGE_ACTUAL_USED_SIZE(cpgd), size);

    return (EC_TRUE);
}

EC_BOOL cpgd_free_space(CPGD *cpgd, const uint16_t block_no, const uint16_t page_4k_no, const uint32_t size)
{
    CPGB    *cpgb;
    
    uint16_t page_4k_num_used;    
    
    uint16_t pgb_assign_bitmap_old;
    uint16_t pgb_assign_bitmap_new;   

    CPGD_ASSERT(0 < size);
    
    if(CPGB_CACHE_MAX_BYTE_SIZE < size)
    {
        sys_log(LOGSTDERR, "error:cpgd_free_space: invalid size %u due to overflow\n", size);
        return (EC_FALSE);
    }

    cpgb = CPGD_BLOCK_NODE(cpgd, block_no);
    pgb_assign_bitmap_old = CPGB_PAGE_MODEL_ASSIGN_BITMAP(cpgb);
    
    if(EC_FALSE == cpgb_free_space(cpgb, page_4k_no, size))
    {
        sys_log(LOGSTDOUT, "error:cpgd_free_space: block_no %u free space of page_4k_no %u, size %u failed\n", 
                           block_no, page_4k_no, size);
        return (EC_FALSE);
    }

    pgb_assign_bitmap_new = CPGB_PAGE_MODEL_ASSIGN_BITMAP(cpgb);

    if(pgb_assign_bitmap_new != pgb_assign_bitmap_old)
    {
        uint16_t page_model_old;
        uint16_t page_model_new;     
        
        page_model_old = __cpgd_page_model_get(cpgd, pgb_assign_bitmap_old);
        page_model_new = __cpgd_page_model_get(cpgd, pgb_assign_bitmap_new);

        if(CPGB_MODEL_NUM > page_model_old)
        {
            cpgd_del_block(cpgd, block_no, page_model_old);
        }
        cpgd_add_block(cpgd, block_no, page_model_new);
    }

    page_4k_num_used = (uint16_t)((size + CPGB_PAGE_4K_BYTE_SIZE - 1) >> CPGB_PAGE_4K_BIT_SIZE);
 
    CPGD_PAGE_4K_USED_NUM(cpgd)      -= page_4k_num_used;
    CPGD_PAGE_ACTUAL_USED_SIZE(cpgd) -= size;
    
    sys_log(LOGSTDNULL, "[DEBUG] cpgd_free_space: pgd_page_4k_used_num %u due to decrement %u\n", 
                        CPGD_PAGE_4K_USED_NUM(cpgd), page_4k_num_used);
    sys_log(LOGSTDNULL, "[DEBUG] cpgd_free_space: pgd_actual_used_size %u due to decrement %u\n", 
                        CPGD_PAGE_ACTUAL_USED_SIZE(cpgd), size);

    return (EC_TRUE);    
}

EC_BOOL cpgd_is_full(const CPGD *cpgd)
{
    if(CPGD_PAGE_4K_USED_NUM(cpgd) == CPGD_PAGE_4K_MAX_NUM(cpgd))
    {
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

EC_BOOL cpgd_is_empty(const CPGD *cpgd)
{
    if(0 == CPGD_PAGE_4K_USED_NUM(cpgd) && 0 < CPGD_PAGE_4K_MAX_NUM(cpgd))
    {
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

/*compute cpgd current page model support up to*/
uint16_t cpgd_page_model(const CPGD *cpgd)
{
    uint16_t page_model;
    uint16_t pgd_assign_bitmap;
    uint16_t e;

    pgd_assign_bitmap = CPGD_PAGE_MODEL_ASSIGN_BITMAP(cpgd);
    for(page_model = 0, e = 1; CPGB_MODEL_NUM > page_model && 0 == (pgd_assign_bitmap & e); e <<= 1, page_model ++)
    {
        /*do nothing*/
    }

    sys_log(LOGSTDOUT, "[DEBUG] cpgd_page_model: cpgd %p: assign bitmap %s ==> page_model %u\n", 
                       cpgd, c_uint16_to_bin_str(pgd_assign_bitmap), page_model);        

    return (page_model);                       
}

EC_BOOL cpgd_flush_size(const CPGD *cpgd, UINT32 *size)
{
    uint16_t block_no;

    cpgd_hdr_flush_size(CPGD_HEADER(cpgd), size);    

    for(block_no = 0; block_no < CPGD_PAGE_BLOCK_MAX_NUM(cpgd); block_no ++)
    {
        cpgb_flush_size(CPGD_BLOCK_NODE(cpgd, block_no), size);
    }
    return (EC_TRUE);
}

EC_BOOL cpgd_flush(const CPGD *cpgd, int fd, UINT32 *offset)
{
    uint16_t block_no;

    /*flush CPGD_HEADER*/
    if(EC_FALSE == cpgd_hdr_flush(CPGD_HEADER(cpgd), fd, offset))
    {
        sys_log(LOGSTDOUT, "error:cpgd_flush: flush CPGD_HEADER at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }

    /*flush CPGD_BLOCK_NODE table*/
    for(block_no = 0; block_no < CPGD_PAGE_BLOCK_MAX_NUM(cpgd); block_no ++)
    {
        if(EC_FALSE == cpgb_flush(CPGD_BLOCK_NODE(cpgd, block_no), fd, offset))
        {
            sys_log(LOGSTDOUT, "error:cpgd_flush: flush CPGD_BLOCK_NODE of block_no %u at offset %u of fd %d failed\n", 
                                block_no, (*offset), fd);
            return (EC_FALSE);
        }
    }
    
    return (EC_TRUE);
}

EC_BOOL cpgd_load(CPGD *cpgd, int fd, UINT32 *offset)
{
    uint16_t block_no;

    if(NULL_PTR == CPGD_HEADER(cpgd))
    {
        CPGD_HEADER(cpgd) = safe_malloc(sizeof(CPGD_HDR), LOC_CPGD_0010);
        if(NULL_PTR == CPGD_HEADER(cpgd))
        {
            sys_log(LOGSTDOUT, "error:cpgd_load: malloc CPGD_HDR failed\n");
            return (EC_FALSE);
        }
    }

    /*load rbtree pool*/
    if(EC_FALSE == cpgd_hdr_load(CPGD_HEADER(cpgd), fd, offset))
    {
        sys_log(LOGSTDOUT, "error:cpgd_load: load CPGD_HEADER at offset %u of fd %d failed\n", (*offset), fd);
        return (EC_FALSE);
    }

    /*load CPGD_BLOCK_NODE table*/
    for(block_no = 0; block_no < CPGD_PAGE_BLOCK_MAX_NUM(cpgd); block_no ++)
    {
        if(NULL_PTR == CPGD_BLOCK_NODE(cpgd, block_no))
        {
            CPGD_BLOCK_CPGB(cpgd, block_no) = safe_malloc(sizeof(CPGB), LOC_CPGD_0011);
            if(NULL_PTR == CPGD_BLOCK_CPGB(cpgd, block_no))
            {
                sys_log(LOGSTDOUT, "error:cpgd_load: malloc block %u failed\n", block_no);
                return (EC_FALSE);
            }
        }
        if(EC_FALSE == cpgb_load(CPGD_BLOCK_CPGB(cpgd, block_no), fd, offset))
        {
            sys_log(LOGSTDOUT, "error:cpgd_load: load CPGD_BLOCK_NODE of block_no %u at offset %u of fd %d failed\n", 
                                block_no, (*offset), fd);
            return (EC_FALSE);
        }
    }    
    return (EC_TRUE);
}

EC_BOOL cpgd_check(const CPGD *cpgd)
{
    uint16_t  pgd_assign_bitmap;
    uint16_t  pgb_assign_bitmap;/*all pgb's bitmap*/
    uint16_t  block_no;
    uint16_t  block_num;

    uint64_t  pgd_actual_used_size;
    uint64_t  pgb_actual_used_size;/*all pgb's used size*/

    uint32_t  pgd_page_4k_max_num;
    uint32_t  pgb_page_4k_max_num;/*all pgb's 4k-page max num*/

    uint32_t  pgd_page_4k_used_num;
    uint32_t  pgb_page_4k_used_num;/*all pgb's 4k-page used num*/

    pgd_assign_bitmap    = CPGD_PAGE_MODEL_ASSIGN_BITMAP(cpgd);
    pgd_actual_used_size = CPGD_PAGE_ACTUAL_USED_SIZE(cpgd);
    pgd_page_4k_max_num  = CPGD_PAGE_4K_MAX_NUM(cpgd);
    pgd_page_4k_used_num = CPGD_PAGE_4K_USED_NUM(cpgd);
    block_num = CPGD_PAGE_BLOCK_MAX_NUM(cpgd);

    pgb_assign_bitmap    = 0;
    pgb_actual_used_size = 0;
    pgb_page_4k_max_num  = 0;
    pgb_page_4k_used_num = 0;
    
    for(block_no = 0; block_no < block_num; block_no ++)
    {
        pgb_assign_bitmap    |= CPGB_PAGE_MODEL_ASSIGN_BITMAP(CPGD_BLOCK_NODE(cpgd, block_no));
        pgb_actual_used_size += CPGB_PAGE_ACTUAL_USED_SIZE(CPGD_BLOCK_NODE(cpgd, block_no));
        pgb_page_4k_max_num  += CPGB_PAGE_4K_MAX_NUM(CPGD_BLOCK_NODE(cpgd, block_no));
        pgb_page_4k_used_num += CPGB_PAGE_4K_USED_NUM(CPGD_BLOCK_NODE(cpgd, block_no));
    }

    if(pgd_assign_bitmap != pgb_assign_bitmap)
    {
        sys_log(LOGSTDOUT, "error:cpgd_check: inconsistent bitmap: pgd_assign_bitmap = %s, pgb_assign_bitmap = %s\n",
                           c_uint16_to_bin_str(pgd_assign_bitmap), c_uint16_to_bin_str(pgb_assign_bitmap));
        return (EC_FALSE);
    }

    if(pgd_actual_used_size != pgb_actual_used_size)
    {
        sys_log(LOGSTDOUT, "error:cpgd_check: inconsistent actual used size: pgd_actual_used_size = %llu, pgb_actual_used_size = %llu\n",
                            pgd_actual_used_size, pgb_actual_used_size);
        return (EC_FALSE);
    }

    if(pgd_page_4k_max_num != pgb_page_4k_max_num)
    {
        sys_log(LOGSTDOUT, "error:cpgd_check: inconsistent 4k-page max num: pgd_page_4k_max_num = %u, pgb_page_4k_max_num = %u\n", 
                            pgd_page_4k_max_num, pgb_page_4k_max_num);
        return (EC_FALSE);
    }

    if(pgd_page_4k_used_num != pgb_page_4k_used_num)
    {
        sys_log(LOGSTDOUT, "error:cpgd_check: inconsistent 4k-page used num: pgd_page_4k_used_num = %u, pgb_page_4k_used_num = %u\n", 
                            pgd_page_4k_used_num, pgb_page_4k_used_num);
        return (EC_FALSE);
    }    
    
    /*check block table*/
    for(block_no = 0; block_no < CPGD_PAGE_BLOCK_MAX_NUM(cpgd); block_no ++)
    {
        if(EC_FALSE == cpgb_check(CPGD_BLOCK_NODE(cpgd, block_no)))
        {
            sys_log(LOGSTDOUT, "error:cpgd_check: check CPGD_BLOCK_NODE of block_no %u failed\n", block_no);
            return (EC_FALSE);
        }
    }
    sys_log(LOGSTDOUT, "cpgd_check: cpgd %p check passed\n", cpgd);
    return (EC_TRUE);
}

void cpgd_print(LOG *log, const CPGD *cpgd)
{
    uint16_t  page_model;
    
    REAL      used_size;
    REAL      occupied_size;
    REAL      ratio;

    CPGD_ASSERT(NULL_PTR != cpgd);

    //cpgrb_pool_print(log, CPGD_PAGE_BLOCK_CPGRB_POOL(cpgd));

    if(0)
    {
        for(page_model = 0; CPGB_MODEL_NUM > page_model; page_model ++)
        {
            //cpgrb_tree_print(log, CPGD_PAGE_BLOCK_CPGRB_POOL(cpgd), CPGD_PAGE_MODEL_BLOCK_CPGRB_ROOT_POS(cpgd, page_model));
            //sys_log(log, "----------------------------------------------------------\n");
            sys_log(log, "cpgd_print: page_model %u, block root_pos %u\n", 
                         page_model, 
                         CPGD_PAGE_MODEL_BLOCK_CPGRB_ROOT_POS(cpgd, page_model));
        }
    }
    used_size     = (0.0 + CPGD_PAGE_ACTUAL_USED_SIZE(cpgd));
    occupied_size = (0.0 + (((uint64_t)CPGD_PAGE_4K_USED_NUM(cpgd)) << CPGB_PAGE_4K_BIT_SIZE));
    ratio         = (EC_TRUE == REAL_ISZERO(ERR_MODULE_ID, occupied_size) ? 0.0 : (used_size / occupied_size));
/*
    sys_log(log, "cpgd_print: cpgd %p, ratio %.2f\n", 
                 cpgd, 
                 EC_TRUE == REAL_ISZERO(ERR_MODULE_ID, occupied_size) ? 0.0 : (used_size / occupied_size)
                 );
*/    
    sys_log(log, "cpgd_print: cpgd %p, block num %u, 4k-page max num %u, 4k-page used num %u, used size %llu, ratio %.2f\n", 
                 cpgd, 
                 CPGD_PAGE_BLOCK_MAX_NUM(cpgd), 
                 CPGD_PAGE_4K_MAX_NUM(cpgd),
                 CPGD_PAGE_4K_USED_NUM(cpgd), 
                 CPGD_PAGE_ACTUAL_USED_SIZE(cpgd),
                 ratio
                 );

    sys_log(log, "cpgd_print: cpgd %p, assign bitmap %s \n", 
                 cpgd, 
                 c_uint16_to_bin_str(CPGD_PAGE_MODEL_ASSIGN_BITMAP(cpgd))
                 );

    if(0)
    {
        for(page_model = 0; CPGB_MODEL_NUM > page_model; page_model ++)
        {
            if(CPGD_PAGE_MODEL_ASSIGN_BITMAP(cpgd) & (1 << page_model))
            {
                sys_log(log, "cpgd_print: cpgd %p, model %u has page to assign\n", cpgd, page_model);
            }
            else
            {
                sys_log(log, "cpgd_print: cpgd %p, model %u no  page to assign\n", cpgd, page_model);
            }
        }
    }

    if(0)
    {
        uint16_t  block_no;
        for(block_no = 0; block_no < CPGD_PAGE_BLOCK_MAX_NUM(cpgd); block_no ++)
        {
            sys_log(log, "cpgd_print: block %u is\n", block_no);
            cpgb_print(log, CPGD_BLOCK_NODE(cpgd, block_no));
        }
    }

    return;    
}

/* ---- debug ---- */
EC_BOOL cpgd_debug_cmp(const CPGD *cpgd_1st, const CPGD *cpgd_2nd)
{
    uint16_t page_model;
    uint16_t block_no;

    /*cpgrb pool*/
    if(EC_FALSE == cpgrb_debug_cmp(CPGD_PAGE_BLOCK_CPGRB_POOL(cpgd_1st), CPGD_PAGE_BLOCK_CPGRB_POOL(cpgd_2nd)))
    {
        sys_log(LOGSTDOUT, "error:cpgd_debug_cmp: inconsistent cpgrb pool\n");
        return (EC_FALSE);
    }

    /*root pos*/
    for(page_model = 0; CPGB_MODEL_NUM > page_model; page_model ++ )
    {
        uint16_t root_pos_1st;
        uint16_t root_pos_2nd;

        root_pos_1st = CPGD_PAGE_MODEL_BLOCK_CPGRB_ROOT_POS(cpgd_1st, page_model);
        root_pos_2nd = CPGD_PAGE_MODEL_BLOCK_CPGRB_ROOT_POS(cpgd_2nd, page_model);

        if(root_pos_1st != root_pos_2nd)
        {
            sys_log(LOGSTDERR, "error:cpgd_debug_cmp: inconsistent root_pos: %u != %u at page_model %u\n", 
                                root_pos_1st, root_pos_2nd, page_model);
            return (EC_FALSE);
        }     
    }

    /*assign bitmap*/
    if(CPGD_PAGE_MODEL_ASSIGN_BITMAP(cpgd_1st) != CPGD_PAGE_MODEL_ASSIGN_BITMAP(cpgd_1st))
    {
        sys_log(LOGSTDERR, "error:cpgd_debug_cmp: inconsistent CPGD_PAGE_MODEL_ASSIGN_BITMAP: %u != %u\n", 
                            CPGD_PAGE_MODEL_ASSIGN_BITMAP(cpgd_1st), CPGD_PAGE_MODEL_ASSIGN_BITMAP(cpgd_2nd));
        return (EC_FALSE);
    }

    /*block max num*/
    if(CPGD_PAGE_BLOCK_MAX_NUM(cpgd_1st) != CPGD_PAGE_BLOCK_MAX_NUM(cpgd_1st))
    {
        sys_log(LOGSTDERR, "error:cpgd_debug_cmp: inconsistent CPGD_PAGE_BLOCK_MAX_NUM: %u != %u\n", 
                            CPGD_PAGE_BLOCK_MAX_NUM(cpgd_1st), CPGD_PAGE_BLOCK_MAX_NUM(cpgd_2nd));
        return (EC_FALSE);
    }    
    
    /*4k-page max num*/
    if(CPGD_PAGE_4K_MAX_NUM(cpgd_1st) != CPGD_PAGE_4K_MAX_NUM(cpgd_1st))
    {
        sys_log(LOGSTDERR, "error:cpgd_debug_cmp: inconsistent CPGD_PAGE_4K_MAX_NUM: %u != %u\n", 
                            CPGD_PAGE_4K_MAX_NUM(cpgd_1st), CPGD_PAGE_BLOCK_MAX_NUM(cpgd_2nd));
        return (EC_FALSE);
    }

    /*4k-page used num*/
    if(CPGD_PAGE_4K_USED_NUM(cpgd_1st) != CPGD_PAGE_4K_USED_NUM(cpgd_1st))
    {
        sys_log(LOGSTDERR, "error:cpgd_debug_cmp: inconsistent CPGD_PAGE_4K_USED_NUM: %u != %u\n", 
                            CPGD_PAGE_4K_USED_NUM(cpgd_1st), CPGD_PAGE_4K_USED_NUM(cpgd_2nd));
        return (EC_FALSE);
    }

    /*4k-page actual used bytes num*/
    if(CPGD_PAGE_ACTUAL_USED_SIZE(cpgd_1st) != CPGD_PAGE_ACTUAL_USED_SIZE(cpgd_1st))
    {
        sys_log(LOGSTDERR, "error:cpgd_debug_cmp: inconsistent CPGD_PAGE_ACTUAL_USED_SIZE: %u != %u\n", 
                            CPGD_PAGE_ACTUAL_USED_SIZE(cpgd_1st), CPGD_PAGE_ACTUAL_USED_SIZE(cpgd_2nd));
        return (EC_FALSE);
    }    

    /*block cpgb*/
    for(block_no = 0; block_no < CPGD_PAGE_BLOCK_MAX_NUM(cpgd_1st); block_no ++)
    {
        if(EC_FALSE == cpgb_debug_cmp(CPGD_BLOCK_NODE(cpgd_1st, block_no), CPGD_BLOCK_NODE(cpgd_2nd, block_no)))
        {
            sys_log(LOGSTDOUT, "error:cpgd_debug_cmp: inconsistent CPGD_BLOCK_NODE at block_no %u\n", block_no);
            return (EC_FALSE);
        }
    }
    
    return (EC_TRUE);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

