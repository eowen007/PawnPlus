#include "dyn_object.h"
#include "../fixes/linux.h"

dyn_object::dyn_object() : is_array(true), array_size(0), array_value(nullptr)
{

}

dyn_object::dyn_object(AMX *amx, cell value, cell tag_id) : is_array(false), cell_value(value), tag_name(find_tag(amx, tag_id))
{

}

dyn_object::dyn_object(AMX *amx, cell *arr, cell size, cell tag_id) : is_array(true), array_size(size), tag_name(find_tag(amx, tag_id))
{
	array_value = std::make_unique<cell[]>(size);
	std::memcpy(array_value.get(), arr, size * sizeof(cell));
}

dyn_object::dyn_object(AMX *amx, cell *str) : is_array(true), tag_name("char")
{
	if(str == nullptr || !str[0])
	{
		array_size = 1;
		array_value = std::make_unique<cell[]>(1);
		array_value[0] = 0;
		return;
	}
	int len;
	amx_StrLen(str, &len);
	size_t size;
	if(str[0] & 0xFF000000)
	{
		size = 1 + ((len - 1) / sizeof(cell));
	}else{
		size = len;
	}
	array_size = size + 1;
	array_value = std::make_unique<cell[]>(array_size);
	std::memcpy(array_value.get(), str, size * sizeof(cell));
	array_value[size] = 0;
}

dyn_object::dyn_object(const dyn_object &obj) : is_array(obj.is_array), tag_name(obj.tag_name)
{
	if(is_array)
	{
		array_size = obj.array_size;
		array_value = std::make_unique<cell[]>(array_size);
		std::memcpy(array_value.get(), obj.array_value.get(), array_size * sizeof(cell));
	}else{
		cell_value = obj.cell_value;
	}
}

dyn_object::dyn_object(dyn_object &&obj) : is_array(obj.is_array), tag_name(std::move(obj.tag_name))
{
	if(is_array)
	{
		array_size = obj.array_size;
		array_value = std::move(obj.array_value);
	}else{
		cell_value = obj.cell_value;
	}
}

std::string dyn_object::find_tag(AMX *amx, cell tag_id) const
{
	tag_id &= 0x7FFFFFFF;

	int len;
	amx_NameLength(amx, &len);
	char *tagname = static_cast<char*>(alloca(len+1));

	int num;
	amx_NumTags(amx, &num);
	for(int i = 0; i < num; i++)
	{
		cell tag_id2;
		if(!amx_GetTag(amx, i, tagname, &tag_id2))
		{
			if(tag_id == tag_id2)
			{
				return std::string(tagname);
			}
		}
	}
	return std::string();
}

bool dyn_object::check_tag(AMX *amx, cell tag_id) const
{
	tag_id &= 0x7FFFFFFF;

	if(tag_id == 0) //weak tags
	{
		if(tag_name.empty()) return true;
		char c = tag_name[0];
		if(c < 'A' || c > 'Z') return true;
	}

	int len;
	amx_NameLength(amx, &len);
	char *tagname = static_cast<char*>(alloca(len+1));

	int num;
	amx_NumTags(amx, &num);
	for(int i = 0; i < num; i++)
	{
		cell tag_id2;
		if(!amx_GetTag(amx, i, tagname, &tag_id2) && tag_name == tagname)
		{
			if(
				tag_id == tag_id2 ||
				(tag_id == 0 && (tag_id2 & 0x40000000) == 0) ||
				(tag_name == "GlobalString" && find_tag(amx, tag_id) == "String")
			)
			{
				return true;
			}
		}
	}
	return false;
}

bool dyn_object::get_cell(cell index, cell &value) const
{
	if(index < 0) return false;
	if(is_array)
	{
		if(index >= array_size) return false;
		value = array_value[index];
		return true;
	} else {
		if(index > 0) return false;
		value = cell_value;
		return true;
	}
}

cell dyn_object::get_array(cell *arr, cell maxsize) const
{
	if(is_array)
	{
		if(maxsize > array_size) maxsize = array_size;
		std::memcpy(arr, array_value.get(), maxsize * sizeof(cell));
		return maxsize;
	}else{
		*arr = cell_value;
		return 1;
	}
}

bool dyn_object::set_cell(cell index, cell value)
{
	if(index < 0) return false;
	if(is_array)
	{
		if(index >= array_size) return false;
		array_value[index] = value;
		return true;
	}
	return false;
}

cell dyn_object::get_tag(AMX *amx) const
{
	if(tag_name.empty()) return 0x80000000;

	int len;
	amx_NameLength(amx, &len);
	char *tagname = static_cast<char*>(alloca(len+1));

	int num;
	amx_NumTags(amx, &num);
	for(int i = 0; i < num; i++)
	{
		cell tag_id;
		if(!amx_GetTag(amx, i, tagname, &tag_id))
		{
			if(tag_name == tagname)
			{
				return tag_id | 0x80000000;
			}
		}
	}
	return 0;
}

cell dyn_object::store(AMX *amx) const
{
	if(is_array)
	{
		cell amx_addr, *addr;
		amx_Allot(amx, array_size, &amx_addr, &addr);
		std::memcpy(addr, array_value.get(), array_size * sizeof(cell));
		return amx_addr;
	}else{
		return cell_value;
	}
}

void dyn_object::load(AMX *amx, cell amx_addr)
{
	if(is_array)
	{
		cell *addr;
		amx_GetAddr(amx, amx_addr, &addr);
		std::memcpy(array_value.get(), addr, array_size * sizeof(cell));
	}
}

cell dyn_object::get_size() const
{
	if(is_array)
	{
		return array_size;
	}else{
		return 1;
	}
}

char dyn_object::get_specifier() const
{
	if(is_array)
	{
		if(tag_name == "char") return 's';
		if(array_size > 1) return 'a';
	}
	if(tag_name == "Float") return 'f';
	if(tag_name == "String" || tag_name == "GlobalString") return 'S';
	return 'i';
}

template <class T>
inline void hash_combine(size_t& seed, const T& v)
{
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

size_t dyn_object::get_hash() const
{
	std::hash<cell> hasher;

	size_t hash = 0;
	if(is_array)
	{
		for(cell i = 0; i < array_size; i++)
		{
			hash_combine(hash, array_value[i]);
		}
	}else{
		hash = hasher(cell_value);
	}

	hash_combine(hash, tag_name);
	return hash;
}

bool dyn_object::empty() const
{
	return is_array ? array_size == 0 : false;
}

cell &dyn_object::operator[](cell index)
{
	if(is_array)
	{
		return array_value[index];
	} else {
		return (&cell_value)[index];
	}
}

const cell &dyn_object::operator[](cell index) const
{
	if(is_array)
	{
		return array_value[index];
	} else {
		return (&cell_value)[index];
	}
}

bool operator==(const dyn_object &a, const dyn_object &b)
{
	if(a.is_array != b.is_array || a.tag_name != b.tag_name) return false;
	if(a.is_array)
	{
		if(a.array_size != b.array_size) return false;
		return !std::memcmp(a.array_value.get(), b.array_value.get(), a.array_size * sizeof(cell));
	}else{
		return a.cell_value == b.cell_value;
	}
}

dyn_object &dyn_object::operator=(const dyn_object &obj)
{
	if(this == &obj) return *this;
	is_array = obj.is_array;
	tag_name = obj.tag_name;
	if(is_array)
	{
		array_size = obj.array_size;
		array_value = std::make_unique<cell[]>(array_size);
		std::memcpy(array_value.get(), obj.array_value.get(), array_size * sizeof(cell));
	}else{
		cell_value = obj.cell_value;
	}
	return *this;
}

dyn_object &dyn_object::operator=(dyn_object &&obj)
{
	if(this == &obj) return *this;
	is_array = obj.is_array;
	tag_name = std::move(obj.tag_name);
	if(is_array)
	{
		array_size = obj.array_size;
		array_value = std::move(obj.array_value);
	}else{
		cell_value = obj.cell_value;
	}
	return *this;
}
