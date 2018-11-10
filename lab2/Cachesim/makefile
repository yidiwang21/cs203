cachesim: main.o cache.o
	g++ main.o cache.o -o cachesim

cache_arch.o: cache.cpp
	g++ -c -std=c++11 cache.cpp

main.o: main.cpp
	g++ -c -std=c++11 main.cpp

clean:
	rm *.o cachesim