#include "json_parser.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#define MAX_ERROR_LENGTH 256

typedef struct {
    const char* input;
    size_t position;
    char error[MAX_ERROR_LENGTH];
} Parser;

static void skip_whitespace(Parser* parser) {
    while (isspace(parser->input[parser->position])) {
        parser->position++;
    }
}

static char peek(Parser* parser) {
    return parser->input[parser->position];
}

static char advance(Parser* parser) {
    return parser->input[parser->position++];
}

static int match(Parser* parser, char expected) {
    if (peek(parser) == expected) {
        advance(parser);
        return 1;
    }
    return 0;
}

static JsonValue* parse_value(Parser* parser);

static char* parse_string_content(Parser* parser) {
    size_t start = parser->position;
    size_t length = 0;
    
    while (peek(parser) != '"' && peek(parser) != '\0') {
        if (peek(parser) == '\\') {
            advance(parser); // Skip backslash
            advance(parser); // Skip escaped character
        } else {
            advance(parser);
        }
        length++;
    }
    
    if (peek(parser) == '\0') {
        snprintf(parser->error, MAX_ERROR_LENGTH, "Unterminated string");
        return NULL;
    }
    
    char* result = (char*)malloc(length + 1);
    if (!result) return NULL;
    
    size_t j = 0;
    for (size_t i = start; i < parser->position; i++) {
        if (parser->input[i] == '\\') {
            i++;
            switch (parser->input[i]) {
                case 'n': result[j++] = '\n'; break;
                case 't': result[j++] = '\t'; break;
                case 'r': result[j++] = '\r'; break;
                default: result[j++] = parser->input[i];
            }
        } else {
            result[j++] = parser->input[i];
        }
    }
    result[j] = '\0';
    
    return result;
}

static JsonValue* parse_string(Parser* parser) {
    if (!match(parser, '"')) {
        snprintf(parser->error, MAX_ERROR_LENGTH, "Expected '\"'");
        return NULL;
    }
    
    char* content = parse_string_content(parser);
    if (!content) return NULL;
    
    if (!match(parser, '"')) {
        free(content);
        snprintf(parser->error, MAX_ERROR_LENGTH, "Expected '\"'");
        return NULL;
    }
    
    JsonValue* value = json_create_string(content);
    free(content);
    return value;
}

static JsonValue* parse_number(Parser* parser) {
    char* endptr;
    double number = strtod(&parser->input[parser->position], &endptr);
    
    if (endptr == &parser->input[parser->position]) {
        snprintf(parser->error, MAX_ERROR_LENGTH, "Invalid number");
        return NULL;
    }
    
    parser->position += (endptr - &parser->input[parser->position]);
    return json_create_number(number);
}

static JsonValue* parse_array(Parser* parser) {
    JsonValue* array_value = json_create_array();
    if (!array_value) return NULL;
    
    advance(parser); // Skip '['
    skip_whitespace(parser);
    
    if (match(parser, ']')) {
        return array_value;
    }
    
    while (1) {
        JsonValue* element = parse_value(parser);
        if (!element) {
            json_free(array_value);
            return NULL;
        }
        
        json_array_append(array_value->value.array, element);
        
        skip_whitespace(parser);
        if (match(parser, ']')) break;
        
        if (!match(parser, ',')) {
            snprintf(parser->error, MAX_ERROR_LENGTH, "Expected ',' or ']'");
            json_free(array_value);
            return NULL;
        }
        
        skip_whitespace(parser);
    }
    
    return array_value;
}

static JsonValue* parse_object(Parser* parser) {
    JsonValue* object_value = json_create_object();
    if (!object_value) return NULL;
    
    advance(parser); // Skip '{'
    skip_whitespace(parser);
    
    if (match(parser, '}')) {
        return object_value;
    }
    
    while (1) {
        skip_whitespace(parser);
        
        if (peek(parser) != '"') {
            snprintf(parser->error, MAX_ERROR_LENGTH, "Expected string key");
            json_free(object_value);
            return NULL;
        }
        
        JsonValue* key = parse_string(parser);
        if (!key) {
            json_free(object_value);
            return NULL;
        }
        
        skip_whitespace(parser);
        
        if (!match(parser, ':')) {
            snprintf(parser->error, MAX_ERROR_LENGTH, "Expected ':'");
            json_free(key);
            json_free(object_value);
            return NULL;
        }
        
        skip_whitespace(parser);
        
        JsonValue* value = parse_value(parser);
        if (!value) {
            json_free(key);
            json_free(object_value);
            return NULL;
        }
        
        json_object_set(object_value->value.object, key->value.string, value);
        json_free(key);
        
        skip_whitespace(parser);
        if (match(parser, '}')) break;
        
        if (!match(parser, ',')) {
            snprintf(parser->error, MAX_ERROR_LENGTH, "Expected ',' or '}'");
            json_free(object_value);
            return NULL;
        }
    }
    
    return object_value;
}

static JsonValue* parse_value(Parser* parser) {
    skip_whitespace(parser);
    
    switch (peek(parser)) {
        case 'n':
            if (strncmp(&parser->input[parser->position], "null", 4) == 0) {
                parser->position += 4;
                return json_create_null();
            }
            break;
            
        case 't':
            if (strncmp(&parser->input[parser->position], "true", 4) == 0) {
                parser->position += 4;
                return json_create_boolean(1);
            }
            break;
            
        case 'f':
            if (strncmp(&parser->input[parser->position], "false", 5) == 0) {
                parser->position += 5;
                return json_create_boolean(0);
            }
            break;
            
        case '"': return parse_string(parser);
        case '[': return parse_array(parser);
        case '{': return parse_object(parser);
        case '-':
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            return parse_number(parser);
    }
    
    snprintf(parser->error, MAX_ERROR_LENGTH, "Unexpected character");
    return NULL;
}

JsonValue* json_parse(const char* input, char** error) {
    Parser parser = {
        .input = input,
        .position = 0,
        .error = {0}
    };
    
    JsonValue* value = parse_value(&parser);
    if (!value && error) {
        *error = strdup(parser.error);
    }
    return value;
}

void json_free(JsonValue* value) {
    if (!value) return;
    
    switch (value->type) {
        case JSON_STRING:
            free(value->value.string);
            break;
            
        case JSON_ARRAY: {
            JsonArrayElement* element = value->value.array->head;
            while (element) {
                JsonArrayElement* next = element->next;
                json_free(element->value);
                free(element);
                element = next;
            }
            free(value->value.array);
            break;
        }
        
        case JSON_OBJECT: {
            JsonKeyValue* pair = value->value.object->head;
            while (pair) {
                JsonKeyValue* next = pair->next;
                free(pair->key);
                json_free(pair->value);
                free(pair);
                pair = next;
            }
            free(value->value.object);
            break;
        }
        
        default:
            break;
    }
    
    free(value);
}

// Value creation functions
JsonValue* json_create_null(void) {
    JsonValue* value = (JsonValue*)malloc(sizeof(JsonValue));
    if (value) value->type = JSON_NULL;
    return value;
}

JsonValue* json_create_boolean(int boolean_value) {
    JsonValue* value = (JsonValue*)malloc(sizeof(JsonValue));
    if (value) {
        value->type = JSON_BOOLEAN;
        value->value.boolean = boolean_value;
    }
    return value;
}

JsonValue* json_create_number(double number_value) {
    JsonValue* value = (JsonValue*)malloc(sizeof(JsonValue));
    if (value) {
        value->type = JSON_NUMBER;
        value->value.number = number_value;
    }
    return value;
}

JsonValue* json_create_string(const char* string_value) {
    JsonValue* value = (JsonValue*)malloc(sizeof(JsonValue));
    if (!value) return NULL;
    
    value->type = JSON_STRING;
    value->value.string = strdup(string_value);
    if (!value->value.string) {
        free(value);
        return NULL;
    }
    return value;
}

JsonValue* json_create_array(void) {
    JsonValue* value = (JsonValue*)malloc(sizeof(JsonValue));
    if (!value) return NULL;
    
    value->type = JSON_ARRAY;
    value->value.array = (JsonArray*)malloc(sizeof(JsonArray));
    if (!value->value.array) {
        free(value);
        return NULL;
    }
    
    value->value.array->head = NULL;
    value->value.array->length = 0;
    return value;
}

JsonValue* json_create_object(void) {
    JsonValue* value = (JsonValue*)malloc(sizeof(JsonValue));
    if (!value) return NULL;
    
    value->type = JSON_OBJECT;
    value->value.object = (JsonObject*)malloc(sizeof(JsonObject));
    if (!value->value.object) {
        free(value);
        return NULL;
    }
    
    value->value.object->head = NULL;
    value->value.object->length = 0;
    return value;
}

// Array operations
void json_array_append(JsonArray* array, JsonValue* value) {
    JsonArrayElement* element = (JsonArrayElement*)malloc(sizeof(JsonArrayElement));
    if (!element) return;
    
    element->value = value;
    element->next = NULL;
    
    if (!array->head) {
        array->head = element;
    } else {
        JsonArrayElement* current = array->head;
        while (current->next) {
            current = current->next;
        }
        current->next = element;
    }
    array->length++;
}

JsonValue* json_array_get(JsonArray* array, size_t index) {
    if (index >= array->length) return NULL;
    
    JsonArrayElement* current = array->head;
    for (size_t i = 0; i < index; i++) {
        current = current->next;
    }
    return current->value;
}

// Object operations
void json_object_set(JsonObject* object, const char* key, JsonValue* value) {
    JsonKeyValue* pair = (JsonKeyValue*)malloc(sizeof(JsonKeyValue));
    if (!pair) return;
    
    pair->key = strdup(key);
    pair->value = value;
    pair->next = NULL;
    
    if (!object->head) {
        object->head = pair;
    } else {
        JsonKeyValue* current = object->head;
        while (current->next) {
            current = current->next;
        }
        current->next = pair;
    }
    object->length++;
}

JsonValue* json_object_get(JsonObject* object, const char* key) {
    JsonKeyValue* current = object->head;
    while (current) {
        if (strcmp(current->key, key) == 0) {
            return current->value;
        }
        current = current->next;
    }
    return NULL;
}

// Value getters with type checking
int json_get_boolean(JsonValue* value, int* success) {
    if (value && value->type == JSON_BOOLEAN) {
        if (success) *success = 1;
        return value->value.boolean;
    }
    if (success) *success = 0;
    return 0;
}

double json_get_number(JsonValue* value, int* success) {
    if (value && value->type == JSON_NUMBER) {
        if (success) *success = 1;
        return value->value.number;
    }
    if (success) *success = 0;
    return 0.0;
}

const char* json_get_string(JsonValue* value, int* success) {
    if (value && value->type == JSON_STRING) {
        if (success) *success = 1;
        return value->value.string;
    }
    if (success) *success = 0;
    return NULL;
}

JsonArray* json_get_array(JsonValue* value, int* success) {
    if (value && value->type == JSON_ARRAY) {
        if (success) *success = 1;
        return value->value.array;
    }
    if (success) *success = 0;
    return NULL;
}

JsonObject* json_get_object(JsonValue* value, int* success) {
    if (value && value->type == JSON_OBJECT) {
        if (success) *success = 1;
        return value->value.object;
    }
    if (success) *success = 0;
    return NULL;
}
