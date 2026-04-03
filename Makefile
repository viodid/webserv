BINARY 		= webserv
CXX 		= g++
CXXFLAGS	= -std=c++98 -Wall -Wextra -Werror -Wpedantic -g3 -O0 -DDEBUG=1

SRCS 		= $(shell find . -type f -name '*.cpp' ! -name 'main.cpp' ! -name 'test_Socket.cpp' ! -name 'test_Config.cpp')
#SRCS		= $(shell find src -type f -name '*.cpp')
OBJS 		= $(SRCS:.cpp=.o)

# ----- Test targets -----
# Requires: libgtest and libgtest_main (sudo apt install libgtest-dev)
TEST_BINARY		= test_parser
TEST_SRCS		= $(filter-out src/main.cpp, $(SRCS)) \
			  test/test_ConfigParser.cpp
TEST_OBJS		= $(TEST_SRCS:.cpp=.o)
TEST_CXXFLAGS	= -std=c++17 -Wall -Wextra -Werror -g3 -O0 -DDEBUG=1
TEST_LDFLAGS	= -lgtest -lgtest_main -lpthread


all: $(BINARY)

$(BINARY): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build and run HttpRequestParser unit tests
test_parser: $(TEST_OBJS)
	$(CXX) $(TEST_CXXFLAGS) -o $(TEST_BINARY) $^ $(TEST_LDFLAGS)

test/%.o: test/%.cpp
	$(CXX) $(TEST_CXXFLAGS) -c $< -o $@

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run_tests: test_parser
	./$(TEST_BINARY)

test: run_tests

clean:
	rm -f $(OBJS) $(TEST_OBJS)

fclean: clean
	rm -f $(BINARY) $(TEST_BINARY)

re: clean all

.PHONY: all test_parser run_tests clean fclean re

