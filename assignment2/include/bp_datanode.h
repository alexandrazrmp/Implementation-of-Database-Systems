#ifndef BP_DATANODE_H
#define BP_DATANODE_H
#include <record.h>
#include <record.h>
#include <bf.h>
#include <bp_file.h>
#include <bp_indexnode.h>

//kokkino
typedef struct {

    int record_counter;
    BF_Block* data_block;
    BPLUS_INDEX_NODE* next;
    //maybe platos dictionary
} BPLUS_DATA_NODE;

#endif 