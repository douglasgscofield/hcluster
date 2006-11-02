/* This hash table is a closed hash using double hashing method. */
#ifndef HASH_COM_H_
#define HASH_COM_H_

#include <string.h>
#include <malloc.h>

typedef unsigned int bit32_t;
typedef unsigned long long bit64_t;
typedef unsigned short bit16_t;

const double __lih_HASH_UPPER = 0.90;
const int __lih_HASH_PRIME_SIZE = 30;

/* kinds of hash functions for string */

inline bit32_t __lih_sgi_hash_string(const char* s)
{
	bit32_t h = 0;
	for ( ; *s; s++)
		h = 5 * h + *s;
	return h;
}
inline bit32_t __lih_ELF_hash_string(const char *key)
{
	bit32_t g, h = 0;
	while (*key) {
		h = (h << 4) + *key++;
		g = h & 0xf0000000ul;
		if (g) h ^= g >> 24;
		h &= ~g;
	}
	return h;
}

// This will be the default hashing function for string.
// Do a web search "g_str_hash X31_HASH" for more information.

inline bit32_t __lih_X31_hash_string(const char *s)
{
	bit32_t h = 0;
	for ( ; *s; s++)
		h = (h << 5) - h + *s;
	return h;
}

/* kinds of hash functions for bit32_t */

inline bit32_t __lih_Knuth_hash_int(bit32_t key)
{
	return key * 2654435761ul;
}
// Note that key = 0 will cause a key zero
inline bit32_t __lih_Jenkins_hash_int(bit32_t key)
{
	key += (key << 12);
	key ^= (key >> 22);
	key += (key << 4);
	key ^= (key >> 9);
	key += (key << 10);
	key ^= (key >> 2);
	key += (key << 7);
	key ^= (key >> 12);
	return key;
}
inline bit64_t __lih_Jenkins_hash_64(bit64_t key)
{
	key += ~(key << 32);
	key ^= (key >> 22);
	key += ~(key << 13);
	key ^= (key >> 8);
	key += (key << 3);
	key ^= (key >> 15);
	key += ~(key << 27);
	key ^= (key >> 31);
	return key;
}

// This will be the default function for bit32_t

inline bit32_t __lih_Wang_hash_int(bit32_t key)
{
	key += ~(key << 15);
	key ^=  (key >> 10);
	key +=  (key << 3);
	key ^=  (key >> 6);
	key += ~(key << 11);
	key ^=  (key >> 16);
	return key;
}

/* default hash functions for "bit32_t" and "const char*" */

inline bit32_t __lih_hash_fun(bit32_t key)
{
#ifndef LIH_HASH_INT
	return __lih_Wang_hash_int(key);
#else
	return key;
#endif
}
inline bit32_t __lih_hash_fun(const char *key)
{
	return __lih_X31_hash_string(key);
}
inline bit32_t __lih_hash_fun(bit64_t key)
{
#ifdef LIH_HASH_INT
	return bit32_t(__lih_Jenkins_hash_64(key));
#else
	return bit32_t(key>>16) ^ bit32_t(key);
#endif
}
inline bit32_t __lih_hash_fun(bit16_t key)
{
#ifndef LIH_HASH_INT
	return __lih_Wang_hash_int(bit32_t(key));
#else
	return bit32_t(key);
#endif
}

/* judge equal for "bit32_t" and "const char*" */

inline bool __lih_key_equal(bit32_t a, bit32_t b)
{
	return a == b;
}
inline bool __lih_key_equal(const char *a, const char *b)
{
	return strcmp(a, b) == 0;
}
inline bool __lih_key_equal(bit64_t a, bit64_t b)
{
	return a == b;
}
inline bool __lih_key_equal(bit16_t a, bit16_t b)
{
	return a == b;
}

/* prime table */

static const bit32_t __lih_prime_list[__lih_HASH_PRIME_SIZE] =
{
  0ul,          3ul,          53ul,         97ul,         193ul,
  389ul,        769ul,        1543ul,       3079ul,       6151ul,
  12289ul,      24593ul,      49157ul,      98317ul,      196613ul,
  393241ul,     786433ul,     1572869ul,    3145739ul,    6291469ul,
  12582917ul,   25165843ul,   50331653ul,   100663319ul,  201326611ul,
  402653189ul,  805306457ul,  1610612741ul, 3221225473ul, 4294967291ul
};

template <class KeyType, class TYPE>
inline TYPE *__lih_hash_insert_aux(TYPE *vkp, size_t m, KeyType key)
{
	bit32_t inc, k, i, site;
	site = m;
	k = __lih_hash_fun(key);
	i = k % m;
	inc = 1 + k % (m - 1);
	
/*	this line will be useful for fixed-size hash
	size_t last = (i + (m - 1) * inc) % m;
	while (i != last && !vkp[i].isempty && !__lih_key_equal(vkp[i].key, key)) {
		if (vkp[i].isdel) site = i;
		if (i + inc >= m) {
			i = i + inc - m;
		} else i += inc;
	}*/ 
	// I think the codes below is more appropriate, for the "last" above
	// may miss one element in the hash.
	bit32_t last = i;
	while (!vkp[i].isempty && !__lih_key_equal(vkp[i].key, key)) {
		if (vkp[i].isdel) site = i;
		if (i + inc >= m) {
			i = i + inc - m;
		} else i += inc;
		if (i == last) return vkp + site;
	}
	if (vkp[i].isempty && site != m) return vkp + site;
		else return vkp + i;
}
template <class KeyType, class TYPE>
inline TYPE *__lih_hash_search_aux(TYPE *vkp, size_t m, KeyType key)
{
	if (!m) return 0;
	bit32_t inc, k, i;
	k = __lih_hash_fun(key);
	i = k % m;
	inc = 1 + k % (m - 1);
/*	size_t last = (i + (m - 1) * inc) % m;
	while (i != last && !vkp[i].isempty && !__lih_key_equal(vkp[i].key, key)) {
		if (i + inc >= m) {
			i = i + inc - m;
		} else i += inc;
	}*/ // I think the codes below is more appropriate.
	bit32_t last = i;
	while (!vkp[i].isempty && !__lih_key_equal(vkp[i].key, key)) {
		if (i + inc >= m) {
			i = i + inc - m;
		} else i += inc;
		if (i == last) return 0;
	}
	return vkp + i;
}
template <class KeyType, class TYPE>
inline bool __lih_hash_erase_aux(TYPE *vkp, size_t m, KeyType key)
{
	TYPE *p;
	p = __lih_hash_search_aux(vkp, m, key);
	if (p && !p->isempty) {
		if (p->isdel) return false;
		p->isdel = true;
		return true;
	} else return false;
}
template <class TYPE>
inline void __lih_hash_clear_aux(TYPE *vkp, size_t m)
{
	for (size_t i = 0; i < m; ++i) {
		vkp[i].isempty = true;
		vkp[i].isdel = false;
	}
}
inline size_t __lih_hash_cal_size(size_t m)
{
	bit32_t t;
	t = __lih_HASH_PRIME_SIZE - 1;
	while (__lih_prime_list[t] > m) --t;
	return __lih_prime_list[t+1];
}

template <class TYPE>
class __lih_hash_base_iterator 
{
protected:
	TYPE *cur, *end;
public:
	__lih_hash_base_iterator(TYPE *i, TYPE *e)
	{
		cur = i;
		end = e;
		while (cur != end) {
			if (cur->isempty || cur->isdel) ++cur;
				else return;
		}
	}
	__lih_hash_base_iterator() {};
	~__lih_hash_base_iterator() {};
	inline void inc(void)
	{
		if (cur == end) return;
		cur++;
		while (cur != end) {
			if (cur->isempty || cur->isdel) ++cur;
				else return;
		}
		return;
	}
	inline bool not_end() { return cur != end; };
};

template <class TYPE>
class __lih_hash_base_class
{
protected:
	size_t curr_m, count_n, upper_bound;
	TYPE *val_key_pair;
public:
	typedef __lih_hash_base_iterator<TYPE> iterator;
	__lih_hash_base_class(void)
	{
		val_key_pair = 0;
		curr_m = 0;
		count_n = 0;
		upper_bound = 0;
	}
	~__lih_hash_base_class(void) { ::free(val_key_pair); }
	inline void clear(void)
	{
		__lih_hash_clear_aux(val_key_pair, curr_m);
		count_n = 0;
	}
	inline size_t size(void) { return count_n; };
	inline size_t capacity(void) { return curr_m; };
	inline void free()
	{
		::free(val_key_pair);
		val_key_pair = 0;
		curr_m = 0;
		count_n = 0;
		upper_bound = 0;
	}
};

#endif // HASH_COM_H_
