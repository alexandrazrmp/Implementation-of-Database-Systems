#ifndef BP_DATANODE_H
#define BP_DATANODE_H
#include <record.h>
#include <record.h>
#include <bf.h>
#include <bp_file.h>
#include <bp_indexnode.h>



typedef struct {

    int record_counter; //Number of stored records in the array
    int NextDataBlockNum; //The id of the next data block
    Record Records[4]; //Array to store the Records in the data block
    
} BPLUS_DATA_NODE;

BPLUS_DATA_NODE* create_root_data_node(int *fd,BF_Block* block);


BPLUS_DATA_NODE* create_data_node(int *fd,BF_Block* block, int next);


bool is_full_data(BPLUS_DATA_NODE* BP_DATA);

void split_data(BPLUS_INDEX_NODE *INDEX_NODE,BPLUS_DATA_NODE *Data_Node,int* block, int* ins_index, int* ins_key,int key,int* fd);

void print_data(int *fd, int root_block_num);

#endif 