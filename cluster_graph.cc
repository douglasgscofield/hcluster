#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include "cluster_graph.h"

int gc_weight_w = 2;

static double CG_satur_thres = 0.70;

static bool cal_vertex_info(VertexInfo &dest, CVertex *src1,
		CVertex *src2, hash_map_misc<weight_t, cedge_t> &edge_set, bool flag = true)
{
	cedge_t n_edge, etmp;
	weight_t w;
	cedge_t tmp;
	
	etmp = src1->v_set->size() * src2->v_set->size();
	hash_set_misc<cvertex_t>::iterator p1, p2;
	tmp = 0;
	n_edge = 0;
	for (p1 = src1->v_set->begin(); p1.not_end(); p1.inc()) {
		for (p2 = src2->v_set->begin(); p2.not_end(); p2.inc()) {
			if (edge_set.find(cal_edge(*p1, *p2), w)) {
				tmp += w;
				++n_edge;
			}
		}
	}
	if (flag && double(n_edge) / etmp < CG_satur_thres)
		return false; // cannot be merged
	dest.all = src1->vi.all + src2->vi.all + etmp;
	dest.num = src1->vi.num + src2->vi.num + n_edge;
	dest.w = weight_t((double(src1->vi.w) * src1->vi.num + src2->vi.w 
				* src2->vi.num + tmp) / dest.num + 0.5);
	return true;
}

#ifdef LIH_DEBUG
void ClusterGraph::show_vertex(cvertex_t v)
{
	CVertex *p = cluster_graph + v;
	printf("Vertex: %u; vi = (%u, %u, %u); opt = (%u, %u, %u); opt_ind = %u\n",
			v, p->vi.w, p->vi.num, p->vi.all,
			p->opt.w, p->opt.num, p->opt.all, p->opt_ind);
	hash_set_misc<cvertex_t>::iterator iter;
	printf("v_set: %5u   ", p->v_set->size());
	for (iter = p->v_set->begin(); iter.not_end(); iter.inc())
		printf("%5u", *iter);
	printf("\nn_set: %5u   ", p->n_set->size());
	for (iter = p->n_set->begin(); iter.not_end(); iter.inc())
		printf("%5d", *iter);
	printf("\n");
}
#endif // LIH_DEBUG

bool ClusterGraph::add(bit32_t v1, bit32_t v2, weight_t w)
{
	cvertex_t tmp1, tmp2;
	
	if (v1 == v2 || w < threshold) return false;
	
	if (!conv_list2.find(v1, tmp1)) {
		tmp1 = conv_list1.size();
		conv_list1.push_back(v1);
		conv_list2.insert(v1, tmp1);
	}
	if (!conv_list2.find(v2, tmp2)) {
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
	cedge_t tmp_e = cal_edge(tmp1, tmp2);
	weight_t tmp_w;
	if (edge_set.find(tmp_e, tmp_w)) {
		if (w <= tmp_w) return false;
	} else {
		cluster_graph[tmp1].n_set->insert(tmp2);
		cluster_graph[tmp2].n_set->insert(tmp1);
	}
	edge_set.insert(tmp_e, w);
	return true;
}
void ClusterGraph::clear()
{
	max_vertices = 0;
	conv_list1.rewind();
	conv_list2.free();
	edge_set.free();
	for (cvertex_t v = 0; v < total_vertices; ++v)
		cluster_graph[v].clear(v);
}
inline void ClusterGraph::set_max(cvertex_t v, CVertex *p)
{
	CVertex *q;
	hash_set_misc<cvertex_t>::iterator iter;
	VertexInfo tmp_vi;
	p->opt_ind = max_vertices;
	p->opt.num = 0; p->opt.w = 0;

	for (iter = p->n_set->begin(); iter.not_end(); iter.inc()) {
		q = cluster_graph + *iter;
		if (v < *iter) {
			cal_vertex_info(tmp_vi, p, q, edge_set);
			if (p->opt < tmp_vi) {
				p->opt = tmp_vi;
				p->opt_ind = *iter;
			}
		}
	}
}
void ClusterGraph::merge(cvertex_t v1, cvertex_t v2) // v1 < v2
{
	CVertex *src1, *src2, *p;
	hash_set_misc<cvertex_t>::iterator iter;
	VertexInfo tmp_vi;

	src1 = cluster_graph + v1;
	src2 = cluster_graph + v2;

	// calculate vertex information and store it in src->vi
	cal_vertex_info(tmp_vi, src1, src2, edge_set);
	//if (tmp_vi.all == 0)
	//	fprintf(stderr, "(%u,%u) : (%u,%u) (%u,%u) (%u,%u)\n", v1, v2, tmp_vi.num, tmp_vi.all,
	//			src1->vi.num, src1->vi.all, src2->vi.num, src2->vi.all);
	src1->vi = tmp_vi;

	// join set src2->v_set into src1->v_set, and so it is with n_set
	for (iter = src2->v_set->begin(); iter.not_end(); iter.inc())
		src1->v_set->insert(*iter);
	for (iter = src2->n_set->begin(); iter.not_end(); iter.inc()) {
		cluster_graph[*iter].n_set->erase(v2);
		if (*iter != v1) {
			src1->n_set->insert(*iter);
			cluster_graph[*iter].n_set->insert(v1);
		}
	}
	src2->v_set->free(); // free the memory
	src2->n_set->free();
	src2->opt_ind = max_vertices; // close this vertex

	// if no vertex greater than the current is found,
	// this vertex will be closed.
	src1->opt_ind = max_vertices;
	src1->opt.num = 0; src1->opt.w = 0;

	// re-calculate the related edges
	for (iter = src1->n_set->begin(); iter.not_end(); iter.inc()) {
		p = cluster_graph + *iter;
		if (!cal_vertex_info(tmp_vi, p, src1, edge_set)) {
			// delete this edge
			src1->n_set->erase(*iter);
			p->n_set->erase(v1);
			if (p->opt_ind == v1 || p->opt_ind == v2)
				set_max(*iter, p); // recalculate the optimal node
			continue;
		}
		if (v1 < *iter) {
			if (src1->opt < tmp_vi) {
				src1->opt = tmp_vi;
				src1->opt_ind = *iter;
			}
			if (p->opt_ind == v2) set_max(*iter, p); // p->opt_ind != v1
		} else {
			if (p->opt < tmp_vi) {
				p->opt = tmp_vi;
				p->opt_ind = v1;
			}
		}
	}
}
void ClusterGraph::init(weight_t thres, double st)
{
	threshold = thres;
	CG_satur_thres = st;
}
ClusterGraph::~ClusterGraph()
{
	cvertex_t i;
	for (i = 0; i < total_vertices; ++i) {
		delete cluster_graph[i].v_set;
		delete cluster_graph[i].n_set;
	}
	delete[] cluster_graph;
}
void ClusterGraph::init_opt()
{
	CVertex *p;
	weight_t opt;
	cvertex_t v, opt_ind;
	weight_t w;
	hash_set_misc<cvertex_t>::iterator iter;
	
	for (v = 0; v < max_vertices; ++v) {
		opt = 0;
		opt_ind = max_vertices;
		p = cluster_graph + v;
		for (iter = p->n_set->begin(); iter.not_end(); iter.inc()) {
			if (*iter > v) {
				edge_set.find(cal_edge2(v, *iter), w);
				if (w > opt) {
					opt = w; opt_ind = *iter;
				}
			}
		}
		p->opt.w = opt;
		p->opt_ind = opt_ind;
	}
}
cvertex_t ClusterGraph::flag_all()
{
	// initialize the opt and opt_ind in cluster_graph
	if (gc_flag & GC_VERBOSE) {
		fprintf(stderr, "----- %u vertices, %u edges -----\n",
				unsigned(conv_list2.size()), unsigned(edge_set.size()));
		fflush(stderr);
	}
	
	init_opt();

	cvertex_t opt_ind, flag = 0;
	VertexInfo opt;
	hash_set_misc<cvertex_t>::iterator iter;
	CVertex *p;
	cvertex_t count = 0;
	
#ifdef LIH_DEBUG
//	for (cvertex_t v = 0; v < max_vertices; v++)
//		show_vertex(v);
#endif // LIH_DEBUG

	do {
		opt_ind = max_vertices;
		opt.num = 0; opt.w = 0;
		for (p = cluster_graph; p < cluster_graph + max_vertices; ++p) {
			// if the vertex is not closed
			if (p->opt_ind != max_vertices && opt < p->opt) {
				opt = p->opt;
				opt_ind = p - cluster_graph;
			}
		}
		if (opt_ind != max_vertices) { // find an optimal one
			merge(opt_ind, cluster_graph[opt_ind].opt_ind);
			++count;
#ifdef LIH_DEBUG
			show_vertex(opt_ind);
#endif // LIH_DEBUG
		}
		if ((gc_flag &GC_VERBOSE) && count%1000 == 0) {
			fprintf(stderr, "%u times of merge occur\n", count);
			fflush(stderr);
		}
	} while (opt_ind != max_vertices);

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
void ClusterGraph::output(FILE *fpout, bit32_t start)
{
	extern char **bg_name_list;
	CVertex *p;
	hash_set_misc<cvertex_t>::iterator iter, iter2;
	bit32_t flag = 0;
	size_t count;
	svector<weight_t> tmp_array;
	weight_t tmp_w;
	bit64_t sum;

	for (p = cluster_graph; p < cluster_graph + max_vertices; ++p) {
		if (p->v_set->size() && bg_name_list[conv_list1[p - cluster_graph]]) {
			count = p->v_set->size();
			if (gc_flag & GC_DETAIL) fputc('>', fpout);
			// calculate some basic statistic in the v_set
			tmp_array.rewind();
			sum = 0;
			if (count != 1) {
				for (iter = p->v_set->begin(); iter.not_end(); iter.inc()) {
					for (iter2 = p->v_set->begin(); iter2.not_end(); iter2.inc()) {
						if (*iter < *iter2 && edge_set.find(cal_edge2(*iter, *iter2), tmp_w)) {
							 tmp_array.push_back(tmp_w);
							 sum += tmp_w;
						}
					}
				}
				tmp_array.sort();
				fprintf(fpout, "%u\t%u\t%d\t%d\t%.3f\t%u\t", flag + start, start,
						int(double(sum)/tmp_array.size()+0.5),
						int(tmp_array[tmp_array.size()/2]),
						double(p->vi.num)/p->vi.all, unsigned(count));
			} else fprintf(fpout, "%u\t%u\t0\t0\t1.000\t1\t", flag + start, start);
			for (iter = p->v_set->begin(); iter.not_end(); iter.inc())
				fprintf(fpout, "%s,", bg_name_list[conv_list1[*iter]]);
			fputc('\n', fpout);
			++flag;
			if (!(gc_flag & GC_DETAIL)) continue;
			// output detailed edge information
			if (count == 1) { fprintf(fpout, "//\n"); continue; }
			for (iter = p->v_set->begin(); iter.not_end(); iter.inc()) {
				for (iter2 = p->v_set->begin(); iter2.not_end(); iter2.inc()) {
					if (*iter < *iter2 && edge_set.find(cal_edge2(*iter, *iter2), tmp_w))
						fprintf(fpout, "%s\t%s\t%d\n", bg_name_list[conv_list1[*iter]],
								bg_name_list[conv_list1[*iter2]], tmp_w);
				}
			}
			fprintf(fpout, "//\n");
		}
	}
}
