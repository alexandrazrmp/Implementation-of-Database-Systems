#ifndef BP_INDEX_NODE_H
#define BP_INDEX_NODE_H

#define KEYS_NUM 31 //in the block can be inserted up to 61 keys , you can change it if you want 

#include <record.h>
#include <bf.h>
#include <bp_file.h>


typedef struct
{
    int block_id;//The id of the block
    bool root; // a variable that shows if we are in root
    bool leaf; //a variable that shows if we in the last row of the tree
    int counter_keys; //Number of keys in the node
    int keys[KEYS_NUM]; // Array to store keys 
    int pointers[KEYS_NUM+1]; // Array to store pointers

}BPLUS_INDEX_NODE;

//function that creates a new index node 
BPLUS_INDEX_NODE* create_index_node(int *fd,  bool is_root, bool is_leaf,BF_Block* block);

// Function to check if an index node is full
bool is_full_index(BPLUS_INDEX_NODE* BP_INDEX);

// Function that inserts keys and pointers if the index node is not full in sorted order
void Order_Keys(BPLUS_INDEX_NODE* INDEX_NODE,int value1,int value2);

// Recursive function to search where to insert a key spliting where is necessary 
int search(BPLUS_INDEX_NODE* INDEX_NODE,BPLUS_INFO* BP_INFO ,int key,int* fd,int* block, int *ins_index, int* ins_key,BF_Block* curblock);

//function that splits the index node and the root index node
int split_index(BPLUS_INDEX_NODE *INDEX_NODE,int* block, int* ins_index, int* ins_key,int* fd,BF_Block* curblock);
int split_root(BPLUS_INFO *BP_INFO, BPLUS_INDEX_NODE *INDEX_NODE, int *block, int *ins_index, int *ins_key, int *fd);

//printing function
void print_index(int *fd, int root_block_num);

#endif