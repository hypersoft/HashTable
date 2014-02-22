HashTable
=========

Breakfast is served.

[Wikipedia Description of Hash Table](http://en.wikipedia.org/wiki/Hash_table)

In computing, a hash table (also hash map) is a data structure used to implement
an associative array, a structure that can map keys to values. A hash table uses
a hash function to compute an index into an array of buckets or slots, from
which the correct value can be found.
***

## Overview

This library written in 100% C is an implementation of hash table featuring
separate chaining with linked lists and guaranteed logical order enumeration. It
also supports binary keys and data of arbitrary lengths.

* Maintainer: [Triston J. Taylor](http://github.com/hypersoft)
* License: BSD 2-Clause
* Version Tracking: [Hypersoft MakeStats](http://github.com/hypersoft/MakeStats)
* Source Code Platform: Any
* Documentation: HTML with Hierarchical Image Map Navigation

## Notable Feature List

*  Error Reporting Using Function Return Sentinels + `errno`
*  Automated Item Enumeration in Forward and Reverse
*  Manual Enumeration Capable
*  Item Access by Item Reference or Key
*  Descriptive Error Retrieval
*  Call Back Events: Construct, Deconstruct, Put, Get, and Delete
*  Arbitrary Binary Key +/ Data (UTF-8, Integer, Double, Pointer, and Block)
*  Private Data Association
*  Enumerable, Writable and Configurable Item Properties (ECMAScript Like)
*  Load Factor Metering
*  Item Access Hit Counter
*  Jenkins' One At a Time Hashing

## Discussion

### Binary/String Key Mixing
Upon putting an entry into a hash table, both key and value are copied into
the hash table to ensure the integrity and availability of the data for future
requests. Not only will the hash table store these properties, it will also 
track the size of the data supplied. Knowing the length of the key, is an
essential component of lookup performance, wherefore the API, will check the
key's length before performing a memory comparison.

#### Note
It is not advisable to employ binary keys with string keys in the same table.
This equates most apparently to the end user, as two different representations,
of the same value. Private data can be used to store a composite set of user
managed key-value-pairs if need be.