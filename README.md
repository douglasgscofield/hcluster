## hcluster_sg

**NOT MY SOFTWARE**

My clone of the sourceforge subdirectory for **hcluster_sg**, Heng Li's tool for hierarchical clustering of proteins.  Initial commit is a direct conversion of <http://sourceforge.net/p/treesoft/code/HEAD/tree/trunk/hcluster/>:

    git svn clone --trunk=trunk/hcluster http://svn.code.sf.net/p/treesoft/code hcluster

plus this `README.md`.

Tried again, to get the newest branch that includes commits to fix compilation errors due to missing `malloc.h`.

    git svn clone  --trunk=hcluster http://svn.code.sf.net/p/treesoft/code/branches/lh3 hcluster
    
Still got some compilation errors/warnings.

~~~~
fb166: ~/github/local/tmp/hcluster $ make CXX=clang
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
./stdhash.H:492:13: error: use of undeclared identifier 'direct_insert_aux'
                int ret = direct_insert_aux(key, this->n_capacity, this->keys, this->flags, &i);
                          ^
                          this->
cluster_graph.cc:83:14: note: in instantiation of member function 'hash_map_misc<unsigned int, unsigned int>::insert' requested here
                conv_list2.insert(v1, tmp1);
                           ^
./stdhash.H:291:13: note: must qualify identifier to find this declaration in dependent base class
        inline int direct_insert_aux(const keytype_t &key, hashint_t m, keytype_t *K, __lh3_flag_t *F, hashint_t *i) {
                   ^
2 errors generated.
make: *** [cluster_graph.o] Error 1
~~~~

After fixing the above,

~~~~
cluster.cc:16:62: warning: format specifies type 'int' but the argument has type 'size_t' (aka 'unsigned long') [-Wformat]
        fprintf(stderr, "          -m NUM     maximum size [%d]\n", gc_max_cluster_size);
                                                            ~~      ^~~~~~~~~~~~~~~~~~~
                                                            %lu
~~~~

Fixed that too, now compiles cleanly.
