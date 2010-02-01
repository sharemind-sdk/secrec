#ifndef CMAP_H
#define CMAP_H

#ifdef __cplusplus
#include <map>
extern "C" {
#endif /* ifdef __cplusplus */

struct cmap;

struct cmap *cmap_init();
void cmap_free(struct cmap *map);

unsigned cmap_contains(struct cmap *map, void *key);
void * cmap_value(struct cmap *map, void *key);
unsigned cmap_insert(struct cmap *map, void *key, void *value);
void cmap_insert_replace(struct cmap *map, void *key, void *value);
unsigned cmap_erase(struct cmap *map, void *key);

#ifdef __cplusplus
} // extern "C"

struct cmap {
    std::map<void*, void*> p;
};
#endif /* ifdef __cplusplus */

#endif
