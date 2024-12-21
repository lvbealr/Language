#include "binaryTreeDef.h"
#include "binaryTree.h"

int main(int argc, char *argv[]) {
  binaryTree<int> tree = {};
  BINARY_TREE_INITIALIZE(&tree);
  tree.root->data = 300;

  nodeLink(&tree, tree.root, LEFT);
  tree.root->left->data = 200;

  nodeLink(&tree, tree.root->left, LEFT);
  tree.root->left->left->data = 100;

  callPrintBinaryTree(&tree, INFIX, stdout);

  DUMP_(&tree);

  treeDestruct(&tree);

  return 0;
}