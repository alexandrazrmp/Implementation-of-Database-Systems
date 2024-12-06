#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "bp_file.h"
#include "record.h"
#include <bp_datanode.h>
#include <stdbool.h>

#define MAX_OPEN_FILES 20
#define bplus_ERROR -1

int last_block = -1;

#define CALL_BF(call)         \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK)        \
    {                         \
      BF_PrintError(code);    \
      return bplus_ERROR;     \
    }                         \
  }
#define CALL_OR_EXIT(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {         \
    BF_PrintError(code);    \
    exit(code);       \
  }                        \
}
int open_files = 0;
int files[MAX_OPEN_FILES];


int BP_CreateFile(char *fileName)
{
  BF_Block *block;
  int fd;
  void* data;
  BF_Block_Init(&block);

  //create and open file
  CALL_BF(BF_CreateFile(fileName));
  CALL_BF(BF_OpenFile(fileName,&fd));

  //allocate the first block and store metadata
  CALL_BF(BF_AllocateBlock(fd,block));
  data = BF_Block_GetData(block);
  BPLUS_INFO bpinfo;
  bpinfo.max_height= 0;
  bpinfo.root = -1;
  bpinfo.data_size = BF_BLOCK_SIZE/sizeof(Record);
  bpinfo.index_size = 4;

  

  memcpy(data, &bpinfo, sizeof(BPLUS_INFO));            //storing metadata
  BF_Block_SetDirty(block);                             //marking the block as dirty since it has been altered
  CALL_BF(BF_UnpinBlock(block));                        //unpinning the block in order to close the file
  CALL_BF(BF_CloseFile(fd));
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
  files[open_files] = *file_desc;
  open_files++;
  data = BF_Block_GetData(block);
  bpInfo = data;
  return bpInfo;
}

int BP_CloseFile(int file_desc,BPLUS_INFO* info)
{ 
  
  int blocks;
  CALL_BF(BF_GetBlockCounter(file_desc,&blocks));
  BF_Block *block;
  BF_Block_Init(&block);
  //unpinning all blocks in order to close the file
  for(int i=0;i<blocks;i++){
    CALL_BF(BF_GetBlock(file_desc,i,block));
    CALL_BF(BF_UnpinBlock(block));
  }
  CALL_BF(BF_CloseFile(file_desc));
  open_files--;
  //handle file array
  return 0;

}

int BP_InsertEntry(int file_desc,BPLUS_INFO *bplus_info, Record record)
{ 

  //if we are in the root
  if(bplus_info->root==-1){

    BPLUS_INDEX_NODE* BP_INDEX;
    BPLUS_DATA_NODE* BP_DATA;
    BP_INDEX= create_index_node(&file_desc,last_block,true,false);
    last_block++;
    bplus_info->root = last_block;
    BP_DATA=create_data_node(&file_desc);
    
    // create data block
    // BF_Block_SetDirty???
  }

  
  return 0;
}

int BP_GetEntry(int file_desc,BPLUS_INFO *bplus_info, int value,Record** record)
{  
  *record=NULL;
  return 0;
}