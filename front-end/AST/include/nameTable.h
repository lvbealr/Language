#ifndef NAME_TABLE_H_
#define NAME_TABLE_H_

#include <cstddef>

#include "buffer.h"

enum class nameType {
    IDENTIFIER = 1 << 0,
    OPERATOR   = 1 << 1,
    TYPE_NAME  = 1 << 2,
    SEPARATOR  = 1 << 3,
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

    size_t rbpOffset = 0;
};

enum class localNameType {
    VARIABLE_IDENTIFIER = 1 << 0,
    FUNCTION_IDENTIFIER = 1 << 1
};

struct localNameTableElement {
    localNameType type         = localNameType::VARIABLE_IDENTIFIER;
    size_t        globalNameID = 0;
};

struct localNameTable {
    int    nameTableID                     = 0;
    size_t size                            = 0;
    Buffer<localNameTableElement> elements = {};
};

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

bufferError initializeNameTable    (Buffer<nameTableElement> *nameTable,       bool  isGlobal);
bufferError addIdentifier          (Buffer<nameTableElement> *nameTable, const char *identifier);

bufferError addLocalIdentifier     (int nameTableIndex, Buffer<localNameTable> *localTables, localNameTableElement newElement, size_t idSize);

int addLocalNameTable     (int nameTableID,    Buffer<localNameTable> *localTables);
int getIndexInLocalTable  (int nameTableIndex, Buffer<localNameTable> *localTables, size_t globalNameID, localNameType nameType);
int getLocalNameTableIndex(int nameTableID,    Buffer<localNameTable> *localTables);

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

#endif // NAME_TABLE_H_