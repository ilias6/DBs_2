#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "HT_functs.h"
#include "../Record/Record.h"
#include "../BF.h"
#include "../hash_functions/hashfuncts.h"

#define MAX_FILES 100
extern int num_of_open_files;
                             /*

                                      WE SUPPOSE THAT BUCKET SIZE == BLOCK_SIZE


                                                                                      */
// GLOBAL POINTER TO HASH FUNCT --> IF WE WANT TO CHANGE HASHFUNCT WE CHANGE THIS POINTER

unsigned long int (*hash_funct)(void * value,char type)=djb2;

void HT_Init(){
  BF_Init();
  return ;
}

int HT_CreateIndex( char *fileName,char attrType,char* attrName, int attrLength,int buckets){
  int err_no;
  File_type t=HASH_FILE;
  if(BF_CreateFile(fileName)<0){
    BF_PrintError("Error creating file");
    return -1;
  }
  int fd=BF_OpenFile(fileName);
  if(fd<0){
    BF_PrintError("Error opening file");
    return -1;
  }
  do{
    // if fail -> try again
  }while(BF_AllocateBlock(fd)<0);

  uint8_t * block;

  if(BF_ReadBlock(fd,0,(void**)&block)<0){
    BF_PrintError("Error reading block 0 from file");
    return -1;
  }
  memcpy(block,&t,sizeof(File_type));
  memcpy(block+sizeof(File_type),(uint8_t *)&attrType,sizeof(char));
  memcpy(block+sizeof(File_type)+sizeof(char),(uint8_t *)&attrLength,sizeof(int));
  memcpy(block+sizeof(File_type)+sizeof(char)+sizeof(int),(uint8_t *)attrName,attrLength);
  memcpy(block+sizeof(File_type)+sizeof(char)+sizeof(int)+attrLength,&buckets,sizeof(int));

  if(BF_WriteBlock(fd,0)<0){
    BF_PrintError("Error writing block 0 to file");
    return -1;
  }
  // allocate bucket blocks under superblock and write 0 as first int indicating current records in block
  int records_at_start=0;
  int next_bucket=0; // next bucket is written at the end of bucket indicating which block is next
  // each bucket conatins one int at start indicating how many records are in bucket
  // and one int in end indicating which block is next when bucket is full (like linked list)
  // we conclude that each bucket has BLOCK_SIZE - 2*sizeof(int) bytes available for data
  // AS IN HEAP FILE WE MAY HAVE FRAGMENTATION
  for(int i=1;i<=buckets;i++){
    do{
      // if fail -> try again
    }while(BF_AllocateBlock(fd)<0);
    if(BF_ReadBlock(fd,i,(void**)&block)<0){
      BF_PrintError("Error reading block 0 from file");
      return -1;
    }
    memcpy(block,&records_at_start,sizeof(int));
    memcpy(block + BLOCK_SIZE-sizeof(int),&next_bucket,sizeof(int));
    if(BF_WriteBlock(fd,i)<0){
      BF_PrintError("Error writing block to file");
      return -1;
    }
  }
  if(BF_CloseFile(fd)<0){
    BF_PrintError("Error closing file");
    return -1;
  }
  return 0;
}

HT_info * HT_OpenIndex(char *fileName) {
  // open up to MAX_FILES
  if(num_of_open_files > MAX_FILES)
    return NULL;

  HT_info * info=malloc(sizeof(HT_info));
  if(info==NULL){
    printf("Malloc error line 88!\n");
    return NULL;
  }
  info->fileDesc=BF_OpenFile(fileName);
  if(info->fileDesc<0){
    free(info);
    BF_PrintError("Error opening file");
    return NULL;
  }
  uint8_t * block;
  do{
  }while(BF_ReadBlock(info->fileDesc,0,(void **)&block)<0);

  memcpy(&info->t,block,sizeof(File_type));
  if(info->t!=HASH_FILE){
    BF_CloseFile(info->fileDesc);
    free(info);
    return NULL;
  }
  memcpy(&info->attrType,block+sizeof(File_type),sizeof(uint8_t));
  memcpy(&info->attrLength,block+sizeof(File_type)+1,sizeof(int));

  do{
  }while((info->attrName=malloc(info->attrLength))==NULL);

  memcpy(info->attrName,block+sizeof(File_type)+ 1 + sizeof(int),info->attrLength);
  memcpy(&info->numBuckets,block+sizeof(File_type)+ 1 + sizeof(int)+info->attrLength,sizeof(int));

  num_of_open_files++;
  return info;
}

int HT_CloseIndex(HT_info* header_info ){

  if(BF_CloseFile(header_info->fileDesc)<0){
    BF_PrintError("Error closing file");
    return -1;
  }
  --num_of_open_files;

  free(header_info->attrName);
  free(header_info);

  return 0;
}

int HT_InsertEntry(HT_info header_info,Record record) {
  void * val;
  static int flag=0;
  // this static flag is for not checking every time which key to hash
  // so the flag is set only the first time of an insert and then the key is well known!
  switch(flag){
    case 0:
      if(!strcmp(header_info.attrName,"id")){
        val=&record.id;
        flag=1;
      }else if(!strcmp(header_info.attrName,"name")){
        val=record.name;
        flag=2;
      }else if(!strcmp(header_info.attrName,"surname")){
        val=record.surname;
        flag=3;
      }else{
        val=record.address;
        flag=4;
      }
      break;
    case 1:
      val=&record.id;
      break;
    case 2:
      val=record.name;
      break;
    case 3:
      val=record.surname;
      break;
    case 4:
      val=record.address;
      break;
  }
  // insert starts here
  // FIRST WE NEED TO CHECK IF RECOORD EXISTS
  int start_bucket=(int)((hash_funct(val,header_info.attrType)%header_info.numBuckets))+1;
  int current_block=start_bucket;
  int last_block=start_bucket;
  uint8_t * block;
  int records_in_block;
  int num = 0;
  int next;
  while(current_block!=0) {
    if (BF_ReadBlock(header_info.fileDesc,current_block,(void**)&block)<0){
      BF_PrintError("Error reading block from file line 181 [HT_InsertEntry]");
      return -1;
    }
    //get how many entries there are in bucket
    memcpy(&records_in_block,block,sizeof(int));
    if (records_in_block == 0) {
      if(BF_ReadBlock(header_info.fileDesc,last_block,(void**)&block)<0){
        BF_PrintError("Error reading block from file line 188 [HT_InsertEntry]");
        return -1;
      }
      break;
    }

    Record r;
    for(int j=0;j<records_in_block;j++){
      if(mem_to_record(block+sizeof(int)+j*sizeofRecord(&record),&r)==0){
        printf("Mem to record fail![HP_InsertEntry]");
        return -1;
      }

      if(reccmp(&r,&record,header_info.attrName)==0){
        //it means record exists
        // printf("Record exists (id: %d)!\n",record.id);
        return -1;
      }
    }
    // if not in this block search the next linked block
    last_block=current_block;
    num = records_in_block;
    memcpy(&current_block,block+BLOCK_SIZE-sizeof(int),sizeof(int));
    next = current_block;
  }
  records_in_block = num;

  // if doesn't exist and block is full of records

  if(records_in_block==(BLOCK_SIZE-2*sizeof(int))/sizeofRecord(&record)){
    int new_block_id;
    records_in_block=0;
    if (next == 0) {
      // we must append a new block to file and link it to last_block
      // allocate block
      do{
        // if fail -> try again
      }while(BF_AllocateBlock(header_info.fileDesc)<0);
      new_block_id=BF_GetBlockCounter(header_info.fileDesc)-1;
      memcpy(block+BLOCK_SIZE-sizeof(int),&new_block_id,sizeof(int));
      //write new_block_id in the end of last_block
      // and write changes to memory
      if(BF_WriteBlock(header_info.fileDesc,last_block)<0){
        BF_PrintError("Error writing block 0 to file");
        return -1;
      }
    }
    else {
      new_block_id=next;
    }
    last_block=new_block_id;
    // and then get the new block
    if(BF_ReadBlock(header_info.fileDesc,new_block_id,(void**)&block)<0){
      BF_PrintError("Error reading block from file line 241 [HP_insertEntry]");
      return -1;
    }
  }

  if(record_to_mem(block+sizeof(int)+records_in_block*sizeofRecord(&record),&record)==0){
    // printf("---Failed to insert record with id %d---\n",record.id);
    return -1;
  }
  records_in_block++;
  // update number of records in block
  memcpy(block, &records_in_block,sizeof(int));

  if(BF_WriteBlock(header_info.fileDesc,last_block)<0){
    BF_PrintError("Error writing block 0 to file");
    return -1;
  }

  return last_block;
}

int HT_DeleteEntry(HT_info header_info,void*value){
  //FIRST WE NEED TO CHECK IF RECORD EXISTS
  int start_bucket=(int)((hash_funct(value,header_info.attrType)%header_info.numBuckets))+1;
  int current_block=start_bucket;
  // if there is no linked list in bucket the last_block i same as start_bucket
  int last_block=start_bucket;
  uint8_t * block;
  int which_record;
  int exist=0;
  int records_in_block;
  while(current_block!=0 && !exist){
    if(BF_ReadBlock(header_info.fileDesc,current_block,(void**)&block)<0){
      BF_PrintError("Error reading block from file line 274 [HP_DeleteEntry]");
      return -1;
    }
    //get how many entries there are in bucket
    memcpy(&records_in_block,block,sizeof(int));
    Record r;
    for(int j=0;j<records_in_block;j++){
      if(mem_to_record(block+sizeof(int)+j*sizeofRecord(&r),&r)==0){
        printf("Mem to record fail![HP_DeleteEntry]");
        return -1;
      }
      if(rec_key_cmp(&r,value,header_info.attrName)==0){
        //it means record exists
        // printf("Record exists (id: %d)!\n",record.id);
        exist=1;
        which_record=j;
        break;
      }
    }
    // if not in this block search the next linked block
    last_block=current_block;
    memcpy(&current_block,block+BLOCK_SIZE-sizeof(int),sizeof(int));
  }
  if(exist){
    //now last_block contains the id of block where record is located
    //SO WE ASSIGN IT TO WHICH_BLOCK AND CONTINUE TO SEARCH THE LAST NOT EMPTY BLOCK FROM LIST
    int which_block=last_block;
    Record last_record;
    //next block to search is current_block
    while(current_block!=0){
      if(BF_ReadBlock(header_info.fileDesc,current_block,(void**)&block)<0){
        BF_PrintError("Error reading block from file line 305 [HP_DeleteEntry]");
        return -1;
      }
      memcpy(&records_in_block,block,sizeof(int));
      if(records_in_block==0){
        // if there are no records in block it means all the following blocks are empty too..
        break;
      }
      last_block=current_block;
      memcpy(&current_block,block+BLOCK_SIZE-sizeof(int),sizeof(int));
    }
    //read last block
    if(BF_ReadBlock(header_info.fileDesc,last_block,(void**)&block)<0){
      BF_PrintError("Error reading block from file after locating last block with at least 1 rec! [HP_DeleteEntry]");
      return -1;
    }
    // if im in last block and there is only 1 record simpy decrease the counter
    if(which_record==last_block && records_in_block==1){
      records_in_block=0;
      memcpy(block,&records_in_block,sizeof(int));
      if(BF_WriteBlock(header_info.fileDesc,which_block)<0){
        BF_PrintError("Error writing block to file");
        return -1;
      }
      return 0;
    }
    memcpy(&records_in_block,block,sizeof(int));
    //get the last record
    if(mem_to_record(block+sizeof(int)+(records_in_block-1)*sizeofRecord(&last_record),&last_record)==0){
      printf("Mem to record fail![HP_Delete]");
      return -1;
    }
    // decrease counter
    records_in_block--;
    // printf("%d %d\n", last_block, records_in_block);
    // and write to block
    memcpy(block,&records_in_block,sizeof(int));
    if(BF_WriteBlock(header_info.fileDesc,last_block)<0){
      BF_PrintError("Error writing last block from linked list to file");
      return -1;
    }
    // get block where record is located
    if(BF_ReadBlock(header_info.fileDesc,which_block,(void**)&block)<0){
      BF_PrintError("Error reading block where record is located line 348 [HP_DeleteEntry]");
      return -1;
    }
    // now ovewrite the record to be deleted;
    if(record_to_mem(block+sizeof(int)+which_record*sizeofRecord(&last_record),&last_record)==0)
      return -1;

    if(BF_WriteBlock(header_info.fileDesc,which_block)<0){
      BF_PrintError("Error writing block to file [HP_deleteentry]");
      return -1;
      }
    // deletion is over
    return 0;
  }
  // if doesn't exist
  return -1;
}

int HT_GetAllEntries( HT_info header_info,void *value){
  // FIRST WE MUST SEARCH FOR RECORD WITH PRIMARY KEY== VALUE
  // IF WE DONT FIND IT WE MUST PRINT ALL RECORDS FROM BUCKETS
  int blocks_read=0;
  uint8_t * block;
  int records_in_block;
  Record r;

  if (value!=NULL) {
    int start_bucket=(int)((hash_funct(value,header_info.attrType)%header_info.numBuckets))+1;
    int current_block=start_bucket;
    while(current_block!=0){
      if(BF_ReadBlock(header_info.fileDesc,current_block,(void**)&block)<0){
        BF_PrintError("Error reading block from file line 379 [HT_GetAllEntries]");
        return -1;
      }
      blocks_read++;
      //get how many entries there are in bucket
      memcpy(&records_in_block,block,sizeof(int));
      Record r;
      for(int j=0;j<records_in_block;j++){
        if(mem_to_record(block+sizeof(int)+j*sizeofRecord(&r),&r)==0){
          printf("Mem to record fail![HT_GetAllEntries]");
          return -1;
        }
        if(rec_key_cmp(&r,value,header_info.attrName)==0){
          //it means record exists
          printRecord(r);
          return blocks_read;
        }
      }
      // if not in this block search the next linked block
      memcpy(&current_block,block+BLOCK_SIZE-sizeof(int),sizeof(int));
    }
  }
  else {
    for (int i=1;i<=header_info.numBuckets;++i) {
      int current_block = i;
      while(current_block!=0){
        if(BF_ReadBlock(header_info.fileDesc,current_block,(void**)&block)<0){
          BF_PrintError("Error reading block from file line 406 [HT_GetAllEntries]");
          return -1;
        }
        blocks_read++;
        //get how many entries there are in bucket
        memcpy(&records_in_block,block,sizeof(int));
        Record r;
        for(int j=0;j<records_in_block;j++){
          if(mem_to_record(block+sizeof(int)+j*sizeofRecord(&r),&r)==0){
            printf("Mem to record fail![HT_GetAllEntries]");
            return -1;
          }
          printf("--------------------\n");
          printRecord(r);
        }
        // if not in this block search the next linked block
        memcpy(&current_block,block+BLOCK_SIZE-sizeof(int),sizeof(int));
      }
    }
  }

  return blocks_read;
}

int PHashStatistics(HT_info * info) {
  uint8_t * block = NULL;
  int blocks_num = BF_GetBlockCounter(info->fileDesc);
  printf("\n\n*************************\n");
  printf("The file has %d blocks.\n", blocks_num);
  printf("*************************\n\n");

  Record r = {0};
  int records_in_block = 0;

  int buckets_over = 0;
  int blocks_per_bucket = 0;
  for (int i = 1; i <= info->numBuckets; i++) {
      int min = (BLOCK_SIZE -2*sizeof(int))/sizeofRecord(&r);
      int max = -1;
      float avg = 0.0;

      int block_count = 0;
      int current_block = i;
      while (current_block != 0) {
          ++block_count;
          if(BF_ReadBlock(info->fileDesc, current_block, (void **)&block) < 0) {
              BF_PrintError("Error reading block from file line 462 [HashStatistics]\n");
              return -1;
          }
          //get how many entries there are in bucket
          memcpy(&records_in_block, block, sizeof(int));
          if (records_in_block == 0)
              break;

          if (records_in_block < min)
              min = records_in_block;
          if (records_in_block > max)
              max = records_in_block;
          avg += records_in_block;

          memcpy(&current_block, block +BLOCK_SIZE-sizeof(int), sizeof(int));
      }
      blocks_per_bucket += block_count;

      if (block_count != 0)
          avg /= block_count;
      if (block_count > 1)
          buckets_over++;
      printf("For bucket: %d\tMin: %d\tMax: %d\tAvg: %f\tOverflow_blocks: %d\n", i, min, max, avg, block_count-1);
  }
  blocks_per_bucket /= info->numBuckets;

  printf("\n\n******************************************\n");
  printf("Average blocks number for each bucket: %d\n", blocks_per_bucket);
  printf("******************************************\n\n");

  printf("\n\n*******************************************\n");
  printf("Number of buckets with overflow blocks: %d\n", buckets_over);
  printf("*******************************************\n\n");

  if (HT_CloseIndex(info)==-1) {
    printf("FAILED TO CLOSE DATABASE FILE\n");
    return -1;
  }

  return 0;
}
