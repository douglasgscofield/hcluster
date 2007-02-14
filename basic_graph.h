#ifndef BASIC_GRAPH_H_
#define BASIC_GRAPH_H_

#include <stdlib.h>
#include <stdio.h>
#include "stdhash.H"
#include "basic.h"
#include "cluster_graph.h"

#ifdef GRAPH64
typedef bit32_t bvertex_t;
#ifndef LH3_SAVE_MEMORY
typedef bit64_t bedge_t;
#endif
#else

typedef bit16_t bvertex_t;
#ifndef LH3_SAVE_MEMORY
typedef bit32_t bedge_t;
#endif
#endif

extern int gc_min_weight;

class BasicGraph
{
	typedef BasicVertex<bvertex_t> BVertex;
protected:
	ClusterGraph cgraph;
	bvertex_t max_vertices, total_vertices;
	BVertex *basic_graph;
#ifndef LH3_SAVE_MEMORY
	hash_map_misc<weight_t, bedge_t> edge_set; // C++ will call the destructor automatically.
#endif
	bvertex_t flag_one(bvertex_t, bvertex_t, bvertex_t*, bvertex_t* = 0);
public:
	BasicGraph(void) { total_vertices = max_vertices = 0; basic_graph = 0; }
	~BasicGraph(void);
	bool add(bvertex_t v1, bvertex_t v2, weight_t w);
	void assign_category(bvertex_t v, unsigned char cat)
	{
		if (v < max_vertices) basic_graph[v].cat = cat;
	}
	bvertex_t main(FILE *fp = stdout);
};

void free_all();
size_t read_graph(FILE *fp, BasicGraph &);
void gc_read_category(FILE *fp, BasicGraph *bg);

#endif // BASIC_GRAPH_H_
