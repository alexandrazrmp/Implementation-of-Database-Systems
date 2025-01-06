#include <merge.h>
#include <stdio.h>
#include "chunk.h"


CHUNK_Iterator CHUNK_CreateIterator(int fileDesc, int blocksInChunk){
    CHUNK_Iterator iterator;

    // Initialize the iterator fields
    iterator.file_desc = fileDesc;

    // Start from the first block (usually block 1, as block 0 might be metadata)
    iterator.current = 1;

    // Get the total number of blocks in the file to determine the last block ID
    int totalBlocks;
    //CALL_BF CHANGE
    CALL_BF(BF_GetBlockCounter(fileDesc, &totalBlocks));

    // Assign the last block ID and set the number of blocks per chunk
    iterator.lastBlocksID = totalBlocks - 1; 
    iterator.blocksInChunk = blocksInChunk;

    return iterator;
}

int CHUNK_GetNext(CHUNK_Iterator *iterator, CHUNK *chunk) {
    // Check if the current block is beyond the last block in the file
    if (iterator->current > iterator->lastBlocksID) {
        return -1;  // No more chunks, return error code (-1)
    }
    
    // Set up the chunk with current chunk details
    chunk->file_desc = iterator->file_desc;
    
    // Set the from_BlockId of the chunk as the current block of the iterator
    chunk->from_BlockId = iterator->current;

    // Set the to_BlockId as the end block for this chunk
    chunk->to_BlockId = iterator->current + iterator->blocksInChunk - 1;

    // Ensure that the to_BlockId doesn't exceed the last block in the file
    if (chunk->to_BlockId > iterator->lastBlocksID) {
        chunk->to_BlockId = iterator->lastBlocksID;
    }

    // Calculate the number of blocks in the chunk (inclusive range)
    chunk->blocksInChunk = chunk->to_BlockId - chunk->from_BlockId + 1;

    // Calculate the number of records in the chunk (approximated per block)
    int recordsInBlock = HP_GetMaxRecordsInBlock(iterator->file_desc);  // Get the max records per block
    chunk->recordsInChunk = chunk->blocksInChunk * recordsInBlock; // Assumes all blocks are full

    // Move the iterator's current position forward by the number of blocks in the chunk
    iterator->current = chunk->to_BlockId + 1;

    return 0;  // Successfully retrieved the next chunk
}


int CHUNK_GetIthRecordInChunk(CHUNK* chunk, int i, Record* record) {
    // Check if the given index 'i' is within valid bounds
    if (i < 0 || i >= chunk->recordsInChunk) {
        return -1;  // Index out of bounds, return error
    }

    // Calculate the block and the record position within that block
    int recordsPerBlock = HP_GetMaxRecordsInBlock(chunk->file_desc);
    
    // Find which block the i-th record resides in
    int blockIndex = i / recordsPerBlock;
    int recordIndexInBlock = i % recordsPerBlock;

    // Find the actual block ID within the chunk
    int blockId = chunk->from_BlockId + blockIndex;

    // Retrieve the block using BF_GetBlock
    BF_Block *block;
    BF_Block_Init(&block);
    BF_ErrorCode err = BF_GetBlock(chunk->file_desc, blockId, block);

    if (err != BF_OK) {
        BF_PrintError(err);
        // BF_Block_Destroy(&block);
        return -1;  // Error retrieving block
    }

    // Get the data from the block
    char *data = BF_Block_GetData(block);
    
    // Retrieve the record from the block based on the index
    *record = *(Record *)(data + recordIndexInBlock * sizeof(Record));

    // Unpin the block after use
    err = BF_UnpinBlock(block);
    if (err != BF_OK) {
        BF_PrintError(err);
        // BF_Block_Destroy(&block);
        return -1;  // Error unpinning block
    }
    CALL_BF(BF_UnpinBlock(block));
    // Destroy the block to free the memory
    // BF_Block_Destroy(&block); //If we lose data LOOK FIRST!!!!

    return 0;  // Successfully retrieved the record
}

int CHUNK_UpdateIthRecord(CHUNK* chunk, int i, Record record) {
    // Check if the given index 'i' is within valid bounds
    if (i < 0 || i >= chunk->recordsInChunk) {
        return -1;  // Index out of bounds, return error
    }

    // Calculate the block and the record position within that block
    int recordsPerBlock = HP_GetMaxRecordsInBlock(chunk->file_desc);

    // Find which block the i-th record resides in
    int blockIndex = i / recordsPerBlock;
    int recordIndexInBlock = i % recordsPerBlock;

    // Find the actual block ID within the chunk
    int blockId = chunk->from_BlockId + blockIndex;

    // Retrieve the block using BF_GetBlock
    BF_Block *block;
    BF_Block_Init(&block);
    BF_ErrorCode err = BF_GetBlock(chunk->file_desc, blockId, block);

    if (err != BF_OK) {
        BF_PrintError(err);
        //BF_Block_Destroy(&block);
        return -1;  // Error retrieving block
    }

    // Get the data from the block
    char *data = BF_Block_GetData(block);
    
    // Update the record at the specified index within the block
    *(Record *)(data + recordIndexInBlock * sizeof(Record)) = record;

    // Mark the block as dirty, indicating that we modified its contents
    BF_Block_SetDirty(block);

    // Unpin the block after the update
    err = BF_UnpinBlock(block);
    if (err != BF_OK) {
        BF_PrintError(err);
        //BF_Block_Destroy(&block);
        return -1;  // Error unpinning block
    }
    //Unpin Block
    CALL_BF(BF_UnpinBlock(block));
    // Destroy the block to free the memory
    //BF_Block_Destroy(&block);

    return 0;  // Successfully updated the record
}

void CHUNK_Print(CHUNK chunk){

    // Loop over each block in the chunk
    for (int blockId = chunk.from_BlockId; blockId <= chunk.to_BlockId; blockId++) {
        // Retrieve the block using BF_GetBlock
        BF_Block *block;
        BF_Block_Init(&block);
        BF_ErrorCode err = BF_GetBlock(chunk.file_desc, blockId, block);

        if (err != BF_OK) {
            BF_PrintError(err);
            //BF_Block_Destroy(&block);
            return;  // If there is an error retrieving the block, exit the function
        }

        // Get the data from the block
        char *data = BF_Block_GetData(block);

        // Print all records in the block
        int recordsPerBlock = HP_GetMaxRecordsInBlock(chunk.file_desc);
        for (int i = 0; i < recordsPerBlock; i++) {
            Record *record = (Record *)(data + i * sizeof(Record));
            
            // Print the record data
            printf("Block %d, Record %d: %d, %s, %s, %s\n", blockId, i, record->id, record->name, record->surname, record->city);
            
        }

        // Unpin the block after processing
        err = BF_UnpinBlock(block);
        if (err != BF_OK) {
            BF_PrintError(err);
           // BF_Block_Destroy(&block);
            return;  // If there's an error unpinning the block, exit the function
        }

        // Destroy the block to free the memory
        // BF_Block_Destroy(&block);
    }

}


CHUNK_RecordIterator CHUNK_CreateRecordIterator(CHUNK *chunk){
    CHUNK_RecordIterator iterator;
    iterator.chunk = *chunk;  // Initialize the iterator with the provided chunk
    iterator.currentBlockId = chunk->from_BlockId;  // Start at the first block
    iterator.cursor = 0;  // Start at the first record in the block

    return iterator;
}

int CHUNK_GetNextRecord(CHUNK_RecordIterator *iterator, Record *record) {
    BF_Block *block;
    BF_Block_Init(&block);

    while (1) {
        // Check if the current block is within the chunk's bounds
        if (iterator->currentBlockId > iterator->chunk.to_BlockId) {
            BF_Block_Destroy(&block);
            return -1;  // No more records in the chunk
        }

        // Retrieve the block
        BF_ErrorCode err = BF_GetBlock(iterator->chunk.file_desc, iterator->currentBlockId, block);
        if (err != BF_OK) {
            BF_PrintError(err);
            BF_Block_Destroy(&block);
            return -1;
        }

        // Get the data from the block
        char *data = BF_Block_GetData(block);
        int recordsPerBlock = HP_GetMaxRecordsInBlock(iterator->chunk.file_desc);

        // Check if the cursor is within the valid range for the block
        if (iterator->cursor < recordsPerBlock) {
            // Retrieve the record
            *record = *((Record *)(data + iterator->cursor * sizeof(Record)));
            iterator->cursor++;

            // If the cursor exceeds the block, reset and move to the next block
            if (iterator->cursor >= recordsPerBlock) {
                iterator->cursor = 0;
                iterator->currentBlockId++;
            }

            // Unpin the current block
            err = BF_UnpinBlock(block);
            if (err != BF_OK) {
                BF_PrintError(err);
                BF_Block_Destroy(&block);
                return -1;
            }

            BF_Block_Destroy(&block);
            return 0;  // Successfully retrieved a record
        }

        // If the cursor is invalid, move to the next block
        iterator->cursor = 0;
        iterator->currentBlockId++;

        // Unpin the current block before retrying
        err = BF_UnpinBlock(block);
        if (err != BF_OK) {
            BF_PrintError(err);
            BF_Block_Destroy(&block);
            return -1;
        }
    }
}
