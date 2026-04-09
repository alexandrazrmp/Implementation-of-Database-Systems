# Implementation of Database Systems

Course repository for an **Implementation of Database Systems** class. The project is split into three C assignments that build progressively on top of a block-file layer:

1. **Heap file management**
2. **B+ tree indexing**
3. **External merge sort over heap files**

---

Each assignment is self-contained and follows the same layout:

- `include/`: public headers
- `src/`: implementation files
- `examples/`: driver programs / demos
- `lib/`: prebuilt support libraries used at link time
- `build/`: compiled binaries

---

## What the repository implements

### Assignment 1 — Heap file layer

This part implements a simple **heap file** abstraction on top of the provided block-file (`BF`) library.

Core record model:

```c
typedef struct Record {
    int id;
    char name[15];
    char surname[20];
    char city[20];
} Record;
```

Main files:

- `assignment1/include/hp_file.h`
- `assignment1/src/hp_file.c`
- `assignment1/src/record.c`
- `assignment1/examples/bf_main.c`

Implemented heap-file API:

- `HP_CreateFile(char *fileName)`
- `HP_OpenFile(char *fileName, int *file_desc)`
- `HP_CloseFile(int file_desc, HP_info* header_info)`
- `HP_InsertEntry(int file_desc, HP_info* header_info, Record record)`
- `HP_GetAllEntries(int file_desc, HP_info* header_info, int id)`

### How it works

- The first block stores heap-file metadata in `HP_info`.
- Records are appended sequentially to the last data block.
- When the last block becomes full, a new one is allocated.
- Search is linear and scans the file block by block.

### Notes

- The implementation computes block capacity from `BF_BLOCK_SIZE / sizeof(Record)`.
- The example `bf_main.c` demonstrates raw block allocation and reading through the BF library.
  
---

### Assignment 2 — B+ tree file and index nodes

This assignment builds a **B+ tree index** for the same `Record` type.

Main files:

- `assignment2/include/bp_file.h`
- `assignment2/include/bp_datanode.h`
- `assignment2/include/bp_indexnode.h`
- `assignment2/src/bp_file.c`
- `assignment2/src/bp_datanode.c`
- `assignment2/src/bp_indexnode.c`
- `assignment2/examples/bp_main.c`
- `assignment2/examples/bp_main_ex.c`

Core structures:

- `BPLUS_INFO`: tree metadata (`height`, `root`)
- `BPLUS_DATA_NODE`: leaf/data block node with linked next-data-block pointer
- `BPLUS_INDEX_NODE`: internal/leaf-aware index node with ordered keys and child pointers

Important implementation details visible from the headers:

- Data nodes hold `RECORDS_NUM = 8` records.
- Index nodes hold `KEYS_NUM = 31` keys and `KEYS_NUM + 1` pointers.
- The code includes explicit split helpers for data nodes, index nodes, and root splitting.

Key operations exposed by the code:

- File lifecycle: create, open, close
- Insert into the B+ tree
- Search for a record by key
- Print the tree / index structure

### Demo behavior

`bp_main.c` shows the intended usage pattern:

- create/open `data.db`
- insert **5000 random records**
- search for record with `id = 500`
- print the resulting index structure

### Implementation approach

From the bundled assignment notes, the insertion path is based on:

- recursive search down the tree
- fullness checks before/after insertion
- sorted insertion of keys inside index blocks
- split propagation upward when needed
- special handling when the root itself overflows

---

### Assignment 3 — External merge sort

This part implements an **external merge sort** pipeline over heap files.

### Main concepts

- `CHUNK`: a contiguous range of blocks in a heap file
- `CHUNK_Iterator`: iterates chunk-by-chunk through a file
- `CHUNK_RecordIterator`: iterates record-by-record inside a chunk

Main operations:

- `sort_FileInChunks(int file_desc, int numBlocksInChunk)`
- `sort_Chunk(CHUNK* chunk)`
- `merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc)`

### Sorting order

Records are sorted in **ascending order by `name`, then by `surname`**.

### Workflow

1. Create and populate a heap file with random records.
2. Sort each chunk independently in-place.
3. Repeatedly merge `bWay` sorted chunks into new heap files.
4. Continue until the output becomes a single sorted run.

### Included demos

`sort_main.c` uses:

- `RECORDS_NUM = 500`
- `chunkSize = 5`
- `bWay = 4`

`sort_main_upload.c` includes two larger test scenarios and prints the number of chunks after each sort/merge phase:

- `./test1.db` with `4608` records, `chunkSize = 1`, `bWay = 2`
- `./test2.db` with `11520` records, `chunkSize = 5`, `bWay = 4`

## Build and run

Each assignment has its own `Makefile`.

### Assignment 1

```bash
cd assignment1
make bf
./build/bf_main
```

`Makefile` also contains heap-file related targets such as `hp` / `main` that compile a heap-file example binary.

### Assignment 2

```bash
cd assignment2
make bplus
./build/bplus_main
```

### Assignment 3

```bash
cd assignment3
make sort
./build/sort_main
```

Alternative upload/test driver:

```bash
cd assignment3
make sort_upload
./build/sort_main_upload
```

---

## Dependencies

This repository expects the provided support libraries used in the course exercises:

- `bf` block-file library
- in assignment 3, a heap-file library linked as `-lhp_file`

Linking is handled by the assignment Makefiles via the local `lib/` directories.
