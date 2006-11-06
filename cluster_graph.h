#ifndef CLUSTER_GRAPH_H_
#define CLUSTER_GRAPH_H_

#include "hash_misc.h"
#include "svector.h"
#include "basic.h"

#ifdef GRAPH64
typedef bit32_t cvertex_t;
typedef bit64_t	cedge_t;
#else
typedef bit16_t cvertex_t;
typedef bit32_t	cedge_t;
#endif

extern int gc_weight_w;

struct VertexInfo
{
	weight_t w;
	cedge_t num, all;
	VertexInfo() { w = 0; num = 0; all = 0; };
	inline void clear() { w = 0; num = 0; all = 0; };
};

inline bit32_t gc_cal_q(const VertexInfo &v)
{
	return (bit32_t)100 * v.num / (v.all + 1) + v.w * gc_weight_w;
}

//inline bool operator< (const VertexInfo &e1, const VertexInfo &e2)
//{
	// Note: 100*e1.num may cause overflow for some special graph.
//	if (100 * e1.num / (e1.all + 1) + e1.w * gc_weight_w <
//		100 * e2.num / (e2.all + 1) + e2.w * gc_weight_w) return true;
//	return false;
//}

struct CVertex
{
	VertexInfo vi, opt;
	cvertex_t opt_ind;
	bit32_t opt_q;
	hash_set_misc<cvertex_t> *v_set; // the contracted vertices
	hash_set_misc<cvertex_t> *n_set; // the neighbours
	inline void init(cvertex_t i)
	{
		vi.clear(); opt.clear(); opt_q = 0;
		v_set = new hash_set_misc<cvertex_t>;
		n_set = new hash_set_misc<cvertex_t>;
		v_set->insert(i);
	}
	inline void clear(cvertex_t i)
	{
		vi.clear(); opt.clear(); opt_q = 0;
		v_set->free();
		n_set->free();
		v_set->insert(i);
	}
};

class ClusterGraph
{
	// in a cluster_graph, the index of the current vertex must be GREATER
	// than its opt_ind.
	inline void set_max(cvertex_t v, CVertex *p);
protected:
	cvertex_t total_vertices, max_vertices;
	CVertex *cluster_graph;
	weight_t threshold;
	svector<bit32_t> conv_list1;
	hash_map_misc<cvertex_t, bit32_t> conv_list2;
	hash_map_misc<weight_t, cedge_t> edge_set;
	void merge(cvertex_t v1, cvertex_t v2, VertexInfo *rst_vi = 0);
	void init_opt();
public:
	ClusterGraph() { cluster_graph = 0; total_vertices = max_vertices = 0; };
	~ClusterGraph();
#ifdef LIH_DEBUG
	void show_vertex(cvertex_t);
#endif // LIH_DEBUG
	void init(weight_t thres, double);
	bool add(bit32_t v1, bit32_t v2, weight_t w);
	cvertex_t flag_all();
	void output(FILE*, bit32_t);
	void clear();
};

#endif // CLUSTER_GRAPH_H_
