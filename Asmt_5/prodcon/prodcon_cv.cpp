#include <pthread.h>
#include <sys/time.h>
#include <iostream>

using std::cout; 
using std::cerr; 
using std::endl;
using std::flush;

// mutex for critical sections
pthread_mutex_t mutex;

// condition variable to let the know the 
// consumer can now read
pthread_cond_t canRead_cv;
// and the associated mutex
pthread_mutex_t canRead_mutex;

// condition variable to let the know the 
// producer can now write
pthread_cond_t canWrite_cv;
// and the associated mutex
pthread_mutex_t canWrite_mutex;



// our buffer where the producers will write and the
// consumers will read.
int bufferlen;
char* buffer;

// the buffer only has one read/write head
int pos = 0;


// remove the last item written.
char RemoveItem()
{
  char item;

  pthread_mutex_lock(&mutex);
  item = buffer[pos--];

  // draw a space over the last character drawn,
  // and backup one
  cout << "\E[1D \E[1D" << flush;

  pthread_mutex_unlock(&mutex);

  return item;
}

// move the head over one place and write to that spot
// no error checking is done here on purpose.
void InsertItem(char item)
{
  pthread_mutex_lock(&mutex);
  buffer[++pos] = item;
  cout << item << flush;
  pthread_mutex_unlock(&mutex);
}


void Sleep(float wait_time_ms)
{
  // add randomness
  wait_time_ms = (rand() / (float(RAND_MAX)+1.0f))*2.0*wait_time_ms;
#ifdef WIN32
  _sleep(int(wait_time_ms));
#else
  usleep(int(wait_time_ms * 1E3f)); // convert from ms to us
#endif
}


/**
 * this is the start routine for the producer thread
 */
void* ProducerStart(void* arg)
{
  while(true) {

    // locking on this mutex, because we need to ensure
    // that pos doesn't change in here.
    pthread_mutex_lock(&mutex);

    // if the consumer can read now, let it know
    if (pos > 0) {
      pthread_mutex_lock(&canRead_mutex);
      pthread_cond_signal(&canRead_cv);
      pthread_mutex_unlock(&canRead_mutex);
    }

    pthread_mutex_unlock(&mutex);

    // wait until we can write
    pthread_mutex_lock(&canWrite_mutex);
    pthread_cond_wait(&canWrite_cv, &canWrite_mutex);
    pthread_mutex_unlock(&canWrite_mutex);

    // so this doesn't go too quickly,
    // and to add some randomness
    Sleep(500);

    // write something
    InsertItem('A');
  }

  // yeah, it'll never get here, but just to be complete...
  pthread_exit(NULL);
}

/**
 * this is the start routine for the producer thread
 */
void* ConsumerStart(void* arg)
{
  while(true) {

    // locking on this mutex, because we need to ensure
    // that pos doesn't change in here.
    pthread_mutex_lock(&mutex);

    // if the producer can write now, let it know
    if (pos < bufferlen) {
      pthread_mutex_lock(&canWrite_mutex);
      pthread_cond_signal(&canWrite_cv);
      pthread_mutex_unlock(&canWrite_mutex);
    }

    pthread_mutex_unlock(&mutex);

    // wait until we can read
    pthread_mutex_lock(&canRead_mutex);
    pthread_cond_wait(&canRead_cv, &canRead_mutex);
    pthread_mutex_unlock(&canRead_mutex);

    // so this doesn't go too quickly,
    // and to add some randomness
    Sleep(500);

    // read something
    char item = RemoveItem();
  }

  // yeah, it'll never get here, but just to be complete...
  pthread_exit(NULL);
}


// just some ANSI escape sequences to make the buffer
// pretty
void DrawEmptyBuffer()
{
  // move down a line.
  cout << endl;

  // try to center this. assume an 80 character-wide terminal
  int column = (80-bufferlen) / 2 - 1;
  cout << "\E[" << column << "C";

  // draw an empty red on blue buffer
  cout << "\E[31;44m";
  for (int i = 0; i < bufferlen; i++) {
    cout << " ";
  }
  // now go back to the beginning
  cout << "\E[" << bufferlen << "D" << flush;
}


int main(int argc, char* argv[])
{
  if (argc != 2) {
    cerr << "Usage: " << argv[0]
	 << " buffer_length" << endl;
    exit(1);
  }

  // seed the random number generator
  timeval tv;
  gettimeofday(&tv, NULL);
  srand(tv.tv_usec);

  
  // create the buffer
  // to be written by the producer
  // and read by the consumer
  bufferlen = atoi(argv[1]);  
  buffer = new char[bufferlen];

  DrawEmptyBuffer();

  // initialize the mutexes and condition variables
  pthread_mutex_init(&mutex, NULL);
  pthread_mutex_init(&canRead_mutex, NULL);
  pthread_mutex_init(&canWrite_mutex, NULL);
  pthread_cond_init(&canRead_cv, NULL);
  pthread_cond_init(&canWrite_cv, NULL);



  // explicitly make the threads joinable
  pthread_attr_t thread_attr;
  pthread_attr_init(&thread_attr);
  pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_JOINABLE);


  // create bufferlen producers, and bufferlen consumers

  // keep one thread around to join on, since none of them
  // will ever terminate
  pthread_t first_thread;

  for (int i = 0; i < bufferlen; i++) {
    pthread_t prod_thread, con_thread;
    pthread_create(&prod_thread, &thread_attr,
		   ProducerStart, NULL);
    pthread_create(&con_thread, &thread_attr,
		   ConsumerStart, NULL);
    
    if (i == 0)
      first_thread = prod_thread;
  }


  // wait for the threads to finish (which they never will...)
  pthread_join(first_thread, NULL);

}
