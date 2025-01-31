#include "customWarning.h"
#include "nameTable.h"
#include "buffer.h"

bufferError initializeNameTable(Buffer<nameTableElement> *nameTable, int isGlobal) {
    customWarning(nameTable != NULL, bufferError::POINTER_IS_NULL);

    if (bufferInitialize(nameTable) != bufferError::NO_BUFFER_ERROR) {
        return bufferError::CALLOC_ERROR;
    }
}

bufferError addIdentifier(Buffer<nameTableElement> *nameTable, const char *identifier) {

}

bufferError addLocalIdentifier(int nameTableIndex, Buffer<localNameTable> *localTables, localNameTableElement newElement, size_t idSize) {

}

int addLocalNameTable(int nameTableID, Buffer<localNameTable> *localTables) {

}

int getIndexInLocalTable(int nameTableIndex, Buffer<localNameTable> *localTables) {
    
}

int getLocalNameTableIndex(int nameTableID, Buffer<localNameTable> *localTables, size_t globalNameID, localNameType nameType) {

}