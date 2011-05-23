#include "../ThreadedEventCounter.h"

// This includes the declaration of static members
// If you have multiple source files, you'll want to compile
// and link this file separately
#include "../ThreadedEventCounter.cpp"

#include <omp.h>
#include <map>

// Needed if you want to declare custom hardware events
//#include <papi.h>


// The events that you want to track should be enumerated
enum Events {
	EVENT_A,
	EVENT_B
};

int main() {

	int maxThreads = omp_get_max_threads();
	int numEvents = 2;
	int maxFrames = 3;
	
	// To use the events to be tracked are: PAPI_TOT_CYC, PAPI_TOT_INS, and PAPI_L1_TCM:
	EventTimer::initialize(maxThreads,numEvents,maxFrames,(void*)omp_get_thread_num);
	
	// Or if you are sure that you only want to use the timing part:
	//
	// EventTimer::initialize(maxThreads,numEvents,maxFrames);
	
	// Or you can specify your own events, but make sure that they
	// can be measured on a per core instance. You can get the available 
	// event by calling papi_avail from the command line
	//
	// If you register more than 4 events, you need to define MAX_HW_EVENTS during compilation.
	// 
	// Make sure to include <papi.h>
	//
	// std::vector<int> myEvents;
	// myEvents.push_back(PAPI_TOT_INS);
	// myEvents.push_back(PAPI_L1_TCM);
	// 
	// EventTimer::initialize(maxThreads, numEvents, maxFrames, (void*)omp_get_thread_num, &myEvents);


	// This is only needed if you want to count hw events
	// Each thread that you want to track must call this function
	#pragma omp parallel for
	for (int i = 0; i < maxThreads; i++) {
		int tid = omp_get_thread_num();
		EventTimer::registerThread(tid);
	}


	// This would be equivilant to the main program loop
	for (int i = 0; i < maxFrames; i++) {
		#pragma omp parallel for
		for (int j = 0; j < maxThreads; j++) {
			int tid = omp_get_thread_num();
			EventTimer::start(tid, EVENT_A);
			for (int k = 0; k < 100000; k++);
			EventTimer::stop(tid, EVENT_A);
		}
		
		// You can have multiple event instances
		#pragma omp parallel for
		for (int j = 0; j < 2*maxThreads; j++) {
			int tid = omp_get_thread_num();
			EventTimer::start(tid, EVENT_B);
			for (int k = 0; k < 100000; k++);
			EventTimer::stop(tid, EVENT_B);
		}
		
		// *** make sure to call this at the end of the main loop.
		EventTimer::advanceFrame();
	}
	
	
	// This is how you print out the results
	std::map<int,std::string> eventList;
	eventList[EVENT_A] = "Event A";
	eventList[EVENT_B] = "Event B";
	
	EventTimer::printTimes(&eventList);
	
	// Can replace the default tab by giving an ascii deliminator
	// char COMMA = 44;
	//
	// EventTimer::printTimes(&eventList,COMMA);
	
	// Can also give io stream if you want to print the results to a
	// file. Note, all previous arguments must be given values. The 
	// eventList can be null, in which case the event ID will be printed
	// A reference to the TAB character (EventTimer::TAB) is included 
	// for convience 
	// 
	// EventTimer::printTimes(NULL,EventTimer::TAB,std::cout);
	
	
}
