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
    CALL_BF(BF_AllocateBlock(fd,block));
    data = BF_Block_GetData(block);
    BP_INFO->record_counter = 0;
    BP_INFO->NextDataBlockNum = -1;
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
    if (key>ins_key)
    {
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
        j++;
    }
    newdata->record_counter=j;
    Data_Node->record_counter=mid;

    newdata->NextDataBlockNum = Data_Node->NextDataBlockNum;
    int new_block_id;
    BF_GetBlockCounter(fd,&new_block_id);
    *ins_index = new_block_id - 1;
    Data_Node->NextDataBlockNum = new_block_id - 1;
    

}

