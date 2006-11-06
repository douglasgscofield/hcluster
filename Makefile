CC=			g++
CFLAGS=		-Wall -g -pg -DLIH_DEBUG #-DGRAPH64
OBJS=		cluster_graph.o read_file.o basic_graph.o

%.o:%.cc
		$(CC) -c $(CFLAGS) $< -o $@
%.o:%.c
		$(CC) -c $(CFLAGS) $< -o $@

all:gcluster
		echo;

gcluster:$(OBJS) cluster.o
		$(CC) $(CFLAGS) $^ $(LIBS) -o $@

clean:
		rm -f *.o gcluster gmon.* bb.out *.yy.c
		rm -f *.aux *.log *.dvi *.ps *.pdf
