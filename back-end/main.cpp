#include "core.h"
#include "treeReader.h"
#include "AST.h"
#include "asmTranslator.h"
#include "astDumpBack.h"

int main() {
    translationContext context = {};
    initializeTranslationContext(&context);

    const char *ASTFileName = "19.04.2025-22:12:29.AST";
    const char *NTFileName  = "19.04.2025-22:12:29.nameTable";
    
    readNameTable(&context, NTFileName);
    readAST      (&context, ASTFileName);

    for (size_t i = 0; i < context.nameTable->currentIndex; i++) {
        printf("%s\n", context.nameTable->data[i].name);
    }

    for (size_t i = 0; i < context.localTables->currentIndex; i++) {
        printf("Local table %lu:\n", i);

        for (size_t j = 0; j < context.localTables->data[i].size; j++) {
            printf("%lu %p\n", context.localTables->data[i].elements.data[j].globalNameID, context.nameTable->data[context.localTables->data[i].elements.data[j].globalNameID].name);
        }
    }

    dumpContext dumpCtxt = {};
    initializeDumpContext(&dumpCtxt, "output.dot");

    dumpTree(&context, &dumpCtxt, context.AST);

    translateToASM(&context, "output.asm");

    return 0;
}