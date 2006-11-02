#ifndef BASIC_H_
#define BASIC_H_

#include <malloc.h>
#include "hash_misc.h"
#include "hash_char.h"
#include "svector.h"

const int BG_BLOCK_SIZE = 32;
const int BG_GRAPH_BLOCK_SIZE = 1024;

const unsigned GC_VERBOSE = 0x0001;
const unsigned GC_DETAIL = 0x0002;
const unsigned GC_NO_CLUSTER = 0x0004;

// (vertex_t<<16)|vertex_t must be bit32_t for it will be stored
// in my hash (see below cal_edge()). I think this will be revised later

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
	size_t max, num;
	EdgeStruct<VTYPE> *list;

	BasicVertex(void) { init(); }
	~BasicVertex(void) { free(list); }
	void init() { num = 0; max = 0; list = 0; flag = 0; }
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

inline bit64_t cal_edge(bit32_t v1, bit32_t v2)
{
	if (v1 < v2) return (bit64_t(v1) << 32) | v2;
		else return (bit64_t(v2) << 32) | v1;
}
inline bit64_t cal_edge2(bit32_t v1, bit64_t v2)
{
	return (bit64_t(v1) << 32) | v2;
}
inline bit32_t cal_edge(bit16_t v1, bit16_t v2)
{
	if (v1 < v2) return (bit32_t(v1) << 16) | v2;
		else return (bit32_t(v2) << 16) | v1;
}
inline bit32_t cal_edge2(bit16_t v1, bit16_t v2) // v1 < v2
{
	return (bit32_t(v1) << 16) | v2;
}

extern unsigned gc_flag;
void free_all();

#endif // BASIC_H_
