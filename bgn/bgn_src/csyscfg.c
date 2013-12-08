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

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "type.h"
#include "mm.h"
#include "log.h"

#include "cstring.h"
#include "cvector.h"
#include "cset.h"

#include "cxml.h"
#include "taskcfg.h"
#include "cparacfg.inc"
#include "cparacfg.h"
#include "cmpic.inc"
#include "cbtimer.h"
#include "cmisc.h"

#include "cdfs.h"
#include "cbgt.h"
#include "task.h"
#include "csyscfg.inc"
#include "csyscfg.h"
#include "cxml.h"

CLUSTER_NODE_CFG *cluster_node_cfg_new()
{
    CLUSTER_NODE_CFG *cluster_node_cfg;
    alloc_static_mem(MD_TBD, CMPI_ANY_MODI, MM_CLUSTER_NODE_CFG, &cluster_node_cfg, LOC_CSYSCFG_0001);
    if(NULL_PTR != cluster_node_cfg)
    {
        cluster_node_cfg_init(cluster_node_cfg);
    }
    return (cluster_node_cfg);
}

EC_BOOL cluster_node_cfg_init(CLUSTER_NODE_CFG *cluster_node_cfg)
{
    cstring_init(CLUSTER_NODE_CFG_ROLE(cluster_node_cfg), NULL_PTR);
    cmap_init(CLUSTER_NODE_CFG_EXTRAS(cluster_node_cfg), (CMAP_KEY_FREE)cstring_free_1, (CMAP_VAL_FREE)cstring_free_1, LOC_CSYSCFG_0002);
    CLUSTER_NODE_CFG_TCID(cluster_node_cfg) = CMPI_ERROR_TCID;
    cvector_init(CLUSTER_NODE_CFG_RANK_VEC(cluster_node_cfg), 0, MM_UINT32, CVECTOR_LOCK_ENABLE, LOC_CSYSCFG_0003);
    return (EC_TRUE);
}

EC_BOOL cluster_node_cfg_clean(CLUSTER_NODE_CFG *cluster_node_cfg)
{
    cstring_clean(CLUSTER_NODE_CFG_ROLE(cluster_node_cfg));
    cmap_clean(CLUSTER_NODE_CFG_EXTRAS(cluster_node_cfg), LOC_CSYSCFG_0004);
    CLUSTER_NODE_CFG_TCID(cluster_node_cfg) = CMPI_ERROR_TCID;
    cvector_clean(CLUSTER_NODE_CFG_RANK_VEC(cluster_node_cfg), NULL_PTR, LOC_CSYSCFG_0005);
    return (EC_TRUE);
}

EC_BOOL cluster_node_cfg_free(CLUSTER_NODE_CFG *cluster_node_cfg)
{
    if(NULL_PTR != cluster_node_cfg)
    {
        cluster_node_cfg_clean(cluster_node_cfg);
        free_static_mem(MD_TBD, CMPI_ANY_MODI, MM_CLUSTER_NODE_CFG, cluster_node_cfg, LOC_CSYSCFG_0006);
    }
    return (EC_TRUE);
}

EC_BOOL cluster_node_cfg_check_tcid(const CLUSTER_NODE_CFG *cluster_node_cfg, const UINT32 tcid)
{
    if(tcid == CLUSTER_NODE_CFG_TCID(cluster_node_cfg))
    {
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

EC_BOOL cluster_node_cfg_check_rank_exist(const CLUSTER_NODE_CFG *cluster_node_cfg, const UINT32 rank)
{
    if(CVECTOR_ERR_POS == cvector_search_front(CLUSTER_NODE_CFG_RANK_VEC(cluster_node_cfg), (void *)rank, NULL_PTR))
    {
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

EC_BOOL cluster_node_cfg_check_role_str(const CLUSTER_NODE_CFG *cluster_node_cfg, const char *role_str)
{
    if(EC_TRUE == c_str_is_in((char *)CLUSTER_NODE_CFG_ROLE_STR(cluster_node_cfg), (const char *)":,;", role_str))
    {
        sys_log(LOGSTDOUT, "[DEBUG] cluster_node_cfg_check_role_str: cluster node role %s is in %s\n", 
                           (char *)CLUSTER_NODE_CFG_ROLE_STR(cluster_node_cfg), role_str);
        return (EC_TRUE);
    }
    
    sys_log(LOGSTDOUT, "[DEBUG] cluster_node_cfg_check_role_str: cluster node role %s is NOT in %s\n", 
                       (char *)CLUSTER_NODE_CFG_ROLE_STR(cluster_node_cfg), role_str);
    return (EC_FALSE);
}

EC_BOOL cluster_node_cfg_check_role_cstr(const CLUSTER_NODE_CFG *cluster_node_cfg, const CSTRING *role_cstr)
{
    return cstring_cmp(role_cstr, CLUSTER_NODE_CFG_ROLE(cluster_node_cfg));
}

EC_BOOL cluster_node_cfg_check_group_str(const CLUSTER_NODE_CFG *cluster_node_cfg, const char *group_str)
{
    if(EC_TRUE == c_str_is_in((char *)CLUSTER_NODE_CFG_GROUP_STR(cluster_node_cfg), (const char *)":,;", group_str))
    {
        sys_log(LOGSTDOUT, "[DEBUG] cluster_node_cfg_check_group_str: cluster node group %s is in %s\n", 
                           (char *)CLUSTER_NODE_CFG_GROUP_STR(cluster_node_cfg), group_str);
        return (EC_TRUE);
    }
    
    sys_log(LOGSTDOUT, "[DEBUG] cluster_node_cfg_check_group_str: cluster node group %s is NOT in %s\n", 
                       (char *)CLUSTER_NODE_CFG_GROUP_STR(cluster_node_cfg), group_str);
    return (EC_FALSE);
}

EC_BOOL cluster_node_cfg_check_group_cstr(const CLUSTER_NODE_CFG *cluster_node_cfg, const CSTRING *group_cstr)
{
    return cstring_cmp(group_cstr, CLUSTER_NODE_CFG_GROUP(cluster_node_cfg));
}

EC_BOOL cluster_node_cfg_collect_tcid_vec_by_role_cstr(const CLUSTER_NODE_CFG *cluster_node_cfg, const CSTRING *role_cstr, CVECTOR *tcid_vec)
{
    if(EC_TRUE == cluster_node_cfg_check_role_cstr(cluster_node_cfg, role_cstr))
    {
        cvector_push(tcid_vec, (void *)CLUSTER_NODE_CFG_TCID(cluster_node_cfg));
    }
    return (EC_TRUE);
}

EC_BOOL cluster_node_cfg_collect_tcid_vec_by_role_str(const CLUSTER_NODE_CFG *cluster_node_cfg, const char *role_str, CVECTOR *tcid_vec)
{
    if(EC_TRUE == cluster_node_cfg_check_role_str(cluster_node_cfg, role_str))
    {
        cvector_push(tcid_vec, (void *)CLUSTER_NODE_CFG_TCID(cluster_node_cfg));
    }
    return (EC_TRUE);
}


EC_BOOL cluster_node_cfg_collect_tcid_vec_by_group_cstr(const CLUSTER_NODE_CFG *cluster_node_cfg, const CSTRING *group_cstr, CVECTOR *tcid_vec)
{
    if(EC_TRUE == cluster_node_cfg_check_group_cstr(cluster_node_cfg, group_cstr))
    {
        cvector_push(tcid_vec, (void *)CLUSTER_NODE_CFG_TCID(cluster_node_cfg));
    }
    return (EC_TRUE);
}

EC_BOOL cluster_node_cfg_collect_tcid_vec_by_group_str(const CLUSTER_NODE_CFG *cluster_node_cfg, const char *group_str, CVECTOR *tcid_vec)
{
    if(EC_TRUE == cluster_node_cfg_check_group_str(cluster_node_cfg, group_str))
    {
        cvector_push(tcid_vec, (void *)CLUSTER_NODE_CFG_TCID(cluster_node_cfg));
    }
    return (EC_TRUE);
}

EC_BOOL cluster_node_cfg_collect_tcid_vec_by_role_and_group_cstr(const CLUSTER_NODE_CFG *cluster_node_cfg, const CSTRING *role_cstr, const CSTRING *group_cstr, CVECTOR *tcid_vec)
{
    if(EC_TRUE == cluster_node_cfg_check_role_cstr(cluster_node_cfg, role_cstr) 
    && EC_TRUE == cluster_node_cfg_check_group_cstr(cluster_node_cfg, group_cstr))
    {
        cvector_push(tcid_vec, (void *)CLUSTER_NODE_CFG_TCID(cluster_node_cfg));
    }
    return (EC_TRUE);
}

EC_BOOL cluster_node_cfg_collect_tcid_vec_by_role_and_group_str(const CLUSTER_NODE_CFG *cluster_node_cfg, const char *role_str, const char *group_str, CVECTOR *tcid_vec)
{
    if(EC_TRUE == cluster_node_cfg_check_role_str(cluster_node_cfg, role_str) 
    && EC_TRUE == cluster_node_cfg_check_group_str(cluster_node_cfg, group_str))
    {
        cvector_push(tcid_vec, (void *)CLUSTER_NODE_CFG_TCID(cluster_node_cfg));
    }
    return (EC_TRUE);
}

EC_BOOL cluster_node_cfg_collect_tcid_vec_all(const CLUSTER_NODE_CFG *cluster_node_cfg, CVECTOR *tcid_vec)
{
    cvector_push(tcid_vec, (void *)CLUSTER_NODE_CFG_TCID(cluster_node_cfg));
    return (EC_TRUE);
}

CSTRING *cluster_node_cfg_get_extra_val_by_key_str(const CLUSTER_NODE_CFG *cluster_node_cfg, const char *key_str)
{
    CSTRING key_cstr;

    cstring_set_str(&key_cstr, (const UINT8 *)key_str);
    return (CSTRING *)cmap_get_val_by_key(CLUSTER_NODE_CFG_EXTRAS(cluster_node_cfg), (void *)&key_cstr, (CMAP_KEY_CMP)cstring_cmp);
}

CSTRING *cluster_node_cfg_get_extra_val_by_key_cstr(const CLUSTER_NODE_CFG *cluster_node_cfg, const CSTRING *key_cstr)
{
    return (CSTRING *)cmap_get_val_by_key(CLUSTER_NODE_CFG_EXTRAS(cluster_node_cfg), (void *)key_cstr, (CMAP_KEY_CMP)cstring_cmp);
}

static void __cluster_node_cfg_extra_print_xml(LOG *log, const CMAP_NODE *extra)
{
    sys_print(log, " %s=\"%s\"" , 
                   (const char *)cstring_get_str((CSTRING *)CMAP_NODE_KEY(extra)),
                   (const char *)cstring_get_str((CSTRING *)CMAP_NODE_VAL(extra))
            );
    return;
}

static void __cluster_node_cfg_extras_print_xml(LOG *log, const CMAP *extras)
{
    CLIST_DATA *clist_data;
    CLIST_LOOP_NEXT(CMAP_NODES(extras), clist_data)
    {
        CMAP_NODE *extra;
        extra = (CMAP_NODE *)CLIST_DATA_DATA(clist_data);
        __cluster_node_cfg_extra_print_xml(log, extra);
    }
    return;
}

void cluster_node_cfg_print_xml(LOG *log, const CLUSTER_NODE_CFG *cluster_node_cfg, const UINT32 level)
{
    char *rank_str;
    char *group_str;

    rank_str  = uint32_vec_to_str(CLUSTER_NODE_CFG_RANK_VEC(cluster_node_cfg));
    group_str = (char *)CLUSTER_NODE_CFG_GROUP_STR(cluster_node_cfg);

    if(NULL_PTR != rank_str)
    {    
        c_ident_print(log, level);
        sys_print(log, "<node");
        sys_print(log, " role=\"%s\"" , (const char *)CLUSTER_NODE_CFG_ROLE_STR(cluster_node_cfg));
        sys_print(log, " tcid=\"%s\"" , (const char *)CLUSTER_NODE_CFG_TCID_STR(cluster_node_cfg));
        sys_print(log, " rank=\"%s\"" , rank_str);
        __cluster_node_cfg_extras_print_xml(log, CLUSTER_NODE_CFG_EXTRAS(cluster_node_cfg));
        sys_print(log, "/>\n");

        safe_free(rank_str, LOC_CSYSCFG_0007);
    }
    else
    {
        c_ident_print(log, level);
        sys_print(log, "<node");
        sys_print(log, " role=\"%s\"" , (const char *)CLUSTER_NODE_CFG_ROLE_STR(cluster_node_cfg));
        sys_print(log, " tcid=\"%s\"" , (const char *)CLUSTER_NODE_CFG_TCID_STR(cluster_node_cfg));
        sys_print(log, " rank=\"\""   );
        __cluster_node_cfg_extras_print_xml(log, CLUSTER_NODE_CFG_EXTRAS(cluster_node_cfg));
        sys_print(log, "/>\n");    
    }

    return;
}

CLUSTER_CFG *cluster_cfg_new()
{
    CLUSTER_CFG *cluster_cfg;
    alloc_static_mem(MD_TBD, CMPI_ANY_MODI, MM_CLUSTER_CFG, &cluster_cfg, LOC_CSYSCFG_0008);
    if(NULL_PTR != cluster_cfg)
    {
        cluster_cfg_init(cluster_cfg);
    }
    return (cluster_cfg);
}

EC_BOOL cluster_cfg_init(CLUSTER_CFG *cluster_cfg)
{
    CLUSTER_CFG_ID(cluster_cfg) = CLUSTER_ID_ERROR;
    cstring_init(CLUSTER_CFG_NAME(cluster_cfg), NULL_PTR);
    CLUSTER_CFG_MODEL(cluster_cfg) = MODEL_TYPE_ERROR;
    cmap_init(CLUSTER_CFG_EXTRAS(cluster_cfg), (CMAP_KEY_FREE)cstring_free_1, (CMAP_VAL_FREE)cstring_free_1, LOC_CSYSCFG_0009);
    cvector_init(CLUSTER_CFG_NODES(cluster_cfg), 0, MM_CLUSTER_NODE_CFG, CVECTOR_LOCK_ENABLE, LOC_CSYSCFG_0010);
    return (EC_TRUE);
}

EC_BOOL cluster_cfg_clean(CLUSTER_CFG *cluster_cfg)
{
    CLUSTER_CFG_ID(cluster_cfg) = CLUSTER_ID_ERROR;
    cstring_clean(CLUSTER_CFG_NAME(cluster_cfg));
    CLUSTER_CFG_MODEL(cluster_cfg) = MODEL_TYPE_ERROR;
    cmap_clean(CLUSTER_CFG_EXTRAS(cluster_cfg), LOC_CSYSCFG_0011);
    cvector_clean(CLUSTER_CFG_NODES(cluster_cfg), (CVECTOR_DATA_CLEANER)cluster_node_cfg_free, LOC_CSYSCFG_0012);
    return (EC_TRUE);
}

EC_BOOL cluster_cfg_free(CLUSTER_CFG *cluster_cfg)
{
    if(NULL_PTR != cluster_cfg)
    {
        cluster_cfg_clean(cluster_cfg);
        free_static_mem(MD_TBD, CMPI_ANY_MODI, MM_CLUSTER_CFG, cluster_cfg, LOC_CSYSCFG_0013);
    }
    return (EC_TRUE);
}

EC_BOOL cluster_cfg_check_id(const CLUSTER_CFG *cluster_cfg, const UINT32 id)
{
    if(id == CLUSTER_CFG_ID(cluster_cfg))
    {
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

EC_BOOL cluster_cfg_check_name_str(const CLUSTER_CFG *cluster_cfg, const char *name_str)
{
    if(0 == strcmp(name_str, (char *)CLUSTER_CFG_NAME_STR(cluster_cfg)))
    {
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

EC_BOOL cluster_cfg_check_name_cstr(const CLUSTER_CFG *cluster_cfg, const CSTRING *name_cstr)
{
    return cstring_cmp(name_cstr, CLUSTER_CFG_NAME(cluster_cfg));
}

EC_BOOL cluster_cfg_check_tcid_exist(const CLUSTER_CFG *cluster_cfg, const UINT32 tcid)
{
    if(CVECTOR_ERR_POS == cvector_search_front(CLUSTER_CFG_NODES(cluster_cfg), (void *)tcid, (CVECTOR_DATA_CMP)cluster_node_cfg_check_tcid))
    {
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

CLUSTER_NODE_CFG *cluster_cfg_search_by_tcid_rank(const CLUSTER_CFG *cluster_cfg, const UINT32 tcid, const UINT32 rank)
{
    CVECTOR *cluster_nodes;
    UINT32 pos;

    cluster_nodes = (CVECTOR *)CLUSTER_CFG_NODES(cluster_cfg);
    CVECTOR_LOCK(cluster_nodes, LOC_CSYSCFG_0014);
    for(pos = 0; pos < cvector_size(cluster_nodes); pos ++)
    {
        CLUSTER_NODE_CFG *cluster_node_cfg;
        cluster_node_cfg = (CLUSTER_NODE_CFG *)cvector_get_no_lock(cluster_nodes, pos);
        if(NULL_PTR == cluster_node_cfg)
        {
            continue;
        }

        if(EC_TRUE == cluster_node_cfg_check_tcid(cluster_node_cfg, tcid)
        && EC_TRUE == cluster_node_cfg_check_rank_exist(cluster_node_cfg, rank))
        {
            CVECTOR_UNLOCK(cluster_nodes, LOC_CSYSCFG_0015);
            return (cluster_node_cfg);
        }        
    }
    CVECTOR_UNLOCK(cluster_nodes, LOC_CSYSCFG_0016);
    return (NULL_PTR);
}

EC_BOOL cluster_cfg_check_duplicate(const CLUSTER_CFG *cluster_cfg_1st, const CLUSTER_CFG *cluster_cfg_2nd)
{
    if(CLUSTER_CFG_ID(cluster_cfg_1st) == CLUSTER_CFG_ID(cluster_cfg_2nd))
    {
        return (EC_TRUE);
    }

    if(EC_TRUE == cstring_cmp(CLUSTER_CFG_NAME(cluster_cfg_1st), CLUSTER_CFG_NAME(cluster_cfg_2nd)))
    {
        return (EC_TRUE);
    }   

    return (EC_FALSE);
}

EC_BOOL cluster_cfg_collect_tcid_vec_by_group_cstr(const CLUSTER_CFG *cluster_cfg, const UINT32 model, const CSTRING *group_cstr, CVECTOR *tcid_vec)
{
    EC_BOOL ret;

    if(MODEL_TYPE_ANY != model && model != CLUSTER_CFG_MODEL(cluster_cfg))
    {
        return (EC_TRUE);
    }

    ret = EC_FALSE;
    return cvector_loop((CVECTOR *)CLUSTER_CFG_NODES(cluster_cfg), (void *)&ret, NULL_PTR, 
                        (UINT32)3, 
                        (UINT32)0, 
                        (UINT32)cluster_node_cfg_collect_tcid_vec_by_group_cstr,
                        NULL_PTR,
                        group_cstr,
                        tcid_vec);
}

EC_BOOL cluster_cfg_collect_tcid_vec_by_group_str(const CLUSTER_CFG *cluster_cfg, const UINT32 model, const char *group_str, CVECTOR *tcid_vec)
{
    EC_BOOL ret;

    if(MODEL_TYPE_ANY != model && model != CLUSTER_CFG_MODEL(cluster_cfg))
    {
        return (EC_TRUE);
    }

    ret = EC_FALSE;
    return cvector_loop((CVECTOR *)CLUSTER_CFG_NODES(cluster_cfg), (void *)&ret, NULL_PTR, 
                        (UINT32)3, 
                        (UINT32)0, 
                        (UINT32)cluster_node_cfg_collect_tcid_vec_by_group_str,
                        NULL_PTR,
                        group_str,
                        tcid_vec);
}

EC_BOOL cluster_cfg_collect_tcid_vec_by_role_cstr(const CLUSTER_CFG *cluster_cfg, const UINT32 model, const CSTRING *role_cstr, CVECTOR *tcid_vec)
{
    EC_BOOL ret;

    if(MODEL_TYPE_ANY != model && model != CLUSTER_CFG_MODEL(cluster_cfg))
    {
        return (EC_TRUE);
    }
    
    ret = EC_FALSE;
    return cvector_loop((CVECTOR *)CLUSTER_CFG_NODES(cluster_cfg), (void *)&ret, NULL_PTR, 
                        (UINT32)3, 
                        (UINT32)0, 
                        (UINT32)cluster_node_cfg_collect_tcid_vec_by_role_cstr,
                        NULL_PTR,
                        role_cstr,
                        tcid_vec);
}

EC_BOOL cluster_cfg_collect_tcid_vec_by_role_str(const CLUSTER_CFG *cluster_cfg, const UINT32 model, const char *role_str, CVECTOR *tcid_vec)
{
    EC_BOOL ret;

    if(MODEL_TYPE_ANY != model && model != CLUSTER_CFG_MODEL(cluster_cfg))
    {
        return (EC_TRUE);
    }    

    ret = EC_FALSE;
    return cvector_loop((CVECTOR *)CLUSTER_CFG_NODES(cluster_cfg), (void *)&ret, NULL_PTR, 
                        (UINT32)3, 
                        (UINT32)0, 
                        (UINT32)cluster_node_cfg_collect_tcid_vec_by_role_str,
                        NULL_PTR,
                        role_str,
                        tcid_vec);
}

EC_BOOL cluster_cfg_collect_tcid_vec_by_role_and_group_cstr(const CLUSTER_CFG *cluster_cfg, const UINT32 model, const CSTRING *role_cstr, const CSTRING *group_cstr, CVECTOR *tcid_vec)
{
    EC_BOOL ret;

    if(MODEL_TYPE_ANY != model && model != CLUSTER_CFG_MODEL(cluster_cfg))
    {
        return (EC_TRUE);
    }    

    ret = EC_FALSE;
    return cvector_loop((CVECTOR *)CLUSTER_CFG_NODES(cluster_cfg), (void *)&ret, NULL_PTR, 
                        (UINT32)4, 
                        (UINT32)0, 
                        (UINT32)cluster_node_cfg_collect_tcid_vec_by_role_and_group_cstr,
                        NULL_PTR,
                        role_cstr,
                        group_cstr,
                        tcid_vec);
}

EC_BOOL cluster_cfg_collect_tcid_vec_by_role_and_group_str(const CLUSTER_CFG *cluster_cfg, const UINT32 model, const char *role_str, const char *group_str, CVECTOR *tcid_vec)
{
    EC_BOOL ret;

    if(MODEL_TYPE_ANY != model && model != CLUSTER_CFG_MODEL(cluster_cfg))
    {
        return (EC_TRUE);
    }    

    ret = EC_FALSE;
    return cvector_loop((CVECTOR *)CLUSTER_CFG_NODES(cluster_cfg), (void *)&ret, NULL_PTR, 
                        (UINT32)4, 
                        (UINT32)0, 
                        (UINT32)cluster_node_cfg_collect_tcid_vec_by_role_and_group_str,
                        NULL_PTR,
                        role_str,
                        group_str,
                        tcid_vec);
}

CSTRING *cluster_cfg_get_extra_val_by_key_str(const CLUSTER_CFG *cluster_cfg, const char *key_str)
{
    CSTRING key_cstr;

    cstring_set_str(&key_cstr, (const UINT8 *)key_str);
    return (CSTRING *)cmap_get_val_by_key(CLUSTER_CFG_EXTRAS(cluster_cfg), (void *)&key_cstr, (CMAP_KEY_CMP)cstring_cmp);
}

CSTRING *cluster_cfg_get_extra_val_by_key_cstr(const CLUSTER_CFG *cluster_cfg, const CSTRING *key_cstr)
{
    return (CSTRING *)cmap_get_val_by_key(CLUSTER_CFG_EXTRAS(cluster_cfg), (void *)key_cstr, (CMAP_KEY_CMP)cstring_cmp);
}

CSTRING *cluster_cfg_get_node_extra_val_by_key_str(const CLUSTER_CFG *cluster_cfg, const UINT32 tcid, const UINT32 rank, const char *key_str)
{
    CLUSTER_NODE_CFG *cluster_node_cfg;

    cluster_node_cfg = cluster_cfg_search_by_tcid_rank(cluster_cfg, tcid, rank);
    if(NULL_PTR == cluster_node_cfg)
    {
        return (NULL_PTR);
    }

    return cluster_node_cfg_get_extra_val_by_key_str(cluster_node_cfg, key_str);    
}

CSTRING *cluster_cfg_get_node_extra_val_by_key_cstr(const CLUSTER_CFG *cluster_cfg, const UINT32 tcid, const UINT32 rank, const CSTRING *key_cstr)
{
    CLUSTER_NODE_CFG *cluster_node_cfg;

    cluster_node_cfg = cluster_cfg_search_by_tcid_rank(cluster_cfg, tcid, rank);
    if(NULL_PTR == cluster_node_cfg)
    {
        return (NULL_PTR);
    }

    return cluster_node_cfg_get_extra_val_by_key_cstr(cluster_node_cfg, key_cstr);    
}

static void __cluster_cfg_extra_print_xml(LOG *log, const CMAP_NODE *extra)
{
    sys_print(log, " %s=\"%s\"" , 
                   (const char *)cstring_get_str((CSTRING *)CMAP_NODE_KEY(extra)),
                   (const char *)cstring_get_str((CSTRING *)CMAP_NODE_VAL(extra))
            );
    return;
}

static void __cluster_cfg_extras_print_xml(LOG *log, const CMAP *extras)
{
    CLIST_DATA *clist_data;
    CLIST_LOOP_NEXT(CMAP_NODES(extras), clist_data)
    {
        CMAP_NODE *extra;
        extra = (CMAP_NODE *)CLIST_DATA_DATA(clist_data);
        __cluster_cfg_extra_print_xml(log, extra);
    }
    return;
}

void cluster_cfg_print_xml(LOG *log, const CLUSTER_CFG *cluster_cfg, const UINT32 level)
{
    const char *mode_str;

    switch(CLUSTER_CFG_MODEL(cluster_cfg))
    {
        case MODEL_TYPE_MASTER_SLAVE:
            mode_str = (const char *)"master_slave";
            break;
        case MODEL_TYPE_CROSS_CONNEC:
            mode_str = (const char *)"cross";
            break;
        case MODEL_TYPE_HSDFS_CONNEC:
            mode_str = (const char *)"hsdfs";
            break;    
        case MODEL_TYPE_HSBGT_CONNEC:
            mode_str = (const char *)"hsbgt";
            break;              
        default:
            mode_str = (const char *)"UNKNOWN";
    }

    c_ident_print(log, level);
    sys_print(log, "<cluster");
    sys_print(log, " id=\"%ld\""  , CLUSTER_CFG_ID(cluster_cfg));
    sys_print(log, " name=\"%s\"" , (const char *)CLUSTER_CFG_NAME_STR(cluster_cfg));
    sys_print(log, " model=\"%s\"", mode_str);
    __cluster_cfg_extras_print_xml(log, CLUSTER_CFG_EXTRAS(cluster_cfg));
    sys_print(log, ">\n");

    cvector_print_level(log, CLUSTER_CFG_NODES(cluster_cfg), level + 1, (CVECTOR_DATA_LEVEL_PRINT)cluster_node_cfg_print_xml);

    c_ident_print(log, level);
    sys_print(log, "</cluster>\n");
    return;
}

MCAST_CFG *mcast_cfg_new()
{
    MCAST_CFG *mcast_cfg;
    alloc_static_mem(MD_TBD, CMPI_ANY_MODI, MM_MCAST_CFG, &mcast_cfg, LOC_CSYSCFG_0017);
    if(NULL_PTR != mcast_cfg)
    {
        mcast_cfg_init(mcast_cfg);
    }
    return (mcast_cfg);
}

EC_BOOL mcast_cfg_init(MCAST_CFG *mcast_cfg)
{
    MCAST_CFG_TYPE(mcast_cfg)     = MCAST_TYPE_IS_ERR;
    MCAST_CFG_TCID(mcast_cfg)     = CMPI_ERROR_TCID;
    MCAST_CFG_IPADDR(mcast_cfg)    = CMPI_ERROR_IPADDR;
    MCAST_CFG_PORT(mcast_cfg)      = CMPI_ERROR_SRVPORT;
    MCAST_CFG_EXPIRE(mcast_cfg)    = CBTIMER_NEVER_EXPIRE;
    MCAST_CFG_TIMEOUT(mcast_cfg)   = MCAST_SRV_DEFAULT_INTERVAL;
    MCAST_CFG_AUTO_FLAG(mcast_cfg) = MCAST_SRV_NOT_AUTO_BOOTUP;
    return (EC_TRUE);
}

EC_BOOL mcast_cfg_clean(MCAST_CFG *mcast_cfg)
{
    MCAST_CFG_TYPE(mcast_cfg)     = MCAST_TYPE_IS_ERR;
    MCAST_CFG_TCID(mcast_cfg)     = CMPI_ERROR_TCID;
    MCAST_CFG_IPADDR(mcast_cfg)    = CMPI_ERROR_IPADDR;
    MCAST_CFG_PORT(mcast_cfg)      = CMPI_ERROR_SRVPORT;
    MCAST_CFG_EXPIRE(mcast_cfg)    = CBTIMER_NEVER_EXPIRE;
    MCAST_CFG_TIMEOUT(mcast_cfg)   = MCAST_SRV_DEFAULT_INTERVAL;
    MCAST_CFG_AUTO_FLAG(mcast_cfg) = MCAST_SRV_NOT_AUTO_BOOTUP;

    return (EC_TRUE);
}

EC_BOOL mcast_cfg_free(MCAST_CFG *mcast_cfg)
{
    if(NULL_PTR != mcast_cfg)
    {
        mcast_cfg_clean(mcast_cfg);
        free_static_mem(MD_TBD, CMPI_ANY_MODI, MM_MCAST_CFG, mcast_cfg, LOC_CSYSCFG_0018);
    }
    return (EC_TRUE);
}

void mcast_cfg_body_print_xml(LOG *log, const MCAST_CFG *mcast_cfg, const UINT32 level)
{
    const char *type_str;
    const char *auto_str;

    switch(MCAST_CFG_TYPE(mcast_cfg))
    {
        case MCAST_TYPE_IS_MASTER:
            type_str = "master";
            break;
        case MCAST_TYPE_IS_SLAVE:
            type_str = "slave";
            break;
        default:
            type_str = "UNKNOWN";
    }

    switch(MCAST_CFG_AUTO_FLAG(mcast_cfg))
    {
        case MCAST_SRV_WILL_AUTO_BOOTUP:
            auto_str = "true";
            break;
        case MCAST_SRV_NOT_AUTO_BOOTUP:
            auto_str = "false";
            break;
        default:
            auto_str = "UNKNOWN";
    }

    c_ident_print(log, level);

    sys_print(log, "<udp");
    sys_print(log, " type=\"%s\""          , type_str);
    sys_print(log, " tcid=\"%s\""          , MCAST_CFG_TCID_STR(mcast_cfg));
    sys_print(log, " srvipaddr=\"%s\""     , MCAST_CFG_IPADDR_STR(mcast_cfg));
    sys_print(log, " srvport=\"%ld\""      , MCAST_CFG_PORT(mcast_cfg));
    sys_print(log, " expire=\"%ld\""       , MCAST_CFG_EXPIRE(mcast_cfg));
    sys_print(log, " timeout=\"%ld\""      , MCAST_CFG_TIMEOUT(mcast_cfg));
    sys_print(log, " auto=\"%s\""          , auto_str);
    sys_print(log, "/>\n");
    return;
}

void mcast_cfg_print_xml(LOG *log, const MCAST_CFG *mcast_cfg, const UINT32 level)
{
    if(CMPI_ERROR_TCID != MCAST_CFG_TCID(mcast_cfg))
    {
        c_ident_print(log, level);
        sys_print(log, "<udpMulticastConfig>\n");

        mcast_cfg_body_print_xml(log, mcast_cfg, level + 1);

        c_ident_print(log, level);
        sys_print(log, "</udpMulticastConfig>\n");
    }
    return;
}

BCAST_DHCP_CFG *bcast_dhcp_cfg_new()
{
    BCAST_DHCP_CFG *bcast_dhcp_cfg;
    alloc_static_mem(MD_TBD, CMPI_ANY_MODI, MM_BCAST_DHCP_CFG, &bcast_dhcp_cfg, LOC_CSYSCFG_0019);
    if(NULL_PTR != bcast_dhcp_cfg)
    {
        bcast_dhcp_cfg_init(bcast_dhcp_cfg);
    }
    return (bcast_dhcp_cfg);
}

EC_BOOL bcast_dhcp_cfg_init(BCAST_DHCP_CFG *bcast_dhcp_cfg)
{
    BCAST_DHCP_CFG_TYPE(bcast_dhcp_cfg)     = BCAST_DHCP_TYPE_IS_ERR;
    BCAST_DHCP_CFG_TCID(bcast_dhcp_cfg)     = CMPI_ERROR_TCID;
    cstring_init(BCAST_DHCP_NETCARD(bcast_dhcp_cfg), (UINT8 *)"eth0");/*default*/
    BCAST_DHCP_CFG_SUBNET(bcast_dhcp_cfg)    = CMPI_ERROR_IPADDR;
    BCAST_DHCP_CFG_MASK(bcast_dhcp_cfg)      = CMPI_ERROR_MASK;
    BCAST_DHCP_CFG_AUTO_FLAG(bcast_dhcp_cfg) = BCAST_DHCP_SRV_NOT_AUTO_BOOTUP;
    return (EC_TRUE);
}

EC_BOOL bcast_dhcp_cfg_clean(BCAST_DHCP_CFG *bcast_dhcp_cfg)
{
    BCAST_DHCP_CFG_TYPE(bcast_dhcp_cfg)     = BCAST_DHCP_TYPE_IS_ERR;
    BCAST_DHCP_CFG_TCID(bcast_dhcp_cfg)     = CMPI_ERROR_TCID;
    cstring_clean(BCAST_DHCP_NETCARD(bcast_dhcp_cfg));
    BCAST_DHCP_CFG_SUBNET(bcast_dhcp_cfg)    = CMPI_ERROR_IPADDR;
    BCAST_DHCP_CFG_MASK(bcast_dhcp_cfg)      = CMPI_ERROR_SRVPORT;
    BCAST_DHCP_CFG_AUTO_FLAG(bcast_dhcp_cfg) = BCAST_DHCP_SRV_NOT_AUTO_BOOTUP;

    return (EC_TRUE);
}

EC_BOOL bcast_dhcp_cfg_free(BCAST_DHCP_CFG *bcast_dhcp_cfg)
{
    if(NULL_PTR != bcast_dhcp_cfg)
    {
        bcast_dhcp_cfg_clean(bcast_dhcp_cfg);
        free_static_mem(MD_TBD, CMPI_ANY_MODI, MM_BCAST_DHCP_CFG, bcast_dhcp_cfg, LOC_CSYSCFG_0020);
    }
    return (EC_TRUE);
}

void bcast_dhcp_cfg_body_print_xml(LOG *log, const BCAST_DHCP_CFG *bcast_dhcp_cfg, const UINT32 level)
{
    const char *type_str;
    const char *auto_str;

    switch(BCAST_DHCP_CFG_TYPE(bcast_dhcp_cfg))
    {
        case BCAST_DHCP_TYPE_IS_MASTER:
            type_str = "master";
            break;
        case BCAST_DHCP_TYPE_IS_SLAVE:
            type_str = "slave";
            break;
        default:
            type_str = "UNKNOWN";
    }

    switch(BCAST_DHCP_CFG_AUTO_FLAG(bcast_dhcp_cfg))
    {
        case BCAST_DHCP_SRV_WILL_AUTO_BOOTUP:
            auto_str = "true";
            break;
        case BCAST_DHCP_SRV_NOT_AUTO_BOOTUP:
            auto_str = "false";
            break;
        default:
            auto_str = "UNKNOWN";
    }

    c_ident_print(log, level);

    sys_print(log, "<dhcp");
    sys_print(log, " type=\"%s\""          , type_str);
    sys_print(log, " tcid=\"%s\""          , BCAST_DHCP_CFG_TCID_STR(bcast_dhcp_cfg));
    sys_print(log, " eth=\"%s\""           , (char *)BCAST_DHCP_NETCARD_STR(bcast_dhcp_cfg));
    sys_print(log, " subnet=\"%s\""        , BCAST_DHCP_CFG_SUBNET_STR(bcast_dhcp_cfg));
    //sys_print(log, " mask=\"%s\""          , BCAST_DHCP_CFG_MASK_STR(bcast_dhcp_cfg));
    sys_print(log, " mask=\"%ld\""         , (UINT32)ipv4_subnet_mask_prefix(BCAST_DHCP_CFG_MASK(bcast_dhcp_cfg)));
    sys_print(log, " auto=\"%s\""          , auto_str);
    sys_print(log, "/>\n");
    return;
}

void bcast_dhcp_cfg_print_xml(LOG *log, const BCAST_DHCP_CFG *bcast_dhcp_cfg, const UINT32 level)
{
    if(CMPI_ERROR_TCID != BCAST_DHCP_CFG_TCID(bcast_dhcp_cfg))
    {
        c_ident_print(log, level);
        sys_print(log, "<udpBroadcastDHCPConfig>\n");

        bcast_dhcp_cfg_body_print_xml(log, bcast_dhcp_cfg, level + 1);

        c_ident_print(log, level);
        sys_print(log, "</udpBroadcastDHCPConfig>\n");
    }
    return;
}

GANGLIA_CFG *ganglia_cfg_new()
{
    GANGLIA_CFG *ganglia_cfg;
    alloc_static_mem(MD_TBD, CMPI_ANY_MODI, MM_GANGLIA_CFG, &ganglia_cfg, LOC_CSYSCFG_0021);
    if(NULL_PTR != ganglia_cfg)
    {
        ganglia_cfg_init(ganglia_cfg);
    }
    return (ganglia_cfg);
}

EC_BOOL ganglia_cfg_init(GANGLIA_CFG *ganglia_cfg)
{
    GANGLIA_CFG_TCID(ganglia_cfg)     = CMPI_ERROR_TCID;
    GANGLIA_CFG_IPADDR(ganglia_cfg)    = CMPI_ERROR_IPADDR;
    GANGLIA_CFG_PORT(ganglia_cfg)      = CMPI_ERROR_SRVPORT;
    GANGLIA_CFG_AUTO_FLAG(ganglia_cfg) = GANGLIA_NOT_AUTO_BOOTUP;
    return (EC_TRUE);
}

EC_BOOL ganglia_cfg_clean(GANGLIA_CFG *ganglia_cfg)
{
    GANGLIA_CFG_TCID(ganglia_cfg)     = CMPI_ERROR_TCID;
    GANGLIA_CFG_IPADDR(ganglia_cfg)    = CMPI_ERROR_IPADDR;
    GANGLIA_CFG_PORT(ganglia_cfg)      = CMPI_ERROR_SRVPORT;
    GANGLIA_CFG_AUTO_FLAG(ganglia_cfg) = GANGLIA_NOT_AUTO_BOOTUP;

    return (EC_TRUE);
}

EC_BOOL ganglia_cfg_free(GANGLIA_CFG *ganglia_cfg)
{
    if(NULL_PTR != ganglia_cfg)
    {
        ganglia_cfg_clean(ganglia_cfg);
        free_static_mem(MD_TBD, CMPI_ANY_MODI, MM_GANGLIA_CFG, ganglia_cfg, LOC_CSYSCFG_0022);
    }
    return (EC_TRUE);
}

void ganglia_cfg_body_print_xml(LOG *log, const GANGLIA_CFG *ganglia_cfg, const UINT32 level)
{
    const char *auto_str;
    switch(GANGLIA_CFG_AUTO_FLAG(ganglia_cfg))
    {
        case GANGLIA_WILL_AUTO_BOOTUP:
            auto_str = "true";
            break;
        case GANGLIA_NOT_AUTO_BOOTUP:
            auto_str = "false";
            break;
        default:
            auto_str = "UNKNOWN";
    }

    c_ident_print(log, level);

    sys_print(log, "<ganglia");
    sys_print(log, " tcid=\"%s\""   , (const char *)GANGLIA_CFG_TCID_STR(ganglia_cfg));
    sys_print(log, " ip=\"%s\""     , (char *)GANGLIA_CFG_IPADDR_STR(ganglia_cfg));
    sys_print(log, " port=\"%ld\""  , GANGLIA_CFG_PORT(ganglia_cfg));
    sys_print(log, " auto=\"%s\""   , auto_str);
    sys_print(log, "/>\n");
    return;
}

void ganglia_cfg_print_xml(LOG *log, const GANGLIA_CFG *ganglia_cfg, const UINT32 level)
{
    c_ident_print(log, level);
    sys_print(log, "<gangliaConfig>\n");

    ganglia_cfg_body_print_xml(log, ganglia_cfg, level + 1);

    c_ident_print(log, level);
    sys_print(log, "</gangliaConfig>\n");
    return;
}

void cparacfg_thread_cfg_print_xml(LOG *log, const CPARACFG *cparacfg, const UINT32 level)
{
    c_ident_print(log, level);

    sys_print(log, "<threadConfig");
    sys_print(log, " maxReqThreadNum=\"%ld\""          , CPARACFG_TASK_REQ_THREAD_MAX_NUM(cparacfg));
    sys_print(log, " maxRspThreadNum=\"%ld\""          , CPARACFG_TASK_RSP_THREAD_MAX_NUM(cparacfg));
    sys_print(log, " maxStackSize=\"%ld\""             , CPARACFG_CTHREAD_STACK_MAX_SIZE(cparacfg));
    sys_print(log, " stackGuardSize=\"%ld\""           , CPARACFG_CTHREAD_STACK_GUARD_SIZE(cparacfg));
#if 0/*not release yet*/
    sys_print(log, " taskReqHandleThreadSwitch=\"%s\"" , CPARACFG_TASK_REQ_HANDLE_THREAD_SWITCH_STR(cparacfg));
    sys_print(log, " taskReqDecodeThreadSwitch=\"%s\"" , CPARACFG_TASK_REQ_DECODE_THREAD_SWITCH_STR(cparacfg));
    sys_print(log, " taskRspDecodeThreadSwitch=\"%s\"" , CPARACFG_TASK_RSP_DECODE_THREAD_SWITCH_STR(cparacfg));
    sys_print(log, " taskFwdDecodeThreadSwitch=\"%s\"" , CPARACFG_TASK_FWD_DECODE_THREAD_SWITCH_STR(cparacfg));
#endif
    sys_print(log, "/>\n");

    return ;
}

void cparacfg_csocket_cfg_print_xml(LOG *log, const CPARACFG *cparacfg, const UINT32 level)
{
    c_ident_print(log, level);

    sys_print(log, "<socketConfig");
    sys_print(log, " socketSendBuffSize=\"%ld\""        , CPARACFG_CSOCKET_SOSNDBUFF_SIZE(cparacfg));
    sys_print(log, " socketRecvBuffSize=\"%ld\""        , CPARACFG_CSOCKET_SORCVBUFF_SIZE(cparacfg));
    sys_print(log, " socketCrbuffSize=\"%ld\" "         , CPARACFG_CSOCKET_CRBUFF_SIZE(cparacfg));
    sys_print(log, " socketConnectionNum=\"%ld\""       , CPARACFG_CSOCKET_CNODE_NUM(cparacfg));
    sys_print(log, "/>\n");

    return;
}

void cparacfg_log_cfg_print_xml(LOG *log, const CPARACFG *cparacfg, const UINT32 level)
{
    c_ident_print(log, level);
    sys_print(log, "<logConfig");
    sys_print(log, " logMaxRecords=\"%ld\""          , CPARACFG_FILE_LOG_MAX_RECORDS(cparacfg));
    sys_print(log, " logNameWithDataSwitch=\"%s\""   , CPARACFG_FILE_LOG_NAME_WITH_DATE_SWITCH_STR(cparacfg));
    sys_print(log, "/>\n");
    return;
}

void cparacfg_print_xml(LOG *log, const CPARACFG *cparacfg, const UINT32 level)
{
    c_ident_print(log, level);
    sys_print(log, "<paraConfig tcid=\"%s\" rank=\"%ld\">\n",
                    c_word_to_ipv4(CPARACFG_TCID(cparacfg)),
                    CPARACFG_RANK(cparacfg));

    cparacfg_thread_cfg_print_xml (log, cparacfg, level + 1);
#if 0/*not release yet*/
    if(CMPI_FWD_RANK == CPARACFG_RANK(cparacfg))
    {
        cparacfg_csocket_cfg_print_xml(log, cparacfg, level + 1);
    }
    cparacfg_log_cfg_print_xml(log, cparacfg, level + 1);
#endif
    c_ident_print(log, level);
    sys_print(log, "</paraConfig>\n");
    return;
}

void paras_cfg_print_xml(LOG *log, const CVECTOR *paras_cfg, const UINT32 level)
{
    UINT32 num;
    UINT32 pos;

    num = cvector_size(paras_cfg);
    if(0 == num)
    {
        return;
    }

    c_ident_print(log, level);
    sys_print(log, "<parasConfig>\n");

    CVECTOR_LOCK(paras_cfg, LOC_CSYSCFG_0023);
    for(pos = 0; pos < num; pos ++)
    {
        CPARACFG *cparacfg;

        cparacfg = (CPARACFG *)cvector_get_no_lock(paras_cfg, pos);
        if(NULL_PTR == cparacfg)
        {
            continue;
        }

        cparacfg_print_xml(log, cparacfg, level + 1);
    }
    CVECTOR_UNLOCK(paras_cfg, LOC_CSYSCFG_0024);

    c_ident_print(log, level);
    sys_print(log, "</parasConfig>\n");
    return;
}

MACIP_CFG *macip_cfg_new()
{
    MACIP_CFG *macip_cfg;
    alloc_static_mem(MD_TBD, CMPI_ANY_MODI, MM_MACIP_CFG, &macip_cfg, LOC_CSYSCFG_0025);
    if(NULL_PTR != macip_cfg)
    {
        macip_cfg_init(macip_cfg);
    }
    return (macip_cfg);
}

EC_BOOL macip_cfg_init(MACIP_CFG *macip_cfg)
{
    BSET(MACIP_CFG_MAC_ADDR(macip_cfg), 0, MACIP_CFG_MAC_SIZE);
    MACIP_CFG_IPV4_ADDR(macip_cfg) = CMPI_ERROR_IPADDR;
    return (EC_TRUE);
}

EC_BOOL macip_cfg_clean(MACIP_CFG *macip_cfg)
{
    BSET(MACIP_CFG_MAC_ADDR(macip_cfg), 0, MACIP_CFG_MAC_SIZE);
    MACIP_CFG_IPV4_ADDR(macip_cfg) = CMPI_ERROR_IPADDR;
    return (EC_TRUE);
}

EC_BOOL macip_cfg_free(MACIP_CFG *macip_cfg)
{
    if(NULL_PTR != macip_cfg)
    {
        macip_cfg_clean(macip_cfg);
        free_static_mem(MD_TBD, CMPI_ANY_MODI, MM_MACIP_CFG, macip_cfg, LOC_CSYSCFG_0026);
    }
    return (EC_TRUE);
}

EC_BOOL macip_cfg_set(MACIP_CFG *macip_cfg, const UINT8 *mac_addr, const UINT32 ipv4_addr)
{
    BCOPY(mac_addr, MACIP_CFG_MAC_ADDR(macip_cfg), MACIP_CFG_MAC_SIZE);
    MACIP_CFG_IPV4_ADDR(macip_cfg) = ipv4_addr;
    return (EC_TRUE);
}

EC_BOOL macip_cfg_check_ipv4_addr(const MACIP_CFG *macip_cfg, const UINT32 ipv4_addr)
{
    if(ipv4_addr == MACIP_CFG_IPV4_ADDR(macip_cfg))
    {
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

EC_BOOL macip_cfg_check_mac_addr(const MACIP_CFG *macip_cfg, const UINT8 * mac_addr)
{
    if(0 == BCMP(mac_addr, MACIP_CFG_MAC_ADDR(macip_cfg), MACIP_CFG_MAC_SIZE))
    {
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

EC_BOOL macip_cfg_has_null_mac_addr(const MACIP_CFG *macip_cfg)
{
    UINT32 pos;
    const UINT8 *mac_addr;

    mac_addr = MACIP_CFG_MAC_ADDR(macip_cfg);
    for(pos = 0; pos < MACIP_CFG_MAC_SIZE; pos ++)
    {
        if(0 != mac_addr[ pos ])
        {
            return (EC_FALSE);
        }
    }
    return (EC_TRUE);
}

void macip_cfg_print_xml(LOG *log, const MACIP_CFG *macip_cfg, const UINT32 level)
{
    c_ident_print(log, level);

    sys_print(log, "<map");
    sys_print(log, " ipaddr=\"%s\""        , MACIP_CFG_IPV4_ADDR_STR(macip_cfg));
    sys_print(log, " macaddr=\"%s\""       , MACIP_CFG_MAC_ADDR_STR(macip_cfg));
    sys_print(log, "/>\n");
    return;
}

void macip_cfg_vec_print_xml(LOG *log, const CVECTOR *macip_cfg_vec, const UINT32 level)
{
    UINT32 num;
    UINT32 pos;

    num = cvector_size(macip_cfg_vec);
    if(0 == num)
    {
        return;
    }

    c_ident_print(log, level);
    sys_print(log, "<macIpMapsConfig>\n");

    CVECTOR_LOCK(macip_cfg_vec, LOC_CSYSCFG_0027);
    for(pos = 0; pos < num; pos ++)
    {
        MACIP_CFG *macip_cfg;

        macip_cfg = (MACIP_CFG *)cvector_get_no_lock(macip_cfg_vec, pos);
        if(NULL_PTR == macip_cfg)
        {
            continue;
        }

        macip_cfg_print_xml(log, macip_cfg, level + 1);
    }
    CVECTOR_UNLOCK(macip_cfg_vec, LOC_CSYSCFG_0028);

    c_ident_print(log, level);
    sys_print(log, "</macIpMapsConfig>\n");
    return;
}

SYS_CFG *sys_cfg_new()
{
    SYS_CFG *sys_cfg;

    alloc_static_mem(MD_TBD, CMPI_ANY_MODI, MM_SYS_CFG, &sys_cfg, LOC_CSYSCFG_0029);
    if(NULL_PTR != sys_cfg)
    {
        sys_cfg_init(sys_cfg);
    }
    return (sys_cfg);
}

EC_BOOL sys_cfg_init(SYS_CFG *sys_cfg)
{
    task_cfg_init(SYS_CFG_TASK_CFG(sys_cfg));
    cvector_init(SYS_CFG_CLUSTER_VEC(sys_cfg)  , 0, MM_CLUSTER_CFG, CVECTOR_LOCK_ENABLE, LOC_CSYSCFG_0030);
    cvector_init(SYS_CFG_MACIP_CFG_VEC(sys_cfg), 0, MM_MACIP_CFG  , CVECTOR_LOCK_ENABLE, LOC_CSYSCFG_0031);
    cvector_init(SYS_CFG_PARAS_CFG(sys_cfg)    , 0, MM_CPARACFG   , CVECTOR_LOCK_ENABLE, LOC_CSYSCFG_0032);

    mcast_cfg_init(SYS_CFG_MCAST_CFG(sys_cfg));
    bcast_dhcp_cfg_init(SYS_CFG_BCAST_DHCP_CFG(sys_cfg));
    ganglia_cfg_init(SYS_CFG_GANGLIA_CFG(sys_cfg));

    return (EC_TRUE);
}

EC_BOOL sys_cfg_clean(SYS_CFG *sys_cfg)
{
    task_cfg_clean(SYS_CFG_TASK_CFG(sys_cfg));
    cvector_clean(SYS_CFG_CLUSTER_VEC(sys_cfg)  , (CVECTOR_DATA_CLEANER)cluster_cfg_free, LOC_CSYSCFG_0033);
    cvector_clean(SYS_CFG_MACIP_CFG_VEC(sys_cfg), (CVECTOR_DATA_CLEANER)macip_cfg_free  , LOC_CSYSCFG_0034);
    cvector_clean(SYS_CFG_PARAS_CFG(sys_cfg)    , (CVECTOR_DATA_CLEANER)cparacfg_free   , LOC_CSYSCFG_0035);

    mcast_cfg_clean(SYS_CFG_MCAST_CFG(sys_cfg));
    bcast_dhcp_cfg_clean(SYS_CFG_BCAST_DHCP_CFG(sys_cfg));
    ganglia_cfg_clean(SYS_CFG_GANGLIA_CFG(sys_cfg));

    return (EC_TRUE);
}

EC_BOOL sys_cfg_free(SYS_CFG *sys_cfg)
{
    if(NULL_PTR != sys_cfg)
    {
        sys_cfg_clean(sys_cfg);
        free_static_mem(MD_TBD, CMPI_ANY_MODI, MM_SYS_CFG, sys_cfg, LOC_CSYSCFG_0036);
    }
    return (EC_TRUE);
}

EC_BOOL sys_cfg_load(SYS_CFG *sys_cfg, const char *xml_fname)
{
    xmlDocPtr  sys_cfg_doc;
    xmlNodePtr sys_cfg_root;

    sys_cfg_doc  = cxml_new((UINT8 *)xml_fname);
    if(NULL_PTR == sys_cfg_doc)
    {
        sys_log(LOGSTDOUT, "error:sys_cfg_load: load %s failed\n", xml_fname);
        return (EC_FALSE);
    }

    sys_cfg_root = cxml_get_root(sys_cfg_doc);
    
    if(EC_FALSE == cxml_parse_sys_cfg(sys_cfg_root, sys_cfg))
    {
        sys_log(LOGSTDOUT, "error:sys_cfg_load: parse %s failed\n", xml_fname);
        cxml_free(sys_cfg_doc);
        return (EC_FALSE);
    }

    cxml_free(sys_cfg_doc);
    return (EC_TRUE);
}

CPARACFG *sys_cfg_search_cparacfg(const SYS_CFG *sys_cfg, const UINT32 tcid, const UINT32 rank)
{
    CVECTOR *paras_cfg;
    UINT32 pos;

    paras_cfg = SYS_CFG_PARAS_CFG((SYS_CFG *)sys_cfg);

    CVECTOR_LOCK(paras_cfg, LOC_CSYSCFG_0037);
    for(pos = 0; pos < cvector_size(paras_cfg); pos ++)
    {
        CPARACFG *cparacfg;

        cparacfg = (CPARACFG *)cvector_get_no_lock(paras_cfg, pos);
        if(NULL_PTR == cparacfg)
        {
            continue;
        }

        if(tcid == CPARACFG_TCID(cparacfg) && rank == CPARACFG_RANK(cparacfg))
        {
            CVECTOR_UNLOCK(paras_cfg, LOC_CSYSCFG_0038);
            return (cparacfg);
        }
    }
    CVECTOR_UNLOCK(paras_cfg, LOC_CSYSCFG_0039);
    return (NULL_PTR);
}

TASKS_CFG *sys_cfg_search_tasks_cfg(const SYS_CFG *sys_cfg, const UINT32 tcid, const UINT32 maski, const UINT32 maske)
{
    return task_cfg_searchs(SYS_CFG_TASK_CFG((SYS_CFG *)sys_cfg), tcid, maski, maske);
}

TASKS_CFG *sys_cfg_search_tasks_cfg_by_ip(const SYS_CFG *sys_cfg, const UINT32 ipaddr, const UINT32 port)
{
    return task_cfg_searchs_by_ip(SYS_CFG_TASK_CFG((SYS_CFG *)sys_cfg), ipaddr, port);
}

TASKS_CFG *sys_cfg_search_tasks_cfg_by_netcards(const SYS_CFG *sys_cfg, const CSET *cnetcard_set)
{
    return task_cfg_searchs_by_netcards(SYS_CFG_TASK_CFG((SYS_CFG *)sys_cfg), cnetcard_set);
}

MCAST_CFG *sys_cfg_search_mcast_cfg(const SYS_CFG *sys_cfg, const UINT32 tcid)
{
    MCAST_CFG *mcast_cfg;

    mcast_cfg = SYS_CFG_MCAST_CFG((SYS_CFG *)sys_cfg);

    if(tcid == MCAST_CFG_TCID(mcast_cfg))
    {
        return (mcast_cfg);
    }

    return (NULL_PTR);
}

BCAST_DHCP_CFG *sys_cfg_search_bcast_dhcp_cfg(const SYS_CFG *sys_cfg, const UINT32 tcid)
{
    BCAST_DHCP_CFG *bcast_dhcp_cfg;

    bcast_dhcp_cfg = SYS_CFG_BCAST_DHCP_CFG((SYS_CFG *)sys_cfg);

    if(tcid == BCAST_DHCP_CFG_TCID(bcast_dhcp_cfg))
    {
        return (bcast_dhcp_cfg);
    }

    return (NULL_PTR);
}

TASK_CFG *sys_cfg_filter_task_cfg(const SYS_CFG *sys_cfg, const UINT32 tcid)
{
    TASK_CFG *task_cfg;

    task_cfg = task_cfg_new();
    if(NULL_PTR == task_cfg)
    {
        sys_log(LOGSTDOUT, "error:sys_cfg_filter_task_cfg: new task cfg failed\n");
        return (NULL_PTR);
    }

    if(EC_FALSE == task_cfg_filter(SYS_CFG_TASK_CFG(sys_cfg), tcid, task_cfg))
    {
        sys_log(LOGSTDOUT, "error:sys_cfg_filter_task_cfg: filter tcid %s from task cfg of sys cfg failed\n", c_word_to_ipv4(tcid));
        task_cfg_free(task_cfg);
        return (NULL_PTR);
    }

    return (task_cfg);
}

TASK_CFG *sys_cfg_get_task_cfg(const SYS_CFG *sys_cfg)
{
    return SYS_CFG_TASK_CFG((SYS_CFG *)sys_cfg);
}

CVECTOR *sys_cfg_get_cluster_cfg_vec(const SYS_CFG *sys_cfg)
{
    return SYS_CFG_CLUSTER_VEC((SYS_CFG *)sys_cfg);
}

CLUSTER_CFG *sys_cfg_get_cluster_cfg_by_name_cstr(const SYS_CFG *sys_cfg, const CSTRING *name_cstr)
{
    UINT32 pos;
    pos = cvector_search_front(SYS_CFG_CLUSTER_VEC(sys_cfg), (void *)name_cstr, (CVECTOR_DATA_CMP)cluster_cfg_check_name_cstr);
    return (CLUSTER_CFG *)cvector_get(SYS_CFG_CLUSTER_VEC(sys_cfg), pos);
}

CLUSTER_CFG *sys_cfg_get_cluster_cfg_by_name_str(const SYS_CFG *sys_cfg, const char *name_str)
{
    UINT32 pos;
    pos = cvector_search_front(SYS_CFG_CLUSTER_VEC(sys_cfg), (void *)name_str, (CVECTOR_DATA_CMP)cluster_cfg_check_name_str);
    return (CLUSTER_CFG *)cvector_get(SYS_CFG_CLUSTER_VEC(sys_cfg), pos);
}

CLUSTER_CFG *sys_cfg_get_cluster_cfg_by_id(const SYS_CFG *sys_cfg, const UINT32 id)
{
    UINT32 pos;
    pos = cvector_search_front(SYS_CFG_CLUSTER_VEC(sys_cfg), (void *)id, (CVECTOR_DATA_CMP)cluster_cfg_check_id);
    return (CLUSTER_CFG *)cvector_get(SYS_CFG_CLUSTER_VEC(sys_cfg), pos);
}

CVECTOR *sys_cfg_get_paras_cfg(const SYS_CFG *sys_cfg)
{
    return SYS_CFG_PARAS_CFG((SYS_CFG *)sys_cfg);
}

CVECTOR *sys_cfg_get_macip_cfg_vec(const SYS_CFG *sys_cfg)
{
    return SYS_CFG_MACIP_CFG_VEC((SYS_CFG *)sys_cfg);
}

MCAST_CFG *sys_cfg_get_mcast_cfg(const SYS_CFG *sys_cfg)
{
    return SYS_CFG_MCAST_CFG((SYS_CFG *)sys_cfg);
}

BCAST_DHCP_CFG *sys_cfg_get_bcast_dhcp_cfg(const SYS_CFG *sys_cfg)
{
    return SYS_CFG_BCAST_DHCP_CFG((SYS_CFG *)sys_cfg);
}

UINT32 sys_cfg_get_task_cfg_default_csrv_port(const SYS_CFG *sys_cfg)
{
    return task_cfg_default_csrv_port(SYS_CFG_TASK_CFG(sys_cfg));
}

MACIP_CFG *sys_cfg_search_macip_cfg_by_ipv4_addr(const SYS_CFG *sys_cfg, const UINT32 ipv4_addr)
{
    CVECTOR *macip_cfg_vec;
    UINT32 pos;

    macip_cfg_vec = sys_cfg_get_macip_cfg_vec(sys_cfg);
    pos = cvector_search_front(macip_cfg_vec, (void *)ipv4_addr, (CVECTOR_DATA_CMP)macip_cfg_check_ipv4_addr);
    if(CVECTOR_ERR_POS != pos)
    {
        return (MACIP_CFG *)cvector_get(macip_cfg_vec, pos);
    }
    return (NULL_PTR);
}

MACIP_CFG *sys_cfg_search_macip_cfg_by_mac_addr(const SYS_CFG *sys_cfg, const UINT8 *mac_addr)
{
    CVECTOR *macip_cfg_vec;
    UINT32 pos;

    macip_cfg_vec = sys_cfg_get_macip_cfg_vec(sys_cfg);
    pos = cvector_search_front(macip_cfg_vec, (void *)mac_addr, (CVECTOR_DATA_CMP)macip_cfg_check_mac_addr);
    if(CVECTOR_ERR_POS != pos)
    {
        return (MACIP_CFG *)cvector_get(macip_cfg_vec, pos);
    }
    return (NULL_PTR);
}

static EC_BOOL __cluster_node_cfg_collect_tcid(const CLUSTER_NODE_CFG *cluster_node_cfg, const char *role, CVECTOR * dn_tcid_vec)
{
    if(
       EC_TRUE == cluster_node_cfg_check_role_str(cluster_node_cfg, role)
    && CVECTOR_ERR_POS == cvector_search_front(dn_tcid_vec, (void *)CLUSTER_NODE_CFG_TCID(cluster_node_cfg), NULL_PTR)
    )
    {
        cvector_push(dn_tcid_vec, (void *)CLUSTER_NODE_CFG_TCID(cluster_node_cfg));
    }
    return (EC_TRUE);
}

EC_BOOL sys_cfg_collect_hsdfs_dn_tcid_vec(const SYS_CFG *sys_cfg, const CVECTOR *cluster_id_vec, CVECTOR * dn_tcid_vec)
{
    UINT32 pos;

    CVECTOR_LOCK(SYS_CFG_CLUSTER_VEC(sys_cfg), LOC_CSYSCFG_0040);
    for(pos = 0; pos < cvector_size(SYS_CFG_CLUSTER_VEC(sys_cfg)); pos ++)
    {
        CLUSTER_CFG *cluster_cfg;        

        cluster_cfg = (CLUSTER_CFG *)cvector_get_no_lock(SYS_CFG_CLUSTER_VEC(sys_cfg), pos);
        if(NULL_PTR == cluster_cfg)
        {
            continue;
        }

        if(CVECTOR_ERR_POS == cvector_search_front(cluster_id_vec, (void *)CLUSTER_CFG_ID(cluster_cfg), NULL_PTR))
        {
            continue;
        }

        cluster_cfg_collect_tcid_vec_by_role_str(cluster_cfg, MODEL_TYPE_HSDFS_CONNEC, (const char *)"datanode:dn", dn_tcid_vec);
    }
    CVECTOR_UNLOCK(SYS_CFG_CLUSTER_VEC(sys_cfg), LOC_CSYSCFG_0041);
    return (EC_TRUE);
}

EC_BOOL sys_cfg_collect_hsdfs_np_tcid_vec(const SYS_CFG *sys_cfg, const CVECTOR *cluster_id_vec, CVECTOR * np_tcid_vec)
{
    UINT32 pos;

    CVECTOR_LOCK(SYS_CFG_CLUSTER_VEC(sys_cfg), LOC_CSYSCFG_0042);
    for(pos = 0; pos < cvector_size(SYS_CFG_CLUSTER_VEC(sys_cfg)); pos ++)
    {
        CLUSTER_CFG *cluster_cfg;        

        cluster_cfg = (CLUSTER_CFG *)cvector_get_no_lock(SYS_CFG_CLUSTER_VEC(sys_cfg), pos);
        if(NULL_PTR == cluster_cfg)
        {
            continue;
        }

        if(CVECTOR_ERR_POS == cvector_search_front(cluster_id_vec, (void *)CLUSTER_CFG_ID(cluster_cfg), NULL_PTR))
        {
            continue;
        }        

        cluster_cfg_collect_tcid_vec_by_role_str(cluster_cfg, MODEL_TYPE_HSDFS_CONNEC, (const char *)"namenode:np", np_tcid_vec);
    }
    CVECTOR_UNLOCK(SYS_CFG_CLUSTER_VEC(sys_cfg), LOC_CSYSCFG_0043);
    return (EC_TRUE);
}

EC_BOOL sys_cfg_collect_hsdfs_client_tcid_vec(const SYS_CFG *sys_cfg, const CVECTOR *cluster_id_vec, CVECTOR * client_tcid_vec)
{
    UINT32 pos;

    CVECTOR_LOCK(SYS_CFG_CLUSTER_VEC(sys_cfg), LOC_CSYSCFG_0044);
    for(pos = 0; pos < cvector_size(SYS_CFG_CLUSTER_VEC(sys_cfg)); pos ++)
    {
        CLUSTER_CFG *cluster_cfg;        

        cluster_cfg = (CLUSTER_CFG *)cvector_get_no_lock(SYS_CFG_CLUSTER_VEC(sys_cfg), pos);
        if(NULL_PTR == cluster_cfg)
        {
            continue;
        }

        if(CVECTOR_ERR_POS == cvector_search_front(cluster_id_vec, (void *)CLUSTER_CFG_ID(cluster_cfg), NULL_PTR))
        {
            continue;
        }

        cluster_cfg_collect_tcid_vec_by_role_str(cluster_cfg, MODEL_TYPE_HSDFS_CONNEC, (const char *)"client", client_tcid_vec);
    }
    CVECTOR_UNLOCK(SYS_CFG_CLUSTER_VEC(sys_cfg), LOC_CSYSCFG_0045);
    return (EC_TRUE);
}

CSTRING *sys_cfg_get_hsdfs_np_root_dir(const SYS_CFG *sys_cfg, const UINT32 cluster_id)
{
    CLUSTER_CFG *cluster_cfg;
    CSTRING     *np_root_dir;

    cluster_cfg = sys_cfg_get_cluster_cfg_by_id(sys_cfg, cluster_id);
    if(NULL_PTR == cluster_cfg)
    {
        sys_log(LOGSTDOUT, "error:sys_cfg_get_hsdfs_np_root_dir: undefined cluster %ld\n", cluster_id);
        return (NULL_PTR);
    }

    if(MODEL_TYPE_HSDFS_CONNEC != CLUSTER_CFG_MODEL(cluster_cfg))
    {
        sys_log(LOGSTDOUT, "error:sys_cfg_get_hsdfs_np_root_dir: cluster id %ld is not hsdfs model\n", CLUSTER_CFG_ID(cluster_cfg));
        return (NULL_PTR);    
    }

    np_root_dir = cluster_cfg_get_node_extra_val_by_key_str(cluster_cfg, CMPI_LOCAL_TCID, CMPI_LOCAL_RANK, (const char *)"npdir");
    if(NULL_PTR != np_root_dir)
    {
        return (np_root_dir);
    }

    np_root_dir = cluster_cfg_get_extra_val_by_key_str(cluster_cfg, (const char *)"npdir");
    if(NULL_PTR != np_root_dir)
    {
        return (np_root_dir);
    }

    return (NULL_PTR);
}

CSTRING *sys_cfg_get_hsdfs_dn_root_dir(const SYS_CFG *sys_cfg, const UINT32 cluster_id)
{
    CLUSTER_CFG *cluster_cfg;
    CSTRING     *dn_root_dir;

    cluster_cfg = sys_cfg_get_cluster_cfg_by_id(sys_cfg, cluster_id);
    if(NULL_PTR == cluster_cfg)
    {
        sys_log(LOGSTDOUT, "error:sys_cfg_get_hsdfs_dn_root_dir: undefined cluster %ld\n", cluster_id);
        return (NULL_PTR);
    }
    
    if(MODEL_TYPE_HSDFS_CONNEC != CLUSTER_CFG_MODEL(cluster_cfg))
    {
        sys_log(LOGSTDOUT, "error:sys_cfg_get_hsdfs_dn_root_dir: cluster id %ld is not hsdfs model\n", CLUSTER_CFG_ID(cluster_cfg));
        return (NULL_PTR);    
    }

    dn_root_dir = cluster_cfg_get_node_extra_val_by_key_str(cluster_cfg, CMPI_LOCAL_TCID, CMPI_LOCAL_RANK, (const char *)"dndir");
    if(NULL_PTR != dn_root_dir)
    {
        return (dn_root_dir);
    }

    dn_root_dir = cluster_cfg_get_extra_val_by_key_str(cluster_cfg, (const char *)"dndir");
    if(NULL_PTR != dn_root_dir)
    {
        return (dn_root_dir);
    }

    return (NULL_PTR);
}

CSTRING *sys_cfg_collect_hsdfs_np_root_dir(const SYS_CFG *sys_cfg, const CVECTOR *cluster_id_vec)
{
    UINT32 pos;    

    CVECTOR_LOCK(cluster_id_vec, LOC_CSYSCFG_0046);
    for(pos = 0; pos < cvector_size(cluster_id_vec); pos ++)
    {
        UINT32 cluster_id;
        CSTRING *np_root_dir;

        cluster_id = (UINT32)cvector_get_no_lock(cluster_id_vec, pos);        
        np_root_dir = sys_cfg_get_hsdfs_np_root_dir(sys_cfg, cluster_id);
        if(NULL_PTR != np_root_dir)
        {
            CVECTOR_UNLOCK(cluster_id_vec, LOC_CSYSCFG_0047);
            return (np_root_dir);
        }
    }
    CVECTOR_UNLOCK(cluster_id_vec, LOC_CSYSCFG_0048);
    return (NULL_PTR);
}

CSTRING *sys_cfg_collect_hsdfs_dn_root_dir(const SYS_CFG *sys_cfg, const CVECTOR *cluster_id_vec)
{
    UINT32 pos;    

    CVECTOR_LOCK(cluster_id_vec, LOC_CSYSCFG_0049);
    for(pos = 0; pos < cvector_size(cluster_id_vec); pos ++)
    {
        UINT32 cluster_id;
        CSTRING *dn_root_dir;
        
        cluster_id = (UINT32)cvector_get_no_lock(cluster_id_vec, pos);       
        dn_root_dir = sys_cfg_get_hsdfs_dn_root_dir(sys_cfg, cluster_id);
        if(NULL_PTR != dn_root_dir)
        {
            CVECTOR_UNLOCK(cluster_id_vec, LOC_CSYSCFG_0050);
            return (dn_root_dir);
        }
    }
    CVECTOR_UNLOCK(cluster_id_vec, LOC_CSYSCFG_0051);
    return (NULL_PTR);
}

EC_BOOL sys_cfg_collect_hsbgt_root_tcid_vec(const SYS_CFG *sys_cfg, const CVECTOR *cluster_id_vec, CVECTOR * root_tcid_vec)
{
    UINT32 pos;

    CVECTOR_LOCK(SYS_CFG_CLUSTER_VEC(sys_cfg), LOC_CSYSCFG_0052);
    for(pos = 0; pos < cvector_size(SYS_CFG_CLUSTER_VEC(sys_cfg)); pos ++)
    {
        CLUSTER_CFG *cluster_cfg;        

        cluster_cfg = (CLUSTER_CFG *)cvector_get_no_lock(SYS_CFG_CLUSTER_VEC(sys_cfg), pos);
        if(NULL_PTR == cluster_cfg)
        {
            continue;
        }

        if(CVECTOR_ERR_POS == cvector_search_front(cluster_id_vec, (void *)CLUSTER_CFG_ID(cluster_cfg), NULL_PTR))
        {
            continue;
        }

        cluster_cfg_collect_tcid_vec_by_role_and_group_str(cluster_cfg, MODEL_TYPE_HSBGT_CONNEC, (const char *)"table", (const char *)"root", root_tcid_vec);
    }
    CVECTOR_UNLOCK(SYS_CFG_CLUSTER_VEC(sys_cfg), LOC_CSYSCFG_0053);
    return (EC_TRUE);
}

EC_BOOL sys_cfg_collect_hsbgt_table_tcid_vec(const SYS_CFG *sys_cfg, const CVECTOR *cluster_id_vec, CVECTOR * table_tcid_vec)
{
    UINT32 pos;

    CVECTOR_LOCK(SYS_CFG_CLUSTER_VEC(sys_cfg), LOC_CSYSCFG_0054);
    for(pos = 0; pos < cvector_size(SYS_CFG_CLUSTER_VEC(sys_cfg)); pos ++)
    {
        CLUSTER_CFG *cluster_cfg;        

        cluster_cfg = (CLUSTER_CFG *)cvector_get_no_lock(SYS_CFG_CLUSTER_VEC(sys_cfg), pos);
        if(NULL_PTR == cluster_cfg)
        {
            continue;
        }

        if(CVECTOR_ERR_POS == cvector_search_front(cluster_id_vec, (void *)CLUSTER_CFG_ID(cluster_cfg), NULL_PTR))
        {
            continue;
        }

        cluster_cfg_collect_tcid_vec_by_role_str(cluster_cfg, MODEL_TYPE_HSBGT_CONNEC, (const char *)"table", table_tcid_vec);
    }
    CVECTOR_UNLOCK(SYS_CFG_CLUSTER_VEC(sys_cfg), LOC_CSYSCFG_0055);
    return (EC_TRUE);
}

EC_BOOL sys_cfg_collect_hsbgt_client_tcid_vec(const SYS_CFG *sys_cfg, const CVECTOR *cluster_id_vec, CVECTOR * table_tcid_vec)
{
    UINT32 pos;

    CVECTOR_LOCK(SYS_CFG_CLUSTER_VEC(sys_cfg), LOC_CSYSCFG_0056);
    for(pos = 0; pos < cvector_size(SYS_CFG_CLUSTER_VEC(sys_cfg)); pos ++)
    {
        CLUSTER_CFG *cluster_cfg;        

        cluster_cfg = (CLUSTER_CFG *)cvector_get_no_lock(SYS_CFG_CLUSTER_VEC(sys_cfg), pos);
        if(NULL_PTR == cluster_cfg)
        {
            continue;
        }

        if(CVECTOR_ERR_POS == cvector_search_front(cluster_id_vec, (void *)CLUSTER_CFG_ID(cluster_cfg), NULL_PTR))
        {
            continue;
        }

        cluster_cfg_collect_tcid_vec_by_role_str(cluster_cfg, MODEL_TYPE_HSBGT_CONNEC, (const char *)"client", table_tcid_vec);
    }
    CVECTOR_UNLOCK(SYS_CFG_CLUSTER_VEC(sys_cfg), LOC_CSYSCFG_0057);
    return (EC_TRUE);
}

CSTRING *sys_cfg_get_hsbgt_root_table_dir(const SYS_CFG *sys_cfg, const UINT32 cluster_id)
{
    CLUSTER_CFG *cluster_cfg;
    CSTRING     *root_table_dir;

    cluster_cfg = sys_cfg_get_cluster_cfg_by_id(sys_cfg, cluster_id);
    if(NULL_PTR == cluster_cfg)
    {
        sys_log(LOGSTDOUT, "error:sys_cfg_get_hsbgt_root_table_dir: undefined cluster %ld\n", cluster_id);
        return (NULL_PTR);
    }
    
    if(MODEL_TYPE_HSBGT_CONNEC != CLUSTER_CFG_MODEL(cluster_cfg))
    {
        sys_log(LOGSTDOUT, "error:sys_cfg_get_hsbgt_root_table_dir: cluster id %ld is not hsbgt model\n", CLUSTER_CFG_ID(cluster_cfg));
        return (NULL_PTR);    
    }

    root_table_dir = cluster_cfg_get_node_extra_val_by_key_str(cluster_cfg, CMPI_LOCAL_TCID, CMPI_LOCAL_RANK, (const char *)"roottabledir");
    if(NULL_PTR != root_table_dir)
    {
        return (root_table_dir);
    }

    root_table_dir = cluster_cfg_get_extra_val_by_key_str(cluster_cfg, (const char *)"roottabledir");
    if(NULL_PTR != root_table_dir)
    {
        return (root_table_dir);
    }

    return (NULL_PTR);
}

CSTRING *sys_cfg_collect_hsbgt_root_table_dir(const SYS_CFG *sys_cfg, const CVECTOR *cluster_id_vec)
{
    UINT32 pos;    

    CVECTOR_LOCK(cluster_id_vec, LOC_CSYSCFG_0058);
    for(pos = 0; pos < cvector_size(cluster_id_vec); pos ++)
    {
        UINT32 cluster_id;
        CSTRING *root_table_dir;

        cluster_id = (UINT32)cvector_get_no_lock(cluster_id_vec, pos);        
        root_table_dir = sys_cfg_get_hsbgt_root_table_dir(sys_cfg, cluster_id);
        if(NULL_PTR != root_table_dir)
        {
            CVECTOR_UNLOCK(cluster_id_vec, LOC_CSYSCFG_0059);
            return (root_table_dir);
        }
    }
    CVECTOR_UNLOCK(cluster_id_vec, LOC_CSYSCFG_0060);
    return (NULL_PTR);
}

EC_BOOL sys_cfg_add_macip_cfg(SYS_CFG *sys_cfg, const UINT32 ipv4_addr, const UINT8 *mac_addr)
{
    MACIP_CFG   *macip_cfg;
    macip_cfg = sys_cfg_search_macip_cfg_by_ipv4_addr(sys_cfg, ipv4_addr);
    if(NULL_PTR != macip_cfg)
    {
        sys_log(LOGSTDOUT, "warn:sys_cfg_add_macip_cfg: macip_cfg for ipv4 addr %s already exist, give up binding to mac %s\n",
                            c_word_to_ipv4(ipv4_addr), mac_addr_to_str(mac_addr)
                            );
        return (EC_TRUE);
    }

    macip_cfg = macip_cfg_new();
    if(NULL_PTR == macip_cfg)
    {
        sys_log(LOGSTDOUT, "error:sys_cfg_add_macip_cfg: new macip_cfg failed\n");
        return (EC_FALSE);
    }

    macip_cfg_set(macip_cfg, mac_addr, ipv4_addr);
    cvector_push(SYS_CFG_MACIP_CFG_VEC(sys_cfg), (void *)macip_cfg);

    sys_log(LOGSTDOUT, "[DEBUG] sys_cfg_add_macip_cfg: bind (ip %s, mac %s)\n",
                        c_word_to_ipv4(ipv4_addr), mac_addr_to_str(mac_addr)
                        );
    return (EC_TRUE);
}

EC_BOOL sys_cfg_add_tasks_cfg(SYS_CFG *sys_cfg, const UINT32 tcid, const UINT32 maski, const UINT32 maske, const UINT32 srvipaddr, const UINT32 srvport, const UINT32 csrvport)
{
    TASKS_CFG *tasks_cfg;
    tasks_cfg = sys_cfg_search_tasks_cfg(sys_cfg, tcid, maski, maske);
    if(NULL_PTR != tasks_cfg)
    {
        sys_log(LOGSTDOUT, "warn:sys_cfg_add_tasks_cfg: tasks_cfg for tcid %s already exist, "
                            "give up add tasks cfg (tcid %s, maski %s, maske %s, srvipaddr %s, srvport %ld, csrvport %ld)\n",
                            c_word_to_ipv4(tcid),
                            c_word_to_ipv4(tcid), c_word_to_ipv4(maski),c_word_to_ipv4(maske),
                            c_word_to_ipv4(srvipaddr), srvport,
                            csrvport
                            );
        return (EC_TRUE);
    }

    tasks_cfg = tasks_cfg_new();
    if(NULL_PTR == tasks_cfg)
    {
        sys_log(LOGSTDOUT, "error:sys_cfg_add_tasks_cfg: new tasks_cfg failed\n");
        return (EC_FALSE);
    }

    TASKS_CFG_TCID(tasks_cfg)       = tcid;
    TASKS_CFG_MASKI(tasks_cfg)      = maski;
    TASKS_CFG_MASKE(tasks_cfg)      = maske;
    TASKS_CFG_SRVIPADDR(tasks_cfg)  = srvipaddr;
    TASKS_CFG_SRVPORT(tasks_cfg)    = srvport;
    TASKS_CFG_CSRVPORT(tasks_cfg)   = csrvport;

    cvector_push(TASK_CFG_TASKS_CFG_VEC(SYS_CFG_TASK_CFG(sys_cfg)), (void *)tasks_cfg);

    sys_log(LOGSTDOUT, "[DEBUG] sys_cfg_add_tasks_cfg: add tasks cfg (tcid %s, maski %s, maske %s, srvipaddr %s, srvport %ld, csrvport %ld)\n",
                        c_word_to_ipv4(tcid), c_word_to_ipv4(maski),c_word_to_ipv4(maske),
                        c_word_to_ipv4(srvipaddr), srvport,
                        csrvport
                        );
    return (EC_TRUE);
}

EC_BOOL sys_cfg_flush_xml(const SYS_CFG *sys_cfg, const CSTRING *sys_cfg_xml_cstr)
{
    LOG *log;
    CBYTES *cbytes;

    log = log_cstr_open();
    if(NULL_PTR == log)
    {
        sys_log(LOGSTDOUT, "error:sys_cfg_flush_xml: new log for %s failed\n", (char *)cstring_get_str(sys_cfg_xml_cstr));
        return (EC_FALSE);
    }

    sys_log(LOGSTDOUT, "[DEBUG] sys_cfg_flush_xml: try to flush sysconfig to %s\n", (char *)cstring_get_str(sys_cfg_xml_cstr));
    sys_cfg_print_xml(log, sys_cfg, 0);

    cbytes = cstring_get_cbytes(LOG_CSTR(log));
    if(NULL_PTR == cbytes)
    {
        sys_log(LOGSTDOUT, "error:sys_cfg_flush_xml: get cbytes from log cstr failed\n");
        log_cstr_close(log);
        return (EC_FALSE);
    }
    if(EC_FALSE == super_upload(0, sys_cfg_xml_cstr, cbytes, EC_TRUE))
    {
        sys_log(LOGSTDOUT, "error:sys_cfg_flush_xml:  flush sysconfig to %s failed\n", (char *)cstring_get_str(sys_cfg_xml_cstr));
        log_cstr_close(log);
        cbytes_free(cbytes, LOC_CSYSCFG_0061);
        return (EC_FALSE);
    }

    log_file_close(log);
    cbytes_free(cbytes, LOC_CSYSCFG_0062);
    return (EC_TRUE);
}

void sys_cfg_cluster_cfg_vec_print_xml(LOG *log, const CVECTOR *cluster_cfg_vec, const UINT32 level)
{
    c_ident_print(log, level);
    sys_print(log, "<clusters>\n");

    cvector_print_level(log, cluster_cfg_vec, level + 1, (CVECTOR_DATA_LEVEL_PRINT)cluster_cfg_print_xml);

    c_ident_print(log, level);
    sys_print(log, "</clusters>\n");
    return;
}

void sys_cfg_print_xml(LOG *log, const SYS_CFG *sys_cfg, const UINT32 level)
{
    c_ident_print(log, level);
    sys_print(log, "<sysConfig>\n");

    task_cfg_print_xml(log, SYS_CFG_TASK_CFG(sys_cfg), level + 1);
    sys_cfg_cluster_cfg_vec_print_xml(log, SYS_CFG_CLUSTER_VEC(sys_cfg), level + 1);
    mcast_cfg_print_xml(log, SYS_CFG_MCAST_CFG(sys_cfg), level + 1);
    bcast_dhcp_cfg_print_xml(log, SYS_CFG_BCAST_DHCP_CFG(sys_cfg), level + 1);
    paras_cfg_print_xml(log, SYS_CFG_PARAS_CFG(sys_cfg), level + 1);
    macip_cfg_vec_print_xml(log, SYS_CFG_MACIP_CFG_VEC(sys_cfg), level + 1);

    c_ident_print(log, level);
    sys_print(log, "</sysConfig>\n");
    return;
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/


