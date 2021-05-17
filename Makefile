
all: shash_main

shash_main: hash_functions/hashfuncts.o SHASH_FILE/SHT_functs.o HASH_FILE/HT_functs.o SHASH_FILE/SHT_main.o BF_64.a Record/Recordfuncts.o Record/SecondaryRecordfuncts.o statistics.o
	gcc -o shash_main SHASH_FILE/SHT_functs.o SHASH_FILE/SHT_main.o BF_64.a Record/Recordfuncts.o HASH_FILE/HT_functs.o Record/SecondaryRecordfuncts.o hash_functions/hashfuncts.o statistics.o -no-pie

SHT_main.o:  SHASH_FILE/HP_main.c SHASH_FILE/SHT_functs.h Record/SecondaryRecord.h Ht_info.h Record/Record.h
	gcc -c SHASH_FILE/HP_main.c


SHT_functs.o:  SHASH_FILE/SHT_functs.c Record/SecondaryRecord.h Record/Record.h SHASH_FILE/SHT_functs.h BF.h hash_functions/hashfuncts.h HASH_FILE/HT_functs.h
	gcc -c SHASH_FILE/SHT_functs.c
HT_functs.o:  HASH_FILE/HT_functs.c  Record/Recordfuncts.h HASH_FILE/HT_functs.h hash_functions/hashfuncts.h BF.h
	gcc -c HASH_FILE/HT_functs.c

statistics.o: statistics.h statistics.c Record/Record.h
	gcc -c statistics.c

hashfuncts.o:  hash_functions/hashfuncts.c hash_functions/hashfuncts.h
	gcc -c hash_functions/hashfuncts.c


SecondaryRecordfuncts.o : Record/SecondaryRecordfuncts.c Record/SecondaryRecord.h Record/Record.h
	gcc -c Record/SecondaryRecordfuncts.c
Recordfuncts.o: Record/Recordfuncts.c Record/Record.h
	gcc -c Record/Recordfuncts.c


clean:
	rm HASH_FILE/*.o SHASH_FILE/*.o Record/*.o hash_functions/*.o ./statistics.o shash_main primary_hash_db secondary_hash_db
