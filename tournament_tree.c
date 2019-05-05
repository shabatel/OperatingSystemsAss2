#include "types.h"
#include "user.h"
#include "spinlock.h"
#include "kthread.h"
#include "tournament_tree.h"

//task 3.2
trnmnt_tree* trnmnt_tree_alloc(int depth) {
  if (depth < 1)          //invalid depth
    return 0;

  trnmnt_tree *tree;
  // initialize tree space
  tree = (trnmnt_tree*)malloc(sizeof (trnmnt_tree));

  // calculate 2^depth for tree size      -1?
  tree->size = 1;
  int index;
  for (index = 0; index < depth; index ++){
    tree->size = tree->size*2;
  }

  // initialize thread id nodes(?)
  tree->threadNodes = (int*) malloc(sizeof(int)*(tree->size));
  for(index = 0; index < tree->size; index++){
    tree->threadNodes[index] = 0;
  }

  // initialize mutex id nodes
  tree->mutexNodes = (int*) malloc(sizeof(int)*(tree->size - 1));
  for(index = 0; index < tree->size - 1; index++){
    tree->mutexNodes[index] = kthread_mutex_alloc();
  }
  return tree;
}

int trnmnt_tree_dealloc(struct trnmnt_tree* tree) {
  if(tree == 0)
    return -1;

  int index;
  for(index = 0 ; index < tree->size - 1; index++) {
      int result = kthread_mutex_dealloc(tree->mutexNodes[index]);
      if (result == -1)
          return -1;
  }

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
  // check tree bounds for id
  if((ID < 0) || (ID > tree->size - 1)) {
    return -1;
  }

  // check if the id already in use
  if(tree->threadNodes[ID] != 0) {
    return -1;
  }

  // acquire the id
  tree->threadNodes[ID] = 1;
  return mutex_acquire(tree , ID/2 + (tree->size - 1)/2);
}


int
mutex_release( trnmnt_tree* tree , int stopCondition, int ID){
        // release the final lock
    if( ID == stopCondition){
        return kthread_mutex_unlock(tree->mutexNodes[ID]);
    }
        //  release all fathers
    if( mutex_release(tree, stopCondition, (ID-1)/2) == -1){
        return -1;
    }
        //release curr ID
    if(kthread_mutex_unlock(tree->mutexNodes[ID]) == -1){
        return -1;
    }

    return 0;
}


int trnmnt_tree_release(struct trnmnt_tree* tree,int ID) {
    if((ID < 0) || (ID > tree->size - 1)) {
        return -1;
    }
    // check if the id is not in use
    if(tree->threadNodes[ID] == 0) {
        return -1;
    }

    return mutex_release(tree, 0, ID/2 + (tree->size - 1)/2);
}
