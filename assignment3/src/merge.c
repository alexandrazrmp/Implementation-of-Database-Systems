#include "merge.h"
#include <stdio.h>
#include <stdbool.h>

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
                if (HP_InsertEntry(output_FileDesc, outputRecord) == -1) {
                    fprintf(stderr, "Error writing record to output file.\n");
                    exit(EXIT_FAILURE);
                }

                // Advance the record iterator for the chosen chunk
                if (CHUNK_GetNextRecord(&recordIterators[minIndex], &records[minIndex]) != 0) {
                    // If no more records in this chunk, mark it as inactive
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
                    if (HP_InsertEntry(output_FileDesc, outputRecord) == -1) {
                        fprintf(stderr, "Error writing record to output file.\n");
                        exit(EXIT_FAILURE);
                    }
                }
                activeChunks[i] = 0;  // Mark chunk as fully processed
                chunkCount--;
            }
        }
    }
}





// void merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc) {
//     // Initialize the chunk iterator to start reading chunks
//     CHUNK_Iterator chunkIterator = CHUNK_CreateIterator(input_FileDesc, chunkSize);

//     // Allocate memory for chunks, their record iterators, and record storage
//     CHUNK chunks[bWay];
//     CHUNK_RecordIterator recordIterators[bWay];
//     Record records[bWay];
//     bool activeChunks[bWay];  // Tracks whether a chunk is still active

//     // Main merge loop
//     while (1) {
//         int chunkCount = 0; // Tracks the number of active chunks currently loaded

//         // Load up to `bWay` chunks into memory
//         for (int i = 0; i < bWay; i++) {
//             if (CHUNK_GetNext(&chunkIterator, &chunks[i]) == 0) {
//                 recordIterators[i] = CHUNK_CreateRecordIterator(&chunks[i]);
//                 if (CHUNK_GetNextRecord(&recordIterators[i], &records[i]) == 0) {
//                     activeChunks[i] = true;  // Mark chunk as active
//                     chunkCount++;
//                 } else {
//                     activeChunks[i] = false; // Mark as inactive if no records
//                 }
//             } else {
//                 activeChunks[i] = false; // Mark as inactive if no more chunks
//             }
//         }

//         // If no chunks were loaded, exit the loop
//         if (chunkCount == 0) {
//             break;
//         }

//         // Merge records from active chunks
//         while (chunkCount > 0) {
//             int minIndex = -1;

//             // Find the smallest record among active chunks
//             for (int i = 0; i < bWay; i++) {
//                 if (activeChunks[i]) {
//                     if (minIndex == -1 || shouldSwap(&records[minIndex], &records[i])) {
//                         minIndex = i;
//                     }
//                 }
//             }

//             // Write the smallest record to the output file
//             if (minIndex != -1) {
//                 if (HP_InsertEntry(output_FileDesc, records[minIndex]) == -1) {
//                     fprintf(stderr, "Error writing record to output file.\n");
//                     exit(EXIT_FAILURE);
//                 }

//                 // Advance the record iterator for the selected chunk
//                 if (CHUNK_GetNextRecord(&recordIterators[minIndex], &records[minIndex]) != 0) {
//                     // If no more records in this chunk, mark it as inactive
//                     activeChunks[minIndex] = false;
//                     chunkCount--;
//                 }
//             }
//         }

//         // Process any remaining records in partially processed chunks
//         for (int i = 0; i < bWay; i++) {
//             if (activeChunks[i]) {
//                 while (CHUNK_GetNextRecord(&recordIterators[i], &records[i]) == 0) {
//                     if (HP_InsertEntry(output_FileDesc, records[i]) == -1) {
//                         fprintf(stderr, "Error writing remaining record to output file.\n");
//                         exit(EXIT_FAILURE);
//                     }
//                 }
//                 activeChunks[i] = false; // Mark chunk as fully processed
//             }
//         }
//     }
// }




//JASON BELOW, WORKS

// void merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc) {
//     // Initialize a CHUNK_Iterator for traversing the input file in chunks of size chunkSize.
//     CHUNK_Iterator iterator = CHUNK_CreateIterator(input_FileDesc, chunkSize);
//     int minIndex = -1;

//     // Arrays to store the current chunk and record iterators for b chunks.
//     CHUNK* chunks[bWay];  // No need for CHUNK** anymore, just CHUNK* pointers
//     CHUNK_RecordIterator recordIterators[bWay];
//     Record* currentRecords[bWay]; // To store the current record from each chunk
//     int activeChunks[bWay];       // Track active chunks
//     int chunkCount = 0;
//     int i;

//     // Initialize chunks and record iterators for the first 'bWay' chunks.
//     for(i = 0; i < bWay; i++) {
//     // Allocate memory for each chunk and check if allocation is successful
//         chunks[i] = (CHUNK*)malloc(sizeof(CHUNK));
//         if (chunks[i] == NULL) {
//             printf("Memory allocation failed for chunk %d\n", i);
//             return;
//         }

//         // Debugging statement to confirm allocation
//         printf("Allocated memory for chunk %d at address %p\n", i, chunks[i]);

//         // Ensure chunk is initialized before passing to CHUNK_GetNext
//         if (CHUNK_GetNext(&iterator, chunks[i]) != 0) {
//             break;
//         }

//         // Initialize record iterator for this chunk
//         recordIterators[i] = CHUNK_CreateRecordIterator(chunks[i]);
//         currentRecords[i] = (Record*)malloc(sizeof(Record));  // Allocate memory for each record

//         if (CHUNK_GetNextRecord(&recordIterators[i], currentRecords[i]) == 0) {
//             activeChunks[chunkCount++] = i;  // Mark chunk as active
//         }
// }

//     // Process chunks in a merge fashion
//     while(chunkCount > 0) {
//         minIndex = -1; // Reset minIndex at the beginning of each loop

//         // Find the smallest record among active chunks
//         for(i = 0; i < chunkCount; i++) {
//             if (minIndex == -1 || shouldSwap(currentRecords[activeChunks[i]], currentRecords[activeChunks[minIndex]])) {
//                 minIndex = i;
//             }
//         }

//         // Insert the smallest record into the output file
//         if (minIndex != -1) {
//             HP_InsertEntry(output_FileDesc, *currentRecords[activeChunks[minIndex]]);
//         }

//         // Advance the iterator for the chunk that provided the smallest record
//         int activeChunk = activeChunks[minIndex];
//         if (CHUNK_GetNextRecord(&recordIterators[activeChunk], currentRecords[activeChunk]) != 0) {
//     // Attempt to refill the exhausted chunk
//             if (CHUNK_GetNext(&iterator, chunks[activeChunk]) == 0) {
//                 recordIterators[activeChunk] = CHUNK_CreateRecordIterator(chunks[activeChunk]);
//                 if (CHUNK_GetNextRecord(&recordIterators[activeChunk], currentRecords[activeChunk]) == 0) {
//                     continue;  // Successfully refilled the chunk
//                 }
//             }

//             // Remove the chunk from activeChunks
//             activeChunks[minIndex] = activeChunks[--chunkCount];
//         }
//     }

//     // Free allocated memory for current records and chunks
//     // for(i = 0; i < bWay; i++) {
//     //     if (currentRecords[i] != NULL) {
//     //         free(currentRecords[i]);
//     //     }
//     //     // if (chunks[i] != NULL) {
//     //     //     free(chunks[i]);
//     //     // }
//     // }
// }

//END OF JASON



// /* Merges bWay chunks of size chunkSize from the input file to the output file. 
//  * It uses CHUNK_Iterator and CHUNK_RecordIterator for chunk iteration and record retrieval.
//  */
// void merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc) {
//     // Initialize chunk iterators and record iterators
//     CHUNK_Iterator iterator = CHUNK_CreateIterator(input_FileDesc, chunkSize);
//     CHUNK_RecordIterator record_iter[bWay];

//     Record *records[bWay];
//     int records_valid[bWay];  // To track whether a chunk has more records to process

//     // Initialize each iterator for the bWay chunks
//     for (int i = 0; i < bWay; i++) {
//         record_iter[i] = CHUNK_CreateRecordIterator(iterator);  // Initialize record iterators for each chunk
//         records[i] = malloc(sizeof(Record));  // Allocate memory for the current record in the chunk
//         records_valid[i] = 1;  // Initially assume that each chunk has valid records
//     }

//     // Open the output file and start merging
//     while (1) {
//         // Keep track of the minimum record among the bWay chunks
//         Record *minRecord = NULL;
//         int minIndex = -1;

//         // Iterate over each chunk to get the next record
//         for (int i = 0; i < bWay; i++) {
//             if (records_valid[i] && CHUNK_GetNextRecord(iterator, record_iter[i], records[i]) == 0) {
//                 // If there are more records in the chunk, compare with the others
//                 if (minRecord == NULL || shouldSwap(minRecord, records[i])) {
//                     minRecord = records[i];
//                     minIndex = i;
//                 }
//             } else {
//                 // If no more records in the chunk, mark it as invalid
//                 records_valid[i] = 0;
//             }
//         }

//         // If all chunks are exhausted (no more valid records), break the loop
//         if (minIndex == -1) {
//             break;
//         }

//         // Write the minimum record found to the output file
//         HP_InsertEntry(output_FileDesc, *minRecord);

//         // After writing the minimum record, mark it as processed (retrieve the next record from the chunk)
//         records_valid[minIndex] = 0;  // Mark the current chunk as processed
//     }

//     // Clean up: free allocated memory for records
//     for (int i = 0; i < bWay; i++) {
//         free(records[i]);
//     }

// }
// void merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc) {
//     CHUNK_Iterator iterator = CHUNK_CreateIterator(input_FileDesc, chunkSize);
//     CHUNK* chunks[bWay];
//     CHUNK_RecordIterator recordIterators[bWay];
//     Record* currentRecords[bWay];
//     int activeChunks[bWay];
//     int chunkCount = 0;

//     // Initialize chunks and iterators
//     for (int i = 0; i < bWay; i++) {
//         chunks[i] = (CHUNK*)malloc(sizeof(CHUNK));
//         if (chunks[i] == NULL || CHUNK_GetNext(&iterator, chunks[i]) != 0) break;

//         recordIterators[i] = CHUNK_CreateRecordIterator(chunks[i]);
//         currentRecords[i] = (Record*)malloc(sizeof(Record));
//         if (CHUNK_GetNextRecord(&recordIterators[i], currentRecords[i]) == 0) {
//             activeChunks[chunkCount++] = i;
//         }
//     }

//     // Merge chunks
//     while (chunkCount > 0) {
//         int minIndex = -1;

//         // Find the smallest record
//         for (int i = 0; i < chunkCount; i++) {
//             if (minIndex == -1 || shouldSwap(currentRecords[activeChunks[minIndex]], currentRecords[activeChunks[i]])) {
//                 minIndex = i;
//             }
//         }

//         int activeChunk = activeChunks[minIndex];
//         HP_InsertEntry(output_FileDesc, *currentRecords[activeChunk]);

//         // Advance the iterator
//         if (CHUNK_GetNextRecord(&recordIterators[activeChunk], currentRecords[activeChunk]) != 0) {
//             if (CHUNK_GetNext(&iterator, chunks[activeChunk]) == 0) {
//                 recordIterators[activeChunk] = CHUNK_CreateRecordIterator(chunks[activeChunk]);
//                 if (CHUNK_GetNextRecord(&recordIterators[activeChunk], currentRecords[activeChunk]) == 0) continue;
//             }

//             // Remove exhausted chunk
//             activeChunks[minIndex] = activeChunks[--chunkCount];
//         }
//     }

//     // Free resources
//     for (int i = 0; i < bWay; i++) {
//         free(currentRecords[i]);
//         free(chunks[i]);
//     }
// }






