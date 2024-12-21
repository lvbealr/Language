#ifndef NAME_TABLE_H_
#define NAME_TABLE_H_

#include <cstddef>

#include "buffer.h"

enum class nameType {
    LATIN      = 0,
    CYRILLIC   = 1,
    OPERATOR   = 2,
    SEPARATOR  = 3,
    NUMBER     = 4,
    NAME_TYPE  = 5,
    NAME       = 6,
    UNDEFINED  = 7,
    IDENTIFIER = 8
};

#define KEYWORD(NAME, NUMBER, ...) NAME = NUMBER,

enum class keywordIdentifier {
    UNDEFINED = 0,
    
    #include "keywords.def"
};

#undef KEYWORD

struct nameTableElement {
    const char       *name    = NULL;
    nameType          type    = nameType::IDENTIFIER;
    keywordIdentifier keyword = keywordIdentifier::UNDEFINED;
};

enum localNameType {
    VARIABLE_IDENTIFIER = 1,
    FUNCTION_IDENTIFIER = 2
};

struct localNameTableElement {
    localNameType type         = VARIABLE_IDENTIFIER;
    size_t        globalNameID = 0;
};

struct localNameTable {
    int    nameTableID                     = 0;
    size_t size                            = 0;
    Buffer<localNameTableElement> elements = {};
};

bufferError initializeNameTable  (Buffer<nameTableElement> *nameTable,       int   isGlobal);
bufferError identifyElement      (Buffer<nameTableElement> *nameTable, const char *identifier);

bufferError identifyLocalElement (int nameTableID, Buffer<localNameTable> *localTables, localNameTableElement newElement, size_t size);

int         addLocalNameTable    (int nameTableID, Buffer<localNameTable> *localTables);
int         getLocalNameTableID  (int nameTableID, Buffer<localNameTable> *localTables);
int         getLocalElementID    (int nameTableID, Buffer<localNameTable> *localTables, size_t globalNameID, localNameType type);

#endif // NAME_TABLE_H_
