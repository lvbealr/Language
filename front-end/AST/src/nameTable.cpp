#include "customWarning.h"
#include "nameTable.h"
#include "buffer.h"

bufferError initializeNameTable(Buffer<nameTableElement> *nameTable, int isGlobal) {
    customWarning(nameTable, bufferError::POINTER_IS_NULL);

    if (bufferInitialize(nameTable) != bufferError::NO_BUFFER_ERROR) {
        return bufferError::CALLOC_ERROR;
    }

    if (isGlobal) {
        #define KEYWORD(NAME, NUMBER, KEYWORD, TYPE, ...) {                                                         \
            nameTableElement newElement = {.name = KEYWORD, .type = TYPE, .keyword = static_cast<Keyword>(NUMBER)}; \
            if (writeDataToBuffer(nameTable, &element, 1) != bufferError::NO_BUFFER_ERROR) {                        \
                return bufferError::BUFFER_ENDED;                                                                   \
            }                                                                                                       \
        }

        #include "keywords.def"

        #undef KEYWORD
    }

    return bufferError::NO_BUFFER_ERROR;
}

bufferError addIdentifier(Buffer<nameTableElement> *nameTable, const char *identifier) {
    customWarning(identifier, bufferError::POINTER_IS_NULL);

    nameTableElement newElement = {.name = (char *)identifier, .type = nameType::IDENTIFIER, .keyword = Keyword::UNDEFINED};

    return writeDataToBuffer(nameTable, &newElement, 1);
}

bufferError addLocalIdentifier(int nameTableIndex, Buffer<localNameTable> *localTables, localNameTableElement newElement, size_t idSize) {
    customWarning(localTables, bufferError::POINTER_IS_NULL);

    if (localTables->currentIndex >= nameTableIndex && nameTableIndex < 0) {
        return bufferError::NO_BUFFER;
    }

    localTables->data[nameTableIndex].size += idSize;

    return writeDataToBuffer(&localTables->data[nameTableIndex].elements, &newElement, 1);
}

int addLocalNameTable(int nameTableID, Buffer<localNameTable> *localTables) {
    customWarning(localTables, bufferError::POINTER_IS_NULL);

    localNameTable newLocalTable = {.nameTableID = nameTableID, .size = 0, .elements = {}};

    if (bufferInitialize(&newLocalTable.elements) != bufferError::NO_BUFFER_ERROR) {
        return -1;
    }

    writeDataToBuffer(localTables, &newLocalTable, 1);

    return localTables->currentIndex - 1;
}

int getIndexInLocalTable(int nameTableIndex, Buffer<localNameTable> *localTables, size_t globalNameID, localNameType nameType) {
    customWarning(localTables, bufferError::POINTER_IS_NULL);

    if (localTables->currentIndex >= nameTableIndex && nameTableIndex < 0) {
        return -1;
    }

    for (size_t nameIndex = 0; nameIndex < localTables->data[nameTableIndex].elements.currentIndex; nameIndex++) {
        if (localTables->data[nameTableIndex].elements.data[nameIndex].globalNameID == globalNameID &&
           ((int)localTables->data[nameTableIndex].elements.data[nameIndex].type & (int)nameType)) {
            
            return nameIndex;
            
           }
        }

    return -1;
}    

int getLocalNameTableIndex(int nameTableID, Buffer<localNameTable> *localTables) {
    customWarning(localTables, bufferError::POINTER_IS_NULL);

    for (size_t tableIndex = 0; tableIndex < localTables->currentIndex; tableIndex++) {
        if (localTables->data[tableIndex].nameTableID == nameTableID) {
            return tableIndex;
        }
    }

    return -1;
}