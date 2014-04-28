#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "basic_graph.h"
#include "cluster_graph.h"

void usage()
{
	fprintf(stderr, "\n");
	fprintf(stderr, "Program : hcluster_sg (Hierarchically clustering on a sparse graph)\n");
	fprintf(stderr, "Version : %s, build %s\n", GC_VERSION, GC_BUILD);
	fprintf(stderr, "Contact : Heng Li <lh3lh3@gmail.com>\n\n");
	fprintf(stderr, "Usage   : hcluster_sg [options] [input_file]\n\n");
	fprintf(stderr, "Options : -w NUM     minimum edge weight [%d]\n", int(gc_min_weight));
	fprintf(stderr, "          -s FNUM    minimum edge density between a join [%.2f]\n", 1.0*gc_min_edge_density/GC_EI_MASK);
	fprintf(stderr, "          -m NUM     maximum size [%lu]\n", gc_max_cluster_size);
	fprintf(stderr, "          -o STRING  output file [stdout]\n");
	fprintf(stderr, "          -c         only find single-linkage clusters (bypass h-cluster)\n");
	fprintf(stderr, "          -v         verbose mode\n");
	fprintf(stderr, "          -h         help\n");
#ifndef LH3_SAVE_MEMORY
	fprintf(stderr, "          -d         detailed edge information\n");
#endif
	fprintf(stderr, "\nAdvanced Options:\n\n");
	fprintf(stderr, "          -b FNUM    breaking edge density [%.2f]\n", 1.0*gc_breaking_edge_density/GC_EI_MASK);
	fprintf(stderr, "          -O         the once-fail-inactive-forever mode\n");
	fprintf(stderr, "          -r         weight resolution for '-O' [%d]\n", gc_weight_resolution);
	fprintf(stderr, "          -C FILE    category file\n");
	fprintf(stderr, "          -L NUM     stringent level ('3' is the strictest) [%d]\n", gc_strict_outgroup_level);
	fprintf(stderr, "\n");
	exit(1);
}

int main(int argc, char *argv[])
{	
	int c;
	FILE *fp = 0, *fpout = stdout;
	FILE *fpcat = 0;
#ifndef LH3_SAVE_MEMORY
	while((c = getopt(argc, argv, "w:s:o:hdvcm:C:L:Or:b:")) >= 0) {
#else
	while((c = getopt(argc, argv, "w:s:o:hvcm:C:L:Or:b:")) >= 0) {
#endif
		switch(c) {
			case 'w': gc_min_weight = atoi(optarg); break;
		    case 'b': gc_breaking_edge_density = int(GC_EI_MASK * atof(optarg) + 1.0); break;
			case 's': gc_min_edge_density = int(GC_EI_MASK * atof(optarg) + 1.0); break;
			case 'm': gc_max_cluster_size = atoi(optarg); break;
			case 'o': fpout = fopen(optarg, "w+"); break;
#ifndef LH3_SAVE_MEMORY
			case 'd': gc_flag |= GC_DETAIL; break;
#endif
			case 'v': gc_flag |= GC_VERBOSE; break;
			case 'c': gc_flag |= GC_NO_CLUSTER; break;
			case 'C': fpcat = fopen(optarg, "r");
					  if (fpcat == 0)
					  	fprintf(stderr, "[main] Fail to open file '%s'. Continue anyway.", optarg);
					  break;
			case 'L': gc_strict_outgroup_level = atoi(optarg); break;
			case 'O': gc_once_fail_mode = 1; break;
			case 'r': gc_weight_resolution = atoi(optarg); break;
			case 'h': usage();
			default : usage();
		}
	}
	if (optind < argc) {
		if (argv[optind][0] == '-') {
			fp = stdin;
		} else {
			fp = fopen(argv[optind], "r");
			if (!fp) {
				fprintf(stderr, "Error: Cannot open the file %s\n", argv[optind]);
				exit(1);
			}
		}
	} else usage();
	BasicGraph bg;
	read_graph(fp, bg);
	if (fpcat) {
		gc_read_category(fpcat, &bg);
		fclose(fpcat);
	}
	if (fp != stdin) fclose(fp);
	bg.main(fpout);
	free_all();
	return 0;
}
