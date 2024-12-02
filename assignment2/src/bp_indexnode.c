#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "bp_file.h"
#include "record.h"
#include "bp_indexnode.h"


BPLUS_INDEX_NODE* create_index_node(int *fd,int last_block,int height){
    BF_Block *block;
    BPLUS_INDEX_NODE* BP_INFO;
    void* data;
    BF_Block_Init(&block);
    CALL_BF(BF_AllocateBlock(fd,block));
    data = BF_Block_GetData(block);
    BP_INFO->block_id = last_block + 1;
    BP_INFO->counter_keys = 0;
    BP_INFO->height=height;
    return BP_INFO;
}

bool is_full(BPLUS_INDEX_NODE* BP_INFO){

    if (BP_INFO->counter_keys==4){
        return true;
    }
    return false;
}

int search_split(BPLUS_INDEX_NODE* INDEX_NODE,BPLUS_INFO* BP_INFO ,int block_id,int key,int* fd,int* block){
    int i;

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
    }
    if(INDEX_NODE->height==BP_INFO->max_height){
        if(!is_full(INDEX_NODE)){
            *block = INDEX_NODE->pointers[i];
        }else{
            //split data
            //split index
        }
        
    }
    return 0;
}





int search_split2(BPLUS_INDEX_NODE* INDEX_NODE,BPLUS_INFO* BP_INFO ,int block_id,int key){
    int i;
    if(INDEX_NODE->height==BP_INFO->max_height + 1){    //change gia omorfia me height tou tree
        //if the node of data fits the value
            //return the index where it should be placed
        //else
            //
            //return search_split
    }






    return 0;
}



