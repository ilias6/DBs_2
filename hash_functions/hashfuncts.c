#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hashfuncts.h"

//source: http://www.cse.yorku.ca/~oz/hash.html

unsigned long djb2(void * value,char type){
  char * str;
  char number[11];
  //if type is int
  if(type=='i'){
    sprintf(number,"%d",*(int *)value);
    str=number;
  }else{
    //else if its char
    str=(char *)value;
  }
  unsigned long hash = 5381;
  int c;
  int i=0;
  while (c = str[i++])
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  return hash;
}

unsigned long sdbm(void * value,char type){
  char * str;
  char number[11];
  //if type is int
  if(type=='i'){
    sprintf(number,"%d",*(int *)value);
    str=number;
  }else{
    //else if its char
    str=(char *)value;
  }
  unsigned long hash = 0;
  int c;
  int i=0;
  while (c =str[i++])
    hash = c + (hash << 6) + (hash << 16) - hash;
  return hash;
}
