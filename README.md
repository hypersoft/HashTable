HashTable
=========

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

*  Item Access via Linear Item Reference or Key
*  Arbitrary Binary Key +/ Data (UTF-8, Integer, Double, Pointer, and Block)
*  Jenkins' One At a Time Hashing (Perl Like)
*  Enumerable, Writable and Configurable Item Properties (ECMAScript Like)
*  Call Back Events: Construct, Deconstruct, Put, Get, and Delete
*  Selective Linear Sorting Call Back Interface with User Function
*  Selective Hash Sorting Call Back Interface with User Function
*  Automated Linear Enumeration Call Back Interface with Forward, Reverse, and
User Function
*  Automated Hash Enumeration Call Back Interface with Forward, Reverse and User
Function
*  Manual Linear Enumeration via Probing
*  Error Reporting Using Function Return Sentinels + `errno`
*  Descriptive Error Retrieval
*  Private Data Association
*  Load Factor Metering
*  Item Access Hit Counters

## Discussion

### Make System
The Makefile included with this project is specifically hand crafted for Unix
Likes featuring the GNU Make tool. If your system does not have a POSIX shell
layer, it is highly recommended that you should acquire one. Not only is the
Makefile hand crafted, it features automated source code revision number
tracking using Hypersoft Systems' MakeStats, which is an invaluable addition,
for portable and robust runtime binary compatibility resolution.

### Hypersoft Systems HyperVariant
Binary data storage for items is achieved by employing the HyperVariant library
as a static dependency. If you have HyperVariant installed locally, you will
need to hand hack the Makefile to achieve system level linkage. HyperVariant in
and of itself, is not a HUGE dependency, in light of this fact static linking
is the recommended method of implementation.

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

### Questions, Comments, Suggestions Networking and Donations
By all means, feel free to make contributions to this project, as well as,
inquire about your development issues. You can reach the maintainer
[here](mailto:pc.wiz.tt@gmail.com) or if you feel like going social
[here](https://facebook.com/pc.wiz.tt). This project is beyond low budget, and
has taken significant resources in time, talent, experience, and equipment to
produce. The maintainer is NOT a hobbyist; He is a full-time self-employed
Software Engineer who could greatly use more than a free beer. Should you find
this project to be of any commercial use to you, any level of
[financial re-imbursement](https://paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=DG3H6F8DSG4BC)
would be prudent.
