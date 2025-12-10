BINARY 		= webserv
CXX 		= g++
CXXFLAGS	= -std=c++17 -Wall -Wextra -Wpedantic -g3 -O0 -DDEBUG=1
LDFLAGS		= -lgtest -lgtest_main -lpthread

SRCS		= $(shell find . -type f -name '*.cpp')
OBJS 		= $(SRCS:.cpp=.o)


all: $(BINARY)

$(BINARY): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(BINARY)
	./$(BINARY)

clean:
	rm -f $(OBJS)

fclean: clean
	rm $(BINARY)

re: clean all

.PHONY: all test clean rebuild

