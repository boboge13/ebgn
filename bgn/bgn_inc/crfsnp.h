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

#ifndef _CRFSNP_H
#define _CRFSNP_H

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
#include "log.h"

#include "cvector.h"
#include "cmutex.h"
#include "cstring.h"
#include "task.inc"
#include "chashalgo.h"
#include "crfsnprb.h"
#include "crfsnp.inc"
#include "crfsdt.h"

EC_BOOL crfsnp_model_str(const uint8_t crfsnp_model, char **mod_str);

uint32_t crfsnp_model_get(const char *mod_str);

EC_BOOL crfsnp_model_file_size(const uint8_t crfsnp_model, UINT32 *file_size);

EC_BOOL crfsnp_model_item_max_num(const uint8_t crfsnp_model, uint32_t *item_max_num);

EC_BOOL crfsnp_inode_init(CRFSNP_INODE *crfsnp_inode);

EC_BOOL crfsnp_inode_clean(CRFSNP_INODE *crfsnp_inode);

EC_BOOL crfsnp_inode_clone(const CRFSNP_INODE *crfsnp_inode_src, CRFSNP_INODE *crfsnp_inode_des);

void crfsnp_inode_print(LOG *log, const CRFSNP_INODE *crfsnp_inode);

void crfsnp_inode_log_no_lock(LOG *log, const CRFSNP_INODE *crfsnp_inode);

CRFSNP_FNODE *crfsnp_fnode_new();

CRFSNP_FNODE *crfsnp_fnode_make(const CRFSNP_FNODE *crfsnp_fnode_src);

EC_BOOL crfsnp_fnode_init(CRFSNP_FNODE *crfsnp_fnode);

EC_BOOL crfsnp_fnode_clean(CRFSNP_FNODE *crfsnp_fnode);

EC_BOOL crfsnp_fnode_free(CRFSNP_FNODE *crfsnp_fnode);

EC_BOOL crfsnp_fnode_init_0(const UINT32 md_id, CRFSNP_FNODE *crfsnp_fnode);

EC_BOOL crfsnp_fnode_clean_0(const UINT32 md_id, CRFSNP_FNODE *crfsnp_fnode);

EC_BOOL crfsnp_fnode_free_0(const UINT32 md_id, CRFSNP_FNODE *crfsnp_fnode);

EC_BOOL crfsnp_fnode_clone(const CRFSNP_FNODE *crfsnp_fnode_src, CRFSNP_FNODE *crfsnp_fnode_des);

EC_BOOL crfsnp_fnode_check_inode_exist(const CRFSNP_INODE *inode, const CRFSNP_FNODE *crfsnp_fnode);

EC_BOOL crfsnp_fnode_cmp(const CRFSNP_FNODE *crfsnp_fnode_1st, const CRFSNP_FNODE *crfsnp_fnode_2nd);

EC_BOOL crfsnp_fnode_import(const CRFSNP_FNODE *crfsnp_fnode_src, CRFSNP_FNODE *crfsnp_fnode_des);

char *crfsnp_fnode_md5sum_str(const CRFSNP_FNODE *crfsnp_fnode, uint8_t *md5str, const uint32_t max_len);

uint32_t crfsnp_fnode_count_replica(const CRFSNP_FNODE *crfsnp_fnode);

void crfsnp_fnode_print(LOG *log, const CRFSNP_FNODE *crfsnp_fnode);

void crfsnp_fnode_log_no_lock(LOG *log, const CRFSNP_FNODE *crfsnp_fnode);

CRFSNP_DNODE *crfsnp_dnode_new();

EC_BOOL crfsnp_dnode_init(CRFSNP_DNODE *crfsnp_dnode);

EC_BOOL crfsnp_dnode_clean(CRFSNP_DNODE *crfsnp_dnode);

EC_BOOL crfsnp_dnode_free(CRFSNP_DNODE *crfsnp_dnode);

EC_BOOL crfsnp_dnode_clone(const CRFSNP_DNODE *crfsnp_dnode_src, CRFSNP_DNODE *crfsnp_dnode_des);

CRFSNP_BNODE *crfsnp_bnode_new();

EC_BOOL crfsnp_bnode_init(CRFSNP_BNODE *crfsnp_bnode);

EC_BOOL crfsnp_bnode_clean(CRFSNP_BNODE *crfsnp_bnode);

EC_BOOL crfsnp_bnode_free(CRFSNP_BNODE *crfsnp_bnode);

EC_BOOL crfsnp_bnode_clone(const CRFSNP_BNODE *crfsnp_bnode_src, CRFSNP_BNODE *crfsnp_bnode_des);

void crfsnp_bnode_print_all(LOG *log, const CRFSNP *crfsnp, const CRFSNP_BNODE *crfsnp_bnode);

void crfsnp_bnode_print(LOG *log, const CRFSNP_BNODE *crfsnp_bnode);

CRFSNP_ITEM *crfsnp_item_new();

EC_BOOL crfsnp_item_init(CRFSNP_ITEM *crfsnp_item);

EC_BOOL crfsnp_item_clean(CRFSNP_ITEM *crfsnp_item);

EC_BOOL crfsnp_item_clone(const CRFSNP_ITEM *crfsnp_item_src, CRFSNP_ITEM *crfsnp_item_des);

EC_BOOL crfsnp_item_free(CRFSNP_ITEM *crfsnp_item);

EC_BOOL crfsnp_item_init_0(const UINT32 md_id, CRFSNP_ITEM *crfsnp_item);

EC_BOOL crfsnp_item_clean_0(const UINT32 md_id, CRFSNP_ITEM *crfsnp_item);

EC_BOOL crfsnp_item_free_0(const UINT32 md_id, CRFSNP_ITEM *crfsnp_item);

EC_BOOL crfsnp_item_set_key(CRFSNP_ITEM *crfsnp_item, const uint32_t klen, const uint8_t *key);

void crfsnp_item_print(LOG *log, const CRFSNP_ITEM *crfsnp_item);

EC_BOOL crfsnp_item_load(CRFSNP *crfsnp, uint32_t *offset, CRFSNP_ITEM *crfsnp_item);

EC_BOOL crfsnp_item_flush(CRFSNP *crfsnp, uint32_t *offset, const CRFSNP_ITEM *crfsnp_item);

EC_BOOL crfsnp_item_is(const CRFSNP_ITEM *crfsnp_item, const uint32_t klen, const uint8_t *key);

CRFSNP_ITEM *crfsnp_item_parent(const CRFSNP *crfsnp, const CRFSNP_ITEM *crfsnp_item);

CRFSNP_ITEM *crfsnp_item_left(const CRFSNP *crfsnp, const CRFSNP_ITEM *crfsnp_item);

CRFSNP_ITEM *crfsnp_item_right(const CRFSNP *crfsnp, const CRFSNP_ITEM *crfsnp_item);

EC_BOOL crfsnp_header_init(CRFSNP_HEADER *crfsnp_header, const uint32_t np_id, const uint8_t model, const uint8_t first_chash_algo_id, const uint8_t second_chash_algo_id);

EC_BOOL crfsnp_header_clean(CRFSNP_HEADER *crfsnp_header);

CRFSNP_HEADER *crfsnp_header_open(const uint32_t np_id, const UINT32 fsize, int fd);

CRFSNP_HEADER *crfsnp_header_clone(CRFSNP_HEADER *src_crfsnp_header, const uint32_t des_np_id, const UINT32 fsize, int fd);

CRFSNP_HEADER *crfsnp_header_create(const uint32_t np_id, const UINT32 fsize, int fd, const uint8_t np_model);

CRFSNP_HEADER *crfsnp_header_sync(CRFSNP_HEADER *crfsnp_header, const uint32_t np_id, const UINT32 fsize, int fd);

CRFSNP_HEADER *crfsnp_header_close(CRFSNP_HEADER *crfsnp_header, const uint32_t np_id, const UINT32 fsize, int fd);

CRFSNP *crfsnp_new();

EC_BOOL crfsnp_init(CRFSNP *crfsnp);

EC_BOOL crfsnp_clean(CRFSNP *crfsnp);

EC_BOOL crfsnp_free(CRFSNP *crfsnp);

EC_BOOL crfsnp_is_full(const CRFSNP *crfsnp);

void crfsnp_header_print(LOG *log, const CRFSNP *crfsnp);

void crfsnp_print(LOG *log, const CRFSNP *crfsnp);

CRFSNP_ITEM *crfsnp_dnode_find(const CRFSNP *crfsnp, const CRFSNP_DNODE *crfsnp_dnode, const uint32_t second_hash, const uint32_t klen, const uint8_t *key);

uint32_t crfsnp_dnode_search(const CRFSNP *crfsnp, const CRFSNP_DNODE *crfsnp_dnode, const uint32_t second_hash, const uint32_t klen, const uint8_t *key);

uint32_t crfsnp_dnode_insert(CRFSNP *crfsnp, const uint32_t parent_pos, const uint32_t path_seg_second_hash, const uint32_t path_seg_len, const uint8_t *path_seg, const uint32_t dir_flag);

/**
* umount one son from crfsnp_dnode,  where son is regular file item or dir item without any son
* crfsnp_dnode will be impacted on bucket and file num
**/
uint32_t crfsnp_dnode_umount_son(const CRFSNP *crfsnp, CRFSNP_DNODE *crfsnp_dnode, const uint32_t second_hash, const uint32_t klen, const uint8_t *key);
EC_BOOL crfsnp_dnode_delete_dir_son(const CRFSNP *crfsnp, CRFSNP_DNODE *crfsnp_dnode);

EC_BOOL crfsnp_dnode_delete_dir_single_son(const CRFSNP *crfsnp, CRFSNP_DNODE *crfsnp_dnode, CRFSNP_ITEM *crfsnp_item_del);

CRFSNP_ITEM *crfsnp_bnode_find(const CRFSNP *crfsnp, const CRFSNP_BNODE *crfsnp_bnode, const uint32_t second_hash, const uint32_t klen, const uint8_t *key);

uint32_t crfsnp_bnode_search(const CRFSNP *crfsnp, const CRFSNP_BNODE *crfsnp_bnode, const uint32_t second_hash, const uint32_t klen, const uint8_t *key);

uint32_t crfsnp_bnode_insert(CRFSNP *crfsnp, const uint32_t parent_pos, const uint32_t path_seg_second_hash, const uint32_t path_seg_len, const uint8_t *path_seg, const uint32_t dir_flag);

/*delete one dir son, not including crfsnp_bnode itself*/
EC_BOOL crfsnp_bnode_delete_dir_son(const CRFSNP *crfsnp, CRFSNP_BNODE *crfsnp_bnode);

EC_BOOL crfsnp_bnode_delete_single_son(const CRFSNP *crfsnp, CRFSNP_BNODE *crfsnp_bnode, CRFSNP_ITEM *crfsnp_item_del);

uint32_t crfsnp_search_no_lock(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path, const uint32_t dflag);

uint32_t crfsnp_search(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path, const uint32_t dflag);

uint32_t crfsnp_insert_no_lock(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path, const uint32_t dflag);

uint32_t crfsnp_insert(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path, const uint32_t dflag);

CRFSNP_ITEM *crfsnp_fetch(const CRFSNP *crfsnp, const uint32_t node_pos);

EC_BOOL crfsnp_inode_update(CRFSNP *crfsnp, CRFSNP_INODE *crfsnp_inode, 
                                   const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                                   const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no);

EC_BOOL crfsnp_fnode_update(CRFSNP *crfsnp, CRFSNP_FNODE *crfsnp_fnode, 
                                   const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                                   const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no);

EC_BOOL crfsnp_bucket_update(CRFSNP *crfsnp, const uint32_t node_pos, 
                                   const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                                   const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no);

EC_BOOL crfsnp_dnode_update(CRFSNP *crfsnp, CRFSNP_DNODE *crfsnp_dnode, 
                                   const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                                   const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no);

EC_BOOL crfsnp_bnode_update(CRFSNP *crfsnp, CRFSNP_BNODE *crfsnp_bnode, 
                                   const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                                   const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no);

EC_BOOL crfsnp_item_update(CRFSNP *crfsnp, CRFSNP_ITEM *crfsnp_item, 
                                   const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                                   const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no);

EC_BOOL crfsnp_update_no_lock(CRFSNP *crfsnp, 
                               const uint16_t src_disk_no, const uint16_t src_block_no, const uint16_t src_page_no, 
                               const uint16_t des_disk_no, const uint16_t des_block_no, const uint16_t des_page_no);

CRFSNP_ITEM *crfsnp_set(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path, const uint32_t dflag);

CRFSNP_ITEM *crfsnp_get(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path, const uint32_t dflag);

EC_BOOL crfsnp_delete(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path, const uint32_t dflag);

EC_BOOL crfsnp_move(CRFSNP *src_crfsnp, CRFSNP *des_crfsnp, const uint32_t src_path_len, const uint8_t *src_path, const uint32_t des_path_len, const uint8_t *des_path, const uint32_t dflag);

EC_BOOL crfsnp_umount_item(CRFSNP *crfsnp, const uint32_t node_pos);

EC_BOOL crfsnp_umount_item_deep(CRFSNP *crfsnp, const uint32_t node_pos);

EC_BOOL crfsnp_umount(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path, const uint32_t dflag);

EC_BOOL crfsnp_recycle_item_file(CRFSNP *crfsnp, CRFSNP_ITEM *crfsnp_item, CRFSNP_RECYCLE_DN *crfsnp_recycle_dn);

EC_BOOL crfsnp_recycle_dnode_item(CRFSNP *crfsnp, CRFSNP_DNODE *crfsnp_dnode, CRFSNP_ITEM *crfsnp_item, CRFSNP_RECYCLE_DN *crfsnp_recycle_dn);

EC_BOOL crfsnp_recycle_dnode(CRFSNP *crfsnp, CRFSNP_DNODE *crfsnp_dnode, const uint32_t node_pos, CRFSNP_RECYCLE_DN *crfsnp_recycle_dn);

EC_BOOL crfsnp_recycle_item_dir(CRFSNP *crfsnp, CRFSNP_ITEM *crfsnp_item, CRFSNP_RECYCLE_DN *crfsnp_recycle_dn);

EC_BOOL crfsnp_recycle_bnode_item(CRFSNP *crfsnp, CRFSNP_BNODE *crfsnp_bnode, CRFSNP_ITEM *crfsnp_item, CRFSNP_RECYCLE_DN *crfsnp_recycle_dn);

EC_BOOL crfsnp_recycle_bnode(CRFSNP *crfsnp, CRFSNP_BNODE *crfsnp_bnode, const uint32_t node_pos, CRFSNP_RECYCLE_DN *crfsnp_recycle_dn);

EC_BOOL crfsnp_recycle_item_b(CRFSNP *crfsnp, CRFSNP_ITEM *crfsnp_item, CRFSNP_RECYCLE_DN *crfsnp_recycle_dn);

EC_BOOL crfsnp_recycle(CRFSNP *crfsnp, CRFSNP_RECYCLE_DN *crfsnp_recycle_dn);

void crfsnp_make_b_seg_key(const uint32_t seg_no, uint8_t *key, const uint32_t key_max_len, uint32_t *klen);

EC_BOOL crfsnp_path_name(const CRFSNP *crfsnp, const uint32_t node_pos, const uint32_t path_max_len, uint32_t *path_len, uint8_t *path);

EC_BOOL crfsnp_path_name_cstr(const CRFSNP *crfsnp, const uint32_t node_pos, CSTRING *path_cstr);

EC_BOOL crfsnp_seg_name(const CRFSNP *crfsnp, const uint32_t offset, const uint32_t seg_name_max_len, uint32_t *seg_name_len, uint8_t *seg_name);

EC_BOOL crfsnp_seg_name_cstr(const CRFSNP *crfsnp, const uint32_t offset, CSTRING *seg_cstr);

EC_BOOL crfsnp_list_path_vec(const CRFSNP *crfsnp, const uint32_t node_pos, CVECTOR *path_cstr_vec);

EC_BOOL crfsnp_list_seg_vec(const CRFSNP *crfsnp, const uint32_t node_pos, CVECTOR *seg_cstr_vec);

EC_BOOL crfsnp_file_num(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path, uint32_t *file_num);

EC_BOOL crfsnp_file_size(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path, uint64_t *file_size);

EC_BOOL crfsnp_store_size_b(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path, uint64_t *store_size);

EC_BOOL crfsnp_mkdirs(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path);

EC_BOOL crfsnp_file_md5sum(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path, CMD5_DIGEST *md5sum);

EC_BOOL crfsnp_file_md5sum_b(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path, const uint32_t seg_no, CMD5_DIGEST *md5sum);

CRFSNP *crfsnp_open(const char *np_root_dir, const uint32_t np_id);

EC_BOOL crfsnp_close(CRFSNP *crfsnp);

EC_BOOL crfsnp_sync(CRFSNP *crfsnp);

EC_BOOL crfsnp_create_root_item(CRFSNP *crfsnp);

CRFSNP *crfsnp_clone(CRFSNP *src_crfsnp, const char *np_root_dir, const uint32_t des_np_id);

CRFSNP *crfsnp_create(const char *np_root_dir, const uint32_t np_id, const uint8_t np_model, const uint8_t hash_2nd_algo_id);

EC_BOOL crfsnp_show_item_full_path(LOG *log, const CRFSNP *crfsnp, const uint32_t node_pos);

EC_BOOL crfsnp_show_item(LOG *log, const CRFSNP *crfsnp, const uint32_t node_pos);

EC_BOOL crfsnp_show_dir(LOG *log, const CRFSNP *crfsnp, const CRFSNP_ITEM  *crfsnp_item);

EC_BOOL crfsnp_show_path(LOG *log, CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path);

EC_BOOL crfsnp_show_dir_depth(LOG *log, const CRFSNP *crfsnp, const CRFSNP_ITEM  *crfsnp_item);

EC_BOOL crfsnp_show_item_depth(LOG *log, const CRFSNP *crfsnp, const uint32_t node_pos);

EC_BOOL crfsnp_show_path_depth(LOG *log, CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path);

EC_BOOL crfsnp_get_first_fname_of_dir(const CRFSNP *crfsnp, const CRFSNP_ITEM  *crfsnp_item, uint8_t **fname, uint32_t *dflag);

EC_BOOL crfsnp_get_first_fname_of_path(CRFSNP *crfsnp, const uint32_t path_len, const uint8_t *path, uint8_t **fname, uint32_t *dflag);

EC_BOOL crfsnp_collect_items_no_lock(CRFSNP *crfsnp, const CSTRING *path, const UINT32 dflag, CVECTOR *crfsnp_item_vec);

CRFSNP *crfsnp_mem_create(const uint32_t np_id, const uint8_t np_model, const uint8_t hash_2nd_algo_id);

EC_BOOL crfsnp_mem_clean(CRFSNP *crfsnp);

EC_BOOL crfsnp_mem_free(CRFSNP *crfsnp);

/*------------------------------------------------ transfer node -----------------------------------------*/
CRFSNP_TRANS_NODE *crfsnp_trans_node_new();

EC_BOOL crfsnp_trans_node_init(CRFSNP_TRANS_NODE *crfsnp_trans_node);

EC_BOOL crfsnp_trans_node_clean(CRFSNP_TRANS_NODE *crfsnp_trans_node);

EC_BOOL crfsnp_trans_node_free(CRFSNP_TRANS_NODE *crfsnp_trans_node);

void crfsnp_trans_node_print(LOG *log, const CRFSNP_TRANS_NODE *crfsnp_trans_node);

CRFSNP_TRANS_NODE *crfsnp_trans_node_make(const uint32_t node_pos, const UINT32 len);

/*------------------------------------------------ transfer prepare -----------------------------------------*/
EC_BOOL crfsnp_transfer_pre_item_file(CRFSNP *crfsnp, const uint32_t node_pos, const CSTRING *dir_path, const CRFSDT_PNODE *crfsdt_pnode);

EC_BOOL crfsnp_transfer_pre_item_file_b(CRFSNP *crfsnp, const uint32_t node_pos, const CSTRING *dir_path, const CRFSDT_PNODE *crfsdt_pnode);

EC_BOOL crfsnp_transfer_pre_item_dir(CRFSNP *crfsnp, CRFSNP_ITEM *crfsnp_item, const CSTRING *dir_path, const CRFSDT_PNODE *crfsdt_pnode);

EC_BOOL crfsnp_transfer_pre_dnode_item(CRFSNP *crfsnp, const uint32_t node_pos, const CSTRING *dir_path, const CRFSDT_PNODE *crfsdt_pnode);

EC_BOOL crfsnp_transfer_pre_dnode(CRFSNP *crfsnp, const uint32_t node_pos, const CSTRING *dir_path, const CRFSDT_PNODE *crfsdt_pnode);

EC_BOOL crfsnp_transfer_pre(CRFSNP *crfsnp, const CSTRING *dir_path, const CRFSDT_PNODE *crfsdt_pnode);

/*------------------------------------------------ transfer handle -----------------------------------------*/
EC_BOOL crfsnp_transfer_handle_file(CRFSNP *crfsnp, const uint32_t node_pos, const CRFSDT_PNODE *crfsdt_pnode, const CRFSNP_TRANS_DN *crfsnp_trans_dn, TASK_MGR *task_mgr, CVECTOR *crfsnp_trans_node_vec);

EC_BOOL crfsnp_transfer_handle_file_b(CRFSNP *crfsnp, const uint32_t node_pos, const CRFSDT_PNODE *crfsdt_pnode, const CRFSNP_TRANS_DN *crfsnp_trans_dn);

EC_BOOL crfsnp_transfer_handle_item_dir(CRFSNP *crfsnp, const uint32_t node_pos, const uint32_t dflag, const CRFSDT_PNODE *crfsdt_pnode, const CRFSNP_TRANS_DN *crfsnp_trans_dn, TASK_MGR *task_mgr, CVECTOR *crfsnp_trans_node_vec);

EC_BOOL crfsnp_transfer_handle_dnode_item(CRFSNP *crfsnp, const uint32_t node_pos, const uint32_t dflag, const CRFSDT_PNODE *crfsdt_pnode, const CRFSNP_TRANS_DN *crfsnp_trans_dn, TASK_MGR *task_mgr, CVECTOR *crfsnp_trans_node_vec);

EC_BOOL crfsnp_transfer_handle_dnode(CRFSNP *crfsnp, const uint32_t node_pos, const uint32_t dflag, const CRFSDT_PNODE *crfsdt_pnode, const CRFSNP_TRANS_DN *crfsnp_trans_dn, TASK_MGR *task_mgr, CVECTOR *crfsnp_trans_node_vec);

EC_BOOL crfsnp_transfer_handle(CRFSNP *crfsnp, const CSTRING *dir_path, const CRFSDT_PNODE *crfsdt_pnode, const CRFSNP_TRANS_DN *crfsnp_trans_dn);

/*------------------------------------------------ transfer post clean -----------------------------------------*/
EC_BOOL crfsnp_transfer_post_file(CRFSNP *crfsnp, const uint32_t node_pos, const CRFSDT_PNODE *crfsdt_pnode, const CRFSNP_TRANS_DN *crfsnp_trans_dn);

EC_BOOL crfsnp_transfer_post_file_b(CRFSNP *crfsnp, const uint32_t node_pos, const CRFSDT_PNODE *crfsdt_pnode, const CRFSNP_TRANS_DN *crfsnp_trans_dn);

EC_BOOL crfsnp_transfer_post_item_dir(CRFSNP *crfsnp, const uint32_t node_pos, const CRFSDT_PNODE *crfsdt_pnode, const CRFSNP_TRANS_DN *crfsnp_trans_dn);

EC_BOOL crfsnp_transfer_post_dnode_item(CRFSNP *crfsnp, const uint32_t node_pos, const CRFSDT_PNODE *crfsdt_pnode, const CRFSNP_TRANS_DN *crfsnp_trans_dn);

EC_BOOL crfsnp_transfer_post_dnode(CRFSNP *crfsnp, const uint32_t node_pos, const CRFSDT_PNODE *crfsdt_pnode, const CRFSNP_TRANS_DN *crfsnp_trans_dn);

EC_BOOL crfsnp_transfer_post(CRFSNP *crfsnp, const CSTRING *dir_path, const CRFSDT_PNODE *crfsdt_pnode, const CRFSNP_TRANS_DN *crfsnp_trans_dn);


#endif/* _CRFSNP_H */

#ifdef __cplusplus
}
#endif/*__cplusplus*/

