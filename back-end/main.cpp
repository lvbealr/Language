#include "core.h"
#include "include/astDumpBack.h"
#include "include/core.h"
#include "treeReader.h"
#include "AST.h"
#include "asmTranslator.h"
#include "IRBasics.h"
#include "linkedListAddons.h"

int main() {
    const char *ASTFileName = "20.04.2025-23:50:35.AST";
    const char *NTFileName  = "20.04.2025-23:50:35.nameTable";

    translationContext ASTContext = {};
    initializeTranslationContext(&ASTContext);

    IR_Context IRContext = {};
    initializeIRContext(&IRContext, &ASTContext);
    
    readNameTable(&ASTContext, NTFileName);
    readAST      (&ASTContext, ASTFileName);

    dumpContext dumpContext = {};
    initializeDumpContext(&dumpContext, "output.dot");

    dumpTree(&ASTContext, &dumpContext, ASTContext.AST);

    destroyTranslationContext(&ASTContext);
    destroyIRContext         (&IRContext);
    destroyDumpContext       (&dumpContext);

    return 0;
}