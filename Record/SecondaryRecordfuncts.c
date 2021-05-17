#include <stdio.h>
#include "SecondaryRecord.h"
#include "Record.h"
#include <stdlib.h>
#include <string.h>

void printSecondaryRecord(SecondaryRecord r){
  printf("----------------------\n");
  printRecord(r.record);
  printf("BLOCK: %d\n",r.blockId);
  return ;
}

int sizeofSecondaryRecord(SecondaryRecord * r) {
  return sizeof(r->record.surname) +sizeof(r->record.id) +sizeof(r->blockId);
}

void setSecondaryRecord(SecondaryRecord * rptr, int blockId) {
  rptr->blockId = blockId;
  return ;
}

int secreccmp(SecondaryRecord * r1, SecondaryRecord *r2, char * attrib) {//compare depending attribute values
  if (!strcmp(attrib, "id")) {
    if (r1->record.id > r2->record.id)
      return 1;
    else if (r1->record.id == r2->record.id)
      return 0;
    else
      return -1;
  }
  else if (!strcmp(attrib, "name")) {
    return strcmp(r1->record.name, r2->record.name);
  }
  else if (!strcmp(attrib, "surname")) {
    return strcmp(r1->record.surname, r2->record.surname);
  }
  else if (!strcmp(attrib, "address")) {
    return strcmp(r1->record.address, r2->record.address);
  }
}

//from r to mem
int secondary_record_to_mem(uint8_t * mem, SecondaryRecord * r) {
  int offset = 0;
  memcpy(mem +offset, r->record.surname, sizeof(r->record.surname));
  offset += sizeof(r->record.surname);
  memcpy(mem +offset, &r->blockId, sizeof(int));
  offset += sizeof(int);
  return offset;//returns how many bytes are written to mem
}

int mem_to_secondary_record(uint8_t * mem, SecondaryRecord * r) {
  int offset = 0;
  memcpy(r->record.surname, mem+offset, sizeof(r->record.surname));
  offset += sizeof(r->record.surname);
  memcpy(&r->blockId, mem+offset, sizeof(int));
  offset += sizeof(int);
  return offset;
}
