#include <stdio.h>
#include <stdlib.h>
#include "basic_graph.h"
#include "hash_char.h"

size_t bg_nl_num, bg_nl_max;
char **bg_name_list = 0;
unsigned gc_flag = 0;
hash_map_char<bvertex_t> bg_name_hash;

void gc_read_category(FILE *fp, BasicGraph *bg)
{
	bit32_t tmp;
	bvertex_t v;
	char tmp1[1024];
	while (!feof(fp)) {
		fscanf(fp, "%s%u", tmp1, &tmp);
		if (bg_name_hash.find(tmp1, &v))
			bg->assign_category(v, (unsigned char)tmp);
	}
}

size_t read_graph(FILE *fp, BasicGraph &bg)
{
	bit32_t tmp;
	bvertex_t v1, v2;
	char tmp1[1024], tmp2[1024];
	int count = 0;

	if (gc_flag & GC_VERBOSE) fprintf(stderr, "***** read file *****\n");
	while (!feof(fp)) {
		fscanf(fp, "%s%s%u", tmp1, tmp2, &tmp);
		++count;
		if (!bg_name_hash.find(tmp1, &v1)) {
			v1 = bg_name_hash.size();
			bg_name_hash.insert(tmp1, v1);
			if (v1 == bg_nl_max) {
				bg_nl_max += BG_GRAPH_BLOCK_SIZE;
				bg_name_list = (char**)realloc(bg_name_list, sizeof(char*) * bg_nl_max);
				for (size_t i = v1; i < bg_nl_max; ++i)
					bg_name_list[i] = 0;
			}
			bg_name_list[v1] = (char*)malloc(strlen(tmp1) + 1);
			strcpy(bg_name_list[v1], tmp1);
		}
		if (!bg_name_hash.find(tmp2, &v2)) {
			v2 = bg_name_hash.size();
			bg_name_hash.insert(tmp2, v2);
			if (v2 == bg_nl_max) {
				bg_nl_max += BG_GRAPH_BLOCK_SIZE;
				bg_name_list = (char**)realloc(bg_name_list, sizeof(char*) * bg_nl_max);
				for (size_t i = v2; i < bg_nl_max; ++i)
					bg_name_list[i] = 0;
			}
			bg_name_list[v2] = (char*)malloc(strlen(tmp2) + 1);
			strcpy(bg_name_list[v2], tmp2);
		}
		bg.add(v1, v2, (weight_t)tmp);
		if ((gc_flag & GC_VERBOSE) && count%100000 == 0) {
			fprintf(stderr, "%d records are processed, %u vertices are stored\n",
					count, unsigned(bg_name_hash.size()));
			fflush(stderr);
		}
	}
	bg_nl_num = bg_name_hash.size();
	return bg_nl_num;
}
void free_all()
{
	for (size_t i = 0; i < bg_nl_num; ++i)
		free(bg_name_list[i]);
	free(bg_name_list);
}
