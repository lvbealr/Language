#include "binaryTree.h"
#include "binaryTreeDef.h"
#include "core.h"
#include "buffer.h"
#include "lexer.h"
#include "parser.h"
#include <stdio.h>
#include "astDump.h"
#include "treeSaver.h"

int main(int argc, char *argv[]) {
    
    Buffer<char> fileContentBuffer = {};
    writeFileDataToBuffer(&fileContentBuffer, "test.txt");

    compilationContext context = {};
    initializeCompilationContext(&context, fileContentBuffer.data);

    lexicalAnalysis(&context);

    for (int i = 0; i < context.tokens->currentIndex; i++) {
        dumpToken(&context, context.tokens->data[i]);
    }

    parseCode(&context);

    dumpContext dumpCtxt = {};
    dumpTree(&context, &dumpCtxt, context.AST);
    
    saveDataContext saveCtxt = {};
    saveNameTable(&context, &saveCtxt);

    printf("%s", saveCtxt.buffer->data);

    destroySaveDataContext(&saveCtxt);
    destroyCompilationContext(&context);

    return 0;
}