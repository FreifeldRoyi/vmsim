#include "map.h"

#include <stdlib.h>
#include <string.h>

static int find_key(map_t* map, void* key)
{
	int i;
	for (i=0; i<map->size; ++i)
	{
		if (!map->comp(map->keys[i],key))
		{
			return i;
		}
	}
	return -1;
}

static void remove_idx(void** arr, int idx, int arrsize)
{
	int i;
	free(arr[idx]);
	for (i=idx; i<arrsize-1; ++i)
	{
		arr[i] = arr[i+1];
	}
}

errcode_t map_init(map_t* map, int key_size, int val_size, comparator_t comp)
{
	map->comp = comp;
	map->key_size = key_size;
	map->val_size = val_size;
	map->keys = map->values = NULL;
	map->size = 0;

	return ecSuccess;
}

errcode_t map_set(map_t* map, void* key, void* val)
{
	int idx = find_key(map, key);
	if (idx == -1)
	{
		++map->size;
		map->keys = realloc(map->keys, map->size * sizeof(void*));
		map->values = realloc(map->values, map->size * sizeof(void*));

		idx = map->size - 1;

		map->keys[idx] = malloc(map->key_size);
		map->values[idx] = malloc(map->val_size);
		memcpy(map->keys[idx], key, map->key_size);
	}

	memcpy(map->values[idx], val, map->val_size);
	return ecSuccess;
}

errcode_t map_get(map_t* map, void* key, void* val)
{
	int idx = find_key(map, key);
	if (idx == -1)
	{
		return ecNotFound;
	}
	if (val == NULL)
	{
		return ecSuccess;
	}
	memcpy(val, map->values[idx], map->val_size);
	return ecSuccess;
}

errcode_t map_remove(map_t* map, void* key)
{
	int idx = find_key(map, key);
	if (idx == -1)
	{
		return ecNotFound;
	}

	remove_idx(map->keys, idx, map->size);
	remove_idx(map->values, idx, map->size);

	--map->size;

	map->keys = realloc(map->keys, map->size * sizeof(void*));
	map->values = realloc(map->values, map->size * sizeof(void*));

	return ecSuccess;
}

void map_destroy(map_t* map)
{
	int i;
	for (i=0; i<map->size; ++i)
	{
		free(map->keys[i]);
		free(map->values[i]);
	}
	if (map->size > 0)
	{
		free(map->keys);
		free(map->values);
	}
	map->keys = NULL;
	map->values = NULL;
}

#include "tests/map_tests.c"
