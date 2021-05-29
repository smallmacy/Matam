#include "map.h"
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

/* Implementing Map struct*/

/** Type for defining the key-values relationship */
typedef struct Key_Data_t *KeyData;

struct Map_t {
    int size;
    KeyData first_node;
    KeyData iterator;
    copyMapDataElements copyData;
    copyMapKeyElements copyKey;
    freeMapDataElements freeData;
    freeMapKeyElements freeKey;
    compareMapKeyElements compareKeys;
};

/* Implementing Key-Value struct*/
struct Key_Data_t {
    MapKeyElement key;
    MapDataElement data;
    struct Key_Data_t *next;
};


/**
* Helper function addNode: adds a new node to the KeyData linked list.
* @param map - The map for which to add the KeyData element
* @param keyElement - The key element which need to be reassigned
* @param dataElement - The new data element to associate with the given key.

* @return
* 	MAP_NULL_ARGUMENT if a NULL was sent as map or as iterator
* 	MAP_OUT_OF_MEMORY if an allocation failed (Meaning the function for copying
* 	an element failed)
* 	MAP_SUCCESS the KeyData element had been inserted successfully
*/

static MapResult addNode(Map map, MapKeyElement keyElement, MapDataElement dataElement);


Map mapCreate(copyMapDataElements copyDataElement,
              copyMapKeyElements copyKeyElement,
              freeMapDataElements freeDataElement,
              freeMapKeyElements freeKeyElement,
              compareMapKeyElements compareKeyElements) {
    if (!copyDataElement || !freeDataElement || !compareKeyElements || !copyKeyElement || !freeKeyElement)
        return NULL;
    Map map = malloc(sizeof(struct Map_t));
    if (map == NULL)
        return NULL;
    map->size = 0;
    map->first_node = NULL;
    map->copyData = copyDataElement;
    map->copyKey = copyKeyElement;
    map->freeData = freeDataElement;
    map->freeKey = freeKeyElement;
    map->compareKeys = compareKeyElements;
    map->iterator = map->first_node;
    return map;

}

void mapDestroy(Map map) {
    if (map == NULL)
        return;
    mapClear(map);
    free(map->iterator);
    map->iterator=NULL;
    free(map->first_node);
    map->first_node=NULL;
    free(map);
    map=NULL;
}


Map mapCopy(Map map) {
    if (map == NULL)
        return NULL;
    Map new_map = mapCreate(map->copyData, map->copyKey, map->freeData, map->freeKey, map->compareKeys);
    if (new_map == NULL) {
        return NULL;
    }

    map->iterator = map->first_node;
    // In case the old map is empty
    if (map->first_node == NULL) {
        new_map->first_node = NULL;
        return new_map;
    }
    //else
    MapKeyElement key;
    MapDataElement data;
    while (map->iterator != NULL) {
        key = map->copyKey(map->iterator->key);
        if (key == NULL) {
            mapDestroy(new_map);
            return NULL;
        }
        data = map->copyData(map->iterator->data);
        if (data == NULL) {
            new_map->freeKey(key);
            mapDestroy(new_map);
            return NULL;
        }
        MapResult result = mapPut(new_map, key, data);
        if (result == MAP_OUT_OF_MEMORY) {
            map->freeKey(key);
            map->freeData(data);
            mapDestroy(new_map);
            return NULL;
        }
        map->iterator = map->iterator->next;
    }
    map->freeData(data);
    map->freeKey(key);
    new_map->iterator=NULL;
    new_map->size = map->size;
    return new_map;

}


int mapGetSize(Map map) {
    if (map == NULL)
        return -1;
    return map->size;
}


bool mapContains(Map map, MapKeyElement element) {
    if (map == NULL || element == NULL)
        return false;
    KeyData ptr = map->first_node;
    if (ptr == NULL)
        return false;
    while (ptr != NULL) {
        if (map->compareKeys(ptr->key, element) == 0)
            return true;
        ptr = ptr->next;
    }
    //Didn't find the key;
    return false;
}


MapResult mapPut(Map map, MapKeyElement keyElement, MapDataElement dataElement) {
    if ((!map) || (!keyElement) || (!dataElement)) {
        return MAP_NULL_ARGUMENT;
    }
    if (map->first_node == NULL) {
        MapResult result = addNode(map, keyElement, dataElement);
        if(result != MAP_SUCCESS){
            return MAP_OUT_OF_MEMORY;
        }

        return MAP_SUCCESS;
    }

    map->iterator = map->first_node;
    if(map->compareKeys(map->first_node->key, keyElement) > 0) {

        MapResult result = addNode(map, keyElement, dataElement);
        if(result != MAP_SUCCESS){
            return MAP_OUT_OF_MEMORY;
        }
        return MAP_SUCCESS;
    }
        if(map->compareKeys(map->first_node->key, keyElement) == 0){
            MapDataElement tmp = map->copyData(dataElement);
            if (tmp == NULL) {
                return MAP_OUT_OF_MEMORY;
            }
            map->freeData(map->first_node->data);
            map->first_node->data = tmp;
            return MAP_SUCCESS;

        }
    // Run to relevant node
    while (map->iterator->next && map->compareKeys(map->iterator->next->key, keyElement) < 0) {
        map->iterator = map->iterator->next;
    }
    //Insert KeyData element after iterator.
    if (map->iterator->next == NULL || map->compareKeys(map->iterator->next->key, keyElement) > 0) {
        MapResult result = addNode(map, keyElement, dataElement);
        if(result != MAP_SUCCESS){
            return MAP_OUT_OF_MEMORY;
        }

    }

    //Exists. hence, update.
    if (map->compareKeys(map->iterator->next->key, keyElement) == 0) {
        // Moving to the right KeyData element needed to update.
        map->iterator = map->iterator->next;
        MapDataElement tmp = map->copyData(dataElement);
        if (tmp == NULL) {
            return MAP_OUT_OF_MEMORY;
        }
        map->freeData(map->iterator->data);
        map->iterator->data = tmp;
    }
    return MAP_SUCCESS;
}


MapDataElement mapGet(Map map, MapKeyElement keyElement) {
    if (map == NULL || keyElement == NULL)
        return NULL;
    KeyData ptr = map->first_node;
    if (ptr == NULL)
        return NULL;
    while (ptr != NULL) {
        if (map->compareKeys(ptr->key, keyElement) == 0) {
            return ptr->data;
        }
        if (map->compareKeys(ptr->key, keyElement) > 0) {
            return NULL;
        }
        ptr = ptr->next;
    }
    //Didn't find the key;
    return NULL;
}


MapResult mapRemove(Map map, MapKeyElement keyElement) {
    if (map == NULL || keyElement == NULL)
        return MAP_NULL_ARGUMENT;
    if (map->first_node == NULL)
        return MAP_ITEM_DOES_NOT_EXIST;

    if (map->compareKeys(map->first_node->key, keyElement) == 0) {
        KeyData tmp = map->first_node;
        map->first_node = map->first_node->next;

        map->freeKey(tmp->key);
        tmp->key=NULL;
        map->freeData(tmp->data);
        tmp->data=NULL;
        free(tmp);
        tmp=NULL;
        map->size--;
        return MAP_SUCCESS;
    }

    map->iterator = map->first_node;

    while (map->iterator->next != NULL) {
        if (map->compareKeys(map->iterator->next->key, keyElement) == 0) {
            KeyData tmp = map->iterator->next;
            map->iterator->next = (map->iterator->next->next == NULL) ? map->iterator->next->next : NULL;

            map->freeKey(tmp->key);
            tmp->key=NULL;
            map->freeData(tmp->data);
            tmp->data=NULL;
            free(tmp);
            tmp = NULL;
            map->size--;
            return MAP_SUCCESS;
        }
        if (map->compareKeys(map->iterator->key, keyElement) > 0)
            break;
        map->iterator = map->iterator->next;
    }
    return MAP_ITEM_DOES_NOT_EXIST;

}

MapKeyElement mapGetFirst(Map map) {
    if (map == NULL || map->first_node == NULL) {
        return NULL;
    }
    map->iterator = map->first_node;
    ///Changes
    MapKeyElement first_key = map->copyKey(map->first_node->key);
    if(!first_key){
        return NULL;
    }
    return first_key;
}


MapKeyElement mapGetNext(Map map) {
    if (map == NULL || map->iterator == NULL)
        return NULL;
    map->iterator = map->iterator->next;
    if (map->iterator == NULL)
        return NULL;
    ///Changes
    MapKeyElement next_key = map->copyKey(map->iterator->key);
    if(!next_key) {
        return NULL;
    }
    return next_key;
}


MapResult mapClear(Map map) {
    if (map == NULL)
        return MAP_NULL_ARGUMENT;
    if (map->first_node == NULL)
        return MAP_SUCCESS;
    KeyData temp = NULL;
    map->iterator = map->first_node;
    while (map->iterator != NULL) {
        temp = map->iterator;
        map->iterator = map->iterator->next;
        map->freeData(temp->data);
        temp->data = NULL;
        map->freeKey(temp->key);
        temp->key = NULL;
        free(temp);
        temp = NULL;
    }
    map->first_node = NULL;
    map->size = 0;
    return MAP_SUCCESS;
}


static MapResult addNode(Map map, MapKeyElement key_element, MapDataElement data_element) {

    KeyData tmp = (KeyData) malloc(sizeof(struct Key_Data_t));
    if (tmp == NULL) {
        return MAP_OUT_OF_MEMORY;
    }
    tmp->key = map->copyKey(key_element);
    if (tmp->key == NULL) {
        free(tmp);
        tmp = NULL;
        return MAP_OUT_OF_MEMORY;
    }
    tmp->data = map->copyData(data_element);
    if (tmp->data == NULL) {
        map->freeKey(tmp->key);
        tmp->key = NULL;
        free(tmp);
        tmp = NULL;
        return MAP_OUT_OF_MEMORY;
    }
    //Check if map is empty
    if (map->size == 0) {
        map->first_node = tmp;
        tmp->next = NULL;

    }
    else if(map->compareKeys(map->first_node->key, tmp->key) > 0) {
        tmp->next = map->first_node;
        map->first_node = tmp;
    }
    else {
        tmp->next = map->iterator->next;
        map->iterator->next = tmp;
    }
    map->size++;
    return MAP_SUCCESS;
}
