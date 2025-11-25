CXX = g++ 
CXXFLAGS = -O2 -std=c++17 -Wall -Wextra -Werror -g
OBJS = main.o scanner.o parser.o ir.o renamer.o graph.o scheduler.o output.o
TARGET = schedule

build: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)