#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include "shm.h"

static struct memory *mem = NULL;

int main(const int argc, char * const argv[]){

  if(argc != 3){
    fprintf(stderr, "Usage: ./bin_adder xx yy\n");
    return -1;
  }

  mem = shm_attach(0, 0);
	if(mem == NULL){
		return 1;
	}
  const int xx = atoi(argv[1]); //index number starting the list
  const int yy = atoi(argv[2]); //number of integers to be added

  FILE * log = fopen("adder_log.txt", "a");
  if(log == NULL){
		perror("freopen");
		return -1;
	}

  //log when we start adding
  sem_wait(&mem->lock);
  //sleep(1); //UNCOMMENT LATER
    fprintf(log, "PID=%d xx=%d yy=%d at time %d s %d ns\n", getpid(), xx, yy, mem->clock.s, mem->clock.ns);
  //sleep(1); //UNCOMMENT LATER
  sem_post(&mem->lock);

  //add the numbers
  int i;
  for(i=1; i < yy; i++){
    mem->numbers[xx] += mem->numbers[xx+i];
  }

  sem_wait(&mem->lock);

  //sleep(1); //UNCOMMENT LATER
    fprintf(log, "PID=%d result=%d at time %d s %d ns\n", getpid(), mem->numbers[xx], mem->clock.s, mem->clock.ns);
    fflush(log);
    fclose(log);
  //sleep(1); //UNCOMMENT LATER
  sem_post(&mem->lock);

  shm_detach(0);
  return 0;
}
