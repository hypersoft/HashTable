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

#define HashTable_c
#include "HashTable.h"
#undef HashTable_c

#include "HyperVariant.h"

/*
 * This value will be used to determine how many slots to allocate.
 */
#ifndef HT_RESERVE_SLOTS
#define HT_RESERVE_SLOTS (sizeof(size_t) << 3)
#endif

// Compile Makefile Definitions
const char * HashTableVendor = BUILD_VERSION_VENDOR;
const char * HashTableVersion = BUILD_VERSION_TRIPLET;
const char * HashTableDescription = BUILD_VERSION_DESCRIPTION;
const long HashTableBuildNumber = BUILD_VERSION_NUMBER;

typedef struct sHashTableRecord {
	size_t hitCount;
	HyperVariant key;
	HyperVariant value;
	struct sHashTableRecord * successor;
} sHashTableRecord;

#define HashTableRecordSize sizeof(sHashTableRecord)
#define HashTableRecord sHashTableRecord *
#define HashTableRecordList sHashTableRecord **
#define HashTableRecordItems sHashTableRecord **

typedef struct sHashTable {
	HashTableRecordItems item;
	size_t itemsUsed;
	size_t itemsTotal;
	size_t itemsMax;
	HashTableRecordList slot;
	size_t slotCount;
	HashTableEventHandler eventHandler;
	HashTableEvent events;
	size_t impact;
	void * private;
} sHashTable;

#define HashTableSize sizeof(sHashTable)
#define HashTable sHashTable *

/* for these inlines: d should be volatile; re: optimization issues */
#define htDblIsNaN(d) (d != d)
#define htDblInfinity(d) \
	(((d == d) && ((d - d) != 0.0)) ? (d < 0.0 ? -1 : 1) : 0)


#define htKeyHash(table, keyLength, realKey) \
	( htCreateHash ( keyLength, realKey ) % (table->slotCount) )

/* scan UTF strings for length if not supplied, and return FAIL if empty */
#define __htKeyLength(l, v, h) if ( ! l ) { \
	if (h & HTR_UTF8) l = strlen(ptrVar(v)); \
	if ( ! l ) return 0; \
}

#define htRealKey(l, v, h) (h & HTR_DOUBLE)?&v:ptrVar(v); __htKeyLength(l, v, h)

#define htCreateRecord() calloc(1, HashTableRecordSize)

#define htRecordReference(e) ((e) ? e->hitCount++, varprvti(e->key) : 0)

#define htCompareRecordToRealKey(e, l, k) \
	(varlen(e->key) == l) && (memcmp(e->key, k, l) == 0)

/* I wouldn't call this on an incomplete record if I were you... */
#define htRecordImpact(r) ( \
	(varimpact(r->key)) + (varimpact(r->value)) + (HashTableRecordSize) \
)

/* Jenkins' "One At a Time Hash" === Perl "Like" Hashing */
inline static size_t htCreateHash (size_t length, char * realKey)
{
	size_t hash, i;
	for ( hash = i = 0; i < length; ++i ) hash += realKey[i],
		hash += ( hash << 10 ), hash ^= ( hash >> 6 );
	hash += ( hash << 3 ), hash ^= ( hash >> 11 ), hash += ( hash << 15 );
	return hash;
}

inline static HashTableRecord htFindKeyWithParent (
	HashTable ht, size_t keyLength, void * realKey,
	HashTableRecord primary, HashTableRecord * parent
) {
	while ( primary ) {
		if (htCompareRecordToRealKey(primary, keyLength, realKey))
			return primary;
		*parent = primary; primary = primary->successor;
	}
	return NULL;
}

inline static HashTableRecord htFindKey (
	HashTable ht, size_t keyLength, void * realKey
) {
	HashTableRecord primary = ht->slot[htKeyHash(ht, keyLength, realKey)];
	while ( primary ) {
		if (htCompareRecordToRealKey(primary, keyLength, realKey))
			return primary;
		primary = primary->successor;
	}
	return NULL;
}

HashTable NewHashTable
(size_t size,
	HashTableEvent withEvents,
	HashTableEventHandler eventHandler,
	void * private
) {
	HashTable ht = calloc(1, HashTableSize);

	if (!ht) return NULL;

	if (!size) size = HT_RESERVE_SLOTS;

	ht->slotCount = size, ht->events = withEvents,
	ht->eventHandler = eventHandler,
	ht->private = private,
	ht->slot = calloc(size, sizeof(void*));

	if ( !ht->slot ) { free(ht); return NULL; }

	ht->item = calloc(8, sizeof(void*));
	if ( !ht->item ) { free(ht), free(ht->slot); return NULL; }

	ht->itemsMax = 8,
	ht->impact = HashTableSize + (sizeof(void*) * (size + 8));

	if (withEvents & HT_EVENT_CONSTRUCTED && eventHandler)
		eventHandler(ht, HT_EVENT_CONSTRUCTED, 0, 0, private);

	return ht;

}

void OptimizeHashTable
(
	HashTable ht
) {
	/* doesn't do anything yet */
}

HashTable DestroyHashTable
(
	HashTable ht
) {
	if (!ht) return NULL;
	size_t item, length = ht->itemsMax; HashTableRecord target = NULL;
	for (item = 0; item < length; item++) {
		if ((target = ht->item[item])) {
			varfree(target->key); varfree(target->value); free(target);
		}
	}
	free(ht->item), free(ht->slot), free(ht);
	return NULL;
}

size_t HashTableSlotCount
(
	HashTable ht
) {
	if (!ht) return 0;
	return ht->slotCount;
}

size_t HashTableSlotsUsed
(
	HashTable ht
) {
	if (!ht) return 0;
	size_t used = 0, index, max = ht->slotCount;
	for (index = 0; index < max; index++) if (ht->slot[index]) used++;
	return used;
}

double HashTableLoadFactor
(
	HashTable ht
) {
	if (!ht) return (double) 0.0;
	volatile double factor = (((double)ht->itemsTotal)/((double)ht->slotCount));
	return htDblInfinity(factor) ? 0 : factor;
}

size_t HashTableImpact
(
	HashTable ht
) {
	if (!ht) return 0;
	return ht->impact;
}

size_t HashTableDistribution
(
	HashTable ht,
	size_t keyLength,
	double key,
	HashTableRecordFlags hint
) {
	if (!ht) return 0;
	void * realKey = htRealKey(keyLength, key, hint);
	register HashTableRecord child;
	register size_t distribution = 0;
	child = ht->slot[htKeyHash(ht, keyLength, realKey)];
	while (child) distribution++, child = child->successor;
	return distribution;
}

size_t HashTableRecordHits
(
	HashTable ht,
	HashTableItem reference
) {
	if (!ht) return 0;
	if (ht->itemsMax > --reference) {
		HashTableRecord item = ht->item[reference];
		if (item) return item->hitCount;
		return 0;
	}
	return 0;
}

bool HashTablePutPrivate
(
	HashTable ht,
	void * private
) {
	if (!ht) return false;
	ht->private = private; return true;
}

void * HashTableGetPrivate
(
	HashTable ht
) {
	if (!ht) return NULL;
	return ht->private;
}

HashTableItem HashTablePut
(
	HashTable ht,
	size_t keyLength,
	double key,
	HashTableRecordFlags keyHint,
	size_t valueLength,
	double value,
	HashTableRecordFlags valueHint
) {

	if (!ht) return 0;

	HyperVariant varKey, varValue;

	char * realKey = htRealKey(keyLength, key, keyHint);

	if (!valueLength) {
		if (valueHint & HTR_UTF8) valueLength = strlen(ptrVar(value));
	}

	size_t index = htKeyHash(ht, keyLength, realKey);

	HashTableRecord root, * parent = NULL, * current = htFindKeyWithParent(
		ht, keyLength, realKey, (root = ht->slot[index]), &parent
	);

	if ( current ) { /* There is already a pair for this key; Update Value */
		if (!(varValue = varcreate(valueLength, value, valueHint))) return 0;
		ht->impact += varimpact(current->value),
		ht->impact += varimpact(varValue);
		varfree(current->value);
		current->value = varValue, current->hitCount++;
		return varprvti (current->key);
	}

	HashTableRecord this = htCreateRecord();
	if ( ! (varKey = varcreate(keyLength, key, keyHint))) {
		free(this); return 0;
	} else 	this->key = varKey;

	if ( ! (varValue = varcreate(valueLength, value, valueHint))) {
		free(this), varfree(varKey); return 0;
	} else this->value = varValue;

	ht->impact += htRecordImpact(this), ht->itemsTotal++;

	ht->item[ht->itemsUsed++] = this;
	varprvti(varKey) = ht->itemsUsed;

	/* TODO: HT_EVENT_PUT !*/

	if ( ! root ) ht->slot[index] = this;
	else if ( root == current ) root->successor = this;
	else parent->successor = this;

	return ht->itemsUsed;

}
#include <stdio.h>

HashTableItem HashTableGet
(
	HashTable ht,
	size_t keyLength,
	double key,
	HashTableRecordFlags hint
) {
	char * realKey = htRealKey(keyLength, key, hint);
	HashTableRecord item = htFindKey(ht, keyLength, realKey);
	return htRecordReference(item);
}
