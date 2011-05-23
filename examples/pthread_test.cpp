#include "../ThreadedEventCounter.h"
#include "../ThreadedEventCounter.cpp"
#include <pthread.h>
#include <map>
#include <vector>


/*
 * See the OPENMP version for further options and documentation
 * This is only intended to show the differences between openmp and
 * pthreads usage of the library.
 * 
 */

enum Events {
	EVENT_A,
	EVENT_B
};


pthread_barrier_t barrier;
void* thread_kernel(void *args) ;
struct ThreadArgs {
	int thread_num;
};

int maxFrames;
char singleExecuteLock[64]; // make sure that the lock variable is on it's own cache line

int main() {
	
	int maxThreads = 10;
	int numEvents = 2;
	maxFrames = 3;
	
	
	pthread_barrier_init(&barrier, NULL, maxThreads);
	// lock for incrementing the frame counter
	singleExecuteLock[0] = 0;
	
	
	/*
	* KEY DIFFERENCES BETWEEN OPENMP USE AND PTHREAD USE
	* 
	* Need to initialize with pthread_self
	* 
	* Also need to give each thread an ID value between 0 and numThreads
	*/
	EventTimer::initialize(maxThreads,numEvents,maxFrames,(void*)pthread_self);

	// This is the other difference; Each thread needs to know it's thread number (0-numThread)
	// cannot use pthread_self as the id as not within range
	std::vector<ThreadArgs> args(maxThreads);
	for (int i = 0; i < maxThreads; ++i) {
		args[i].thread_num = i;
	}

	// Spawn Threads
	std::vector<int> threadReturnVals(maxThreads);	
	std::vector<pthread_t> thread(maxThreads);
	for (int i = 0; i < maxThreads; ++i) {
		threadReturnVals[i] = pthread_create(&(thread[i]), NULL, thread_kernel, &(args[i]));
	}
	
	// Wait for them to finish
	for (int i = 0; i < maxThreads; i++) {
		pthread_join(thread[i], NULL);		
	}


	std::map<int,std::string> eventList;
	eventList[EVENT_A] = "Event A";
	eventList[EVENT_B] = "Event B";
	EventTimer::printTimes(&eventList);


}



void* thread_kernel(void *args) {
	ThreadArgs *my_args;
	my_args = (ThreadArgs*) args;
	int myTid = (*my_args).thread_num;
	
	// Make sure to register the thread in order to count hw events
	EventTimer::registerThread(myTid);
	
	pthread_barrier_wait(&barrier);
	for (int i = 0; i < maxFrames; i++) {
		EventTimer::start(myTid, EVENT_A);
		for (int k = 0; k < 100000; k++);
		EventTimer::stop(myTid, EVENT_A);
		
		EventTimer::start(myTid, EVENT_B);
		for (int k = 0; k < 100000; k++);
		EventTimer::stop(myTid, EVENT_B);
		
		
		// The following ensures that only 1 thread advances the frame
		// Reset of the lock needs to go after the 2nd barrier to ensure
		// the frame doesn't get accidencly advanced twice.
		pthread_barrier_wait(&barrier);
		if (__sync_bool_compare_and_swap(&(singleExecuteLock[0]), 0, 1)) {
			EventTimer::advanceFrame();
		}
		pthread_barrier_wait(&barrier);
		singleExecuteLock[0]=0;
	}
}
