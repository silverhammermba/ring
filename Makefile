CXXFLAGS=-std=c++11

hide: main.o
	$(CXX) $(CXXFLAGS) -o $@ $+ -lGL -lSDL2
