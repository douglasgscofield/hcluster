# hcluster_sg

**NOT MY SOFTWARE**

My clone of the sourceforge subdirectory for **hcluster_sg**, Heng Li's tool for hierarchical clustering of proteins.

```
$ ./hcluster_sg

Program : hcluster_sg (Hierarchically clustering on a sparse graph)
Version : 0.5.1, build 19Aug2014
Contact : Douglas Scofield <douglasgscofield@gmail.com> (repository maintainer)
          Written by and Copyright (c) 2006 Heng Li <lh3lh3@gmail.com>

Usage   : hcluster_sg [options] [input_file]

Options : -w NUM     minimum edge weight [20]
          -s FNUM    minimum edge density between a join (only for hierarchical clustering) [0.50]
          -m NUM     maximum size (only for hierarchical clustering) [500]
          -o STRING  output file [stdout]
          -c         only find single-linkage clusters (bypass h-cluster)
          -v         verbose mode
          -h         help

Advanced Options (only for hierarchical clustering):

          -b FNUM    breaking edge density [0.10]
          -O         the once-fail-inactive-forever mode
          -r         weight resolution for '-O' [5]
          -C FILE    category file
          -L NUM     stringent level ('3' is the strictest) [2]
```

<hr>

# Using `hcluster_sg`

The following has been gleaned from the source and the SVN commit history.

## Hierarchical and single-linkage clustering

Default clustering is hierarchical.

```
$ ./hcluster_sg exam-1.txt
0	0	57	1.000	0	13	11,0,4,3,5,12,7,10,13,2,1,6,9,
1	0	23	0.671	0	46	45,39,71,83,49,61,64,38,25,72,44,29,90,35,30,28,62,65,63,88,89,34,82,73,52,74,17,55,41,56,59,51,32,33,87,31,85,27,48,66,50,37,60,77,26,54,
2	0	0	1.000	0	1	84,
3	0	33	1.000	0	8	101,96,92,98,94,100,91,99,
4	0	28	1.000	0	4	40,43,76,42,
5	0	0	1.000	0	1	8,
6	0	54	1.000	0	8	18,23,21,15,22,20,14,16,
7	0	15	0.596	0	17	24,95,58,69,78,36,68,70,57,67,97,79,53,47,75,86,46,
8	0	0	1.000	0	1	102,
```
With `-c`, only single-linkage clustering is performed.  With the example dataset (see below for file formats), the difference is dramatic.

```
$ ./hcluster_sg -c  exam-1.txt
0	99	0,13,39,84,83,96,101,100,99,98,76,43,8,42,40,14,24,86,97,95,102,79,78,75,70,69,68,67,58,57,53,47,46,36,23,22,21,20,18,16,15,94,92,91,90,89,88,87,85,73,41,38,33,27,26,25,17,82,77,74,72,71,66,65,64,63,62,61,60,59,56,55,54,52,51,50,49,48,45,44,34,32,31,37,35,30,29,28,12,11,10,9,7,6,5,4,3,2,1,
1	1	19,
2	1	80,
3	1	81,
4	1	93,
```


## Options

### `-O`, `-r`, the once-fail-inactive-forever mode

From message for commit 035eab27d1f5356aacd3d594149f754e4059a2e4: When a joining is considered invalid, `hcluster_sg` will test whether this join has better weight than the optimum of permitted edges (supplied by the `-r` option?). If so, the program will disallow this node to join others.

### `-C`, `-L`, using categories

From changes for commit 61d55907459beaaa8c05edf8a38b1ac384d93636, and reading the source for `cluster_graph.cc`.  In `gc_verify_edge()` in looking whether two vertices connected by an edge can be joined, if the two vertices have categories, then what happens depends on the category values and the value of `-L`.  Categories use a three-low-bits system.

`-L 1` (*testing only, do not use*)
: if neither vertex has all three low bits set (`p->cat != 0x7`) then the verticies are joinable, otherwise they are broken.

`-L 2` (*default*)
: if the categories are equal, and a single bit is set (value is 1, 2, or 4), then they are joinable, else they are broken; otherwise if there is no bitwise overlap between the categories or there is a low-bit overlap between the categories (`if ((p->cat & q->cat) == 0 || (p->cat & q->cat) == 0x1)`) then they are joinable; otherwise they are broken.

`-L 3` (*most stringent*)
: if the categories are equal and a single bit is set (1, 2, or 4), then they are joinable; otherwise if there is no bitwise overlap between the categories (`if ((p->cat & q->cat) == 0)`) then they are joinable; otherwise they are broken.

In `cluster_graph.cc`, merging two vertices results in a combining of their respective categories (line 172):

```c++
src1->cat |= src2->cat;
```

#### Effect of categories on output

The example categories file assigned everything (but ID 102) the category 1, except for 56 and 83 which are given category 2.  When clustering without and then with categories, 56 and 83 are placed into separate clusters after incorporating categories.  Note also column 5 which seems to be the bitwise-AND of the category values in the cluster.

```
$ ./hcluster_sg exam-1.txt
0	0	57	1.000	0	13	11,0,4,3,5,12,7,10,13,2,1,6,9,
1	0	23	0.671	0	46	45,39,71,83,49,61,64,38,25,72,44,29,90,35,30,28,62,65,63,88,89,34,82,73,52,74,17,55,41,56,59,51,32,33,87,31,85,27,48,66,50,37,60,77,26,54,
2	0	0	1.000	0	1	84,
3	0	33	1.000	0	8	101,96,92,98,94,100,91,99,
4	0	28	1.000	0	4	40,43,76,42,
5	0	0	1.000	0	1	8,
6	0	54	1.000	0	8	18,23,21,15,22,20,14,16,
7	0	15	0.596	0	17	24,95,58,69,78,36,68,70,57,67,97,79,53,47,75,86,46,
8	0	0	1.000	0	1	102,
$ ./hcluster_sg -C exam-1.cat exam-1.txt
0	0	57	1.000	1	13	11,0,4,3,5,12,7,10,13,2,1,6,9,
1	0	30	0.816	3	30	59,54,28,32,62,45,35,56,82,52,65,51,74,50,61,55,49,44,60,37,31,30,39,71,64,29,63,66,85,34,
2	0	36	0.937	3	20	83,88,77,73,48,91,26,101,90,27,17,89,41,72,94,38,25,33,92,87,
3	0	0	1.000	1	1	84,
4	0	35	1.000	1	4	100,98,96,99,
5	0	28	1.000	1	4	40,43,76,42,
6	0	0	1.000	1	1	8,
7	0	54	1.000	1	8	18,23,21,15,22,20,14,16,
8	0	15	0.596	1	17	24,95,58,69,78,36,68,70,57,67,97,79,53,47,75,86,46,
9	0	0	1.000	0	1	102,
```

Reading over Heng Li's thesis, these categories may establish an unrooted constraint tree on the clusters... perhaps those with the same bit set > 1 should be in separate clusters... I can't see my way through the semantics just now.

#### More categories notes

From <https://www.biostars.org/p/70008/>:

The original algorithm for Orthomcl didn't do any adhoc weighing of the species
in the sets, although this might have changed. There is an alternative to
orthomcl that does take into account ingroup and outgroup species, which is to
use hcluster_sg to do the clustering for you blast scores:

http://treesoft.svn.sourceforge.net/viewvc/treesoft/branches/lh3/hcluster/

Click on the Download GNU tarball at the bottom of the page.

The input file is an A.B.C format where protein A and B are followed by the
blast score or evalue (scaled from, say, 0-100) and another file, optionally,
which is the "categories" file. This software allows you to define these
"categories", see exam-1.cat as an example. In these categories, you can split
your sets into species that are very close together and species that can be
called outgroups, and outgroups can also have different levels. So ingroups for
close subgroups, then outgroups of different levels, will be taken into account
when doing the clustering, so that you are not leaving too many outgroup
proteins behind just because they are more distant in the phylogenetic tree
than the ingroup species.

The hcluster_sg software was (I think still is) the software used in the
EnsemblCompara GeneTrees pipeline: it scales really well and it's used for
trees that encompass the whole tree of life, including eukarya, prokarya and
archaea, and produces very decent protein clusters given the right categories.



## Output file formats

Output files contain several TAB-separated columns.  With default hierarchical clustering, the output format contains seven columns.

1. Cluster ID
2. Unknown
3. Unknown, perhaps mean weight within the cluster?
4. Unknown, perhaps cluster cutoff?
5. The final bitwise set of categories for the cluster
6. Cluster size
7. Cluster members

"Cluster members" is a comma-separated list of IDs in the cluster, and includes a terminating comma.  "Cluster size" is the number of IDs in "cluster members".

With single-linkage clustering (`-c`), the output format contains just three TAB-separated columns.

1. Cluster ID
2. Cluster size
3. Cluster members


## Input file formats

Apart from the above, the documentation is pretty thin, so I'll describe what I've been able to deduce so far.  There are two example files included with the source, a file of weights `exam-1.txt` and a file of categories `exam-1.cat`.

### Weight file

The file `exam-1.txt` contains pairwise weights/similarities/correlations, with a column format of

**id-1**   **id-2**   **integer-weight**

The C code that does the read is in `read_graph()` within `read_file.cc`, and the weight is eventually cast to `weight_t` which is defined in `basic.h` to be

```C++
typedef unsigned char weight_t;
```

so the IDs can be any character value while the weights must be an unsigned integer between 0 and 255, at least going by the datatypes. 

`exam-1.txt`:

```
0	1	95
0	2	92
1	2	92
0	3	97
1	3	95
2	3	92
...
0	8	0
1	8	0
2	8	0
3	8	0
4	8	1
5	8	0
6	8	0
7	8	0
...
```

### Category file

The file `exam-1.cat` contains category designations, with a column format of

**id**   **category**

The C code that does the read is in `gc_read_category()` within `read_file.cc`, and the category is eventually cast to `unsigned char` so, like the weight, the category must be an unsigned integer between 0 and 255, at least going by the datatypes. 

`exam-1.cat`:

```
0	1
1	1
2	1
3	1
4	1
...
54	1
55	1
56	2
57	1
58	1
...
```

But what does the category mean?

<hr>

# Repository history

## Converting from Sourceforge SVN

My initial attempt was a direct conversion of <http://sourceforge.net/p/treesoft/code/HEAD/tree/trunk/hcluster/>:

    git svn clone --trunk=trunk/hcluster http://svn.code.sf.net/p/treesoft/code hcluster

but then noticed the new commits (e.g., fix for missing `malloc.h`) on the `lh3` branch so starting with that instead:

    git svn clone  --trunk=hcluster http://svn.code.sf.net/p/treesoft/code/branches/lh3 hcluster

## Initial fixups

Fixed some compilation errors and warnings:

```
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
```

and a couple more examples of missing `this->`.  After fixing the above,

```
$ make CXX=clang
cluster.cc:16:62: warning: format specifies type 'int' but the argument has type 'size_t' (aka 'unsigned long') [-Wformat]
        fprintf(stderr, "          -m NUM     maximum size [%d]\n", gc_max_cluster_size);
                                                            ~~      ^~~~~~~~~~~~~~~~~~~
                                                            %lu
```

Fixed that too, now compiles with just a single warning of an unused variable.

```
$ make
g++ -c -g -Wall -O2  -DLH3_SAVE_MEMORY -DGRAPH64  cluster_graph.cc -o cluster_graph.o
cluster_graph.cc: In member function 'void ClusterGraph::merge(cvertex_t, cvertex_t)':
cluster_graph.cc:218:12: warning: variable 'tmp_optidx' set but not used [-Wunused-but-set-variable]
g++ -c -g -Wall -O2  -DLH3_SAVE_MEMORY -DGRAPH64  read_file.cc -o read_file.o
g++ -c -g -Wall -O2  -DLH3_SAVE_MEMORY -DGRAPH64  basic_graph.cc -o basic_graph.o
g++ -c -g -Wall -O2  -DLH3_SAVE_MEMORY -DGRAPH64  -DGC_VERSION="\"0.5.1\"" -DGC_BUILD=\"`date +%d%b%Y`\" cluster.cc -o cluster.o
g++ -g -Wall -O2  -DLH3_SAVE_MEMORY -DGRAPH64  cluster_graph.o read_file.o basic_graph.o cluster.o  -o hcluster_sg
```

## Further changes

Going through the code, it looks like `-DLH3_SAVE_MEMORY` may not be required.  What it does, from the message on commit d2c4a8f13f367d87bf76240ff114636875cd102a:

<blockquote>
Allow to compile in "SAVE_MEMORY" mode which will reduce the memory consumption by 30-40%. Speed should be accelerated at the same time because 64-bit hashinsertions and queries are totally removed. The bad side is the "BasicGraph" will no longer capable of checking whether an edge appears several times in the input file. Users should control this by themselves. "ClusterGraph" is less affected because it is using hash to store edges.
</blockquote>


