#include "types.h"
#include "user.h"
#include "spinlock.h"
#include "tournament_tree.h"

//task 3.2
struct trnmnt_tree* trnmnt_tree_alloc(int depth) {
  if (depth < 1)          //invalid depth
    return 0;

  struct trnmnt_tree *tree;
  // initialize tree space
  tree = (struct trnmnt_tree*)malloc(sizeof(struct trnmnt_tree));

  // calculate 2^depth for tree size
  tree->size = 1;
  int index;
  for (index = 0; index < depth; index ++){
    tree->size = tree->size*2;
  }

  // initialize thread id nodes
  tree->threadNodes = malloc(sizeof(int)*(tree->size));
  for(index = 0; index < tree->size; index++){
    tree->threadNodes[index] = 0;
  }

  // initialize mutex id nodes
  tree->mutexNodes = malloc(sizeof(int)*(tree->size - 1));
  for(index = 0; index < tree->size - 1; index++){
    tree->mutexNodes[index] = kthread_mutex_alloc();
  }

  tree->currentNumberOfThreads = 0;
  tree->currentNumberOfWaitings = 0;
  return tree;
}

int trnmnt_tree_dealloc(struct trnmnt_tree* tree) {
  if(tree == 0)
    return -1;

  if(tree->currentNumberOfThreads != 0)
    return -1;

  int index;
  for(index = 0 ; index < tree->size - 1; index++)
    kthread_mutex_dealloc(tree->mutexNodes[index]);

  free(tree->threadNodes);
  free(tree->mutexNodes);
  free(tree);
  return 0;
}

int
mutex_acquire(struct trnmnt_tree* tree , int ID){
  if(kthread_mutex_lock(tree->mutexNodes[ID]) == -1){
    return -1;
  }
  if(ID == 0){
    return 0;
  }
  return mutex_acquire(tree , (ID -1)/2);
}

int
trnmnt_tree_acquire(struct trnmnt_tree* tree,int ID){
  acquire(tree->lock);
  // check tree bounds for id
  if((ID < 0) || (ID > tree->size - 1)) {
    release(tree->lock);
    return -1;
  }

  // check if the id already in use
  if(tree->threadNodes[ID] != 0) {
    release(tree->lock);
    return -1;
  }

  // acquire the id
  tree->threadNodes[ID] = 1;
  tree->currentNumberOfThreads += 1;
  release(tree->lock);
  return mutex_acquire(tree , ID/2 + (tree->size - 1)/2);
}

int trnmnt_tree_release(struct trnmnt_tree* tree,int ID) {
  return 0;
}
