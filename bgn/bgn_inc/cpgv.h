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

#ifndef    _CPGV_H
#define    _CPGV_H

/*page disk volume, one page = 4KB, one page disk = 2^14 page block = 2^14 * 64MB = 1TB, one page volume = 2^6 * page disk = 64TB*/

#include "type.h"
#include "cpgrb.h"
#include "cpgd.h"

#define CPGV_001TB_DISK_NUM  ((uint16_t)(1 <<  0))
#define CPGV_002TB_DISK_NUM  ((uint16_t)(1 <<  1))
#define CPGV_004TB_DISK_NUM  ((uint16_t)(1 <<  2))
#define CPGV_008TB_DISK_NUM  ((uint16_t)(1 <<  3))
#define CPGV_016TB_DISK_NUM  ((uint16_t)(1 <<  4))
#define CPGV_032TB_DISK_NUM  ((uint16_t)(1 <<  5))
#define CPGV_064TB_DISK_NUM  ((uint16_t)(1 <<  6))

#define CPGV_MAX_DISK_NUM               (CPGV_064TB_DISK_NUM)

#define CPGV_PAGE_DISK_IS_FREE          ((uint8_t) 1)
#define CPGV_PAGE_DISK_IS_NOT_FREE      ((uint8_t) 0)

#define CPGV_HDR_PAD_SIZE                (4040)

typedef struct
{
    CPGRB_POOL   pgv_disk_rb_pool;
    
    uint16_t     pgv_disk_rb_root_pos[ CPGB_MODEL_NUM ];/*root pos of rbtree*/
    uint16_t     rsvd1;
    
    uint16_t     pgv_assign_bitmap; /*when some page model can provide pages or can borrow from upper, set bit to 1*/
    uint16_t     pgv_disk_max_num; /*max disk number */    
    uint32_t     rsvd2;
    
    uint64_t     pgv_page_4k_max_num; /*max pages number */
    uint64_t     pgv_page_4k_used_num;/*used pages number*/
    uint64_t     pgv_actual_used_size;/*actual used bytes*/

    uint8_t      rsvd3[CPGV_HDR_PAD_SIZE];
}CPGV_HDR;/*4k-alignment*/

#define CPGV_HDR_CPGRB_POOL(cpgv_hdr)                           (&((cpgv_hdr)->pgv_disk_rb_pool))
#define CPGV_HDR_DISK_CPGRB_ROOT_POS_TBL(cpgv_hdr)              ((cpgv_hdr)->pgv_disk_rb_root_pos)
#define CPGV_HDR_DISK_CPGRB_ROOT_POS(cpgv_hdr, page_model)      ((cpgv_hdr)->pgv_disk_rb_root_pos[ (page_model) ])
#define CPGV_HDR_ASSIGN_BITMAP(cpgv_hdr)                        ((cpgv_hdr)->pgv_assign_bitmap)
#define CPGV_HDR_PAGE_DISK_MAX_NUM(cpgv_hdr)                    ((cpgv_hdr)->pgv_disk_max_num)
#define CPGV_HDR_PAGE_4K_MAX_NUM(cpgv_hdr)                      ((cpgv_hdr)->pgv_page_4k_max_num)
#define CPGV_HDR_PAGE_4K_USED_NUM(cpgv_hdr)                     ((cpgv_hdr)->pgv_page_4k_used_num)
#define CPGV_HDR_PAGE_ACTUAL_USED_SIZE(cpgv_hdr)                ((cpgv_hdr)->pgv_actual_used_size)


typedef struct
{
    int          pgv_fd;
    int          rsvd1;
    uint8_t     *pgv_fname;
    uint32_t     pgv_fsize;
    uint32_t     rsvd2;
    CPGV_HDR    *pgv_hdr;
    CPGD        *pgv_disk_tbl[CPGV_MAX_DISK_NUM];
}CPGV;

#define CPGV_FD(cpgv)                                            ((cpgv)->pgv_fd)
#define CPGV_FNAME(cpgv)                                         ((cpgv)->pgv_fname)
#define CPGV_FSIZE(cpgv)                                         ((cpgv)->pgv_fsize)
#define CPGV_HEADER(cpgv)                                        ((cpgv)->pgv_hdr)
#define CPGV_PAGE_DISK_CPGRB_POOL(cpgv)                          (CPGV_HDR_CPGRB_POOL(CPGV_HEADER(cpgv)))
#define CPGV_PAGE_MODEL_DISK_CPGRB_ROOT_POS_TBL(cpgv)            (CPGV_HDR_DISK_CPGRB_ROOT_POS_TBL(CPGV_HEADER(cpgv)))
#define CPGV_PAGE_MODEL_DISK_CPGRB_ROOT_POS(cpgv, page_model)    (CPGV_HDR_DISK_CPGRB_ROOT_POS(CPGV_HEADER(cpgv), page_model))
#define CPGV_PAGE_MODEL_ASSIGN_BITMAP(cpgv)                      (CPGV_HDR_ASSIGN_BITMAP(CPGV_HEADER(cpgv)))
#define CPGV_PAGE_DISK_MAX_NUM(cpgv)                             (CPGV_HDR_PAGE_DISK_MAX_NUM(CPGV_HEADER(cpgv)))
#define CPGV_PAGE_4K_MAX_NUM(cpgv)                               (CPGV_HDR_PAGE_4K_MAX_NUM(CPGV_HEADER(cpgv)))
#define CPGV_PAGE_4K_USED_NUM(cpgv)                              (CPGV_HDR_PAGE_4K_USED_NUM(CPGV_HEADER(cpgv)))
#define CPGV_PAGE_ACTUAL_USED_SIZE(cpgv)                         (CPGV_HDR_PAGE_ACTUAL_USED_SIZE(CPGV_HEADER(cpgv)))
#define CPGV_DISK_TBL(cpgv)                                      ((cpgv)->pgv_disk_tbl)
#define CPGV_DISK_CPGD(cpgv, disk_no)                            ((cpgv)->pgv_disk_tbl[ disk_no ])
#define CPGV_DISK_NODE(cpgv, disk_no)                            ((CPGV_MAX_DISK_NUM <= (disk_no)) ? NULL_PTR : CPGV_DISK_CPGD(cpgv, disk_no))


CPGV_HDR *cpgv_hdr_new(CPGV *cpgv, const uint16_t block_num);

EC_BOOL cpgv_hdr_free(CPGV *cpgv);

CPGV_HDR *cpgv_hdr_open(CPGV *cpgv);

EC_BOOL cpgv_hdr_close(CPGV *cpgv);

EC_BOOL cpgv_hdr_sync(CPGV *cpgv);

CPGV *cpgv_new(const uint8_t *cpgv_fname, const uint16_t block_num);

EC_BOOL cpgv_free(CPGV *cpgv);

CPGV *cpgv_open(const uint8_t *cpgv_fname);

EC_BOOL cpgv_close(CPGV *cpgv);

EC_BOOL cpgv_sync(CPGV *cpgv);

/* one disk = 1TB */
EC_BOOL cpgv_init(CPGV *cpgv);

void cpgv_clean(CPGV *cpgv);

/*add one free disk into pool*/
EC_BOOL cpgv_add_disk(CPGV *cpgv, const uint16_t disk_no, const uint16_t page_model);

/*del one free disk from pool*/
EC_BOOL cpgv_del_disk(CPGV *cpgv, const uint16_t disk_no, const uint16_t page_model);

EC_BOOL cpgv_new_space(CPGV *cpgv, const uint32_t size, uint16_t *disk_no, uint16_t *block_no, uint16_t *page_4k_no);

EC_BOOL cpgv_free_space(CPGV *cpgv, const uint16_t disk_no, const uint16_t block_no, const uint16_t page_4k_no, const uint32_t size);

EC_BOOL cpgv_is_full(const CPGV *cpgv);

EC_BOOL cpgv_is_empty(const CPGV *cpgv);

EC_BOOL cpgv_flush_size(const CPGV *cpgv, UINT32 *size);

EC_BOOL cpgv_flush(const CPGV *cpgv, int fd, UINT32 *offset);

EC_BOOL cpgv_load(CPGV *cpgv, int fd, UINT32 *offset);

EC_BOOL cpgv_check(const CPGV *cpgv);

void cpgv_print(LOG *log, const CPGV *cpgv);

CPGV *cpgv_new(const uint8_t *cpgv_dat_file, const uint16_t block_num);

EC_BOOL cpgv_free(CPGV *cpgv);

EC_BOOL cpgv_close(CPGV *cpgv);

CPGV *cpgv_open(const uint8_t *cpgv_dat_file);

/* ---- debug ---- */
EC_BOOL cpgv_debug_cmp(const CPGV *cpgv_1st, const CPGV *cpgv_2nd);


#endif    /* _CPGV_H */

#ifdef __cplusplus
}
#endif/*__cplusplus*/
