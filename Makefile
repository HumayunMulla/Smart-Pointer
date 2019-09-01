CC=g++
CFLAGS=-std=c++14 -Wall -Wextra -pedantic -O2

test_orig: Test_orig.cpp SharedPtr.hpp
	$(CC) $(CFLAGS) -pthread Test_orig.cpp -o test_orig

test_1: Test_1.cpp SharedPtr.hpp
	$(CC) $(CFLAGS) Test_1.cpp -o test_1

clean:
	rm -rf test_*