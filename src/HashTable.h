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
  HT_OP_ERROR_NO_KEY,
  HT_OP_ERROR_NO_KEY_LENGTH,
  HT_OP_ERROR_NO_VALUE
};

typedef void HashTable;

extern bool
HashTablePutPrivate ( HashTable * hashTable, void * userData );

extern void *
HashTableGetPrivate ( HashTable * hashTable );

extern enum HashTableErrorCode
HashTableGetLastError ( HashTable * hashTable );

extern size_t
HashTableGetEntryCount ( HashTable * hashTable );

extern bool
HashTableDelete ( HashTable * hashTable, char * key, size_t keyLength );

extern const void *
HashTableGet (
HashTable *hashTable, void * key, size_t * valueLength
);

extern int
HashTableGetInt(HashTable * hashTable, char * key, size_t * length);

extern double
HashTableGetDouble(HashTable * hashTable, char * key, size_t * length);

extern bool
HashTablePut ( HashTable * hashTable,
char * key, void * value, size_t valueLength, size_t padding, bool overwrite
);

extern bool
HashTablePutUTF8(HashTable * hashTable, char * key, char * value, bool overwrite);

extern bool
HashTablePutInt(
	HashTable * hashTable, char * key, int value, bool overwrite
);

extern bool
HashTablePutDouble(
	HashTable * hashTable, char * key, double value, bool overwrite
);

extern HashTable *
NewHashTable ( size_t size, void * userData );

#endif /* HashTable_h */
