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

#ifndef HashTable_h

#define HashTable_h

#define htDoc(...) /* this thing */
#define htDocFires(...) /* these events */
#define htDocThrows(...) /* these errors */

#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <errno.h>

typedef size_t HashTableItem;

#define HashTableBitFlag(n) (1 << (n))

typedef enum eHashTableError {
	HT_ERROR_TABLE_UNINITIALIZED = EOWNERDEAD,
	HT_ERROR_ALLOCATION_FAILURE = ENOMEM,
	HT_ERROR_UNSUPPORTED_FUNCTION = ENOTSUP,
	HT_ERROR_ZERO_LENGTH_KEY = EDESTADDRREQ,
	HT_ERROR_KEY_NOT_FOUND = EADDRNOTAVAIL,
	HT_ERROR_INVALID_REFERENCE = EFAULT,
	HT_ERROR_NOT_CONFIGURABLE_ITEM = EACCES,
	HT_ERROR_NOT_WRITABLE_ITEM = EROFS,
	HT_ERROR_NO_CALLBACK_HANDLER = ENOEXEC,
	HT_ERROR_INVALID_TYPE_REQUEST = EINVAL
} HashTableError;

typedef enum eHashTableEvent {
	HT_EVENT_CONSTRUCTED     = HashTableBitFlag(1),
	HT_EVENT_PUT             = HashTableBitFlag(2),
	HT_EVENT_GET             = HashTableBitFlag(3),
	HT_EVENT_DELETE          = HashTableBitFlag(4),
	HT_EVENT_DESTRUCTING     = HashTableBitFlag(5)
} HashTableEvent;

typedef HashTableItem (*HashTableEventHandler)
(
	void * hashTable,
	HashTableEvent event,
	HashTableItem reference,
	void * private
);

typedef enum eHashTableEnumerateDirection {
	HT_ENUMERATE_FORWARD = 0,
	HT_ENUMERATE_REVERSE = 1
} HashTableEnumerateDirection;

typedef bool (*HashTableEnumerationHandler)
(
	void * hashTable,
	HashTableEnumerateDirection direction,
	HashTableItem item,
	void * private
);

typedef enum eHashTableSortType {
	HT_SORT_NUMERIC     = HashTableBitFlag(1),
	HT_SORT_ALPHA       = HashTableBitFlag(2),
	HT_SORT_EMPTY_ITEMS = HashTableBitFlag(3)
} HashTableSortType;

typedef enum eHashTableSortDirection {
	HT_SORT_ASCENDING  = HashTableBitFlag(1),
	HT_SORT_DESCENDING = HashTableBitFlag(2)
} HashTableSortDirection;

typedef HashTableItem (*HashTableSortHandler)
(
	void * hashTable,
	HashTableSortType type,
	HashTableSortDirection direction,
	HashTableItem primary,
	HashTableItem secondary,
	void * private
);

typedef const void * HashTableData;

typedef enum eHashTableDataFlags {
	HTI_NUMBER = 1 << 1,
	HTI_DOUBLE = 1 << 2,
	HTI_POINTER = 1 << 3,
	HTI_BLOCK = 1 << 4,
	HTI_UTF8 = 1 << 5, HTI_UCS1 = HTI_UTF8,
	HTI_UTF16 = 1 << 6,	HTI_UCS2 = HTI_UTF16,
	HTI_UTF32 = 1 << 7, HTI_UCS4 = HTI_UTF32,
	HTI_NON_ENUMERABLE   = HashTableBitFlag(8),
	HTI_NON_WRITABLE     = HashTableBitFlag(9),
	HTI_NON_CONFIGURABLE = HashTableBitFlag(10),
} HashTableDataFlags;

#ifndef HashTable_c

#define dblval(i) ((double)(size_t)(i))

#define numvar(i) sizeof(size_t), dblval(i), HTI_NUMBER
#define ptrvar(p) sizeof(void *), dblval(p), HTI_POINTER
#define dblvar(d) sizeof(double), d, HTI_DOUBLE
#define utf8var(p) 0, dblval(p), HTI_UTF8
#define utf16var(p) 0, dblval(p), HTI_UTF16
#define utf32var(p) 0, dblval(p), HTI_UTF32
#define utf8blk(p, b) b, dblval(p), HTI_UTF8
#define utf16blk(p, b) b, dblval(p), HTI_UTF16
#define utf32blk(p, b) b, dblval(p), HTI_UTF32
#define blkvar(p, b) b, dblval((void*)p), HTI_BLOCK

extern const char * HashTableVendor;
extern const char * HashTableVersion;
extern const char * HashTableDescription;
extern const long HashTableBuildNumber;

typedef void * HashTable;

/* Lifecyle */
// =============================================================================

HashTableData HashTableUserData
(
	size_t valueLength,
	double value,
	size_t valueHint
);

void FreeHashTableUserData
(
	HashTableData * data
);

extern HashTable NewHashTable
(
	size_t size,
	HashTableEvent withEvents,
	HashTableEventHandler eventHandler,
	void * userData
);

extern void OptimizeHashTable
(
	HashTable hashTable,
	size_t slots,
	size_t references
);

void DestroyHashTable
(
	HashTable * ht
);

void HashTableRegisterEvents
(
	HashTable hashTable,
	HashTableEvent withEvents,
	HashTableEventHandler eventHandler
);

/* Statistics */
// =============================================================================

size_t HashTableItemsUsed
(
	HashTable hashTable
);

size_t HashTableItemsTotal
(
	HashTable hashTable
);

size_t HashTableItemsMax
(
	HashTable hashTable
);

extern size_t HashTableSlotCount
(
	HashTable hashTable
);

extern size_t HashTableSlotsUsed
(
	HashTable hashTable
);

extern double HashTableLoadFactor
(
	HashTable hashTable
);

extern size_t HashTableImpact
(
	HashTable hashTable
);

HashTableItem HashTableHasKey
(
	HashTable hashTable,
	size_t keyLength,
	double key,
	HashTableDataFlags hint
);

bool HashTableHasItem
(
	HashTable hashTable,
	HashTableItem reference
);

size_t HashTableItemDistribution
(
	HashTable hashTable,
	HashTableItem reference
);

size_t HashTableItemHits
(
	HashTable hashTable,
	HashTableItem reference
);

size_t HashTableItemImpact
(
	HashTable hashTable,
	HashTableItem reference
);

/* Data Management */
// =============================================================================

extern bool HashTablePutPrivate
(
	HashTable hashTable,
	void * userData
);

extern void * HashTableGetPrivate
(
	HashTable hashTable
);

HashTableItem HashTablePut
(
	HashTable hashTable,
	size_t keyLength,
	double key,
	HashTableDataFlags keyHint,
	size_t valueLength,
	double value,
	HashTableDataFlags valueHint
);

HashTableItem HashTablePutItemByKey
(
	HashTable hashTable,
	HashTableData realKey,
	size_t valueLength,
	double value,
	HashTableDataFlags valueHint
);

HashTableItem HashTablePutItemByKeyData
(
	HashTable hashTable,
	HashTableData realKey,
	HashTableData realData
);

HashTableItem HashTableGetItemByKey
(
	HashTable hashTable,
	HashTableData realKey
);

HashTableItem HashTableGet
(
	HashTable hashTable,
	size_t keyLength,
	double key,
	HashTableDataFlags hint
);

bool HashTableDeleteItem
(
	HashTable hashTable,
	HashTableItem reference
);

HashTableData HashTableItemKey
(
	HashTable hashTable,
	HashTableItem reference
);

HashTableData HashTableItemData
(
	HashTable hashTable,
	HashTableItem reference
);

size_t HashTableDataLength
(
	HashTableData data
);

HashTableDataFlags HashTableDataType
(
	HashTableData data
);

HashTableDataFlags HashTableDataSettings
(
	HashTableData data
);

bool HashTableItemGetEnumerable
(
	HashTable hashTable,
	HashTableItem reference
);

bool HashTableItemSetEnumerable
(
	HashTable hashTable,
	HashTableItem reference,
	bool value
);

bool HashTableItemGetWritable
(
	HashTable hashTable,
	HashTableItem reference
);

bool HashTableItemSetWritable
(
	HashTable hashTable,
	HashTableItem reference,
	bool value
);

bool HashTableItemGetConfigurable
(
	HashTable hashTable,
	HashTableItem reference
);

bool HashTableItemSetConfigurable
(
	HashTable hashTable,
	HashTableItem reference,
	bool value
);

/* Extended Operations */
// =============================================================================

void HashTableEnumerate
(
	HashTable hashTable,
	HashTableEnumerateDirection direction,
	HashTableEnumerationHandler handler,
	void * private
);

void HashTableSortItems
(
	HashTable hashTable,
	HashTableSortType type,
	HashTableSortDirection direction,
	HashTableSortHandler sortHandler,
	void * private
);

void HashTableSortItemHash
(
	HashTable hashTable,
	HashTableItem reference,
	HashTableSortType type,
	HashTableSortDirection direction,
	HashTableSortHandler sortHandler,
	void * private
);

void HashTableEnumerateItemHash
(
	HashTable hashTable,
	HashTableItem reference,
	HashTableEnumerateDirection direction,
	HashTableEnumerationHandler handler,
	void * private
);

const char * HashTableErrorMessage
(
	void
);

#endif

#endif /* HashTable_h */
