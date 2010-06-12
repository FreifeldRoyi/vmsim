#ifndef MAP_H_
#define MAP_H_

/**A map - a container that maps keys to values and allows searching by keys.*/

#include "vmsim_types.h"

/**A comparator function for keys. It should return TRUE iff *k1 == *k2.
 * */
typedef BOOL (*comparator_t) (void* k1, void* k2);

typedef struct {
	void** keys;
	void** values;

	comparator_t comp;

	int key_size;
	int val_size;

	int size;
}map_t;

/**Initializ a map.
 * @param map the map to initialize
 * @param key_size the size (in bytes) of the keys for this map.
 * @param val_size the size (in bytes) of the values of this map.
 * @param comp the comparison function to use when searching for values in this map.
 *
 * @return ecSuccess on success, some other value on failure.
 * */
errcode_t map_init(map_t* map, int key_size, int val_size, comparator_t comp);

/**Set a value for a key. If no value for this key is stored in this map, a new
 * record for this key is created. otherwise the current value is changed.
 * @param map the map to use.
 * @param key the key whose value should be set
 * @param val the new value for the key
 *
 * @return ecSuccess on success, some other value on failure.
 * */
errcode_t map_set(map_t* map, void* key, void* val);

/**Get the current value for a key, or check if the key appears in the bitmap.
 *
 * @param map the map to use.
 * @param key the key to look for.
 * @param val if this pointer is not null, it receives the value of the key.
 *
 * @return ecSuccess on success, ecNotFound if the key is not found in the map.
 * */
errcode_t map_get(map_t* map, void* key, void* val);

/**Remove a key from the map
 *
 * @param map the map to use.
 * @param key the key to remove
 * @return ecSuccess on success, ecNotFound if the key is not found in the map.
 * */
errcode_t map_remove(map_t* map, void* key);

/**Finalize a map.
 * @param map the map to finalize
 * */
void map_destroy(map_t* map);

#endif /* MAP_H_ */
