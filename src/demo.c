
#include "HashTable.h"

#include <stdio.h>

void htItemReport(HashTable ht, HashTableItem item) {
	HashTableData key = HashTableItemKey(ht, item);
	HashTableDataFlags keyFlags = HashTableDataSettings(key);
	printf("\tItem Reference: %i\n", item);
	printf("\tItem Key Type: UTF8\n");
	printf("\tItem Key Length: %i + (1 null) bytes\n",
		HashTableDataLength(key)
	);
	if (keyFlags & HTI_UTF8) {
		printf("\tItem Key Value: %s\n", key);
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

	} else if (event == HT_EVENT_DELETE) {
		printf("HashTable (%p) requesting delete item: %i\n", ht, item);
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
	puts("HashTable Stats\n");
	printf("      Items Max: %i\n", HashTableItemsMax(ht));
	printf("     Items Used: %i\n", HashTableItemsUsed(ht));
	printf("    Items Total: %i\n", HashTableItemsTotal(ht));
	printf("          Slots: %i\n", HashTableSlotCount(ht));
	printf("     Slots Used: %i\n", HashTableSlotsUsed(ht));
	printf("    Load Factor: %0.2g%%\n", HashTableLoadFactor(ht)*100);
	printf("  Memory Impact: %i bytes\n\n", HashTableImpact(ht));
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
		x, utf8var("First Item"), utf8var("My first item")
	);

	HashTableItem second = HashTablePut(
		x, utf8var("Second Item"), utf8var("My second item")
	);

	HashTableItem third = HashTablePut(
		x, utf8var("Third Item"), utf8var("My third item")
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

	OptimizeHashTable(x, 5, 0);
	DumpHashTableStats(x);

	DestroyHashTable(&x);

	puts("");

	return 0;

}
