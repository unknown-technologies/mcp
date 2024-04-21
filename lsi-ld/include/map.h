#ifndef __MAP_H__
#define __MAP_H__

#define	MAP_BUCKETS	16

typedef struct MAPENTRY MAPENTRY;

struct MAPENTRY {
	const char*	key;
	void*		value;
	MAPENTRY*	next;
};

typedef struct {
	MAPENTRY*	buckets[MAP_BUCKETS];
} MAP;

void	MAPInit(MAP* map);
void	MAPDestroy(MAP* map);
BOOL	MAPPut(MAP* map, const char* key, void* value);
void*	MAPGet(MAP* map, const char* key);

#endif
