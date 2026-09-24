SRCS = $(wildcard src/*.cpp)
SRC_OBJS = $(patsubst src%.cpp,obj%.o,$(SRCS))

TESTS = $(wildcard test/*.cpp)
TEST_OBJS = $(patsubst test%.cpp,obj%.o,$(TESTS))

CXX = g++

.PHONY: test clean

obj/caslasm.o: src/caslasm.cpp src/startup.cas
	$(CXX) -Wall -std=c++11 -Iinclude -c -o $@ $<

obj/%.o: src/%.cpp
	$(CXX) -Wall -std=c++11 -Iinclude -c -o $@ $<

obj/%.o: test/%.cpp
	$(CXX) -Wall -std=c++11 -Iinclude -c -o $@ $<

caslasm: $(SRC_OBJS)
	$(CXX) -Wall -g -std=c++11 -o caslasm $^

test: $(filter-out obj/caslasm.o,$(SRC_OBJS)) $(TEST_OBJS)
	$(CXX) -std=c++11 -o test.o -Llib -lgmock -lgmock_main -lpthread $^
	./test.o
	rm test.o

clean:
	rm -f obj/*.o caslasm caslasm.exe
