#define MAX_STACK_SIZE 4000
#define MAX_MUTEXES 64

struct trnmnt_tree {
  int size;
  int* threadNodes;
  int* mutexNodes;
  int currentNumberOfThreads;
  int currentNumberOfWaitings;
  struct spinlock lock;
};


struct trnmnt_tree* trnmnt_tree_alloc(int depth);
int trnmnt_tree_dealloc(struct trnmnt_tree* tree);
int trnmnt_tree_acquire(struct trnmnt_tree* tree,int ID);
int trnmnt_tree_release(struct trnmnt_tree* tree,int ID);
