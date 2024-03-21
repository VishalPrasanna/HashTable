
ifeq ($(DEBUG),1)
CFLAGS = -fPIC  -std=c99 -ggdb -msse4.2 -mpclmul -march=native -funroll-loops -Wstrict-overflow -Wstrict-aliasing -Wall -Wextra -pedantic -Wshadow -fsanitize=undefined  -fno-omit-frame-pointer -fsanitize=address
else
CFLAGS = -fPIC -std=c99 -O3 -msse4.2 -mpclmul -march=native -funroll-loops -Wstrict-overflow -Wstrict-aliasing -Wall -Wextra -pedantic -Wshadow
endif 


OBJ = main.o HashTable.o bloom_filter.o

all: start clean

start: run
	./run

run: main.o HashTable.o bloom_filter.o
	gcc  -o run main.o HashTable.o bloom_filter.o -lclhash -lm

runc: main.o HashTable.o bloom_filter.o
	gcc -o runc main.o HashTable.o bloom_filter.o -Wl,-rpath, -Llib/temp -l_clhash -lm
 
main.o: main.c
	gcc -g -c main.c -o main.o 

HashTable.o: HashTable.c 
	gcc -g -c HashTable.c -fpic -o HashTable.o

lib_clhash.so: clhash.o
	gcc $(CFLAGS) clhash.o -shared -o libclhash.so 

clhash.o: clhash.c
	gcc $(CFLAGS) -c clhash.c -o clhash.o

test: ./nousefile/test.c
	gcc -o test ./nousefile/test.c bloom_filter.c -I. -lclhash -lm

leak_check: runc
	valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20  ./runc

bloom_filter.o: bloom_filter.c
	gcc bloom_filter.c -c -o bloom_filter.o


benchmark: benchmark_generate benchmark_run clean

benchmark_generate: ./benchmark/googlebenchmark.cc
	g++ ./benchmark/googlebenchmark.cc HashTable.c bloom_filter.c  -std=c++11 -isystem benchmark/include \
	-Lbenchmark/build/src -lbenchmark -lpthread -lclhash -lm -I. -o gbenchmark

benchmark_run:
	./gbenchmark --benchmark_counters_tabular=true

clean:
	rm -f $(OBJ) 