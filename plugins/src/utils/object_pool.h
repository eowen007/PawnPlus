#ifndef OBJECT_POOL_H_INCLUDED
#define OBJECT_POOL_H_INCLUDED

#include "sdk/amx/amx.h"
#include <vector>
#include <unordered_map>
#include <memory>

template <class ObjType>
class object_pool
{
public:
	typedef ObjType *object_ptr;
	typedef const ObjType *const_object_ptr;
	typedef decltype(&object_ptr()->operator[](0)) inner_ptr;
	typedef decltype(&const_object_ptr()->operator[](0)) const_inner_ptr;

private:
	typedef std::vector<std::unique_ptr<ObjType>> list_type;
	list_type object_list;
	list_type tmp_object_list;
	std::unordered_map<const_inner_ptr, const_object_ptr> inner_cache;
	
public:
	object_ptr add(bool temp);
	//object_ptr add(object_ptr obj, bool temp);
	object_ptr add(ObjType &&obj, bool temp);
	cell get_address(AMX *amx, const_object_ptr obj) const;
	cell get_inner_address(AMX *amx, const_object_ptr obj) const;
	bool is_null_address(AMX *amx, cell addr) const;
	bool move_to_global(const_object_ptr obj);
	bool move_to_local(const_object_ptr obj);
	void set_cache(const_object_ptr obj);
	object_ptr find_cache(const_inner_ptr ptr);
	bool free(object_ptr obj);
	void clear_tmp();
	bool is_valid(const_object_ptr obj) const;
	object_ptr get(AMX *amx, cell addr);

private:
	typename list_type::const_iterator find_in_list(const_object_ptr obj, const list_type &list) const;
	typename list_type::iterator find_in_list(const_object_ptr obj, list_type &list) const;
};

#endif
