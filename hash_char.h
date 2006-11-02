#ifndef HASH_CHAR_H_
#define HASH_CHAR_H_

#include <malloc.h>
#include <string.h>
#include "hash_com.h"

template <class ValueType>
struct __lih_ValKeyPairChar
{
	ValueType val;
	char *key;
	bool isempty, isdel;
};

template <class ValueType>
class __lih_hash_map_iterator:public
		__lih_hash_base_iterator<__lih_ValKeyPairChar<ValueType> >
{
public:
	__lih_hash_map_iterator() {};
	__lih_hash_map_iterator(__lih_ValKeyPairChar<ValueType> *i,
					__lih_ValKeyPairChar<ValueType> *e):
			__lih_hash_base_iterator<__lih_ValKeyPairChar<ValueType> >(i, e) {};
	inline const char *operator& () { return cur->key; };
	inline const ValueType &operator* () { return cur->val; };
};

template <class ValueType>
class hash_map_char:public __lih_hash_base_class
				<__lih_ValKeyPairChar<ValueType> >
{
	typedef __lih_ValKeyPairChar<ValueType> ValKeyStruct;
	
	inline bool insert_aux(ValKeyStruct *vkp, size_t m,
			const char *key, const ValueType &val) {
		ValKeyStruct *p;
		p = __lih_hash_insert_aux(vkp, m, key);
		p->val = val;
		if (p->isempty) {
			p->key = (char*)malloc(sizeof(char) * (strlen(key) + 1));
			strcpy(p->key, key);
			p->isempty = false;
		} else if (p->isdel) {
			::free(p->key);
			p->key = (char*)malloc(sizeof(char) * (strlen(key) + 1));
			strcpy(p->key, key);
			p->isdel = false;
		} else return true;
		return false;
	}
public:
	typedef __lih_hash_map_iterator<ValueType> iterator;
	hash_map_char(void) {};
	~hash_map_char(void) { clear(); };
	inline void clear(void)
	{
		ValKeyStruct *p;
		for (p = val_key_pair; p < val_key_pair + curr_m; p++) {
			if (!p->isempty) ::free(p->key);
		}
		__lih_hash_base_class<ValKeyStruct>::clear();	
	}
	inline void resize(size_t m)
	{
		size_t new_m, new_upper;
		new_m = __lih_hash_cal_size(m);
		new_upper = int(new_m * __lih_HASH_UPPER + 0.5);
		// if count_n is beyond the new upper boundary, return
		if (count_n >= new_upper) return;

		ValKeyStruct *new_vkp, *p;
		new_vkp = (ValKeyStruct*)malloc(new_m * sizeof(ValKeyStruct));
		__lih_hash_clear_aux(new_vkp, new_m);
		
		for (p = val_key_pair; p < val_key_pair + curr_m; p++) {
			if (!p->isempty) {
				if (!p->isdel) {
					insert_aux(new_vkp, new_m, p->key, p->val);
				}
				::free(p->key);
			}
		}
		::free(val_key_pair);
		val_key_pair = new_vkp;
		curr_m = new_m;
		upper_bound = new_upper;
	}
	inline bool insert(const char *key, const ValueType &val)
	{
		if (count_n >= upper_bound)
			resize(curr_m + 1);
		if (insert_aux(val_key_pair, curr_m, key, val)) return true;
		count_n++;
		return false;
	}
	inline bool find(const char *key, ValueType &value)
	{
		ValKeyStruct *p;
		p = __lih_hash_search_aux(val_key_pair, curr_m, key);
		if (p && !p->isempty && !p->isdel) {
			value = p->val;
			return true;
		}
		return false;
	}
	inline iterator begin()
	{
		return iterator(val_key_pair, val_key_pair + curr_m);
	}	
};

struct __lih_KeyStructChar
{
	char *key;
	bool isempty, isdel;
};

class hash_set_char:public __lih_hash_base_class<__lih_KeyStructChar>
{
	typedef __lih_KeyStructChar KeyStruct;
	
	inline bool insert_aux(KeyStruct *vkp, size_t m,
			const char *key) {
		KeyStruct *p;
		p = __lih_hash_insert_aux(vkp, m, key);
		if (p->isempty) {
			p->key = (char*)malloc(sizeof(char) * (strlen(key) + 1));
			strcpy(p->key, key);
			p->isempty = false;
		} else if (p->isdel) {
			::free(p->key);
			p->key = (char*)malloc(sizeof(char) * (strlen(key) + 1));
			strcpy(p->key, key);
			p->isdel = false;
		} return true;
		return false;
	}
public:
	hash_set_char(void) {};
	~hash_set_char(void) { clear(); };
	inline void clear(void)
	{
		KeyStruct *p;
		for (p = val_key_pair; p < val_key_pair + curr_m; p++) {
			if (!p->isempty) ::free(p->key);
		}
		__lih_hash_base_class<KeyStruct>::clear();	
	}
	inline void resize(size_t m)
	{
		size_t new_m, new_upper;
		new_m = __lih_hash_cal_size(m);
		new_upper = int(new_m * __lih_HASH_UPPER + 0.5);
		// if count_n is beyond the new upper boundary, return
		if (count_n >= new_upper) return;

		KeyStruct *new_vkp, *p;
		new_vkp = (KeyStruct*)malloc(new_m * sizeof(KeyStruct));
		__lih_hash_clear_aux(new_vkp, new_m);
		
		for (p = val_key_pair; p < val_key_pair + curr_m; p++) {
			if (!p->isempty) {
				if (!p->isdel) {
					insert_aux(new_vkp, new_m, p->key);
				}
				::free(p->key);
			}
		}
		::free(val_key_pair);
		val_key_pair = new_vkp;
		curr_m = new_m;
		upper_bound = new_upper;
	}
	inline bool insert(const char *key)
	{
		if (count_n >= upper_bound)
			resize(curr_m + 1);
		if (insert_aux(val_key_pair, curr_m, key)) return true;
		count_n++;
		return false;
	}
	inline bool find(const char *key)
	{
		KeyStruct *p;
		p = __lih_hash_search_aux(val_key_pair, curr_m, key);
		if (p && !p->isempty && !p->isdel) {
			return true;
		}
		return false;
	}
};

#endif // HASH_CHAR_H_
