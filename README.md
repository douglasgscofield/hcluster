## hcluster_sg

**NOT MY SOFTWARE**

My clone of the sourceforge subdirectory for **hcluster_sg**, Heng Li's tool for hierarchical clustering of proteins.

~~~~
$ ./hcluster_sg

Program : hcluster_sg (Hierarchically clustering on a sparse graph)
Version : 0.5.1, build 28Apr2014
Contact : Heng Li <lh3lh3@gmail.com>

Usage   : hcluster_sg [options] [input_file]

Options : -w NUM     minimum edge weight [20]
          -s FNUM    minimum edge density between a join [0.50]
          -m NUM     maximum size [500]
          -o STRING  output file [stdout]
          -c         only find single-linkage clusters (bypass h-cluster)
          -v         verbose mode
          -h         help

Advanced Options:

          -b FNUM    breaking edge density [0.10]
          -O         the once-fail-inactive-forever mode
          -r         weight resolution for '-O' [5]
          -C FILE    category file
          -L NUM     stringent level ('3' is the strictest) [2]

~~~~

### Converting from Sourceforge SVN

My initial attempt was a direct conversion of <http://sourceforge.net/p/treesoft/code/HEAD/tree/trunk/hcluster/>:

    git svn clone --trunk=trunk/hcluster http://svn.code.sf.net/p/treesoft/code hcluster

but then noticed the new commits (e.g., fix for missing `malloc.h`) on the `lh3` branch so starting with that instead:

    git svn clone  --trunk=hcluster http://svn.code.sf.net/p/treesoft/code/branches/lh3 hcluster

### Initial fixups

Fixed some compilation errors and warnings:

~~~~
$ make CXX=clang
clang -c -g -Wall -O2  -DLH3_SAVE_MEMORY -DGRAPH64  cluster_graph.cc -o cluster_graph.o
In file included from cluster_graph.cc:3:
In file included from ./cluster_graph.h:4:
./stdhash.H:411:13: error: use of undeclared identifier 'direct_insert_aux'
                int ret = direct_insert_aux(key, this->n_capacity, this->keys, this->flags, &i);
                          ^
                          this->
./cluster_graph.h:48:10: note: in instantiation of member function 'hash_set_misc<unsigned int>::insert' requested here
                v_set->insert(i);
                       ^
./stdhash.H:291:13: note: must qualify identifier to find this declaration in dependent base class
        inline int direct_insert_aux(const keytype_t &key, hashint_t m, keytype_t *K, __lh3_flag_t *F, hashint_t *i) {
                   ^
~~~~

and a couple more examples of missing `this->`.  After fixing the above,

~~~~
$ make CXX=clang
cluster.cc:16:62: warning: format specifies type 'int' but the argument has type 'size_t' (aka 'unsigned long') [-Wformat]
        fprintf(stderr, "          -m NUM     maximum size [%d]\n", gc_max_cluster_size);
                                                            ~~      ^~~~~~~~~~~~~~~~~~~
                                                            %lu
~~~~

Fixed that too, now compiles with just a single warning of an unused variable.

~~~~
$ make
g++ -c -g -Wall -O2  -DLH3_SAVE_MEMORY -DGRAPH64  cluster_graph.cc -o cluster_graph.o
cluster_graph.cc: In member function 'void ClusterGraph::merge(cvertex_t, cvertex_t)':
cluster_graph.cc:218:12: warning: variable 'tmp_optidx' set but not used [-Wunused-but-set-variable]
g++ -c -g -Wall -O2  -DLH3_SAVE_MEMORY -DGRAPH64  read_file.cc -o read_file.o
g++ -c -g -Wall -O2  -DLH3_SAVE_MEMORY -DGRAPH64  basic_graph.cc -o basic_graph.o
g++ -c -g -Wall -O2  -DLH3_SAVE_MEMORY -DGRAPH64  -DGC_VERSION="\"0.5.1\"" -DGC_BUILD=\"`date +%d%b%Y`\" cluster.cc -o cluster.o
g++ -g -Wall -O2  -DLH3_SAVE_MEMORY -DGRAPH64  cluster_graph.o read_file.o basic_graph.o cluster.o  -o hcluster_sg
~~~~
