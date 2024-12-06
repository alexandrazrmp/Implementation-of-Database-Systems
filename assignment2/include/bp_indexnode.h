#ifndef BP_INDEX_NODE_H
#define BP_INDEX_NODE_H
#include <record.h>
#include <bf.h>
#include <bp_file.h>

//mayro
typedef struct
{
    int block_id;
    bool root; // if we are in root
    bool leaf; // if we in the last row of the tree
    int counter_keys; //Number of keys in the node
    int keys[4]; // Array to store keys 
    int pointers[5]; // Array to store pointers

    // int keys[((BF_BLOCK_SIZE/sizeof(int))/2)-3];
    // int pointers[((BF_BLOCK_SIZE/sizeof(int))/2)-2];

}BPLUS_INDEX_NODE;


BPLUS_INDEX_NODE* create_index_node(int *fd,  bool is_root, bool is_leaf,BF_Block* block);

bool is_full_index(BPLUS_INDEX_NODE* BP_INDEX);

void Order_Keys(BPLUS_INDEX_NODE* INDEX_NODE,int value1,int value2);

int search(BPLUS_INDEX_NODE* INDEX_NODE,BPLUS_INFO* BP_INFO ,int key,int* fd,int* block, int *ins_index, int* ins_key);

int split_index(BPLUS_INDEX_NODE *INDEX_NODE,int* block, int* ins_index, int* ins_key,int* fd);

int split_root(BPLUS_INFO *BP_INFO, BPLUS_INDEX_NODE *INDEX_NODE, int *block, int *ins_index, int *ins_key, int *fd);
//void print_index(int fd, int root_block_num); 

void print_index(int *fd, int root_block_num);

#endif