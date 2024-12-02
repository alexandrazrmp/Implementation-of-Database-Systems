#ifndef BP_INDEX_NODE_H
#define BP_INDEX_NODE_H
#include <record.h>
#include <bf.h>
#include <bp_file.h>

//mayro
typedef struct
{
    BPLUS_INFO* info;
    int block_id;
    int counter_keys;

} BPLUS_INDEX_NODE;

#endif