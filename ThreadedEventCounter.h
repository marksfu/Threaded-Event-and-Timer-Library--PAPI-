
#pragma once
#ifndef THREADED_EVENT_COUNTER
#define THREADED_EVENT_COUNTER

#include <sys/time.h>
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <assert.h>

#ifdef HW_EVENTS
	#include <papi.h>
	#include <pthread.h>
	#include <omp.h>
	#ifndef MAX_EVENTS
		#define MAX_HW_EVENTS 4
	#endif // Max_Events
#endif //HW_EVENTS

#define TAB_CHAR 9
#define MILLION 1000000
#define DEFAULT_EVENT_INSTANCES 1

struct TimeEvent {
	timeval timeStart;
	timeval timeStop;
#ifdef HW_EVENTS
	long_long eventStart[MAX_HW_EVENTS];
	long_long eventStop[MAX_HW_EVENTS];
#endif //HW_EVENTS
};



/*! --------------------------------------------------------------------
 *
 * ThreadedEventCounter.hpp
 *
 * Timing code to measure event times and hw counters
 * 
 * Compile flags:
 * 
 * HW_EVENTS - enable PAPI counters, must use -lpapi with compile
 * 
 * DISABLE_EVENT_COUNTER - make all function bodies empty
 * 
 * 
 *	loop {
 *		eventA();
 *		eventB();
 *		eventC();
 *	} while(condition);
 *
 *Timing code goes around the events.
 *
 * Make sure to initialize using: EventTimer::initialize(int, int)
 * 
 ---------------------------------------------------------------------*/
class EventTimer {
private:
	EventTimer() {}
	
	static std::vector<int> eventSet;
	static std::vector<int> hwEvents;
	static timeval timeInit;
	static std::vector<int> maxEventInstances;
	static int frameCnt;
	
public:
	/*! \brief Ascii Tab value
	 */
	static char TAB;
	/*! \brief Data container
	 * 
	 *  data[threadNum][eventNum][frameNum][eventInstance]
	 */
	static std::vector<std::vector<std::vector<std::vector <TimeEvent> > > > data;
	
	
private:
	static void initializeTimer(int numThreads, int numEvents, int numFrames) {
#ifndef DISABLE_EVENT_COUNTER
		TAB = TAB_CHAR;
		// Allocate memory for data
		assert(data.size()==0);
		maxEventInstances.resize(numEvents);
		for (int i = 0; i < numEvents; i++) {
			maxEventInstances[i] = DEFAULT_EVENT_INSTANCES;
		}
		frameCnt = 0;
		data.resize(numThreads);
		for (int thread = 0; thread < numThreads; thread++) {
			data[thread].resize(numEvents);
			for (int event = 0; event < numEvents; event++) {
				data[thread][event].resize(numFrames);
				for (int frame = 0; frame < numFrames; frame++) {
					// must reserve something. Contrary to what is
					// posted online, vectors can be initialized with 0
					// capacity.
					data[thread][event][frame].reserve(DEFAULT_EVENT_INSTANCES);
				}
			}
		}
       
		// get the starting time so we can subtract it 
		// from the measurments in order to reduce print out size
		gettimeofday(&timeInit, NULL);
#endif //DISABLE_EVENT_COUNTER
	}
	
	
public:

	/*! \brief Static class initializer
	 * 
	 * \param numThreads an integer to define the maximum number of threads
	 * \param numEvents an integer for the maximum number of unique events
	 * \param numFrames an integer for max frames / program loops
	 * \param _hwEvents a pointer to a vector of PAPI event (must be able to be 
	 *        measured on a per chip instance. If no pointer is given, then the following
	 *        events are used: PAPI_TOT_CYC, PAPI_TOT_INS, PAPI_L1_TCM
	 * \return Nothing
	 * 
	 * hwEvents is a given default value so that the same initializer can be
	 * used for timing only.
	 */
	static void initialize(int numThreads, int numEvents, int numFrames, void* threadID_Func=NULL, std::vector<int>* _hwEvents = NULL) {
#ifndef DISABLE_EVENT_COUNTER
		initializeTimer(numThreads, numEvents, numFrames);
		
#ifdef HW_EVENTS		
		eventSet.resize(numThreads);
		int retval = PAPI_library_init(PAPI_VER_CURRENT);
		assert(retval == PAPI_VER_CURRENT && "Error: PAPI not the version this binary was compiled with");
		
		// these thread ID functions do not have to be associated with the thread enumerated id of the
		// program. They are only required so that PAPI can distinguish between thread events.
		// The user is still responsible for handing an enumerated thread ID to the timing library
		retval = PAPI_thread_init((long unsigned int(*)())threadID_Func);

		
		assert(retval == PAPI_OK && "Error: Couldn't intialize thread support");
		
		if (_hwEvents == NULL) {
			hwEvents.push_back(PAPI_TOT_CYC);
			hwEvents.push_back(PAPI_TOT_INS);
			hwEvents.push_back(PAPI_L1_TCM);
		} else {
			hwEvents = *_hwEvents;
		}
		
#endif //HW_EVENTS
#endif //DISABLE_EVENT_COUNTER
	}
	
	/*! \brief Start an event
	 *
	 * \param thread an integer to specify the thread id; must be between 0 and numThreads
	 * \param event an integer to specify the event id; must be between 0 and numEvents
	 * \param frame an integer to specify the frame id; must be between 0 and numFrames. Defaults to the current frame counter 
	 */
	static void start(int thread, int event, int frame=frameCnt) {
#ifndef DISABLE_EVENT_COUNTER
		// if you get a segfault from calling this function, double check to make sure
		// that HRtimer::initialize() was called
		
		TimeEvent t;
		gettimeofday(&t.timeStart, NULL);
		__sync_synchronize();
#ifdef HW_EVENTS
		// be careful of overflowing the event counter; depends on the number of events registered
		int ret = PAPI_read(eventSet[thread], t.eventStart);
		assert(ret == PAPI_OK && "Error reading values");
#endif //HW_EVENTS
		data[thread][event][frame].push_back(t);
#endif //DISABLE_EVENT_COUNTER
	}

	/*! \brief Stops an event
	 *
	 * \param thread an integer to specify the thread id; must be between 0 and numThreads
	 * \param event an integer to specify the event id; must be between 0 and numEvents
	 * \param frame an integer to specify the frame id; must be between 0 and numFrames. Defaults to the current frame counter 
	 */
	static void stop(int thread, int event, int frame=frameCnt) {
			
#ifndef DISABLE_EVENT_COUNTER
		// if you get a segfault from calling this function, double check to make sure
		// that HRtimer::initialize() was called
		timeval t;
		gettimeofday(&t, NULL);
		__sync_synchronize();
#ifdef HW_EVENTS
		int ret = PAPI_read(eventSet[thread], data[thread][event][frame].back().eventStop);
		assert(ret == PAPI_OK && "Error reading values");
#endif //HW_EVENTS
		data[thread][event][frame].back().timeStop = t;
#endif //DISABLE_EVENT_COUNTER
	}
	
	/*! \brief Registers a thread for PAPI counting
	 *
	 * \param tid an integer between 0 and numThreads
	 * This is not needed if only using for timing. 
	 */
	static void registerThread(int tid) {
#ifndef DISABLE_EVENT_COUNTER
#ifdef HW_EVENTS
	// tid must be between 0 and MAX_THREAD_NUM
		eventSet[tid] = PAPI_NULL;
		int ret;
		
		// init event set
		ret = PAPI_create_eventset(&(eventSet[tid]));
		assert(ret == PAPI_OK && "Error initializing event set");
		
		for (int i = 0; i < hwEvents.size(); i++) {
			ret = PAPI_add_event(eventSet[tid], hwEvents[i]);
			char EventCodeStr[PAPI_MAX_STR_LEN];
			PAPI_event_code_to_name(hwEvents[i], (char*)&EventCodeStr);
			assert(ret == PAPI_OK && "Error adding " && EventCodeStr);
		}
		
		// start the counting
		ret = PAPI_start(eventSet[tid]);
		assert(ret == PAPI_OK && "Error starting hw counting");
#endif //HW_EVENTS
#endif //DISABLE_EVENT_COUNTER
	}
	

	/*! \brief Prints output to given io stream
	 *
	 * \param eventName a pointer to std::map; to map event id's to strings
	 * \param sep an char specifying the ascii char to use as the deliminator; default is set to TAB
	 * \param out a reference to an iostream; default is std::cout to print to screen
	 */
	static void printTimes(std::map<int,std::string>* eventNames = NULL,char sep = TAB_CHAR, std::ostream& out=std::cout) {
#ifndef DISABLE_EVENT_COUNTER
		out << "Thread" << sep << "Frame" << sep << "Event" << sep <<  "uStart" << sep <<  "uStop";
#ifdef HW_EVENTS
		for (int i = 0; i < hwEvents.size(); i++) {
			char EventCodeStr[PAPI_MAX_STR_LEN];
			PAPI_event_code_to_name(hwEvents[i], (char*)&EventCodeStr);
			out << sep << EventCodeStr;
		}
#endif //HW_EVENTS
		out << std::endl;
		for (int thread = 0; thread < data.size(); thread++) {
			for (int event = 0; event < data[thread].size(); event++) {	
				for (int frame = 0; frame < data[thread][event].size(); frame++) {
					for (int i = 0; i < data[thread][event][frame].size(); i++) {
						std::cout << thread << sep << frame << sep;
						if (eventNames == NULL) {
							out << event;
						} else { 
							out << (*eventNames)[event];
						}
						suseconds_t uStart, uStop;
						TimeEvent &e = data[thread][event][frame][i];
                        uStart = (suseconds_t)((e.timeStart.tv_sec - timeInit.tv_sec)*MILLION) + e.timeStart.tv_usec - timeInit.tv_usec;
                        uStop = (suseconds_t)((e.timeStop.tv_sec - timeInit.tv_sec)*MILLION) + e.timeStop.tv_usec - timeInit.tv_usec;
                        out << sep << uStart << sep << uStop;
#ifdef HW_EVENTS
                        for (int i = 0; i < hwEvents.size(); i++) {
							out << sep << e.eventStop[i] - e.eventStart[i];
						}
#endif //HW_EVENTS
                        out << std::endl;
					}
				}		
			}
		}
#endif //DISABLE_EVENT_COUNTER
	}
	
	/*! \brief Advance the frame
	 * 
	 * Increased the frame count by one.
	 *
	 * Todo: between frames, check the max event size. If it, the vector needed to 
	 * be resized, then resize future event to prevent alloction during mid event. 
	 */
	static void advanceFrame() {
#ifndef DISABLE_EVENT_COUNTER
		//data[threadNum][eventNum][frameNum][eventInstance]
		for (int e = 0; e < data[0].size(); e++) {
			int frameMaxInst = 0;
			for (int t = 0; t < data.size(); t++) {
				int s = data[t][e][frameCnt].size();
				if (frameMaxInst < s) {
					frameMaxInst = s;
				}
			}
			if (frameMaxInst > maxEventInstances[e]) {
                int maxFrames = data[0][0].size();
                
				
				int newMax = frameMaxInst * 2;
			    maxEventInstances[e] = newMax;
				for (int t = 0; t < data.size(); t++) {
					for (int f = frameCnt+1; f < maxFrames; f++) {
						data[t][e][f].reserve(newMax);
					}
				}
			}
		}
		frameCnt++;
#endif //DISABLE_EVENT_COUNTER
	}
};




#endif //THREADED_EVENT_COUNTER
