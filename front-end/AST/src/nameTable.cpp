#include "buffer.h"
#include "nameTable.h"
#include "customWarning.h"

static const size_t DEFAULT_BUFFER_NAME_TABLE_CAPACITY = 2;

bufferError initializeNameTable(Buffer<nameTableElement> *nameTable, int isGlobal) {
    customWarning(nameTable != NULL, POINTER_IS_NULL);

    if (bufferInitialize(nameTable, DEFAULT_BUFFER_NAME_TABLE_CAPACITY)) {
        return POINTER_IS_NULL;
    }

    if (isGlobal) {
        #define KEYWORD(NAME, NUMBER, LEXEME, TYPE) {                                                            \
            nameTableElement newElement = {.name = LEXEME, .type = static_cast <nameType> (TYPE),                \
                                                        .keyword = static_cast <keywordIdentifier> (NUMBER)};    \
            if (writeDataToBuffer(nameTable, &newElement, 1) != NO_BUFFER_ERROR) {                               \
                return WRITE_DATA_ERROR;                                                                         \
            }                                                                                                    \
        }

        #include "keywords.def"

        #undef KEYWORD
    }

    return NO_BUFFER_ERROR;
}

bufferError identifyElement(Buffer<nameTableElement> *nameTable, const char *identifier) {
    customWarning(nameTable  != NULL, POINTER_IS_NULL);
    customWarning(identifier != NULL, POINTER_IS_NULL);

    nameTableElement newElement = {.name = identifier, .type = nameType::IDENTIFIER, .keyword = keywordIdentifier::UNDEFINED};

    return writeDataToBuffer(nameTable, &newElement, 1);    
}

int addLocalNameTable(int nameTableID, Buffer<localNameTable> *localTables) {
    customWarning(localTables != NULL, POINTER_IS_NULL);

    localNameTable newTable = {.nameTableID = nameTableID, .size = 0, .elements = {}};

    if (bufferInitialize(&newTable.elements, DEFAULT_BUFFER_NAME_TABLE_CAPACITY) != NO_BUFFER_ERROR) {
        return -1;
    }

    writeDataToBuffer(localTables, &newTable, 1);

    return localTables->count - 1;
}

bufferError identifyLocalElement(int nameTableID, Buffer<localNameTable> *localTables, localNameTableElement newElement, size_t size) {
    customWarning(localTables != NULL, POINTER_IS_NULL);

    if (localTables->count >= nameTableID && nameTableID < 0) {
        return NO_BUFFER;
    }

    localTables->data[nameTableID].size += size;

    return writeDataToBuffer(&localTables->data[nameTableID].elements, &newElement, 1);
}

int getLocalNameTableID(int nameTableID, Buffer<localNameTable> *localTables) {
    customWarning(localTables != NULL, POINTER_IS_NULL);

    for (size_t tableID = 0; tableID < localTables->count; tableID++) {
        if (localTables->data[tableID].nameTableID == nameTableID) {
            return tableID;
        }
    }

    return -1;
}

int getLocalElementID(int nameTableID, Buffer<localNameTable> *localTables, size_t globalNameID, localNameType type) {
    customWarning(localTables != NULL, POINTER_IS_NULL);
    
    if (localTables->count >= nameTableID && nameTableID < 0) {
        return -1;
    }

    for (size_t nameID = 0; nameID < localTables->data[nameTableID].elements.count; nameID++) {
        if (localTables->data[nameTableID].elements.data[nameID].globalNameID == globalNameID &&
        ((int) localTables->data[nameTableID].elements.data[nameID].type == type)) {
            return nameID;
        }
    }

    return -1;
}
