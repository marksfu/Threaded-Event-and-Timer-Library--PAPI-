TARGETS:= openmp-timerOnly openmp-hwCounters openmp-disableEvents pthread-hwCounters singleThread

all: $(TARGETS)

# Nothing special here
openmp-timerOnly: openmp_test.cpp 
	g++ -fopenmp -o $@ openmp_test.cpp

# Add the compile flag DISABLE_EVENT_COUNTER to remove event 
# counting, all calls should then be optimized out
openmp-disableEvents: openmp_test.cpp 
	g++ -fopenmp -DDISABLE_EVENT_COUNTER -o $@ openmp_test.cpp

# To compile with papi counters, add HW_EVENTS flag. Also
# add -lpapi to the library. Must have papi compatable kernel
openmp-hwCounters: openmp_test.cpp 
	g++ -fopenmp -lpapi -DHW_EVENTS -o $@ openmp_test.cpp


pthread-hwCounters: pthread_test.cpp .
	g++ -lpthread -lpapi -DHW_EVENTS -o $@ pthread_test.cpp

singleThread: singleThreaded.cpp 
	g++ -lpapi -DHW_EVENTS -o $@ singleThreaded.cpp

clean: 
	rm -f $(TARGETS)
