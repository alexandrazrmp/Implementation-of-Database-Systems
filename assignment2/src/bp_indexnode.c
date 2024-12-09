#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "bp_file.h"
#include "record.h"
#include "bp_indexnode.h"
#include "bp_datanode.h"

BPLUS_INDEX_NODE *create_index_node(int *fd, bool is_root, bool is_leaf, BF_Block *block)
{
    BPLUS_INDEX_NODE *BP_INFO;
    void *data;

    //initialize a block to hold the index node
    BF_Block_Init(&block);
    CALL_OR_EXIT(BF_AllocateBlock(*fd, block));  // Allocate a new block in the file
    data = BF_Block_GetData(block);

    //initialize the index node structure (number of keys , if it is root or leaf)
    BP_INFO->block_id = last_block_num + 1;  // Assign a new block ID with the help of the global variable
    last_block_num++;
    BP_INFO->counter_keys = 0;  
    BP_INFO->root = is_root;    
    BP_INFO->leaf = is_leaf;    

    // Initialize keys and pointers to -1 (empty state)
    memset(BP_INFO->keys, -1, sizeof(BP_INFO->keys));
    memset(BP_INFO->pointers, -1, sizeof(BP_INFO->pointers));

    // Mark the block as modified
    BF_Block_SetDirty(block);

    return BP_INFO;  
}

bool is_full_index(BPLUS_INDEX_NODE *BP_INFO)
{
    // A node is full if the number of keys are .....................
    if (BP_INFO->counter_keys == 4)
    {
        return true;
    }
    return false;
}

void Order_Keys(BPLUS_INDEX_NODE* INDEX_NODE,int value1,int value2) {
    int i;
    for (i = INDEX_NODE->counter_keys; i >= 0; i--)
    {
        if (value2 < INDEX_NODE->keys[i])
        {
            // Shift keys and pointers to the right to make space
            INDEX_NODE->keys[i + 1] = INDEX_NODE->keys[i];
            INDEX_NODE->pointers[i + 2] = INDEX_NODE->pointers[i+1];
        }
        else
        {
            // Insert the new value at the correct position
            INDEX_NODE->keys[i + 1] = value2;
            INDEX_NODE->pointers[i + 2] = value1;
            break; // Exit the loop as the insertion is complete
        }
    }
    // Handle the case where value2 is smaller than all elements
    if (i < 0)
    {
        INDEX_NODE->keys[0] = value2;
        INDEX_NODE->pointers[1] = value1;
    }
}

int search(BPLUS_INDEX_NODE *INDEX_NODE, BPLUS_INFO *BP_INFO, int key, int *fd, int *block, int *ins_index, int *ins_key)
{
    int i;
    BF_Block *dataBlock;
    void *data;
    BF_Block_Init(&dataBlock);

//if seg fault look first!!!!////////////////////////////////////////////////////////////
    // find the  pointer(i) to the block we must search
    if (INDEX_NODE->keys[(INDEX_NODE->counter_keys) - 1] < key) {
        i = (INDEX_NODE->counter_keys) - 1;  
    } else {
        for (i = 0; i < INDEX_NODE->counter_keys; i++) {
            if (INDEX_NODE->keys[i] > key) {
                break;  
            }
        }
    }

    //Handle the case where there is no data block at the pointer
    if (INDEX_NODE->pointers[i] == -1) {
        BPLUS_DATA_NODE *Data_Node = create_data_node(fd);
        *block = last_block_num;  // Return the block ID of the new data node
        return 0;  
    }

    // get the data block for the pointer
    CALL_OR_EXIT(BF_GetBlock(*fd, INDEX_NODE->pointers[i], dataBlock));
    data = BF_Block_GetData(dataBlock);

    //Handle the case where the node is both a root and a leaf
    //this happens when we insert the first indexes
    if (INDEX_NODE->leaf && INDEX_NODE->root) {
        BPLUS_DATA_NODE *Data_Node = (BPLUS_DATA_NODE *)data;

        // Case 1: The key can be inserted directly (the data node is not full)
        if (!is_full_data(Data_Node)) {
            *block = INDEX_NODE->pointers[i];
            BF_Block_SetDirty(dataBlock);
            CALL_OR_EXIT(BF_UnpinBlock(dataBlock));
            return 0;
        }
        // Case 2: The data node is full, but the index node isn't full
        else if (!is_full_index(INDEX_NODE)) {
            split_data(INDEX_NODE, Data_Node, block, ins_index, ins_key, key, fd);
            Order_Keys(INDEX_NODE, *ins_index, *ins_key);
            BF_Block_SetDirty(dataBlock);
            CALL_OR_EXIT(BF_UnpinBlock(dataBlock));
            return 0;
        }
        // Case 3: Both the data and index nodes are full
        //in this case we need to split the root 
        else {
            split_data(INDEX_NODE, Data_Node, block, ins_index, ins_key, key, fd);
            split_root(BP_INFO, INDEX_NODE, block, ins_index, ins_key, fd);
            BF_Block_SetDirty(dataBlock);
            CALL_OR_EXIT(BF_UnpinBlock(dataBlock));
            return 0;
        }
    }
    // If we have reached final level of b-tree
    else if (INDEX_NODE->leaf)
    {
        BPLUS_DATA_NODE *Data_Node;
        Data_Node = data;
        // Case 1: The key can be inserted directly (the data node is not full)
        if (!is_full_data(Data_Node))
        {
            *block = INDEX_NODE->pointers[i];
            BF_Block_SetDirty(dataBlock);
            CALL_OR_EXIT(BF_UnpinBlock(dataBlock));
            return 0;
        }
        // Case 2: The data node is full, but the index node isn't full
        else if (!is_full_index(INDEX_NODE))
        {
            split_data(INDEX_NODE, Data_Node, block, ins_index, ins_key, key, fd);
            Order_Keys(INDEX_NODE,*ins_index,*ins_key);
            BF_Block_SetDirty(dataBlock);
            CALL_OR_EXIT(BF_UnpinBlock(dataBlock));
            return 0;
        }
        // Case 3: Both the data and index nodes are full
        //in this case we need to split bith data and index node
        else
        {
            split_data(INDEX_NODE, Data_Node, block, ins_index, ins_key, key, fd);
            split_index(INDEX_NODE, block, ins_index, ins_key, fd);
            BF_Block_SetDirty(dataBlock);
            CALL_OR_EXIT(BF_UnpinBlock(dataBlock));
            return -1;
        }
    }
    // If we still need to go down (node is neither a leaf nor the root)
    else if (!INDEX_NODE->leaf && !INDEX_NODE->root)
    {
        BPLUS_INDEX_NODE *NEXT_INDEX; 
        int ret; //the return value from the search function
        int value1, value2; //Temporary variables to hold the new key and pointer

        // Get the next index node from the current block's data
        NEXT_INDEX = data; 

        // Recursively call the search function on the next index node
        ret = search(NEXT_INDEX, BP_INFO, key, fd, block, &value1, &value2);
        
    
        if (ret == 0)
        {
            return 0;
        }
        else
        {
            //check if the current index node has space
            if (!is_full_index(INDEX_NODE)){

                // Insert the new key and pointer into the root node
                Order_Keys(INDEX_NODE,value1,value2);

                //set dirty and unpin the block
                BF_Block_SetDirty(dataBlock);
                CALL_OR_EXIT(BF_UnpinBlock(dataBlock));
                return 0;
            }
            // If the current index node is full
            else{
                // Split the node
                split_index(INDEX_NODE, block, &value1, &value2, fd); 

                // Update the insertion key and pointer for the parent node
                *ins_index = value1;
                *ins_key = value2;

                //set dirty and unpin the block     
                BF_Block_SetDirty(dataBlock); 
                CALL_OR_EXIT(BF_UnpinBlock(dataBlock)); 
                return -1; // Indicate that we need to repeat the process
            }
        }
    }
    // If we are at the root
    else
    {
        BPLUS_INDEX_NODE *NEXT_INDEX; 
        int ret; //the return value from the search function
        int value1, value2; //Temporary variables to hold the new key and pointer

        // Get the next index node from the current block's data
        NEXT_INDEX = data; 

        // Recursively call the search function on the next index node
        ret = search(NEXT_INDEX, BP_INFO, key, fd, block, &value1, &value2);
        
        if (ret == 0)
        {
            return 0;
        }
        else
        {
            // Check if the root node has space
            if (!is_full_index(INDEX_NODE)){

                // Insert the new key and pointer into the root node
                Order_Keys(INDEX_NODE,value1,value2);

                //set dirty and unpin the block
                BF_Block_SetDirty(dataBlock);
                CALL_OR_EXIT(BF_UnpinBlock(dataBlock));
                return 0;
            }
            // If the root node is full, it needs to be split
            else
            {
                //Split the root 
                split_root(BP_INFO, INDEX_NODE, block, ins_index, ins_key, fd);
                
                //set dirty and unpin the block
                BF_Block_SetDirty(dataBlock);
                CALL_OR_EXIT(BF_UnpinBlock(dataBlock));
                return 0; // pointer to index node to
            }
        }
    }

    return -1;
}

int split_index(BPLUS_INDEX_NODE *INDEX_NODE, int *block, int *ins_index, int *ins_key, int *fd)
{

    BF_Block *block1;
    BPLUS_INDEX_NODE *newindex = create_index_node(fd, INDEX_NODE->root, INDEX_NODE->leaf, block1);
    
    // Store the key and index to be inserted
    int key = *ins_key;
    int index = *ins_index;

    // Temporary arrays to hold the sorted keys and pointers after insertion
    int temp_keys[INDEX_NODE->counter_keys + 1];    // Array to hold the result of the keys after the insertion
    int temp_pointers[INDEX_NODE->counter_keys + 2]; // Array to hold the result of the pointers after the insertion
    int i;    // Iterator for the original keys array
    int j = 0;       // Iterator for the temporary keys array

    // Insert all elements into the temporary arrays in sorted order
    temp_pointers[0] = INDEX_NODE->keys[0]; // Initialize the first pointer
    for (i = 0; i < INDEX_NODE->counter_keys; i++)
    {
        // Insert the new key and pointer in the correct position
        if (key < INDEX_NODE->keys[i] && j == i)
        {
            temp_keys[j] = key; 
            temp_pointers[j] = index;
            j++;
        }
        // Copy the existing keys and pointers into the temporary arrays
        temp_keys[j] = INDEX_NODE->keys[i];
        temp_pointers[j + 1] = INDEX_NODE->pointers[i + 1];
        j++;
    }

    // If the new key is larger than all existing keys, insert it at the end
    if (j < INDEX_NODE->counter_keys + 1)
    {
        temp_keys[j] = key; 
        temp_pointers[j + 1] = index;
    }

    //Set the keys and pointers of the original node with the value -1
    memset(INDEX_NODE->keys, -1, sizeof(INDEX_NODE->keys));
    memset(INDEX_NODE->pointers, -1, sizeof(INDEX_NODE->pointers));
    INDEX_NODE->counter_keys = 0; // Reset the key counter for the current node

    //Assign the first pointer to the index node because it will never change 
    INDEX_NODE->pointers[0] = temp_pointers[0];

    // Find the mid-point for splitting the keys
    int mid = (INDEX_NODE->counter_keys + 1) / 2;
    *ins_key = temp_keys[mid]; // Return the middle key to the parent

    // Fill the left half of the keys and pointers in the original node
    for (i = 0; i < mid; i++)
    {
        INDEX_NODE->keys[i] = temp_keys[i];
        INDEX_NODE->pointers[i + 1] = temp_pointers[i + 1]; 
        INDEX_NODE->counter_keys++;
    }

    // Fill the right half of the keys and pointers into the new index node
    j = 0;
    for (i = mid + 1; i < INDEX_NODE->counter_keys; i++)
    {
        newindex->keys[j] = temp_keys[i];
        newindex->pointers[j] = temp_pointers[i]; 
        newindex->counter_keys++; 
        j++;
    }

    // Assign the final pointer to the new node
    newindex->pointers[j] = temp_pointers[i];

    // Update the block counter and unpin the block for the new node
    CALL_OR_EXIT(BF_GetBlockCounter(*fd, ins_index)); 
    *ins_index--;
    CALL_OR_EXIT(BF_UnpinBlock(block1)); 

    return 0; 
}

int split_root(BPLUS_INFO *BP_INFO, BPLUS_INDEX_NODE *INDEX_NODE, int *block, int *ins_index, int *ins_key, int *fd)
{
    BF_Block *block_root;
    BF_Block *block_index;
    BPLUS_INDEX_NODE *newindex = create_index_node(fd, false, INDEX_NODE->leaf, block_index);
    
    //store the value
    int key = *ins_key;
    int index = *ins_index;

    int temp_keys[INDEX_NODE->counter_keys + 1];     // Array to hold the result of the keys after the insertion
    int temp_pointers[INDEX_NODE->counter_keys + 2]; // Array to hold the result of the pointers after the insertion
    int i;    // Iterator for the key array
    int j = 0;  // Iterator for the temp_key array

    // Insert all elements into the temporary arrays in sorted order
    temp_pointers[0] = INDEX_NODE->keys[0]; // Initialize the first pointer
    for (i = 0; i < INDEX_NODE->counter_keys; i++)
    {
        // Insert the new key and pointer in the correct position
        if (key < INDEX_NODE->keys[i] && j == i)
        {
            temp_keys[j] = key; 
            temp_pointers[j] = index;
            j++;
        }
        // Copy the existing keys and pointers into the temporary arrays
        temp_keys[j] = INDEX_NODE->keys[i];
        temp_pointers[j + 1] = INDEX_NODE->pointers[i + 1];
        j++;
    }

    // If the new element is the largest, insert it at the end
    if (j < INDEX_NODE->counter_keys + 1)
    {
        temp_keys[j] = key;
        temp_pointers[j + 1] = index;
    }

    //Set the old node with the value -1 
    memset(INDEX_NODE->keys, -1, sizeof(INDEX_NODE->keys));
    memset(INDEX_NODE->pointers, -1, sizeof(INDEX_NODE->pointers));
    INDEX_NODE->counter_keys = 0;

    //assign the first pointer to the index node because it will never change 
    INDEX_NODE->pointers[0] = temp_pointers[0];

    //Find the mid-point for splitting the keys 
    int mid = (INDEX_NODE->counter_keys + 1) / 2;
    key = temp_keys[mid]; 

    for (i = 0; i < mid; i++)
    {
        INDEX_NODE->keys[i] = temp_keys[i];
        INDEX_NODE->pointers[i + 1] = temp_pointers[i + 1];
        INDEX_NODE->counter_keys++;
    }

    j = 0;
    for (i = mid + 1; i < INDEX_NODE->counter_keys; i++)
    {
        newindex->keys[j] = temp_keys[i];
        newindex->pointers[j] = temp_pointers[i];
        newindex->counter_keys++;
        j++;
    }
    newindex->pointers[j] = temp_pointers[i];

    CALL_OR_EXIT(BF_GetBlockCounter(*fd, &index));

    BPLUS_INDEX_NODE *newROOT = create_index_node(fd, true, false, block_root);
    INDEX_NODE->root = 0;
    newROOT->keys[0] = key;
    newROOT->counter_keys = 1;
    newROOT->pointers[0] = INDEX_NODE->block_id;
    newROOT->pointers[1] = index - 1;
    BP_INFO->root = index;
    CALL_OR_EXIT(BF_UnpinBlock(block_index));
    CALL_OR_EXIT(BF_UnpinBlock(block_root));

    return 0;
}

// Recursive helper function to print a single level of the B+ tree
void print_level(int *fd, int *blocks, int num_blocks, int level, int max_width) {
    BF_Block *block;
    BF_Block_Init(&block);

    int spacing = max_width / (num_blocks + 1);  // Calculate spacing between nodes
    printf("\nLevel %d:\n", level);

    for (int i = 0; i < num_blocks; i++) {
        int block_id = blocks[i];

        // Print spacing before the block
        for (int s = 0; s < spacing; s++) {
            printf(" ");
        }

        // Get the block data
        CALL_OR_EXIT(BF_GetBlock(*fd, block_id, block));
        void *data = BF_Block_GetData(block);
        BPLUS_INDEX_NODE *index_node = (BPLUS_INDEX_NODE *)data;

        // Print the block keys
        printf("[");
        for (int j = 0; j < index_node->counter_keys; j++) {
            printf("%d", index_node->keys[j]);
            if (j < index_node->counter_keys - 1) {
                printf(" | ");
            }
        }
        printf("]");

        // Unpin the block
        BF_UnpinBlock(block);
    }
    printf("\n");

}

// Function to recursively traverse the tree and collect child blocks for the next level
int collect_children(int *fd, int *parent_blocks, int num_parent_blocks, int *child_blocks) {
    BF_Block *block;
    BF_Block_Init(&block);

    int child_index = 0;
    for (int i = 0; i < num_parent_blocks; i++) {
        int block_id = parent_blocks[i];

        CALL_OR_EXIT(BF_GetBlock(*fd, block_id, block));
        void *data = BF_Block_GetData(block);
        BPLUS_INDEX_NODE *index_node = (BPLUS_INDEX_NODE *)data;

        // If not a leaf, collect child pointers
        if (!index_node->leaf) {
            for (int j = 0; j <= index_node->counter_keys; j++) {
                if (index_node->pointers[j] != -1) {
                    child_blocks[child_index++] = index_node->pointers[j];
                }
            }
        }

        BF_UnpinBlock(block);
    }
    return child_index;  // Return the number of children collected
}

// Main function to print the B+ tree as a tree structure
void print_index(int *fd, int root_block_num) {
    if (root_block_num < 0) {
        printf("Invalid root block number.\n");
        return;
    }

    int parent_blocks[1000];  // Array to store blocks at the current level
    int child_blocks[1000];   // Array to store blocks at the next level
    int num_parent_blocks = 1;  // Start with the root block
    parent_blocks[0] = root_block_num;

    int level = 0;
    int max_width = 80;  // Adjust this for a wider/narrower tree display

    printf("B+ Tree Structure:\n");
    while (num_parent_blocks > 0) {
        // Print the current level
        print_level(fd, parent_blocks, num_parent_blocks, level, max_width);

        // Collect child blocks for the next level
        int num_child_blocks = collect_children(fd, parent_blocks, num_parent_blocks, child_blocks);

        // Move to the next level
        num_parent_blocks = num_child_blocks;
        for (int i = 0; i < num_child_blocks; i++) {
            parent_blocks[i] = child_blocks[i];
        }

        level++;
    }
}
