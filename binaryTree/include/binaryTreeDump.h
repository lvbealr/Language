#ifndef BINARY_TREE_DUMP_H_
#define BINARY_TREE_DUMP_H_

#include "binaryTreeDef.h"

template<typename DT>
binaryTreeError binaryTreeDump(binaryTree<DT> *tree) {
    customWarning(tree != NULL, binaryTreeError::TREE_NULL_POINTER);

    tree->infoData->dumpFileName = (char *)"temp.dot";

    FILE *dumpFile = fopen(tree->infoData->dumpFileName, "w");
    customWarning(dumpFile != NULL, binaryTreeError::DUMP_FILE_BAD_POINTER);

    // .DOT HEADER //
    fprintf(dumpFile, "digraph binaryTree {\nsplines=ortho;\nrankdir=HR;\nnodesep=0.4;"
                      "\nnode [shape=record, fontname=\"JetBrains Mono\", fontsize=\"10\", color=\"gray\", style=\"rounded\"];\n"
                      "edge [style=dashed, color=\"green\", weight=\"10\", penwidth=\"2\", "
                      "arrowsize=\"0.4\"];\n");
    // .DOT HEADER //


    // NODE //
    binaryTreeNodeDump    (dumpFile, tree->root);
    binaryTreeNodeDumpLink(dumpFile, tree->root);
    // NODE //

    fprintf(dumpFile, "}\n");

    char *buffer = (char *)calloc(MAX_CMD_BUFFER_SIZE, sizeof(char));
    customWarning(buffer != NULL, binaryTreeError::BAD_BUFFER_POINTER);

    fclose(dumpFile);

    snprintf(buffer, MAX_CMD_BUFFER_SIZE, "echo '<div>' >> %s", tree->infoData->htmlDumpPath);
    system(buffer);

    *buffer = {};

    writeHtmlHeader(tree);

    snprintf(buffer, MAX_CMD_BUFFER_SIZE, "dot -Tsvg %s >> %s",
             tree->infoData->dumpFileName, tree->infoData->htmlDumpPath);
    system(buffer);

    *buffer = {};

    snprintf(buffer, MAX_CMD_BUFFER_SIZE, "echo '</div><hr size='2' color='#ff9900'>' >> %s", tree->infoData->htmlDumpPath);
    system(buffer);

    *buffer = {};

    snprintf(buffer, MAX_CMD_BUFFER_SIZE, "rm -rf temp.svg");
    system(buffer);

    FREE_(buffer);

    return binaryTreeError::NO_ERRORS;
}

template<typename DT> binaryTreeError binaryTreeNodeDump(FILE *dumpFile, node<DT> *currentNode) {
    customWarning(dumpFile    != NULL, binaryTreeError::DUMP_FILE_BAD_POINTER);

    if (currentNode == NULL) {
        return binaryTreeError::NODE_NULL_POINTER;
    }

    fprintf(dumpFile, "p%p [label = \"{ <p> [%p] | <d> data = [%d] | { <l> [%p] | <r> [%p] }}\"];\n",
            currentNode, currentNode, currentNode->data, currentNode->left, currentNode->right);

    if (currentNode->left != NULL) {
        binaryTreeNodeDump(dumpFile, currentNode->left);
    }

    if (currentNode->right != NULL) {
        binaryTreeNodeDump(dumpFile, currentNode->right);
    }

    return binaryTreeError::NO_ERRORS;
}

template<typename DT> binaryTreeError binaryTreeNodeDumpLink(FILE *dumpFile, node<DT> *currentNode) {
    customWarning(dumpFile != NULL, binaryTreeError::DUMP_FILE_BAD_POINTER);

    if (currentNode == NULL) {
        return binaryTreeError::NODE_NULL_POINTER;
    }

    if (currentNode->left != NULL) {
        fprintf(dumpFile, "p%p:<l> -> p%p\n", currentNode, currentNode->left);
        binaryTreeNodeDumpLink(dumpFile, currentNode->left);
    }

    if (currentNode->right != NULL) {
        fprintf(dumpFile, "p%p:<r> -> p%p\n", currentNode, currentNode->right);
        binaryTreeNodeDumpLink(dumpFile, currentNode->right);
    }

    return binaryTreeError::NO_ERRORS;
}

template<typename DT> binaryTreeError writeHtmlHeader(binaryTree<DT> *tree) {
    customWarning(tree != NULL, binaryTreeError::TREE_NULL_POINTER);

    char *header = (char *)calloc(MAX_HEADER_SIZE, sizeof(char));
    customWarning(header != NULL, binaryTreeError::BAD_INFO_POINTER);

    snprintf(header, MAX_HEADER_SIZE, "<br><br><div style='font-size:22px'><b><u>binaryTree</u><font color='DeepSkyBlue'>" " [%p]" "</font></b>"
                                      " at <b><u>%s:%d</u> <u>(%s)</u></b> <font color='DarkOrange'><b><br>born at</b></font>"
                                      " <b><u>%s:%d</u></b> (%s)<br><br></div>",
             tree, tree->infoData->lastUsedFileName, tree->infoData->lastUsedLine, tree->infoData->lastUsedFunctionName,
                       tree->infoData->bornFileName, tree->infoData->bornLine,     tree->infoData->bornFunctionName);

    int openFile = open(tree->infoData->htmlDumpPath, O_WRONLY | O_APPEND);
    customWarning(openFile != (int) binaryTreeError::NO_SUCH_FILE, binaryTreeError::NO_SUCH_FILE);

    ssize_t writeFile = write(openFile, header, MAX_HEADER_SIZE);

    close(openFile);

    FREE_(header);

    return binaryTreeError::NO_ERRORS;
}

#endif // BINARY_TREE_DUMP_H_
