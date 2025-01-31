#include "binaryTree.h"

int main(int argc, char *argv[]) {
    __OPTIONS_DATA_INITIALIZE__();
    parseConsole(argc, argv);

    binaryTree<int> tree = {};
    BINARY_TREE_INITIALIZE(&tree);

    __OPTIONS_DATA_DESTRUCT__();

    return 0;
}