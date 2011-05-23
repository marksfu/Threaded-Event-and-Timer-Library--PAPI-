#include "../ThreadedEventCounter.h"
#include "../ThreadedEventCounter.cpp"
#include <omp.h>
#include <vector>
#include <papi.h>
#include <iostream>
#include <stdlib.h>

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cout << "Usage: ./overhead #threads #itterations" << std::endl;
	}
	
	int maxThreads = atoi(argv[1]);
	int maxFrames = atoi(argv[2]);
	
	omp_set_num_threads(maxThreads);
	std::vector<int> myEvents;
	myEvents.push_back(PAPI_TOT_CYC);
	myEvents.push_back(PAPI_TOT_INS);
	myEvents.push_back(PAPI_L1_TCM);
	EventTimer::initialize(maxThreads,1,maxFrames,(void*)omp_get_thread_num,&myEvents);

	#pragma omp parallel for
	for (int i = 0; i < maxThreads; i++) {
		int tid = omp_get_thread_num();
		EventTimer::registerThread(tid);
	}
	

	#pragma omp parallel for
	for (int j = 0; j < maxThreads; j++) {
		int tid = omp_get_thread_num();
		for (int i = 0; i < maxFrames; i++) {
			EventTimer::start(tid, 0, i);
			EventTimer::stop(tid, 0, i);
			__sync_synchronize();
		}
	}
	
	
	#ifdef HW_EVENTS
	std::vector<float> cycles(maxThreads);
	#endif
	std::vector<float> time(maxThreads);
	for (int i = 0; i < maxThreads; i++) {
		suseconds_t uStart, uStop;
		TimeEvent &s = EventTimer::data[i][0][0][0];
		TimeEvent &t = EventTimer::data[i][0][maxFrames-1][0];
		time[i] = ((float)((t.timeStop.tv_sec - s.timeStart.tv_sec)*1000000) + t.timeStop.tv_usec - s.timeStart.tv_usec)/maxFrames;
        #ifdef HW_EVENTS      
		cycles[i] = ((float)(EventTimer::data[i][0][maxFrames-1][0].eventStop[0] - EventTimer::data[i][0][0][0].eventStart[0]))/maxFrames;
		#endif
	}
	
	float ttime = 0.0f;
	for (int i = 0; i < maxThreads; i++) {
		ttime += time[i]*1000;
	}
	std::cout << "Threads: " << maxThreads << std::endl;
	std::cout << "Itterations: " << maxFrames << std::endl;
	std::cout << maxThreads*maxFrames << " pairs of start stop events" << std::endl;
	std::cout << std::endl << "Average per pair:" << std::endl;
	std::cout << ttime/maxThreads << " nsec" << std::endl;
	
	#ifdef HW_EVENTS
	float tcycles = 0.0f;
	for (int i = 0; i < maxThreads; i++) {
		tcycles += cycles[i];
	}
	std::cout << tcycles/maxThreads << " cycles" << std::endl;
	
	#endif

}
