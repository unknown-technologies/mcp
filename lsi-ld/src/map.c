#include <string.h>
#include <stdlib.h>
#include "types.h"
#include "map.h"

unsigned int MAPHashKey(const char* key)
{
	unsigned int hash = 0;
	for(const char* s = key; *s; s++) {
		hash = hash * 31 + *s;
	}
	return hash;
}

void MAPInit(MAP* map)
{
	memset(map, 0, sizeof(MAP));
}

void MAPDestroy(MAP* map)
{
	for(unsigned int i = 0; i < MAP_BUCKETS; i++) {
		MAPENTRY* entry = map->buckets[i];
		while(entry) {
			MAPENTRY* next = entry->next;
			free(entry);
			entry = next;
		}
	}
}

BOOL MAPPut(MAP* map, const char* key, void* value)
{
	unsigned int hash = MAPHashKey(key);
	unsigned int idx = hash % MAP_BUCKETS;
	MAPENTRY* entry = NULL;
	if(map->buckets[idx] == NULL) {
		entry = (MAPENTRY*) malloc(sizeof(MAPENTRY));
		map->buckets[idx] = entry;
	} else {
		MAPENTRY* e = map->buckets[idx];
		while(e) {
			if(!strcmp(e->key, key)) {
				e->value = value;
				return FALSE;
			} else if(e->next) {
				e = e->next;
			} else {
				entry = (MAPENTRY*) malloc(sizeof(MAPENTRY));
				e->next = entry;
				break;
			}
		}
	}

	entry->key = key;
	entry->value = value;
	entry->next = NULL;
	map->size++;

	return TRUE;
}

void* MAPGet(MAP* map, const char* key)
{
	unsigned int hash = MAPHashKey(key);
	unsigned int idx = hash % MAP_BUCKETS;

	MAPENTRY* entry = map->buckets[idx];
	while(entry) {
		if(!strcmp(entry->key, key)) {
			return entry->value;
		} else {
			entry = entry->next;
		}
	}

	return NULL;
}

unsigned int MAPGetSize(MAP* map)
{
	return map->size;
}

void MAPIterate(MAP* map, void (*iterator)(const char* key, void* value, void* user), void* user)
{
	for(unsigned int i = 0; i < MAP_BUCKETS; i++) {
		MAPENTRY* entry = map->buckets[i];
		while(entry) {
			iterator(entry->key, entry->value, user);
			entry = entry->next;
		}
	}
}
