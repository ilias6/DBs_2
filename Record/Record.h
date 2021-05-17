#ifndef __RECORD_H__
#define __RECORD_H__
#include <stdint.h>

typedef struct record {
  int id;
  char name[32];
  char surname[32];
  char address[64];
} Record;


void printRecord(Record r);
int sizeofRecord(Record *r);
void setRecord(Record * rptr,int i,char * n,char * s,char *a);
int reccmp(Record * r1,Record *r2,char * attrib);//compare depending attribute value
int rec_key_cmp(Record * r1,void *value,char * attrib);
//from r to mem
int record_to_mem(uint8_t * mem,Record * r);//pointer is for not copying whole struct in each call
//from mem to r
int mem_to_record(uint8_t * mem,Record * r);
#endif
