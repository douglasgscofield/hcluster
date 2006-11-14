#include <stdio.h>
#include <stdlib.h>
#include "cluster_graph.h"

// Well, I do not like these global variables. It is my old codes and I am lazy...
size_t gc_max_cluster_size = 500;
int gc_min_edge_density = 128; // 128/255 = 0.50
int gc_breaking_edge_density = 26; // 26/255 = 0.10
int gc_strict_outgroup_level = 2;
int gc_once_fail_mode = 0;
int gc_weight_resolution = 5;

extern int gc_min_weight;

#define GC_VERIFY_JOINABLE 0
#define GC_VERIFY_LINKED 1
#define GC_VERIFY_BROKEN 2

// test whether an edge is valid.
inline int gc_verify_edge(edgeinfo_t ei, CVertex *p, CVertex *q)
{
	int retval = GC_VERIFY_JOINABLE;
	if (p->v_set->size() > gc_max_cluster_size || q->v_set->size() > gc_max_cluster_size) retval = GC_VERIFY_BROKEN;
	else if ((ei&GC_EI_MASK) < gc_breaking_edge_density) retval = GC_VERIFY_BROKEN;
	else if ((ei&GC_EI_MASK) < gc_min_edge_density) retval = GC_VERIFY_LINKED;
	else if (p->cat && q->cat) {
		if (gc_strict_outgroup_level == 3) { // the most stringent
			if (p->cat == q->cat && (p->cat == 0x1 || p->cat == 0x2 || p->cat == 0x4)) retval = GC_VERIFY_JOINABLE;
			else if ((p->cat & q->cat) == 0) retval = GC_VERIFY_JOINABLE;
			else retval = GC_VERIFY_BROKEN;
		} else if (gc_strict_outgroup_level == 2) { // less stringent. The default.
			if (p->cat == q->cat) retval = (p->cat == 0x1 || p->cat == 0x2 || p->cat == 0x4)? GC_VERIFY_JOINABLE : GC_VERIFY_BROKEN;
			else if ((p->cat & q->cat) == 0 || (p->cat & q->cat) == 0x1) retval = GC_VERIFY_JOINABLE;
			else retval = GC_VERIFY_BROKEN;
		} else if (gc_strict_outgroup_level == 1) { // just for test. Do not use it.
			retval = (p->cat != 0x7 && q->cat != 0x7)? GC_VERIFY_JOINABLE : GC_VERIFY_BROKEN;
		}
	}
	return retval;
}
inline edgeinfo_t gc_cal_edge_info(edgeinfo_t ei1, edgeinfo_t ei2, size_t s1, size_t s2)
{
	bit32_t weight_all, n_edge_all;
	edgeinfo_t edge_info;
	weight_all = ((bit32_t)ei1>>GC_EI_OFFSET) * s1 + ((bit32_t)ei2>>GC_EI_OFFSET) * s2;
	n_edge_all = (bit32_t)(ei1&GC_EI_MASK) * s1 + (bit32_t)(ei2&GC_EI_MASK) * s2;
	edge_info = ((edgeinfo_t)((double)weight_all / (s1 + s2) + 0.5) << GC_EI_OFFSET)
				| ((edgeinfo_t)((double)n_edge_all / (s1 + s2) + 0.5));
	return edge_info;
}

#ifdef LIH_DEBUG
void ClusterGraph::show_vertex(cvertex_t v)
{
	CVertex *p = cluster_graph + v;
	printf("Vertex: %u; last = (%u, %u); opt = (%u, %u); optidx = %u\n",
		   v,
		   p->last>>GC_EI_OFFSET,
		   int(100.0 * (p->last&GC_EI_MASK) / GC_EI_MASK_float + 0.5),
		   p->opt>>GC_EI_OFFSET,
		   int(100.0 * (p->opt&GC_EI_MASK) / GC_EI_MASK_float + 0.5), p->optidx);
	cvertices_t::iterator iter;
	printf("v_set: %5u   ", p->v_set->size());
	for (iter = p->v_set->begin(); iter != p->v_set->end(); ++iter)
		if (isfilled(iter)) printf("%5u", iter->key);
	printf("\nn_set: %5u   ", p->n_set->size());
	cneighbour_t::iterator iter_map;
	for (iter_map = p->n_set->begin(); iter_map != p->n_set->end(); ++iter_map)
		if (isfilled(iter_map)) printf("%5d", iter_map->key);
	printf("\n");
}
#endif // LIH_DEBUG

bool ClusterGraph::add(cvertex_t v1, cvertex_t v2, weight_t w)
{
	cvertex_t tmp1, tmp2;
	
	if (v1 == v2 || w < gc_min_weight) return false;
	
	if (!conv_list2.find(v1, &tmp1)) {
		tmp1 = conv_list1.size();
		conv_list1.push_back(v1);
		conv_list2.insert(v1, tmp1);
	}
	if (!conv_list2.find(v2, &tmp2)) {
		tmp2 = conv_list1.size();
		conv_list1.push_back(v2);
		conv_list2.insert(v2, tmp2);
	}
	// allocate and construct
#ifndef GRAPH64
	if (conv_list1.size() >= 0xfffe) { // overflow
		fprintf(stderr, "Error: Size of the maximum group exceeds 64K. Please compile as -DGRAPH64\n");
		exit(1);
	}
#endif
	max_vertices = conv_list1.size();
	if (max_vertices >= total_vertices)  {
		cvertex_t tmp = total_vertices;
		total_vertices = max_vertices + BG_GRAPH_BLOCK_SIZE;
		cluster_graph = (CVertex*)realloc(cluster_graph, sizeof(CVertex) * total_vertices);
		for (cvertex_t i = tmp; i < total_vertices; ++i)
			cluster_graph[i].init(i);
	}
	// add edge and vertices
#ifndef LH3_SAVE_MEMORY
	cedge_t tmp_e = cal_edge(tmp1, tmp2);
	weight_t tmp_w;
	if (edge_set.find(tmp_e, &tmp_w)) {
		if (w <= tmp_w) return false;
	} else {
#endif
		cluster_graph[tmp1].n_set->insert(tmp2, ((edgeinfo_t)w<<GC_EI_OFFSET)|GC_EI_MASK);
		cluster_graph[tmp2].n_set->insert(tmp1, ((edgeinfo_t)w<<GC_EI_OFFSET)|GC_EI_MASK);
#ifndef LH3_SAVE_MEMORY
	}
	edge_set.insert(tmp_e, w);
#endif
	return true;
}
cvertex_t ClusterGraph::assign_category_ext(cvertex_t v, unsigned char cat)
{
	cvertex_t tmp;
	if (!conv_list2.find(v, &tmp)) {
		fprintf(stderr, "[assign_category_ext] vertex '%d' is not present in the graph.\n", v);
		return 0;
	}
	cluster_graph[tmp].cat = cat;
	return tmp;
}
void ClusterGraph::clear()
{
	max_vertices = 0;
	conv_list1.rewind();
	conv_list2.free();
#ifndef LH3_SAVE_MEMORY
	edge_set.free();
#endif
	for (cvertex_t v = 0; v < total_vertices; ++v)
		cluster_graph[v].clear(v);
}
inline void ClusterGraph::set_max(cvertex_t v)
{
	cneighbour_t::iterator iter;
	CVertex *p = cluster_graph + v;
	p->optidx = max_vertices; p->opt = 0;
	for (iter = p->n_set->begin(); iter != p->n_set->end(); ++iter) {
		if (isfilled(iter) && v < iter->key) {
			if (p->opt < iter->val && (iter->val&GC_EI_MASK) >= gc_min_edge_density) {
				p->optidx = iter->key;
				p->opt = iter->val;
			}
		}
	}
}
inline void ClusterGraph::inactivate_vertex(cvertex_t v)
{
	 CVertex *p, *q;
	 cneighbour_t::iterator iter;
	 p = cluster_graph + v;
	 for (iter = p->n_set->begin(); iter != p->n_set->end(); ++iter) {
		  if (!isfilled(iter)) continue;
		  q = cluster_graph + iter->key;
		  q->n_set->erase(v);
		  if (q->optidx == v) set_max(iter->key);
	 }
	 p->optidx = max_vertices;
	 p->opt = 0;
	 p->n_set->free(); // clear its neighbours
}
void ClusterGraph::merge(cvertex_t v1, cvertex_t v2) // v1 < v2
{
	CVertex *src1, *src2, *p;
	cneighbour_t::iterator iter;

	src1 = cluster_graph + v1;
	src2 = cluster_graph + v2;
	src1->last = src1->opt;

	{ // Update all related neighbour set (n_set). Note that invalid edges will be removed at the next step, but not here.
		edgeinfo_t ei1, ei2, edge_info;
		cvertices_t::iterator iter_set;

		for (iter = src1->n_set->begin(); iter != src1->n_set->end(); ++iter) {
			if (!isfilled(iter) || iter->key == v2) continue;
			ei1 = iter->val;
			p = cluster_graph + iter->key;
			if (src2->n_set->find(iter->key, &ei2)) // iter_map->key is also present in v2
				edge_info = gc_cal_edge_info(ei1, ei2, src1->v_set->size(), src2->v_set->size());
			else edge_info = gc_cal_edge_info(ei1, 0, src1->v_set->size(), src2->v_set->size());
			iter->val = edge_info;
			p->n_set->insert(v1, edge_info);
		}
		for (iter = src2->n_set->begin(); iter != src2->n_set->end(); ++iter) {
			if (!isfilled(iter)) continue;
			p = cluster_graph + iter->key;
			p->n_set->erase(v2);
			if (iter->key == v1) continue;
			ei2 = iter->val;
			if (!src1->n_set->find(iter->key, &ei1)) {// iter_map->key is not present in v1
				edge_info = gc_cal_edge_info(ei2, 0, src2->v_set->size(), src1->v_set->size());
				src1->n_set->insert(iter->key, edge_info);
				p->n_set->insert(v1, edge_info);
			} // else, p->n_set has already been updated
		}
		for (iter_set = src2->v_set->begin(); iter_set != src2->v_set->end(); ++iter_set)
			if (isfilled(iter_set)) src1->v_set->insert(iter_set->key);
	}
	// set src1->cat
	src1->cat |= src2->cat;
	// delete vertex src2
	src2->v_set->free();
	src2->n_set->free();
	src2->optidx = max_vertices; src2->opt = 0; // close this vertex

	// remove invalid edges, and reset affected "->optidx"
	edgeinfo_t tmp_opt = 0;
	cvertex_t tmp_optidx = max_vertices;
	src1->optidx = max_vertices; src1->opt = 0;
	if (src1->v_set->size() > gc_max_cluster_size) { // if the cluster goes too large
		 inactivate_vertex(v1);
		 return;
	}
	for (iter = src1->n_set->begin(); iter != src1->n_set->end(); ++iter) {
		if (!isfilled(iter)) continue;
		p = cluster_graph + iter->key;
		int retval = gc_verify_edge(iter->val, src1, p);
		if (retval == GC_VERIFY_BROKEN) { // then delete this neighbour/edge
			if (iter->val > tmp_opt) {// see, tmp_opt is set here, before checking whether a join is valid.
				tmp_opt = iter->val;
				tmp_optidx = iter->key;
			}
			src1->n_set->erase(iter->key); // remove iter->key from v1
			p->n_set->erase(v1); // remove v1 from iter->key
		}
		if (p->optidx == v1 || p->optidx == v2) set_max(iter->key); // recalculate the optimal vertex
		if (retval != GC_VERIFY_JOINABLE) continue;
		if (v1 < iter->key) { // reset "src1->opt"
			if (src1->opt < iter->val) {
				src1->optidx = iter->key;
				src1->opt = iter->val;
			}
		} else { // reset "p->opt"
			if (p->opt < iter->val) {
				p->optidx = v1;
				p->opt = iter->val;
			}
		}
	}
	// ok, let's see the role of tmp_opt
	if (gc_once_fail_mode && tmp_opt > src1->opt + (gc_weight_resolution<<GC_EI_OFFSET)) {
		 // which means the best join is actually invalid.
#ifdef LIH_DEBUG
		printf("[merge] best join invalid between %d and %d (%x,%x,%d)\n", v1, v2, tmp_opt, src1->opt, tmp_optidx);
		show_vertex(v1);
#endif
		inactivate_vertex(v1);
	}
}
ClusterGraph::~ClusterGraph()
{
	cvertex_t i;
	for (i = 0; i < total_vertices; ++i) {
		delete cluster_graph[i].v_set;
		delete cluster_graph[i].n_set;
	}
	free(cluster_graph);
}
cvertex_t ClusterGraph::flag_all()
{
	// initialize the opt and opt_ind in cluster_graph
	if (gc_flag & GC_VERBOSE) {
#ifndef LH3_SAVE_MEMORY
		fprintf(stderr, "----- %u vertices, %u edges -----\n",
				unsigned(conv_list2.size()), unsigned(edge_set.size()));
#else
		fprintf(stderr, "----- %u vertices -----\n", unsigned(conv_list2.size()));
#endif
		fflush(stderr);
	}
	
	for (cvertex_t v = 0; v < max_vertices; ++v) set_max(v);

	cvertex_t optidx, flag = 0;
	edgeinfo_t opt;
	CVertex *p;
	cvertex_t count = 0;
	
#ifdef LIH_DEBUG
//	for (cvertex_t v = 0; v < max_vertices; v++)
//		show_vertex(v);
#endif // LIH_DEBUG

	do {
		optidx = max_vertices; opt = 0;
		for (p = cluster_graph; p < cluster_graph + max_vertices; ++p) {
			// if the vertex is not closed
			if (p->optidx != max_vertices && opt < p->opt) {
				opt = p->opt;
				optidx = p - cluster_graph;
			}
		}
		if (optidx != max_vertices) { // find an optimal one
			merge(optidx, cluster_graph[optidx].optidx);
			++count;
#ifdef LIH_DEBUG
			show_vertex(optidx);
#endif // LIH_DEBUG
		}
		if ((gc_flag &GC_VERBOSE) && count%1000 == 0) {
			fprintf(stderr, "%u times of merge occur\n", count);
			fflush(stderr);
		}
	} while (optidx != max_vertices);

#ifdef LIH_DEBUG
	printf("END OF CLUSTER\n");
	for (cvertex_t v = 0; v < max_vertices; ++v)
		show_vertex(v);
#endif // LIH_DEBUG
	for (p = cluster_graph; p < cluster_graph + max_vertices; ++p) {
		if (p->v_set->size()) ++flag;
	}
	return flag;
}
void ClusterGraph::output(FILE *fpout, cvertex_t start)
{
	extern char **bg_name_list;
	CVertex *p;
	cvertices_t::iterator iter;
#ifndef LH3_SAVE_MEMORY
	cvertices_t::iterator iter2;
	weight_t tmp_w;
#endif
	cvertex_t flag = 0;
	size_t count;

	for (p = cluster_graph; p < cluster_graph + max_vertices; ++p) {
		if (p->v_set->size() && bg_name_list[conv_list1[p - cluster_graph]]) {
			count = p->v_set->size();
#ifndef LH3_SAVE_MEMORY
			if (gc_flag & GC_DETAIL) fputc('>', fpout);
#endif
			// calculate some basic statistic in the v_set
			if (count != 1) {
				fprintf(fpout, "%u\t%u\t%d\t%.3f\t%x\t%u\t", flag + start, start, p->last>>GC_EI_OFFSET,
					(p->last&GC_EI_MASK)/GC_EI_MASK_float, p->cat, unsigned(count));
			} else fprintf(fpout, "%u\t%u\t0\t1.000\t%x\t1\t", flag + start, start, p->cat);
			for (iter = p->v_set->begin(); iter != p->v_set->end(); ++iter)
				if (isfilled(iter)) fprintf(fpout, "%s,", bg_name_list[conv_list1[iter->key]]);
			fputc('\n', fpout);
			++flag;
#ifndef LH3_SAVE_MEMORY
			if (!(gc_flag & GC_DETAIL)) continue;
			// output detailed edge information
			if (count == 1) { fprintf(fpout, "//\n"); continue; }
			for (iter = p->v_set->begin(); iter != p->v_set->end(); ++iter) {
				if (!isfilled(iter)) continue;
				for (iter2 = p->v_set->begin(); iter2 != p->v_set->end(); ++iter2) {
					if (!isfilled(iter2)) continue;
					if (iter->key < iter2->key && edge_set.find(cal_edge2(iter->key, iter2->key), &tmp_w))
						fprintf(fpout, "%s\t%s\t%d\n", bg_name_list[conv_list1[iter->key]],
								bg_name_list[conv_list1[iter2->key]], tmp_w);
				}
			}
			fprintf(fpout, "//\n");
#endif
		}
	}
}
