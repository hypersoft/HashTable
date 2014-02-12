
#include "HashTable.h"
#include <stdio.h>

int main ( int argc, char **argv )
{
	enum HashTableErrorCode e = 0;
	HashTable * hashTable = NewHashTable ( 0, NULL );
	printf ( "New default hash table: %p\n", hashTable );

	char * key = "double.value";
	double value = 123.444;

	if ( HashTablePutDouble(hashTable, key, value, true) ) {
		double got = HashTableGetDouble(hashTable, key);
		printf ( "%s == %g\n", key, got );
	} else {
		e = HashTableGetLastError ( hashTable );
		fprintf(stderr, "%s: Couldn't put %s: Last Error: %lu\n",
			argv[0], key, e
		);
		return e;
	}

	return 0;
}
