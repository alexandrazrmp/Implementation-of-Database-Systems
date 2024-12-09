#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "bp_file.h"
#include "record.h"
#include <bp_datanode.h>
#include <stdbool.h>

#define MAX_OPEN_FILES 20

int open_files = 0;
int files[MAX_OPEN_FILES];


int last_block_num=-1;


int BP_CreateFile(char *fileName)
{
  BF_Block *block;
  int fd;
  void* data;
  BF_Block_Init(&block);

  //create and open file
  CALL_OR_EXIT(BF_CreateFile(fileName));
  CALL_OR_EXIT(BF_OpenFile(fileName,&fd));

  //allocate the first block and store metadata
  CALL_OR_EXIT(BF_AllocateBlock(fd,block));
  data = BF_Block_GetData(block);
  BPLUS_INFO bpinfo;
  //bpinfo.max_height= 0;
  bpinfo.root = -1;
  bpinfo.data_size = 4;
  bpinfo.index_size = 4;

  last_block_num++;

  memcpy(data, &bpinfo, sizeof(BPLUS_INFO));            //storing metadata
  BF_Block_SetDirty(block);                             //marking the block as dirty since it has been altered
  CALL_OR_EXIT(BF_UnpinBlock(block));                        //unpinning the block in order to close the file
  CALL_OR_EXIT(BF_CloseFile(fd));
  return 0;
}


BPLUS_INFO* BP_OpenFile(char *fileName, int *file_desc)
{
  if (open_files == MAX_OPEN_FILES){
    printf("Couldn't open file, max open files exceeded\n");
    return NULL;
  }
  
  BPLUS_INFO* bpInfo;
  BF_Block *block;
  BF_Block_Init(&block);
  void* data;
  CALL_OR_EXIT(BF_OpenFile(fileName,file_desc));  //opening file
  CALL_OR_EXIT(BF_GetBlock(*file_desc,0,block));  //getting metadata of the first block
  //files[open_files] = *file_desc;
  //open_files++;
  data = BF_Block_GetData(block);
  bpInfo = data;
  CALL_OR_EXIT(BF_GetBlockCounter(*file_desc,&last_block_num));
  last_block_num--; //last block is getblockcounter -1
  return bpInfo;
}

int BP_CloseFile(int file_desc,BPLUS_INFO* info)
{ 
  
  int blocks;
  CALL_OR_EXIT(BF_GetBlockCounter(file_desc,&blocks));
  BF_Block *block;
  BF_Block_Init(&block);
  //unpinning all blocks in order to close the file
  for(int i=0;i<blocks;i++){
    CALL_OR_EXIT(BF_GetBlock(file_desc,i,block));
    CALL_OR_EXIT(BF_UnpinBlock(block));
  }
  CALL_OR_EXIT(BF_CloseFile(file_desc));
  open_files--;
  //handle file array
  return 0;

}

int BP_InsertEntry(int file_desc,BPLUS_INFO *bplus_info, Record record)
{ 
  BPLUS_INDEX_NODE* BP_INDEX;
  BPLUS_DATA_NODE* BP_DATA;
  int* ins_block;
  int* temp1;
  int* temp2;
  ins_block = malloc(sizeof(int));
  temp1 = malloc(sizeof(int));
  temp2 = malloc(sizeof(int));


  //if we have no root
  if(bplus_info->root==-1){
    BF_Block* block1;
    BF_Block* block2;
    BP_INDEX = create_index_node(&file_desc,1,1,block1);
    bplus_info->root = last_block_num;
    BP_DATA=create_root_data_node(&file_desc);
    BP_DATA->Records[0] = record;
    BP_INDEX->keys[0] = record.id;
    BP_INDEX->pointers[1] = last_block_num;
    //printf("Before %d\n", BP_DATA->record_counter);
    //BP_DATA->record_counter++;
    //printf("After %d\n", BP_DATA->record_counter);
    BP_INDEX->counter_keys = 1;
    //unpin data block
    //BF_Block_SetDirty(block1);
    //BF_Block_SetDirty(block2);
    //CALL_OR_EXIT(BF_UnpinBlock(block1));
    //CALL_OR_EXIT(BF_UnpinBlock(block2));
    return 0;
  }
  else{
    BF_Block* block1;
    BF_Block* block2;
    BF_Block_Init(&block1);
    BF_Block_Init(&block2);
    CALL_OR_EXIT(BF_GetBlock(file_desc,bplus_info->root,block1));
    void* data = BF_Block_GetData(block1);
    BP_INDEX = (BPLUS_INDEX_NODE*)data;

//TO DELETE

// printf("record.id: %d\n", record.id);  // Integer value
// printf("file_desc: %d\n", file_desc);  // Integer value

//END OF TO DELETE

    if(search(BP_INDEX,bplus_info,record.id,&file_desc,ins_block,temp1,temp2) != 0){
      //printf("PROBLEM\n");
      return -1;
    }
    printf("Ins_block = %d , last_block_counter = %d\n",*ins_block,last_block_num);
    CALL_OR_EXIT(BF_GetBlock(file_desc,*ins_block ,block2));
    data = BF_Block_GetData(block2);
    BP_DATA = (BPLUS_DATA_NODE*) data;
    //printf("After %d\n", BP_DATA->record_counter);
    int i;
    printf("New loop\n");
    for (i = BP_DATA->record_counter-1; i >= 0; i--)
    {
      printf("INDEX->records = %d\n",BP_DATA->record_counter);
      if (record.id < BP_DATA->Records[i].id)
      {
        // Shift keys and pointers to the right to make space
        BP_DATA->Records[i + 1] = BP_DATA->Records[i];
      }
      else
      {
        // Insert the new value at the correct position
        BP_DATA->Records[i + 1] = record;
        BP_DATA->record_counter++;
        // printf("in correct positiob\n");
        printf("Rec = %d,Rec = %d,Rec = %d,Rec = %d\n",BP_DATA->Records[0].id,BP_DATA->Records[1].id,BP_DATA->Records[2].id,BP_DATA->Records[3].id);
        break; // Exit the loop as the insertion is complete
      }
    }
    printf("end of loop!!!\n");
    // Handle the case where value2 is smaller than all elements
    if (i < 0)
    {

    //printf("THIS IS the case where value2 is smaller than all elements\n");
     
      BP_DATA->Records[0] = record;
      BP_DATA->record_counter++;
      
    }
    BF_Block_SetDirty(block1);
    BF_Block_SetDirty(block2);
    CALL_OR_EXIT(BF_UnpinBlock(block1));
    CALL_OR_EXIT(BF_UnpinBlock(block2));
    return 0;
  }
}

int BP_GetEntry(int file_desc,BPLUS_INFO *bplus_info, int value,Record** record)
{  

  if (bplus_info->root == -1) {
    // If the root is -1, the tree is empty.
    printf("B+ Tree is empty.\n");
    *record = NULL;
    return -1;
  }

  int current_block_num = bplus_info->root;
  BF_Block *block;
  BF_Block_Init(&block);
  BPLUS_INDEX_NODE *BP_INFO;
  
  while(true){

    CALL_OR_EXIT(BF_GetBlock(file_desc, current_block_num, block));
    void *data = BF_Block_GetData(block);
    BP_INFO = data;


    int i;
    for ( i = BP_INFO->counter_keys; i >= 0; i--)
    {
      if (BP_INFO->keys[i]>value)
      {
        break;
      }
      
    }
    if (i<0)
    {
      current_block_num=BP_INFO->pointers[0];
    }else{
      current_block_num=BP_INFO->pointers[i+1];
    }
    
    if(BP_INFO->leaf){
      break;
    }

  }

  BPLUS_DATA_NODE* DATA_NODE;
  CALL_OR_EXIT(BF_GetBlock(file_desc, current_block_num, block));
  void *data = BF_Block_GetData(block);
  DATA_NODE = data;


  for (int i = 0; i < DATA_NODE->record_counter; i++)
  {
    if (DATA_NODE->Records[i].id == value){
      
      **record = DATA_NODE->Records[i];
      return 0;
    } 
  }
  

  return -1;
}