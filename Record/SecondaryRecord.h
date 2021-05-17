#ifndef __SECONDARYRECORD_H__
#define __SECONDARYRECORD_H__
#include "Record.h"
typedef struct sec_record {
  Record record;
  int blockId;
} SecondaryRecord;
void printSecondaryRecord(SecondaryRecord r);
int sizeofSecondaryRecord(SecondaryRecord *r);
void setSecondaryRecord(SecondaryRecord * rptr, int blockId);
int secreccmp(SecondaryRecord * r1,SecondaryRecord *r2,char * attrib);//compare depending attribute value
//from r to mem
int secondary_record_to_mem(uint8_t * mem,SecondaryRecord * r);//pointer is for not copying whole struct in each call
//from mem to r
int mem_to_secondary_record(uint8_t * mem, SecondaryRecord * r);
#endif
