BINARY 		= webserv
CXX 		= g++
STD			= 98
CXXFLAGS	= -std=c++$(STD) -Wall -Wextra -Wpedantic -g3 -O0 -DDEBUG=1
LDFLAGS		= -lgtest -lgtest_main -lpthread

SRCS		= $(shell find . -type f -name '*.cpp')
OBJS 		= $(SRCS:.cpp=.o)


all: $(BINARY)

$(BINARY): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: STD=17
test: re
	./$(BINARY)

clean:
	rm -f $(OBJS)

fclean: clean
	rm $(BINARY)

re: clean all

.PHONY: all test clean rebuild

