#include "merge.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>


void merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc) {
    // Initialize the chunk iterator to start from block 1 (skipping metadata in block 0)
    CHUNK_Iterator chunkIterator = CHUNK_CreateIterator(input_FileDesc, chunkSize);

    // Allocate memory for chunks and their record iterators
    CHUNK chunks[bWay];
    CHUNK_RecordIterator recordIterators[bWay];
    Record records[bWay];
    int activeChunks[bWay];  // Tracks active chunks (1 for active, 0 for exhausted)

    // Output record buffer for writing
    Record outputRecord;

    // Loop to process all chunks
    while (1) {
        int chunkCount = 0;  // Count of chunks currently loaded

        // Load up to bWay chunks into the arrays
        for (int i = 0; i < bWay; i++) {
            if (CHUNK_GetNext(&chunkIterator, &chunks[i]) == 0) {
                recordIterators[i] = CHUNK_CreateRecordIterator(&chunks[i]);
                if (CHUNK_GetNextRecord(&recordIterators[i], &records[i]) == 0) {
                    activeChunks[i] = 1;  // Mark this chunk as active
                    chunkCount++;
                } else {
                    activeChunks[i] = 0;  // Mark as inactive if no records
                }
            } else {
                activeChunks[i] = 0;  // Mark as inactive if no more chunks
            }
        }

        // If no chunks were loaded, we are done
        if (chunkCount == 0) {
            break;
        }

        // Merge the bWay chunks
        while (chunkCount > 0) {
            int minIndex = -1;

            // Find the smallest record among the active chunks
            for (int i = 0; i < bWay; i++) {
                if (activeChunks[i]) {
                    if (minIndex == -1 || shouldSwap(&records[minIndex], &records[i])) {
                        minIndex = i;
                    }
                }
            }

            // Write the smallest record to the output file
            if (minIndex != -1) {
                outputRecord = records[minIndex];

                // Write the record to the output file
                if(strlen(outputRecord.name) != 0 || strlen(outputRecord.surname) != 0){
                    if (HP_InsertEntry(output_FileDesc, outputRecord) == -1) {
                        fprintf(stderr, "Error writing record to output file.\n");
                        exit(EXIT_FAILURE);
                    }
                }
                if (CHUNK_GetNextRecord(&recordIterators[minIndex], &records[minIndex]) != 0) {
                    activeChunks[minIndex] = 0;
                    chunkCount--;
                }
                
            }
        }

        // If fewer than bWay chunks are left, process them before exiting
        for (int i = 0; i < bWay; i++) {
            if (activeChunks[i]) {

                // Write all remaining records for active chunks
                while (CHUNK_GetNextRecord(&recordIterators[i], &records[i]) == 0) {
                    outputRecord = records[i];
                    if(strlen(outputRecord.name) != 0 || strlen(outputRecord.surname) != 0){
                        if (HP_InsertEntry(output_FileDesc, outputRecord) == -1) {
                            fprintf(stderr, "Error writing record to output file.\n");
                            exit(EXIT_FAILURE);
                        }
                    }
                }
                activeChunks[i] = 0;  // Mark chunk as fully processed
                chunkCount--;
            }
        }

    }
    
}


