HashTable
=========

1. Public
---------

* HashTableItem
typedef size_t HashTableItem

A temporary reference to an item within a hash table. This item is guaranteed valid, so long as no items have been deleted from the corresponding hash table, and that same hash table has not been optimized since obtaining this reference.

* enum HashTableItemFlags
  - HTI_INT
size_t HTI_INT

The data supplied is a platform sized integer value.
  - HTI_DOUBLE
size_t HTI_DOUBLE

The data supplied is an IEEE 754 double precision floating point value.
  - HTI_POINTER
size_t HTI_POINTER

The data supplied is a platform pointer value.
  - HTI_UTF8
size_T HTI_UTF8

The data supplied is an ANSI C or UTF-8 encoded string.
  - HTI_BLOCK
size_T HTI_BLOCK

Generic block data of the specified length.
  - HTI_NON_ENUMERABLE
size_t HTI_NON_ENUMERABLE

Do not list in enumeration.
  - HTI_NON_WRITABLE
size_t HTI_NON_WRITABLE

Do not overwrite.
  - HTI_NON_CONFIGURABLE
size_t HTI_NON_CONFIGURABLE

Do not delete, do not modify enumerable or configurable.

* ht Macros
Use these macros to perform variant data requests between your application and the HashTable API.
  - htInt
parameter list htInt(size_t i)

Returns parameter list for API functions to diagnose and transpose an integer as a record key or value.
  - htPtr
parameter list htInt(void * p)

Returns parameter list for API functions to diagnose and transpose a pointer as a record key or value.
  - htDbl
parameter list htInt(dobule d)

Returns parameter list for API functions to diagnose and transpose a double as a record key or value.
  - htStr
parameter list htInt(char * s)

Returns parameter list for API functions to diagnose and transpose an ANSI or UTF-8 string as a record key or value.
  - htBlk
parameter list htInt(void * b, size_t s)

Returns parameter list for API functions to diagnose and transpose an arbitrary block of data with specified length as a record key or value.
  - htStrN
parameter list htInt(char * b, size_t s)

Returns parameter list for API functions to diagnose and transpose an arbitrary length ANSI or UTF-8 string pointer as a record key or value.

* HashTableError
enum HashTableError

Error codes that are set in errno in the event of an API call error.
  - HT_ERROR_TABLE_UNINITIALIZED
size_t HT_ERROR_TABLE_UNINITIALIZED

The hash table parameter was invalid.
  - HT_ERROR_ALLOCATION_FAILURE
size_t HT_ERROR_ALLOCATION_FAILURE

A data allocation request failed.
  - HT_ERROR_UNSUPPORTED_FUNCTION
size_t HT_ERROR_UNSUPPORTED_FUNCTION

The library does not support the function called.
  - HT_ERROR_ZERO_LENGTH_KEY
size_t HT_ERROR_ZERO_LENGTH_KEY

A key specifying zero length was supplied, or a UTF-8 key is empty. If you would like to specify a null key, specify the length as 1.
  - HT_ERROR_KEY_NOT_FOUND
size_t HT_ERROR_KEY_NOT_FOUND

The specified key could not be located within the hash table.
  - HT_ERROR_INVALID_REFERENCE
size_t HT_ERROR_INVALID_REFERENCE

The specified HashTableItem was invalid.
  - HT_ERROR_NOT_WRITABLE_ITEM
size_t HT_ERROR_NOT_WRITABLE_ITEM

The request could not be completed because the item is configured read only.
  - HT_ERROR_NOT_CONFIGURABLE_ITEM
size_t HT_ERROR_NOT_CONFIGURABLE_ITEM

The request could not be completed because the item is non-configurable.

* enum HashTableEvent
  - HT_EVENT_CONSTRUCTED
size_t HT_EVENT_CONSTRUCTED

A new hash table has been successfully constructed.
  - HT_EVENT_PUT
size_T HT_EVENT_PUT

The API has been instructed to put data into the hash table.
  - HT_EVENT_GET
size_t HT_EVENT_GET

The API has been queried for a record.
  - HT_EVENT_DELETE
size_t HT_EVENT_DELETE

The API has been instructed to delete a record.
  - HT_EVENT_ENUMERATE
size_t HT_EVENT_ENUMERATE

The hash table is being enumerated.
  - HT_EVENT_SORT_ITEMS
size_t HT_EVENT_SORT_ITEMS

The API would like the user to make a sort decision on table items.
  - HT_EVENT_SORT_HASH
size_t HT_EVENT_SORT_HASH

The API would like the user to make a sort decision on a hash table slot.

  - HT_EVENT_SORT_NUMERIC
size_t HT_EVENT_SORT_NUMERIC

The API would like the user to make a sort decision based on numeric properties. If not specified, implementation defined sort is assumed.
  - HT_EVENT_SORT_DESCENDING
size_t HT_EVENT_SORT_DESCENDING

The API would like the user to make a descending sort decision. If not specified ascending sort order is implied.

  - HT_EVENT_DESTRUCTING
size_t HT_EVENT_DESTRUCTING

A hash table is about to be destroyed.

* (*HashTableEventHandler)
HashTableItem HashTableEventHandler(HashTable ht, HashTableEvent event, HashTableItem primary, HashTableItem secondary, void * private)

The hash table event call back.

* Lifecycle
Operations which manage the existence or "instancing" of hash tables.
  - NewHashTable
HashTable NewHashTable(size_t size, HashTableEvent withEvents, HashTableEventHandler eventHandler, void * private)

Create a new hash table from scratch. 
  - OptimizeHashTable
void OptimizeHashTable(HashTable ht, size_t slots)

Instructs the API to perform a one-time shrinking and expanding of a hash table for optimal performance. Optionally, the number of desired slots may be specified.

Once the hash table has been optimized, if any items have previously been removed since any prior optimizations, no gurantees are made as to the validity of any prior HashTableItem references. More specifically, optimizing a hash table invalidates HashTableItem references.

This procedure is NOT currently functional.
  - DestroyHashTable
HashTable DestroyHashTable(HashTable ht)

Completely destroys a hash table and all of its records, releasing the total memory impact.
  - HashTableRegisterEvents
void HashTableRegisterEvents(HashTable ht, HashTableEvent withEvents, HashTableEventHandler eventHandler)

Modifies the event flags and callback (if supplied, non-zero) associated with a particular hash table.

* Statistics
  - HashTableSlotCount
size_t HashTableSlotCount(HashTable ht)

Returns the total number of slots for a particular hash table.
  - HashTableSlotsUsed
size_t HashTableSlotsUsed(HashTable ht)

Returns the number of slots containing one or more records.
  - HashTableLoadFactor
double HashTableLoadFactor(HashTable ht)

Returns the total number of hash table records divided by the number of slots for a particular hash table.
  - HashTableImpact
size_t HashTableImpact(HashTable ht)

Get report on how many logical bytes this hash table occupies in memory.
  - HashTableDistribution
size_t HashTableDistribution(HashTable ht, size_t keyLength, double key, HashTableItemFlags hint);

Generate a report on how many entries are associated with a key's hash.  - HashTableItemHits
size_t HashTableItemHits(HashTable ht, HashTableItem reference)

Returns the number of times a hash table item has been set or requested. This operation does not alter any items in the hash table nor does it alter the hash table itself.
  - HashTableItemLength
size_t HashTableItemLength(HashTable ht, HashTableItem reference)

Returns the length of a hash table item value.
  - HashTableItemImpact
size_t HashTableItemImpact(HashTable ht, HashTableItem reference)

Generates a report on the total number of bytes used to store an item.

* Data Management
  - HashTableGetPrivate
void * HashTableGetPrivate(HashTable ht)

Returns the private user data associated with a hash table.
  - HashTablePutPrivate
void HashTablePutPrivate(HashTable ht, void * private)

Stores the private data supplied as an user association of data for a hash table.
  - HashTablePut
HashTableItem HashTablePut(HashTable ht, size_t keyLength, double key, HashTableItemFlags keyHint, size_t valueLength, double value, HashTableItemFlags valueHint)

Places a key value pair into the hash table and returns the item's reference.
  - HashTableGet
HashTableItem HashTableGet(HashTable ht, size_t keyLength, double key, HashTableItemFlags hint)

Get's an item reference by its key.
  - HashTableDeleteItem
bool HashTableDeleteItem(HashTable ht, HashTableItem reference)

Removes an item from a hash table.  - HashTableItemGetFlags
HashTableItemFlags HashTableItemGetFlags(HashTable ht, HashTableItem reference)

Retrieves the flags associated with an item.
  - HashTableItemPutFlags
  - HashTableItemPutPrivate
  - HashTableItemGetPrivate
  - HashTableItemValue
Returns a constant pointer to the item data.
* Extended Operations
  - HashTableEnumerate
  - HashTableSortItems
  - HashTableSortHash
  - HashTableErrorMessage
const char * HashTableErrorMessage(void)

Returns a string representing the contents of errno, if errno equates to a HashTableError.

2. Private
----------

* struct sHashTableRecord
  - hitCount
size_t hitCount

The number of times this record has been set or requested by the user. This number does not include initialization.
  - key
HyperVariant key

As far as we are concerned, this is just a void * for allocated data.
  - value
HyperVariant value

See sHashTableRecord.key for more information.
  - successor
struct sHashTableRecord * successor

The next record to succeed this one in the hash table.

* HashTableRecordSize
#define HashTableRecordSize sizeof(sHashTableRecord)

The size in bytes of a single sHashTableRecord.

* HashTableRecord
#define HashTableRecord sHashTableRecord *

An alias for sHashTableRecord *.

* HashTableRecordItems
#define HashTableRecordItems sHashTableRecord **

An alias for sHashTableRecord **.
* HashTableRecordList
#define HashTableRecordList sHashTableRecord **

An alias for sHashTableRecord **.
* struct sHashTable
  - item
HashTableRecordItems item

Pointer array containing the logical order of all hash table entries.
  - itemsUsed
size_t itemsUsed

The number of items written to in the linear pointer array.
  - itemsTotal
size_t itemsTotal

The number of data records actually held by this hash table.
  - itemsMax
size_t itemsMax

The number of records that can be written to the linear pointer array.
  - slot
sHashTable ** slot

This pointer array, stores the entries that are also stored in linear, except it stores them in true Hash Table fashion as a linked list of finite buckets or "slots".
  - slotCount
size_t slotCount

The number of slots this table has allocated for record list storage.
  - eventHandler
HashTableEventHandler eventHandler

The call back procedure which is called when hash table events are "raised".
  - events
size_t eventFlags

The HashTableEvent flags this table's event handler handles.  - impact
size_t impact

The number of bytes consumed in memory by this hash table and all of its records.
  - private
void * private

The private data the user associates with this hash table.

* HashTableSize
#define HashTableSize sizeof(sHashTable)

The size in bytes of a single sHashTable.

* HashTable
#define HashTable sHashTable *

An alias for sHashTable *.

* htReturnIfTableUninitialized
void htReturnIfTableUninitialized(HashTable ht)

Return 0 if ht is null.

* htVoidIfTableUninitialized
void htVoidIfTableUninitialized(HashTable ht)

Return undefined if table is Null (uninitialized).

* htVoidUnsupportedFunction
void htVoidUnsupportedFunction(void)

Set errno to HT_ERROR_UNSUPPORTED_FUNCTION and Return.

* htReturnIfZeroLengthKey
void htReturnIfZeroLengthKey(size_t length)

Set errno to HT_ERROR_ZERO_LENGTH_KEY and return 0.

* htReturnIfAllocationFailure
void htReturnIfAllocationFailure(void * ptr, ...)

If ptr is null, set errno HT_ERROR_ALLOCATION_FAILURE and execute statements (expectedly free(), ...) and return 0.

* htReturnIfInvalidReference
void htReturnIfInvalidReference(HashTable ht, HashTableItem reference)

If reference is an invalid item reference, set errno to HT_ERROR_INVALID_REFERENCE and return 0.

* htReturnIfKeyNotFound
void htReturnIfKeyNotFound(HashTableRecord item)

If item is null, set errno to HT_ERROR_KEY_NOT_FOUND and return 0.

* htReturnIfNotWritableItem
void htReturnIfNotWritableItem(HashTableRecord item)

If item has flags HTI_CONSTANT, set errno to HT_ERROR_NOT_WRITABLE_ITEM and return 0.

* htReturnIfNotConfigurableItem
void htReturnIfNotConfigurableItem(HashTableItem reference)

If the item cannot be configured set errno HT_ERROR_NOT_CONFIGURABLE_ITEM and return 0.

* htItemAccess
void htItemAccess(HashTable ht, HashTableItem reference, identifier label, expression)

Return error if table uninitialized, return error if invalid reference, assign hash table item reference to label, if label is non-null, return label expression otherwise, return invalid reference error.

* htDblIsNaN
boolean htDblIsNaN(double value)

Returns true or false if the value specified is representative of the IEEE 754 floating point value NaN.

* htDblInfinity
(signed int) trilean htDblInfinity(double value)

Returns -1 if the double value specified is an IEEE 754 negative infinity, 1 if the value specified is an IEEE 754 positive infinity, and zero otherwise. Trilean meaning three state value.

* htKeyHash
size_t htKeyHash(HashTable ht, size_t keyLength, void * realKey)

Hashes a key and distributes (selects a slot for) the key in the table.

* void * htRealKey()
void * htRealKey(HashTableRecordFlag hint, double value)

Automatically selects pointer to double, or pointer in double based on the contents of flag.

Value must be an lvalue.

* htRecordImpact
size_t htRecordImpact(HashTableRecord record)

Returns the size in bytes occupied by a hash table record.

* htCreateRecord
HashTableRecord htCreateRecord(void)

Creates a new hash table record.


* htCompareRecordToRealKey
boolean htCompareRecordToRealKey(HashTable ht, HashTableRecord record, size_t keyLength, void * realKey)

Compares a hash table record to a real key.

* htCreateHash
size_t htCreateHash(size_t keyLength, void * realKey)

Creates a hash of any data supplied, using the hash table library's hashing algorithm.

* htFindKeyWithParent
HashTableRecord htFindKeyWithParent(HashTable table, size_t keyLength, void * realKey, HashTableRecord primary, HashTableRecord * parent)

Locate a key within the table beginning the search at primary, returning the results found, as well as the predecessor of the key found (or not found).

* htFindKey
HashTableRecord htFindKey(HashTable table, size_t keyLength, void * realKey)

Locate a key within a table's records and return its 1 based index within the linear list or zero if not found.

* htReturnIfItemNotFound
