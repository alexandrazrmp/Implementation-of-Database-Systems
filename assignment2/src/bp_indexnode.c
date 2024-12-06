#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "bp_file.h"
#include "record.h"
#include "bp_indexnode.h"
#include "bp_datanode.h"

BPLUS_INDEX_NODE* create_index_node(int *fd,  bool is_root, bool is_leaf) {
    BF_Block *block;
    BPLUS_INDEX_NODE* BP_INFO;
    void* data;
    BF_Block_Init(&block);
    CALL_OR_EXIT(BF_AllocateBlock(*fd, block));
    data = BF_Block_GetData(block);

    //initialize the index node structure
    BP_INFO->block_id = last_block_num + 1; 
    last_block_num++;
    BP_INFO->counter_keys = 0;         
    BP_INFO->root = is_root;          
    BP_INFO->leaf = is_leaf;        


    memset(BP_INFO->keys, -1, sizeof(BP_INFO->keys));
    memset(BP_INFO->pointers, -1, sizeof(BP_INFO->pointers));
    BF_Block_SetDirty(block);//maybe dirty

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
    CALL_BF(BF_GetBlock(*fd,i,dataBlock));
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
            // int * ins_index, ins_key;
            // split_data(INDEX_NODE,Data_Node,block, &ins_index, &ins_key);
            //add ins_index and ins_key to index node
            return 0;
        }
        else{
            //int * ins_index, ins_key;
            //split_data(INDEX_NODE,Data_Node,block, &ins_index_to_new_block &ins_key_that_goes_up); 
            //split_index(INDEX_NODE, &ins_index_to_new_block &ins_key_that_goes_up);
            
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
                //split_index(INDEX_NODE, &ins_index_to_new_block &ins_key_that_goes_up);
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
                //make new root using split root
                //split_index(INDEX_NODE, &ins_index_to_new_block &ins_key_that_goes_up);
                // BP_INFO->root=value1
                return 0; //pointer to index node to 
            }
            
        
        } 
    }
   
    return -1;
}

int split_index(BPLUS_INDEX_NODE *INDEX_NODE,int* block, int* ins_index, int* ins_key,int* fd){
   
   
    BPLUS_INDEX_NODE* newindex=create_index_node(fd,INDEX_NODE->root,INDEX_NODE->leaf);
    int key = *ins_key;
    int index = *ins_index;

    int temp_keys[INDEX_NODE->counter_keys+1];     // Array to hold the result of the keys after the insertion
    int temp_pointers[INDEX_NODE->counter_keys+2];     // Array to hold the result of the pointers after the insertion
    int i ; // Iterator for the key array
    int j = 0; // Iterator for the temp_key array

    // Insert elements into temp maintaining the sorted order
    temp_pointers[0]=INDEX_NODE->keys[0];
    for (i = 0; i < INDEX_NODE->counter_keys; i++){
        if (key < INDEX_NODE->keys[i] && j == i) {
            temp_keys[j] = key; // Insert the new element in sorted order
            temp_pointers[j]= index;
            j++;
        } 
        temp_keys[j] = INDEX_NODE->keys[i]; 
        temp_pointers[j+1]=INDEX_NODE->pointers[i+1];
        j++;
    }

    // If the new element is the largest, insert it at the end
    if (j < INDEX_NODE->counter_keys+1) {
        temp_keys[j] = key;
        temp_pointers[j+1]=index;
    }


    memset(INDEX_NODE->keys, -1, sizeof(INDEX_NODE->keys));
    memset(INDEX_NODE->pointers, -1, sizeof(INDEX_NODE->pointers));
    INDEX_NODE->counter_keys = 0;

    INDEX_NODE->pointers[0]=temp_pointers[0];
    int mid=(INDEX_NODE->counter_keys+1)/2;
    *ins_key = temp_keys[mid];//return mid
    
    for ( i = 0; i < mid; i++)
    {
        INDEX_NODE->keys[i]=temp_keys[i];
        INDEX_NODE->pointers[i+1]=temp_pointers[i+1];
        INDEX_NODE->counter_keys++;
    }
    
    j = 0;
    for ( i = mid+1; i < INDEX_NODE->counter_keys; i++)
    {
        newindex->keys[j]=temp_keys[i];
        newindex->pointers[j]=temp_pointers[i];
        newindex->counter_keys++;
        j++;
    }
    newindex->pointers[j]=temp_pointers[i];

    CALL_BF(BF_GetBlockCounter(*fd,ins_index));
    

    return 0;
}

int split_root(BPLUS_INDEX_NODE *INDEX_NODE,int* block, int* ins_index, int* ins_key,int* fd){
    
    BPLUS_INDEX_NODE* newindex=create_index_node(fd,false,INDEX_NODE->leaf);
    int key = *ins_key;
    int index = *ins_index;

    int temp_keys[INDEX_NODE->counter_keys+1];     // Array to hold the result of the keys after the insertion
    int temp_pointers[INDEX_NODE->counter_keys+2];     // Array to hold the result of the pointers after the insertion
    int i ; // Iterator for the key array
    int j = 0; // Iterator for the temp_key array

    // Insert elements into temp maintaining the sorted order
    temp_pointers[0]=INDEX_NODE->keys[0];
    for (i = 0; i < INDEX_NODE->counter_keys; i++){
        if (key < INDEX_NODE->keys[i] && j == i) {
            temp_keys[j] = key; // Insert the new element in sorted order
            temp_pointers[j]= index;
            j++;
        } 
        temp_keys[j] = INDEX_NODE->keys[i]; 
        temp_pointers[j+1]=INDEX_NODE->pointers[i+1];
        j++;
    }

    // If the new element is the largest, insert it at the end
    if (j < INDEX_NODE->counter_keys+1) {
        temp_keys[j] = key;
        temp_pointers[j+1]=index;
    }


    memset(INDEX_NODE->keys, -1, sizeof(INDEX_NODE->keys));
    memset(INDEX_NODE->pointers, -1, sizeof(INDEX_NODE->pointers));
    INDEX_NODE->counter_keys = 0;
    INDEX_NODE->pointers[0]=temp_pointers[0];
    int mid=(INDEX_NODE->counter_keys+1)/2;
    key = temp_keys[mid];//return mid
    
    for ( i = 0; i < mid; i++)
    {
        INDEX_NODE->keys[i]=temp_keys[i];
        INDEX_NODE->pointers[i+1]=temp_pointers[i+1];
        INDEX_NODE->counter_keys++;
    }
    
    j = 0;
    for ( i = mid+1; i < INDEX_NODE->counter_keys; i++)
    {
        newindex->keys[j]=temp_keys[i];
        newindex->pointers[j]=temp_pointers[i];
        newindex->counter_keys++;
        j++;
    }
    newindex->pointers[j]=temp_pointers[i];

    CALL_BF(BF_GetBlockCounter(*fd,&index)); 

    
    BPLUS_INDEX_NODE* newROOT=create_index_node(fd,true,false);
    INDEX_NODE->root = 0; 
    newROOT->keys[0] = key;
    newROOT->counter_keys = 1;
    newROOT->pointers[0] = INDEX_NODE->block_id;
    newROOT->pointers[1] = index;

    return 0;
}





void print_index(int fd, int root_block_num){

}

