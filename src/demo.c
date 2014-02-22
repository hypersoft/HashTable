
#include "HashTable.h"

#include <stdio.h>

void htItemReport(HashTable ht, HashTableItem item) {
	HashTableItemFlags keyFlags = HashTableItemKeyConfiguration(ht, item);
	if (keyFlags & HTI_UTF8) {
		printf("\tItem Reference: %i\n", item);
		printf("\tItem Key Type: UTF8\n");
		printf("\tItem Key Length: %i + (1 null) bytes\n",
			HashTableItemKeyLength(ht, item)
		);
		printf("\tItem Key Value: %s\n", HashTableItemKey(ht, item));
	}
	puts("");
}

HashTableItem htEventCallBack
(
	HashTable ht,
	HashTableEvent event,
	HashTableItem item,
	void * private
) {
	if (event == HT_EVENT_CONSTRUCTED) {
		printf("New HashTable (%p) constructed\n", ht);
	} else if (event == HT_EVENT_PUT) {
		printf("HashTable (%p) requesting put item:\n\n", ht);
		htItemReport(ht, item);
		return true;
	} else if (event == HT_EVENT_DELETE) {
		printf("HashTable (%p) requesting delete item: %i\n", ht, item);
		return true;
	} else if (event == HT_EVENT_DESTRUCTING) {
		printf("Destroying HashTable (%p)\n", ht);
	}
	return item;
}

bool htEnumerationCallBack
(
	void * hashTable,
	HashTableEnumerateDirection direction,
	HashTableItem item,
	void * private
) {
	printf("Enumerating Item Reference: %i\n", item);
	return true;
}

void DumpHashTableStats(HashTable ht) {
	printf(
		"HashTable Slot Count: %i slots\n"
		"HashTable Slots Used: %i slots\n"
		"HashTable Impact: %i bytes\n"
		"HashTable Load: %.1g%%\n\n",
		HashTableSlotCount(ht),
		HashTableSlotsUsed(ht),
		HashTableImpact(ht),
		HashTableLoadFactor(ht)
	);
}

int main ( int argc, char **argv )
{

	puts("");

	HashTable x = NewHashTable(0,
		HT_EVENT_CONSTRUCTED | HT_EVENT_DESTRUCTING |
		HT_EVENT_PUT | HT_EVENT_DELETE,
		htEventCallBack, NULL
	);

	puts("");

	HashTableItem first = HashTablePut(
		x, htStr("First Item"), htStr("My first item")
	);

	HashTableItem second = HashTablePut(
		x, htStr("Second Item"), htStr("My second item")
	);

	HashTableItem third = HashTablePut(
		x, htStr("Third Item"), htStr("My third item")
	);

	puts("Enumerate forwards");
	HashTableEnumerate(x, HT_ENUMERATE_FORWARD, htEnumerationCallBack, NULL);
	puts("");

	HashTableDeleteItem(x, second);
	puts("");

	puts("Enumerate backwards");
	HashTableEnumerate(x, HT_ENUMERATE_REVERSE, htEnumerationCallBack, NULL);
	puts("");

	DumpHashTableStats(x);

	x = DestroyHashTable(x);

	puts("");

	return 0;

}
