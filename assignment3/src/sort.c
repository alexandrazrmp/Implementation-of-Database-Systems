#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bf.h"
#include "hp_file.h"
#include "record.h"
#include "sort.h"
#include "merge.h"
#include "chunk.h"

bool shouldSwap(Record* rec1,Record* rec2){
    
    // First, compare the names alphabetically
    int nameComparison = strcmp(rec1->name, rec2->name);
    
    if (nameComparison > 0) {
        // rec1's name comes after rec2's name (swap needed)
        return true;
    } else if (nameComparison < 0) {
        // rec1's name comes before rec2's name (no swap needed)
        return false;
    } else {
        // Names are equal, compare the surnames
        int surnameComparison = strcmp(rec1->surname, rec2->surname);
        
        if (surnameComparison > 0) {
            // rec1's surname comes after rec2's surname (swap needed)
            return true;
        } else if (surnameComparison < 0) {
            // rec1's surname comes before rec2's surname (no swap needed)
            return false;
        } else {
            // Both name and surname are equal, no swap needed
            return false;
        }
    }

}

void swapRecords(Record* rec1, Record* rec2){
    Record temp = *rec1;
    *rec1 = *rec2;
    *rec2 = temp;
}

void sort_FileInChunks(int file_desc, int numBlocksInChunk){
    // Retrieve the total number of blocks in the file
    int totalBlocks;
    if (BF_GetBlockCounter(file_desc, &totalBlocks) != BF_OK) {
        BF_PrintError(BF_ERROR);
        return;
    }
    
    // Process the file in chunks
    int blocksProcessed = 1;
    
    int totalRecords = 0;

    while (blocksProcessed < totalBlocks) {
        // Calculate the end block for the current chunk
        int chunkEndBlock = blocksProcessed + numBlocksInChunk - 1;
        if (chunkEndBlock >= totalBlocks) {
            chunkEndBlock = totalBlocks - 1;
        }
        
        // Retrieve all records in this chunk
        Record* recordsInChunk = (Record*)malloc(sizeof(Record) * numBlocksInChunk * HP_GetMaxRecordsInBlock(file_desc));
        int recordCount = 0;
        
        for (int blockId = blocksProcessed; blockId <= chunkEndBlock; blockId++) {
            // Retrieve the records from each block in the chunk
            BF_Block* block;
            BF_Block_Init(&block);
            if (BF_GetBlock(file_desc, blockId, block) != BF_OK) {
                BF_PrintError(BF_ERROR);
                free(recordsInChunk);
                return;
            }

            // Fetch records from the block (assuming that HP_GetRecord function can load records into an array)
            int recordsInBlock = HP_GetRecordCounter(file_desc, blockId);
            for (int i = 0; i < recordsInBlock; i++) {
                Record* record = malloc(sizeof(Record));
                if (HP_GetRecord(file_desc, blockId, i, record) == 0) {
                    recordsInChunk[recordCount++] = *record;
                }
            }

            BF_UnpinBlock(block); // Unpin the block after fetching records
            BF_Block_Destroy(&block);  // Free the block
        }

        // Perform an in-place sorting of the records using Bubble Sort (or any other sorting method you prefer)
        for (int i = 0; i < recordCount - 1; i++) {
            for (int j = 0; j < recordCount - i - 1; j++) {
                if (shouldSwap(&recordsInChunk[j], &recordsInChunk[j + 1])) {
                    swapRecords(&recordsInChunk[j], &recordsInChunk[j + 1]);  // Swap if order is incorrect
                }
            }
        }

        // Write the sorted records back to the blocks in the chunk
        int recordIndex = 0;
        for (int blockId = blocksProcessed; blockId <= chunkEndBlock; blockId++) {
            BF_Block* block;
            BF_Block_Init(&block);
            if (BF_GetBlock(file_desc, blockId, block) != BF_OK) {
                BF_PrintError(BF_ERROR);
                free(recordsInChunk);
                return;
            }

            // Write sorted records back to the block
            int recordsInBlock = HP_GetRecordCounter(file_desc, blockId);
            for (int i = 0; i < recordsInBlock; i++) {
                if (recordIndex < recordCount) {
                    Record* record = &recordsInChunk[recordIndex++];
                    HP_UpdateRecord(file_desc, blockId, i, *record);  // Updating record in block
                }
            }

            BF_UnpinBlock(block); // Unpin the block after updating records
            BF_Block_Destroy(&block);  // Free the block
        }

        // Move to the next chunk
        blocksProcessed = chunkEndBlock + 1;
        
       
        // printf("Sorting completed\n");
        // for (int i = 0; i < numBlocksInChunk * HP_GetMaxRecordsInBlock(file_desc) ; i++)
        // {
        //     printf("the name is %s and surname %s and id  %d\n",recordsInChunk[i].name,recordsInChunk[i].surname,recordsInChunk[i].id);
        //     totalRecords++;
        // }
        // printf("\n\n\n"); 
        
        
        // free(recordsInChunk);  // Free the allocated memory for records in the chunk
    
    }

    // printf("Total records: %d\n", totalRecords);

    
}


/* Sorts records within a CHUNK in ascending order based on the name and surname of each person. */
void sort_Chunk(CHUNK* chunk) {
    int i, j;
    CHUNK_Iterator c_iter = CHUNK_CreateIterator(chunk->file_desc, chunk->blocksInChunk);
    CHUNK_RecordIterator r_iter = CHUNK_CreateRecordIterator(chunk);

    Record temp;  // Temporary record for swapping
    Record rec1, rec2;  // Variables to hold records

    // Outer loop: iterate through each record in the chunk
    for (i = 0; i < chunk->recordsInChunk; i++) {
        if (CHUNK_GetIthRecordInChunk(chunk, i, &rec1) == -1) {
            printf("Couldn't return record %d\n", i);
            return;
        }

        // Inner loop: compare the current record with subsequent records
        for (j = 0; j < (chunk->recordsInChunk) - i - 1; j++) {
            if (CHUNK_GetIthRecordInChunk(chunk, j, &rec2) == -1) {
                printf("Couldn't return record %d\n", j);
                return;
            }

            // Check if the two records should be swapped
            if (shouldSwap(&rec1, &rec2) == true) {
                // Swap the records
                swapRecords(&rec1, &rec2);

                // After swapping, update the chunk with the new records
                if (CHUNK_UpdateIthRecord(chunk, i, rec1) == -1) {
                    printf("Couldn't update record %d\n", i);
                    return;
                }

                if (CHUNK_UpdateIthRecord(chunk, j, rec2) == -1) {
                    printf("Couldn't update record %d\n", j);
                    return;
                }
            }
        }
    }
}
