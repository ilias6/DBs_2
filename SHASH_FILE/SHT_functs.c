#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "SHT_functs.h"
#include "../HASH_FILE/HT_functs.h"
#include "../Record/SecondaryRecord.h"
#include "../Record/Record.h"
#include "../BF.h"
#include "../hash_functions/hashfuncts.h"

#define MAX_FILES 100
extern int num_of_open_files;
                             /*

                                      WE SUPPOSE THAT BUCKET SIZE == BLOCK_SIZE


                                                                                      */
// GLOBAL POINTER TO HASH FUNCT --> IF WE WANT TO CHANGE HASHFUNCT WE CHANGE THIS POINTER

unsigned long int (*hash_funct)(void * value,char type);

void SHT_Init() {
  BF_Init();
  return ;
}

int SHT_CreateIndex(char * sfileName, char * attrName, int attrLength, int buckets, char * fileName){

  if (HT_CreateIndex(fileName, 'i', "id", 3, buckets) < 0) {
    printf("Error creating primary index file\n");
    return -1;
  }
  File_type type = SHASH_FILE;
  if(BF_CreateFile(sfileName) < 0) {
    BF_PrintError("Error creating secondary index file");
    return -1;
  }
  int sec_fd = BF_OpenFile(sfileName);
  if (sec_fd < 0) {
    BF_PrintError("Error opening secondary index file");
    return -1;
  }
  do {
    // if fail -> try again
  } while(BF_AllocateBlock(sec_fd) < 0);
  uint8_t * block = NULL;
  if (BF_ReadBlock(sec_fd, 0, (void**)&block) < 0) {
    BF_PrintError("Error reading block 0 from file");
    return -1;
  }

  type = SHASH_FILE;
  memcpy(block, &type, sizeof(File_type));
  memcpy(block+sizeof(File_type), (uint8_t *)&attrLength, sizeof(int));
  memcpy(block+sizeof(File_type)+sizeof(int),(uint8_t *)attrName, attrLength);
  memcpy(block+sizeof(File_type)+sizeof(int)+attrLength, &buckets, sizeof(int));
  int prim_name_size = strlen(fileName)+1;
  memcpy(block+sizeof(File_type)+2*sizeof(int)+attrLength, &prim_name_size, sizeof(int));
  memcpy(block+sizeof(File_type)+3*sizeof(int)+attrLength, (uint8_t *)fileName, prim_name_size);

  if(BF_WriteBlock(sec_fd, 0) < 0) {
    BF_PrintError("Error writing block 0 to file");
    return -1;
  }

  // allocate bucket blocks under superblock and write 0 as first int indicating current records in block
  int records_at_start = 0;
  int next_overflow = 0; // next bucket is written at the end of bucket indicating which block is next
  // each bucket conatins one int at start indicating how many records are in bucket
  // and one int in end indicating which block is next when bucket is full (like linked list)
  // we conclude that each bucket has BLOCK_SIZE - 2*sizeof(int) bytes available for data
  for(int i = 1; i <= buckets; i++) {
    do {
    } while(BF_AllocateBlock(sec_fd) < 0);

    if (BF_ReadBlock(sec_fd, i, (void**)&block) < 0) {
      BF_PrintError("Error reading block 0 from secondary file");
      return -1;
    }
    memcpy(block, &records_at_start, sizeof(int));
    memcpy(block +BLOCK_SIZE -sizeof(int), &next_overflow, sizeof(int));
    if (BF_WriteBlock(sec_fd, i) < 0) {
      BF_PrintError("Error writing block to secondary file");
      return -1;
    }
  }

  if (BF_CloseFile(sec_fd) < 0) {
    BF_PrintError("Error closing secondary index file");
    return -1;
  }

  return 0;
}

SHT_info * SHT_OpenIndex(char * fileName) {
  num_of_open_files++;
  // open up to MAX_FILES
  if (num_of_open_files > MAX_FILES)
    return NULL;
  SHT_info * info = malloc(sizeof(SHT_info));
  if (info==NULL) {
    printf("Malloc error line 118!\n");
    return NULL;
  }
  info->fileDesc = BF_OpenFile(fileName);
  if (info->fileDesc < 0) {
    free(info);
    BF_PrintError("Error opening file");
    return NULL;
  }
  uint8_t * block;
  do{
  } while(BF_ReadBlock(info->fileDesc, 0, (void **)&block) < 0);

  memcpy(&info->t, block, sizeof(File_type));
  if (info->t != SHASH_FILE) {
    info->fileDesc = BF_CloseFile(info->fileDesc);
    free(info);
    return NULL;
  }
  memcpy(&info->attrLength, block+sizeof(File_type), sizeof(int));

  do{
  }while((info->attrName=malloc(info->attrLength))==NULL);

  memcpy(info->attrName, block +sizeof(File_type) +sizeof(int), info->attrLength);
  memcpy(&info->numBuckets, block +sizeof(File_type) +sizeof(int) +info->attrLength, sizeof(int));
  int prim_name_size = 0;
  memcpy(&prim_name_size, block +sizeof(File_type) + 2*sizeof(int) +info->attrLength, sizeof(int));

  do{
  }while((info->fileName=malloc(prim_name_size))==NULL);

  memcpy(info->fileName, block +sizeof(File_type) + 3*sizeof(int) +info->attrLength, prim_name_size);
  num_of_open_files++;
  return info;
}

int SHT_InsertEntry(SHT_info sec_info, SecondaryRecord r_to_insert) {
  /*Getting the id of the block that the record was inserted in the primary index file*/
  HT_info * prim_info = HT_OpenIndex(sec_info.fileName);
  int blockid = HT_InsertEntry(*prim_info, r_to_insert.record);
  if (HT_CloseIndex(prim_info)==-1) {
      return -1;
  }
  if (blockid < 0 ) {
    printf("Record already exists!\n");
    return -1;
  }

  r_to_insert.blockId = blockid;

  static int flag = 0;
  // this static flag is for not checking every time which key to hash
  // so the flag is set only the first time of an insert and then the key is well known!
  void * val = NULL;
  switch(flag) {
    case 0:
      if(!strcmp(sec_info.attrName, "id")) {
        val = &r_to_insert.record.id;
        flag = 1;
      }
      else if(!strcmp(sec_info.attrName, "name")) {
        val = r_to_insert.record.name;
        flag = 2;
      }
      else if(!strcmp(sec_info.attrName,"surname")) {
        val = r_to_insert.record.surname;
        flag = 3;
      }
      else {
        val = r_to_insert.record.address;
        flag = 4;
      }
      break;
    case 1:
      val=&r_to_insert.record.id;
      break;
    case 2:
      val=r_to_insert.record.name;
      break;
    case 3:
      val=r_to_insert.record.surname;
      break;
    case 4:
      val=r_to_insert.record.address;
      break;
  }

  uint8_t * block = NULL;
  int records_in_block = 0;
  SecondaryRecord sec_rec = {0};

  int start_bucket = (int)((hash_funct(val, 's')%sec_info.numBuckets))+1;
  int current_block = start_bucket;
  int last_block = current_block;
  while (current_block != 0) {
    if (BF_ReadBlock(sec_info.fileDesc, current_block, (void **)&block) < 0) {
      BF_PrintError("Error reading block from file line 197 [SHT_InsertEntry]");
      return -1;
    }

    last_block = current_block;
    memcpy(&records_in_block, block, sizeof(int));
    for (int i = 0; i < records_in_block; ++i) {
      mem_to_secondary_record(block+sizeof(int)+i*sizeofSecondaryRecord(&sec_rec), &sec_rec);
      if ((sec_rec.blockId == r_to_insert.blockId) &&
      (!strcmp(sec_rec.record.surname, r_to_insert.record.surname)))
          return 0;
    }
    if (records_in_block == (BLOCK_SIZE -2*sizeof(int))/sizeofSecondaryRecord(&r_to_insert))
      memcpy(&current_block, block +BLOCK_SIZE -sizeof(int), sizeof(int));
    else
      break;

  }
  /*Allocate new block*/
  if (current_block == 0) {
    do {
      // if fail -> try again
    } while(BF_AllocateBlock(sec_info.fileDesc) < 0);
    int new_block_id = BF_GetBlockCounter(sec_info.fileDesc)-1;
    memcpy(block +BLOCK_SIZE -sizeof(int), &new_block_id, sizeof(int));
    if (BF_WriteBlock(sec_info.fileDesc, last_block) < 0) {
      BF_PrintError("Error writing block 0 to file");
      return -1;
    }
    if (BF_ReadBlock(sec_info.fileDesc, new_block_id, (void**)&block) < 0) {
      BF_PrintError("Error reading block from file line 181 [HT_InsertEntry]");
      return -1;
    }
    int next = 0;
    memcpy(block +BLOCK_SIZE -sizeof(int), &next, sizeof(int));
    records_in_block = 0;
    current_block = new_block_id;
  }
  if (secondary_record_to_mem(block +sizeof(int) +records_in_block*sizeofSecondaryRecord(&r_to_insert), &r_to_insert) == 0) {
    return -1;
  }
  records_in_block++;
  memcpy(block, &records_in_block, sizeof(int));
  if (BF_WriteBlock(sec_info.fileDesc, current_block) < 0) {
    BF_PrintError("Error writing block 0 to file");
    return -1;
  }

  return 0;
}

int SHT_GetAllEntries(SHT_info sec_info, HT_info prim_info, void * value) {
  int blocks_read = 0;

  uint8_t * prim_block = NULL;
  uint8_t * sec_block = NULL;

  SecondaryRecord sec_rec = {0};
  Record record = {0};

  int records_in_prim_block = 0;
  int records_in_sec_block = 0;

  int current_block = (int)((hash_funct(value, 's')%sec_info.numBuckets))+1;
/*Looking all overflow blocks to find record with key == value (aka surname)*/
  while (current_block != 0) {
    if (BF_ReadBlock(sec_info.fileDesc, current_block, (void **)&sec_block) < 0) {
      BF_PrintError("Error reading block from file line 197 [SHT_InsertEntry]");
      return -1;
    }
    blocks_read++;
    memcpy(&records_in_sec_block, sec_block, sizeof(int));
    /*Check surnames of this block*/
    for (int i = 0; i < records_in_sec_block; ++i) {
      mem_to_secondary_record(sec_block +i*sizeofSecondaryRecord(&sec_rec)+sizeof(int), &sec_rec);
      /*Not the one we want*/
      if (strcmp(sec_rec.record.surname, value))
        continue;
      /*Found one, so get the block of the primary index file*/
      if (BF_ReadBlock(prim_info.fileDesc, sec_rec.blockId, (void **)&prim_block) < 0) {
        BF_PrintError("Error reading block from file line 197 [SHT_InsertEntry]");
        return -1;
      }
      blocks_read++;

      memcpy(&records_in_prim_block, prim_block, sizeof(int));
      /*Scan the entire block and print the records that have the surname == value*/
      for (int j = 0; j < records_in_prim_block; ++j) {
        mem_to_record(prim_block +j*sizeofRecord(&record)+sizeof(int), &record);
        if (!strcmp(sec_rec.record.surname, record.surname))
          printRecord(record);
      }

    }
    /*Move to the next overflow block*/
    memcpy(&current_block, sec_block +BLOCK_SIZE -sizeof(int), sizeof(int));
  }

  return blocks_read;
}

int SHT_CloseIndex(SHT_info * sec_info) {
  if (BF_CloseFile(sec_info->fileDesc) < 0) {
    BF_PrintError("Error closing file");
    return -1;
  }
  num_of_open_files--;

  free(sec_info->attrName);
  free(sec_info->fileName);
  free(sec_info);

  return 0;
}

int SHashStatistics(SHT_info * info) {
  uint8_t * block = NULL;
  int blocks_num = BF_GetBlockCounter(info->fileDesc);
  printf("\n\n*************************\n");
  printf("The file has %d blocks.\n", blocks_num);
  printf("*************************\n\n");

  SecondaryRecord r = {0};
  int records_in_block = 0;

  int buckets_over = 0;
  int blocks_per_bucket = 0;
  for (int i = 1; i <= info->numBuckets; i++) {
      int min = (BLOCK_SIZE -2*sizeof(int))/sizeofSecondaryRecord(&r);
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

  if (SHT_CloseIndex(info)==-1) {
    printf("FAILED TO CLOSE DATABASE FILE\n");
    return -1;
  }

  return 0;
}
