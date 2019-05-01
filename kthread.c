#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "kthread.h"


int kthread_create(void (*start_func)(), void* stack) {
  struct proc *curproc = myproc();
  struct thread *t;
  char * sp;
  acquire(&ptable.lock);
  t = searchThreadByStatus(myproc(), T_UNUSED);
  t->state = T_EMBRYO;
  t->tid = nexttid++;
  t->proc = curproc;
  release(&ptable.lock);

  // Allocate kernel stack.
  if ((t->kstack = kalloc()) == 0) {
    t->state = T_UNUSED;
    return -1;
  }
  sp = t->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *t->tf;
  t->tf = (struct trapframe *)sp;

  // Set up new context to start executing at start_func,
  // which returns to trapret.
  sp -= 4;
  *(uint *) sp = (uint) trapret;

  sp -= sizeof *t->context;
  t->context = (struct context *) sp;
  memset(t->context, 0, sizeof *t->context);
  t->context->eip = (uint)forkret;

  //init thread trapframe
  memset(t->tf, 0, sizeof(*t->tf));
  t->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  t->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  t->tf->es = t->tf->ds;
  t->tf->ss = t->tf->ds;
  t->tf->eflags = FL_IF;
  //struct thread *currthread = mythread();
  //*t->tf = *currthread->tf;

  t->tf->esp = (uint)stack;
  t->tf->eip = (uint)start_func; // beginning of initcode.S
  acquire(&ptable.lock);
  t->state = RUNNABLE;
  release(&ptable.lock);

  return t->tid;
}

int kthread_id() {
  return mythread()->tid;
}

void kthread_exit() {
  struct proc *curproc = myproc();
  struct thread *currThread = mythread();

  acquire(&ptable.lock);

  if (countRunnableThreads(curproc) > 1) {
    currThread->state = T_ZOMBIE;
    wakeup(currThread);
    sched();
  } else {
    if (curproc == initproc)
      panic("init exiting");

    int fd;
    // Close all open files.
    release(&ptable.lock)
    for (fd = 0; fd < NOFILE; fd++) {
      if (curproc->ofile[fd]) {
        fileclose(curproc->ofile[fd]);
        curproc->ofile[fd] = 0;
      }
    }

    begin_op();
    iput(curproc->cwd);
    end_op();
    curproc->cwd = 0;

    acquire(&ptable.lock);

    // Parent might be sleeping in wait().
    wakeup1(curproc->parent);

    struct proc *p;
    // Pass abandoned children to init.
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
      if (p->parent == curproc) {
        p->parent = initproc;
        if (p->state == ZOMBIE)
          wakeup1(initproc);
      }
    }

    // Jump into the scheduler, never to return.
    curproc->state = ZOMBIE;
    currThread->state = T_ZOMBIE;
    sched();
    panic("k thread zombie exit");

    release(&ptable.lock)
  }
}

int kthread_join(int thread_id) {
  struct proc * currProc = myproc();
  struct thread *t;

  if(thread_id == mythread()->tid) {
    return -1;
  }

  acquire(&ptabl#include "kthread.h"

  int found = 0;
  for (t = currProc->pthreads; t < &currProc->pthreads[NTHREAD]; t++) {
    if (t->tid == thread_id) {
      found = 1;
      break;
    }
  }

  if (found == 0) {
    release(&ptable.lock);
    return -1;
  }

  if (t->state == T_ZOMBIE) {				//TODO: add release?
    return 0;
  }

  while ((t->state != T_ZOMBIE) && (t->state != T_UNUSED)) {
    sleep(t, &ptable.lock);
    if (currProc->killed != 0) {
      release(&ptable.lock);
      return -1;
    }
  }

  release(&ptable.lock);

  return 0;
}

int kthread_mutex_alloc(){
	struct kthread_mutex_t *mut;

	acquire(&mtable.lock);

	for (mut = mtable.mutex_arr; mut < &mtable.mutex_arr[MAX_MUTEXES]; mut++)
		if (mut->state == M_UNUSED)
			goto found;

	release(&ptable.lock);
	return -1;

	found:
	mut->state = M_INUSE;
	mut->locked = 0;
	mut->mid = nextmid++;
	mut->thread = mythread();

	release(&mtable.lock);

	return mut->mid;
}

int kthread_mutex_dealloc(int mutex_id){
	struct kthread_mutex_t *mut;

	acquire(&mtable.lock);

	for (mut = mtable.mutex_arr; mut < &mtable.mutex_arr[MAX_MUTEXES]; mut++){
		if (mut->mid == mutex_id){
			if(!(mut->locked)){						//The given mutex is currently unlocked
				mut->state = M_UNUSED;
				mut->thread = 0;
				release(&mtable.lock);
				return 0;
			}
		}
	}
	release(&mtable.lock);					// dealloc failed
	return -1;
}

int kthread_mutex_lock(int mutex_id){
	struct kthread_mutex_t *mut;
	//struct thread *currThread = mythread();

	acquire(&mtable.lock);

	for (mut = mtable.mutex_arr ; mut < &mtable.mutex_arr[MAX_MUTEXES]; mut++) {
        if (mut->mid == mutex_id)
			goto found;
	}
	release(&mtable.lock);							// not found
	return -1;

	found:

	if (mut->state == M_UNUSED){				// the mutex is unused, failed.
		release(&mtable.lock);
		return -1;
	}

    while (mut->locked) {
        sleep(mut, &mut->lock);
    }
    mut->locked = 1;
    mut->thread = mythread();

	release(&mtable.lock);
	return 0;
}

int kthread_mutex_unlock(int mutex_id){
	struct kthread_mutex_t *mut;
	//struct thread *currThread = mythread();

	acquire(&mtable.lock);

	for (mut = mtable.mutex_arr ; mut < &mtable.mutex_arr[MAX_MUTEXES]; mut++) {
         if (mut->mid == mutex_id)
			goto found;
	}
	release(&mtable.lock);							// not found
	return -1;

	found:

	if (mut->state == M_UNUSED || !(mut->locked)){				// the mutex is unused or unlocked, failed.
		release(&mtable.lock);
		return -1;
	}

	if(mut->locked){
        if(mut->thread == mythread()){			// the calling thread is the owner thread
            mut->locked = 0;
            mut->thread = 0;
            wakeup(mut);

            release(&mtable.lock);
            return 0;
        }
	}

	release(&mtable.lock);
	return -1;
}
