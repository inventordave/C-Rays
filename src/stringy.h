#ifndef STRINGY_H
#define STRINGY_H

#include <stddef.h>

// String handle type for managing string lifetimes
typedef struct StringHandle {
    char* data;
    size_t ref_count;
    size_t length;
} StringHandle;

// String pool for deduplication
typedef struct StringPool {
    StringHandle** handles;
    size_t capacity;
    size_t size;
} StringPool;

// Initialize the string pool
void string_pool_init(void);

// Clean up the string pool
void string_pool_cleanup(void);

// Create a new string handle or get existing one from pool
StringHandle* string_create(const char* str);

// Increment reference count
void string_retain(StringHandle* handle);

// Decrement reference count and free if zero
void string_release(StringHandle* handle);

// Get string data
const char* string_get(const StringHandle* handle);

// Get string length
size_t string_length(const StringHandle* handle);

// Compare two string handles
int string_compare(const StringHandle* a, const StringHandle* b);

#endif
