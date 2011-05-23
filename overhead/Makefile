all: timingOnly-overhead timingWithPapi-overhead 

timingOnly-overhead: overhead.cpp 
	g++ -fopenmp -O3 -funroll-loops -fprefetch-loop-arrays -static-libgcc -o $@ overhead.cpp

timingWithPapi-overhead: overhead.cpp
	g++ -fopenmp -O3 -funroll-loops -fprefetch-loop-arrays -static-libgcc -lpapi -DHW_EVENTS -o $@ overhead.cpp

