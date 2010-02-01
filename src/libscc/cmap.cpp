#include "cmap.h"

#include <map>


using namespace std;
typedef map<void*,void*>::iterator VVMI;

extern "C" struct cmap *cmap_init() {
    return new cmap;
}

extern "C" void cmap_free(struct cmap *map) {
    delete map;
}

extern "C" unsigned cmap_contains(struct cmap *map, void *key) {
    return map->p.find(key) != map->p.end();
}

extern "C" void *cmap_value(struct cmap *map, void *key) {
    VVMI it = map->p.find(key);
    if (it == map->p.end()) return 0;
    return (*it).second;
}

extern "C" unsigned cmap_insert(struct cmap *map, void *key, void *value) {
    return map->p.insert(pair<void*,void*>(key, value)).second;
}

extern "C" void cmap_insert_replace(struct cmap *map, void *key, void *value) {
    map->p[key] = value;
}

extern "C" unsigned cmap_erase(struct cmap *map, void *key) {
    return map->p.erase(key);
}
