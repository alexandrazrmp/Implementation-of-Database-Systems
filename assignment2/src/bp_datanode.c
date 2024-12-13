#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "bp_file.h"
#include "record.h"
#include "bp_datanode.h"

BPLUS_DATA_NODE* create_root_data_node(int *fd,BF_Block* block){
    
    //initialize the first data node pointed by the root 
    BPLUS_DATA_NODE* BP_DATA;
    void* data;
    CALL_OR_EXIT(BF_AllocateBlock(*fd,block));
    data = BF_Block_GetData(block);
    BP_DATA = data;
    BP_DATA->record_counter = 1; 
    BP_DATA->NextDataBlockNum = -1; //we use the value -1 to indicate that there is no next data block
    last_block_num++; //we increase the global variable that we use as a block id
    BF_Block_SetDirty(block);
    return BP_DATA;
}

BPLUS_DATA_NODE* create_data_node(int *fd,BF_Block* block, int next){
    
    BPLUS_DATA_NODE* BP_DATA;
    void* data;
    CALL_OR_EXIT(BF_AllocateBlock(*fd,block));
    data = BF_Block_GetData(block);
    BP_DATA = data;

    //If the variable next isn't -1 it means that we are on the first data block (pointers[0])
    //next holds the integer (block id) that points to the already allocated block to the right (poiters[1])
    if (next!=-1){                                                              
        (BP_DATA->NextDataBlockNum) = next;                                       
        printf("POINTERS[1]=%d\n", next);                                       
    }

    last_block_num++;//we increase the global variable that we use as a block id
    BF_Block_SetDirty(block);
    return BP_DATA;
}

bool is_full_data(BPLUS_DATA_NODE* BP_DATA){

    if (BP_DATA->record_counter==8){
        return true;
    }
    return false;
}

void split_data(BPLUS_INDEX_NODE *INDEX_NODE,BPLUS_DATA_NODE *Data_Node,int* block, int* ins_index, int* ins_key,int key,int* fd){
    

    BF_Block *block1;
    BF_Block_Init(&block1);
    BPLUS_DATA_NODE* newdata=create_data_node(fd,block1, -1);   //the next will be -1 because we dont know the NextDataBlockNum yet 
    int index = *ins_index;

    // Array to hold the result of the keys after the insertion
    int *temp_keys;   
    temp_keys = malloc((Data_Node->record_counter + 1)*sizeof(int));
    int i = 0;   // Iterator for the key array
    int j = 0;  // Iterator for the temp_key array
    
    // Insert the new element in sorted order
    while (i < Data_Node->record_counter)
    {
        if (key < Data_Node->Records[i].id && j == i)
        {
            temp_keys[j] = key; 
            j++;
        }
        if(Data_Node->Records[i].id == 0){
            j++;
            i++;
            continue;
        }
        temp_keys[j] = Data_Node->Records[i].id;
        j++;
        i++;
    }

    // If the new element is the largest, insert it at the end
    if (j < INDEX_NODE->counter_keys + 1)
    {
        temp_keys[j] = key;
    }

    //We find mid of the temporary array that will be returned in the arguments of the function
    int mid = (Data_Node->record_counter + 1) / 2;
    int temp = Data_Node->record_counter;
    Data_Node->record_counter = 0;

    //
    int num;                                                    
    BF_GetBlockCounter(*fd, &num);                              
    num--;                                                       
    printf("split and next = %d\n", num);
    Data_Node->NextDataBlockNum = num;                         
    
    //we insert at the already existing data node the sorted values from the temporary array until the middle element 
    //and we change the record counter 
    *ins_key = temp_keys[mid];
    int z=0;
    for (i = 0; i <mid; i++)
    {   if(temp_keys[i] != key){
            Data_Node->record_counter++;
        }
        else{
            z=1;
        }
    }

    //we insert at the new data node we created the sorted values from the temporary array beginning from the middle element up to the end
    //and at the same time we count how many records we insert (record counter )
    j=0;
    newdata->record_counter = 0;
    for (i = mid ; i <=temp; i++)
    {
        if(temp_keys[i] != key){
            if(Data_Node->Records[i-z].id == 0){ 
                j=j+1;
                continue;
            }
            newdata->Records[j] = Data_Node->Records[i-z];
            newdata->record_counter+=1;
            j++;
        }
        else{
            z = 1;
        }
    }

    //we return the index that should be inserted in the index node along with key 
    CALL_OR_EXIT(BF_GetBlockCounter(*fd, ins_index));
    *ins_index =*ins_index-1;
    if(temp_keys[mid]<key){
        *block = *ins_index;
    }
   
    //free the array we created
    free(temp_keys);
    
    
    BF_Block_SetDirty(block1);
    //BF_Block_SetDirty(Data_Node);
    CALL_OR_EXIT(BF_UnpinBlock(block1));
    //CALL_OR_EXIT(BF_UnpinBlock(Data_Node));
    //add next block num
    return ;

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

    //traverse in the data blocks using NextDataBlockNum
    while (current_block_num != -1) {
        CALL_OR_EXIT(BF_GetBlock(*fd, current_block_num, block));
        void *data = BF_Block_GetData(block);
        BPLUS_DATA_NODE *data_node = (BPLUS_DATA_NODE *)data;

        printf("Data Node (Block: %d):\n", current_block_num);
        printf("  Record Counter: %d\n", data_node->record_counter);
        for (int i = 0; i < data_node->record_counter; i++) {
            printf("Person with id %d and name %s \n", data_node->Records[i].id,data_node->Records[i].name);
        }
        printf("EDW      %d\n", data_node->NextDataBlockNum);
        current_block_num = data_node->NextDataBlockNum;

        CALL_OR_EXIT(BF_UnpinBlock(block));
    }

}