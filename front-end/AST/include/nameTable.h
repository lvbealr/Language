#ifndef NAME_TABLE_H_
#define NAME_TABLE_H_

#include <cstddef>

#include "buffer.h"

enum class nameType {
    IDENTIFIER = 1,
    OPERATOR   = 2,
    TYPE_NAME  = 3,
    SEPARATOR  = 4,
};

#define KEYWORD(NAME, NUMBER, ...) NAME = NUMBER,

enum class Keyword {
    UNDEFINED = 0,
    
    #include "keywords.def"
};

#undef KEYWORD

struct nameTableElement {
    char    *name    = NULL;
    nameType type    = nameType::IDENTIFIER;
    Keyword  keyword = Keyword::UNDEFINED;
};

enum class localNameType {
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

bufferError initializeNameTable    (Buffer<nameTableElement> *nameTable,       int   isGlobal);
bufferError addIdentifier          (Buffer<nameTableElement> *nameTable, const char *identifier);

bufferError addLocalIdentifier     (int nameTableIndex, Buffer<localNameTable> *localTables, localNameTableElement newElement, size_t idSize);

int         addLocalNameTable      (int nameTableID,    Buffer<localNameTable> *localTables);
int         getIndexInLocalTable   (int nameTableIndex, Buffer<localNameTable> *localTables);
int         getLocalNameTableIndex (int nameTableID,    Buffer<localNameTable> *localTables, size_t globalNameID, localNameType nameType);

#endif // NAME_TABLE_H_