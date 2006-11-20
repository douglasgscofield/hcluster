#ifndef BASIC_H_
#define BASIC_H_

#include <malloc.h>
#include "stdhash.H"
#include "svector.h"

const int BG_BLOCK_SIZE = 32;
const int BG_GRAPH_BLOCK_SIZE = 1024;

const unsigned GC_VERBOSE = 0x0001;
#ifndef LH3_SAVE_MEMORY
const unsigned GC_DETAIL = 0x0002;
#endif
const unsigned GC_NO_CLUSTER = 0x0004;

typedef unsigned char weight_t;

template <class VTYPE>
struct EdgeStruct
{
	VTYPE v;
	weight_t w;
};
template <class VTYPE>
struct BasicVertex
{
	VTYPE flag;
	unsigned char cat;
	size_t max, num;
	EdgeStruct<VTYPE> *list;

	BasicVertex(void) { init(); }
	~BasicVertex(void) { free(list); }
	void init() { num = 0; max = 0; list = 0; flag = 0; cat = 0; }
	inline void add(VTYPE v, weight_t w)
	{
		if (num == max) {
			max += BG_BLOCK_SIZE;
			list = (EdgeStruct<VTYPE>*) realloc(list, sizeof(EdgeStruct<VTYPE>) * max);
		}
		list[num].v = v;
		list[num].w = w;
		++num;
	}
};

#ifndef LH3_SAVE_MEMORY
#ifdef GRAPH64
inline bit64_t cal_edge(bit32_t v1, bit32_t v2)
{
	if (v1 < v2) return (bit64_t(v1) << 32) | v2;
		else return (bit64_t(v2) << 32) | v1;
}
inline bit64_t cal_edge2(bit32_t v1, bit32_t v2)
{
	return (bit64_t(v1) << 32) | v2;
}
#else
inline bit32_t cal_edge(bit16_t v1, bit16_t v2)
{
	if (v1 < v2) return (bit32_t(v1) << 16) | v2;
		else return (bit32_t(v2) << 16) | v1;
}
inline bit32_t cal_edge2(bit16_t v1, bit16_t v2) // v1 < v2
{
	return (bit32_t(v1) << 16) | v2;
}
#endif
#endif // LH3_SAVE_MEMORY

extern unsigned gc_flag;
void free_all();

#endif // BASIC_H_
