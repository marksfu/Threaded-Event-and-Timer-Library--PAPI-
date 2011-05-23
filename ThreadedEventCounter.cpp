#include "ThreadedEventCounter.h"

// allocate the static members
extern int EventTimer::frameCnt;
#ifndef DISABLE_EVENT_COUNTER
extern std::vector<std::vector<std::vector<std::vector <TimeEvent> > > > EventTimer::data;

extern timeval EventTimer::timeInit;
extern char EventTimer::TAB;
extern std::vector<int> EventTimer::maxEventInstances;


#ifdef HW_EVENTS
extern std::vector<int> EventTimer::eventSet;
extern std::vector<int> EventTimer::hwEvents;
#endif //HW_EVENTS
#endif
