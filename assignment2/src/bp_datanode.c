#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "bp_file.h"
#include "record.h"
#include "bp_datanode.h"


BPLUS_DATA_NODE* create_data_node(int *fd){
    BF_Block *block;
    BPLUS_DATA_NODE* BP_INFO;
    void* data;
    BF_Block_Init(&block);
    CALL_OR_EXIT(BF_AllocateBlock(*fd,block));
    data = BF_Block_GetData(block);
    BP_INFO = data;
    BP_INFO->record_counter = 0;
    BP_INFO->NextDataBlockNum = -1;
    BF_Block_SetDirty(block);
    return BP_INFO;
}

bool is_full_data(BPLUS_DATA_NODE* BP_DATA){

    if (BP_DATA->record_counter==4){
        return true;
    }
    return false;
}

void split_data(BPLUS_INDEX_NODE *INDEX_NODE,BPLUS_DATA_NODE *Data_Node,int* block, int* ins_index, int* ins_key,int key,int* fd){
    BPLUS_DATA_NODE* newdata=create_data_node(fd);

    int mid=Data_Node->record_counter/2;
    *ins_key=Data_Node->Records[mid].id;//return mid
    if (key > (*ins_key))
    {   *block = last_block_num + 1;
        if (key<Data_Node->Records[mid+1].id)
        {
            *ins_key=key;
        }
    }
    int j=0;
    for (int i = mid; i < Data_Node->record_counter; i++)
    {
        newdata->Records[j]=Data_Node->Records[i];
        // Data_Node.Records[i]=NULL;
        newdata->record_counter++;
        Data_Node->record_counter--;
        j++;
    }
  

    newdata->NextDataBlockNum = Data_Node->NextDataBlockNum;
    int new_block_id;
    BF_GetBlockCounter(*fd,&new_block_id);
    *ins_index = new_block_id - 1;
    Data_Node->NextDataBlockNum = new_block_id - 1;
    //sort
}

void print_data(int fd, int root_block_num){

    BF_Block *block;
    BF_Block_Init(&block);

    //traverse the tree to find the most left index in the last level (leaf level)
    int current_block_num = root_block_num;
    while (true) {
        CALL_OR_EXIT(BF_GetBlock(fd, current_block_num, block));
        void *data = BF_Block_GetData(block);
        BPLUS_INDEX_NODE *index_node = (BPLUS_INDEX_NODE *)data;
        
        // If its the leaf stop traversal
        if (index_node->leaf) {
            break;
        }

        current_block_num = index_node->pointers[0];
        CALL_OR_EXIT(BF_UnpinBlock(block));
    }

    while (current_block_num != -1) {
        CALL_OR_EXIT(BF_GetBlock(fd, current_block_num, block));
        void *data = BF_Block_GetData(block);
        BPLUS_DATA_NODE *data_node = (BPLUS_DATA_NODE *)data;

        printf("Data Node (Block: %d):\n", current_block_num);
        printf("  Record Counter: %d\n", data_node->record_counter);
        for (int i = 0; i < data_node->record_counter; i++) {
            printf("    Person with id %d and name %s %s\n", data_node->Records[i].id,data_node->Records[i].name,data_node->Records[i].surname );
        }
        
        current_block_num = data_node->NextDataBlockNum;

        CALL_OR_EXIT(BF_UnpinBlock(block));
    }

}