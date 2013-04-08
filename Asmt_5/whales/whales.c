
/*

Sami Fekadu - sf5fw
Thomas Foulds - tcf9bj
Amanda Ray - ajr2fu


*/


#define _BSD_SOURCE  // usleep is from BSD

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



typedef struct whale{
  int    id;
  struct whale *next;
  struct whale *tail;

} whale_t;



whale_t *maleQ;
whale_t *femaleQ;


//Initialize Whale Queues
void whaleInit(){

	maleQ=(whale_t*) malloc(sizeof(whale_t));
	femaleQ=(whale_t*) malloc(sizeof(whale_t));

	maleQ->next=NULL;
	maleQ->tail=maleQ;
	maleQ->id=0;

	femaleQ->next=NULL;
	femaleQ->tail=femaleQ;
	femaleQ->id=0;

}


double elapsed_time() {
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return (tv.tv_sec - time0.tv_sec + (tv.tv_usec - time0.tv_usec) / 1.e6 );
}

sem_t male_avail;
sem_t fem_avail;
sem_t maker_avail;


int male;
int female;




void create_male(int id) {

	LOCKED_PRINTF("(t=%f) %d: %s\n", elapsed_time(), id, "Male Created");

	//is a pair ready to be matched?
	if((male > 0) && (female > 0))
	{
		sem_post(&maker_avail);
		LOCKED_PRINTF("(t=%f) %d: %s\n", elapsed_time(), id, "Became MatchMaker");
		LOCKED_PRINTF("(t=%f) %d: %s\n", elapsed_time(), id, "Mated");
		
		if(maleQ->next!=NULL && femaleQ->next!=NULL)		
		{
			maleQ->next->tail=maleQ->tail;
			maleQ=maleQ->next;
			femaleQ->next->tail=femaleQ->tail;
			femaleQ=femaleQ->next;
			
		}
		else{
			maleQ=NULL;
			

			femaleQ=NULL;
			

			}

		male--;
		female--;
	}
	//otherwise is there a female sem_waiting for a mate?
	else if (female > 0)
	{

//found mate
		LOCKED_PRINTF("(t=%f) %d: %s\n", elapsed_time(), id, "Found Mate");
		LOCKED_PRINTF("(t=%f) %d: %s\n", elapsed_time(), femaleQ->id, "Found Mate");
		male++;

		whale_t *temp=(whale_t*) malloc(sizeof(whale_t));
		temp->id =id;

maleQ->tail->next=temp;
maleQ->tail=temp;
maleQ->tail->next=NULL;







		sem_post(&male_avail);
		sem_wait(&maker_avail);
	}
	else
	{


	whale_t *temp=(whale_t*) malloc(sizeof(whale_t));
		temp->id =id;

maleQ->tail->next=temp;
maleQ->tail=temp;
maleQ->tail->next=NULL;


		male++;
		sem_post(&male_avail);
		sem_wait(&fem_avail);
		sem_wait(&maker_avail);
	}


}

void create_female(int id) {
	LOCKED_PRINTF("(t=%f) %d: %s\n", elapsed_time(), id, "Female Created");

	//is a pair ready to be matched?
	if((male > 0) && (female > 0))
	{

		sem_post(&maker_avail);
		LOCKED_PRINTF("(t=%f) %d: %s\n", elapsed_time(), id, "Became MatchMaker");
		LOCKED_PRINTF("(t=%f) %d: %s\n", elapsed_time(), id, "Mated");


		if(maleQ->next!=NULL && femaleQ->next!=NULL)		
		{
			maleQ->next->tail=maleQ->tail;
			maleQ=maleQ->next;
			femaleQ->next->tail=femaleQ->tail;
			femaleQ=femaleQ->next;
			
		}
		else{
			maleQ=NULL;
			

			femaleQ=NULL;
			

			}


		male--;
		female--;
	}
	//otherwise is there a male sem_waiting for a mate?
	else if (male > 0)
	{


		LOCKED_PRINTF("(t=%f) %d: %s\n", elapsed_time(), maleQ->id, "Found Mate");
		LOCKED_PRINTF("(t=%f) %d: %s\n", elapsed_time(), id, "Found Mate");

			whale_t *temp=(whale_t*) malloc(sizeof(whale_t));
		temp->id =id;

femaleQ->tail->next=temp;
femaleQ->tail=temp;
femaleQ->tail->next=NULL;


		female++;
		sem_post(&fem_avail);
		sem_wait(&maker_avail);
	}
	else
	{


		

whale_t *temp=(whale_t*) malloc(sizeof(whale_t));
		temp->id =id;

femaleQ->tail->next=temp;
femaleQ->tail=temp;
femaleQ->tail->next=NULL;


		female++;
		sem_post(&fem_avail);
		sem_wait(&male_avail);
		sem_wait(&maker_avail);
	}


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

void whale_sleep(int sem_wait_time_ms) {
	usleep(sem_wait_time_ms * 1000); // convert from ms to us
}

void usage(char *arg0) {
	fprintf(stderr, "Usage: %s num_whales sem_wait_time\n"
					"\tnum_whales - the total number of whales to create\n"
	 				"\tsem_wait_time - the amount of time to sem_wait before creating another whale (seconds)\n",
					arg0);
	exit(-1);
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		usage(argv[0]);
	}
	whaleInit();
	pthread_mutex_init(&stdout_lock, NULL);

	// seed the random number generator
	struct timeval tv;
	gettimeofday(&tv, NULL);
	srand(tv.tv_usec);

	gettimeofday(&time0, NULL);

	int num_whales = atoi(argv[1]);
	int sem_wait_time_ms = atof(argv[2]) * 1000;

	// initialize the semaphores
  	sem_init(&male_avail, 0, 0);
 	sem_init(&fem_avail, 0, 0);
  	sem_init(&maker_avail, 0, 0);

	pthread_attr_t thread_attr;
	pthread_attr_init(&thread_attr);
	pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);

	for (int whale = 0; whale < num_whales; whale++) {
		whale_sleep(sem_wait_time_ms);
		pthread_t whale_thread_id;
		pthread_create(&whale_thread_id, &thread_attr, create_whale, (void *) &whale);
	}

	// this should enough time to resolve the matches
	// than can be resolved
	whale_sleep(sem_wait_time_ms * 10.0);
	
	pthread_mutex_destroy(&stdout_lock);
}
