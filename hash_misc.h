#ifndef HASH_MISC_H_
#define HASH_MISC_H_

#include "hash_com.h"

template <class ValueType, class KeyType>
struct __lih_ValKeyPairInt
{
	ValueType val;
	KeyType key;
	bool isempty, isdel;
};

template <class ValueType, class KeyType = bit32_t>
class hash_map_misc:public __lih_hash_base_class<__lih_ValKeyPairInt<ValueType, KeyType> >
{
	typedef __lih_ValKeyPairInt<ValueType, KeyType> ValKeyStruct;
	
	inline bool insert_aux(ValKeyStruct *vkp, size_t m, KeyType key, const ValueType &val) {
		ValKeyStruct *p;
		p = __lih_hash_insert_aux(vkp, m, key);
		p->val = val;
		if (p->isempty) {
			p->key = key;
			p->isempty = false;
		} else if (p->isdel) {
			p->key = key;
			p->isdel = false;
		} else return true;
		return false;
	}
public:
	hash_map_misc(void) {};
	~hash_map_misc(void) {};
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
			if (!p->isempty && !p->isdel)
				insert_aux(new_vkp, new_m, p->key, p->val);
		}
		::free(val_key_pair);
		val_key_pair = new_vkp;
		curr_m = new_m;
		upper_bound = new_upper;
	}
	inline bool insert(KeyType key, const ValueType &val)
	{
		if (count_n >= upper_bound)
			resize(curr_m + 1);
		if (insert_aux(val_key_pair, curr_m, key, val)) return true;
		count_n++;
		return false;
	}
	inline bool find(KeyType key, ValueType &value)
	{
		ValKeyStruct *p;
		p = __lih_hash_search_aux(val_key_pair, curr_m, key);
		if (p && !p->isempty && !p->isdel) {
			value = p->val;
			return true;
		}
		return false;
	}
	inline bool erase(KeyType key)
	{
		if (__lih_hash_erase_aux(val_key_pair, curr_m, key)) {
			--count_n;
			return true;
		}
		return false;
	}
};

template <class KeyType>
struct __lih_KeyStructInt
{
	KeyType key;
	bool isempty, isdel;
};

template <class KeyType>
class __lih_hash_set_int_iterator:public 
		__lih_hash_base_iterator<__lih_KeyStructInt<KeyType> >
{
public:
	__lih_hash_set_int_iterator() {};
	__lih_hash_set_int_iterator(__lih_KeyStructInt<KeyType> *i, __lih_KeyStructInt<KeyType> *e):
			__lih_hash_base_iterator<__lih_KeyStructInt<KeyType> >(i, e) {};
	inline KeyType operator* () { return cur->key; }
};

template <class KeyType>
class hash_set_misc : public __lih_hash_base_class<__lih_KeyStructInt<KeyType> >
{
	typedef __lih_KeyStructInt<KeyType> KeyStruct;
	
	inline bool insert_aux(KeyStruct *vkp, size_t m, KeyType key)
	{
		KeyStruct *p;
		p = __lih_hash_insert_aux(vkp, m, key);
		if (p->isempty) {
			p->key = key;
			p->isempty = false;
		} else if (p->isdel) {
			p->key = key;
			p->isdel = false;
		} else return true;
		return false;
	}
public:
	typedef __lih_hash_set_int_iterator<KeyType> iterator;
	hash_set_misc(void) {};
	~hash_set_misc(void) {};
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
			if (!p->isempty && !p->isdel)
				insert_aux(new_vkp, new_m, p->key);
		}
		::free(val_key_pair);
		val_key_pair = new_vkp;
		curr_m = new_m;
		upper_bound = new_upper;
	}
	inline bool insert(KeyType key)
	{
		if (count_n >= upper_bound)
			resize(curr_m + 1);
		if (insert_aux(val_key_pair, curr_m, key)) return true;
		count_n++;
		return false;
	}
	inline bool find(KeyType key)
	{
		KeyStruct *p;
		p = __lih_hash_search_aux(val_key_pair, curr_m, key);
		if (p && !p->isempty && !p->isdel) return true;
		return false;
	}
	inline bool erase(KeyType key)
	{
		if (__lih_hash_erase_aux(val_key_pair, curr_m, key)) {
			--count_n;
			return true;
		}
		return false;
	}
	inline iterator begin()
	{
		return iterator(val_key_pair, val_key_pair + curr_m);
	}
};
#endif // HASH_INT_H_
