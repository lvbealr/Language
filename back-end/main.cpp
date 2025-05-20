#include "core.h"
#include "treeReader.h"
#include "AST.h"
#include "asmTranslator.h"
#include "astDumpBack.h"

int main() {
    translationContext context = {};
    initializeTranslationContext(&context);

    const char *ASTFileName = "20.04.2025-02:21:50.AST";
    const char *NTFileName  = "20.04.2025-02:21:50.nameTable";
    
    readNameTable(&context, NTFileName);
    readAST      (&context, ASTFileName);

    dumpContext dumpCtxt = {};
    initializeDumpContext(&dumpCtxt, "output.dot");

    dumpTree(&context, &dumpCtxt, context.AST);

    translateToASM(&context, "output.s");

    return 0;
}