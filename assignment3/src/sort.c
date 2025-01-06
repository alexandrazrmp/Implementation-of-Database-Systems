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

void sort_FileInChunks(int file_desc, int numBlocksInChunk) {
    int totalBlocks;

    // Get the total number of blocks in the file
    if (BF_GetBlockCounter(file_desc, &totalBlocks) != BF_OK) {
        BF_PrintError(BF_ERROR);
        return;
    }

    CHUNK_Iterator iterator = CHUNK_CreateIterator(file_desc, numBlocksInChunk);
    CHUNK chunk;

    // Iterate over chunks
    while (CHUNK_GetNext(&iterator, &chunk) == 0) {
        // Sort the current chunk
        sort_Chunk(&chunk);
    }
}


void sort_Chunk(CHUNK* chunk) {
    int recordCount = chunk->recordsInChunk;

    // Allocate memory for all records in the chunk
    Record* records = (Record*)malloc(sizeof(Record) * recordCount);
    if (!records) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return;
    }

    // Retrieve all records in the chunk
    for (int i = 0; i < recordCount; i++) {
        if (CHUNK_GetIthRecordInChunk(chunk, i, &records[i]) != 0) {
            fprintf(stderr, "Error: Couldn't retrieve record %d\n", i);
            free(records);
            return;
        }
    }

    // Sort records in-place using Bubble Sort (or any preferred sorting algorithm)
    for (int i = 0; i < recordCount - 1; i++) {
        for (int j = 0; j < recordCount - i - 1; j++) {
            if (shouldSwap(&records[j], &records[j + 1])) {
                swapRecords(&records[j], &records[j + 1]);
            }
        }
    }

    // Update the chunk with sorted records
    for (int i = 0; i < recordCount; i++) {
        if (CHUNK_UpdateIthRecord(chunk, i, records[i]) != 0) {
            fprintf(stderr, "Error: Couldn't update record %d\n", i);
            free(records);
            return;
        }
    }

    free(records);
}
