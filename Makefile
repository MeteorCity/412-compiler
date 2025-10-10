CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -g
OBJS = main.o scanner.o parser.o ir.o renamer.o allocator.o
TARGET = 412alloc

build: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)