#include "AST.h"
#include "ASMGenerator.h"
#include "astDumpBack.h"
#include "asmTranslator.h"
#include "core.h"
#include "IRBasics.h"
#include "IRGenerator.h"
#include "linkedListAddons.h"
#include "treeReader.h"

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

    generateASM(&IRContext, IRContext.representation, "output.s");

    destroyTranslationContext(&ASTContext);
    destroyIRContext         (&IRContext);
    destroyDumpContext       (&dumpContext);

    return 0;
}