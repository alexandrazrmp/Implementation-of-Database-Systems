#include <merge.h>
#include <stdio.h>
#include <stdbool.h>

// void merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc ){
//     // Initialize a CHUNK_Iterator for traversing the input file in chunks of size chunkSize.
//     CHUNK_Iterator iterator = CHUNK_CreateIterator(input_FileDesc, chunkSize);
    
//     // Arrays to store the current chunk and record iterators for b chunks.
//     CHUNK chunks[bWay];
//     CHUNK_RecordIterator recordIterators[bWay];

//     // Initialize the chunks and iterators for the first 'bWay' chunks.
//     int chunkCount = 0;
//     for (int i = 0; i < bWay && CHUNK_GetNext(&iterator, &chunks[i]) == 0; i++) {
//         recordIterators[i] = CHUNK_CreateRecordIterator(&chunks[i]);
//         chunkCount++;
//     }

//     // Allocate memory for the merged records (temporary storage for the merged output).
//     Record mergedRecord;
//     Record* tempRecords = (Record*)malloc(chunkSize * sizeof(Record));  // To hold a chunk of records
//     int recordsInTemp = 0;

//     // Process chunks in a merge fashion
//     while (chunkCount > 0) {
//         // Find the smallest record among the b chunks
//         int minIndex = -1;
//         for (int i = 0; i < chunkCount; i++) {
//             if (CHUNK_GetNextRecord(&recordIterators[i], &mergedRecord) == 0) {
//                 if (minIndex == -1 || shouldSwap(&mergedRecord, &tempRecords[minIndex])) {
//                     minIndex = i;
//                     tempRecords[recordsInTemp++] = mergedRecord;
//                 }
//             }
//         }
        
//         // Once the smallest record is found, write it to the output file.
//         // Here you would add code to write the record to the output file.
//         // e.g., WRITE_RECORD_TO_FILE(output_FileDesc, &tempRecords[recordsInTemp - 1]);

//         // If the chunk from which the smallest record came has more records, continue iterating
//         if (recordsInTemp < chunkSize) {
//             chunkCount--;
//         }
//     }

//     // Free the dynamically allocated memory
//     free(tempRecords);
// }

void merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc) {
    // Initialize a CHUNK_Iterator for traversing the input file in chunks of size chunkSize.
    CHUNK_Iterator iterator = CHUNK_CreateIterator(input_FileDesc, chunkSize);
    
    // Arrays to store the current chunk and record iterators for b chunks.
    CHUNK chunks[bWay];
    CHUNK_RecordIterator recordIterators[bWay];
    Record currentRecords[bWay]; // To store the current record from each chunk
    int activeChunks[bWay];      // Track active chunks
    int chunkCount = 0;

    // Initialize the chunks and iterators for the first 'bWay' chunks.
    for (int i = 0; i < bWay && CHUNK_GetNext(&iterator, &chunks[i]) == 0; i++) {

        recordIterators[i] = CHUNK_CreateRecordIterator(&chunks[i]);

        if (CHUNK_GetNextRecord(&recordIterators[i], &currentRecords[i]) == 0) {
            activeChunks[chunkCount++] = i; // Mark chunk as active

        }
    }
printf("end of loop i < bWay\n");

    // Initialize output file tracking
    int outputBlockId = HP_GetIdOfLastBlock(output_FileDesc) + 1;
    int outputCursor = 0;
    printf("1\n");
    // Process chunks in a merge fashion
    while (chunkCount > 0) {
        printf(" chunck  %d\n",chunkCount);        
        // Find the smallest record among active chunks
        int minIndex = -1;
        for (int i = 0; i < chunkCount; i++) {
            int chunkIndex = activeChunks[i];
            if (minIndex == -1 || shouldSwap(&currentRecords[chunkIndex], &currentRecords[minIndex])) {
                minIndex = chunkIndex;
            }
        }
        printf("2\n");
        // Write the smallest record directly to the output file
        if (outputCursor == HP_GetMaxRecordsInBlock(output_FileDesc)) {
            // Advance to the next block if the current block is full
            if(outputBlockId<iterator.lastBlocksID){

                outputBlockId++;
                outputCursor = 0;
            }
           
        }
        printf("2.12\n");
    
        if (HP_UpdateRecord(output_FileDesc, outputBlockId, outputCursor++, currentRecords[minIndex]) != 1) {
            fprintf(stderr, "Error writing record to output file\n");
            return;
        }
        printf("3\n");
        // Advance the iterator for the chunk that provided the smallest record
        if (CHUNK_GetNextRecord(&recordIterators[minIndex], &currentRecords[minIndex]) != 0) {
            // Remove the chunk from activeChunks if it is exhausted
            for (int i = 0; i < chunkCount; i++) {
                if (activeChunks[i] == minIndex) {
                    activeChunks[i] = activeChunks[--chunkCount];
                    break;
                }
            }
        }
    }

    printf("Merging completed\n");
    // Ensure all blocks are unpinned
    for (int i = 1; i <= outputBlockId; i++) {
        HP_Unpin(output_FileDesc, i);
    }
}







