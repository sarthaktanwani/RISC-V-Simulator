# CPPFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb
CPPFLAGS=
TARGET=cpusim
DEFINE=

all: *.cpp
	g++ $(CFLAGS) -o $(TARGET) $^

test-r:
	./$(TARGET) ../Trace/22instMem-r.txt
test-sw:
	./$(TARGET) ../Trace/22instMem-sw.txt
test-all:
	./$(TARGET) ../Trace/22instMem-all.txt
test-all-new:
	./$(TARGET) ../Trace/22instMem-all.txt

cpusim-r: *.cpp
	g++ $(CFLAGS) -o $(TARGET) $^ && ./$(TARGET) ../Trace/22instMem-r.txt

cpusim-sw: *.cpp
	g++ $(CFLAGS) -o $(TARGET) $^ && ./$(TARGET) ../Trace/22instMem-sw.txt

cpusim-all: *.cpp
	g++ $(CFLAGS) -o $(TARGET) $^ && ./$(TARGET) ../Trace/22instMem-all.txt
cpusim-all-new: *.cpp
	g++ $(CFLAGS) -o $(TARGET) $^ && ./$(TARGET) ../Trace/22instMem-all_new.txt


clean:
	rm $(TARGET)