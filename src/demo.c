
#include "HashTable.h"

#include <stdio.h>

void htStats(HashTable ht) {
	printf(
		"Size: %i slots\n"
		"Impact: %i bytes\n"
		"Load: %g%%\n",
		HashTableSlotCount(ht),
		HashTableImpact(ht),
		HashTableLoadFactor(ht)
	);
}

int main ( int argc, char **argv )
{
	HashTable x = NewHashTable(8, 0, NULL, NULL);
	char * name = "this.value";
	HashTableItem result = HashTablePut(x, htStr(name), htStr("unlimited"));
	printf("Initial record index: %i\n", result);
	result = HashTableGet(x, htStr(name));
	printf("Retrieved record index: %i\n", result);
	result = HashTableDistribution(x, htStr(name));
	printf("Initial record distribution: %i\n", result);
	htStats(x);
	DestroyHashTable(x);
	return 0;
}
