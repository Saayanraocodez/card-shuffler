CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -I../shuffler
SRC = ../shuffler/sha256.cpp ../shuffler/chacha20.cpp ../shuffler/rng.cpp ../shuffler/shuffle_core.cpp

test_core: test_core.cpp $(SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@

run: test_core
	./test_core

clean:
	rm -f test_core
