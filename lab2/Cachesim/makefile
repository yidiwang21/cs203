cachesim: main.o cache_arch.o
	g++ main.o cache_arch.o -o cachesim

cache_arch.o: cache_arch.cpp
	g++ -c -std=c++11 cache_arch.cpp

main.o: main.cpp
	g++ -c -std=c++11 main.cpp

clean:
	rm *.o cachesim