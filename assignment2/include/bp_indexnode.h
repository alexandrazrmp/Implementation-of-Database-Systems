#ifndef BP_INDEX_NODE_H
#define BP_INDEX_NODE_H
#include <record.h>
#include <bf.h>
#include <bp_file.h>

//mayro
typedef struct
{
    int block_id;
    int height;
    int counter_keys;
    int keys[4];
    int pointers[5];
    // int keys[((BF_BLOCK_SIZE/sizeof(int))/2)-3];
    // int pointers[((BF_BLOCK_SIZE/sizeof(int))/2)-2];

}BPLUS_INDEX_NODE;



BPLUS_INDEX_NODE* create_index_node(int *fd,int last_block,int height);

bool is_full(BPLUS_INDEX_NODE* BP_INFO);

int search_split(BPLUS_INDEX_NODE* INDEX_NODE,BPLUS_INFO* BP_INFO ,int block_id,int key);

#endif