#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "bp_file.h"
#include "record.h"
#include "bp_datanode.h"

BPLUS_DATA_NODE* create_data_node(int *fd){
    BF_Block *block;
    BPLUS_DATA_NODE* BP_INFO;
    void* data;
    BF_Block_Init(&block);
    CALL_BF(BF_AllocateBlock(fd,block));
    data = BF_Block_GetData(block);
    BP_INFO->record_counter = 0;
    BP_INFO->NextDataBlockNum = -1;
    return BP_INFO;
}