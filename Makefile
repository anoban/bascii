CC = /usr/bin/gcc

CPP = /usr/bin/g++

DEBUG = -DDEBUG -D_DEBUG -O0 -g3

NODEBUG = -D_NDEBUG -DNDEBUG -O3 -g0

CCFLAGS = -Wall -Wextra -std=c17 -march=tigerlake -mavx512f -ffast-math -mprefer-vector-width=512

CPPFLAGS = -Wall -Wextra -std=c++20 -march=tigerlake -mavx512f -ffast-math -mprefer-vector-width=512

INCLUDE_DIRS = -I./include/

TEST_INCLUDE_DIRS = $(INCLUDE_DIRS) I./tests/googletest/ -I./tests/googletest/include/

build:
	$(CC) $(INCLUDE_DIRS) ./main.c $(CFLAGS) $(NODEBUG) -o bmpasc.out

test:
	$(CPP) $(INCLUDE_DIRS) ./tests/main.cpp $(CFLAGS) -D__TEST__ $(NODEBUG) -o test.out

clean:
	rm -f ./*.out
	rm -f ./*.o
