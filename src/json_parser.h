#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <stddef.h>

// JSON value types
typedef enum {
    JSON_NULL,
    JSON_BOOLEAN,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
} JsonValueType;

// Forward declarations
struct JsonValue;
struct JsonObject;
struct JsonArray;

// Key-value pair for objects
typedef struct JsonKeyValue {
    char* key;
    struct JsonValue* value;
    struct JsonKeyValue* next;
} JsonKeyValue;

// Array element
typedef struct JsonArrayElement {
    struct JsonValue* value;
    struct JsonArrayElement* next;
} JsonArrayElement;

// JSON value union
typedef struct JsonValue {
    JsonValueType type;
    union {
        int boolean;
        double number;
        char* string;
        struct JsonArray* array;
        struct JsonObject* object;
    } value;
} JsonValue;

// JSON array
typedef struct JsonArray {
    JsonArrayElement* head;
    size_t length;
} JsonArray;

// JSON object
typedef struct JsonObject {
    JsonKeyValue* head;
    size_t length;
} JsonObject;

// Parsing functions
JsonValue* json_parse(const char* input, char** error);
void json_free(JsonValue* value);

// Value creation functions
JsonValue* json_create_null(void);
JsonValue* json_create_boolean(int value);
JsonValue* json_create_number(double value);
JsonValue* json_create_string(const char* value);
JsonValue* json_create_array(void);
JsonValue* json_create_object(void);

// Array operations
void json_array_append(JsonArray* array, JsonValue* value);
JsonValue* json_array_get(JsonArray* array, size_t index);

// Object operations
void json_object_set(JsonObject* object, const char* key, JsonValue* value);
JsonValue* json_object_get(JsonObject* object, const char* key);

// Value getters with type checking
int json_get_boolean(JsonValue* value, int* success);
double json_get_number(JsonValue* value, int* success);
const char* json_get_string(JsonValue* value, int* success);
JsonArray* json_get_array(JsonValue* value, int* success);
JsonObject* json_get_object(JsonValue* value, int* success);

#endif
