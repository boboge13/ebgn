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

static uint32_t
__getNodeSize(BTreeNode *node, const uint16_t *keySizes)
{
    uint32_t l, i;

    if (node == NULL)
    {
        return 0;
    }
    if (keySizes == NULL)
    {
        keySizes = node->keySizes;
    }
    l = sizeof(uint8_t) +
        (node->tree->order * sizeof(offset_t)) +
        ((node->tree->order - 1) * sizeof(uint16_t));

    for (i = 0; i < node->tree->order - 1; i++)
    {
        l += keySizes[i];
    }
    return l;
}

static void
__compressNode(BTreeNode *node, uint8_t ***newKeys, uint16_t **newKeySizes)
{
    uint8_t i;

    if (node == NULL || node->keyCount < 2)
    {
        return;
    }
    MEM_CHECK(*newKeys = (uint8_t **)SAFE_MALLOC((node->tree->order - 1) * sizeof(uint8_t *), LOC_BTREE_0039));
    memset(*newKeys, 0, (node->tree->order - 1) * sizeof(uint8_t *));


    MEM_CHECK(*newKeySizes =
              (uint16_t *)SAFE_MALLOC((node->tree->order - 1) * sizeof(uint16_t), LOC_BTREE_0040));
    memset(*newKeySizes, 0, (node->tree->order - 1) * sizeof(uint16_t));

#if (SWITCH_ON == COMPRESS_MODE)
    (*newKeys)[0]     = keyDup(node->keys[0], LOC_BTREE_0041);
    (*newKeySizes)[0] = node->keySizes[0];

    for (i = node->keyCount - 1; i > 0; i--)
    {
        gdbCompressString(node->keys[i - 1], node->keySizes[i - 1],
                          node->keys[i], node->keySizes[i],
                          &(*newKeys)[i], &(*newKeySizes)[i]);
    }
#endif/*(SWITCH_ON == COMPRESS_MODE)*/

#if (SWITCH_OFF == COMPRESS_MODE)
    for(i = 0; i < node->keyCount; i ++)
    {
        (*newKeys)[i]     = keyDup(node->keys[i], LOC_BTREE_0042);
        (*newKeySizes)[i] = node->keySizes[i];
    }
#endif/*(SWITCH_OFF == COMPRESS_MODE)*/
}

static void
__uncompressNode(BTreeNode *node, uint8_t ***newKeys,
                 uint16_t **newKeySizes)
{
    uint8_t i;

    if (node == NULL || node->keyCount < 2)
    {
        return;
    }
    MEM_CHECK(*newKeys = (uint8_t **)SAFE_MALLOC((node->tree->order - 1) * sizeof(uint8_t *), LOC_BTREE_0043));
    memset(*newKeys, 0, (node->tree->order - 1) * sizeof(uint8_t *));


    MEM_CHECK(*newKeySizes = (uint16_t *)SAFE_MALLOC((node->tree->order - 1) * sizeof(uint16_t), LOC_BTREE_0044));
    memset(*newKeySizes, 0, (node->tree->order - 1) * sizeof(uint16_t));

#if (SWITCH_ON == COMPRESS_MODE)
    (*newKeys)[0]     = keyDup(node->keys[0], LOC_BTREE_0045);
    (*newKeySizes)[0] = node->keySizes[0];

    for (i = 1; i <= node->keyCount - 1; i++)
    {
        gdbUncompressString((*newKeys)[i - 1], (*newKeySizes)[i - 1],
                            node->keys[i], node->keySizes[i],
                            &(*newKeys)[i], &(*newKeySizes)[i]);
    }
#endif/*(SWITCH_ON == COMPRESS_MODE)*/
#if (SWITCH_OFF == COMPRESS_MODE)
    for(i = 0; i < node->keyCount; i ++)
    {
        (*newKeys)[i]     = keyDup(node->keys[i], LOC_BTREE_0046);
        (*newKeySizes)[i] = node->keySizes[i];
    }
#endif/*(SWITCH_OFF == COMPRESS_MODE)*/
}

/*comment: read BTree Node out of buffer*/
void *
btreeReadNodeBlock(GdbBlock *block, const uint8_t *buffer, void *extra)
{
    BTreeNode *node;
    uint8_t   i;
    uint32_t  counter = 0;

    node = (BTreeNode *)btreeCreateNodeBlock(block, extra);

    //comment: tree and block of the node were set in btreeCreateNodeBlock
    //node->tree  = (BTree *)extra;
    //node->block = block;

    node->keyCount = gdbGet8(buffer, &counter);

    for (i = 0; i < node->tree->order; i++)
    {
        node->children[i] = gdbGetOffset(buffer, &counter);
    }
    for (i = 0; i < node->tree->order - 1; i++)
    {
        node->keySizes[i] = gdbGet16(buffer, &counter);
    }
    for (i = 0; i < node->tree->order - 1; i++)
    {
        if (node->keySizes[i] > 0)
        {
            MEM_CHECK(node->keys[i] = keyNew(node->keySizes[i], LOC_BTREE_0047));
            memcpy(node->keys[i], buffer + counter, node->keySizes[i]);

            counter += node->keySizes[i];
        }
    }

#if (SWITCH_ON == COMPRESS_MODE)
    if (node->keyCount >= 2)
    {
        uint8_t **newKeys;
        uint16_t *newKeySizes;

        __uncompressNode(node, &newKeys, &newKeySizes);

        /* Free up the compressed keys. */
        for (i = 0; i < node->keyCount; i++)
        {
            if (node->keys[i] != NULL)
            {
                keyFree(node->keys[i], LOC_BTREE_0048);
            }
        }

        SAFE_FREE(node->keys, LOC_BTREE_0049);
        SAFE_FREE(node->keySizes, LOC_BTREE_0050);

        /* Move over the new arrays. */
        node->keys     = newKeys;
        node->keySizes = newKeySizes;
    }
#endif/*(SWITCH_ON == COMPRESS_MODE)*/
    return node;
}

/*comment: write BTree Node into buffer*/
void
btreeWriteNodeBlock(GdbBlock *block, uint8_t **buffer, uint32_t *size)
{
    BTreeNode *node;
    uint8_t    i;
    uint32_t   counter = 0;
    uint8_t ** newKeys;
    uint16_t * newKeySizes;

    node = (BTreeNode *)block->detail;
#if (SWITCH_ON == COMPRESS_MODE)
    /* Compress the node. */
    if (node->keyCount >= 2)
    {
        __compressNode(node, &newKeys, &newKeySizes);
    }
    else
    {
        newKeys     = node->keys;
        newKeySizes = node->keySizes;
    }
#endif/*(SWITCH_ON == COMPRESS_MODE)*/
#if (SWITCH_OFF == COMPRESS_MODE)
    newKeys     = node->keys;
    newKeySizes = node->keySizes;
#endif/*(SWITCH_OFF == COMPRESS_MODE)*/

    *size = __getNodeSize(node, newKeySizes);

    MEM_CHECK(*buffer = (uint8_t *)SAFE_MALLOC(*size, LOC_BTREE_0051));

    gdbPut8(*buffer, &counter, node->keyCount);

    for (i = 0; i < node->tree->order; i++)
    {
        gdbPutOffset(*buffer, &counter, node->children[i]);
    }
    for (i = 0; i < node->tree->order - 1; i++)
    {
        gdbPut16(*buffer, &counter, newKeySizes[i]);
    }
    for (i = 0; i < node->tree->order - 1; i++)
    {
        if (newKeySizes[i] > 0)
        {
            memcpy(*buffer + counter, newKeys[i], newKeySizes[i]);

            counter += newKeySizes[i];
        }
    }
#if (SWITCH_ON == COMPRESS_MODE)
    if (node->keyCount >= 2)
    {
        /* Free up the arrays. */
        for (i = 0; i < node->keyCount; i++)
        {
            if (newKeys[i] != NULL)
            {
                keyFree(newKeys[i], LOC_BTREE_0052);
            }
        }

        SAFE_FREE(newKeys, LOC_BTREE_0053);
        SAFE_FREE(newKeySizes, LOC_BTREE_0054);
    }
#endif/*(SWITCH_ON == COMPRESS_MODE)*/
}

void *
btreeCreateNodeBlock(GdbBlock *block, void *extra)
{
    BTreeNode *node;
    BTree *tree;

    tree = (BTree *)extra;

    MEM_CHECK(node = (BTreeNode *)SAFE_MALLOC(sizeof(BTreeNode), LOC_BTREE_0055));
    memset(node, 0, sizeof(BTreeNode));

    node->tree  = tree;
    node->block = block;

    /*comment: when reach here, we have no idea about keyCount, hence alloc children/keySizes/keys as the max possible num: the order*/

    MEM_CHECK(node->children = (offset_t *)SAFE_MALLOC(tree->order * sizeof(offset_t), LOC_BTREE_0056));
    memset(node->children, 0, tree->order * sizeof(offset_t));


    MEM_CHECK(node->keySizes = (uint16_t *)SAFE_MALLOC((tree->order - 1) * sizeof(uint16_t), LOC_BTREE_0057));
    memset(node->keySizes, 0, (tree->order - 1)  * sizeof(uint16_t));


    MEM_CHECK(node->keys = (uint8_t **)SAFE_MALLOC((tree->order - 1) * sizeof(uint8_t *), LOC_BTREE_0058));
    memset(node->keys, 0, (tree->order - 1) * sizeof(uint8_t *));

    return node;
}

void
btreeDestroyNodeBlock(void *data)
{
    BTreeNode *node = (BTreeNode *)data;
    uint8_t i;

    if (node == NULL)
    {
        return;
    }
    for (i = 0; i < node->keyCount; i++)
    {
        if (node->keys[i] != NULL)
        {
        /*
            sys_log(LOGSTDOUT,"[DEBUG]btreeDestroyNodeBlock: %d# size %d, addr %lx, %.*s\n", i,
                    node->keySizes[i], node->keys[i],
                    node->keySizes[i], node->keys[i]);
        */
            keyFree(node->keys[i], LOC_BTREE_0059);
        }
    }

    if (GDB_IS_DIRTY(node->block))
    {
        sys_log(LOGSTDOUT, "error:btreeDestroyNodeBlock: Dirty node at offset %d has not been written to disk.\n",
                            node->block->offset);
    }

    SAFE_FREE(node->children, LOC_BTREE_0060);
    SAFE_FREE(node->keySizes, LOC_BTREE_0061);
    SAFE_FREE(node->keys, LOC_BTREE_0062);

    SAFE_FREE(node, LOC_BTREE_0063);
}

BTreeNode *
btreeNewNode(BTree *tree)
{
    GdbBlock *block;

    if (tree == NULL)
    {
        return NULL;
    }
    block = gdbNewBlock(tree->block->db, GDB_BLOCK_BTREE_NODE, tree);

    if (block == NULL)
    {
        return NULL;
    }

    /* gdbWriteBlock(block); */
    return (BTreeNode *)block->detail;
}

void
btreeDestroyNode(BTreeNode *node)
{
    if (node != NULL)
    {
        gdbDestroyBlock(node->block);
    }
    return;
}

BTreeNode *
btreeReadNode(BTree *tree, offset_t offset)
{
    GdbBlock *block;

    if (tree == NULL)
    {
        sys_log(LOGSTDOUT, "error:btreeReadNode: tree is null\n");
        return NULL;
    }

    if (offset < DB_HEADER_BLOCK_SIZE)
    {
        sys_log(LOGSTDOUT, "error:btreeReadNode: offset %d < header block size %d\n", offset, DB_HEADER_BLOCK_SIZE);
        return NULL;
    }

    block = gdbReadBlock(tree->block->db, offset, GDB_BLOCK_BTREE_NODE, tree);
    if (block == NULL)
    {
        sys_log(LOGSTDOUT, "error:btreeReadNode: read gdbblock at offset %d failed\n", offset);
        return NULL;
    }

    if(NULL == block->detail)
    {
        sys_log(LOGSTDOUT, "error:btreeReadNode: block at offset %d with null detail\n", offset);
    }
    return (BTreeNode *)block->detail;
}

offset_t
btreeWriteNode(BTreeNode *node)
{
    if (node == NULL)
    {
        return 0;
    }
    gdbWriteBlock(node->block);

    return node->block->offset;
}

void
btreeEraseNode(BTreeNode *node)
{
    GdbBlock *block;

    if (node == NULL || node->block->offset == 0)
    {
        return;
    }
    block = node->block;

    gdbFreeBlock(block->db, block->offset, block->type);

#if 0
    rawFileSeek(block->db->fp, block->offset, SEEK_SET);
    gdbPad(block->db->fp, block->size);
#endif
}

void btreePrintNode(BTreeNode *node)
{
    uint8_t i;
    sys_log(LOGSTDOUT,"[DEBUG] btreePrintNode: node %lx:\n", node);
    sys_log(LOGSTDOUT,"tree %lx, block %lx, keyCount %d\n", node->tree, node->block, node->keyCount);
    for(i = 0; i <= node->keyCount; i ++)
    {
        sys_log(LOGSTDOUT,"child %d: %d\n", i, node->children[i]);
    }
    for(i = 0; i < node->keyCount; i ++)
    {
        sys_log(LOGSTDOUT,"key %d: size %d, word %s\n", i, node->keySizes[i], node->keys[i]);
    }
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

