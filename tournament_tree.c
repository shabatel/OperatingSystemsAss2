#include "types.h"
#include "stat.h"
#include "user.h"
#include "tournament_tree.h"

//task 3.2

int nextTreeId = 1;

struct trnmnt_tree* trnmnt_tree_alloc(int depth) {
  if (depth < 1)          //invalid depth
    return 0;
   struct trnmnt_tree *tree;
  tree->id = nextTreeId++;
  // tree->nodes = malloc(((1 << depth) -1) * sizeof(int));
  return tree;
}

int trnmnt_tree_dealloc(struct trnmnt_tree* tree) {
  return 0;
}

int trnmnt_tree_acquire(struct trnmnt_tree* tree,int ID) {
  return 0;
}

int trnmnt_tree_release(struct trnmnt_tree* tree,int ID) {
  return 0;
}
