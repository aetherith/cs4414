#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/time.h>
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;

// for critical sections
pthread_mutex_t mutex;

sem_t o_avail;
sem_t h1_avail;
sem_t h2_avail;
sem_t h_both_avail;
int num_h;

// returns the approximate elapsed time since the first call
// (in seconds)
float ElapsedTime()
{
  timeval tv;

  gettimeofday(&tv, NULL);
  static timeval startTime = tv;

  return ( (tv.tv_sec - startTime.tv_sec) + 
	   float(tv.tv_usec-startTime.tv_usec) / 1E6f );
}

void H(int id)
{
  pthread_mutex_lock(&mutex);
  cout << "(t="<<ElapsedTime()<<") "<< id << ": H Created" << endl;
  pthread_mutex_unlock(&mutex);

  // am I the first or the second H?
  bool is_first;
  pthread_mutex_lock(&mutex);
  is_first = (num_h % 2 == 0);
  num_h++;
  pthread_mutex_unlock(&mutex);

  if (is_first) {
    sem_wait(&h2_avail);
  } else {
    sem_wait(&o_avail);
    sem_post(&h2_avail);
    sem_post(&h_both_avail);
  }
  
  pthread_mutex_lock(&mutex);
  cout << "(t="<<ElapsedTime()<<") "<< id << ": Water" << endl;
  pthread_mutex_unlock(&mutex);
}

void O(int id)
{
  pthread_mutex_lock(&mutex);  
  cout << "(t="<<ElapsedTime()<<") "<< id << ": O Created" << endl;
  pthread_mutex_unlock(&mutex);

  sem_post(&o_avail);
  sem_wait(&h_both_avail);

  pthread_mutex_lock(&mutex);
  cout << "(t="<<ElapsedTime()<<") "<< id << ": Water" << endl;
  pthread_mutex_unlock(&mutex);
}

void* CreateAtom(void* arg)
{
  int id = (int) arg;

  // is this an H or an O ?
  float r = rand() / (float(RAND_MAX) + 1.0f);
  if (r < 0.666)
    H(id);
  else
    O(id);

  pthread_exit(NULL);
}

void Sleep(float wait_time_ms)
{
#ifdef WIN32
  _sleep(int(wait_time_ms));
#else
  usleep(int(wait_time_ms * 1E3f)); // convert from ms to us
#endif
}

int main(int argc, char *argv[])
{
  if (argc != 3) {
    cerr << "Usage: " << argv[0] << " num_atoms wait_time"
	 << endl
	 << "  num_atoms - the total number of atoms to create"
	 << endl
	 << "  wait_time - the amount of time to wait before "
	 << "creating another atom (seconds)" << endl;
    exit(1);
  }

  // seed the random number generator
  timeval tv;
  gettimeofday(&tv, NULL);
  srand(tv.tv_usec);
  

  ElapsedTime(); // get the start time set.

  int num_atoms = atoi(argv[1]);
  float wait_time_ms = atof(argv[2]) * 1000.0;

  num_h = 0;

  // initialize the semaphores
  sem_init(&h1_avail, 0, 0);
  sem_init(&h2_avail, 0, 0);
  sem_init(&h_both_avail, 0, 0);
  sem_init(&o_avail, 0, 0);

  // initialize the thread attributes so the threads are
  // not joinable (so we can create a lot of them)
  pthread_attr_t thread_attr;
  pthread_attr_init(&thread_attr);
  pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);

  pthread_mutex_init(&mutex, NULL);

  for (int atom = 0; atom < num_atoms; atom++) {
    Sleep(wait_time_ms);
    pthread_t atom_thread_id;
    pthread_create(&atom_thread_id, &thread_attr,
		   CreateAtom, (void*) atom);
  }

  // this should enough time to resolve the matches
  // than can be resolved
  Sleep(1000.0);
}
