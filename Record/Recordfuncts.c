#include "Record.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void printRecord(Record r){
  printf("id: %d\n",r.id);
  printf("name: %s\n",r.name);
  printf("surname: %s\n",r.surname);
  printf("address: %s\n",r.address);
  return ;
}

int sizeofRecord(Record *r){//parameter is pointer because we dont want the whole struct to be copied in each call(like &in c++)
  return sizeof(r->id)+sizeof(r->name)+sizeof(r->surname)+sizeof(r->address);
}

void setRecord(Record * rptr, int i, char * n, char * s, char *a) {
  rptr->id=i;
  strcpy(rptr->name,n);
  strcpy(rptr->surname,s);
  strcpy(rptr->address,a);
  return ;
}

int rec_key_cmp(Record * r1,void *value,char * attrib){
  if(!strcmp(attrib,"id")){
    if(r1->id>*((int *)value))
      return 1;
    else if(r1->id==*((int *)value))
      return 0;
    else
      return -1;
  }else if(!strcmp(attrib,"name")){
    return strcmp(r1->name,(char *)value);
  }else if(!strcmp(attrib,"surname")){
    return strcmp(r1->surname,(char *)value);
  }else if(!strcmp(attrib,"address")){
    return strcmp(r1->address,(char *)value);
  }
}

int reccmp(Record * r1,Record *r2,char * attrib){//compare depending attribute value
  /*  return 0 if equal -1 if smaller 1 if bigger     */
  if(!strcmp(attrib,"id")){
    if(r1->id>r2->id)
      return 1;
    else if(r1->id==r2->id)
      return 0;
    else
      return -1;
  }else if(!strcmp(attrib,"name")){
    return strcmp(r1->name,r2->name);
  }else if(!strcmp(attrib,"surname")){
    return strcmp(r1->surname,r2->surname);
  }else if(!strcmp(attrib,"address")){
    return strcmp(r1->address,r2->address);
  }
}

int record_to_mem(uint8_t * mem,Record * r){//pointer is for not copying whole struct in each call
  int offset=0;
  memcpy(mem,&r->id,sizeof(int));
  offset+=sizeof(int);
  memcpy(mem+offset,r->name,sizeof(r->name));
  offset+=sizeof(r->name);
  memcpy(mem+offset,r->surname,sizeof(r->surname));
  offset+=sizeof(r->surname);
  memcpy(mem+offset,r->address,sizeof(r->address));
  offset+=sizeof(r->address);
  return offset;//returns how many bytes are written to mem
}

int mem_to_record(uint8_t * mem,Record * r){
  int offset=0;
  memcpy(&r->id,mem,sizeof(int));
  offset+=sizeof(int);
  memcpy(r->name,mem+offset,sizeof(r->name));
  offset+=sizeof(r->name);
  memcpy(r->surname,mem+offset,sizeof(r->surname));
  offset+=sizeof(r->surname);
  memcpy(r->address,mem+offset,sizeof(r->address));
  offset+=sizeof(r->address);
  return offset;
}
