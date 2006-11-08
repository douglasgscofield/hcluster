#ifndef CLUSTER_GRAPH_H_
#define CLUSTER_GRAPH_H_

#include "hash_misc.h"
#include "svector.h"
#include "basic.h"

#ifdef GRAPH64
typedef bit32_t cvertex_t;
#ifndef LH3_SAVE_MEMORY
typedef bit64_t	cedge_t;
#endif
#else
typedef bit16_t cvertex_t;
#ifndef LH3_SAVE_MEMORY
typedef bit32_t	cedge_t;
#endif
#endif

typedef bit16_t edgeinfo_t;
#define GC_EI_OFFSET		8
#define GC_EI_MASK			0xff
#define GC_EI_MASK_float	255.0

typedef hash_set_misc<cvertex_t> cvertices_t;
typedef hash_map_misc<edgeinfo_t, cvertex_t> cneighbour_t;

extern size_t gc_max_cluster_size;
extern int gc_min_edge_density;
extern int gc_strict_outgroup_level;
extern int gc_once_fail_mode;

struct CVertex
{
	cvertex_t optidx;
	edgeinfo_t opt, last;
	unsigned char cat;
	cvertices_t *v_set; // the contracted vertices
	cneighbour_t *n_set; // the neighbours
	inline void init(cvertex_t i)
	{
		opt = last = 0;
		cat = 0;
		v_set = new cvertices_t;
		n_set = new cneighbour_t;
		v_set->insert(i);
	}
	inline void clear(cvertex_t i)
	{
		opt = last = 0;
		cat = 0;
		v_set->free(); // this is different from "delete v_set;"
		n_set->free();
		v_set->insert(i);
	}
};

class ClusterGraph
{
	// in a cluster_graph, the index of the current vertex must be GREATER
	// than its opt_ind.
	inline void set_max(cvertex_t v);
protected:
	cvertex_t total_vertices, max_vertices;
	CVertex *cluster_graph;
	weight_t threshold;
	svector<cvertex_t> conv_list1;
	hash_map_misc<cvertex_t, cvertex_t> conv_list2;
#ifndef LH3_SAVE_MEMORY
	hash_map_misc<weight_t, cedge_t> edge_set;
#endif
	void merge(cvertex_t v1, cvertex_t v2);
	void init_opt();
public:
	ClusterGraph() { cluster_graph = 0; total_vertices = max_vertices = 0; };
	~ClusterGraph();
#ifdef LIH_DEBUG
	void show_vertex(cvertex_t);
#endif // LIH_DEBUG
	void init(weight_t thres, double);
	bool add(cvertex_t v1, cvertex_t v2, weight_t w);
	cvertex_t assign_category_ext(cvertex_t v, unsigned char cat); // "ext" means one should look up conv_list2 for the real vertex index
	cvertex_t flag_all();
	void output(FILE*, cvertex_t);
	void clear();
};

#endif // CLUSTER_GRAPH_H_
