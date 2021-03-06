#include <stdio.h>
#include <stdlib.h>
#include "basic_graph.h"

int gc_min_weight = 20;

BasicGraph::~BasicGraph()
{
	for (size_t i = 0; i < total_vertices; ++i)
		free(basic_graph[i].list);
	free(basic_graph);
}
bool BasicGraph::add(bvertex_t v1, bvertex_t v2, weight_t w)
{
	if (v1 == v2) return false;
	if (w >= gc_min_weight) {
#ifndef LH3_SAVE_MEMORY
		bedge_t tmp = cal_edge(v1, v2);
		weight_t tmp_w;
#endif
		bvertex_t tmp1, tmp2;
		tmp1 = (v1 > v2)? v1 : v2;
		if (tmp1 + 1 > max_vertices) max_vertices = tmp1 + 1;
		if (tmp1 >= total_vertices) {
			tmp2 = total_vertices;
			total_vertices = tmp1 + BG_GRAPH_BLOCK_SIZE;
			basic_graph = (BVertex*)realloc(basic_graph, sizeof(BVertex) * total_vertices);
			for (bvertex_t i = tmp2; i < total_vertices; ++i)
				basic_graph[i].init();
		}
#ifndef LH3_SAVE_MEMORY
		if (!(gc_flag & GC_NO_CLUSTER) && edge_set.find(tmp, &tmp_w)) {
			if (w <= tmp_w) return false;
		} else {
#endif
			basic_graph[v1].add(v2, w);
			basic_graph[v2].add(v1, w);
#ifndef LH3_SAVE_MEMORY
		}
		if (!(gc_flag & GC_NO_CLUSTER)) edge_set.insert(tmp, w);
#endif
		return true;
	}
	return false;
}
bvertex_t BasicGraph::flag_one(bvertex_t v, bvertex_t f, bvertex_t *list, bvertex_t *rst)
{
	bvertex_t *q = list, count = 0, *s;
	BVertex *p, *p2;
	EdgeStruct<bvertex_t> *r;

	*q++ = v; s = rst;
	while (q != list) {
		q--;
		p = basic_graph + *q;
		p->flag = f;
		++count;
		if (rst) *s++ = *q; // store the vertex
		for (r = p->list; r < p->list + p->num; ++r) {
			p2 = basic_graph + r->v;
			if (p2->flag == max_vertices + 1) {
				// The vertex p2 has not been used.
				*q++ = r->v;
				// Add p2 to the list and flag it.
				p2->flag = max_vertices;
			}
		}
	}
	return count;
}
bvertex_t BasicGraph::main(FILE *fp)
{
	bvertex_t *list = new bvertex_t[max_vertices];
	bvertex_t *rst = new bvertex_t[max_vertices];
	BVertex *r1;
	EdgeStruct<bvertex_t> *r2;
	bvertex_t i, j, flag, tmp, count, tmp2;
	cvertex_t count2;
	extern char **bg_name_list;
	extern size_t bg_nl_num;

	if (gc_flag & GC_VERBOSE)
#ifndef LH3_SAVE_MEMORY
		fprintf(stderr, "+++++ %u vertices, %u edges +++++\n",
				unsigned(max_vertices), unsigned(edge_set.size()));
#else
		fprintf(stderr, "+++++ %u vertices +++++\n", unsigned(max_vertices));
#endif
	for (i = 0; i < max_vertices; i++)
		basic_graph[i].flag = max_vertices + 1;
	for (i = 0, flag = 0, tmp =0, tmp2 = 0; i < max_vertices; ++i) {
		if (basic_graph[i].flag < max_vertices + 1)
			continue;
		count = flag_one(i, flag, list, rst);
		tmp += count;
		if (count == 1)	continue;
		if (gc_flag & GC_NO_CLUSTER) { // no cluster
			fprintf(fp, "%d\t%d\t", flag, count);
			for (j = 0; j < count; ++j) {
				fprintf(fp, "%s,", bg_name_list[rst[j]]);
				free(bg_name_list[rst[j]]);
				bg_name_list[rst[j]] = 0; // flag used
			}
			fputc('\n', fp);
			++flag;
			continue;
		}
		if (gc_flag & GC_VERBOSE) {
			fprintf(stderr, "%d vertices in single-edge cluster %u\n", count, tmp2);
			fflush(stderr);
		}
		// cluster
		bvertex_t v_to_add;
		for (j = 0; j < count; ++j) {
			v_to_add = rst[j];
			r1 = basic_graph + v_to_add;
			for (r2 = r1->list; r2 < r1->list + r1->num; ++r2)
				if (v_to_add < r2->v) cgraph.add(v_to_add, r2->v, r2->w);
			if (r1->cat) cgraph.assign_category_ext(v_to_add, r1->cat);
		}
		count2 = cgraph.flag_all();
		cgraph.output(fp, tmp2);
		cgraph.clear();
		tmp2 += count2;
		++flag;
	}
	if (tmp != max_vertices)
		fprintf(stderr, "Warning: Some inconsistencies occur.\n");
	
	delete[] list;
	delete[] rst;
	
	if (gc_flag & GC_NO_CLUSTER) {
		for (i = 0; i < bg_nl_num; ++i) // output remained vertices
			if (bg_name_list[i])
				fprintf(fp, "%d\t1\t%s,\n", flag++, bg_name_list[i]);
		free_all();
		exit(0);
	}
	return(flag);
}
