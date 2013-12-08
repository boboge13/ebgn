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

GdbBlock *
gdbCacheAddBlockNoLock(GDatabase *db, GdbBlock *block)
{
    GdbBlock *tempBlock;
    uint32_t i;

    if (block->offset == 0)
    {
        sys_log(LOGSTDOUT, "error:gdbCacheAddBlockNoLock: Trying to add a block to the list with offset 0.\n");
        abort();
    }

    if (block->inList == 1)/*already in cache list*/
    {
        return block;
    }

    /* See if it's already in the list. */
    tempBlock = gdbCacheGetBlockNoLock(db, block->offset);
    if (tempBlock != NULL)
    {
        sys_log(LOGSTDOUT, "[DEBUG] gdbCacheAddBlockNoLock: get cached block %lx with detail %lx and offset %d, db %lx (vs %lx)\n",
                            block, block->detail, block->offset, block->db, db);
        return tempBlock;
    }

    if (db->openBlockCount >= db->openBlockSize)
    {
        GdbBlock **newBlocks;
        uint32_t   newSize;

        newSize = 2 * db->openBlockSize;

        MEM_CHECK(newBlocks = (GdbBlock **)SAFE_MALLOC(newSize * sizeof(GdbBlock *), LOC_DB_0068));
        memset(newBlocks, 0, newSize * sizeof(GdbBlock *));

        for (i = 0; i < db->openBlockSize; i++)
        {
            newBlocks[i] = db->openBlocks[i];
        }
        SAFE_FREE(db->openBlocks, LOC_DB_0069);

        db->openBlocks    = newBlocks;
        db->openBlockSize = newSize;
    }

    /* Find a place to put this. */
    for (i = 0; i < db->openBlockSize; i++)
    {
        if (db->openBlocks[i] == NULL)
        {
            db->openBlocks[i] = block;
            db->openBlockCount++;

            block->refCount++;
            block->inList = 1;

            return block;
        }
    }

    sys_log(LOGSTDOUT, "error:gdbCacheAddBlockNoLock: Unable to place the open block in the block list!\n");
    return NULL;
}

GdbBlock *
gdbCacheAddBlock(GDatabase *db, GdbBlock *block)
{
    GdbBlock *block_cached;
    gdbLockFreeBlockList(db, DB_WRITE_LOCK, LOC_DB_0070);
    block_cached = gdbCacheAddBlockNoLock(db, block);
    //sys_log(LOGSTDOUT, "[DEBUG] gdbCacheAddBlock: openBlockCount %d, openBlockSize %d\n", db->openBlockCount, db->openBlockSize);
    gdbUnlockFreeBlockList(db, LOC_DB_0071);
    return block_cached;
}

uint8_t
gdbCacheRemoveBlockNoLock(GDatabase *db, GdbBlock *block)
{
    uint32_t i;

    if (block->offset == 0)
    {
        sys_log(LOGSTDOUT, "error:gdbCacheRemoveBlockNoLock: Trying to remove block from list with offset 0\n");
        abort();
    }

    if (db->openBlockCount == 0)
    {
        sys_log(LOGSTDOUT, "warn:gdbCacheRemoveBlockNoLock: db->openBlockCount == 0\n");
        return 0;
    }

    for (i = 0; i < db->openBlockSize; i++)
    {
        if (db->openBlocks[i] != NULL &&
            db->openBlocks[i]->offset == block->offset)
        {
            db->openBlocks[i]->refCount--;

            if (db->openBlocks[i]->refCount <= 0)
            {
                db->openBlocks[i] = NULL;
                db->openBlockCount--;
                block->inList = 0;

                return 0;
            }

            return db->openBlocks[i]->refCount;
        }
    }

    sys_log(LOGSTDOUT, "error:gdbCacheRemoveBlockNoLock: No open block found at offset %d!\n",
                        block->offset);

    return 0;
}

uint8_t
gdbCacheRemoveBlock(GDatabase *db, GdbBlock *block)
{
    uint8_t refCount;

    if (block->offset == 0)
    {
        sys_log(LOGSTDOUT, "error:gdbCacheRemoveBlock: Trying to remove block from list with offset 0\n");
        abort();
    }

    gdbLockFreeBlockList(db, DB_WRITE_LOCK, LOC_DB_0072);
    refCount = gdbCacheRemoveBlockNoLock(db, block);
    gdbUnlockFreeBlockList(db, LOC_DB_0073);
    return refCount;
}

GdbBlock *
gdbCacheGetBlockNoLock(GDatabase *db, offset_t offset)
{
    uint32_t i;

    for (i = 0; i < db->openBlockSize; i++)
    {
        if (db->openBlocks[i] != NULL &&
            db->openBlocks[i]->offset == offset)
        {
            db->openBlocks[i]->refCount++;

            return db->openBlocks[i];
        }
    }

    return NULL;
}

GdbBlock *
gdbCacheGetBlock(GDatabase *db, offset_t offset)
{
    GdbBlock *block;

    gdbLockFreeBlockList(db, DB_READ_LOCK, LOC_DB_0074);
    block = gdbCacheGetBlockNoLock(db, offset);
    gdbUnlockFreeBlockList(db, LOC_DB_0075);
    return block;
}

void
gdbCachePrintBlockNoLock(LOG *log, const GDatabase *db)
{
    uint32_t i;

    for (i = 0; i < db->openBlockSize; i++)
    {
        if(db->openBlocks[i] != NULL)
        {
            sys_log(log, "[DEBUG] gdbCachePrintBlockNoLock: [%d] db %lx, block %lx, offset %d, refCount %d, detail %lx\n", i,
                         db, db->openBlocks[i], db->openBlocks[i]->offset, db->openBlocks[i]->refCount, db->openBlocks[i]->detail
                         );
        }
    }
    return;
}


void
gdbCachePrintBlock(LOG *log, GDatabase *db)
{
    gdbLockFreeBlockList(db, DB_READ_LOCK, LOC_DB_0076);
    gdbCachePrintBlockNoLock(log, db);
    gdbUnlockFreeBlockList(db, LOC_DB_0077);
    return;
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/


