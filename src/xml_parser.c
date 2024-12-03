#include "xml_parser.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#define MAX_ERROR_LENGTH 256
#define XML_BUFFER_SIZE 4096

typedef struct {
    const char* input;
    size_t position;
    char error[MAX_ERROR_LENGTH];
} XmlParser;

static void skip_whitespace(XmlParser* parser) {
    while (isspace(parser->input[parser->position])) {
        parser->position++;
    }
}

static char peek(XmlParser* parser) {
    return parser->input[parser->position];
}

static char advance(XmlParser* parser) {
    return parser->input[parser->position++];
}

static int match(XmlParser* parser, char expected) {
    if (peek(parser) == expected) {
        advance(parser);
        return 1;
    }
    return 0;
}

static char* parse_identifier(XmlParser* parser) {
    size_t start = parser->position;
    while (isalnum(peek(parser)) || peek(parser) == '_' || peek(parser) == '-') {
        advance(parser);
    }
    
    size_t length = parser->position - start;
    if (length == 0) return NULL;
    
    char* identifier = (char*)malloc(length + 1);
    if (!identifier) return NULL;
    
    strncpy(identifier, &parser->input[start], length);
    identifier[length] = '\0';
    return identifier;
}

static char* parse_attribute_value(XmlParser* parser) {
    if (!match(parser, '"')) return NULL;
    
    size_t start = parser->position;
    while (peek(parser) != '"' && peek(parser) != '\0') {
        advance(parser);
    }
    
    if (!match(parser, '"')) return NULL;
    
    size_t length = parser->position - start - 1;
    char* value = (char*)malloc(length + 1);
    if (!value) return NULL;
    
    strncpy(value, &parser->input[start], length);
    value[length] = '\0';
    return value;
}

static XmlAttribute* parse_attribute(XmlParser* parser) {
    char* name = parse_identifier(parser);
    if (!name) return NULL;
    
    skip_whitespace(parser);
    if (!match(parser, '=')) {
        free(name);
        return NULL;
    }
    
    skip_whitespace(parser);
    char* value = parse_attribute_value(parser);
    if (!value) {
        free(name);
        return NULL;
    }
    
    XmlAttribute* attr = (XmlAttribute*)malloc(sizeof(XmlAttribute));
    if (!attr) {
        free(name);
        free(value);
        return NULL;
    }
    
    attr->name = name;
    attr->value = value;
    attr->next = NULL;
    return attr;
}

static XmlNode* parse_element(XmlParser* parser);

static XmlNode* parse_node(XmlParser* parser) {
    if (!match(parser, '<')) return NULL;
    
    // Check for closing tag
    if (match(parser, '/')) return NULL;
    
    char* name = parse_identifier(parser);
    if (!name) return NULL;
    
    XmlNode* node = xml_create_node(name);
    free(name);
    
    if (!node) return NULL;
    
    skip_whitespace(parser);
    
    // Parse attributes
    while (peek(parser) != '>' && peek(parser) != '/' && peek(parser) != '\0') {
        XmlAttribute* attr = parse_attribute(parser);
        if (!attr) break;
        
        if (!node->attributes) {
            node->attributes = attr;
        } else {
            XmlAttribute* current = node->attributes;
            while (current->next) {
                current = current->next;
            }
            current->next = attr;
        }
        
        skip_whitespace(parser);
    }
    
    // Self-closing tag
    if (match(parser, '/')) {
        if (!match(parser, '>')) {
            // Error handling
            return node;
        }
        return node;
    }
    
    if (!match(parser, '>')) {
        // Error handling
        return node;
    }
    
    // Parse content and child elements
    while (peek(parser) != '\0') {
        skip_whitespace(parser);
        
        if (peek(parser) == '<') {
            if (parser->input[parser->position + 1] == '/') {
                // Closing tag
                parser->position += 2;
                char* end_name = parse_identifier(parser);
                if (!end_name || strcmp(end_name, node->name) != 0) {
                    free(end_name);
                    // Error handling
                    return node;
                }
                free(end_name);
                
                skip_whitespace(parser);
                if (!match(parser, '>')) {
                    // Error handling
                    return node;
                }
                break;
            } else {
                // Child element
                XmlNode* child = parse_element(parser);
                if (child) {
                    xml_add_child(node, child);
                }
            }
        } else {
            // Text content
            size_t start = parser->position;
            while (peek(parser) != '<' && peek(parser) != '\0') {
                advance(parser);
            }
            
            size_t length = parser->position - start;
            if (length > 0) {
                char* content = (char*)malloc(length + 1);
                if (content) {
                    strncpy(content, &parser->input[start], length);
                    content[length] = '\0';
                    xml_set_content(node, content);
                    free(content);
                }
            }
        }
    }
    
    return node;
}

static XmlNode* parse_element(XmlParser* parser) {
    return parse_node(parser);
}

XmlDocument* xml_parse_string(const char* xml_string) {
    XmlParser parser = {
        .input = xml_string,
        .position = 0,
        .error = {0}
    };
    
    XmlDocument* doc = (XmlDocument*)malloc(sizeof(XmlDocument));
    if (!doc) return NULL;
    
    skip_whitespace(&parser);
    
    // Skip XML declaration if present
    if (parser.input[parser.position] == '<' && parser.input[parser.position + 1] == '?') {
        while (parser.input[parser.position] != '\0') {
            if (parser.input[parser.position] == '?' && parser.input[parser.position + 1] == '>') {
                parser.position += 2;
                break;
            }
            parser.position++;
        }
        skip_whitespace(&parser);
    }
    
    doc->root = parse_element(&parser);
    
    if (!doc->root) {
        snprintf(doc->error, MAX_ERROR_LENGTH, "Failed to parse XML document");
        return doc;
    }
    
    return doc;
}

XmlDocument* xml_parse_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return NULL;
    
    char* buffer = (char*)malloc(XML_BUFFER_SIZE);
    if (!buffer) {
        fclose(file);
        return NULL;
    }
    
    size_t read_size = fread(buffer, 1, XML_BUFFER_SIZE - 1, file);
    buffer[read_size] = '\0';
    
    fclose(file);
    
    XmlDocument* doc = xml_parse_string(buffer);
    free(buffer);
    
    return doc;
}

XmlNode* xml_create_node(const char* name) {
    XmlNode* node = (XmlNode*)malloc(sizeof(XmlNode));
    if (!node) return NULL;
    
    node->name = strdup(name);
    node->content = NULL;
    node->attributes = NULL;
    node->first_child = NULL;
    node->next_sibling = NULL;
    
    return node;
}

void xml_add_child(XmlNode* parent, XmlNode* child) {
    if (!parent || !child) return;
    
    if (!parent->first_child) {
        parent->first_child = child;
    } else {
        XmlNode* current = parent->first_child;
        while (current->next_sibling) {
            current = current->next_sibling;
        }
        current->next_sibling = child;
    }
}

void xml_add_attribute(XmlNode* node, const char* name, const char* value) {
    if (!node || !name || !value) return;
    
    XmlAttribute* attr = (XmlAttribute*)malloc(sizeof(XmlAttribute));
    if (!attr) return;
    
    attr->name = strdup(name);
    attr->value = strdup(value);
    attr->next = NULL;
    
    if (!node->attributes) {
        node->attributes = attr;
    } else {
        XmlAttribute* current = node->attributes;
        while (current->next) {
            current = current->next;
        }
        current->next = attr;
    }
}

void xml_set_content(XmlNode* node, const char* content) {
    if (!node) return;
    
    if (node->content) {
        free(node->content);
    }
    
    node->content = strdup(content);
}

const char* xml_get_attribute(XmlNode* node, const char* name) {
    if (!node || !name) return NULL;
    
    XmlAttribute* current = node->attributes;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current->value;
        }
        current = current->next;
    }
    
    return NULL;
}

const char* xml_get_content(XmlNode* node) {
    return node ? node->content : NULL;
}

XmlNode* xml_find_child(XmlNode* parent, const char* name) {
    if (!parent || !name) return NULL;
    
    XmlNode* current = parent->first_child;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next_sibling;
    }
    
    return NULL;
}

static void xml_free_attributes(XmlAttribute* attr) {
    while (attr) {
        XmlAttribute* next = attr->next;
        free(attr->name);
        free(attr->value);
        free(attr);
        attr = next;
    }
}

static void xml_free_node(XmlNode* node) {
    if (!node) return;
    
    // Free children first
    xml_free_node(node->first_child);
    // Free siblings
    xml_free_node(node->next_sibling);
    
    // Free node data
    free(node->name);
    free(node->content);
    xml_free_attributes(node->attributes);
    free(node);
}

void xml_free_document(XmlDocument* doc) {
    if (!doc) return;
    
    if (doc->root) {
        xml_free_node(doc->root);
    }
    free(doc);
}

XmlNode* xml_find_element(XmlNode* root, const char* path) {
    if (!root || !path) return NULL;
    
    char* path_copy = strdup(path);
    char* token = strtok(path_copy, "/");
    XmlNode* current = root;
    
    while (token && current) {
        current = xml_find_child(current, token);
        token = strtok(NULL, "/");
    }
    
    free(path_copy);
    return current;
}
