// TODO DUMP ERROR (errorWriter.cpp)
// TODO SAVE TREE (treeSaver.cpp)

#include "astDump.h"
#include "binaryTreeDef.h"
#include "customWarning.h"
#include <time.h>

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

#define COPY_STRING(DESTINATION, SOURCE, SIZE) {          \
    DESTINATION = (char *)calloc(SIZE + 1, sizeof(char)); \
    strncpy(DESTINATION, SOURCE, SIZE);                   \
    (DESTINATION)[SIZE] = '\0';                           \
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //
    
dumpError initializeDumpContext(dumpContext *dumpContext, const char *fileName) {
    customWarning(dumpContext,  dumpError::CONTEXT_BAD_POINTER);
    customWarning(fileName, dumpError::BAD_FILENAME);

    COPY_STRING(dumpContext->fileName, fileName, MAX_FILE_NAME_SIZE - 1);

    dumpContext->file = fopen(dumpContext->fileName, "w");
    customWarning(dumpContext->file, dumpError::FILE_OPEN_ERROR);

    return dumpError::NO_ERRORS;
}   

dumpError destroyDumpContext(dumpContext *dumpContext) {
    customWarning(dumpContext, dumpError::CONTEXT_BAD_POINTER);

    if (dumpContext->file) {
        fclose(dumpContext->file);
        dumpContext->file = NULL;
    }

    if (dumpContext->fileName) {
        FREE_(dumpContext->fileName);
        dumpContext->fileName = NULL;
    }

    dumpContext->error = dumpError::NO_ERRORS;

    return dumpError::NO_ERRORS;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

char *getASTFileName(binaryTree<astNode> *tree) {
    customWarning(tree, NULL);

    char *fileName = (char *)calloc(MAX_FILE_NAME_SIZE, sizeof(char));
    customWarning(fileName, NULL);

    const time_t currentTime = time(NULL);
    tm localTime             = *localtime(&currentTime);

    snprintf(fileName, MAX_FILE_NAME_SIZE, "dump/%.2d.%.2d.%.4d-%.2d:%.2d:%.2d.dot", 
                localTime.tm_mday, localTime.tm_mon, localTime.tm_year + 1900,
                localTime.tm_hour, localTime.tm_min, localTime.tm_sec);

    return fileName;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

#define SET_DOT_HEADER(DUMP_CONTEXT) do {                          \
    fprintf((DUMP_CONTEXT)->file, "digraph AST {\n");              \
    fprintf((DUMP_CONTEXT)->file, "node [shape = \"record\"];\n"); \
    fprintf((DUMP_CONTEXT)->file, "rankdir = \"TB\";\n");          \
} while (0)

#define SET_DOT_FOOTER(DUMP_CONTEXT) do {  \
    fprintf((DUMP_CONTEXT)->file, "}\n");  \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

dumpError dumpTree(compilationContext *context, dumpContext *dumpContext, binaryTree<astNode> *tree) {
    customWarning(context,     dumpError::CONTEXT_BAD_POINTER);
    customWarning(dumpContext, dumpError::DUMP_CONTEXT_BAD_POINTER);
    customWarning(tree,        dumpError::TREE_BAD_POINTER);

    char *fileName = getASTFileName(tree);
    customWarning(fileName, dumpError::BAD_FILENAME);

    dumpError error = initializeDumpContext(dumpContext, fileName);
    customWarning(error == dumpError::NO_ERRORS, dumpError::INITIALIZATION_ERROR);

    node<astNode> *root = tree->root;
    customWarning(root, dumpError::NODE_BAD_POINTER);

    SET_DOT_HEADER(dumpContext);

    if (root->left) {
        dumpNode(context, dumpContext, root->left);
    }

    if (root->right) {
        dumpNode(context, dumpContext, root->right);
    }

    FREE_(fileName);

    SET_DOT_FOOTER(dumpContext);

    return dumpError::NO_ERRORS;
}

static int findIdentifierInLocalTable(compilationContext *context, node<astNode> *node) {
    customWarning(context, -1);
    customWarning(node,    -1);

    int nameTableID           = node->data.data.nameTableIndex;
    int localNameTableIndex   = getLocalNameTableIndex(nameTableID, context->localTables);

    int indexInLocalNameTable = getIndexInLocalTable(localNameTableIndex, context->localTables, 
                                                       nameTableID, localNameType::VARIABLE_IDENTIFIER);

    if (indexInLocalNameTable == -1) {
        indexInLocalNameTable = getIndexInLocalTable(localNameTableIndex, context->localTables, 
                                                       nameTableID, localNameType::FUNCTION_IDENTIFIER);
    }

    return indexInLocalNameTable;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

#define SET_LINKS(DUMP_CONTEXT, NODE) do {                                         \
    if ((NODE)->left) {                                                            \
        fprintf((DUMP_CONTEXT)->file, "p%p:<l> -> p%p;\n", (NODE), (NODE)->left);  \
    }                                                                              \
    if ((NODE)->right) {                                                           \
        fprintf((DUMP_CONTEXT)->file, "p%p:<l> -> p%p;\n", (NODE), (NODE)->right); \
    }                                                                              \
} while (0)

#define DUMP_TERMINATOR(CONTEXT, DUMP_CONTEXT, NODE) do {                   \
    fprintf((DUMP_CONTEXT)->file,                                           \
    "p%p [label = \" { TERMINATOR | { %p | %p } } \","                      \
    "style=\"filled\", fillcolor=\"lightgrey\", width=2.0];\n",             \
    (NODE), (NODE)->left, (NODE)->right);                                   \
    SET_LINKS(DUMP_CONTEXT, NODE);                                          \
} while (0)

#define DUMP_CONSTANT(CONTEXT, DUMP_CONTEXT, NODE) do {                     \
    fprintf((DUMP_CONTEXT)->file,                                           \
    "p%p [label = \" { CONSTANT: %d | { %p | %p } } \","                    \
    "style=\"filled\", fillcolor=\"lightblue\", width=2.0];\n",             \
    (NODE), (NODE)->data.data.number, (NODE)->left, (NODE)->right);         \
    SET_LINKS(DUMP_CONTEXT, NODE);                                          \
} while (0)

#define DUMP_STRING(CONTEXT, DUMP_CONTEXT, NODE, NAME, TYPE) do {           \
    fprintf((DUMP_CONTEXT)->file,                                           \
    "p%p [label = \" { %s:\\n%s | { %p | %p } } \","                        \
    "style=\"filled\", fillcolor=\"lightgreen\", width=2.0];\n",            \
    (NODE), #TYPE, NAME, (NODE)->left, (NODE)->right);                      \
    SET_LINKS(DUMP_CONTEXT, NODE);                                          \
} while (0)

#define DUMP_FUNCTION_DEFINITION(CONTEXT, DUMP_CONTEXT, NODE, NAME) do {    \
    fprintf((DUMP_CONTEXT)->file,                                           \
    "p%p [label = \" { FUNCTION: %s | { %p | %p } } \","                    \
    "style=\"filled\", fillcolor=\"yellow\", width=2.0];\n",                \
    (NODE), (NAME), (NODE)->left, (NODE)->right);                           \
    SET_LINKS(DUMP_CONTEXT, NODE);                                          \
} while (0)

#define DUMP_PARAMETERS(CONTEXT, DUMP_CONTEXT, NODE) do {                   \
    fprintf((DUMP_CONTEXT)->file,                                           \
    "p%p [label = \" { PARAMETERS | { %p | %p } } \","                      \
    "style=\"filled\", fillcolor=\"orange\", width=2.0];\n",                \
    (NODE), (NODE)->left, (NODE)->right);                                   \
    SET_LINKS(DUMP_CONTEXT, NODE);                                          \
} while (0)

#define DUMP_VARIABLE_DECLARATION(CONTEXT, DUMP_CONTEXT, NODE, NAME) do {   \
    fprintf((DUMP_CONTEXT)->file,                                           \
    "p%p [label = \" { VARIABLE: %s | { %p | %p } } \","                    \
    "style=\"filled\", fillcolor=\"pink\", width=2.0];\n",                  \
    (NODE), (NAME), (NODE)->left, (NODE)->right);                           \
    SET_LINKS(DUMP_CONTEXT, NODE);                                          \
} while (0)

#define DUMP_FUNCTION_CALL(CONTEXT, DUMP_CONTEXT, NODE) do {                \
    fprintf((DUMP_CONTEXT)->file,                                           \
    "p%p [label = \" { FUNCTION_CALL | { %p | %p } } \","                   \
    "style=\"filled\", fillcolor=\"cyan\", width=2.0];\n",                  \
    (NODE), (NODE)->left, (NODE)->right);                                   \
    SET_LINKS(DUMP_CONTEXT, NODE);                                          \
} while (0)

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

dumpError dumpNode(compilationContext *context, dumpContext *dumpContext, node<astNode> *node) {
    customWarning(context,           dumpError::CONTEXT_BAD_POINTER);
    customWarning(dumpContext,       dumpError::DUMP_CONTEXT_BAD_POINTER);
    customWarning(node,              dumpError::NODE_BAD_POINTER);
    customWarning(dumpContext->file, dumpError::FILE_BAD_POINTER);

    if (node->left) {
        dumpNode(context, dumpContext, node->left);
    }

    if (node->right) {
        dumpNode(context, dumpContext, node->right);
    }

    switch (node->data.type) {
        case nodeType::TERMINATOR:           DUMP_TERMINATOR          (context, dumpContext, node); break;
        case nodeType::CONSTANT:             DUMP_CONSTANT            (context, dumpContext, node); break;

        case nodeType::STRING: {
            char *name = context->nameTable->data[node->data.data.nameTableIndex].name;
            int      nameTableIndex = node->data.data.nameTableIndex;
            nameType nodeNameType   = context->nameTable->data[nameTableIndex].type;

            switch (nodeNameType) {
                case nameType::IDENTIFIER:   DUMP_STRING              (context, dumpContext, node, name, nameType::IDENTIFIER); break;
                case nameType::OPERATOR:     DUMP_STRING              (context, dumpContext, node, name, nameType::OPERATOR);   break;
                case nameType::TYPE_NAME:    DUMP_STRING              (context, dumpContext, node, name, nameType::TYPE_NAME);  break;
                case nameType::SEPARATOR:    DUMP_STRING              (context, dumpContext, node, name, nameType::SEPARATOR);  break;
            }   

            break;
        }

        case nodeType::FUNCTION_DEFINITION: {
            int      nameTableIndex = node->data.data.nameTableIndex;
            char *name = context->nameTable->data[nameTableIndex].name;
                                             DUMP_FUNCTION_DEFINITION (context, dumpContext, node, name);

            break;
        } 
            
        case nodeType::PARAMETERS:           DUMP_PARAMETERS          (context, dumpContext, node); break;

        case nodeType::VARIABLE_DECLARATION: {
            int      nameTableIndex = node->data.data.nameTableIndex;
            char *name = context->nameTable->data[nameTableIndex].name;
                                             DUMP_VARIABLE_DECLARATION(context, dumpContext, node, name);

            break;                                               
        }
        
        case nodeType::FUNCTION_CALL:        DUMP_FUNCTION_CALL       (context, dumpContext, node); break;

        default:                             break;
    }

    return dumpError::NO_ERRORS;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------- //

#undef COPY_STRING

#undef SET_DOT_HEADER
#undef SET_DOT_FOOTER
#undef SET_LINKS

#undef DUMP_TERMINATOR
#undef DUMP_CONSTANT
#undef DUMP_STRING
#undef DUMP_FUNCTION_DEFINITION
#undef DUMP_PARAMETERS
#undef DUMP_VARIABLE_DECLARATION
#undef DUMP_FUNCTION_CALL

// -------------------------------------------------------------------------------------------------------------------------------------------------- //