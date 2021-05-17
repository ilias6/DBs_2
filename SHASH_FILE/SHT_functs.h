#ifndef __SHT_FUNCTS_H__
#define __SHT_FUNCTS_H__
#include "../Ht_info.h"
#include "../Record/SecondaryRecord.h"

void SHT_Init();
int SHT_CreateIndex(char * sfileName, char * attrName, int attrLength, int buckets, char * fileName);
SHT_info * SHT_OpenIndex(char * sfileName);
int SHT_CloseIndex( SHT_info * header_info);
int SHT_InsertEntry(SHT_info header_info, SecondaryRecord record);
int SHT_GetAllEntries(SHT_info sec_info, HT_info prim_info, void * value);
int SHashStatistics(SHT_info *);
#endif
