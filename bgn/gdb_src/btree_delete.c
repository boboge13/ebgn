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

#include "db_internal.h"

#if 0
#define KV_FREE_DEBUG(Node, Index) do{\
    sys_log(LOGSTDOUT,"### try to free key %s, size %d at %s:%d\n", (Node)->keys[(Index)], (Node)->keySizes[(Index)], LOC_BTREE_0019);\
}while(0)
#endif

#if 1
#define KV_FREE_DEBUG(Node, Index) do{}while(0)
#endif

static uint8_t
__removeKey(BTree *tree, BTreeNode *rootNode, const uint8_t *key,
            offset_t *filePos)
{
    uint8_t i;

    for (i = 0;
         i < rootNode->keyCount && keyCmp(rootNode->keys[i], key) < 0;
         i++)
        ;

    btreeDebug(tree, LOC_BTREE_0020);
    if (BTREE_IS_LEAF(rootNode) && i < rootNode->keyCount &&
        keyCmp(rootNode->keys[i], key) == 0)
    {
        *filePos = rootNode->children[i];

        KV_FREE_DEBUG(rootNode, i);
        keyFree(rootNode->keys[i], LOC_BTREE_0021);

        btreeDebug(tree, LOC_BTREE_0022);
        for (; i < rootNode->keyCount - 1; i++)
        {
            rootNode->keys[i]     = rootNode->keys[i + 1];
            rootNode->keySizes[i] = rootNode->keySizes[i + 1];
            rootNode->children[i] = rootNode->children[i + 1];
        }
        btreeDebug(tree, LOC_BTREE_0023);

        rootNode->keys[i]         = NULL;
        rootNode->keySizes[i]     = 0;
        rootNode->children[i]     = rootNode->children[i + 1];
        rootNode->children[i + 1] = 0;

        rootNode->keyCount--;

        GDB_SET_DIRTY(rootNode->block);

        btreeDebug(tree, LOC_BTREE_0024);
        btreeWriteNode(rootNode);
        btreeDebug(tree, LOC_BTREE_0025);

        return 1;
    }
    btreeDebug(tree, LOC_BTREE_0026);
    return 0;
}

static void
__removeKey2(BTree *tree, BTreeNode *rootNode, uint8_t index)
{
    uint8_t i;

    KV_FREE_DEBUG(rootNode, index);
    keyFree(rootNode->keys[index], LOC_BTREE_0027);
    btreeDebug(tree, LOC_BTREE_0028);
    for (i = index; i < rootNode->keyCount - 1; i++)
    {
        rootNode->keys[i]     = rootNode->keys[i + 1];
        rootNode->keySizes[i] = rootNode->keySizes[i + 1];
        rootNode->children[i] = rootNode->children[i + 1];
    }
    btreeDebug(tree, LOC_BTREE_0029);

    rootNode->keys[i]         = NULL;
    rootNode->keySizes[i]     = 0;
    rootNode->children[i]     = rootNode->children[i + 1];
    rootNode->children[i + 1] = 0;

    rootNode->keyCount--;

    GDB_SET_DIRTY(rootNode->block);
    btreeDebug(tree, LOC_BTREE_0030);
    btreeWriteNode(rootNode);
    btreeDebug(tree, LOC_BTREE_0031);
}

static uint8_t
__borrowRight(BTree *tree, BTreeNode *rootNode, BTreeNode *prevNode, uint8_t div)
{
    BTreeNode *node;

    if (div >= prevNode->keyCount)
    {
        return 0;
    }

    btreeDebug(tree, LOC_BTREE_0032);
    node = btreeReadNode(tree, prevNode->children[div + 1]);
    if(NULL == node)
    {
        sys_log(LOGSTDOUT, "error:__borrowRight: read node of child %d of previous node %lx failed\n", div + 1, prevNode);
        return (0);
    }
    btreeDebug(tree, LOC_BTREE_0033);

    if (BTREE_IS_LEAF(node) && node->keyCount > tree->minLeaf)
    {
        rootNode->children[rootNode->keyCount + 1] =
            rootNode->children[rootNode->keyCount];

        rootNode->keys[rootNode->keyCount]     = keyDup(node->keys[0], LOC_BTREE_0034);
        rootNode->keySizes[rootNode->keyCount] = node->keySizes[0];
        rootNode->children[rootNode->keyCount] = node->children[0];

        KV_FREE_DEBUG(prevNode, div);
        keyFree(prevNode->keys[div], LOC_BTREE_0035);

        prevNode->keys[div] = keyDup(rootNode->keys[rootNode->keyCount], LOC_BTREE_0036);
        prevNode->keySizes[div] = rootNode->keySizes[rootNode->keyCount];
    }
    else if (!BTREE_IS_LEAF(node) && node->keyCount > tree->minInt)
    {
        rootNode->keys[rootNode->keyCount] = keyDup(prevNode->keys[div], LOC_BTREE_0037);
        rootNode->keySizes[rootNode->keyCount] = prevNode->keySizes[div];

        KV_FREE_DEBUG(prevNode, div);
        keyFree(prevNode->keys[div], LOC_BTREE_0038);

        prevNode->keys[div]     = keyDup(node->keys[0], LOC_BTREE_0039);
        prevNode->keySizes[div] = node->keySizes[0];

        rootNode->children[rootNode->keyCount + 1] = node->children[0];
    }
    else
    {
        btreeDebug(tree, LOC_BTREE_0040);
        btreeDestroyNode(node);
        btreeDebug(tree, LOC_BTREE_0041);

        return 0;
    }

    btreeDebug(tree, LOC_BTREE_0042);
    GDB_SET_DIRTY(rootNode->block);
    GDB_SET_DIRTY(prevNode->block);

    rootNode->keyCount++;
    btreeDebug(tree, LOC_BTREE_0043);
    __removeKey2(tree, node, 0);
    btreeDebug(tree, LOC_BTREE_0044);
    btreeDestroyNode(node);
    btreeDebug(tree, LOC_BTREE_0045);

    return 1;
}

static uint8_t
__borrowLeft(BTree *tree, BTreeNode *rootNode, BTreeNode *prevNode, uint8_t div)
{
    uint8_t i;
    BTreeNode *node;

    if (div == 0)
    {
        return 0;
    }
    btreeDebug(tree, LOC_BTREE_0046);
    node = btreeReadNode(tree, prevNode->children[div - 1]);
    if(NULL == node)
    {
        sys_log(LOGSTDOUT, "error:__borrowLeft: read node of child %d of previous node %lx failed\n", div - 1, prevNode);
        return (0);
    }
    btreeDebug(tree, LOC_BTREE_0047);

    if (BTREE_IS_LEAF(node) && node->keyCount > tree->minLeaf)
    {
        btreeDebug(tree, LOC_BTREE_0048);
        for (i = rootNode->keyCount; i > 0; i--)
        {
            rootNode->keys[i]         = rootNode->keys[i - 1];
            rootNode->keySizes[i]     = rootNode->keySizes[i - 1];
            rootNode->children[i + 1] = rootNode->children[i];
        }
        btreeDebug(tree, LOC_BTREE_0049);

        rootNode->children[1] = rootNode->children[0];
        rootNode->keys[0]     = keyDup(node->keys[node->keyCount - 1], LOC_BTREE_0050);
        rootNode->keySizes[0] = node->keySizes[node->keyCount - 1];
        rootNode->children[0] = node->children[node->keyCount - 1];

        rootNode->keyCount++;

        KV_FREE_DEBUG(prevNode, div - 1);
        keyFree(prevNode->keys[div - 1], LOC_BTREE_0051);
        prevNode->keys[div - 1]     = keyDup(node->keys[node->keyCount - 2], LOC_BTREE_0052);
        prevNode->keySizes[div - 1] = node->keySizes[node->keyCount - 2];

        node->children[node->keyCount - 1] =
            node->children[node->keyCount];

        node->children[node->keyCount] = 0;

        KV_FREE_DEBUG(node, node->keyCount - 1);
        keyFree(node->keys[node->keyCount - 1], LOC_BTREE_0053);
        node->keys[node->keyCount - 1]     = NULL;
        node->keySizes[node->keyCount - 1] = 0;
    }
    else if (!BTREE_IS_LEAF(node) && node->keyCount > tree->minInt)
    {
        btreeDebug(tree, LOC_BTREE_0054);
        for (i = rootNode->keyCount; i > 0; i--)
        {
            rootNode->keys[i]         = rootNode->keys[i - 1];
            rootNode->keySizes[i]     = rootNode->keySizes[i - 1];
            rootNode->children[i + 1] = rootNode->children[i];
        }
        btreeDebug(tree, LOC_BTREE_0055);

        rootNode->children[1] = rootNode->children[0];
        rootNode->keys[0]     = keyDup(prevNode->keys[div - 1], LOC_BTREE_0056);
        rootNode->keySizes[0] = prevNode->keySizes[div - 1];
        rootNode->children[0] = node->children[node->keyCount];

        rootNode->keyCount++;

        KV_FREE_DEBUG(prevNode, div - 1);
        keyFree(prevNode->keys[div - 1], LOC_BTREE_0057);
        prevNode->keys[div - 1]     = keyDup(node->keys[node->keyCount - 1], LOC_BTREE_0058);
        prevNode->keySizes[div - 1] = node->keySizes[node->keyCount - 1];

        node->children[node->keyCount] = 0;

        KV_FREE_DEBUG(node, node->keyCount - 1);
        keyFree(node->keys[node->keyCount - 1], LOC_BTREE_0059);
        node->keys[node->keyCount - 1]     = NULL;
        node->keySizes[node->keyCount - 1] = 0;
    }
    else
    {
        btreeDebug(tree, LOC_BTREE_0060);
        btreeDestroyNode(node);
        btreeDebug(tree, LOC_BTREE_0061);

        return 0;
    }

    node->keyCount--;

    GDB_SET_DIRTY(rootNode->block);
    GDB_SET_DIRTY(prevNode->block);
    GDB_SET_DIRTY(node->block);

    btreeDebug(tree, LOC_BTREE_0062);
    btreeWriteNode(node);
    btreeDebug(tree, LOC_BTREE_0063);
    btreeDestroyNode(node);
    btreeDebug(tree, LOC_BTREE_0064);

    return 1;
}

static uint8_t
__mergeNode(BTree *tree, BTreeNode *rootNode, BTreeNode *prevNode, uint8_t div)
{
    uint8_t i, j;
    BTreeNode *node;

    /* Try to merge the node with its left sibling. */
    if (div > 0)
    {
        btreeDebug(tree, LOC_BTREE_0065);
        node = btreeReadNode(tree, prevNode->children[div - 1]);
        if(NULL == node)
        {
            sys_log(LOGSTDOUT, "error:__mergeNode: read node of child %d of previous node %lx failed\n", div - 1, prevNode);
            return (0);
        }
        btreeDebug(tree, LOC_BTREE_0066);
        i    = node->keyCount;

        if (!BTREE_IS_LEAF(rootNode))
        {
            node->keys[i]     = keyDup(prevNode->keys[div - 1], LOC_BTREE_0067);
            node->keySizes[i] = prevNode->keySizes[div - 1];
            node->keyCount++;

            i++;
        }

        btreeDebug(tree, LOC_BTREE_0068);
        for (j = 0; j < rootNode->keyCount; j++, i++)
        {
            KV_FREE_DEBUG(node, i);
            //keyFree(node->keys[i], LOC_BTREE_0069);

            node->keys[i]     = keyDup(rootNode->keys[j], LOC_BTREE_0070);
            node->keySizes[i] = rootNode->keySizes[j];
            node->children[i] = rootNode->children[j];
            node->keyCount++;
        }
        btreeDebug(tree, LOC_BTREE_0071);

        node->children[i] = rootNode->children[j];

        GDB_SET_DIRTY(node->block);

        btreeDebug(tree, LOC_BTREE_0072);
        btreeWriteNode(node);
        btreeDebug(tree, LOC_BTREE_0073);

        prevNode->children[div] = node->block->offset;

        GDB_SET_DIRTY(prevNode->block);

        btreeDebug(tree, LOC_BTREE_0074);
        btreeEraseNode(rootNode);
        btreeDebug(tree, LOC_BTREE_0075);
        __removeKey2(tree, prevNode, div - 1);
        btreeDebug(tree, LOC_BTREE_0076);
    }
    else
    {
        /* Must merge the node with its right sibling. */
        btreeDebug(tree, LOC_BTREE_0077);
        node = btreeReadNode(tree, prevNode->children[div + 1]);
        if(NULL == node)
        {
            sys_log(LOGSTDOUT, "error:__mergeNode: read node of child %d of previous node %lx failed\n", div + 1, prevNode);
            return (0);
        }
        btreeDebug(tree, LOC_BTREE_0078);
        i    = rootNode->keyCount;

        if (!BTREE_IS_LEAF(rootNode))
        {
            rootNode->keys[i]     = keyDup(prevNode->keys[div], LOC_BTREE_0079);
            rootNode->keySizes[i] = prevNode->keySizes[div];
            rootNode->keyCount++;

            i++;
        }

        btreeDebug(tree, LOC_BTREE_0080);
        for (j = 0; j < node->keyCount; j++, i++)
        {
            rootNode->keys[i]     = keyDup(node->keys[j], LOC_BTREE_0081);
            rootNode->keySizes[i] = node->keySizes[j];
            rootNode->children[i] = node->children[j];
            rootNode->keyCount++;
        }
        btreeDebug(tree, LOC_BTREE_0082);

        rootNode->children[i]       = node->children[j];
        prevNode->children[div + 1] = rootNode->block->offset;

        GDB_SET_DIRTY(rootNode->block);
        GDB_SET_DIRTY(prevNode->block);

        btreeDebug(tree, LOC_BTREE_0083);

        btreeEraseNode(node);

        btreeDebug(tree, LOC_BTREE_0084);

        __removeKey2(tree, prevNode, div);

        btreeDebug(tree, LOC_BTREE_0085);
    }

    btreeDebug(tree, LOC_BTREE_0086);
    btreeWriteNode(node);
    btreeDebug(tree, LOC_BTREE_0087);
    btreeWriteNode(prevNode);
    btreeDebug(tree, LOC_BTREE_0088);
    btreeWriteNode(rootNode);
    btreeDebug(tree, LOC_BTREE_0089);

    btreeDestroyNode(node);
    btreeDebug(tree, LOC_BTREE_0090);

    return 1;
}

static uint8_t
__delete(BTree *tree, offset_t rootOffset, BTreeNode *prevNode,
         const uint8_t *key, uint8_t index, offset_t *filePos, uint8_t *merged)
{
    uint8_t success = 0;
    BTreeNode *rootNode;

    btreeDebug(tree, LOC_BTREE_0091);
    rootNode = btreeReadNode(tree, rootOffset);
    if(NULL == rootNode)
    {
        sys_log(LOGSTDOUT, "error:__delete: read root node from offset %d failed\n", rootOffset);
        return (0);
    }
    btreeDebug(tree, LOC_BTREE_0092);

    if (BTREE_IS_LEAF(rootNode))
    {
        success = __removeKey(tree, rootNode, key, filePos);
    }
    else
    {
        uint8_t i;

        for (i = 0;
             i < rootNode->keyCount && keyCmp(rootNode->keys[i], key) < 0;
             i++)
            ;

        btreeDebug(tree, LOC_BTREE_0093);
        success = __delete(tree, rootNode->children[i], rootNode, key, i, filePos, merged);
        btreeDebug(tree, LOC_BTREE_0094);
    }

    if (success == 0)
    {
        btreeDebug(tree, LOC_BTREE_0095);
        btreeDestroyNode(rootNode);
        btreeDebug(tree, LOC_BTREE_0096);

        return 0;
    }
    else if ((rootNode->block->offset == tree->root) ||
             (BTREE_IS_LEAF(rootNode)  && rootNode->keyCount >= tree->minLeaf) ||
             (!BTREE_IS_LEAF(rootNode) && rootNode->keyCount >= tree->minInt))
    {
        btreeDebug(tree, LOC_BTREE_0097);
        btreeDestroyNode(rootNode);
        btreeDebug(tree, LOC_BTREE_0098);

        return 1;
    }
    else
    {
        if (__borrowRight(tree, rootNode, prevNode, index) ||
            __borrowLeft(tree, rootNode, prevNode, index))
        {
            *merged = 0;
        }
        else
        {
            *merged = 1;
            btreeDebug(tree, LOC_BTREE_0099);
            __mergeNode(tree, rootNode, prevNode, index);
            btreeDebug(tree, LOC_BTREE_0100);
        }

        btreeDebug(tree, LOC_BTREE_0101);
        btreeWriteNode(rootNode);
        btreeDebug(tree, LOC_BTREE_0102);
        btreeWriteNode(prevNode);
        btreeDebug(tree, LOC_BTREE_0103);
    }

    btreeDebug(tree, LOC_BTREE_0104);
    btreeDestroyNode(rootNode);
    btreeDebug(tree, LOC_BTREE_0105);

    return 1;
}

offset_t
btreeDelete(BTree *tree, const uint8_t *key)
{
    uint8_t i;
    offset_t filePos;
    uint8_t merged, success;
    BTreeNode *rootNode;

    if (tree == NULL || key == NULL ||
        tree->block->db->mode == PM_MODE_READ_ONLY)
    {
        return 0;
    }

    if (tree->block->db->mode == PM_MODE_TEST)
    {
        return btreeSearch(tree, key, keyCmp);
    }

    btreeDebug(tree, LOC_BTREE_0106);

    filePos = 0;
    merged  = 0;
    success = 0;

    /* Read in the tree data. */
    tree->root     = btreeGetRootNode(tree);
    tree->leftLeaf = btreeGetLeftLeaf(tree);
    tree->size     = btreeGetTreeSize(tree);

    /* Read in the root node. */
    rootNode = btreeReadNode(tree, tree->root);
    if(NULL == rootNode)
    {
        sys_log(LOGSTDOUT, "error:btreeDelete: [1] read root node from offset %d failed\n", tree->root);
        return (0);
    }

    btreeDebug(tree, LOC_BTREE_0107);

    for (i = 0;
         i < rootNode->keyCount && keyCmp(rootNode->keys[i], key) < 0;
         i++)
        ;

    success = __delete(tree, tree->root, NULL, key, i, &filePos, &merged);

    if (success == 0)
    {
        btreeDebug(tree, LOC_BTREE_0108);
        btreeDestroyNode(rootNode);
        btreeDebug(tree, LOC_BTREE_0109);
        return 0;
    }

    btreeDebug(tree, LOC_BTREE_0110);

    btreeSetTreeSize(tree, tree->size - 1);
    btreeDebug(tree, LOC_BTREE_0111);

    if (BTREE_IS_LEAF(rootNode) && rootNode->keyCount == 0)
    {
        btreeDebug(tree, LOC_BTREE_0112);
        btreeSetRootNode(tree, 0);
        btreeDebug(tree, LOC_BTREE_0113);
        btreeEraseNode(rootNode);
        btreeDebug(tree, LOC_BTREE_0114);
    }
    else if (merged == 1 && rootNode->keyCount == 0)
    {
        BTreeNode *tempNode;

        btreeDebug(tree, LOC_BTREE_0115);
        btreeSetRootNode(tree, rootNode->children[0]);
        btreeDebug(tree, LOC_BTREE_0116);

        tempNode = btreeReadNode(tree, tree->root);
        if(NULL == tempNode)
        {
            sys_log(LOGSTDOUT, "error:btreeDelete: [2] read root node from offset %d failed\n", tree->root);
            return (0);
        }
        btreeDebug(tree, LOC_BTREE_0117);
        GDB_SET_DIRTY(tempNode->block);

        btreeWriteNode(tempNode);
        btreeDebug(tree, LOC_BTREE_0118);
        btreeDestroyNode(tempNode);
        btreeDebug(tree, LOC_BTREE_0119);

        btreeEraseNode(rootNode);
        btreeDebug(tree, LOC_BTREE_0120);
    }

    btreeDebug(tree, LOC_BTREE_0121);
    btreeDestroyNode(rootNode);
    btreeDebug(tree, LOC_BTREE_0122);

    return filePos;
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

