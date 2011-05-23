#include "../ThreadedEventCounter.h"
#include "../ThreadedEventCounter.cpp"

#include <map>
//#include <iostream>
//#include <papi.h>


// The events that you want to track should be enumerated
enum Events {
	EVENT_A,
	EVENT_B
};

int singleThreadIDfunction() {
	return 1;
}

int main() {

	int numEvents = 2;
	int maxFrames = 3;
	

	EventTimer::initialize(1,numEvents,maxFrames,(void*)singleThreadIDfunction);

	EventTimer::registerThread(0);

	for (int i = 0; i < maxFrames; i++) {
		EventTimer::start(0, EVENT_A);
		for (int k = 0; k < 100000; k++);
		EventTimer::stop(0, EVENT_A);
	
		
		EventTimer::start(0, EVENT_B);
		for (int k = 0; k < 100000; k++);
		EventTimer::stop(0, EVENT_B);

		
		// *** make sure to call this at the end of the main loop.
		EventTimer::advanceFrame();
	}
	
	
	// This is how you print out the results
	std::map<int,std::string> eventList;
	eventList[EVENT_A] = "Event A";
	eventList[EVENT_B] = "Event B";
	
	EventTimer::printTimes(&eventList);
}
