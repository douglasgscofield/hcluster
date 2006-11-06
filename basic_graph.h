#ifndef BASIC_GRAPH_H_
#define BASIC_GRAPH_H_

#include <malloc.h>
#include <stdio.h>
#include "hash_misc.h"
#include "basic.h"
#include "cluster_graph.h"

#ifdef GRAPH64                                                                                                                                 
typedef bit32_t bvertex_t;
typedef bit64_t bedge_t;
#else                                                                                                                                          
typedef bit16_t bvertex_t;
typedef bit32_t bedge_t;
#endif

class BasicGraph
{
	typedef BasicVertex<bvertex_t> BVertex;
protected:
	ClusterGraph cgraph;
	bvertex_t max_vertices, total_vertices;
	BVertex *basic_graph;
	hash_map_misc<weight_t, bedge_t> edge_set;
	weight_t threshold;
	bvertex_t flag_one(bvertex_t, bvertex_t, bvertex_t*, bvertex_t* = 0);
public:
	BasicGraph(void) { total_vertices = max_vertices = 0; basic_graph = 0; }
	~BasicGraph(void);
	void init(weight_t t, double);
	bool add(bvertex_t v1, bvertex_t v2, weight_t w);
	void assign_category(bvertex_t v, unsigned char cat)
	{
		if (v < max_vertices) basic_graph[v].cat = cat;
	}
	bvertex_t main(FILE *fp = stdout);
};

void free_all();
size_t read_graph(FILE *fp, BasicGraph &, weight_t t, double st);

#endif // BASIC_GRAPH_H_
