#include "stringy.h"
#include <stdlib.h>
#include <string.h>

// Global string pool
static StringPool* global_pool = NULL;

void string_pool_init(void) {
    if (!global_pool) {
        global_pool = (StringPool*)malloc(sizeof(StringPool));
        if (global_pool) {
            global_pool->capacity = 16;
            global_pool->size = 0;
            global_pool->handles = (StringHandle**)malloc(
                global_pool->capacity * sizeof(StringHandle*)
            );
        }
    }
}

void string_pool_cleanup(void) {
    if (global_pool) {
        for (size_t i = 0; i < global_pool->size; i++) {
            if (global_pool->handles[i]) {
                free(global_pool->handles[i]->data);
                free(global_pool->handles[i]);
            }
        }
        free(global_pool->handles);
        free(global_pool);
        global_pool = NULL;
    }
}

StringHandle* string_create(const char* str) {
    if (!str) return NULL;
    if (!global_pool) string_pool_init();
    
    // Create new handle regardless of existing strings
    StringHandle* handle = (StringHandle*)malloc(sizeof(StringHandle));
    if (!handle) return NULL;
    
    handle->data = strdup(str);
    handle->length = strlen(str);
    handle->ref_count = 1;
    
    // Add to pool
    if (global_pool->size >= global_pool->capacity) {
        size_t new_capacity = global_pool->capacity * 2;
        StringHandle** new_handles = (StringHandle**)realloc(
            global_pool->handles,
            new_capacity * sizeof(StringHandle*)
        );
        if (new_handles) {
            global_pool->handles = new_handles;
            global_pool->capacity = new_capacity;
        }
    }
    
    if (global_pool->size < global_pool->capacity) {
        global_pool->handles[global_pool->size++] = handle;
    }
    
    return handle;
}

void string_retain(StringHandle* handle) {
    if (handle) {
        handle->ref_count++;
    }
}

void string_release(StringHandle* handle) {
    if (handle && --handle->ref_count == 0) {
        // Remove from pool
        for (size_t i = 0; i < global_pool->size; i++) {
            if (global_pool->handles[i] == handle) {
                // Shift remaining elements
                memmove(&global_pool->handles[i],
                       &global_pool->handles[i + 1],
                       (global_pool->size - i - 1) * sizeof(StringHandle*));
                global_pool->size--;
                break;
            }
        }
        free(handle->data);
        free(handle);
    }
}

const char* string_get(const StringHandle* handle) {
    return handle ? handle->data : NULL;
}

size_t string_length(const StringHandle* handle) {
    return handle ? handle->length : 0;
}

int string_compare(const StringHandle* a, const StringHandle* b) {
    if (!a || !b) return -1;
    return strcmp(a->data, b->data);
}
