#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bf.h"
#include "bp_file.h"
#include "bp_datanode.h"
#include "bp_indexnode.h"

#define INSERT_NUM 5000
#define FILE_NAME "data.db"


#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK)        \
    {                         \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }

void insertEntries();
void findEntries();

int main()
{

  insertEntries();
  findEntries();
  BF_Init(LRU);
  int fd;
  BPLUS_INFO* BP_INFO= BP_OpenFile(FILE_NAME,&fd);
  print_index(&fd,BP_INFO->root);
  BP_CloseFile(fd,BP_INFO);
  BF_Close();

  
}

void insertEntries(){
  BF_Init(LRU);
  BP_CreateFile(FILE_NAME);
  int file_desc;
  BPLUS_INFO* info = BP_OpenFile(FILE_NAME, &file_desc);
  Record record;
  for (int i = 0; i < INSERT_NUM; i++)
  {
    record = randomRecord();
    BP_InsertEntry(file_desc,info, record); 
  }
  BP_CloseFile(file_desc,info);
  BF_Close();
}

void findEntries(){
  int file_desc;
  BPLUS_INFO* info;

  BF_Init(LRU);
  info=BP_OpenFile(FILE_NAME, &file_desc);

  Record tmpRec;  //Αντί για malloc
  Record* result=&tmpRec;
  
  int id=500; 
  printf("Searching for: %d\n",id);
  if(BP_GetEntry( file_desc,info, id,&result)!=-1){
    printRecord(*result);
  }
  BP_CloseFile(file_desc,info);
  BF_Close();
}