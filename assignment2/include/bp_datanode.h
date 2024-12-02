#ifndef BP_DATANODE_H
#define BP_DATANODE_H
#include <record.h>
#include <record.h>
#include <bf.h>
#include <bp_file.h>
#include <bp_indexnode.h>



typedef struct {
    int record_counter;
    BF_Block* data_block;
    int NextDataBlockNum;
    //maybe platos dictionary
} BPLUS_DATA_NODE;

#endif 