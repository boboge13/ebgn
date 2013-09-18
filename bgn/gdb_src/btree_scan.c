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


#include "db_internal.h"

uint8_t btreeScan(const BTree *tree, const RawFile *rawFile, const uint8_t *des_key, int (*keyCompare)(const uint8_t *, const uint8_t *), offset_t *filePos)
{
    BTreeTraversal *trav;
    offset_t offset;

    if (tree == NULL)
    {
        sys_log(LOGSTDOUT, "error:btreeScan: tree is null\n");
        return 0;/*fail*/
    }
    trav = btreeInitTraversal(tree);

    for(offset = btreeGetFirstOffset(trav); 0 != offset; offset = btreeGetNextOffset(trav))
    {
        uint32_t klen;
        uint32_t len;
        uint8_t *key;

        if(RAW_FILE_FAIL == rawFileRead32(rawFile, &klen, offset + sizeof(uint32_t)))
        {
            sys_log(LOGSTDOUT,"error:btreeScan: read data_len at offset %d failed\n", offset);
            btreeDestroyTraversal(trav);
            return 0;
        }

        MEM_CHECK(key = (uint8_t *)SAFE_MALLOC(klen + 2 + 4 , LOC_BTREE_0064));

        if(RAW_FILE_FAIL == rawFileRead8s(rawFile, key, klen + 2 + 4, &len, offset + sizeof(uint32_t)))
        {
            sys_log(LOGSTDOUT,"error:btreeScan: read key %d bytes at offset %d failed\n", klen + 2 + 4, offset + sizeof(uint32_t));

            SAFE_FREE(key, LOC_BTREE_0065);
            btreeDestroyTraversal(trav);
            return 0;
        }

        if(0 == keyCompare(key, des_key))
        {
            SAFE_FREE(key, LOC_BTREE_0066);
            btreeDestroyTraversal(trav);

            (*filePos) = offset;
            return 1;/*succ*/
        }

        SAFE_FREE(key, LOC_BTREE_0067);
    }

    btreeDestroyTraversal(trav);

    (*filePos) = 0;
    return 1;
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

