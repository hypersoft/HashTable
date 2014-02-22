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

#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <errno.h>

#define HashTableItem size_t

#define HashTableBitFlag(n) (1 << (n))

typedef enum eHashTableError {
	HT_ERROR_TABLE_UNINITIALIZED = EOWNERDEAD,
	HT_ERROR_ALLOCATION_FAILURE = ENOMEM,
	HT_ERROR_UNSUPPORTED_FUNCTION = ENOTSUP,
	HT_ERROR_ZERO_LENGTH_KEY = EDESTADDRREQ,
	HT_ERROR_KEY_NOT_FOUND = EADDRNOTAVAIL,
	HT_ERROR_INVALID_REFERENCE = EFAULT,
	HT_ERROR_NOT_CONFIGURABLE_ITEM = EACCES,
	HT_ERROR_NOT_WRITABLE_ITEM = EROFS
} HashTableError;

typedef enum eHashTableEvent {
	HT_EVENT_CONSTRUCTED     = HashTableBitFlag(1),
	HT_EVENT_PUT             = HashTableBitFlag(2),
	HT_EVENT_OVERWRITE       = HashTableBitFlag(3),
	HT_EVENT_GET             = HashTableBitFlag(4),
	HT_EVENT_DELETE          = HashTableBitFlag(5),
	HT_EVENT_DESTRUCTING     = HashTableBitFlag(6)
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
	HT_SORT_NUMERIC = 0,
	HT_SORT_ALPHA   = 1
} HashTableSortType;

typedef enum eHashTableSortDirection {
	HT_SORT_ASCENDING  = 0,
	HT_SORT_DESCENDING = 1
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

typedef enum eHashTableItemFlags {
	HTI_INT              = HashTableBitFlag(1),
	HTI_DOUBLE           = HashTableBitFlag(2),
	HTI_POINTER          = HashTableBitFlag(3),
	HTI_UTF8             = HashTableBitFlag(4),
	HTI_BLOCK            = HashTableBitFlag(5),
	HTI_NON_ENUMERABLE   = HashTableBitFlag(6),
	HTI_NON_WRITABLE     = HashTableBitFlag(7),
	HTI_NON_CONFIGURABLE = HashTableBitFlag(8),
} HashTableItemFlags;

/* cast integer things to double */
#define htIntVal(i) ((double)(uint)(i))

/* Quick value parameter generators */
#define htInt(i) sizeof(uint), htIntVal(i), HTI_INT
#define htPtr(p) sizeof(void *), htIntVal(p), HTI_POINTER
#define htDbl(d) sizeof(double), d, HTI_DOUBLE
#define htStr(s) 0, htIntVal(s), HTI_UTF8
#define htBlk(p, s) s, htIntVal((void*)p), HTI_BLOCK
#define htStrN(s, l) l, htIntVal(s), HTI_UTF8

#ifndef HashTable_c

typedef void * HashTable;

/* Lifecyle */
// =============================================================================

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
	size_t slots
);

extern HashTable DestroyHashTable
(
	HashTable hashTable
);

void HashTableRegisterEvents
(
	HashTable hashTable,
	HashTableEvent withEvents,
	HashTableEventHandler eventHandler
);

/* Statistics */
// =============================================================================

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

extern size_t HashTableDistribution
(
	HashTable hashTable,
	size_t keyLength,
	double key,
	HashTableItemFlags hint
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
	HashTableItemFlags keyHint,
	size_t valueLength,
	double value,
	HashTableItemFlags valueHint
);

HashTableItem HashTableGet
(
	HashTable hashTable,
	size_t keyLength,
	double key,
	HashTableItemFlags hint
);

bool HashTableDeleteItem
(
	HashTable hashTable,
	HashTableItem reference
);

const void * HashTableItemKey
(
	HashTable hashTable,
	HashTableItem reference
);

size_t HashTableItemKeyLength
(
	HashTable hashTable,
	HashTableItem reference
);

HashTableItemFlags HashTableItemKeyType
(
	HashTable hashTable,
	HashTableItem reference
);

HashTableItemFlags HashTableItemKeyConfiguration
(
	HashTable hashTable,
	HashTableItem reference
);

const void * HashTableItemValue
(
	HashTable hashTable,
	HashTableItem reference
);

size_t HashTableItemValueLength
(
	HashTable hashTable,
	HashTableItem reference
);

HashTableItemFlags HashTableItemValueType
(
	HashTable hashTable,
	HashTableItem reference
);

HashTableItemFlags HashTableItemValueConfiguration
(
	HashTable hashTable,
	HashTableItem reference
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

void HashTableSortHash
(
	HashTable hashTable,
	size_t keyLength,
	double key,
	HashTableItemFlags keyHint,
	HashTableSortType type,
	HashTableSortDirection direction,
	HashTableSortHandler sortHandler,
	void * private
);

#endif

#endif /* HashTable_h */
