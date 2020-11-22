CXX = g++
CXFLAGS = -std=c++17 -O2 -mavx2
deps = BOBHash32.h Bitmap.h Bloomfilter.h CMSketch.h
obj = test.o BOBHash32.o Bitmap.o Bloomfilter.o CMSketch.o
program = test.out

$(program): $(obj)
	$(CXX) -o $(program) $(obj) $(CXFLAGS)

BOBHash32.o: BOBHash32.cpp BOBHash32.h
	$(CXX) -c BOBHash32.cpp $(CXFLAGS)

Bitmap.o: Bitmap.cpp Bitmap.h BOBHash32.h
	$(CXX) -c Bitmap.cpp $(CXFLAGS)

Bloomfilter.o: Bloomfilter.cpp Bloomfilter.h BOBHash32.h
	$(CXX) -c Bloomfilter.cpp $(CXFLAGS)

CMSketch.o: CMSketch.cpp CMSketch.h BOBHash32.h
	$(CXX) -c CMSketch.cpp $(CXFLAGS)

test.o: test.cpp $(deps)
	$(CXX) -c test.cpp $(CXFLAGS)

clean:
	rm -rf $(obj) $(program)
