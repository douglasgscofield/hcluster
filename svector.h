#ifndef LIH_SVECTOR_H_
#define LIH_SVECTOR_H_

#include <malloc.h>
#include "qsort.h"

#ifndef LIH_BLOCK_SIZE
#define LIH_BLOCK_SIZE 256
#endif

template <class TYPE>
class svector
{
	size_t num, max;
	TYPE *array;
public:
	typedef TYPE* iterator;
	svector() { construct(); };
	~svector() { destroy(); };
	inline void destroy() { free(array); };
	inline void construct() { num = 0; max = 0; array = 0; }; // for manually construction
	inline void push_back(const TYPE &a)
	{
		if (num == max) {
			max += LIH_BLOCK_SIZE;
			array = (TYPE*)realloc(array, sizeof(TYPE) * max);
		}
		array[num++] = a;
	};
	inline TYPE *push_back()
	{
		if (num == max) {
			max += LIH_BLOCK_SIZE;
			array = (TYPE*)realloc(array, sizeof(TYPE) * max);
		}
		++num;
		return array + num - 1;
	};
	inline TYPE *push_null() { return push_back(); }
	inline void rewind() { num = 0; };
	inline void clear() { rewind(); };
	inline void sort() { quick_sort(array, num); };
	inline void resize(size_t new_size)
	{
		if (new_size < max) {
			num = new_size;
		} else {
			max = new_size;
			num = new_size;
			array = (TYPE*)realloc(array, sizeof(TYPE) * max);
		}
	}
	inline size_t size() { return num; };
	inline iterator begin() { return array; };
	inline iterator end() { return array + num; };
	inline TYPE &operator[](size_t i) { return array[i]; };
};

template <class TYPE>
struct same_pair
{
	TYPE x, y;
	same_pair() {};
	same_pair(const TYPE &xx, const TYPE &yy) { x = xx; y = yy; };
};

#endif // LIH_SVECTOR_H_
