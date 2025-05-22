#include "core.h"
#include "include/astDumpBack.h"
#include "include/core.h"
#include "treeReader.h"
#include "AST.h"
#include "asmTranslator.h"
#include "IRBasics.h"
#include "linkedListAddons.h"
#include "IRGenerator.h"

int main() {
    const char *ASTFileName = "22.04.2025-01:33:22.AST";
    const char *NTFileName  = "22.04.2025-01:33:22.nameTable";

    translationContext ASTContext = {};
    initializeTranslationContext(&ASTContext);

    IR_Context IRContext = {};
    initializeIRContext(&IRContext, &ASTContext);
    
    readNameTable(&ASTContext, NTFileName);
    readAST      (&ASTContext, ASTFileName);

    generateIR(&IRContext);

    printIR(IRContext.representation);

    dumpContext dumpContext = {};
    initializeDumpContext(&dumpContext, "output.dot");

    dumpTree(&ASTContext, &dumpContext, ASTContext.AST);

    generateASM(IRContext.representation, "output.s");

    destroyTranslationContext(&ASTContext);
    destroyIRContext         (&IRContext);
    destroyDumpContext       (&dumpContext);

    return 0;
}