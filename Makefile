CC=			gcc
CXX=		g++
CFLAGS=		-g -Wall -O2 #-pg
CXXFLAGS=	$(CFLAGS)
DFLAGS=		-DLH3_SAVE_MEMORY -DGRAPH64 #-DLIH_DEBUG
OBJS=		cluster_graph.o read_file.o basic_graph.o
VERSION=	0.5.0
GCVERSION=	-DGC_VERSION="\"$(VERSION)\"" -DGC_BUILD=\"`date +%d%b%Y`\"

.SUFFIXES:.c .o .cc

.c.o:
		$(CC) -c $(DFLAGS) $(CFLAGS)  $< -o $@
.cc.o:
		$(CXX) -c $(CFLAGS) $(DFLAGS) $< -o $@

all:hcluster_sg
hcluster_sg:$(OBJS) cluster.o
		$(CXX) $(CFLAGS) $(DFLAGS) $^ $(LIBS) -o $@
cluster_graph.o:cluster_graph.h
cluster.o:cluster.cc
		$(CXX) -c $(CXXFLAGS) $(DFLAGS) $(GCVERSION) cluster.cc -o $@

package:clean
		@(cd ..; mv hcluster hcluster-$(VERSION); \
			(find hcluster-$(VERSION) -type f | grep -v "\.svn" | xargs tar cf -) | gzip > hcluster-$(VERSION).tar.gz; \
			mv hcluster-$(VERSION) hcluster)

clean:
		rm -f *.o gcluster gmon.* bb.out *.yy.c *.aux *.log *.dvi *.ps *.pdf hcluster_sg *~
