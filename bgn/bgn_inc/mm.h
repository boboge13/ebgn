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

#ifndef _MM_H
#define _MM_H

#include <string.h>
#include "type.h"

#include "cstring.h"
#include "cmutex.h"

#ifdef PARSER_MEM_MGR
#include "xmalloc.h"
#include "treelang.h"
#include "coretypes.h"
#include "tree.h"
#endif/*PARSER_MEM_MGR*/

/*MD_TYPE*/
#define     MD_BGNZ      ((UINT32)  0)
#define     MD_BGNZ2     ((UINT32)  1)
#define     MD_BGNZN     ((UINT32)  2)
#define     MD_BGNF2N    ((UINT32)  3)
#define     MD_BGNFP     ((UINT32)  4)
#define     MD_ECF2N     ((UINT32)  5)
#define     MD_ECFP      ((UINT32)  6)
#define     MD_ECCF2N    ((UINT32)  7)
#define     MD_ECCFP     ((UINT32)  8)
#define     MD_CONV      ((UINT32)  9)
#define     MD_POLYZ     ((UINT32) 10)
#define     MD_POLYZ2    ((UINT32) 11)
#define     MD_POLYZN    ((UINT32) 12)
#define     MD_POLYFP    ((UINT32) 13)
#define     MD_POLYF2N   ((UINT32) 14)
#define     MD_SEAFP     ((UINT32) 15)
#define     MD_TASK      ((UINT32) 16)
#define     MD_MATRIXR   ((UINT32) 17)
#define     MD_EBGNZ     ((UINT32) 18)
#define     MD_EBGNZ2    ((UINT32) 19)
#define     MD_VECTORR   ((UINT32) 20)
#define     MD_PARSER    ((UINT32) 21)
#define     MD_EBGNR     ((UINT32) 22)
#define     MD_DMATRIXR  ((UINT32) 23)
#define     MD_VMATRIXR  ((UINT32) 24)
#define     MD_CFILE     ((UINT32) 25)
#define     MD_CDIR      ((UINT32) 26)
#define     MD_CMON      ((UINT32) 27)
#define     MD_TBD       ((UINT32) 28)/* user defined */
#define     MD_CRUN      ((UINT32) 29)/* user interfaces */
#define     MD_VMM       ((UINT32) 30) 
#define     MD_SUPER     ((UINT32) 31)
#define     MD_CTIMER    ((UINT32) 32)
#define     MD_CDFS      ((UINT32) 33)
#define     MD_CSOLR     ((UINT32) 34)
#define     MD_CBGT      ((UINT32) 35)
#define     MD_GANGLIA   ((UINT32) 36)
#define     MD_CSESSION  ((UINT32) 37)
#define     MD_CSCORE    ((UINT32) 38)
#define     MD_END       ((UINT32) 39)

/* Memory Management */
#define                        MM_UINT32    ((UINT32)  0)
#define                        MM_UINT16    ((UINT32)  1)
#define                         MM_UINT8    ((UINT32)  2)
#define                        MM_BIGINT    ((UINT32)  3)
#define                           MM_NAF    ((UINT32)  4)
#define                   MM_CURVE_POINT    ((UINT32)  5)
#define            MM_CURVE_AFFINE_POINT    ((UINT32)  6)
#define                   MM_ECF2N_CURVE    ((UINT32)  7)
#define                    MM_ECFP_CURVE    ((UINT32)  8)
#define                      MM_KEY_PAIR    ((UINT32)  9)
#define                          MM_POLY    ((UINT32) 10)
#define                     MM_POLY_ITEM    ((UINT32) 11)
#define                        MM_DEGREE    ((UINT32) 12)
#define                   MM_STRCHAR128B    ((UINT32) 13)
#define                   MM_STRCHAR256B    ((UINT32) 14)
#define                   MM_STRCHAR512B    ((UINT32) 15)
#define                     MM_STRCHAR1K    ((UINT32) 16)
#define                     MM_STRCHAR2K    ((UINT32) 17)
#define                     MM_STRCHAR4K    ((UINT32) 18)
#define                    MM_STRCHAR10K    ((UINT32) 19)
#define                    MM_STRCHAR64K    ((UINT32) 20)
#define                     MM_STRCHAR1M    ((UINT32) 21)
#define                          MM_EBGN    ((UINT32) 22)
#define                     MM_EBGN_ITEM    ((UINT32) 23)
#define                          MM_REAL    ((UINT32) 24)
#define                  MM_MATRIX_BLOCK    ((UINT32) 25)
#define                 MM_MATRIX_HEADER    ((UINT32) 26)
#define                        MM_MATRIX    ((UINT32) 27)
#define                  MM_VECTOR_BLOCK    ((UINT32) 28)
#define                        MM_VECTOR    ((UINT32) 29)
#ifdef PARSER_MEM_MGR
#define                          MM_PROD    ((UINT32) 30)
#define                          MM_TREE    ((UINT32) 31)
#else
#define                     MM_PROD_RSVD    ((UINT32) 30) /*reserved*/
#define                     MM_TREE_RSVD    ((UINT32) 31) /*reserved*/
#endif/*PARSER_MEM_MGR*/
#define                     MM_TASK_NODE    ((UINT32) 32)
#define                      MM_TASK_MGR    ((UINT32) 33)
#define                  MM_TASK_CONTEXT    ((UINT32) 34)
#define                      MM_MOD_NODE    ((UINT32) 35)
#define                       MM_MOD_MGR    ((UINT32) 36)
#define                     MM_TASKC_MGR    ((UINT32) 37)

#define                     MM_STRCHAR2M    ((UINT32) 38)
#define                     MM_STRCHAR4M    ((UINT32) 39)
#define                     MM_STRCHAR8M    ((UINT32) 40)
#define                    MM_STRCHAR16M    ((UINT32) 41)
#define                    MM_STRCHAR32M    ((UINT32) 42)
#define                    MM_STRCHAR64M    ((UINT32) 43)
#define                   MM_STRCHAR128M    ((UINT32) 44)

#define                    MM_CLIST_DATA    ((UINT32) 45)
#define                   MM_CSTACK_DATA    ((UINT32) 46)
#define                     MM_CSET_DATA    ((UINT32) 47)
#define                   MM_CQUEUE_DATA    ((UINT32) 48)
#define                       MM_CSTRING    ((UINT32) 49)
#define                 MM_FUNC_ADDR_MGR    ((UINT32) 50)
/*---------------- buff mem definition beg: ----------------*/
#define                    BUFF_MEM_DEF_BEG MM_UINT8_064B /*the minimum size buff*/
#define                    BUFF_MEM_DEF_END MM_UINT8_512M /*the maximum size buff*/

#define                    MM_UINT8_064B    ((UINT32) 51)
#define                    MM_UINT8_128B    ((UINT32) 52)
#define                    MM_UINT8_256B    ((UINT32) 53)
#define                    MM_UINT8_512B    ((UINT32) 54)
#define                    MM_UINT8_001K    ((UINT32) 55)
#define                    MM_UINT8_002K    ((UINT32) 56)
#define                    MM_UINT8_004K    ((UINT32) 57)
#define                    MM_UINT8_008K    ((UINT32) 58)
#define                    MM_UINT8_016K    ((UINT32) 59)
#define                    MM_UINT8_032K    ((UINT32) 60)
#define                    MM_UINT8_064K    ((UINT32) 61)
#define                    MM_UINT8_128K    ((UINT32) 62)
#define                    MM_UINT8_256K    ((UINT32) 63)
#define                    MM_UINT8_512K    ((UINT32) 64)
#define                    MM_UINT8_001M    ((UINT32) 65)
#define                    MM_UINT8_002M    ((UINT32) 66)
#define                    MM_UINT8_004M    ((UINT32) 67)
#define                    MM_UINT8_008M    ((UINT32) 68)
#define                    MM_UINT8_016M    ((UINT32) 69)
#define                    MM_UINT8_032M    ((UINT32) 70)
#define                    MM_UINT8_064M    ((UINT32) 71)
#define                    MM_UINT8_128M    ((UINT32) 72)
#define                    MM_UINT8_256M    ((UINT32) 73)
#define                    MM_UINT8_512M    ((UINT32) 74)
/*---------------- buff mem definition end: ----------------*/

/*----------------------- extensive -----------------------*/
#define                MM_REAL_BLOCK_2_2    ((UINT32) 75)
#define                MM_REAL_BLOCK_4_4    ((UINT32) 76)
#define                MM_REAL_BLOCK_8_8    ((UINT32) 77)
#define              MM_REAL_BLOCK_16_16    ((UINT32) 78)
#define              MM_REAL_BLOCK_32_32    ((UINT32) 79)
#define              MM_REAL_BLOCK_64_64    ((UINT32) 80)
#define            MM_REAL_BLOCK_128_128    ((UINT32) 81)
#define            MM_REAL_BLOCK_256_256    ((UINT32) 82)
#define            MM_REAL_BLOCK_512_512    ((UINT32) 83)
#define          MM_REAL_BLOCK_1024_1024    ((UINT32) 84)

#define                         MM_CLIST    ((UINT32) 85)
#define                        MM_CSTACK    ((UINT32) 86)
#define                          MM_CSET    ((UINT32) 87)
#define                        MM_CQUEUE    ((UINT32) 88)
#define                       MM_CVECTOR    ((UINT32) 89)

#define                     MM_TASKS_CFG    ((UINT32) 90)
#define                     MM_TASKR_CFG    ((UINT32) 91)
#define                      MM_TASK_CFG    ((UINT32) 92)

#define                    MM_TASKS_NODE    ((UINT32) 93)
#define                  MM_CROUTER_NODE    ((UINT32) 94)
#define                    MM_TASKC_NODE    ((UINT32) 95)
#define              MM_CROUTER_NODE_VEC    ((UINT32) 96)
#define                   MM_CROUTER_CFG    ((UINT32) 97)
#define                  MM_CTHREAD_TASK    ((UINT32) 98)
#define                      MM_VMM_NODE    ((UINT32) 99)
#define                           MM_LOG    ((UINT32)100)
#define                     MM_COMM_NODE    ((UINT32)101)
#define                     MM_CFILE_SEG    ((UINT32)102)
#define                    MM_CFILE_NODE    ((UINT32)103)
#define                         MM_KBUFF    ((UINT32)104)
#define                      MM_CDIR_SEG    ((UINT32)105)
#define                     MM_CDIR_NODE    ((UINT32)106)
#define                      MM_CMON_OBJ    ((UINT32)107)
#define                  MM_CMON_OBJ_VEC    ((UINT32)108)
#define                MM_CMON_OBJ_MERGE    ((UINT32)109)
#define                 MM_CSOCKET_CNODE    ((UINT32)110)
#define                   MM_CTIMER_NODE    ((UINT32)111)
#define                 MM_CSYS_CPU_STAT    ((UINT32)112)
#define             MM_CSYS_CPU_AVG_STAT    ((UINT32)113)
#define                 MM_CSYS_MEM_STAT    ((UINT32)114)
#define                MM_CPROC_MEM_STAT    ((UINT32)115)
#define                MM_CPROC_CPU_STAT    ((UINT32)116)
#define             MM_CPROC_THREAD_STAT    ((UINT32)117)
#define             MM_CRANK_THREAD_STAT    ((UINT32)118)
#define             MM_CPROC_MODULE_STAT    ((UINT32)119)
#define                 MM_CSYS_ETH_STAT    ((UINT32)120)
#define                 MM_CSYS_DSK_STAT    ((UINT32)121)
#define            MM_MM_MAN_OCCUPY_NODE    ((UINT32)122)
#define              MM_MM_MAN_LOAD_NODE    ((UINT32)123)
#define                  MM_CTHREAD_NODE    ((UINT32)124)
#define                  MM_CTHREAD_POOL    ((UINT32)125)
#define                MM_TASK_RANK_NODE    ((UINT32)126)
#define                       MM_CMD_SEG    ((UINT32)127)
#define                      MM_CMD_PARA    ((UINT32)128)
#define                      MM_CMD_HELP    ((UINT32)129)
#define                      MM_CMD_ELEM    ((UINT32)130)

#define              MM_TASK_REPORT_NODE    ((UINT32)131)
#define                    MM_CHASH_NODE    ((UINT32)132)
#define                     MM_CHASH_VEC    ((UINT32)133)
#define                  MM_CHASHDB_ITEM    ((UINT32)134)
#define                       MM_CHASHDB    ((UINT32)135)
#define                MM_CHASHDB_BUCKET    ((UINT32)136)
#define                        MM_CDFSNP    ((UINT32)137)
#define                   MM_CDFSNP_ITEM    ((UINT32)138)
#define                  MM_CDFSNP_INODE    ((UINT32)139)
#define                  MM_CDFSNP_FNODE    ((UINT32)140)
#define                  MM_CDFSNP_DNODE    ((UINT32)141)
#define                  MM_CDFSDN_CACHE    ((UINT32)142)
#define                  MM_CDFSDN_BLOCK    ((UINT32)143)
#define                        MM_CDFSDN    ((UINT32)144)
#define                 MM_CDFSDN_RECORD    ((UINT32)145)
#define             MM_CDFSDN_RECORD_MGR    ((UINT32)146)
#define                    MM_CLOAD_STAT    ((UINT32)147) 
#define                    MM_CLOAD_NODE    ((UINT32)148)
#define                    MM_CDFSNP_MGR    ((UINT32)149)
#define                   MM_CDFSDN_STAT    ((UINT32)150)
#define                MM_TYPE_CONV_ITEM    ((UINT32)151)
#define                          MM_CSRV    ((UINT32)152)
#define                    MM_CFUSE_MODE    ((UINT32)153)
#define                    MM_CFUSE_STAT    ((UINT32)154)
#define                    MM_CFUSE_NODE    ((UINT32)155)
#define                     MM_CFUSE_COP    ((UINT32)156)
#define                       MM_CBGT_KV    ((UINT32)157)
#define                        MM_CBYTES    ((UINT32)158)
#define                      MM_CBGT_REG    ((UINT32)159)
#define                       MM_CBITMAP    ((UINT32)160)
#define                  MM_CBTIMER_NODE    ((UINT32)161)
#define              MM_CLUSTER_NODE_CFG    ((UINT32)162)
#define                   MM_CLUSTER_CFG    ((UINT32)163)
#define                      MM_CPARACFG    ((UINT32)164)
#define                     MM_MCAST_CFG    ((UINT32)165)
#define                MM_BCAST_DHCP_CFG    ((UINT32)166)
#define                     MM_MACIP_CFG    ((UINT32)167)
#define                   MM_GANGLIA_CFG    ((UINT32)168)
#define                       MM_SYS_CFG    ((UINT32)169)
#define                   MM_SUPER_FNODE    ((UINT32)170)
#define                     MM_CMAP_NODE    ((UINT32)171)
#define                          MM_CMAP    ((UINT32)172)
#define                 MM_CSESSION_NODE    ((UINT32)173)
#define                 MM_CSESSION_ITEM    ((UINT32)174)
#define                        MM_CTIMET    ((UINT32)175)
#define                         MM_CSDOC    ((UINT32)176)
#define                        MM_CSWORD    ((UINT32)177)
#define                   MM_CSDOC_WORDS    ((UINT32)178)
#define                   MM_CSWORD_DOCS    ((UINT32)179)
#define                    MM_CBTREE_KEY    ((UINT32)180)
#define                   MM_CBTREE_NODE    ((UINT32)181)
#define                        MM_CBTREE    ((UINT32)182)
#define                      MM_CBGT_GDB    ((UINT32)183)
#define                MM_COROUTINE_TASK    ((UINT32)184)
#define                MM_COROUTINE_NODE    ((UINT32)185)
#define                MM_COROUTINE_POOL    ((UINT32)186)

#define                           MM_END    ((UINT32)256)
#define                        MM_IGNORE    ((UINT32)0xFFFF)

typedef UINT32 MM_TYPE_UINT32;

#define NODE_LIST_TAIL ((UINT32)~0)

/*MM_USED_FLAG*/
typedef UINT32 MM_USED_FLAG_UINT32;
#define MM_NODE_NOT_USED ((UINT32) 0)
#define MM_NODE_USED     ((UINT32) 1)
#define MM_NODE_OBSOLETE ((UINT32) 2)

#define MM_IS_LOCAL_VMM   ((UINT32) 1)
#define MM_IS_REMOTE_VMM  ((UINT32) 2)

typedef struct
{
    UINT32  next;
    MM_USED_FLAG_UINT32 usedflag;

#if ( SWITCH_ON == STATIC_MEM_DIAG_LOC_SWITCH )
    UINT32 location;
#endif/*SWITCH_ON == STATIC_MEM_DIAG_LOC_SWITCH*/

#if ( SWITCH_ON == MULTI_USER_MODE_SWITCH )
    UINT32  module_type;
    UINT32  module_id;
#endif/*SWITCH_ON == MULTI_USER_MODE_SWITCH*/

    UINT8 * pmem;
}MM_NODE;

typedef struct _MM_NODE_BLOCK
{
    LIST_NODE      linknodeblock;      /*the mount point of nodeblock in linked nodeblock list of manager. note: after initialization, never modify it until destroy*/
    LIST_NODE      freenodeblock;      /*the mount point of nodeblock in free nodeblock list of manager. empty means it not belong to manager, otherwise, belong ot manager*/

    /*current node block info*/
    UINT32          nodenum;        /*number of nodes*/
    UINT32          typesize;       /*bytes*/

    UINT32          maxusedsum;     /*stat data: max number of used nodes*/
    UINT32          curusedsum;     /*stat data: current number of used nodes*/

    UINT32          nextfree;       /*next free node index,if no free node anymore, set it to NODE_LIST_TAIL*/
    MM_NODE *       pnodes;         /*split the memory space into nodes and link them one by one, meanwhile it's the free nodes spool*/
    UINT8   *       pbaseaddr;      /*the head address of memory space under management, head belong to the memory space*/
    UINT8   *       ptailaddr;      /*the tail address of memory space under management, tail is out of the memory space*/
}MM_NODE_BLOCK;

typedef struct
{
    UINT32  type;
    void   *pmem;    
}MM_COMM;/*common dynamic stack memory*/

typedef struct
{
    UINT32          type; 
    union
    {
        MM_NODE_BLOCK * nodeblock;    
        MM_COMM       * mm_comm;
    }u;
}MM_AUX;

typedef struct
{
    MM_TYPE_UINT32  type;
    UINT8  *        name;
    UINT32          nodenumsum;        /*number of nodes*/
    UINT32          nodeblocknum;      /*number of node blocks*/
    UINT32          nodenumperblock;   /*node number per node block*/
    UINT32          typesize;          /*bytes*/

    UINT32          maxusedsum;        /*stat data: max number of used nodes*/
    UINT32          curusedsum;        /*stat data: current number of used nodes*/

    LIST_NODE       linknodeblockhead;    /*point to the head of the node block list*/
    LIST_NODE       freenodeblockhead;    /*next free nodeblock,if no free node anymore, set it to NULL_PTR*/

    CMUTEX          cmutex;
}MM_MAN;/* Manager */



typedef struct
{
    UINT32 location;
    const char *filename;
    UINT32 lineno;
}MM_LOC;

typedef struct
{
    MM_TYPE_UINT32  type;
    UINT32          nodenumsum;        /*number of nodes*/
    UINT32          maxusedsum;        /*stat data: max number of used nodes*/
    UINT32          curusedsum;        /*stat data: current number of used nodes*/    
}MM_MAN_OCCUPY_NODE;

#define MM_MAN_OCCUPY_NODE_TYPE(mm_man_occupy_node)          ((mm_man_occupy_node)->type)
#define MM_MAN_OCCUPY_NODE_SUM(mm_man_occupy_node)           ((mm_man_occupy_node)->nodenumsum)
#define MM_MAN_OCCUPY_NODE_MAX(mm_man_occupy_node)           ((mm_man_occupy_node)->maxusedsum)
#define MM_MAN_OCCUPY_NODE_CUR(mm_man_occupy_node)           ((mm_man_occupy_node)->curusedsum)

typedef struct
{
    MM_TYPE_UINT32  type;
    REAL            maxusedload;       /*stat data: load percent of max used nodes: (100.0 * maxusedsum) / nodenumsum*/
    REAL            curusedload;       /*stat data: load percent of current used nodes: (100.0 * curusedsum) / nodenumsum*/
}MM_MAN_LOAD_NODE;

#define MM_MAN_LOAD_NODE_TYPE(mm_man_load_node)          ((mm_man_load_node)->type)
#define MM_MAN_LOAD_NODE_MAX(mm_man_load_node)           ((mm_man_load_node)->maxusedload)
#define MM_MAN_LOAD_NODE_CUR(mm_man_load_node)           ((mm_man_load_node)->curusedload)


/* interface of MM_NODE_BLOCK*/
#define NODEBLOCK_LINKNODE(pNodeBlock)    (&((pNodeBlock)->linknodeblock))
#define NODEBLOCK_FREENODE(pNodeBlock)    (&((pNodeBlock)->freenodeblock))

#define NODEBLOCK_LINKNODE_INIT(pNodeBlock)    INIT_LIST_BASE_HEAD(NODEBLOCK_LINKNODE(pNodeBlock))
#define NODEBLOCK_FREENODE_INIT(pNodeBlock)    INIT_LIST_BASE_HEAD(NODEBLOCK_FREENODE(pNodeBlock))

#define NODEBLOCK_LINKNODE_IS_EMPTY(pNodeBlock)  list_base_empty(NODEBLOCK_LINKNODE(pNodeBlock))
#define NODEBLOCK_FREENODE_IS_EMPTY(pNodeBlock)  list_base_empty(NODEBLOCK_FREENODE(pNodeBlock))

#define NODEBLOCK_LINKNODE_NEXT(pNodeBlock)  list_base_entry((pNodeBlock)->linknodeblock.next, MM_NODE_BLOCK, linknodeblock)
#define NODEBLOCK_LINKNODE_PREV(pNodeBlock)  list_base_entry((pNodeBlock)->linknodeblock.prev, MM_NODE_BLOCK, linknodeblock)

#define NODEBLOCK_FREENODE_NEXT(pNodeBlock)  list_base_entry((pNodeBlock)->freenodeblock.next, MM_NODE_BLOCK, freenodeblock)
#define NODEBLOCK_FREENODE_PREV(pNodeBlock)  list_base_entry((pNodeBlock)->freenodeblock.prev, MM_NODE_BLOCK, freenodeblock)

#define NODEBLOCK_LINKNODE_DEL(pNodeBlock)   list_base_del(NODEBLOCK_LINKNODE(pNodeBlock))
#define NODEBLOCK_FREENODE_DEL(pNodeBlock)   list_base_del(NODEBLOCK_FREENODE(pNodeBlock))

/*interface of linknodeblockhead of MM_MAN*/
#define MAN_LINKNODEBLOCK_HEAD(pMan)  (&((pMan)->linknodeblockhead))

#define MAN_LINKNODEBLOCK_HEAD_INIT(pMan) INIT_LIST_BASE_HEAD(MAN_LINKNODEBLOCK_HEAD(pMan))

#define MAN_LINKNODEBLOCK_FIRST_NODE(pMan) list_base_entry((pMan)->linknodeblockhead.next, MM_NODE_BLOCK, linknodeblock)

#define MAN_LINKNODEBLOCK_LAST_NODE(pMan)  list_base_entry((pMan)->linknodeblockhead.prev, MM_NODE_BLOCK, linknodeblock)

/*the null item in the pMan which is not any real item*/
#define MAN_LINKNODEBLOCK_NULL_NODE(pMan) list_base_entry(&((pMan)->linknodeblockhead), MM_NODE_BLOCK, linknodeblock)

#define MAN_LINKNODEBLOCK_IS_EMPTY(pMan)  list_base_empty(MAN_LINKNODEBLOCK_HEAD(pMan))

#define MAN_LINKNODEBLOCK_NODE_ADD_TAIL(pMan, pNodeBlock) list_base_add_tail(NODEBLOCK_LINKNODE(pNodeBlock), MAN_LINKNODEBLOCK_HEAD(pMan))

#define MAN_LINKNODEBLOCK_NODE_ADD_HEAD(pMan, pNodeBlock) list_base_add(NODEBLOCK_LINKNODE(pNodeBlock), MAN_LINKNODEBLOCK_HEAD(pMan))

#define MAN_LINKNODEBLOCK_NODE_NEXT(pNodeBlock)  list_base_entry((pNodeBlock)->linknodeblock.next, MM_NODE_BLOCK, linknodeblock)

#define MAN_LINKNODEBLOCK_NODE_PREV(pNodeBlock)  list_base_entry((pNodeBlock)->linknodeblock.prev, MM_NODE_BLOCK, linknodeblock)

#define MAN_LINKNODEBLOCK_NODE_DEL(pNodeBlock)   list_base_del(NODEBLOCK_LINKNODE(pNodeBlock))

#define MAN_LINKNODEBLOCK_LOOP_PREV(pMan, pNodeBlock) \
    for((pNodeBlock) = MAN_LINKNODEBLOCK_LAST_NODE(pMan);  (pNodeBlock) != MAN_LINKNODEBLOCK_NULL_NODE(pMan); (pNodeBlock) = MAN_LINKNODEBLOCK_NODE_PREV(pNodeBlock))

#define MAN_LINKNODEBLOCK_LOOP_NEXT(pMan, pNodeBlock) \
    for((pNodeBlock) = MAN_LINKNODEBLOCK_FIRST_NODE(pMan);  (pNodeBlock) != MAN_LINKNODEBLOCK_NULL_NODE(pMan); (pNodeBlock) = MAN_LINKNODEBLOCK_NODE_NEXT(pNodeBlock))

/*interface of freenodeblockhead of MM_MAN*/
#define MAN_FREENODEBLOCK_HEAD(pMan)  (&((pMan)->freenodeblockhead))

#define MAN_FREENODEBLOCK_HEAD_INIT(pMan) INIT_LIST_BASE_HEAD(MAN_FREENODEBLOCK_HEAD(pMan))

#define MAN_FREENODEBLOCK_FIRST_NODE(pMan) list_base_entry((pMan)->freenodeblockhead.next, MM_NODE_BLOCK, freenodeblock)

#define MAN_FREENODEBLOCK_LAST_NODE(pMan)  list_base_entry((pMan)->freenodeblockhead.prev, MM_NODE_BLOCK, freenodeblock)

/*the null item in the pMan which is not any real item*/
#define MAN_FREENODEBLOCK_NULL_NODE(pMan) list_base_entry(&((pMan)->freenodeblockhead), MM_NODE_BLOCK, freenodeblock)

#define MAN_FREENODEBLOCK_IS_EMPTY(pMan)  list_base_empty(MAN_FREENODEBLOCK_HEAD(pMan))

#define MAN_FREENODEBLOCK_NODE_ADD_TAIL(pMan, pNodeBlock) list_base_add_tail(NODEBLOCK_FREENODE(pNodeBlock), MAN_FREENODEBLOCK_HEAD(pMan))

#define MAN_FREENODEBLOCK_NODE_ADD_HEAD(pMan, pNodeBlock) list_base_add(NODEBLOCK_FREENODE(pNodeBlock), MAN_FREENODEBLOCK_HEAD(pMan))

#define MAN_FREENODEBLOCK_NODE_NEXT(pNodeBlock)  list_base_entry((pNodeBlock)->freenodeblock.next, MM_NODE_BLOCK, freenodeblock)

#define MAN_FREENODEBLOCK_NODE_PREV(pNodeBlock)  list_base_entry((pNodeBlock)->freenodeblock.prev, MM_NODE_BLOCK, freenodeblock)

#define MAN_FREENODEBLOCK_NODE_DEL(pNodeBlock)   list_base_del(NODEBLOCK_FREENODE(pNodeBlock))

#define MAN_FREENODEBLOCK_LOOP_PREV(pMan, pNodeBlock) \
    for((pNodeBlock) = MAN_FREENODEBLOCK_LAST_NODE(pMan);  (pNodeBlock) != MAN_FREENODEBLOCK_NULL_NODE(pMan); (pNodeBlock) = MAN_FREENODEBLOCK_NODE_PREV(pNodeBlock))

#define MAN_FREENODEBLOCK_LOOP_NEXT(pMan, pNodeBlock) \
    for((pNodeBlock) = MAN_FREENODEBLOCK_FIRST_NODE(pMan);  (pNodeBlock) != MAN_FREENODEBLOCK_NULL_NODE(pMan); (pNodeBlock) = MAN_FREENODEBLOCK_NODE_NEXT(pNodeBlock))


UINT32 init_static_mem();

/**
*
*   get file name of location
*
**/
const char *get_file_name_of_location(const UINT32 location);

/**
*
*   get line no of location
*
**/
UINT32 get_line_no_of_location(const UINT32 location);

EC_BOOL check_static_mem_type(const MM_TYPE_UINT32 type, const UINT32 typesize);
UINT32 fetch_static_mem_typesize(MM_TYPE_UINT32 type);
UINT32 alloc_static_mem_0(const UINT32 location,const UINT32 module_type, const UINT32 module_id,const MM_TYPE_UINT32 type,void **ppvoid);
UINT32 free_static_mem_0(const UINT32 location,const UINT32 module_type, const UINT32 module_id,const MM_TYPE_UINT32 type,void *pvoid);

UINT32 init_mm_man(const UINT32 mm_type);
UINT32 reg_mm_man(const UINT32 mm_type, const char *mm_name, const UINT32 block_num, const UINT32 type_size, const UINT32 location);

#define MM_LOC_FILE_NAME(__location__) (get_file_name_of_location(__location__))
#define MM_LOC_LINE_NO(__location__)   (get_line_no_of_location(__location__))

#if(SWITCH_ON == STATIC_MEM_TYPE_CHECK_SWITCH)
static UINT8 *get_short_file_name(const UINT8 *long_file_name)
{
    UINT8 *pch;

    pch = (UINT8 *)strrchr(long_file_name,'\\');
    if ( 0 == pch)
    {
        pch = (UINT8 *)long_file_name;
    }
    else
    {
        pch ++;
    }
    return pch;
}


#define alloc_static_mem(module_type, module_id,mm_type, ppvoid, __location__) do{\
    UINT32  _static_mem_node_type_size;\
    EC_BOOL _static_mem_type_flag;\
    _static_mem_node_type_size = sizeof (**(ppvoid));\
    _static_mem_type_flag = check_static_mem_type(mm_type, _static_mem_node_type_size);\
    if ( EC_FALSE == _static_mem_type_flag)\
    {\
        sys_log(LOGSTDOUT,"alloc_static_mem:>>>at %ld: invalid type %ld.\n",__location__, mm_type);\
    }\
    alloc_static_mem_0(__location__, module_type, module_id, mm_type, (void **)ppvoid);\
}while(0)

#define free_static_mem(module_type, module_id,mm_type, pvoid, __location__) do{\
    UINT32  _static_mem_node_type_size;\
    EC_BOOL _static_mem_type_flag;\
    _static_mem_node_type_size = sizeof (*(pvoid));\
    _static_mem_type_flag = check_static_mem_type(mm_type, _static_mem_node_type_size);\
    if ( EC_FALSE == _static_mem_type_flag)\
    {\
        sys_log(LOGSTDOUT,"free_static_mem:>>>at %ld: invalid type %ld.\n",__location__, mm_type);\
    }\
    free_static_mem_0(__location__,module_type, module_id, mm_type, (void *)pvoid);\
}while(0)
#endif/*(SWITCH_ON == STATIC_MEM_TYPE_CHECK_SWITCH)*/

#if(SWITCH_OFF == STATIC_MEM_TYPE_CHECK_SWITCH)
#define alloc_static_mem(module_type, module_id,mm_type, ppvoid, __location__)  do{\
    alloc_static_mem_0(__location__, module_type, module_id, mm_type, (void **)ppvoid);\
}while(0)

#define free_static_mem(module_type, module_id,mm_type, pvoid, __location__) do{\
    free_static_mem_0(__location__, module_type, module_id, mm_type, (void *)pvoid);\
}while(0)
#endif/*(SWITCH_OFF == STATIC_MEM_TYPE_CHECK_SWITCH)*/


#if ( SWITCH_ON == MULTI_USER_MODE_SWITCH )
UINT32 free_module_static_mem(UINT32 module_type, UINT32 module_id);
#endif/*SWITCH_ON == MULTI_USER_MODE_SWITCH*/

#if ( SWITCH_OFF == MULTI_USER_MODE_SWITCH )
#define free_module_static_mem(module_type, module_id) do{}while(0)
#endif/*SWITCH_OFF == MULTI_USER_MODE_SWITCH*/

UINT32 breathing_static_mem();
UINT32 destory_static_mem();

void *safe_malloc(const UINT32 size, const UINT32 location);
void safe_free(void *pvoid, const UINT32 location);
void *safe_realloc_0(void *old_pvoid, const UINT32 new_size, const UINT32 location);
void *safe_realloc(void *old_pvoid, const UINT32 old_size, const UINT32 new_size, const UINT32 location);
void safe_copy(UINT8 *old_ptr, UINT8 *new_ptr, UINT32 len);

void print_static_mem_status(LOG *log);
void print_static_mem_status_of_type(LOG *log, const MM_TYPE_UINT32  type);
#if ( SWITCH_ON == STATIC_MEM_DIAG_LOC_SWITCH )
UINT32 print_static_mem_diag_info( LOG *log );
UINT32 print_static_mem_diag_info_of_type(LOG *log, const MM_TYPE_UINT32 type);

UINT32 print_static_mem_stat_info(LOG *log);
UINT32 print_static_mem_stat_info_of_type(LOG *log, const MM_TYPE_UINT32 type);
#endif/*SWITCH_ON == STATIC_MEM_DIAG_LOC_SWITCH*/

#if 1
#define SAFE_MALLOC(__size__, __location__)                                   safe_malloc(__size__, __location__)
#define SAFE_FREE(__pvoid__, __location__)                                    safe_free(__pvoid__, __location__)
#define SAFE_REALLOC(__old_pvoid__, __old_size__, __new_size__, __location__) safe_realloc(__old_pvoid__, __old_size__, __new_size__, __location__)
//#define SAFE_REALLOC(__old_pvoid__, __old_size__, __new_size__, __location__) safe_realloc_0(__old_pvoid__, __new_size__, __location__)
#endif

#if 0
#define SAFE_MALLOC(__size__, __location__)                     malloc(__size__)
#define SAFE_FREE(__pvoid__, __location__)                      free(__pvoid__)
#define SAFE_REALLOC(__old_pvoid__, __new_size__, __location__) realloc(__old_pvoid__, __new_size__)
#endif

UINT32 mm_man_occupy_node_init_0(const UINT32 md_id, MM_MAN_OCCUPY_NODE *mm_man_occupy_node);
UINT32 mm_man_occupy_node_clean_0(const UINT32 md_id, MM_MAN_OCCUPY_NODE *mm_man_occupy_node);
UINT32 mm_man_occupy_node_free_0(const UINT32 md_id, MM_MAN_OCCUPY_NODE *mm_man_occupy_node);
UINT32 mm_man_occupy_node_init(MM_MAN_OCCUPY_NODE *mm_man_occupy_node);
UINT32 mm_man_occupy_node_free(MM_MAN_OCCUPY_NODE *mm_man_occupy_node);
UINT32 mm_man_occupy_node_clone(MM_MAN_OCCUPY_NODE *mm_man_occupy_node_src, MM_MAN_OCCUPY_NODE *mm_man_occupy_node_des);
EC_BOOL mm_man_occupy_node_was_used(const MM_TYPE_UINT32 type);
UINT32 mm_man_occupy_node_fetch(const MM_TYPE_UINT32 type, MM_MAN_OCCUPY_NODE *mm_man_occupy_node);

UINT32 mm_man_load_node_init_0(const UINT32 md_id, MM_MAN_LOAD_NODE *mm_man_load_node);
UINT32 mm_man_load_node_clean_0(const UINT32 md_id, MM_MAN_LOAD_NODE *mm_man_load_node);
UINT32 mm_man_load_node_free_0(const UINT32 md_id, MM_MAN_LOAD_NODE *mm_man_load_node);
UINT32 mm_man_load_node_init(MM_MAN_LOAD_NODE *mm_man_load_node);
UINT32 mm_man_load_node_free(MM_MAN_LOAD_NODE *mm_man_load_node);
UINT32 mm_man_load_node_clone(MM_MAN_LOAD_NODE *mm_man_load_node_src, MM_MAN_LOAD_NODE *mm_man_load_node_des);
EC_BOOL mm_man_load_node_was_used(const MM_TYPE_UINT32 type);
UINT32 mm_man_load_node_fetch(const MM_TYPE_UINT32 type, MM_MAN_LOAD_NODE *mm_man_load_node);

#endif/*_MM_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

