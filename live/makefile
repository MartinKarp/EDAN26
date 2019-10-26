CC		= gcc

CFLAGS		= -g -Wall -pedantic -O3 -fopenmp
#CFLAGS		= -g -Wall -pedantic -fsanitize=thread

OBJS		= main.o list.o error.o random.o set.o dataflow.o

OUT		= live

LDFLAGS		= -lpthread

S=100000
V=100000
U=4
A=1000
T=4
P=0

#S=10
#V=30
#U=4
#A=10
#T=4
#P=1
#valgrind --tool=callgrind ./live_org 100000 100000 4 1000 4 0

all: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $(OUT)
	#valgrind --tool=helgrind ./$(OUT) $(S) $(V) $(U) $(A) $(T) $(P)
	./live_org $(S) $(V) $(U) $(A) $(T) $(P)
	./$(OUT) $(S) $(V) $(U) $(A) $(T) $(P)

clean:
	rm -f $(OUT) $(OBJS) cfg.dot
