CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude

SRCS = src/main.cpp src/commands.cpp src/dispatcher.cpp src/utils.cpp 
OBJS = $(SRCS:.cpp=.o)

CustomShell: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) -lreadline

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o CustomShell