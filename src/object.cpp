#include "extm/object.h"
#include <iso646.h>
#include <string>
#include <unordered_map>
#include <vector>



extern "C"
{
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
		uint64_t raw;
		int32_t i32;
		int64_t i64;
		uint32_t u32;
		uint64_t u64;
		float f32;
		double f64;
		void* ptr;
		uintptr_t* uintptr;
		char* cstr;
		char ncstr[8];
		bool flag;
		nycolor_t rgba;
	};

	struct nyobj_t
	{
		uint32_t refcount;
		nyobj_internaltype_t type;
		nyobj_value_t value;
	};

} // extern C


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


	using NyObjMap   = std::unordered_map<std::string, nyobj_refptr_t>;
	using NyObjArray = std::vector<nyobj_refptr_t>;




	inline nybool_t tobool_t(bool v)
	{
		return v ? nytrue : nyfalse;
	}


	nyobj_t* createobject(nyobj_internaltype_t type, nyobj_value_t value)
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


	inline void delete_object_content(nyobj_t& obj)
	{
		switch (obj.type)
		{
			default:
				break;
			case nyit_string:
			{
				free(obj.value.ptr);
				break;
			}
			case nyit_dict:
			{
				delete (reinterpret_cast<NyObjMap*>(obj.value.ptr));
				break;
			}
			case nyit_array:
			{
				delete (reinterpret_cast<NyObjArray*>(obj.value.ptr));
				break;
			}
		}
	}


	void delete_object(nyobj_t* object)
	{
		delete_object_content(*object);
		free(obj);
	}

	static inline void inline_nyobj_unref(nyobj_t* object)
	{
		if (object and 0 == --object->refcount)
			delete_object(object);
	}


	size_t get_string_buffer_size(const nyobj_t* const object)
	{
		return /*size*/ sizeof(uintptr_t) + /*string size*/ *(object->value.uintptr) + /*zero*/ 1u;
	}


	static inline bool nyobj_rawcopy_content(nyobj_t* dest, const nyobj_t* src)
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
				size_t stringsize = get_string_buffer_size(dest);
				char* p = (char*) malloc(size);
				if (!p)
					return false;

				dest->value.ptr = p;
				char* src = src->value.cstr;
				do { *p = *src; } while (size-- != 0);
				break;
			}
			case nyit_dict:
			{
				auto& dict = *reinterpret_cast<NyObjMap*>(rhs->value.ptr);
				auto* newdict = new (std::nothrow) NyObjMap;
				if (!newdict)
					return false;
				try
				{
					for (auto& pair: dict)
					{
						auto* clone = nyobj_clone(pair.second.object);
						if (!clone)
						{
							delete newdict;
							return false;
						}
						newdict->insert(std::make_pair<std::string, nyobj_refptr_t>(pair.first, clone));
						--(clone->refcount);
					}
					dest->value.ptr = newdict;
				}
				catch (...)
				{
					delete newdict;
					return false;
				}
				break;
			}
			case nyit_array:
			{
				auto& array = *reinterpret_cast<NyObjArray*>(rhs->value.ptr);
				auto* newarray = new (std::nothrow) NyObjArray;
				if (!newarray)
					return false;
				try
				{
					newarray->reserve(array.size());
					for (auto& element: array)
					{
						auto* clone = nyobj_clone(element.object);
						if (!clone)
						{
							delete newarray;
							return false;
						}
						newarray->emplace_back(clone);
						--(clone->refcount);
					}
					dest->value.ptr = newarray;
				}
				catch (...)
				{
					delete newarray;
					return false;
				}
				break;
			}
		}
		return true;
	}


	static inline nyobj_t* make_nyobj_str_ex(const char* cstr, uint32_t size)
	{
		auto* object = (nyobj_t*) malloc(sizeof(nyobj_t));
		if (!object)
			return nullptr;

		object->refcount = 1;
		if (size < sizeof(uint64_t) - 1 /*string size*/) // shortstring optimization
		{
			object->type = nyit_shortstring;
			object->value.ncstr[0] = static_cast<uint8_t>(size);
			for (uint32_t i = 0; i != size; ++i)
				object->value.ncstr[i + 1] = cstr[i];
			object->value.ncstr[size + 1] = '\0';
		}
		else
		{
			size_t capacity = size + sizeof(uint64_t) + 1;
			void* p = malloc(capacity);
			if (!p)
			{
				free(object);
				return nullptr;
			}
			*reinterpret_cast<uint32_t*>(p) = size;
			memcpy(p + sizeof(uint32_t), cstr, size);
			p[size + 1 + sizeof(uint32_t)] = '\0';
			object->type = nyit_string;
			object->value.ptr = p;
		}
		return object;
	}


	template<class T, nyobj_t* (*Factory)(T)>
	static inline nyobj_t* nyobj_build_array(const T* args, uint32_t count)
	{
		if (count)
		{
			NyObjArray* array = new (std::nothrow) NyObjArray;
			if (array)
			{
				nyobj_t* obj = createobject(nyit_array, array);
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
		}
		return createobject(nyit_array, nullptr);
	}


	static inline NyObjArray* ensureAndFetchArray(nyobj_t* obj)
	{
		if (obj)
		{
			if (obj->type != nyit_array)
				nyobj_mutate_to_array(obj);
			auto* array = reinterpret_cast<NyObjArray*>(obj->value.ptr);
			if (!array)
			{
				array = new (std::nothrow)
			}
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


extern "C" void nyobj_unref(nyobj_t* obj)
{
	inline_nyobj_unref(obj);
}


extern "C" nyobj_t* nyobj_clone(const nyobj_t* rhs)
{
	if (!rhs)
		return nullptr;

	nyobj_t* newobj = (nyobj_t*) malloc(sizeof(nyobj_t));
	if (!newobj)
		return nullptr;

	newobj->refcount = 1;
	if (not nyobj_rawcopy_content(newobj, rhs))
	{
		free(newobj);
		return nullptr;
	}
	return newobj;
}


extern "C" void nyobj_copy(nyobj_t* dest, const nyobj_t* src)
{
	if (!dest)
		return nullptr;

	delete_object_content(dest);
	if (!src or not nyobj_rawcopy_content(dest, src))
	{
		dest->type = nyit_u32;
		dest->value.u32 = 0;
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
	return createobject(nyit_f32, std::numeric_limits<float>::quiet_NaN());
}


extern "C" nyobj_t* nyobj_color(nycolor_t color)
{
	return createobject(nyit_color, color);
}

extern "C" nyobj_t* nyobj_u64(uint64_t value)
{
	return createobject(nyit_u64, value);
}

extern "C" nyobj_t* nyobj_i64(int64_t value)
{
	return createobject(nyit_i64, value);
}

extern "C" nyobj_t* nyobj_u32(uint32_t value)
{
	return createobject(nyit_u32, value);
}

extern "C" nyobj_t* nyobj_i32(int32_t value)
{
	return createobject(nyit_i32, value);
}

extern "C" nyobj_t* nyobj_f32(float value)
{
	return createobject(nyit_f32, value);
}

extern "C" nyobj_t* nyobj_f64(double value)
{
	return createobject(nyit_f64, value);
}

extern "C" nyobj_t* nyobj_dict()
{
	return createobject(nyit_dict, nullptr);
}

extern "C" nyobj_t* nyobj_array()
{
	return createobject(nyit_dict, nullptr);
}

extern "C" nyobj_t* nyobj_bool(int value)
{
	return createobject(nyit_bool, (value != 0));
}


extern "C" nyobj_t* nyobj_str(const char* cstr)
{
	size_t length = (cstr) ? strlen(cstr) : 0;
	return (length < 2 * 1024 * 1024 * 1024)
		? make_nyobj_str_ex(cstr, static_cast<uint32_t>(length))
		: nullptr;
}

extern "C" nyobj_t* nyobj_str_ex(const char* cstr, uint32_t size)
{
	return make_nyobj_str_ex(cstr, size);
}

extern "C" nyobj_t* nyobj_char(char c)
{
	nyobj_value_t value;
	value.ncstr[0] = '\1'; // size in bytes
	value.ncstr[1] = c;
	value.ncstr[2] = '\0';
	return createobject(nyit_shortstring, value);
}


extern "C" nyobj_t* nyobj_array_of_obj(const nyobj_t** args, uint32_t count)
{
	if (count)
	{
		NyObjArray* array = new (std::nothrow) NyObjArray;
		if (array)
		{
			nyobj_t* obj = createobject(nyit_array, array);
			array->reserve(count);
			for (uint32_t i = 0; i != count; ++i, ++args)
				array->emplace_back(*args);
			return obj;
		}
	}
	return createobject(nyit_array, nullptr);
}


extern "C" nyobj_t* nyobj_array_of_clone_obj(const nyobj_t** args, uint32_t count)
{
	if (count)
	{
		NyObjArray* array = new (std::nothrow) NyObjArray;
		if (array)
		{
			nyobj_t* obj = createobject(nyit_array, array);
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
	}
	return createobject(nyit_array, nullptr);
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




extern "C" nybool_t nyobj_is_i32(const nyobj_t* obj)
{
	return tobool_t(obj and obj->type == nyit_i32);
}

extern "C" nybool_t nyobj_is_i64(const nyobj_t* obj)
{
	return tobool_t(obj and obj->type == nyit_i64);
}

extern "C" nybool_t nyobj_is_u32(const nyobj_t* obj)
{
	return tobool_t(obj and obj->type == nyit_u32);
}

extern "C" nybool_t nyobj_is_u64(const nyobj_t* obj)
{
	return tobool_t(obj and obj->type == nyit_u64);
}

extern "C" nybool_t nyobj_is_f32(const nyobj_t* obj)
{
	return tobool_t(obj and obj->type == nyit_f32);
}

extern "C" nybool_t nyobj_is_f64(const nyobj_t* obj)
{
	return tobool_t(obj and obj->type == nyit_f64);
}

extern "C" nybool_t nyobj_is_array(const nyobj_t* obj)
{
	return tobool_t(obj and obj->type == nyit_array);
}

extern "C" nybool_t nyobj_is_dict(const nyobj_t* obj)
{
	return tobool_t(obj and obj->type == nyit_dict);
}

extern "C" nybool_t nyobj_is_string(const nyobj_t* obj)
{
	return tobool_t(obj and (obj->type == nyit_shortstring or obj->type == nyit_string));
}

extern "C" nybool_t nyobj_is_color(const nyobj_t* obj)
{
	return tobool_t(obj and obj->type == nyit_color);
}

extern "C" nybool_t nyobj_is_rgb(const nyobj_t* obj)
{
	return tobool_t(obj and obj->type == nyit_color and obj->value.rgba[3] == 255);
}

extern "C" nybool_t nyobj_is_rgba(const nyobj_t* obj)
{
	return tobool_t(obj and obj->type == nyit_color and obj->value.rgba[3] != 255);
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
				return *(src->value.u32ptr);
			}
			case nyit_dict:
			{
				if (obj->value.ptr)
					return reinterpret_cast<NyObjMap*>(obj->value.ptr)->size();
				break;
			}
			case nyit_array:
			{
				if (obj->value.ptr)
					return reinterpret_cast<NyObjArray*>(obj->value.ptr)->size();
				break;
			}
			default:
				break;
		}
	}
	return 0;
}


extern "C" nybool_t nyobj_is_empty(const nyobj_t* obj)
{
	if (!obj)
		return nytrue;
	switch (obj->type)
	{
		case nyit_shortstring:
			return (obj->value.ncstr[0] == 0);
		case nyit_dict:
		case nyit_array:
			return (obj->value.ptr == nullptr);
		default:
			break;
	}
	return nyfalse;
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
			auto* dict = reinterpret_cast<NyObjMap*>(obj.value.ptr);
			if (dict)
			{
				newarray = new (std::nothrow) NyObjArray;
				if (newarray)
				{
					newarray->reserve(dict->size());
					for (auto& pair: *dict)
						newarray->emplace_back(pair.second);
				}
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
					newarray->emplace_back(firstitem);
					break;
				}
				delete newarray;
				newarray = nullptr;
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
			auto* array = reinterpret_cast<NyObjArray*>(obj.value.ptr);
			if (array)
			{
				newdict = new (std::nothrow) NyObjMap;
				if (newdict)
				{
					uint32_t i = 0;
					for (auto& element: *array)
						newdict->insert(std::make_pair(to_string(i++), element));
				}
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
					newdict->insert(std::make_pair("_", firstitem));
					break;
				}
				delete newdict;
				newdict = nullptr;
			}
			break;
		}
	}
	obj->type = nyit_array;
	obj->value.ptr = newdict;
}


extern "C" void nyobj_array_add(nyobj_t* array, nyobj_t* obj)
{
	if (array)
	{
		if (array->type != nyit_array)
			nyobj_mutate_to_array(array);

		auto* array = reinterpret_cast<NyObjArray*>(rhs->value.ptr);
		if (array)
			array->emplace_back(obj);
	}
}


extern "C" void nyobj_array_reserve(nyobj_t* array, uint32_t count)
{
	if (array)
	{
		if (array->type != nyit_array)
			nyobj_mutate_to_array(array);

		auto* array = reinterpret_cast<NyObjArray*>(rhs->value.ptr);
		if (array)
			array->reserve(count);
	}
}


extern "C" void nyobj_array_set(nyobj_t* array, uint32_t index, nyobj_t* obj)
{
	if (array)
	{
		if (array->type != nyit_array)
			nyobj_mutate_to_array(array);

		auto* array = reinterpret_cast<NyObjArray*>(rhs->value.ptr);
		if (array)
		{
			if (index < array->size())
			{
				(*index)[index] = obj;
			}
			else
			{
				array->resize(index);
				array->emplace_back(obj);
			}
		}
	}
}


extern "C" void nyobj_array_clear(nyobj_t* array)
{
	if (array)
	{
		if (array->type != nyit_array)
			nyobj_mutate_to_array(array);

		auto* array = reinterpret_cast<NyObjArray*>(rhs->value.ptr);
		if (array)
			array->clear();
	}
}


extern "C" nyobj_t* nyobj_array_set(nyobj_t* array, uint32_t index)
{
	if (array and array->type == nyit_array)
	{
		auto* array = reinterpret_cast<NyObjArray*>(rhs->value.ptr);
		if (array and index < array->size())
			return (*array)[index].object;
	}
	return nullptr;
}


extern "C" nyobj_t* nyobj_get(nyobj_t* obj, const char* key)
{
	if (obj and obj->type == nyit_dict)
	{
		auto* dict = reinterpret_cast<NyObjMap*>(obj.value.ptr);
		auto it = dict->find(key);
		if (it != dict->end())
			return it->second.object;
	}
	return nullptr;
}

extern "C" nyobj_t* nyobj_get_ex(nyobj_t* obj, const char* key, uint32_t keylen)
{
	if (obj and obj->type == nyit_dict)
	{
		auto* dict = reinterpret_cast<NyObjMap*>(obj.value.ptr);
		auto it = dict->find(std::string{key, keylen});
		if (it != dict->end())
			return it->second.object;
	}
	return nullptr;
}


extern "C" nyobj_t* nyobj_erase(nyobj_t* obj, const char* key)
{
	if (obj and obj->type == nyit_dict)
	{
		auto* dict = reinterpret_cast<NyObjMap*>(obj.value.ptr);
		dict->erase(key);
	}
}

extern "C" nyobj_t* nyobj_erase_ex(nyobj_t* obj, const char* key, uint32_t keylen)
{
	if (obj and obj->type == nyit_dict)
	{
		auto* dict = reinterpret_cast<NyObjMap*>(obj.value.ptr);
		dict->erase(std::string{key, keylen});
	}
}

extern "C" nyobj_t* nyobj_set(nyobj_t* obj, const char* key, nyobj_t* value)
{
	if (obj)
	{
		if (obj->type != nyit_dict)
			nyobj_mutate_to_dict(obj);

		auto* dict = reinterpret_cast<NyObjMap*>(obj.value.ptr);
		if (dict)
		{
			std::string sk{key, keylen};
			auto it = dict->find(sk);
			if (it != dict->end())
				it->second = value;
			else
				dict->insert(std::make_pair(std::string{sk}, nyobj_refptr_t{value}));
		}
	}
}

extern "C" nyobj_t* nyobj_set_ex(nyobj_t* obj, const char* key, uint32_t keylen, nyobj_t* value)
{
	if (obj)
	{
		if (obj->type != nyit_dict)
			nyobj_mutate_to_dict(obj);

		auto* dict = reinterpret_cast<NyObjMap*>(obj.value.ptr);
		if (dict)
		{
			std::string sk{key, keylen};
			auto it = dict->find(sk);
			if (it != dict->end())
				it->second = value;
			else
				dict->insert(std::make_pair(std::string{sk}, nyobj_refptr_t{value}));
		}
	}
}


extern "C" void nyobj_dict_clear(nyobj_t* dict)
{
	if (dict)
	{
		if (array->type != nyit_dict)
			nyobj_mutate_to_dict(dict);

		auto* dict = reinterpret_cast<NyObjMap*>(rhs->value.ptr);
		if (dict)
			dict->clear();
	}
}


extern "C" nycolor_t nyobj_as_color(const nyobj_t* obj)
{
	return (obj and obj->type == nyit_color)
		? obj->value.color
		: {0,0,0, 255};
}
