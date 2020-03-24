#include <string.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "shm.h"

static struct memory * mem = NULL;
static int shmid = -1;

struct memory * shm_attach(const int flags, const unsigned int nlines){

  /* Create or get a shared memory */
	unsigned int shm_size = sizeof(struct memory)  + (nlines*sizeof(int));
	if(nlines == 0){
		shm_size = 0;
	}
	shmid = shmget(ftok("shm.c", 1), shm_size, flags);
	if (shmid == -1) {
		perror("shmget");
		return NULL;
	}

	/* Attach to the shared region */
	mem = (struct memory*)shmat(shmid, (void *)0, 0);
	if (mem == (void *)-1) {
		perror("shmat");
		return NULL;
	}

	if(flags){ /* if we create the memory */
		memset(mem, 0, shm_size);
  	if(sem_init(&mem->lock, 1, 1) == -1){
  		perror("sem_init");
  		return NULL;
  	}
  }

	return mem;
}

int shm_detach(const int clear){

  if(mem == NULL){
    fprintf(stderr, "Error: shm is already cleared\n");
    return -1;
  }

  if(clear){
    if(sem_destroy(&mem->lock) == -1){
      perror("sem_destroy");
    }
  }

  if(shmdt(mem) == -1){
    perror("shmdt");
  }

  if(clear){
	  if(shmctl(shmid, IPC_RMID,NULL) == -1){
      perror("shmctl");
    }
  }

  mem = NULL;
  shmid = -1;

  return 0;
}
