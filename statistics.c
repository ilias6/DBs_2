#include "statistics.h"

int HashStatistics(char * filename) {
    HT_info * prim_info = HT_OpenIndex(filename);
    SHT_info * sec_info = NULL;
    int flag = 0;
    if (prim_info == NULL) {
      sec_info = SHT_OpenIndex(filename);
      flag = 1;
      if (sec_info == NULL) {
          printf("FAILED TO OPEN DATABASE FILE\n");
          return -1;
      }
    }

    if (flag)
      return SHashStatistics(sec_info);
    return PHashStatistics(prim_info);
}
