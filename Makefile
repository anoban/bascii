CC = /usr/bin/gcc

CCFLAGS = -Wall -Wextra -std=c17 -march=tigerlake -mavx512f -ffast-math -mprefer-vector-width=512 -lm

CPP = /usr/bin/g++

CPPFLAGS = -Wall -Wextra -std=c++20 -march=tigerlake -mavx512f -ffast-math -mprefer-vector-width=512

DEBUG = -DDEBUG -D_DEBUG -O3 -g3

NODEBUG = -D_NDEBUG -DNDEBUG -O3 -g0

INCLUDE_DIRS = -I./include/

TEST_INCLUDE_DIRS = $(INCLUDE_DIRS) -I./tests/googletest/ -I./tests/googletest/include/

GTEST_ALL = ./tests/googletest/src/gtest-all.cc

build:
	$(CC) $(INCLUDE_DIRS) ./main.c $(CCFLAGS) $(NODEBUG) -o bascii.out

test:
	$(CPP) $(TEST_INCLUDE_DIRS) ./tests/*.cpp $(GTEST_ALL) $(CPPFLAGS) -D__TEST__ $(DEBUG) -o test.out -Wno-missing-field-initializers -Wno-address

clean:
	rm -f ./*.out
	rm -f ./*.o
