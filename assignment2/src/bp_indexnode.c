#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "bp_file.h"
#include "record.h"
#include "bp_indexnode.h"
#include "bp_datanode.h"

// BPLUS_INDEX_NODE* create_index_node(int *fd,int last_block){
//     BF_Block *block;
//     BPLUS_INDEX_NODE* BP_INFO;
//     void* data;
//     BF_Block_Init(&block);
//     CALL_BF(BF_AllocateBlock(fd,block));
//     data = BF_Block_GetData(block);
//     BP_INFO->block_id = last_block + 1;
//     BP_INFO->counter_keys = 0;
//     // BP_INFO->height=height;
//     return BP_INFO;
// }

BPLUS_INDEX_NODE* create_index_node(int *fd, int last_block, bool is_root, bool is_leaf) {
    BF_Block *block;
    BPLUS_INDEX_NODE* BP_INFO;
    void* data;
    BF_Block_Init(&block);
    CALL_BF(BF_AllocateBlock(fd, block));
    data = BF_Block_GetData(block);

    //initialize the index node structure
    BP_INFO->block_id = last_block + 1; 
    BP_INFO->counter_keys = 0;         
    BP_INFO->root = is_root;          
    BP_INFO->leaf = is_leaf;        


    // memset(BP_INFO->keys, 0, sizeof(BP_INFO->keys));
    // memset(BP_INFO->pointers, -1, sizeof(BP_INFO->pointers)); 
    // memcpy(data, BP_INFO, sizeof(BPLUS_INDEX_NODE));
    // BF_Block_SetDirty(block);//maybe dirty

    return BP_INFO;
}

bool is_full_index(BPLUS_INDEX_NODE* BP_INFO){

    if (BP_INFO->counter_keys==4){
        return true;
    }
    return false;
}

int search(BPLUS_INDEX_NODE* INDEX_NODE,BPLUS_INFO* BP_INFO ,int key,int* fd,int* block, int *ins_index, int* ins_key){
    int i;
    BF_Block * dataBlock;
    void* data;
    BF_Block_Init(&dataBlock);
    
    if(INDEX_NODE->keys[(INDEX_NODE->counter_keys)-1]<key){
        i = (INDEX_NODE->counter_keys)-1;
    }
    else{
        for (i = 0; i <INDEX_NODE->counter_keys; i++)
        {
            if(INDEX_NODE->keys[i]>key){
                break;
            }
        }
    }       //i holds the pointer to the block we must search
    CALL_BF(BF_GetBlock(fd,i,dataBlock));
    data = BF_Block_GetData(dataBlock);
    //If we have reached final level of b-tree
    if(INDEX_NODE->leaf){
        BPLUS_DATA_NODE* Data_Node;
        Data_Node = data;
        //Case 1 the entry fits
        if(!is_full_data(Data_Node)){
            *block = INDEX_NODE->pointers[i];
            return 0;
        //Case 2 index node not full but data is
        }else if (!is_full_index(INDEX_NODE)){
            //int * ins_index, ins_key;
            //split_data(INDEX_NODE,Data_Node,block, &ins_index, &ins_key); 
            //sort pointers and keys arrays paralelly 
            return 0;
        }
        else{
            //int * ins_index, ins_key;
            //split_data(INDEX_NODE,Data_Node,block, &ins_index_to_new_block &ins_key_that_goes_up); 
            //spit_index(INDEX_NODE, &ins_index_to_new_block &ins_key_that_goes_up);
            
            return -1; //pointer to index node to 
        }
        
    }
    //If we still need to go down
    else if(!INDEX_NODE->leaf && !INDEX_NODE->root){
        BPLUS_INDEX_NODE* NEXT_INDEX;
        int ret; //Search return value
        int value1,value2;
        NEXT_INDEX = data;
        ret = search(NEXT_INDEX,BP_INFO,key,fd,block,&value1, &value2);
        if(ret == 0){return 0;}
        else{
            if (!is_full_index(INDEX_NODE)){
                //add and sort pointers and keys arrays paralelly
                return 0;
            }
            else{
                //spit_index(INDEX_NODE, &ins_index_to_new_block &ins_key_that_goes_up);
                return -1; //pointer to index node to 
            }
        } 
    }
    //we are at root
    else{
        BPLUS_INDEX_NODE* NEXT_INDEX;
        int ret; //Search return value
        int value1,value2;
        NEXT_INDEX = data;
        ret = search(NEXT_INDEX,BP_INFO,key,fd,block,&value1, &value2);
        if(ret == 0){return 0;}
        else{
            if (!is_full_index(INDEX_NODE)){
                //add and sort pointers and keys arrays paralelly
                return 0;
            }
            else{
                //split_index(INDEX_NODE, &ins_index_to_new_block &ins_key_that_goes_up);
                // BP_INFO->root=value1
                return 0; //pointer to index node to 
            }
            
        
        } 
    }
   
    return -1;
}

int split_index(BPLUS_INDEX_NODE *INDEX_NODE,int* block, int* ins_index, int* ins_key,int* fd){
   
    //use bf to get block data (is leaf/is root) and pass it on to the function
   
    BPLUS_INDEX_NODE* newdata=create_index_node(fd,);
    int key = *ins_key;
    int index = *ins_index;
    int mid=INDEX_NODE->counter_keys/2;
    *ins_key=INDEX_NODE->keys[mid];//return mid
    if (key>ins_key && key<INDEX_NODE->keys[mid+1])
    {
        *ins_key=key;
    }
    
    int i, j=0;
    for (i = mid+1; i < INDEX_NODE->counter_keys; i++)
    {
        newdata->keys[j]=INDEX_NODE->keys[i];
        newdata->pointers[j]=INDEX_NODE->pointers[i];

        // Data_Node.Records[i]=NULL;
        j++;
    }
    newdata->pointers[j]=INDEX_NODE->pointers[i];

    newdata->counter_keys=j;
    INDEX_NODE->counter_keys=mid + 1;
    int new_block_id;
    BF_GetBlockCounter(fd,&new_block_id);
    *ins_index = new_block_id - 1;


    for (i = mid-1;i > -1;i--){
        if (key > INDEX_NODE->keys[i]){
            break;}
        INDEX_NODE->keys[i+1] = INDEX_NODE->keys[i];
        INDEX_NODE->pointers[i+2] = INDEX_NODE->pointers[i+1];
    }
    
    INDEX_NODE->keys[i+1] = key;
    INDEX_NODE->pointers[i+2] = index;


    

    return 0;
}




