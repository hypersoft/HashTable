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

#define htVoidExpression (void)
#define htVirtualImmediateFunction(type) static inline type

// Compile Makefile Definitions
const char * HashTableVendor = BUILD_VERSION_VENDOR;
const char * HashTableVersion = BUILD_VERSION_TRIPLET;
const char * HashTableDescription = BUILD_VERSION_DESCRIPTION;
const unsigned long HashTableBuildNumber = BUILD_VERSION_NUMBER;

const char * htErrorTableUninitialized = \
	"The hash table parameter was invalid";

const char * htErrorAllocationFailure = \
	"A data allocation request failed";

const char * htErrorUnsupportedFunction = \
	"The HashTable library does not support the function called";

const char * htErrorZeroLengthKey = \
	"A key specifying zero length was supplied";

const char * htErrorKeyNotFound = \
	"The specified key could not be located";

const char * htErrorInvalidReference = \
	"The specified item reference was invalid";

const char * htErrorNotWritableItem = \
	"The request could not be completed because the item is read only";

const char * htErrorNotConfigurableItem = \
"The request could not be completed because the item is non-configurable";

const char * htErrorNoCallBackHandler = \
"The request could not be completed becuase the call back handler is undefined";

const char * htErrorInvalidTypeRequest = \
	"The request could not be completed because of a type error";

typedef struct sHashTableRecord {
	size_t hitCount;
	HyperVariant key;
	HyperVariant value;
	struct sHashTableRecord * successor;
} sHashTableRecord;

#define htRecordReference(r) varprvti (r->key)
#define htRecordHash(r) varprvti (r->value)
#define htRecordSettings(r) vartype(r->value)

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

#define htGetEventMask(ht, withEvents) (ht->events & withEvents)
#define HashTableSize sizeof(sHashTable)
#define HashTable sHashTable *

#define HT_ERROR_SENTINEL 0

#define htReturnIfTableUninitialized(table)                                    \
if ( ! (table) )                                                               \
    { errno = HT_ERROR_TABLE_UNINITIALIZED; return HT_ERROR_SENTINEL; }

#define htReturnVoidIfTableUninitialized(table)                                \
if ( ! (table) ) { errno = HT_ERROR_TABLE_UNINITIALIZED; return; }

#define htReturnVoidUnsupportedFunction()                                      \
errno = HT_ERROR_UNSUPPORTED_FUNCTION; return

#define htReturnIfZeroLengthKey(length)                                        \
if ( ! (length) )                                                              \
    { errno = HT_ERROR_ZERO_LENGTH_KEY; return HT_ERROR_SENTINEL; }

#define htReturnIfAllocationFailure(pointer, ...)                              \
if ( ! (pointer) ) {                                                           \
    errno = HT_ERROR_ALLOCATION_FAILURE;                                       \
    __VA_ARGS__;                                                               \
    return HT_ERROR_SENTINEL;                                                  \
}

#define htReturnIfInvalidReference(table, reference)                           \
htReturnIfTableUninitialized(table);                                           \
if ( ! reference || ht->itemsMax < reference || ! ht->item[--reference]) {     \
    errno = HT_ERROR_INVALID_REFERENCE; return HT_ERROR_SENTINEL;              \
}

#define htReturnVoidIfInvalidReference(table, reference)                       \
htReturnVoidIfTableUninitialized(table);                                       \
if ( !reference || reference > table->itemsMax || !table->item[--reference]) { \
    errno = HT_ERROR_INVALID_REFERENCE; return;                                \
}

#define htReturnVoidIfNoCallBackHandler(handler)                               \
if (! (handler) ) { errno = HT_ERROR_NO_CALLBACK_HANDLER; return; }

#define htReturnIfNotWritableItem(i)                                           \
if (htRecordSettings(i) & HTI_NON_WRITABLE) {                                  \
    errno = HT_ERROR_NOT_WRITABLE_ITEM; return HT_ERROR_SENTINEL;              \
}

#define htReturnIfNotConfigurableItem(i)                                       \
if (htRecordSettings(i) & HTI_NON_CONFIGURABLE) {                              \
    errno = HT_ERROR_NOT_CONFIGURABLE_ITEM; return HT_ERROR_SENTINEL;          \
}

/* for these inlines: d should be volatile; re: optimization issues */
#define htDblIsNaN(d) (d != d)

#define htDblInfinity(d)                                                       \
(((d == d) && ((d - d) != 0.0)) ? (d < 0.0 ? -1 : 1) : 0)

#define htKeyHash(table, keyLength, realKey)                                   \
( htCreateHash ( keyLength, realKey ) % (table->slotCount) )

#define htRealKeyOrReturn(length, value, hint)                                 \
(hint & HTI_DOUBLE)? &value : ptrVar(value);                                   \
if ( ! length && (hint & HTI_UTF8)) length = strlen(ptrVar(value));            \
if ( ! length ) {                                                              \
    errno = HT_ERROR_ZERO_LENGTH_KEY; return HT_ERROR_SENTINEL;                \
}

#define htCompareRecordToRealKey(e, l, k)                                      \
((e->key == k) || ((varlen(e->key) == l) && (memcmp(e->key, k, l) == 0)))

/* I wouldn't call this on an incomplete record if I were you... */
#define htRecordImpact(r) (                                                    \
	(varimpact(r->key)) + (varimpact(r->value)) + (HashTableRecordSize)        \
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
	errno = HT_ERROR_INVALID_REFERENCE;
	return NULL;
}

static HashTableRecord htCreateRecord
(
	HashTable ht,
	size_t keyLength, double key, HashTableDataFlags keyHint,
	size_t valueLength, double value, HashTableDataFlags valueHint
) {

	HashTableRecord this = calloc(1, HashTableRecordSize);
	htReturnIfAllocationFailure(this, {});

	htReturnIfAllocationFailure(
		this->key = varcreate(keyLength, key, keyHint),
		free(this)
	);

	htReturnIfAllocationFailure(
		this->value = varcreate(valueLength, value, valueHint),
		free(this), varfree(this->key)
	);

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
	htRecordReference(this) = ht->itemsUsed;

	return this;

}

htVirtualImmediateFunction (HashTableItem) htAutoFireItemEvent
(
	htDoc (does not check) HashTable ht,
	htDoc (does not check) HashTableItem reference,
	htDocFires (any registered) HashTableEvent withEvents,
	htDoc (?:= ht->private) void * private
) {
	if (ht->eventHandler) {
		if (htGetEventMask(ht, withEvents) == withEvents) {
			return ht->eventHandler(
				ht, withEvents,
				reference,
				(private) ? private : ht->private
			);
		}
	}
	return reference;
}

HashTable NewHashTable
(
	size_t size,
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

	htVoidExpression htAutoFireItemEvent(ht, 0, HT_EVENT_CONSTRUCTED, NULL);

	return ht;

}

void OptimizeHashTable
(
	HashTable ht,
	size_t slots,
	size_t references
) {
	htReturnVoidIfTableUninitialized(ht);
	/* doesn't do anything yet */
	slots+1;
	htReturnVoidUnsupportedFunction();
}

HashTable DestroyHashTable
(
	HashTable ht
) {
	htReturnIfTableUninitialized(ht);

	htVoidExpression htAutoFireItemEvent(ht, 0, HT_EVENT_DESTRUCTING, NULL);

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
	htReturnVoidIfTableUninitialized(ht);
	ht->events = withEvents;
	if ( eventHandler ) ht->eventHandler = eventHandler;
}

size_t HashTableItemsUsed
(
	HashTable ht
) {
	htReturnIfTableUninitialized(ht);
	return ht->itemsUsed;
}

size_t HashTableItemsTotal
(
	HashTable ht
) {
	htReturnIfTableUninitialized(ht);
	return ht->itemsTotal;
}

size_t HashTableItemsMax
(
	HashTable ht
) {
	htReturnIfTableUninitialized(ht);
	return ht->itemsMax;
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

HashTableItem HashTableHasKey
(
	HashTable ht,
	size_t keyLength,
	double key,
	HashTableDataFlags hint
) {
	htReturnIfTableUninitialized(ht);
	char * realKey = htRealKeyOrReturn(keyLength, key, hint);

	size_t oldError = errno;
	HashTableRecord item = htFindKey(ht, keyLength, realKey);
	if (item) return htRecordReference(item);
	else errno = oldError;
	return HT_ERROR_SENTINEL;
}

bool HashTableHasItem
(
	HashTable ht,
	HashTableItem reference
) {
	htReturnIfTableUninitialized(ht);
	if ( ! (reference) || ht->itemsMax < --reference) return false;
	return (ht->item[reference]) ? true : false;
}

size_t HashTableItemDistribution
(
	HashTable ht,
	HashTableItem reference
) {
	htReturnIfInvalidReference(ht, reference);
	register HashTableRecord child;
	register size_t distribution = 0;
	child = ht->slot[htRecordHash(ht->item[reference])];
	while (child) distribution++, child = child->successor;
	return distribution;
}

size_t HashTableItemHits
(
	HashTable ht,
	HashTableItem reference
) {
	htReturnIfInvalidReference(ht, reference);
	return ht->item[reference]->hitCount;
}

size_t HashTableItemImpact
(
	HashTable ht,
	HashTableItem reference
) {
	htReturnIfInvalidReference(ht, reference);
	return htRecordImpact(ht->item[reference]);
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

HashTableItem HashTablePut
(
	HashTable ht,
	size_t keyLength,
	double key,
	HashTableDataFlags keyHint,
	size_t valueLength,
	double value,
	HashTableDataFlags valueHint
) {

	htReturnIfTableUninitialized(ht);

	char * realKey = htRealKeyOrReturn(keyLength, key, keyHint);

	if (!valueLength) {
		if (valueHint & HTI_UTF8) valueLength = strlen(ptrVar(value));
	}

	size_t index = htKeyHash(ht, keyLength, realKey);

	HashTableRecord root, * parent = NULL, * current = htFindKeyWithParent(
		ht, keyLength, realKey, (root = ht->slot[index]), &parent
	);

	if ( current ) {

		htReturnIfNotWritableItem(current);

		HyperVariant varValue = varcreate(valueLength, value, valueHint);
		htReturnIfAllocationFailure(varValue, {});

		HashTableItem
			currentSelection = htRecordReference(current),
			/* There is already a pair for this key; Update Value? */
			selection = htAutoFireItemEvent(
				ht, currentSelection, HT_EVENT_PUT, varValue
			)
		;

		if (! selection) goto discardNewRecord;

		if (selection == currentSelection) {
			ht->impact -= varimpact(current->value),
			ht->impact += varimpact(varValue);
			varfree(current->value);
			current->value = varValue,
			current->hitCount++;
			return selection;
		}

		discardNewRecord:
			varfree(varValue);

		return selection;

	}

	HashTableRecord thisRecord = htCreateRecord(
		ht,
		keyLength, key, keyHint,
		valueLength, value, valueHint
	);

	if (thisRecord) {

		htRecordHash(thisRecord) = index;

		HashTableItem
			currentSelection = htRecordReference(thisRecord),
			selection = htAutoFireItemEvent(
				ht, currentSelection, HT_EVENT_PUT, thisRecord->value
			)
		;

		if (selection == currentSelection) {
			if ( ! root ) ht->slot[index] = thisRecord;
			else if ( root == current ) root->successor = thisRecord;
			else parent->successor = thisRecord;
			return currentSelection;
		}

		discardThisRecord:
			ht->item[currentSelection - 1] = NULL,
			ht->itemsTotal--, ht->itemsUsed--,
			ht->impact -= htRecordImpact(thisRecord);
			varfree(thisRecord->key), varfree(thisRecord->value);
			free(thisRecord);

		return selection;

	}

	return HT_ERROR_SENTINEL;

}

HashTableItem HashTableGetItemByKey
(
	HashTable ht,
	HashTableData realKey
) {
	htReturnIfTableUninitialized(ht);

	if (! realKey) {
		errno = HT_ERROR_ZERO_LENGTH_KEY; return HT_ERROR_SENTINEL;
	}

	HashTableRecord item = htFindKey(ht, varlen(realKey), (void*) realKey);

	if (! item) return HT_ERROR_SENTINEL;

	HashTableItem
		currentSelection = htRecordReference(item),
		selection = htAutoFireItemEvent(
				ht, currentSelection, HT_EVENT_GET, item->value
		)
	;

	if (selection == currentSelection) item->hitCount++;
	return selection;

}

HashTableItem HashTableGet
(
	HashTable ht,
	size_t keyLength,
	double key,
	HashTableDataFlags hint
) {

	htReturnIfTableUninitialized(ht);
	char * realKey = htRealKeyOrReturn(keyLength, key, hint);

	HashTableRecord item = htFindKey(ht, keyLength, realKey);

	if (! item) return HT_ERROR_SENTINEL;

	HashTableItem
		currentSelection = htRecordReference(item),
		selection = htAutoFireItemEvent(
				ht, currentSelection, HT_EVENT_GET, item->value
		)
	;

	if (selection == currentSelection) item->hitCount++;

	return selection;

}

bool HashTableDeleteItem
(
	HashTable ht,
	HashTableItem reference
) {
	htReturnIfInvalidReference(ht, reference);
	HashTableRecord parent = NULL;
	HashTableRecord item = ht->item[reference];
	htReturnIfNotConfigurableItem(item);

	HashTableItem
		currentSelection = htRecordReference(item),
		selection = htAutoFireItemEvent(
				ht, currentSelection, HT_EVENT_DELETE, item->value
		)
	;

	if (selection == currentSelection) {
		item = htFindKeyWithParent(
			ht, varlen(item->key), item->key,
			ht->slot[htRecordHash(item)],
			&parent
		);

		if (parent) parent->successor = item->successor;
		else {
			ht->slot[htRecordHash(item)] = item->successor;
		}

		ht->item[reference] = NULL, ht->itemsTotal--,
		ht->impact -= htRecordImpact(item);
		varfree(item->key); varfree(item->value); free(item);
		return true;
	}

	return false;

}

HashTableData HashTableItemKey
(
	HashTable ht,
	HashTableItem reference
) {
	htReturnIfInvalidReference(ht, reference);
	return ht->item[reference]->key;
}

HashTableData HashTableItemData
(
	HashTable ht,
	HashTableItem reference
) {
	htReturnIfInvalidReference(ht, reference);
	return ht->item[reference]->value;
}

size_t HashTableDataLength
(
	HashTableData data
) {
	if (data) return varlen(data);
	return HT_ERROR_SENTINEL;
}

HashTableDataFlags HashTableDataType
(
	HashTableData data
) {
	if (data) return (vartype(data) &
		(HTI_INT | HTI_DOUBLE | HTI_POINTER | HTI_BLOCK)
	);
	return HT_ERROR_SENTINEL;
}

HashTableDataFlags HashTableDataSettings
(
	HashTableData data
) {
	if (data) return vartype(data);
	return HT_ERROR_SENTINEL;
}

bool HashTableItemGetEnumerable
(
	HashTable ht,
	HashTableItem reference
) {
	htReturnIfInvalidReference(ht, reference);
	return \
		(htRecordSettings(ht->item[reference]) & HTI_NON_ENUMERABLE) == 0;
}

bool HashTableItemSetEnumerable
(
	HashTable ht,
	HashTableItem reference,
	bool value
) {
	htReturnIfInvalidReference(ht, reference);
	HashTableRecord item = ht->item[reference];
	htReturnIfNotConfigurableItem(item);
	if (! value) {
		htRecordSettings(item) &= ~HTI_NON_ENUMERABLE;
	} else {
		htRecordSettings(item) |= HTI_NON_ENUMERABLE;
	}
	return true;
}

bool HashTableItemGetWritable
(
	HashTable ht,
	HashTableItem reference
) {
	htReturnIfInvalidReference(ht, reference);
	return \
		(htRecordSettings(ht->item[reference]) & HTI_NON_WRITABLE) == 0;
}

bool HashTableItemSetWritable
(
	HashTable ht,
	HashTableItem reference,
	bool value
) {
	htReturnIfInvalidReference(ht, reference);
	HashTableRecord item = ht->item[reference];
	htReturnIfNotConfigurableItem(item);
	if (! value) {
		htRecordSettings(item) &= ~HTI_NON_WRITABLE;
	} else {
		htRecordSettings(item) |= HTI_NON_WRITABLE;
	}
	return true;
}

bool HashTableItemGetConfigurable
(
	HashTable ht,
	HashTableItem reference
) {
	htReturnIfInvalidReference(ht, reference);
	return \
		(htRecordSettings(ht->item[reference]) & HTI_NON_CONFIGURABLE) == 0
	;
}

bool HashTableItemSetConfigurable
(
	HashTable ht,
	HashTableItem reference,
	bool value
) {
	htReturnIfInvalidReference(ht, reference);
	HashTableRecord item = ht->item[reference];
	htReturnIfNotConfigurableItem(item);
	if (! value) {
		htRecordSettings(item) &= ~HTI_NON_CONFIGURABLE;
	} else {
		htRecordSettings(item) |= HTI_NON_CONFIGURABLE;
	}
	return true;
}

void HashTableEnumerate
(
	HashTable ht,
	HashTableEnumerateDirection direction,
	HashTableEnumerationHandler handler,
	void * private
) {
	htReturnVoidIfTableUninitialized(ht);
	htReturnVoidIfNoCallBackHandler(handler);

	size_t index, maximum = ht->itemsMax;
	HashTableRecord item;
	if (! maximum ) return;
	if (direction == HT_ENUMERATE_FORWARD) {
		for (index = 0; index < maximum; index++) {
			item = ht->item[index];
			if (item && ! (htRecordSettings(item) & HTI_NON_ENUMERABLE)) {
				if ( ! handler(ht, direction, index + 1, private) ) break;
			}
		}
	} else {
		index = maximum;
		while (--index) {
			item = ht->item[index];
			if (item && ! (htRecordSettings(item) & HTI_NON_ENUMERABLE)) {
				if ( ! handler(ht, direction, index + 1, private) ) break;
			}
		}
		/* manually enumerate the last item (0) */
		item = ht->item[0];
		if (item && ! (htRecordSettings(item) & HTI_NON_ENUMERABLE)) {
			handler(ht, direction, index + 1, private);
		}
	}
}

void HashTableSortItems
(
	HashTable ht,
	HashTableSortType type,
	HashTableSortDirection direction,
	HashTableSortHandler sortHandler,
	void * private
) {
	htReturnVoidIfTableUninitialized(ht);
	htReturnVoidIfNoCallBackHandler(sortHandler);

	if (ht->itemsMax < 2) return;

	size_t maximum = ht->itemsMax;
	register HashTableItem primary,secondary;
	HashTableItem selection;

	HashTableRecord primaryRecord;
	HashTableRecord secondaryRecord;

	for (primary = 0; primary < maximum; primary++) {
		if (! ht->item[primary] && ! type & HT_SORT_EMPTY_ITEMS) continue;
		for (secondary = primary + 1; secondary < maximum; secondary++) {
			if (! ht->item[secondary] && ! type & HT_SORT_EMPTY_ITEMS) continue;
			selection = sortHandler(
				ht, type, direction, primary+1, secondary+1, private
			);
			if (! selection--) goto BuildItemReferences;
			else if (selection == secondary){
				primaryRecord = ht->item[primary];
				secondaryRecord = ht->item[secondary];
				ht->item[primary] = secondaryRecord;
				ht->item[secondary] = primaryRecord;
			}
		}
	}

BuildItemReferences:
	for(primary = 0; primary < maximum;) {
		primaryRecord = ht->item[primary++];
		if (primaryRecord) htRecordReference(primaryRecord) = primary;
	}

}

void HashTableSortItemHash
(
	HashTable ht,
	HashTableItem reference,
	HashTableSortType type,
	HashTableSortDirection direction,
	HashTableSortHandler sortHandler,
	void * private
) {

	htReturnVoidIfInvalidReference(ht, reference);
	htReturnVoidIfNoCallBackHandler(sortHandler);

	HashTableRecord item = ht->item[reference];
	size_t maximum = 0, index = 0, recordHash = htRecordHash(item);
	item = ht->slot[recordHash];
	while (item) maximum++, item = item->successor;
	HashTableRecord array[maximum+1];
	array[maximum] = NULL;
	item = ht->slot[recordHash];
	while (item) array[index++] = item, item = item->successor;

	HashTableItem primary, secondary;
	for (primary = 0; primary < maximum; primary++) {
		for (secondary = primary + 1; secondary < maximum; secondary++) {
			HashTableItem selection,
			primaryRef = htRecordReference(array[primary]),
			secondaryRef = htRecordReference(array[secondary]);
			selection = sortHandler(
				ht, type, direction, primaryRef--, secondaryRef--, private
			);
			if (--selection == secondaryRef){
				array[secondary] = ht->item[primaryRef];
				array[primary] = ht->item[secondaryRef];
			}
		}
	}

MakeLinkedList:

	for (index = 0; index < maximum;)
		array[index]->successor = array[++index];
	ht->slot[recordHash] = array[0];

}

void HashTableEnumerateItemHash
(
	HashTable ht,
	HashTableItem reference,
	HashTableEnumerateDirection direction,
	HashTableEnumerationHandler handler,
	void * private
) {
	htReturnVoidIfInvalidReference(ht, reference);
	htReturnVoidIfNoCallBackHandler(handler);

	HashTableRecord item = ht->item[reference];
	size_t maximum = 0, index = 0, recordHash = htRecordHash(item);
	item = ht->slot[recordHash];
	while (item) maximum++, item = item->successor;
	HashTableRecord array[maximum];
	item = ht->slot[recordHash];
	while (item) array[index++] = item, item = item->successor;

	if (direction == HT_ENUMERATE_FORWARD) {
		for (index = 0; index < maximum; index++) {
			item = array[index];
			if (! (htRecordSettings(item) & HTI_NON_ENUMERABLE)) {
				if ( ! handler(ht, direction, htRecordReference(item), private))
					break;
			}
		}
	} else {
		index = maximum;
		while (--index) {
			item = array[index];
			if (! (htRecordSettings(item) & HTI_NON_ENUMERABLE)) {
				if ( ! handler(
					ht, direction, htRecordReference(item), private)
				) break;
			}
		}
		/* manually enumerate the last item (0) */
		item = array[0];
		if (! (htRecordSettings(item) & HTI_NON_ENUMERABLE)) {
			handler(ht, direction, htRecordReference(item), private);
		}
	}
}

const char * HashTableErrorMessage
(
	void
) {
	size_t err = errno;
	if (err == HT_ERROR_TABLE_UNINITIALIZED)
		return htErrorTableUninitialized;
	else if (err == HT_ERROR_ALLOCATION_FAILURE)
		return htErrorAllocationFailure;
	else if (err == HT_ERROR_UNSUPPORTED_FUNCTION)
		return htErrorUnsupportedFunction;
	else if (err == HT_ERROR_ZERO_LENGTH_KEY)
		return htErrorZeroLengthKey;
	else if (err == HT_ERROR_KEY_NOT_FOUND)
		return htErrorKeyNotFound;
	else if (err == HT_ERROR_INVALID_REFERENCE)
		return htErrorInvalidReference;
	else if (err == HT_ERROR_NOT_WRITABLE_ITEM)
		return htErrorNotWritableItem;
	else if (err == HT_ERROR_NOT_CONFIGURABLE_ITEM)
		return htErrorNotConfigurableItem;
	else if (err == HT_ERROR_NO_CALLBACK_HANDLER)
		return htErrorNoCallBackHandler;
	else if (err == HT_ERROR_INVALID_TYPE_REQUEST)
		return htErrorInvalidTypeRequest;
	else return HT_ERROR_SENTINEL;
}
