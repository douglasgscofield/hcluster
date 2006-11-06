#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "basic_graph.h"
#include "cluster_graph.h"

extern size_t gc_max_cluster_size;
static weight_t min_weight = 20;
static double min_satur = 0.70;

void usage()
{
	fprintf(stderr, "\n");
	fprintf(stderr, "Program : hcluster_sg (Hierarchically clustering on a sparse graph)\n");
	fprintf(stderr, "Version : 0.4.0, on 05 November, 2006\n");
	fprintf(stderr, "Contact : Heng Li <lh3lh3@gmail.com>\n\n");
	fprintf(stderr, "Usage   : hcluster_sg [options] [input_file]\n\n");
	fprintf(stderr, "Options : -w NUM     minimum edge weight, default is %d\n", int(min_weight));
	fprintf(stderr, "          -s FNUM    minimum saturation ratio, default is %.2f\n", min_satur);
	fprintf(stderr, "          -m NUM     maximum size [%d]\n", gc_max_cluster_size);
	fprintf(stderr, "          -o STRING  output file, default is stdout\n");
	fprintf(stderr, "          -d         detailed edge information\n");
	fprintf(stderr, "          -c         just find component, do not cluster\n");
	fprintf(stderr, "          -v         verbose mode\n");
	fprintf(stderr, "          -h         help\n\n");
	exit(1);
}

int main(int argc, char *argv[])
{	
	int c;
	FILE *fp, *fpout = stdout;
	while((c = getopt(argc, argv, "w:s:o:hdvcm:")) >= 0) {
		switch(c) {
			case 'w': min_weight = atoi(optarg); break;
			case 's': min_satur = atof(optarg); break;
			case 'm': gc_max_cluster_size = atoi(optarg); break;
			case 'o': fpout = fopen(optarg, "w+"); break;
			case 'd': gc_flag |= GC_DETAIL; break;
			case 'v': gc_flag |= GC_VERBOSE; break;
			case 'c': gc_flag |= GC_NO_CLUSTER; break;
			case 'h': usage();
			default : usage();
		}
	}
	if (optind < argc) {
		fp = fopen(argv[optind], "r");
		if (!fp) {
			fprintf(stderr, "Error: Cannot open the file %s\n", argv[optind]);
			exit(1);
		}
	} else fp = stdin;
	BasicGraph bg;
	read_graph(fp, bg, min_weight, min_satur);
	fclose(fp);
	bg.main(fpout);
	free_all();
	return 0;
}
