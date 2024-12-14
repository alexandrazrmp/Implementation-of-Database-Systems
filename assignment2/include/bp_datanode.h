#ifndef BP_DATANODE_H
#define BP_DATANODE_H
#define RECORDS_NUM 8

#include <record.h>
#include <record.h>
#include <bf.h>
#include <bp_file.h>
#include <bp_indexnode.h>


typedef struct {

    int record_counter; //Number of stored records in the array
    int NextDataBlockNum; //The id of the next data block
    Record Records[RECORDS_NUM]; //Array to store the Records in the data block
    
} BPLUS_DATA_NODE;

//function that creates a new data node 
BPLUS_DATA_NODE* create_data_node(int *fd,BF_Block* block, int next);
BPLUS_DATA_NODE* create_root_data_node(int *fd,BF_Block* block);

// Function to check if an data node is full
bool is_full_data(BPLUS_DATA_NODE* BP_DATA);

//function that splits the data node
void split_data(BPLUS_INDEX_NODE *INDEX_NODE,BPLUS_DATA_NODE *Data_Node,int* block, int* ins_index, int* ins_key,int key,int* fd);


#endif 