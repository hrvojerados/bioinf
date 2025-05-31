
CXX = g++
CXXFLAGS = -O3 -IthirdParty/include -std=c++17 -Wall -Wextra

TEST_DIR = tests
COMMON_SRC = common/BOBHash.cpp

TEST_SRCS = $(wildcard $(TEST_DIR)/test*.cpp)
TEST_BINS = $(patsubst %.cpp, %, $(TEST_SRCS))

all: $(TEST_BINS)

%: %.cpp $(COMMON_SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -f $(TEST_BINS)

.PHONY: all clean
