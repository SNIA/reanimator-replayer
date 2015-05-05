OPTFLAGS = -g -O2 -Wall -Werror

CXXFLAGS = $(OPTFLAGS)
CXX      = g++

TARGETS = syscall-simple-read
SYSCALL_SIMPLE_READ_OBJECTS = syscall-simple-read.o

all: $(TARGETS)

syscall-simple-read: $(SYSCALL_SIMPLE_READ_OBJECTS)
	$(CXX) -o $@ $(SYSCALL_SIMPLE_READ_OBJECTS)

%.o: %.cpp
	$(CXX) -c -o $@ $?
clean:
	rm -f $(TARGETS) $(SYSCALL_SIMPLE_READ_OBJECTS) *~
