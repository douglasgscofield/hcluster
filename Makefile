CC=			g++
CFLAGS=		-O2 -Wall -g #-pg
DFLAGS=		#-DGRAPH64 #-DLIH_DEBUG
OBJS=		cluster_graph.o read_file.o basic_graph.o

%.o:%.cc
		$(CC) -c $(CFLAGS) $(DFLAGS) $< -o $@
%.o:%.c
		$(CC) -c $(CFLAGS) $(DFLAGS) $< -o $@

all:hcluster_sg
hcluster_sg:$(OBJS) cluster.o
		$(CC) $(CFLAGS) $(DFLAGS) $^ $(LIBS) -o $@
cluster.o:cluster_graph.h

clean:
		rm -f *.o gcluster gmon.* bb.out *.yy.c *.aux *.log *.dvi *.ps *.pdf hcluster_sg
