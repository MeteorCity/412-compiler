CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -g
OBJS = main.o scanner.o parser.o ir.o
TARGET = 412fe

build: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)