
#include "HashTable.h"
#include <stdio.h>

int main ( int argc, char **argv )
{
	enum HashTableErrorCode e = 0;
	HashTable * hashTable = NewHashTable ( 0, NULL );
	printf ( "New default hash table: %p\n", hashTable );

	char * key = "double.value";
	double value = 123.444;

	if ( ! HashTablePutDouble(hashTable, key, value, true) ) {
		e = HashTableGetLastError ( hashTable );
		fprintf(stderr, "%s: Couldn't put %s: Last Error: %lu\n",
			argv[0], key, e
		);
		return e;
	}

	if (HashTableHasEntry(hashTable, key)) {
		double got = HashTableGetDouble(hashTable, key);
		e = HashTableGetLastError(hashTable);
		if (!e) printf ( "%s == %g\n", key, got );
		else {
			fprintf(stderr, "%s: Couldn't get %s: Last Error: %lu\n",
				argv[0], key, e
			);
			return e;
		}
	}

	if (HashTableDelete(hashTable, "not-a-key")) {
			fprintf(stderr, "%s: Deleted a non-existant key!\n",
				argv[0]
			);
			return 1;
	} else HashTableGetLastError(hashTable); // belay that order...

	if (HashTableDelete(hashTable, key)) {
			printf("%s: Succesfully deleted %s\n", argv[0], key);
	} else {
		e = HashTableGetLastError(hashTable);
		fprintf(stderr, "%s: Couldn't delete %s: Last Error: %lu\n",
			argv[0], key, e
		);
		return e;
	}

	return 0;
}
