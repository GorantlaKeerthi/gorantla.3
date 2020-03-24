#include <semaphore.h>
#include "clock.h"

#define USERS_COUNT 20

struct memory {
	sem_t lock;						//for adder_log and shared clock
	struct clock clock;

	unsigned int numbers_count;
	unsigned int numbers[1];	//actual size is not 1
};

struct memory * shm_attach(const int flags, const unsigned int nlines);
						int shm_detach(const int clear);
