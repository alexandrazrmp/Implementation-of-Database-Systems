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
    memcpy(data,BP_DATA,sizeof(BPLUS_DATA_NODE));
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
    }

    last_block_num++;//we increase the global variable that we use as a block id
    BF_Block_SetDirty(block);
    memcpy(data,BP_DATA,sizeof(BPLUS_DATA_NODE));
    return BP_DATA;
}

bool is_full_data(BPLUS_DATA_NODE* BP_DATA){

    if (BP_DATA->record_counter==RECORDS_NUM){
        return true;
    }
    return false;
}

void split_data(BPLUS_INDEX_NODE *INDEX_NODE,BPLUS_DATA_NODE *Data_Node,int* block, int* ins_index, int* ins_key,int key,int* fd){
    

    BF_Block *block1;
    BF_Block *block2;

    BF_Block_Init(&block1);
    BF_Block_Init(&block2);
    BPLUS_DATA_NODE* newdata=create_data_node(fd,block1, -1);   //the next will be -1 because we dont know the NextDataBlockNum yet 
    CALL_OR_EXIT(BF_GetBlock(*fd,*block,block2));
    int index = *ins_index;
    void* data;

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

    int num;                                                    
    BF_GetBlockCounter(*fd, &num);                              
    num--;                                                       
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

    //we insert at the new data node the sorted values from the temporary array beginning from the middle element up to the end
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
    //We copy the data block in which the new record WILL NOT enter
    if(temp_keys[mid]<key){
        *block = *ins_index;
        data = BF_Block_GetData(block2);
        memcpy(data,Data_Node,sizeof(BPLUS_DATA_NODE));
        BF_Block_SetDirty(block2);
        CALL_OR_EXIT(BF_UnpinBlock(block2));
    }
    else{
        data = BF_Block_GetData(block1);
        memcpy(data,newdata,sizeof(BPLUS_DATA_NODE));
        BF_Block_SetDirty(block1);
        CALL_OR_EXIT(BF_UnpinBlock(block1));
    }

    //free the array we created
    free(temp_keys);  
    return ;

}
