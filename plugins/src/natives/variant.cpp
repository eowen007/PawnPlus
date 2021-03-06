#include "../natives.h"
#include "../variants.h"

namespace Natives
{
	// native Variant:var_new(AnyTag:value, tag_id=tagof(value));
	static cell AMX_NATIVE_CALL var_new(AMX *amx, cell *params)
	{
		return variants::create(amx, params[1], params[2]);
	}

	// native Variant:var_new_arr(const AnyTag:value[], size=sizeof(value), tag_id=tagof(value));
	static cell AMX_NATIVE_CALL var_new_arr(AMX *amx, cell *params)
	{
		cell *addr;
		amx_GetAddr(amx, params[1], &addr);
		return variants::create(amx, addr, params[2], params[3]);
	}

	// native Variant:var_new_str(const value[]);
	static cell AMX_NATIVE_CALL var_new_str(AMX *amx, cell *params)
	{
		cell *addr;
		amx_GetAddr(amx, params[1], &addr);
		return variants::create(amx, addr);
	}

	// native GlobalVariant:var_to_global(VariantTag:var);
	static cell AMX_NATIVE_CALL var_to_global(AMX *amx, cell *params)
	{
		auto var = reinterpret_cast<dyn_object*>(params[1]);
		if(var == nullptr) return 0;
		variants::pool.move_to_global(var);
		return params[1];
	}

	// native Variant:var_to_local(VariantTag:var);
	static cell AMX_NATIVE_CALL var_to_local(AMX *amx, cell *params)
	{
		auto var = reinterpret_cast<dyn_object*>(params[1]);
		if(var == nullptr) return 0;
		variants::pool.move_to_local(var);
		return params[1];
	}

	// native bool:var_delete(VariantTag:var);
	static cell AMX_NATIVE_CALL var_delete(AMX *amx, cell *params)
	{
		auto var = reinterpret_cast<dyn_object*>(params[1]);
		if(var == nullptr) return 0;
		return variants::pool.free(var);
	}

	// native bool:var_is_valid(VariantTag:var);
	static cell AMX_NATIVE_CALL var_is_valid(AMX *amx, cell *params)
	{
		auto var = reinterpret_cast<dyn_object*>(params[1]);
		return variants::pool.is_valid(var);
	}

	// native var_get(VariantTag:var, offset=0);
	static cell AMX_NATIVE_CALL var_get(AMX *amx, cell *params)
	{
		auto var = reinterpret_cast<dyn_object*>(params[1]);
		if(var == nullptr) return 0;
		cell result;
		if(!var->get_cell(params[2], result)) return 0;
		return result;
	}

	// native var_get_arr(VariantTag:var, AnyTag:value[], size=sizeof(value));
	static cell AMX_NATIVE_CALL var_get_arr(AMX *amx, cell *params)
	{
		auto var = reinterpret_cast<dyn_object*>(params[1]);
		if(var == nullptr) return 0;
		cell *addr;
		amx_GetAddr(amx, params[2], &addr);
		return var->get_array(addr, params[3]);
	}

	// native bool:var_get_checked(VariantTag:var, &AnyTag:value, offset=0, tag_id=tagof(value));
	static cell AMX_NATIVE_CALL var_get_checked(AMX *amx, cell *params)
	{
		auto var = reinterpret_cast<dyn_object*>(params[1]);
		if(var == nullptr) return 0;
		if(!var->check_tag(amx, params[4])) return 0;
		cell *addr;
		amx_GetAddr(amx, params[2], &addr);
		return var->get_cell(params[3], *addr);
	}

	// native var_get_arr_checked(VariantTag:var, AnyTag:value[], size=sizeof(value), tag_id=tagof(value));
	static cell AMX_NATIVE_CALL var_get_arr_checked(AMX *amx, cell *params)
	{
		auto var = reinterpret_cast<dyn_object*>(params[1]);
		if(var == nullptr) return 0;
		if(!var->check_tag(amx, params[4])) return 0;
		cell *addr;
		amx_GetAddr(amx, params[2], &addr);
		return var->get_array(addr, params[3]);
	}

	// native bool:var_set_cell(VariantTag:var, offset, AnyTag:value);
	static cell AMX_NATIVE_CALL var_set_cell(AMX *amx, cell *params)
	{
		auto var = reinterpret_cast<dyn_object*>(params[1]);
		if(var == nullptr) return 0;
		return var->set_cell(params[2], params[3]);
	}

	// native bool:var_set_cell_checked(VariantTag:var, offset, AnyTag:value, tag_id=tagof(value));
	static cell AMX_NATIVE_CALL var_set_cell_checked(AMX *amx, cell *params)
	{
		auto var = reinterpret_cast<dyn_object*>(params[1]);
		if(var == nullptr) return 0;
		if(!var->check_tag(amx, params[4])) return 0;
		return var->set_cell(params[2], params[3]);
	}

	// native var_tagof(VariantTag:var);
	static cell AMX_NATIVE_CALL var_tagof(AMX *amx, cell *params)
	{
		auto var = reinterpret_cast<dyn_object*>(params[1]);
		if(var == nullptr) return 0;
		return var->get_tag(amx);
	}

	// native var_sizeof(VariantTag:var);
	static cell AMX_NATIVE_CALL var_sizeof(AMX *amx, cell *params)
	{
		auto var = reinterpret_cast<dyn_object*>(params[1]);
		if(var == nullptr) return 0;
		return var->get_size();
	}
}

static AMX_NATIVE_INFO native_list[] =
{
	AMX_DECLARE_NATIVE(var_new),
	AMX_DECLARE_NATIVE(var_new_arr),
	AMX_DECLARE_NATIVE(var_new_str),
	AMX_DECLARE_NATIVE(var_to_global),
	AMX_DECLARE_NATIVE(var_to_local),
	AMX_DECLARE_NATIVE(var_delete),
	AMX_DECLARE_NATIVE(var_is_valid),

	AMX_DECLARE_NATIVE(var_get),
	AMX_DECLARE_NATIVE(var_get_arr),
	AMX_DECLARE_NATIVE(var_get_checked),
	AMX_DECLARE_NATIVE(var_get_arr_checked),

	AMX_DECLARE_NATIVE(var_set_cell),
	AMX_DECLARE_NATIVE(var_set_cell_checked),

	AMX_DECLARE_NATIVE(var_tagof),
	AMX_DECLARE_NATIVE(var_sizeof),
};

int RegisterVariantNatives(AMX *amx)
{
	return amx_Register(amx, native_list, sizeof(native_list) / sizeof(*native_list));
}
