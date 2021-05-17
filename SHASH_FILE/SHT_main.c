#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SHT_functs.h"
#include "../statistics.h"
#include "../HASH_FILE/HT_functs.h"
#include "../Record/SecondaryRecord.h"

//colours for printing the output
#define RESET   "\033[0m"
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDYELLOW "\033[1m\033[33m" /*Bold Orange*/
#define WHITE   "\033[37m"      /* White */

//  needed only for the function below
#include "../Ht_info.h"

//  needed only for definition of BLOCK_SIZE
// #include "BF.h"

#define MAX_FILES 100
#define NUM_BUCKETS 29
int num_of_open_files = 0;

void print_info(HT_info * info) {
    Record r;
    printf("File type: %d\n", info->t);
    printf("Attr type: %c\n", info->attrType);
    printf("Attr len: %d\n", info->attrLength);
    printf("Attr name: %s\n", info->attrName);
    printf("Fd: %d\n", info->fileDesc);
    // printf("Max records in block: %d\n",(BLOCK_SIZE-sizeof(int))/sizeofRecord(&r));
    if (info->t==HASH_FILE)
        printf("Num of buckets: %d\n", info->numBuckets);
    return;
}

int main(int argc, char ** argv) {
  if (argc != 2) {
      printf(GREEN"Usage: %s records_file.txt\n"RESET, argv[0]);
      return 1;
  }

  printf(WHITE "**CREATING SECONDARY INDEX...\n->"RESET);
  SHT_Init();
  SHT_CreateIndex("secondary_hash_db", "surname", 8, NUM_BUCKETS, "primary_hash_db");
  printf(GREEN" DATABASE CREATED SUCCESSFULLY\n"RESET);

  printf(WHITE "**OPENING SECONDARY INDEX...\n->"RESET);
  SHT_info * sec_info = NULL;
  sec_info = SHT_OpenIndex("secondary_hash_db");
  if (sec_info == NULL) {
    printf(BOLDRED" DATABASE OPENING FAILED\n"RESET);
    return -1;
  }
  printf(GREEN" DATABASE OPENED SUCCESSFULLY\n"RESET);

  SecondaryRecord sec_r = {0};
  FILE * input_stream = NULL;
  char buff[200] = {0};
  char * name = NULL;
  char * surname = NULL;
  char * address = NULL;
  int id = 0;
  const char delim[2] = ",";
  //open records file for reading
  printf(WHITE "**INSERTING RECORDS FROM FILE TO DATABASE...\n->"RESET);

  input_stream = fopen(argv[1], "r");
  if (input_stream == NULL) {
      printf(BOLDRED " Error opening file!\n" RESET);
      return 1;
  }

  int len = 0;
  while (fgets(buff, 199, input_stream) != NULL) {
      len = strlen(buff);
      buff[len-2] = '\0';
      // printf("%s\n",buff,strlen(buff));
      id = atoi(strtok(buff +1, delim));
      name = strtok(NULL, delim) +1;
      name[strlen(name) -1] = '\0';
      surname = strtok(NULL, delim) +1;
      surname[strlen(surname) -1] = '\0';
      address = strtok(NULL, delim) +1;
      address[strlen(address) -1] = '\0';
      setRecord(&sec_r.record, id, name, surname, address);
      if (SHT_InsertEntry(*sec_info, sec_r) < 0) {
          printf(BOLDRED"--FAILED TO INSERT--\n"RESET);
          printRecord(sec_r.record);
      }

  }
  fclose(input_stream);
  printf(GREEN" RECORDS INSERTED SUCCESSFULLY\n"RESET);

  input_stream = fopen(argv[1], "r");
  if (input_stream == NULL) {
      printf(BOLDRED " Error opening file!\n" RESET);
      return 1;
  }

  for (int i = 0; i < 10 && (fgets(buff, 199, input_stream) != NULL); ++i) {
      len = strlen(buff);
      buff[len-2] = '\0';
      // printf("%s\n",buff,strlen(buff));
      id = atoi(strtok(buff +1, delim));
      name = strtok(NULL, delim) +1;
      name[strlen(name) -1] = '\0';
      surname = strtok(NULL, delim) +1;
      surname[strlen(surname) -1] = '\0';
      address = strtok(NULL, delim) +1;
      address[strlen(address) -1] = '\0';
      setRecord(&sec_r.record, id, name, surname, address);
      if (SHT_InsertEntry(*sec_info, sec_r) < 0) {
          printf(BOLDRED"--FAILED TO INSERT--\n"RESET);
          printRecord(sec_r.record);
      }
  }
  fclose(input_stream);

  printf(WHITE "**SAVING DATABASE AND CLOSING...\n->"RESET);
  if (SHT_CloseIndex(sec_info)==-1) {
      printf(BOLDRED" FAILED TO CLOSE DATABASE (An error occurred)!\n"RESET);
      return 1;
  }
  printf(GREEN" DATABASE CLOSED AND SAVED SUCCESSFULLY\n"RESET);

  printf(WHITE "**OPENING SECONDARY INDEX...\n->"RESET);
  sec_info = SHT_OpenIndex("secondary_hash_db");
  if (sec_info == NULL) {
    printf(BOLDRED" DATABASE OPENING FAILED\n"RESET);
    return -1;
  }
  printf(GREEN" DATABASE OPENED SUCCESSFULLY\n"RESET);

  printf(WHITE "**OPENING PRIMARY INDEX...\n->"RESET);
  HT_info * prim_info = NULL;
  prim_info = HT_OpenIndex("primary_hash_db");
  if (sec_info == NULL) {
    printf(BOLDRED" DATABASE OPENING FAILED\n"RESET);
    return -1;
  }
  printf(GREEN" DATABASE OPENED SUCCESSFULLY\n"RESET);

  int blocks_read = SHT_GetAllEntries(*sec_info, *prim_info, "surname_7");
  printf("\nBlocks read: %d\n\n", blocks_read);
  blocks_read = SHT_GetAllEntries(*sec_info, *prim_info, "surname_9938");
  printf("\nBlocks read: %d\n\n", blocks_read);
  blocks_read = SHT_GetAllEntries(*sec_info, *prim_info, "surname_100000");
  printf("\nBlocks read: %d\n\n", blocks_read);

  printf(WHITE "**SAVING DATABASE AND CLOSING...\n->"RESET);
  if (SHT_CloseIndex(sec_info)==-1) {
      printf(BOLDRED" FAILED TO CLOSE DATABASE (An error occurred)!\n"RESET);
      return 1;
  }
  if (HT_CloseIndex(prim_info)==-1) {
      printf(BOLDRED" FAILED TO CLOSE DATABASE (An error occurred)!\n"RESET);
      return 1;
  }
  printf(GREEN" DATABASE CLOSED AND SAVED SUCCESSFULLY\n"RESET);

  HashStatistics("primary_hash_db");
  HashStatistics("secondary_hash_db");

  return 0;
}
