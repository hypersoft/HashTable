/*

 Copyright (c) 2014, Triston J. Taylor <pc.wiz.tt@gmail.com>
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
	list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice,
	this list of conditions and the following disclaimer in the documentation
	and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

/* Special Thanks to: https://gist.github.com/tonious/1377667 */

#include <string.h>

/*
 * This value will be used to determine how many slots to allocate.
 */
#ifndef HT_DEFAULT_RESERVE_SLOTS
#define HT_DEFAULT_RESERVE_SLOTS 32L
#endif

// Compile Makefile Definitions
const char * HashTableVendor = HT_VERSION_VENDOR;
const char * HashTableVersion = HT_VERSION_TRIPLET;
const char * HashTableDescription = HT_VERSION_DESCRIPTION;
const long HashTableBuildNumber = HT_VERSION_BUILDNO;

struct entry_s {
	size_t hitCount;
	void * key;
	size_t keyLength;
	void * value;
	size_t valueLength;
	struct entry_s * successor;
};

typedef struct entry_s hashtable_entry_t;

struct hashtable_s {
	size_t size;
	size_t entries;
	struct entry_s ** entry;
	int lastError;
	void * userData;
};

typedef struct hashtable_s hashtable_t;

#define HashTable_c
	#include "HashTable.h"
#undef HashTable_c

hashtable_t *
NewHashTable ( size_t size, void * userData )
{
	/* Allocate the table itself. */
	hashtable_t * hashTable = malloc ( sizeof ( hashtable_t ) );

	if ( !hashTable ) return NULL;

	hashTable->size = ( size ) ? size : HT_DEFAULT_RESERVE_SLOTS,
	hashTable->userData = userData;

	/* Allocate pointers to the head nodes. */
	if ( ( hashTable->entry = calloc ( size, sizeof (void* ) )
	) == NULL ) {
		free ( hashTable );
		return NULL;
	}

	/* Return Dataset */
	return hashTable;

}

bool
DestroyHashTable (hashtable_t * hashTable) {
	if (!hashTable) return false; // can't destroy something that does not exist
	size_t slot, limit = hashTable->size;
	for (slot = 0; (slot < limit) && hashTable->entries; slot++) {
		hashtable_entry_t * parent = hashTable->entry[slot], * child;
		while (parent) {
			child = parent->successor;
			/* free up the resources */
			free ( parent->value ), free ( parent->key ),
			free ( parent );
			hashTable->entries--;
			parent = child;
		}
	}
	free(hashTable->entry),	free(hashTable);
	return true;
}

/* INTERNAL: Hash a binary input */
static
size_t
hashBinaryInput ( char * input, size_t length )
{
	size_t hash, i;
	/* Use: Jenkins' "One At a Time Hash" === Perl "Like" Hashing */
	for ( hash = i = 0; i < length; ++i ) {
		hash += input[i], hash += ( hash << 10 ), hash ^= ( hash >> 6 );
	}
	hash += ( hash << 3 ), hash ^= ( hash >> 11 ), hash += ( hash << 15 );

	return hash;
}

/* INTERNAL: distribute a hash over the number of available slots */
#define HashTableIndexOf(table, key, length) \
( hashBinaryInput ( key, length ) % (table->size) )

/* INTERNAL: Create a key-value pair. */
static
hashtable_entry_t *
hashTableCreateEntry ( )
{
	hashtable_entry_t *thisItem = calloc ( 1, sizeof ( hashtable_entry_t ) );
	return thisItem;
}

bool
HashTablePut ( hashtable_t * hashTable,
char * key, void * value, size_t valueLength, size_t padding, bool overwrite
)
{

	if ( !hashTable ) false;

	if ( !key ) {
		hashTable->lastError = HT_OP_ERROR_NO_KEY;
		return false;
	}

	size_t keyLength = strlen(key);

	if ( !keyLength ) {
		hashTable->lastError = HT_OP_ERROR_NO_KEY_LENGTH;
		return 0;
	}

	if ( !value ) {
		hashTable->lastError = HT_OP_ERROR_NO_VALUE;
		return false;
	}

	if (!valueLength) valueLength = strlen(value);

	size_t index = HashTableIndexOf ( hashTable, key, keyLength );

	hashtable_entry_t * primaryItem, * currentItem, * thisItem,
	*previousItem = NULL;

	currentItem = primaryItem = hashTable->entry[index];

	while ( currentItem ) {
		if ( currentItem->keyLength == keyLength ) {
			if ( !memcmp ( key, currentItem->key, keyLength ) ) {
				if ( !overwrite ) {
					hashTable->lastError = HT_OP_ERROR_KEY_EXISTS;
					return false;
				}
				/* There is already a pair for this key; Update Value */
				free ( currentItem->value );
				currentItem->hitCount++;
				currentItem->value = memcpy (
				malloc ( valueLength ), value, valueLength
				);
				currentItem->valueLength = valueLength;
				return true;
			}
		}
		previousItem = currentItem, currentItem = currentItem->successor;
	}

	thisItem = hashTableCreateEntry ( );
	thisItem->key = memcpy ( malloc ( keyLength ), key, keyLength );
	thisItem->keyLength = keyLength;
	thisItem->valueLength = valueLength,
	thisItem->successor = NULL;

	void * storage = memcpy ( malloc ( valueLength + padding ), value, valueLength );
	thisItem->value = storage;
	if (padding) storage += valueLength, memset(storage, 0, padding);

	if ( !primaryItem ) {
		hashTable->entry[index] = thisItem;
	} else if ( primaryItem == currentItem ) primaryItem->successor = thisItem;
	else previousItem->successor = thisItem;

	hashTable->entries++;
	return ( thisItem != NULL );

}

bool
HashTablePutUTF8(hashtable_t * hashTable, char * key, char * value, bool overwrite) {
	return HashTablePut ( hashTable, key, value, 0, 1, overwrite);
}

bool
HashTablePutInt(
	hashtable_t * hashTable, char * key, int value, bool overwrite
) {
	return HashTablePut(hashTable, key, &value, sizeof(int), 0, overwrite);
}

bool
HashTablePutPointer(
	hashtable_t * hashTable, char * key, void * value, bool overwrite
) {
	return HashTablePut(hashTable, key, &value, sizeof(void *), 0, overwrite);
}

bool
HashTablePutDouble(
	hashtable_t * hashTable, char * key, double value, bool overwrite
) {
	return HashTablePut(hashTable, key, &value, sizeof(double), 0, overwrite);
}

const void *
HashTableGet (
hashtable_t * hashTable, char * key, size_t * valueLength
)
{
	if ( !hashTable ) return NULL;

	if ( !key ) {
		hashTable->lastError = HT_OP_ERROR_NO_KEY;
		return NULL;
	}

	size_t keyLength = strlen(key);

	if ( !keyLength ) {
		hashTable->lastError = HT_OP_ERROR_NO_KEY_LENGTH;
		return NULL;
	}

	/* Retrieve Query Index */
	size_t list = HashTableIndexOf ( hashTable, key, keyLength );

	/* Think Outside The Box */
	hashtable_entry_t * thisItem = hashTable->entry [list];

	/* Locate Target */
	while ( thisItem ) {
		if ( thisItem->keyLength == keyLength &&
		!memcmp ( key, thisItem->key, keyLength )
		) /* Target Located */ break;
		else thisItem = thisItem->successor;
	}

	if ( !thisItem ) {
		hashTable->lastError = HT_OP_ERROR_NO_VALUE;
		return 0;
	}

	thisItem->hitCount++;

	/* Return Query Result */
	if (valueLength) *valueLength = thisItem->valueLength;
	return thisItem->value;

}

int
HashTableGetInt(hashtable_t * hashTable, char * key) {
	const int *value = HashTableGet(hashTable, key, NULL);
	if (value) return *value;
	return 0;
}

void *
HashTableGetPointer(hashtable_t * hashTable, char * key) {
	void * const * value = HashTableGet(hashTable, key, NULL);
	if (value) return *value;
	return 0;
}

double
HashTableGetDouble(hashtable_t * hashTable, char * key) {
	const double *value = HashTableGet(hashTable, key, NULL);
	if (value) return *value;
	return 0;
}

bool
HashTableDelete ( hashtable_t * hashTable, char * key )
{
	if ( !hashTable ) return false;

	if ( !key ) {
		hashTable->lastError = HT_OP_ERROR_NO_KEY;
		return false;
	}

	size_t keyLength = strlen(key);

	if ( !keyLength ) {
		hashTable->lastError = HT_OP_ERROR_NO_KEY_LENGTH;
		return false;
	}

	/* Retrieve Query Index */
	size_t list = HashTableIndexOf ( hashTable, key, keyLength );

	/* Think Outside The Box */
	hashtable_entry_t * previousItem, * currentItem = hashTable->entry[list];

	previousItem = currentItem;

	/* Locate Target */
	while ( currentItem ) {
		if ( currentItem->keyLength == keyLength &&
		!memcmp ( key, currentItem->key, keyLength )
		) /* Target Located */ break;
		else previousItem = currentItem, currentItem = currentItem->successor;
	}

	if ( !currentItem ) {
		hashTable->lastError = HT_OP_ERROR_NO_VALUE;
		return false;
	}

	if ( previousItem == currentItem ) { /* delete the list head */
		hashTable->entry[ list ] = NULL;
	} else { /* remove entry from the list */
		previousItem->successor = currentItem->successor;
	}

	/* free up the resources */
	free ( currentItem->value ), free ( currentItem->key ),
	free ( currentItem );

	hashTable->entries--;

	return true;

}

size_t
HashTableGetEntryCount ( hashtable_t * hashTable )
{
	if ( hashTable ) return hashTable->entries;
	return 0;
}

enum HashTableErrorCode
HashTableGetLastError ( hashtable_t * hashTable )
{
	if ( !hashTable ) return HT_OP_ERROR_NO_HASH_TABLE;
	enum HashTableErrorCode result = hashTable->lastError;
	if ( result ) hashTable->lastError = HT_OP_ERROR_NONE;
	return result;
}

void *
HashTableGetPrivate ( hashtable_t * hashTable )
{
	if ( !hashTable ) return NULL;
	return hashTable->userData;
}

bool
HashTablePutPrivate ( hashtable_t * hashTable, void * userData )
{
	if ( !hashTable ) return false;
	hashTable->userData = userData;
	return true;
}

bool
HashTableHasEntry ( hashtable_t * hashTable, char * key )
{
	if ( !hashTable ) return false;

	if ( !key ) {
		hashTable->lastError = HT_OP_ERROR_NO_KEY;
		return false;
	}

	size_t keyLength = strlen(key);

	if ( !keyLength ) {
		hashTable->lastError = HT_OP_ERROR_NO_KEY_LENGTH;
		return false;
	}

	/* Think Outside The Box */
	hashtable_entry_t * search = hashTable->entry[
		HashTableIndexOf ( hashTable, key, keyLength )
	];

	/* Locate Target */
	while ( search ) {
		if ( search->keyLength == keyLength &&
		!memcmp ( key, search->key, keyLength )
		) /* Target Located */ break;
		else search = search->successor;
	}

	if ( !search ) {
		return false;
	}

	return true;

}