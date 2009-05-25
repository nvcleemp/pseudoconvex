CFLAGS = -Wall -g

TARG = cone$(SUFFIX)
OBJS = cone.o pseudoconvex.o

all: $(TARG)

$(TARG): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARG) $(OBJS)

clean:
	rm -f *.o $(TARG)
