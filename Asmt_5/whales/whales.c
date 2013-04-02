#define _BSD_SOURCE	// usleep is from BSD

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/time.h>

/**
 * the output strings MUST be of the form:
 * (t=TIME) WHALE_ID: ACTION
 * 
 * with NO leading or trailing spaces
 * 
 * TIME - the value of elapsed_time()
 * WHALE_ID - the id passed into Male(int id) and Female(int id)
 * ACTION - one of the followoing exact phrases (with no leading
 *					or trailing spaces):
 *
 *	Male Created
 *	Female Created
 *	Found Mate
 *	Became MatchMaker
 *	Mated
 *
 */

static pthread_mutex_t stdout_lock;

#define LOCKED_PRINTF(fmt, args...) \
	pthread_mutex_lock(&stdout_lock); \
	printf(fmt, args); \
	fflush(stdout); \
	pthread_mutex_unlock(&stdout_lock)

static struct timeval time0;

double elapsed_time() {
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return (tv.tv_sec - time0.tv_sec + (tv.tv_usec - time0.tv_usec) / 1.e6 );
}

void create_male(int id) {
	LOCKED_PRINTF("(t=%f) %d: %s\n", elapsed_time(), id, "Male Created");
}

void create_female(int id) {
	LOCKED_PRINTF("(t=%f) %d: %s\n", elapsed_time(), id, "Female Created");
}

void *create_whale(void* arg) {
	int id = *((int *) arg);

	// is this a male or a female?
	if (rand() & 1) { // odd for male
		create_male(id);
	} else { // even for female
		create_female(id);
	}

	pthread_exit(NULL);
}

void whale_sleep(int wait_time_ms) {
	usleep(wait_time_ms * 1000); // convert from ms to us
}

void usage(char *arg0) {
	fprintf(stderr, "Usage: %s num_whales wait_time\n"
					"\tnum_whales - the total number of whales to create\n"
	 				"\twait_time - the amount of time to wait before creating another whale (seconds)\n",
					arg0);
	exit(-1);
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		usage(argv[0]);
	}
	
	pthread_mutex_init(&stdout_lock, NULL);

	// seed the random number generator
	struct timeval tv;
	gettimeofday(&tv, NULL);
	srand(tv.tv_usec);

	gettimeofday(&time0, NULL);

	int num_whales = atoi(argv[1]);
	int wait_time_ms = atof(argv[2]) * 1000;

	pthread_attr_t thread_attr;
	pthread_attr_init(&thread_attr);
	pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);

	for (int whale = 0; whale < num_whales; whale++) {
		whale_sleep(wait_time_ms);
		pthread_t whale_thread_id;
		pthread_create(&whale_thread_id, &thread_attr, create_whale, (void *) &whale);
	}

	// this should enough time to resolve the matches
	// than can be resolved
	whale_sleep(wait_time_ms * 10.0);
	
	pthread_mutex_destroy(&stdout_lock);
}
