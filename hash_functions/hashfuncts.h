#ifndef __HASHFUNCTS_H__
#define __HASHFUNCTS_H__
// this hash functs are for strings --> SO I CALL ITOA TO GENERATE STRING FROM INT INSIDE djb2 and sdbm
unsigned long int djb2(void * value, char type);
unsigned long int sdbm(void * value, char type);
#endif
