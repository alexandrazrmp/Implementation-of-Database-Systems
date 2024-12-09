#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "bp_file.h"
#include "record.h"
#include "bp_datanode.h"

BPLUS_DATA_NODE* create_root_data_node(int *fd){
    BF_Block* block;
    BPLUS_DATA_NODE* BP_INFO;
    void* data;
    BF_Block_Init(&block);
    CALL_OR_EXIT(BF_AllocateBlock(*fd,block));
    data = BF_Block_GetData(block);
    BP_INFO = data;
    BP_INFO->record_counter = 1;
    BP_INFO->NextDataBlockNum = -1;
    last_block_num++;
    BF_Block_SetDirty(block);
    return BP_INFO;
}

BPLUS_DATA_NODE* create_data_node(int *fd){
    BF_Block* block;
    BPLUS_DATA_NODE* BP_INFO;
    void* data;
    BF_Block_Init(&block);
    CALL_OR_EXIT(BF_AllocateBlock(*fd,block));
    data = BF_Block_GetData(block);
    BP_INFO = data;
    //BP_INFO->record_counter = 0;
    BP_INFO->NextDataBlockNum = -1;
    last_block_num++;
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
    

    BF_Block *block1;
    //if (!leaf_flag) BPLUS_INDEX_NODE *newindex = create_index_node(fd, INDEX_NODE->root, false, block1); 
    //else
    BPLUS_DATA_NODE* newdata=create_data_node(fd); 
    int index = *ins_index;

    int temp_keys[Data_Node->record_counter + 1];     // Array to hold the result of the keys after the insertion
    int i;                                           // Iterator for the key array
    int j = 0;                                       // Iterator for the temp_key array
    for (i = 0; i < Data_Node->record_counter ; i++)
    {
        if (key < Data_Node->Records[i].id && j == i)
        {
            temp_keys[j] = key; // Insert the new element in sorted order
            j++;
        }
        temp_keys[j] = Data_Node->Records[i].id;
        j++;
    }
    // If the new element is the largest, insert it at the end
    if (j < INDEX_NODE->counter_keys + 1)
    {
        temp_keys[j] = key;
    }
    int mid = (Data_Node->record_counter + 1) / 2;
    int temp = Data_Node->record_counter;
    Data_Node->record_counter = 0;
    *ins_key = temp_keys[mid];
    int z=0;
    for (i = 0; i < mid; i++)
    {   if(temp_keys[i] != key){
            Data_Node->record_counter++;
        }
        else{
            z=1;
        }
    }
    j=0;
    printf("Key = %d\n",key);
    newdata->record_counter = 0;
    for (i = mid ; i <=temp; i++)
    {
        if(temp_keys[i] != key){
            //printf("record->%d\n",Data_Node->Records[i-z].id);
            newdata->Records[j] = Data_Node->Records[i-z];
            newdata->record_counter+=1;
            j++;
        }
        else{
            z =1;
        }
    }
    printf("%d\n",newdata->record_counter);
    CALL_OR_EXIT(BF_GetBlockCounter(*fd, ins_index));
    if(temp_keys[mid]<key){
        *block = *ins_index - 1;
    }
    //BF_Block_SetDirty(block1);
    //CALL_OR_EXIT(BF_UnpinBlock(block1));
    return ;











    // int mid=Data_Node->record_counter/2;
    // printf("mid = %d\n",mid);
    // *ins_key=Data_Node->Records[mid].id;//return mid
    // if (key > (*ins_key))
    // {   *block = last_block_num + 1;
    //     if (key<Data_Node->Records[mid+1].id)
    //     {
    //         *ins_key=key;
    //     }
    // }
    // int j=0;
    // printf("In split data ins_key = %d\n",*ins_key);
    // for (int i = mid; i < Data_Node->record_counter; i++)
    // {
    //     printf("GG\n");
    //     newdata->Records[j]=Data_Node->Records[i];
    //     // Data_Node.Records[i]=NULL;
    //     newdata->record_counter++;
    //     Data_Node->record_counter--;
    //     j++;
    // }
  

    // newdata->NextDataBlockNum = Data_Node->NextDataBlockNum;
    // int new_block_id;
    // BF_GetBlockCounter(*fd,&new_block_id);
    // *ins_index = new_block_id - 1;
    // Data_Node->NextDataBlockNum = new_block_id - 1;
    // //sort
}

void print_data(int *fd, int root_block_num){

    BF_Block *block;
    BF_Block_Init(&block);

    //traverse the tree to find the most left index in the last level (leaf level)
    int current_block_num = root_block_num;
    while (true) {
        CALL_OR_EXIT(BF_GetBlock(*fd, current_block_num, block));
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
        CALL_OR_EXIT(BF_GetBlock(*fd, current_block_num, block));
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