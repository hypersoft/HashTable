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
#define HT_RESERVE_SLOTS (sizeof(size_t) << 3L)
#endif

#ifndef HT_RESERVE_ITEMS
#define HT_RESERVE_ITEMS 8L
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

#define htReturnIfTableUninitialized(t) \
	if ( ! ht ) { errno = HT_ERROR_TABLE_UNINITIALIZED; return 0; }

#define htVoidIfTableUninitialized(t) \
	if ( ! ht ) { errno = HT_ERROR_TABLE_UNINITIALIZED; return; }

#define htVoidUnsupportedFunction() \
	errno = HT_ERROR_UNSUPPORTED_FUNCTION; return

#define htReturnIfZeroLengthKey(l) \
	if ( ! l ) { errno = HT_ERROR_ZERO_LENGTH_KEY; return 0; }

#define htReturnIfAllocationFailure(p, ...) \
	if ( ! p ) { errno = HT_ERROR_ALLOCATION_FAILURE; __VA_ARGS__; return 0; }

#define htReturnIfInvalidReference(ht, r) \
	if ( ! r || ht->itemsMax < r) { \
		errno = HT_ERROR_INVALID_REFERENCE; return 0; \
}

#define htReturnIfKeyNotFound(i) \
	if ( ! i ) { errno = HT_ERROR_KEY_NOT_FOUND; return 0; }

#define htReturnIfNotWritableItem(i) if (vartype(i->value) & HTR_NON_WRITABLE) { \
	errno = HT_ERROR_NOT_WRITABLE_ITEM; return 0; \
}

#define htReturnIfNotConfigurableItem(i) \
if (vartype(i->value) & HTR_NON_CONFIGURABLE) { \
	errno = HT_ERROR_NOT_CONFIGURABLE_ITEM; return 0; \
}

#define htReturnIfItemNotFound(i) \
	if ( ! i ) { errno = HT_ERROR_INVALID_REFERENCE; return 0; }

#define htItemAccess(ht, r, label, expr) \
	htReturnIfTableUninitialized(ht); \
	htReturnIfInvalidReference(ht, r); \
	HashTableRecord label = ht->item[r]; \
	htReturnIfItemNotFound(label);	\
	if (label) return expr; \
	else errno = HT_ERROR_INVALID_REFERENCE; \
	return 0

/* for these inlines: d should be volatile; re: optimization issues */
#define htDblIsNaN(d) (d != d)
#define htDblInfinity(d) \
	(((d == d) && ((d - d) != 0.0)) ? (d < 0.0 ? -1 : 1) : 0)

#define htKeyHash(table, keyLength, realKey) \
	( htCreateHash ( keyLength, realKey ) % (table->slotCount) )

/* scan UTF strings for length if not supplied, and return FAIL if empty */
#define __htKeyLength(l, v, h) if ( ! l ) { \
	if (h & HTR_UTF8) l = strlen(ptrVar(v)); \
}

#define htRealKey(l, v, h) (h & HTR_DOUBLE)?&v:ptrVar(v); __htKeyLength(l, v, h)

#define htCreateRecord() calloc(1, HashTableRecordSize)

#define htRecordReference(e) ((e) ? e->hitCount++,  : 0)

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
	htReturnIfAllocationFailure(ht, {});

	if (!size) size = HT_RESERVE_SLOTS;

	ht->slotCount = size, ht->events = withEvents,
	ht->eventHandler = eventHandler,
	ht->private = private,
	ht->slot = calloc(size, sizeof(void*));
	htReturnIfAllocationFailure(ht->slot, free(ht));

	ht->impact = HashTableSize + (sizeof(void*) * (size));

	if (withEvents & HT_EVENT_CONSTRUCTED && eventHandler)
		eventHandler(ht, HT_EVENT_CONSTRUCTED, 0, 0, private);

	return ht;

}

void OptimizeHashTable
(
	HashTable ht,
	size_t slots
) {
	htVoidIfTableUninitialized(ht);
	/* doesn't do anything yet */
	slots+1;
	htVoidUnsupportedFunction();
}

HashTable DestroyHashTable
(
	HashTable ht
) {
	htReturnIfTableUninitialized(ht);
	size_t item = 0, length = ht->itemsMax; HashTableRecord target = NULL;
	for (item = 0; item < length; item++) {
		target = ht->item[item];
		if (target) {
			varfree(target->key); varfree(target->value); free(target);
		}
	}
	free(ht->item), free(ht->slot), free(ht);
	return NULL;
}

void HashTableRegisterEvents
(
	HashTable ht,
	HashTableEvent withEvents,
	HashTableEventHandler eventHandler
) {
	htVoidIfTableUninitialized(ht);
	ht->events = withEvents;
	if ( eventHandler ) ht->eventHandler = eventHandler;
}

size_t HashTableSlotCount
(
	HashTable ht
) {
	htReturnIfTableUninitialized(ht);
	return ht->slotCount;
}

size_t HashTableSlotsUsed
(
	HashTable ht
) {
	htReturnIfTableUninitialized(ht);
	size_t used = 0, index, max = ht->slotCount;
	for (index = 0; index < max; index++) if (ht->slot[index]) used++;
	return used;
}

double HashTableLoadFactor
(
	HashTable ht
) {
	htReturnIfTableUninitialized(ht);
	volatile double factor = (((double)ht->itemsTotal)/((double)ht->slotCount));
	return htDblInfinity(factor) ? 0 : factor;
}

size_t HashTableImpact
(
	HashTable ht
) {
	htReturnIfTableUninitialized(ht);
	return ht->impact;
}

size_t HashTableDistribution
(
	HashTable ht,
	size_t keyLength,
	double key,
	HashTableItemFlags hint
) {
	htReturnIfTableUninitialized(ht);
	void * realKey = htRealKey(keyLength, key, hint);
	register HashTableRecord child;
	register size_t distribution = 0;
	child = ht->slot[htKeyHash(ht, keyLength, realKey)];
	while (child) distribution++, child = child->successor;
	return distribution;
}

size_t HashTableItemHits
(
	HashTable ht,
	HashTableItem reference
) {
	htItemAccess(ht, reference, item, item->hitCount);
}

size_t HashTableItemLength
(
	HashTable ht,
	HashTableItem reference
) {
	htItemAccess(ht, reference, item, varlen(item->value));
}

size_t HashTableItemImpact
(
	HashTable ht,
	HashTableItem reference
) {
	htItemAccess(ht, reference, item, htRecordImpact(item));
}

bool HashTablePutPrivate
(
	HashTable ht,
	void * private
) {
	htReturnIfTableUninitialized(ht);
	ht->private = private; return true;
}

void * HashTableGetPrivate
(
	HashTable ht
) {
	htReturnIfTableUninitialized(ht);
	return ht->private;
}

static inline HashTableItem htPutRecordItem
(
	HashTable ht, HashTableRecord this
) {

	if (ht->itemsMax == ht->itemsUsed) {
		HashTableRecordItems list = calloc(
			sizeof(void*),
			ht->itemsUsed + HT_RESERVE_ITEMS
		);
		htReturnIfAllocationFailure(list, {});
		if (ht->item) {
			memcpy(list, ht->item, sizeof(void*) * ht->itemsUsed);
			free(ht->item);
		}
		ht->impact += sizeof(void *) * HT_RESERVE_ITEMS;
		ht->itemsMax += HT_RESERVE_ITEMS;
		ht->item = list;
	}

	ht->item[ht->itemsUsed++] = this;
	ht->itemsTotal++, ht->impact += htRecordImpact(this);
	varprvti(this->key) = ht->itemsUsed;
	return ht->itemsUsed;

}

HashTableItem HashTablePut
(
	HashTable ht,
	size_t keyLength,
	double key,
	HashTableItemFlags keyHint,
	size_t valueLength,
	double value,
	HashTableItemFlags valueHint
) {

	htReturnIfTableUninitialized(ht);

	char * realKey = htRealKey(keyLength, key, keyHint);
	htReturnIfZeroLengthKey(keyLength);

	if (!valueLength) {
		if (valueHint & HTR_UTF8) valueLength = strlen(ptrVar(value));
	}

	size_t index = htKeyHash(ht, keyLength, realKey);

	HashTableRecord root, * parent = NULL, * current = htFindKeyWithParent(
		ht, keyLength, realKey, (root = ht->slot[index]), &parent
	);

	HyperVariant varKey, varValue;

	if ( current ) { /* There is already a pair for this key; Update Value */
		htReturnIfNotWritableItem(current);
		htReturnIfAllocationFailure(
			(varValue = varcreate(valueLength, value, valueHint)), {}
		) else varprvti(varValue) = index;
		/* TODO: HT_EVENT_OVERWRITE !*/
		ht->impact -= varimpact(current->value),
		ht->impact += varimpact(varValue);
		varfree(current->value);
		current->value = varValue, current->hitCount++;
		return varprvti (current->key);
	}

	HashTableRecord this = htCreateRecord();
	htReturnIfAllocationFailure(this, {});

	htReturnIfAllocationFailure(
		(varKey = varcreate(keyLength, key, keyHint)),
		free(this)
	)   else this->key = varKey;

	htReturnIfAllocationFailure(
		(varValue = varcreate(valueLength, value, valueHint)),
		free(this), varfree(varKey)
	)   else this->value = varValue, varprvti(varValue) = index;

	if (htPutRecordItem(ht, this)) {
		/* TODO: HT_EVENT_PUT !*/
		if ( ! root ) ht->slot[index] = this;
		else if ( root == current ) root->successor = this;
		else parent->successor = this;
		return ht->itemsUsed;
	}

	return 0;

}

HashTableItem HashTableGet
(
	HashTable ht,
	size_t keyLength,
	double key,
	HashTableItemFlags hint
) {
	htReturnIfTableUninitialized(ht);
	char * realKey = htRealKey(keyLength, key, hint);
	htReturnIfZeroLengthKey(keyLength);
	HashTableRecord item = htFindKey(ht, keyLength, realKey);
	htReturnIfKeyNotFound(item);
	return varprvti(item->key);
}

bool HashTableDeleteItem
(
	HashTable ht,
	HashTableItem reference
) {

	htReturnIfTableUninitialized(ht);
	htReturnIfInvalidReference(ht, reference);
	HashTableRecord parent = NULL;
	HashTableRecord item = ht->item[reference];
	htReturnIfItemNotFound(item);
	htReturnIfNotConfigurableItem(item);

	item = htFindKeyWithParent(
		ht, varlen(item->key), item->key,
		ht->slot[varprvti(item->value)],
		&parent
	);

	if (parent) parent->successor = item->successor;
	else {
		ht->slot[varprvti(item->value)] = item->successor;
	}

	ht->item[reference] = NULL, ht->itemsTotal--,
	ht->impact -= htRecordImpact(item);
	varfree(item->key); varfree(item->value); free(item);
	return true;

}

HashTableItemFlags HashTableItemGetFlags
(
	HashTable ht,
	HashTableItem reference
) {
	htItemAccess(ht, reference, item, vartype(item->value));
}

bool HashTableItemPutFlags
(
	HashTable ht,
	HashTableItem reference,
	HashTableItemFlags settings
) {
	htReturnIfTableUninitialized(ht);
	htReturnIfInvalidReference(ht, reference);
	HashTableRecord item = ht->item[reference];
	htReturnIfItemNotFound(item);
	htReturnIfNotConfigurableItem(item);
	/*
	 we don't particularly use these flags, but to avoid future collision
	 ignore setting them...
	 */
	settings |= ~(HTR_INT | HTR_DOUBLE | HTR_POINTER | HTR_UTF8 | HTR_BLOCK);
	vartype(item->value) |= settings;
	return true;
}

const void * HashTableItemGetValue
(
	HashTable ht,
	HashTableItem reference
) {
	htItemAccess(ht, reference, item, item->value);
}
