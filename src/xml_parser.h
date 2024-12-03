#ifndef XML_PARSER_H
#define XML_PARSER_H

#include <stddef.h>

// XML attribute structure
typedef struct XmlAttribute {
    char* name;
    char* value;
    struct XmlAttribute* next;
} XmlAttribute;

// XML node structure
typedef struct XmlNode {
    char* name;
    char* content;
    XmlAttribute* attributes;
    struct XmlNode* first_child;
    struct XmlNode* next_sibling;
} XmlNode;

// XML document structure
typedef struct XmlDocument {
    XmlNode* root;
    char error[256];
} XmlDocument;

// Parsing functions
XmlDocument* xml_parse_file(const char* filename);
XmlDocument* xml_parse_string(const char* xml_string);
void xml_free_document(XmlDocument* doc);

// Node operations
XmlNode* xml_create_node(const char* name);
void xml_add_child(XmlNode* parent, XmlNode* child);
void xml_add_attribute(XmlNode* node, const char* name, const char* value);
void xml_set_content(XmlNode* node, const char* content);

// Query functions
XmlNode* xml_find_child(XmlNode* parent, const char* name);
const char* xml_get_attribute(XmlNode* node, const char* name);
const char* xml_get_content(XmlNode* node);
XmlNode* xml_find_element(XmlNode* root, const char* path);

#endif
