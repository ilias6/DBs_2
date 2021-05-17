#ifndef __HT_INFO_H__
#define __HT_INFO_H__

typedef enum file_type {HASH_FILE, SHASH_FILE} File_type;

typedef struct ht_info {
  File_type t;
  char attrType;
  int attrLength;
  char * attrName;
  int fileDesc;
  // this is for hash file
  int numBuckets;
} HT_info;

typedef struct sht_info {
  File_type t;
  int fileDesc;
  char * attrName;
  int attrLength;
  int numBuckets;
  char * fileName;
} SHT_info;

void print_info(HT_info * info);

#endif
