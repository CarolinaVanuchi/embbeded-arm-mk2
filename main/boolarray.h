#ifndef _BOOLARRAY_HEADER_
#define _BOOLARRAY_HEADER_

#include <stdlib.h>
#include <stdbool.h>
#include "esp_heap_caps.h"

typedef struct{
    bool* array;
    size_t size;
}boolArray_t;

boolArray_t *boolArrayCreate(void){
    boolArray_t *array = (boolArray_t*)calloc(1, sizeof(*array));
    array->array = (bool*)calloc(1, sizeof(bool));
    array->size = 0;
    return array;
}

void boolArrayAdd(boolArray_t *array, bool value){
    array->size++;
    // ESP_LOGI("TRY", "1: %p \n", (void*)array->array);
    array->array = (bool*)realloc(array->array, array->size);
    // ESP_LOGI("TRY", "%p - %d", (void*)array->array, array->size);
    array->array[array->size-1] = value;
}

void boolArrayDelete(boolArray_t *array){
    free(array->array);
    free(array);
}

#endif