#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hp_file.h"
#include "record.h"

//handling errors
#define HP_ERROR -1

#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {         \
    BF_PrintError(code);    \
    return HP_ERROR;        \
  }                        \
}
#define CALL_OR_EXIT(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {         \
    BF_PrintError(code);    \
    exit(code);       \
  }                        \
}
int HP_CreateFile(char *fileName){
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
  HP_info File_info;
  File_info.Last_id = 0;
  File_info.Block_size = BF_BLOCK_SIZE/sizeof(Record);  //get the size of each block
  File_info.Last_Block_Capacity = 0;                    //first block capacity, since we just created the file
  File_info.dirty = 0;
  File_info.Last_Block_entries = 0;
  memcpy(data, &File_info, sizeof(HP_info));            //storing metadata
  BF_Block_SetDirty(block);                             //marking the block as dirty since it has been altered
  CALL_BF(BF_UnpinBlock(block));                        //unpinning the block in order to close the file
  CALL_BF(BF_CloseFile(fd));
  return 0;
}

HP_info* HP_OpenFile(char *fileName, int *file_desc){
  HP_info* hpInfo;
  BF_Block *block;
  BF_Block_Init(&block);
  void* data;
  CALL_OR_EXIT(BF_OpenFile(fileName,file_desc));  //opening file
  CALL_OR_EXIT(BF_GetBlock(*file_desc,0,block));  //getting metadata of the first block
  data = BF_Block_GetData(block);
  hpInfo = data;
  return hpInfo;
}


int HP_CloseFile(int file_desc,HP_info* hp_info ){
  int blocks;
  CALL_BF(BF_GetBlockCounter(file_desc,&blocks));
  BF_Block *block;
  BF_Block_Init(&block);
  hp_info->dirty = 0;
  //unpinning all blocks in order to close the file
  for(int i=0;i<blocks;i++){
    CALL_BF(BF_GetBlock(file_desc,i,block));
    CALL_BF(BF_UnpinBlock(block));
  }
  CALL_BF(BF_CloseFile(file_desc));
  return 0;
}

int HP_InsertEntry(int file_desc,HP_info* hp_info, Record record){
  BF_Block *block;
  int Blocknum;
  BF_Block_Init(&block);
  void* data;
  if(hp_info->Last_Block_Capacity==0){          //if there is no room for more Records in the last block (it already has 8)
    CALL_BF(BF_AllocateBlock(file_desc,block)); //allocate a new block
    hp_info->Last_Block_Capacity=hp_info->Block_size; //update last block capacity
    CALL_BF(BF_GetBlockCounter(file_desc,&Blocknum));
    hp_info->Last_id = Blocknum - 1;            //updating the last block id (block enum)
    hp_info->Last_Block_entries = 0;            //the block we just allocated currently has no records
  }
  else {CALL_BF(BF_GetBlock(file_desc,hp_info->Last_id,block));}  //if there is room for an extra record in the block, get it
  data=BF_Block_GetData(block);
  Record* r = data;
  int entries = hp_info->Last_Block_entries;
  r+=entries;
  memcpy(r,&record,sizeof(Record));          //copy the record to the block
  BF_Block_SetDirty(block);                  //mark it as dirty since it has been altered
  hp_info->Last_Block_Capacity--;            //update
  hp_info->Last_Block_entries++;
  CALL_BF(BF_UnpinBlock(block));             //We make the first block dirty if it isn't already here
  if(hp_info->dirty == 0){
    CALL_BF(BF_GetBlock(file_desc,0,block));
    BF_Block_SetDirty(block);
    hp_info->dirty = 1;
  }
  return 0;
}

int HP_GetAllEntries(int file_desc,HP_info* header_info, int id){
  int Read = 0;
  int blocks;
  void* data;
  int flag = 0;
  Record* record;
  CALL_BF(BF_GetBlockCounter(file_desc,&blocks))
  BF_Block *block;
  BF_Block_Init(&block);
  for(int i = 1;i<blocks - 1;i++){                  //for all blocks in hard drive
    CALL_BF(BF_GetBlock(file_desc,i,block));        //get the block
    data = BF_Block_GetData(block);
    record = data;
    for(int j=0;j< header_info->Block_size; j++){   //go through each record in each block except the last one
      if(record[j].id == id){                       //print it if it matches the id given in main
        printf("Found Record with id %d\n",id);
        printRecord(record[j]);
      }
    }
    CALL_BF(BF_UnpinBlock(block));
    Read++;  
  }
  CALL_BF(BF_GetBlock(file_desc,blocks - 1,block));
  data = BF_Block_GetData(block);
  for(int i=0 ; i< header_info->Block_size - header_info->Last_Block_Capacity ; i++){    //go through each record in last block
    flag = 1;
    if(record[i].id == id){                                                              //print it if it matches the id given in main
      printf("Found Record with id %d\n",id);                                 
      printRecord(record[i]);
    }
  }
  if (flag == 1){       //if we entered the last loop, meaning if there was another block that had less than 8 or 8 records
      Read++;
    }
  CALL_BF(BF_UnpinBlock(block));
  return Read;
}