// astDump.h
#ifndef AST_DUMP_H_
#define AST_DUMP_H_

#include "binaryTreeDef.h"
#include "core.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

// Maximum file name size for the DOT file
#define MAX_FILE_NAME_SIZE 256

enum class dumpError {
    NO_ERRORS = 0,
    CONTEXT_BAD_POINTER = 1 << 0,
    DUMP_CONTEXT_BAD_POINTER = 1 << 1,
    TREE_BAD_POINTER = 1 << 2,
    FILE_BAD_POINTER = 1 << 3,
    BAD_FILENAME = 1 << 4,
    FILE_OPEN_ERROR = 1 << 5,
    INITIALIZATION_ERROR = 1 << 6,
    NODE_BAD_POINTER = 1 << 7,
};

// Structure to hold dump context
struct dumpContext {
    FILE* file = nullptr;
    char* fileName = nullptr;
    dumpError error = dumpError::NO_ERRORS;
};

// Enum for dump-specific errors


// Function declarations
dumpError initializeDumpContext(dumpContext* dumpContext, char* fileName);
dumpError destroyDumpContext(dumpContext* dumpContext);
char* getASTFileName(binaryTree<astNode>* tree);
dumpError dumpTree(translationContext* context, dumpContext* dumpContext, binaryTree<astNode>* tree);
dumpError dumpNode(translationContext* context, dumpContext* dumpContext, node<astNode>* node);

#endif // AST_DUMP_H_

// astDump.cpp
#include "customWarning.h"

// Initialize the dump context
dumpError initializeDumpContext(dumpContext* dumpContext, char* fileName) {
    customWarning(dumpContext, dumpError::CONTEXT_BAD_POINTER);
    customWarning(fileName, dumpError::BAD_FILENAME);

    dumpContext->fileName = fileName;
    dumpContext->file = fopen("output.dot", "w");
    customWarning(dumpContext->file, dumpError::FILE_OPEN_ERROR);

    return dumpError::NO_ERRORS;
}

// Destroy the dump context
dumpError destroyDumpContext(dumpContext* dumpContext) {
    customWarning(dumpContext, dumpError::CONTEXT_BAD_POINTER);

    if (dumpContext->file) {
        fclose(dumpContext->file);
        dumpContext->file = nullptr;
    }

    if (dumpContext->fileName) {
        free(dumpContext->fileName);
        dumpContext->fileName = nullptr;
    }

    dumpContext->error = dumpError::NO_ERRORS;
    return dumpError::NO_ERRORS;
}

// Generate a unique filename for the DOT file based on the current timestamp
char* getASTFileName(binaryTree<astNode>* tree) {
    customWarning(tree, NULL);

    char* fileName = (char*)calloc(MAX_FILE_NAME_SIZE, sizeof(char));
    customWarning(fileName, NULL);

    time_t currentTime = time(nullptr);
    struct tm localTime = *localtime(&currentTime);

    snprintf(fileName, MAX_FILE_NAME_SIZE, "dump/%02d.%02d.%04d-%02d:%02d:%02d.dot",
             localTime.tm_mday, localTime.tm_mon + 1, localTime.tm_year + 1900,
             localTime.tm_hour, localTime.tm_min, localTime.tm_sec);

    return fileName;
}

// Macros for DOT file formatting
#define SET_DOT_HEADER(DUMP_CONTEXT) do {                          \
    fprintf((DUMP_CONTEXT)->file, "digraph AST {\n");              \
    fprintf((DUMP_CONTEXT)->file, "node [shape = \"record\"];\n"); \
    fprintf((DUMP_CONTEXT)->file, "rankdir = \"TB\";\n");          \
} while (0)

#define SET_DOT_FOOTER(DUMP_CONTEXT) do {  \
    fprintf((DUMP_CONTEXT)->file, "}\n");  \
} while (0)

#define SET_LINKS(DUMP_CONTEXT, NODE) do {                                         \
    if ((NODE)->left) {                                                            \
        fprintf((DUMP_CONTEXT)->file, "p%p:<l> -> p%p;\n", (NODE), (NODE)->left);  \
    }                                                                              \
    if ((NODE)->right) {                                                           \
        fprintf((DUMP_CONTEXT)->file, "p%p:<r> -> p%p;\n", (NODE), (NODE)->right); \
    }                                                                              \
} while (0)

// Macros for dumping different node types
#define DUMP_TERMINATOR(CONTEXT, DUMP_CONTEXT, NODE) do {                   \
    fprintf((DUMP_CONTEXT)->file,                                           \
            "p%p [label = \" { TERMINATOR | { %p | %p } } \","              \
            "style=\"filled\", fillcolor=\"lightgrey\", width=2.0];\n",     \
            (NODE), (NODE)->left, (NODE)->right);                           \
    SET_LINKS(DUMP_CONTEXT, NODE);                                          \
} while (0)

#define DUMP_CONSTANT(CONTEXT, DUMP_CONTEXT, NODE) do {                     \
    fprintf((DUMP_CONTEXT)->file,                                           \
            "p%p [label = \" { CONSTANT: %d | { %p | %p } } \","            \
            "style=\"filled\", fillcolor=\"lightblue\", width=2.0];\n",     \
            (NODE), (NODE)->data.data.number, (NODE)->left, (NODE)->right); \
    SET_LINKS(DUMP_CONTEXT, NODE);                                          \
} while (0)

#define DUMP_STRING(CONTEXT, DUMP_CONTEXT, NODE, NAME, TYPE) do {           \
    fprintf((DUMP_CONTEXT)->file,                                           \
            "p%p [label = \" { %s:\\n%s | { %p | %p } } \","                \
            "style=\"filled\", fillcolor=\"lightgreen\", width=2.0];\n",    \
            (NODE), #TYPE, NAME, (NODE)->left, (NODE)->right);              \
    SET_LINKS(DUMP_CONTEXT, NODE);                                          \
} while (0)

#define DUMP_FUNCTION_DEFINITION(CONTEXT, DUMP_CONTEXT, NODE, NAME) do {    \
    fprintf((DUMP_CONTEXT)->file,                                           \
            "p%p [label = \" { FUNCTION: %s | { %p | %p } } \","            \
            "style=\"filled\", fillcolor=\"yellow\", width=2.0];\n",        \
            (NODE), (NAME), (NODE)->left, (NODE)->right);                   \
    SET_LINKS(DUMP_CONTEXT, NODE);                                          \
} while (0)

#define DUMP_PARAMETERS(CONTEXT, DUMP_CONTEXT, NODE) do {                   \
    fprintf((DUMP_CONTEXT)->file,                                           \
            "p%p [label = \" { PARAMETERS | { %p | %p } } \","              \
            "style=\"filled\", fillcolor=\"orange\", width=2.0];\n",        \
            (NODE), (NODE)->left, (NODE)->right);                           \
    SET_LINKS(DUMP_CONTEXT, NODE);                                          \
} while (0)

#define DUMP_VARIABLE_DECLARATION(CONTEXT, DUMP_CONTEXT, NODE, NAME) do {   \
    fprintf((DUMP_CONTEXT)->file,                                           \
            "p%p [label = \" { VARIABLE: %s | { %p | %p } } \","            \
            "style=\"filled\", fillcolor=\"pink\", width=2.0];\n",          \
            (NODE), (NAME), (NODE)->left, (NODE)->right);                   \
    SET_LINKS(DUMP_CONTEXT, NODE);                                          \
} while (0)

#define DUMP_FUNCTION_CALL(CONTEXT, DUMP_CONTEXT, NODE) do {                \
    fprintf((DUMP_CONTEXT)->file,                                           \
            "p%p [label = \" { FUNCTION_CALL | { %p | %p } } \","           \
            "style=\"filled\", fillcolor=\"cyan\", width=2.0];\n",          \
            (NODE), (NODE)->left, (NODE)->right);                           \
    SET_LINKS(DUMP_CONTEXT, NODE);                                          \
} while (0)

// Dump the entire AST
dumpError dumpTree(translationContext* context, dumpContext* dumpContext, binaryTree<astNode>* tree) {
    customWarning(context, dumpError::CONTEXT_BAD_POINTER);
    customWarning(dumpContext, dumpError::DUMP_CONTEXT_BAD_POINTER);
    customWarning(tree, dumpError::TREE_BAD_POINTER);

    char* fileName = getASTFileName(tree);
    customWarning(fileName, dumpError::BAD_FILENAME);

    dumpError error = initializeDumpContext(dumpContext, fileName);
    customWarning(error == dumpError::NO_ERRORS, dumpError::INITIALIZATION_ERROR);

    node<astNode>* root = tree->root;
    customWarning(root, dumpError::NODE_BAD_POINTER);

    SET_DOT_HEADER(dumpContext);

    // Dump the root node and its children recursively
    dumpNode(context, dumpContext, root);

    free(fileName); // Free the filename after use
    SET_DOT_FOOTER(dumpContext);

    return dumpError::NO_ERRORS;
}

// Dump a single AST node
dumpError dumpNode(translationContext* context, dumpContext* dumpContext, node<astNode>* node) {
    customWarning(context, dumpError::CONTEXT_BAD_POINTER);
    customWarning(dumpContext, dumpError::DUMP_CONTEXT_BAD_POINTER);
    customWarning(node, dumpError::NODE_BAD_POINTER);
    customWarning(dumpContext->file, dumpError::FILE_BAD_POINTER);

    // Recursively dump children first (post-order traversal)
    if (node->left) {
        dumpNode(context, dumpContext, node->left);
    }
    if (node->right) {
        dumpNode(context, dumpContext, node->right);
    }

    switch (node->data.type) {
        case nodeType::TERMINATOR:
            DUMP_TERMINATOR(context, dumpContext, node);
            break;

        case nodeType::CONSTANT:
            DUMP_CONSTANT(context, dumpContext, node);
            break;

        case nodeType::STRING: {
            size_t nameTableIndex = node->data.data.nameTableIndex;
            // Validate name table index
            if (!context->nameTable || nameTableIndex >= context->nameTable->currentIndex) {
                fprintf(dumpContext->file,
                        "p%p [label = \" { STRING: INVALID_INDEX | { %p | %p } } \","
                        "style=\"filled\", fillcolor=\"red\", width=2.0];\n",
                        node, node->left, node->right);
                SET_LINKS(dumpContext, node);
                break;
            }
            char* name = context->nameTable->data[nameTableIndex].name;
            nameType nodeNameType = context->nameTable->data[nameTableIndex].type;

            switch (nodeNameType) {
                case nameType::IDENTIFIER:
                    DUMP_STRING(context, dumpContext, node, name, IDENTIFIER);
                    break;
                case nameType::OPERATOR:
                    DUMP_STRING(context, dumpContext, node, name, OPERATOR);
                    break;
                case nameType::TYPE_NAME:
                    DUMP_STRING(context, dumpContext, node, name, TYPE_NAME);
                    break;
                case nameType::SEPARATOR:
                    DUMP_STRING(context, dumpContext, node, name, SEPARATOR);
                    break;
                default:
                    fprintf(dumpContext->file,
                            "p%p [label = \" { STRING: UNKNOWN_TYPE | { %p | %p } } \","
                            "style=\"filled\", fillcolor=\"red\", width=2.0];\n",
                            node, node->left, node->right);
                    SET_LINKS(dumpContext, node);
                    break;
            }
            break;
        }

        case nodeType::FUNCTION_DEFINITION: {
            size_t nameTableIndex = node->data.data.nameTableIndex;
            if (!context->nameTable || nameTableIndex >= context->nameTable->currentIndex) {
                fprintf(dumpContext->file,
                        "p%p [label = \" { FUNCTION: INVALID_INDEX | { %p | %p } } \","
                        "style=\"filled\", fillcolor=\"red\", width=2.0];\n",
                        node, node->left, node->right);
                SET_LINKS(dumpContext, node);
                break;
            }
            char* name = context->nameTable->data[nameTableIndex].name;
            DUMP_FUNCTION_DEFINITION(context, dumpContext, node, name);
            break;
        }

        case nodeType::PARAMETERS:
            DUMP_PARAMETERS(context, dumpContext, node);
            break;

        case nodeType::VARIABLE_DECLARATION: {
            size_t nameTableIndex = node->data.data.nameTableIndex;
            if (!context->nameTable || nameTableIndex >= context->nameTable->currentIndex) {
                fprintf(dumpContext->file,
                        "p%p [label = \" { VARIABLE: INVALID_INDEX | { %p | %p } } \","
                        "style=\"filled\", fillcolor=\"red\", width=2.0];\n",
                        node, node->left, node->right);
                SET_LINKS(dumpContext, node);
                break;
            }
            char* name = context->nameTable->data[nameTableIndex].name;
            DUMP_VARIABLE_DECLARATION(context, dumpContext, node, name);
            break;
        }

        case nodeType::FUNCTION_CALL:
            DUMP_FUNCTION_CALL(context, dumpContext, node);
            break;

        default:
            fprintf(dumpContext->file,
                    "p%p [label = \" { UNKNOWN_NODE | { %p | %p } } \","
                    "style=\"filled\", fillcolor=\"red\", width=2.0];\n",
                    node, node->left, node->right);
            SET_LINKS(dumpContext, node);
            break;
    }

    return dumpError::NO_ERRORS;
}

// Undefine macros to prevent namespace pollution
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