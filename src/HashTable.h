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

enum HashTableErrorCode {
	HT_OP_ERROR_NONE = 0,
	HT_OP_ERROR_NO_HASH_TABLE,
	HT_OP_ERROR_NO_KEY,
	HT_OP_ERROR_NO_KEY_LENGTH,
	HT_OP_ERROR_NO_VALUE,
	HT_OP_ERROR_KEY_EXISTS,
	HT_OP_ERROR_ALLOCATION_FAILURE
};

#ifdef HashTable_c
	#define HashTableLinkage
	#define HashTable hashtable_t
#else
	#define HashTableLinkage extern
	typedef void HashTable;
#endif

HashTableLinkage bool
HashTablePutPrivate ( HashTable * hashTable, void * userData );

HashTableLinkage void *
HashTableGetPrivate ( HashTable * hashTable );

/* Function: HashTableGetLastError
 *
 * Description
 *
 *     Retrieve AND clear the last error code for a HashTable.
 *
 * Parameters
 *
 * hashTable:
 *
 *     Any HashTable reference returned from NewHashTable()
 *
* Return Value
 *
 *     Exactly 1 member of an enumeration value from HashTableErrorCode.
 *
 */
HashTableLinkage enum HashTableErrorCode
HashTableGetLastError ( HashTable * hashTable );

/* Function: HashTableEntryCount
 *
 * Description
 *
 *     Retrieve the number of entries stored in a HashTable.
 *
 * Parameters
 *
 * hashTable:
 *
 *     Any HashTable reference returned from NewHashTable()
 *
* Return Value
 *
 *     The number of entries stored in this HashTable, or ZERO if the
 *     HashTable doesn't exist.
 *
 */
HashTableLinkage size_t
HashTableGetEntryCount ( HashTable * hashTable );

/* Function: HashTableDelete
 *
 * Description
 *
 *     Remove an entry from the hash table, and release the allocated data
 *     for the corresponding entry key.
 *
 * Parameters
 *
 * hashTable:
 *
 *     Any HashTable reference returned from NewHashTable()
 *
 * key:
 *
 *     A zero terminated string of bytes to use as an identifier.
 *
 * Return Value
 *
 *     Boolean status: true: success, false: failed
 *
 */
HashTableLinkage bool
HashTableDelete ( HashTable * hashTable, char * key);

/* Function: HashTableGet
 *
 * Description
 *
 *     Retrieve the constant pointer stored in the hash table for any particular
 *     entry. If the operation fails, a NULL pointer will be returned and an
 *     error code will be set. See also: HashTableGetLastError(), and
 *     HashTableErrorCode.
 *
 * Parameters
 *
 * hashTable:
 *
 *     Any HashTable reference returned from NewHashTable()
 *
 * key:
 *
 *     A zero terminated string of bytes to use as an identifier.
 *
 * valueLength:
 *
 *     An optional pointer to a size_t value to store the length of the entry
 *     data. If the operation fails, this data will not be modified.
 *
 * Return Value
 *
 *     A constant pointer to the internalized hash table entry data for this
 *     key.Use HashTableDelete() to free this data.
 *
 */
HashTableLinkage const void *
HashTableGet (HashTable *hashTable, char * key, size_t * valueLength);

/* Function: HashTableGetUTF8
 *
 * Description
 *
 *     Retrieve the constant pointer stored in the hash table for a UTF8
 *     entry. If the operation fails, a NULL pointer will be returned and an
 *     error code will be set. See also: HashTableGetLastError(), and
 *     HashTableErrorCode.
 *
 * Parameters
 *
 * hashTable:
 *
 *     Any HashTable reference returned from NewHashTable()
 *
 * key:
 *
 *     A zero terminated string of bytes to use as an identifier.
 *
 * valueLength:
 *
 *     An optional pointer to a size_t value to store the length of the UTF8
 *     data. If the operation fails, this data will not be modified.
 *
 * Return Value
 *
 *     A constant pointer to the internalized hash table UTF8 data for this
 *     key.Use HashTableDelete() to free this data.
 *
 */
#define HashTableGetUTF8 HashTableGet

/* Function: HashTableGetInt
 *
 * Description
 *
 *     Retrieve the constant pointer stored in the hash table for an int sized
 *     value. If the operation fails, a NULL pointer will be returned and an
 *     error code will be set. See also: HashTableGetLastError(), and
 *     HashTableErrorCode.
 *
 * Parameters
 *
 * hashTable:
 *
 *     Any HashTable reference returned from NewHashTable()
 *
 * key:
 *
 *     A zero terminated string of bytes to use as an identifier.
 *
 * Return Value
 *
 *     The int sized value stored in the HashTable for this entry or zero
 *     if the entry is undefined.
 *     You should use the length parameter to verify that the data is of the
 *     expected return type length (sizeof(int)).
 *
 */
HashTableLinkage int
HashTableGetInt(HashTable * hashTable, char * key);

/* Function: HashTableGetPointer
 *
 * Description
 *
 *     Retrieve the pointer stored in the hash table. If the operation fails, 
 *     a NULL pointer will be returned and an error code will be set. See also:
 *     HashTableGetLastError(), and HashTableErrorCode.
 *
 * Parameters
 *
 * hashTable:
 *
 *     Any HashTable reference returned from NewHashTable()
 *
 * key:
 *
 *     A zero terminated string of bytes to use as an identifier.
 *
 * Return Value
 *
 *     The int sized value stored in the HashTable for this entry or zero
 *     if the entry is undefined.
 *     You should use the length parameter to verify that the data is of the
 *     expected return type length (sizeof(int)).
 *
 */
HashTableLinkage void *
HashTableGetPointer(HashTable * hashTable, char * key);

/* Function: HashTableGetDouble
 *
 * Description
 *
 *     Retrieve the constant pointer stored in the hash table for a double sized
 *     value. If the operation fails, a NULL pointer will be returned and an
 *     error code will be set. See also: HashTableGetLastError(), and
 *     HashTableErrorCode.
 *
 * Parameters
 *
 * hashTable:
 *
 *     Any HashTable reference returned from NewHashTable()
 *
 * key:
 *
 *     A zero terminated string of bytes to use as an identifier.
 *
 * Return Value
 *
 *     The double sized value stored in the HashTable for this entry or zero
 *     if the entry is undefined. If this value is ZERO (0) you should check
 *     the last error code if it matters to your process.
 *
 */
HashTableLinkage double
HashTableGetDouble(HashTable * hashTable, char * key);

/* Function: HashTablePut
 *
 * Description
 *
 *     Place a UTF-8 identified entry into the hash table.
 *
 * Parameters
 *
 * hashTable:
 *
 *     Any HashTable reference returned from NewHashTable()
 *
 * key:
 *
 *     A zero terminated string of bytes to use as an identifier.
 *
 * value:
 *
 *     A pointer to the binary data to associate with the key.
 *
 * valueLength:
 *
 *     The size in bytes of the value data.
 *
 * padding:
 *
 *     The number of zero terminated bytes to follow the supplied data.
 *
 * overwrite:
 *
 *     Boolean value. True if the operation should destroy existing data with
 *     the same key.
 *
 * Return Value
 *
 *     Boolean status: true: success, false: failed
 *
 */
HashTableLinkage bool
HashTablePut ( HashTable * hashTable,
	char * key, void * value, size_t valueLength, size_t padding, bool overwrite
);

/* Function: HashTablePutUTF8
 *
 * Description
 *
 *     Place a UTF-8 identified entry consisting of UTF-8 data into the hash
 *     table.
 *
 * Parameters
 *
 * hashTable:
 *
 *     Any HashTable reference returned from NewHashTable()
 *
 * key:
 *
 *     A zero terminated string of bytes to use as an identifier.
 *
 * value:
 *
 *     A pointer to the zero terminated string of bytes to store for this key.
 *
 * overwrite:
 *
 *     Boolean value. True if the operation should destroy existing data with
 *     the same key.
 *
 * Return Value
 *
 *     Boolean status: true: success, false: failed
 *
 */
HashTableLinkage bool
HashTablePutUTF8(
	HashTable * hashTable, char * key, char * value, bool overwrite
);

/* Function: HashTablePutInt
 *
 * Description
 *
 *     Place a UTF-8 identified entry consisting of an int sized value into the
 *     hash table.
 *
 * Parameters
 *
 * hashTable:
 *
 *     Any HashTable reference returned from NewHashTable()
 *
 * key:
 *
 *     A zero terminated string of bytes to use as an identifier.
 *
 * value:
 *
 *     Any platform sized int value.
 *
 * overwrite:
 *
 *     Boolean value. True if the operation should destroy existing data with
 *     the same key.
 *
 * Return Value
 *
 *     Boolean status: true: success, false: failed
 *
 */
HashTableLinkage bool
HashTablePutInt(
	HashTable * hashTable, char * key, int value, bool overwrite
);

/* Function: HashTablePutPointer
 *
 * Description
 *
 *     Place a UTF-8 identified entry consisting of any pointer value into the
 *     hash table.
 *
 * Parameters
 *
 * hashTable:
 *
 *     Any HashTable reference returned from NewHashTable()
 *
 * key:
 *
 *     A zero terminated string of bytes to use as an identifier.
 *
 * value:
 *
 *     Any pointer value.
 *
 * overwrite:
 *
 *     Boolean value. True if the operation should destroy existing data with
 *     the same key.
 *
 * Return Value
 *
 *     Boolean status: true: success, false: failed
 *
 */
HashTableLinkage bool
HashTablePutPointer(
	HashTable * hashTable, char * key, void * value, bool overwrite
);

/* Function: HashTablePutDouble
 *
 * Description
 *
 *     Place a UTF-8 identified entry consisting of a double sized value into
 *     the hash table.
 *
 * Parameters
 *
 * hashTable:
 *
 *     Any HashTable reference returned from NewHashTable()
 *
 * key:
 *
 *     A zero terminated string of bytes to use as an identifier.
 *
 * value:
 *
 *     Any platform sized double value.
 *
 * overwrite:
 *
 *     Boolean value. True if the operation should destroy existing data with
 *     the same key.
 *
 * Return Value
 *
 *     Boolean status: true: success, false: failed
 *
 */
HashTableLinkage bool
HashTablePutDouble(
	HashTable * hashTable, char * key, double value, bool overwrite
);

/* Function: NewHashTable
 *
 * Description
 *
 *     Create a new HashTable of an arbitrary size with private userData.
 *
 * Parameters
 *
 * size:
 *
 *      The number of hash table entries or "list slots" to allocate. If this
 *      parameter is ZERO, then the implementation defined value of
 *      HT_DEFAULT_RESERVE_SLOTS will be used.
 *
 * userData:
 *
 *     A pointer to any data the user must associate with the returned hash
 *     table reference.
 *
 * Return Value
 *
 *     New HashTable reference.
 *
 */
HashTableLinkage HashTable *
NewHashTable ( size_t size, void * userData );

/* Function: DestroyHashTable
 *
 * Description
 *
 *     Deallocates all HashTable allocated data, then destroys the HashTable
 *     itself.
 *
 * Parameters
 *
 * hashTable:
 *
 *     Any HashTable reference returned from NewHashTable()
 *
 * Return Value
 *
 *     Boolean status: true: success, false: failed
 *
 */
HashTableLinkage bool
DestroyHashTable (HashTable * hashTable);

HashTableLinkage bool
HashTableHasEntry ( HashTable * hashTable, char * key );

#undef HashTableLinkage

#endif /* HashTable_h */
