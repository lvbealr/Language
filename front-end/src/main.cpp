#include <cstdio>
#include <cctype>

#include "lexer.h"
#include "binaryTree.h"
#include "AST.h"
#include "core.h"

int main(int argc, char *argv[]) {
    Buffer<char>   code   = {};
    Buffer<char *> lines  = {};
    Buffer<Token>  tokens = {};

    readFile(&code, "test.txt");
    getLinePointerFromFile(&lines, &code);

    Buffer<Keyword> keywords = {};
    initializeKeywordBuffer(&keywords, KEYWORD_NUMBER);

    initializeTokenBuffer(&keywords, &lines, &tokens, code.capacity);

    binaryTree<node<astNode> *> astTree = {};    
    BINARY_TREE_INITIALIZE(&astTree);

    compilationBuffer content = {};
    content.tokens = &tokens;
    dumpTokenList(&content);
}
