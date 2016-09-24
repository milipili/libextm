#include "extm/object.h"
#include <iso646.h>
#include <string>
#include <unordered_map>
#include <vector>


using NyObjMap   = std::unordered_map<std::string, nyobj_refptr_t>;
using NyObjArray = std::vector<nyobj_refptr_t>;


enum class nyobj_internaltype_t: uint32_t
{
	nyit_dict,
	nyit_array,
	nyit_shortstring,
	nyit_string, // uintptr_t + char + zero-terminated
	nyit_bool,
	nyit_i32,
	nyit_i64,
	nyit_u32,
	nyit_u64,
	nyit_f32,
	nyit_f64,
	nyit_color,
};


union nyobj_value_t
{
	uintptr_t raw;
	int32_t i32;
	int64_t i64;
	uint32_t u32;
	uint64_t u64;
	float f32;
	double f64;
	bool onoff;
	nycolor_t rgba;
	NyObjMap* dict;
	NyObjArray* array;
	char shortstr[8];
	uint32_t* string;
};


struct nyobj_t
{
	uint32_t refcount;
	nyobj_internaltype_t type;
	nyobj_value_t value;
};


namespace // anonymous
{


	struct nyobj_refptr_t final
	{
		nyobj_refptr_t() = default;
		nyobj_refptr_t(nyobj_t* object)
			: object(object)
		{
			if (object)
				++(object->refcount);
		}

		nyobj_refptr_t(const nyobj_refptr_t& rhs)
			: object(rhs.object)
		{
			if (object)
				++(object->refcount);
		}

		~nyobj_refptr_t()
		{
			inline_nyobj_unref(object);
		}

		nyobj_refptr_t& operator = (const nyobj_refptr_t& rhs)
		{
			inline_nyobj_unref(object);
			object = rhs.object;
			if (object)
				++(object->refcount);
			return *this;
		}

		nyobj_refptr_t& operator = (nyobj_t* rhs)
		{
			inline_nyobj_unref(object);
			object = rhs;
			if (object)
				++(object->refcount);
			return *this;
		}

		nyobj_t* object = nullptr;

	}; // class nyobj_refptr_t



	template<class T>
	inline T convert_to_number(const nyobj_t* const obj)
	{
		if (obj)
		{
			switch (obj->type)
			{
				case nyit_i32:  return static_cast<T>(obj->value.i32);
				case nyit_i64:  return static_cast<T>(obj->value.i64);
				case nyit_u32:  return static_cast<T>(obj->value.u32);
				case nyit_u64:  return static_cast<T>(obj->value.u64);
				case nyit_f32:  return static_cast<T>(obj->value.f32);
				case nyit_f64:  return static_cast<T>(obj->value.f64);
				case nyit_bool: return obj->value.onoff ? 1 : 0;
				case nyit_color:return static_cast<T>(obj->value.rgba);
				default: break;
			}
		}
		return T();
	}


	inline bool is_length_valid(size_t length)
	{
		return sizeof(size_t) == sizeof(uint32_t) || length < 2 * 1024 * 1024 * 1024;
	}


	inline uint32_t get_string_length(const nyobj_t* obj)
	{
		return *(obj->value.string);
	}


	inline size_t get_string_capacity(uint32_t length)
	{
		return sizeof(uint32_t)
			* ((length + 1/*zero*/ + sizeof(uint32_t)/*strsize*/ + sizeof(uint32_t)/*round*/) / sizeof(uint32_t));
	}


	inline nybool_t to_nybool(bool value)
	{
		return value ? nytrue : nyfalse;
	}


	inline nyobj_t* nyobj_create_object(nyobj_internaltype_t type, nyobj_value_t value)
	{
		nyobj_t* const obj = (nyobj_t*) malloc(sizeof(nyobj_t));
		if (obj)
		{
			obj->refcount = 1;
			obj->type = type;
			obj->value = value;
		}
		return obj;
	}


	inline void nyobj_delete_content(nyobj_t* const obj)
	{
		switch (obj->type)
		{
			case nyit_string: free(obj->value.string); break;
			case nyit_dict:   delete obj->value.dict; break;
			case nyit_array:  delete obj->value.array; break;
			default: break;
		}
	}


	inline void delete_object(nyobj_t* object)
	{
		nyobj_delete_content(object);
		free(obj);
	}

	inline void inline_nyobj_unref(nyobj_t* object)
	{
		if (object and 0 == --object->refcount)
			nyobj_destroy_nocheck(object);
	}


	bool nyobj_copy_content(nyobj_t* dest, const nyobj_t* src)
	{
		nyobj_internaltype_t type = src->type;
		dest->type = type;

		switch (type)
		{
			default:
			{
				dest->value.raw = src->value.raw;
				break;
			}
			case nyit_string:
			{
				uint32_t length = get_string_length(src);
				uint32_t* p = (uint32_t*) malloc(get_string_capacity(length));
				if (!p)
					return false;
				dest->value.string = p;
				memcpy(p, src->value.string, length + sizeof(uint32_t) + 1);
				break;
			}
			case nyit_dict:
			{
				auto* dict = rhs->value.dict;
				if (dict)
				{
					assert(not dict->empty());
					std::unique_ptr<NyObjMap> newdict {new (std::nothrow) NyObjMap};
					if (!newdict)
						return false;
					try
					{
						for (auto& pair: *dict)
						{
							auto* clone = nyobj_clone(pair.second.object);
							if (!clone)
								return false;
							newdict->insert(std::make_pair<std::string, nyobj_refptr_t>(pair.first, clone));
							--(clone->refcount);
						}
						dest->value.dict = newdict.release();
					}
					catch (...) { return false; }
				}
				else
					dest->value.dict = nullptr;
				break;
			}
			case nyit_array:
			{
				auto* array = rhs->value.array;
				if (array)
				{
					assert(not array->empty());
					std::unique_ptr<NyObjArray> newarray {new (std::nothrow) NyObjArray};
					if (!newarray)
						return false;
					try
					{
						newarray->reserve(array->size());
						for (auto& element: *array)
						{
							auto* clone = nyobj_clone(element.object);
							if (!clone)
								return false;
							newarray->emplace_back(clone);
							--(clone->refcount);
						}
						dest->value.dict = newarray.release();
					}
					catch (...) { return false; }
				}
				else
					dest->value.dict = nullptr;
				break;
			}
		}
		return true;
	}


	nyobj_t* nyobj_create_from_cstring(const char* cstr, uint32_t size)
	{
		auto* object = (nyobj_t*) malloc(sizeof(nyobj_t));
		if (!object)
			return nullptr;

		object->refcount = 1;
		if (size < sizeof(uint64_t) - 1 /*string size*/) // shortstring optimization
		{
			object->type = nyit_shortstring;
			char* p = object->value.ncstr;
			*p = static_cast<uint8_t>(size);
			++p;
			for (uint32_t i = 0; i != size; ++i, ++p, ++cstr)
				*p = *cstr;
			*p = '\0';
		}
		else
		{
			uint32_t* p = (uint32_t*) malloc(get_string_capacity(size));
			if (!p)
			{
				free(object);
				return nullptr;
			}
			*p = size;
			p += 1;
			memcpy(p, cstr, size);
			reinterpret_cast<char*>(p)[size] = 0;
			object->type = nyit_string;
			object->value.ptr = p;
		}
		return object;
	}


	template<class T, nyobj_t* (*Factory)(T)>
	inline nyobj_t* nyobj_build_array(const T* args, uint32_t count)
	{
		if (count)
		{
			NyObjArray* array = new (std::nothrow) NyObjArray;
			if (array)
			{
				nyobj_t* obj = nyobj_create_object(nyit_array, array);
				if (obj)
				{
					try
					{
						array->reserve(count);
						for (uint32_t i = 0; i != count; ++i, ++args)
						{
							nyobj_t* obj = Factory(*args);
							array->emplace_back(obj);
							if (obj)
								--(obj->refcount);
						}
						return obj;
					}
					catch (...)
					{
						nyobj_destroy_nocheck(obj);
					}
				}
				else
					delete array;
			}
		}
		return nyobj_create_object(nyit_array, nullptr);
	}


	inline NyObjArray* ensure_array(nyobj_t* obj)
	{
		if (obj)
		{
			if (obj->type != nyit_array)
				nyobj_mutate_to_array(obj);

			auto* array = obj->value.array;
			if (!array)
			{
				array = new (std::nothrow) NyObjArray;
				if (array)
					obj->value.array = array;
			}
			return array;
		}
		return nullptr;
	}


	inline NyObjDict* ensure_dict(nyobj_t* obj)
	{
		if (obj)
		{
			if (obj->type != nyit_dict)
				nyobj_mutate_to_dict(obj);

			auto* dict = obj->value.dict;
			if (!dict)
			{
				dict = new (std::nothrow) NyObjDict;
				if (dict)
					obj->value.dict = dict;
			}
			return dict;
		}
		return nullptr;
	}

} // anonymous namespace




extern "C" void nyobj_ref(nyobj_t* obj)
{
	if (obj)
		++(obj->refcount);
}


extern "C" nyobj_type_t nyobj_type(const nyobj_t* obj)
{
	if (obj)
	{
		switch (obj->type)
		{
			case nyit_string:
			case nyit_shortstring: return nyo_string;
			case nyit_dict:        return nyo_dict;
			case nyit_array:       return nyo_array;
			case nyit_bool:        return nyo_bool;
			case nyit_i32:         return nyo_i32;
			case nyit_i64:         return nyo_i64;
			case nyit_u32:         return nyo_u32;
			case nyit_u64:         return nyo_u64;
			case nyit_f32:         return nyo_f32;
			case nyit_f64:         return nyo_f64;
			case nyit_color:       return nyo_color;
		}
	}
	return nyo_null;
}


extern "C" void nyobj_destroy_nocheck(nyobj_t* obj)
{
	assert(obj != nullptr);
	delete_object(object);
}


extern "C" void nyobj_unref(nyobj_t* obj)
{
	if (obj)
	{
		assert(obj->refcount > 0);
		if (!--(obj->refcount))
			nyobj_destroy_nocheck(object);
	}
}


extern "C" nyobj_t* nyobj_clone(const nyobj_t* rhs)
{
	if (!rhs)
		return nullptr;

	nyobj_t* newobj = (nyobj_t*) malloc(sizeof(nyobj_t));
	if (!newobj)
		return nullptr;

	newobj->refcount = 1;
	if (nyobj_copy_content(newobj, rhs))
		return newobj;
	free(newobj);
	return nullptr;
}


extern "C" void nyobj_copy(nyobj_t* dest, const nyobj_t* src)
{
	if (dest)
	{
		nyobj_delete_content(dest);
		if (!src or not nyobj_copy_content(dest, src))
		{
			dest->type = nyit_u32;
			dest->value.u32 = 0;
		}
	}
}


extern "C" void nyobj_swap(nyobj_t* a, nyobj_t* b)
{
	if (a and b)
	{
		std::swap(a->value, b->value);
		std::swap(a->type, b->type);
	}
}


extern "C" nyobj_t* nyobj_null()
{
	return nullptr;
}


extern "C" nyobj_t* nyobj_nan()
{
	return nyobj_create_object(nyit_f32, std::numeric_limits<float>::quiet_NaN());
}


extern "C" nyobj_t* nyobj_color(nycolor_t color)
{
	return nyobj_create_object(nyit_color, color);
}


extern "C" nyobj_t* nyobj_u64(uint64_t value)
{
	return nyobj_create_object(nyit_u64, value);
}


extern "C" nyobj_t* nyobj_i64(int64_t value)
{
	return nyobj_create_object(nyit_i64, value);
}


extern "C" nyobj_t* nyobj_u32(uint32_t value)
{
	return nyobj_create_object(nyit_u32, value);
}


extern "C" nyobj_t* nyobj_i32(int32_t value)
{
	return nyobj_create_object(nyit_i32, value);
}


extern "C" nyobj_t* nyobj_f32(float value)
{
	return nyobj_create_object(nyit_f32, value);
}


extern "C" nyobj_t* nyobj_f64(double value)
{
	return nyobj_create_object(nyit_f64, value);
}


extern "C" nyobj_t* nyobj_dict()
{
	return nyobj_create_object(nyit_dict, nullptr);
}


extern "C" nyobj_t* nyobj_bool(int value)
{
	return nyobj_create_object(nyit_bool, (value != 0));
}


extern "C" nyobj_t* nyobj_bool(nybool_t value)
{
	return nyobj_create_object(nyit_bool, (value != nyfalse));
}


extern "C" nyobj_t* nyobj_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	return nyobj_create_object(nyit_color, {r, g, b, a});
}


extern "C" nyobj_t* nyobj_rgb(uint8_t r, uint8_t g, uint8_t b)
{
	return nyobj_create_object(nyit_color, {r, g, b, 255u});
}


extern "C" nyobj_t* nyobj_str(const char* cstr)
{
	size_t length = (cstr) ? strlen(cstr) : 0;
	return is_length_valid(length) ? nyobj_create_from_cstring(cstr, static_cast<uint32_t>(length)) : nullptr;
}


extern "C" nyobj_t* nyobj_str_ex(const char* cstr, uint32_t size)
{
	return nyobj_create_from_cstring(cstr, size);
}


extern "C" nyobj_t* nyobj_char(char c)
{
	nyobj_value_t value;
	value.ncstr[0] = '\1'; // size in bytes
	value.ncstr[1] = c;
	value.ncstr[2] = '\0';
	return nyobj_create_object(nyit_shortstring, value);
}




extern "C" nyobj_t* nyobj_array()
{
	return nyobj_create_object(nyit_dict, nullptr);
}


extern "C" nyobj_t* nyobj_array_of_obj(const nyobj_t** args, uint32_t count)
{
	if (count)
	{
		assert(args != nullptr);
		NyObjArray* array = new (std::nothrow) NyObjArray;
		if (array)
		{
			nyobj_t* obj = nyobj_create_object(nyit_array, array);
			if (obj)
			{
				try
				{
					array->reserve(count);
					for (uint32_t i = 0; i != count; ++i, ++args)
						array->emplace_back(*args);
					return obj;
				}
				catch (...)
				{
					nyobj_destroy_nocheck(obj);
				}
			}
			else
				delete array;
		}
	}
	return nyobj_create_object(nyit_array, nullptr);
}


extern "C" nyobj_t* nyobj_array_of_clone_obj(const nyobj_t** args, uint32_t count)
{
	if (count)
	{
		NyObjArray* array = new (std::nothrow) NyObjArray;
		if (array)
		{
			nyobj_t* obj = nyobj_create_object(nyit_array, array);
			if (obj)
			{
				try
				{
					array->reserve(count);
					for (uint32_t i = 0; i != count; ++i, ++args)
					{
						nyobj_t* clone = nyobj_clone(*args);
						array->emplace_back(clone);
						if (clone)
							--(clone->refcount);
					}
					return obj;
				}
				catch (...)
				{
					nyobj_destroy_nocheck(obj);
				}
			}
			else
				delete array;
		}
	}
	return nyobj_create_object(nyit_array, nullptr);
}


extern "C" nyobj_t* nyobj_array_of_strings(const char** args, uint32_t count)
{
	return nyobj_build_array<const char*, nyobj_str>(args, count);
}


extern "C" nyobj_t* nyobj_array_of_i32(const int32_t* args, uint32_t count)
{
	return nyobj_build_array<int32_t, nyobj_i32>(args, count);
}


extern "C" nyobj_t* nyobj_array_of_u32(const uint32_t* args, uint32_t count)
{
	return nyobj_build_array<uint32_t, nyobj_u32>(args, count);
}


extern "C" nyobj_t* nyobj_array_of_i64(const int64_t* args, uint32_t count)
{
	return nyobj_build_array<int64_t, nyobj_i64>(args, count);
}


extern "C" nyobj_t* nyobj_array_of_u64(const uint64_t* args, uint32_t count)
{
	return nyobj_build_array<uint64_t, nyobj_u64>(args, count);
}


extern "C" nyobj_t* nyobj_array_of_f32(const float* args, uint32_t count)
{
	return nyobj_build_array<float, nyobj_f32>(args, count);
}


extern "C" nyobj_t* nyobj_array_of_f64(const double* args, uint32_t count)
{
	return nyobj_build_array<double, nyobj_f64>(args, count);
}




extern "C" nybool_t nyobj_is_empty(const nyobj_t* obj)
{
	if (!obj)
		return nytrue;
	switch (obj->type)
	{
		case nyit_shortstring:
			return (obj->value.ncstr[0] == 0); // nyit_string > few bytes for sure
		case nyit_dict:
		case nyit_array:
			return (obj->value.ptr == nullptr);
		default:
			break;
	}
	return nyfalse;
}


extern "C" uint32_t nyobj_size(const nyobj_t* obj)
{
	if (obj)
	{
		switch (obj->type)
		{
			case nyit_shortstring:
			{
				return obj->value.ncstr[0];
			}
			case nyit_string:
			{
				return get_string_length(obj);
			}
			case nyit_dict:
			{
				if (obj->value.dict)
					return obj->value.dict->size();
				break;
			}
			case nyit_array:
			{
				if (obj->value.array)
					return obj->value.array->size();
				break;
			}
			default:
				break;
		}
	}
	return 0;
}


extern "C" void nyobj_mutate_to_array(nyobj_t* obj)
{
	if (!obj or obj->type == nyit_array)
		return;

	NyObjArray* newarray = nullptr;
	switch (obj->type)
	{
		case nyit_dict:
		{
			auto* dict = obj->value.dict;
			if (dict)
			{
				newarray = new (std::nothrow) NyObjArray;
				if (newarray)
				{
					try
					{
						newarray->reserve(dict->size());
						for (auto& pair: *dict)
							newarray->emplace_back(pair.second);
					}
					catch (...)
					{
						delete newarray;
						newarray = nullptr;
						delete dict;
					}
				}
				else
					delete dict;
			}
			break;
		}
		default:
		{
			newarray = new (std::nothrow) NyObjArray;
			if (newarray)
			{
				auto* firstitem = (nyobj_t*) malloc(sizeof(nyobj_t));
				if (firstitem)
				{
					firstitem->refcount = 0;
					firstitem->type = obj->type;
					firstitem->value = obj->value;
					try
					{
						newarray->emplace_back(firstitem);
					}
					catch (...)
					{
						free(firstitem);
						delete newarray;
						newarray = nullptr;
					}
				}
				else
				{
					delete newarray;
					newarray = nullptr;
				}
			}
			break;
		}
	}
	obj->type = nyit_array;
	obj->value.ptr = newarray;
}


extern "C" void nyobj_mutate_to_dict(nyobj_t* obj)
{
	if (!obj or obj->type == nyit_dict)
		return;

	NyObjArray* newdict = nullptr;
	switch (obj->type)
	{
		case nyit_array:
		{
			auto* array = obj->value.array;
			if (array)
			{
				newdict = new (std::nothrow) NyObjMap;
				if (newdict)
				{
					uint32_t i = 0;
					try
					{
						for (auto& element: *array)
							newdict->insert(std::make_pair(std::to_string(i++), element));
					}
					catch (...)
					{
						delete newdict;
						newdict = nullptr;
						delete array;
					}
				}
				else
					delete array;
			}
			break;
		}
		default:
		{
			newdict = new (std::nothrow) NyObjMap;
			if (newdict)
			{
				auto* firstitem = (nyobj_t*) malloc(sizeof(nyobj_t));
				if (firstitem)
				{
					firstitem->refcount = 0;
					firstitem->type = obj->type;
					firstitem->value = obj->value;
					newdict->insert(std::make_pair(std::string{"_"}, firstitem));
				}
				else
				{
					delete newdict;
					newdict = nullptr;
				}
			}
			break;
		}
	}
	obj->type = nyit_array;
	obj->value.ptr = newdict;
}


extern "C" void nyobj_clear(nyobj_t* obj)
{
	if (obj and obj->value.raw != 0)
	{
		switch (obj->type)
		{
			case nyit_string:
			{
				free(obj->value.string);
				obj->type = nyit_shortstring;
				break;
			}
			case nyit_dict:
			{
				delete obj->value.dict;
				break;
			}
			case nyit_array:
			{
				delete obj->value.array;
				break;
			}
			default:
				break;
		}
		obj->value.raw = 0;
	}
}


extern "C" nybool_t nyobj_append(nyobj_t* obj, nyobj_t* item)
{
	auto* array = ensure_array(obj);
	if (array)
	{
		try
		{
			array->emplace_back(item);
			return nytrue;
		}
		catch (...) {}
	}
	return nyfalse;
}


extern "C" nybool_t nyobj_set_index(nyobj_t* obj, uint32_t index, nyobj_t* item)
{
	auto* array = ensure_array(obj);
	if (array)
	{
		try
		{
			if (not (index < array->size()))
				array->resize(index + 1);
			(*array)[index] = item;
			return nytrue;
		}
		catch (...) {}
	}
	return nyfalse;
}


extern "C" nyobj_t* nyobj_get_index(nyobj_t* obj, uint32_t index)
{
	if (obj and obj->type == nyit_array)
	{
		auto* array = obj->value.array;
		if (array and index < array->size())
			return (*array)[index].object;
	}
	return nullptr;
}


extern "C" nyobj_t* nyobj_get_ex(nyobj_t* obj, const char* key, uint32_t length)
{
	if (obj and key and length < 65536 and obj->type == nyit_dict)
	{
		auto* dict = obj->value.dict;
		if (dict)
		{
			auto it = dict->find(std::string{key, length});
			if (it != dict->end())
				return it->second.object;
		}
	}
	return nullptr;
}


extern "C" nybool_t nyobj_set_ex(nyobj_t* obj, const char* key, uint32_t length, nyobj_t* item)
{
	if (key and length < 65536)
	{
		auto* dict = ensure_dict(obj);
		if (dict)
		{
			try
			{
				(*dict)[std::string{key, length}] = item;
				return nytrue;
			}
			catch (...) {}
		}
	}
	return nyfalse;
}


extern "C" nyobj_t* nyobj_erase_ex(nyobj_t* obj, const char* key, uint32_t length)
{
	if (obj and key and length < 65536 and obj->type == nyit_dict)
	{
		auto* dict = obj->value.dict;
		if (dict)
		{
			auto count = dict->erase(std::string{key, length});
			return to_nybool(count != 0);
		}
	}
	return nullptr;
}


extern "C" int32_t nyobj_as_i32(const nyobj_t* obj)
{
	return convert_to_number<int32_t>(obj);
}

extern "C" int64_t nyobj_as_i64(const nyobj_t* obj)
{
	return convert_to_number<int64_t>(obj);
}

extern "C" uint32_t nyobj_as_u32(const nyobj_t* obj)
{
	return convert_to_number<uint32_t>(obj);
}

extern "C" uint64_t nyobj_as_u64(const nyobj_t* obj)
{
	return convert_to_number<uint64_t>(obj);
}

extern "C" float nyobj_as_f32(const nyobj_t* obj)
{
	return convert_to_number<float>(obj);
}

extern "C" double nyobj_as_f64(const nyobj_t* obj)
{
	return convert_to_number<double>(obj);
}

extern "C" nybool_t nyobj_as_bool(const nyobj_t* obj)
{
	if (obj)
	{
		switch (obj->type)
		{
			case nyit_i32:  return to_nybool(obj->value.i32 != 0);
			case nyit_i64:  return to_nybool(obj->value.i64 != 0);
			case nyit_u32:  return to_nybool(obj->value.u32 != 0);
			case nyit_u64:  return to_nybool(obj->value.u64 != 0);
			case nyit_f32:  return to_nybool(obj->value.f32 != 0);
			case nyit_f64:  return to_nybool(obj->value.f64 != 0);
			case nyit_bool: return to_nybool(obj->value.onoff);
			default: break;
		}
	}
	return nyfalse;
}


extern "C" const char* nyobj_as_str_ex(const nyobj_t* obj, uint32_t* len)
{
	if (obj)
	{
		switch (obj->type)
		{
			case nyit_shortstring: *len = obj->value.shortstr[0]; return &(obj->value.shortstr[1]);
			case nyit_string: *len = get_string_size(obj); return (const char*)(obj->value.string + 1);
			default: break;
		}
	}
	*len = 0;
	return "";
}


extern "C" nycolor_t nyobj_as_color(const nyobj_t* obj)
{
	if (obj)
	{
		switch (obj->type)
		{
			case nyit_color:
				return obj->value.rgba;
			case nyit_u32:
				return static_cast<nycolor_t>(obj->value.u32);
			default:
				break;
		}
	}
	return {0,0,0, 255};
}



/* ------------------------------------------------------------------------------- */









