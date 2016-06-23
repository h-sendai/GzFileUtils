CC = c++
PROG = sample
CXXFLAGS = -g -O2 -Wall
LDLIBS += -lz -lboost_filesystem -lboost_date_time -lboost_system

OBJS += sample.o
OBJS += GzFileUtils.o

all: $(PROG)

$(PROG): $(OBJS)

clean:
	rm -f *.o $(PROG)
