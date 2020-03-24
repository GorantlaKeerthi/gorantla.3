#include <sys/shm.h>

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

#include "shm.h"

struct val_max {
	int val;	//current value
	int max;	//maximum value
};

static struct memory *mem = NULL;
static FILE * log = NULL;

static unsigned int max_child = USERS_COUNT;

static struct val_max running = {0,2};	//running users
static struct val_max started = {0,4};	//started users
static struct val_max exited = {0, USERS_COUNT};	//exited users

static void check_wait_children(){
  pid_t pid;
	int status;
  while((pid = waitpid(-1, &status, WNOHANG)) > 0){
    running.val--;
    exited.val++;
    //sem_wait(&mem->lock);
    fprintf(log, "MASTER: Child %u has terminated at time %d s %d ns\n", pid, mem->clock.s, mem->clock.ns);
    //sem_post(&mem->lock);
  }
}

static void signal_handler(const int sig){

	switch(sig){
		//case SIGCHLD:

		//	break;
		case SIGINT: case SIGTERM: case SIGALRM:
			exited.val = USERS_COUNT; //set the flat to true, to interrupt main loop
      //TODO: kill all started processes
      break;
		default:
      sem_wait(&mem->lock);
			fprintf(log, "MASTER: Signal %d at time %d s %d ns\n", sig, mem->clock.s, mem->clock.ns);
      sem_post(&mem->lock);
			break;
	}
}

static int count_lines(const char * filename){
  int nlines = 0;

  FILE * fin = fopen(filename, "r");
  if(fin == NULL){
    perror("fopen");
    return -1;
  }

  while(!feof(fin)){
    if(fgetc(fin) == '\n'){
      nlines++;
    }
  }

  fclose(fin);
  return nlines;
}

static int load_lines(const char * filename){
  char line[100];

  FILE * fin = fopen(filename, "r");
  if(fin == NULL){
    perror("fopen");
    return -1;
  }

  //read each line and put number in shm
  while(!feof(fin)){
    fgets(line, sizeof(line), fin);
    mem->numbers[mem->numbers_count++] = atoi(line);
  }

  fclose(fin);
  return 0;
}

//Convert integer number to string
static char * num_arg(const unsigned int number){
	size_t len = snprintf(NULL, 0, "%d", number) + 1;
	char * str = (char*) malloc(len);
	snprintf(str, len, "%d", number);
	return str;
}

//Start a user process, if we can
static int exec_bin_adder(const unsigned int index, const unsigned int chunk_size){

	//check the limits on running/started users
	if(	(running.val >= running.max) ||
			(started.val >= started.max)){
		return 0;
	}

	char * xx = num_arg(index);
	char * yy = num_arg(chunk_size);

	//start the child process
	const pid_t pid = fork();
	if(pid < 0){
		perror("fork");
		return -1;

	}else if(pid == 0){
		execl("./bin_adder", "./bin_adder", xx, yy, NULL);
		perror("execl");
		exit(1);

	}else{

    //sem_wait(&mem->lock);
		//fprintf(log, "PID=%d Index=%s Size=%s at time %d s %d ns\n", pid, xx, yy, mem->clock.s, mem->clock.ns);
    //sem_post(&mem->lock);

		running.val++;
		started.val++;
	}

	free(xx);
	free(yy);
	return pid;
}

int main(const int argc, char * const argv[]){

  //if we don't have 2 arguments
  if(argc != 2){
    fprintf(stderr, "Usage: master <input.txt>\n");
    return -1;
  }

  signal(SIGINT,	signal_handler);
  signal(SIGALRM, signal_handler);
	//signal(SIGCHLD, signal_handler);

  log = fopen("adder_log.txt", "w");
  if(log == NULL){
		perror("freopen");
		return -1;
	}

  //alarm(100);

  //count the number of lines in file
  int nlines = count_lines(argv[1]);

  //allocate the shared memory
  mem = shm_attach(0600 | IPC_CREAT | IPC_EXCL, nlines);
	if(mem == NULL){
		return 1;
	}

  if(load_lines(argv[1]) == -1){
    return -1;
  }

  //determinte how much users to fork

  while(nlines >= 2){
    int index = 0;
    started.val = 0;
    exited.val = 0;

    started.max = (nlines / 2);   // n/2 pairs
    unsigned int chunk_size = 2;  //each process gets a pair of ints
    if(index == ((started.max-1)*chunk_size) ){ //if we are last chunk
      chunk_size += nlines % 2;     //add the remainder
    }

    sem_wait(&mem->lock);
  	fprintf(log, "nlines=%d at time %d s %d ns\n", nlines, mem->clock.s, mem->clock.ns);
    sem_post(&mem->lock);

    //simulation loop
  	while(exited.val < started.max){


    	if(exec_bin_adder(index, chunk_size) > 0){
        index += chunk_size;
        if(index == ((started.max-1)*chunk_size) ){ //if we are last chunk
          chunk_size += nlines % 2;     //add the remainder
        }
      }

      sem_wait(&mem->lock);
  		clock_add_ns(&mem->clock, 10000);
      check_wait_children();
  		sem_post(&mem->lock);
  	}

    //move all results to beginning of array
    int i, j=1;
    for(i=1; i < exited.val; i++, j++){ //for each executed process
      mem->numbers[j] = mem->numbers[(i*2)];  //moved result forward
    }
    nlines /= 2;  //reduce the number of lines in half
  }

  sem_wait(&mem->lock);
	fprintf(log, "Simulation done, %d children done at time %d s %d ns\n", exited.val, mem->clock.s, mem->clock.ns);
  fprintf(log, "Result = %d\n", mem->numbers[0]);
  fclose(log);
  sem_post(&mem->lock);

  printf("Result = %d\n", mem->numbers[0]);

  shm_detach(1);

  return 0;
}
