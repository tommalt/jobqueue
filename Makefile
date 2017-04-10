CC=cc
CXX=c++
CFLAGS=-g
CSTD=-std=c89
CXXSTD=-std=c++14
EXECUTABLE=main.exe
OBJS=job.o main.o 
TARGET=main.exe

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(EXECUTABLE) -lpthread

job.o: job.c
	$(CC) -c job.c $(CFLAGS) $(CSTD)

main.o: main.c
	$(CC) -c main.c $(CFLAGS) $(CSTD)

clean:
	rm *.o *.exe

