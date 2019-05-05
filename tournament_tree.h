
typedef struct trnmnt_tree {
  int size;
  int* mutexNodes;      //mutexes id
  int* threadNodes;     //thread id nodes
}trnmnt_tree;


struct trnmnt_tree* trnmnt_tree_alloc(int depth);
int trnmnt_tree_dealloc(struct trnmnt_tree* tree);
int trnmnt_tree_acquire(struct trnmnt_tree* tree,int ID);
int trnmnt_tree_release(struct trnmnt_tree* tree,int ID);
