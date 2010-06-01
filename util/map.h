#ifndef MAP_H_
#define MAP_H_

#include "vmsim_types.h"

typedef BOOL (*comparator_t) (void* k1, void* k2);

typedef struct {
	void** keys;
	void** values;

	comparator_t comp;

	int key_size;
	int val_size;

	int size;
}map_t;

errcode_t map_init(map_t* map, int key_size, int val_size, comparator_t comp);

errcode_t map_set(map_t* map, void* key, void* val);
errcode_t map_get(map_t* map, void* key, void* val);
errcode_t map_remove(map_t* map, void* key);

void map_destroy(map_t* map);

#endif /* MAP_H_ */
