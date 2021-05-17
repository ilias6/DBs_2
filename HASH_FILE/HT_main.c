#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "HT_functs.h"
#include "../Record/Record.h"

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
#define NUM_BUCKETS 19

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

    HT_Init();
    HT_CreateIndex("hash_db", 'i', "id", 3, NUM_BUCKETS);
    HT_info * info;
    info = HT_OpenIndex("hash_db");
    Record r;
    FILE * input_stream;
    char buff[200];
    char * name;
    char * surname;
    char * address;
    int id;
    const char delim[2] = ",";
    //open records file for reading
    printf(WHITE "**INSERTING RECORDS FROM FILE TO DATABASE...\n->"RESET);

    input_stream = fopen(argv[1], "r");
    if (input_stream==NULL) {
        printf(BOLDRED " Error opening file!\n" RESET);
        return 1;
    }

    int len;
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
        setRecord(&r, id, name, surname, address);
        if (HT_InsertEntry(*info, r) <= 0) {
            printf(BOLDRED"--FAILED TO INSERT--\n"RESET);
            printRecord(r);
        }
    }
    fclose(input_stream);
    printf(GREEN" RECORDS INSERTED SUCCESSFULLY\n"RESET);

    id = 801;
    int blocks_read;
    blocks_read = HT_GetAllEntries(*info, &id);
    printf("--------------------\n");
    printf(WHITE "-- %d blocks read --\n" RESET, blocks_read);
    printf("--------------------\n");

    printf(WHITE "**SAVING DATABASE AND CLOSING...\n->"RESET);
    if (HT_CloseIndex(info)==-1) {
        printf(BOLDRED" FAILED TO CLOSE DATABASE (An error occurred)!\n"RESET);
        return 1;
    }
    printf(GREEN" DATABASE CLOSED AND SAVED SUCCESSFULLY\n"RESET);

    /* again open db */
    printf(WHITE "**OPENING DATABASE...\n->"RESET);
    info = HT_OpenIndex("hash_db");
    if (info==NULL) {
        printf(BOLDRED"FAILED TO OPEN DATABASE FILE\n"RESET);
        return 1;
    }
    printf(GREEN" DATABASE OPENED SUCCESSFULLY\n"RESET);

    printf(WHITE "**PRINTING ALL RECORDS\n"RESET);
    blocks_read = HT_GetAllEntries(*info, NULL);

    printf("--------------------\n");
    printf(WHITE "-- %d blocks read --\n" RESET, blocks_read);
    printf("--------------------\n");

    printf(WHITE "**SAVING DATABASE AND CLOSING...\n->"RESET);
    if (HT_CloseIndex(info)==-1) {
        printf(BOLDRED" FAILED TO CLOSE DATABASE (An error occurred)!\n"RESET);
        return 1;
    }
    printf(GREEN" DATABASE CLOSED SUCCESSFULLY\n"RESET);

    printf(BOLDYELLOW"\n\n\tCALCULATING HASH FILE STATISTICS...\n"RESET);
    HashStatistics("hash_db");

    /* again open db and remove 100 records */
    printf(WHITE "**OPENING DATABASE...\n->"RESET);
    info = HT_OpenIndex("hash_db");
    if (info==NULL) {
        printf(BOLDRED"FAILED TO OPEN DATABASE FILE\n"RESET);
        return 1;
    }
    printf(GREEN" DATABASE OPENED SUCCESSFULLY\n"RESET);
    //removing 100 records
    printf(WHITE "**DELETING FROM DATABASE 1000 RECORDS WITH ids [1-1001]...\n"RESET);

    for (int ii = 1; ii <= 1001; ii++)
        if (HT_DeleteEntry(*info, &ii)==-1) {
            printf(RED"Record with id %d doesn't exist!\n"RESET, ii);
        }
    printf("-> ");
    printf(GREEN "RECORDS REMOVED SUCCESSFULLY\n"RESET);
    //closing database for opening again
    printf(WHITE "**SAVING DATABASE AND CLOSING...\n->"RESET);
    if (HT_CloseIndex(info)==-1) {
        printf(BOLDRED" FAILED TO CLOSE DATABASE (An error occurred)!\n"RESET);
        return 1;
    }
    printf(GREEN" DATABASE CLOSED AND SAVED SUCCESSFULLY\n"RESET);
    printf("***************************************************\n");

    printf(BOLDYELLOW"\n\n\tCALCULATING HASH FILE STATISTICS...\n"RESET);
    HashStatistics("hash_db");

    printf(WHITE "**OPENING DATABASE...\n->"RESET);
    info = HT_OpenIndex("hash_db");
    if (info==NULL) {
        printf(BOLDRED"FAILED TO OPEN DATABASE FILE\n"RESET);
        return 1;
    }
    printf(GREEN" DATABASE OPENED SUCCESSFULLY\n"RESET);

    input_stream = fopen(argv[1], "r");
    if (input_stream==NULL) {
        printf(BOLDRED " Error opening file!\n" RESET);
        return 1;
    }

    printf(WHITE "**INSERTING 510 FIRST RECORDS FROM FILE TO DATABASE...\n"RESET);
    for (int i = 0; (i < 510) && (fgets(buff, 199, input_stream) != NULL); ++i) {
        len = strlen(buff);
        buff[len-2] = '\0';
        id = atoi(strtok(buff +1, delim));
        name = strtok(NULL, delim) +1;
        name[strlen(name) -1] = '\0';
        surname = strtok(NULL, delim) +1;
        surname[strlen(surname) -1] = '\0';
        address = strtok(NULL, delim) +1;
        address[strlen(address) -1] = '\0';
        setRecord(&r, id, name, surname, address);
        if (HT_InsertEntry(*info, r) <= 0) {
            printf("->");
            printf(BOLDRED"--FAILED TO INSERT--\n"RESET);
            printRecord(r);
        }
    }
    printf("->");
    printf(GREEN" RECORDS INSERTED SUCCESSFULLY\n"RESET);

    printf(WHITE "**SAVING DATABASE AND CLOSING...\n->"RESET);
    if (HT_CloseIndex(info)==-1) {
        printf(BOLDRED" FAILED TO CLOSE DATABASE (An error occurred)!\n"RESET);
        return 1;
    }
    printf(GREEN" DATABASE CLOSED AND SAVED SUCCESSFULLY\n"RESET);
    printf("***************************************************\n");

    printf(BOLDYELLOW"\n\n\tCALCULATING HASH FILE STATISTICS...\n"RESET);
    HashStatistics("hash_db");

    printf(WHITE "**OPENING DATABASE...\n->"RESET);
    info = HT_OpenIndex("hash_db");
    if (info==NULL) {
        printf(BOLDRED"FAILED TO OPEN DATABASE FILE\n"RESET);
        return 1;
    }
    printf(GREEN" DATABASE OPENED SUCCESSFULLY\n"RESET);

    printf(WHITE "**INSERTING THE NEXT 500 RECORDS FROM FILE TO DATABASE...\n"RESET);
    for (int i = 0; (i < 500) && (fgets(buff, 199, input_stream) != NULL); ++i) {
        len = strlen(buff);
        buff[len-2] = '\0';
        id = atoi(strtok(buff +1, delim));
        name = strtok(NULL, delim) +1;
        name[strlen(name) -1] = '\0';
        surname = strtok(NULL, delim) +1;
        surname[strlen(surname) -1] = '\0';
        address = strtok(NULL, delim) +1;
        address[strlen(address) -1] = '\0';
        setRecord(&r, id, name, surname, address);
        if (HT_InsertEntry(*info, r) <= 0) {
            printf("->");
            printf(BOLDRED"--FAILED TO INSERT--\n"RESET);
            printRecord(r);
        }
    }

    fclose(input_stream);
    printf("->");
    printf(GREEN" RECORDS INSERTED SUCCESSFULLY\n"RESET);

    printf(WHITE "**SAVING DATABASE AND CLOSING...\n->"RESET);
    if (HT_CloseIndex(info)==-1) {
        printf(BOLDRED" FAILED TO CLOSE DATABASE (An error occurred)!\n"RESET);
        return 1;
    }
    printf(GREEN" DATABASE CLOSED AND SAVED SUCCESSFULLY\n"RESET);
    printf("***************************************************\n");

    printf(BOLDYELLOW"\n\n\tCALCULATING HASH FILE STATISTICS...\n"RESET);
    HashStatistics("hash_db");

    return 0;
}
