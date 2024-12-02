#ifndef BP_DATANODE_H
#define BP_DATANODE_H
#include <record.h>
#include <record.h>
#include <bf.h>
#include <bp_file.h>
#include <bp_indexnode.h>



typedef struct {
    int record_counter;
    
    int NextDataBlockNum;
    Record Records[4]; 
    //maybe platos dictionary
} BPLUS_DATA_NODE;

BPLUS_DATA_NODE* create_data_node(int *fd);

#endif 