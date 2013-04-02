#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <iostream>

using std::cout; 
using std::cerr; 
using std::endl;
using std::flush;

// mutex for critical sections
pthread_mutex_t mutex;

// semaphore signifying how many reads the 
// consumer can safely do
sem_t canRead;

// semaphore signifying how many writes the
// producer can safely complete
sem_t canWrite;

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
    // wait until I can write
    sem_wait(&canWrite);

    // so this doesn't go too quickly,
    // and to add some randomness
    Sleep(500);

    // write something
    InsertItem('A');

    // let the reader know there's something to be read
    sem_post(&canRead);
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
    // wait until I can read
    sem_wait(&canRead);

    // so this doesn't go too quickly,
    // and to add some randomness
    Sleep(500);

    // read something
    char item = RemoveItem();

    // let the writer know there's some space available
    sem_post(&canWrite);
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

  // initialize the mutex and semaphores
  // You should add error checking!
  pthread_mutex_init(&mutex, NULL);
  sem_init(&canRead, 0, 0);
  sem_init(&canWrite, 0, bufferlen);



  // explicitly make the threads joinable
  pthread_attr_t thread_attr;
  pthread_attr_init(&thread_attr);
  pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_JOINABLE);


  // create the two threads
  pthread_t prod_thread, con_thread;
  pthread_create(&prod_thread, &thread_attr,
		 ProducerStart, NULL);
  pthread_create(&con_thread, &thread_attr,
		 ConsumerStart, NULL);


  // wait for the threads to finish (which they never will...)
  pthread_join(prod_thread, NULL);
  pthread_join(con_thread, NULL);
}
