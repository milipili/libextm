#ifndef LIBEXTM_OBJECT_H
#define LIBEXTM_OBJECT_H
#include "common.h"




#ifdef __cplusplus
extern "C" {
#endif

/* Object types */
enum nyobj_type_t
{
	nyo_null,    /*!< null object */
	nyo_dict,    /*!< unordered dictionary (unique keys) */
	nyo_array,   /*!< array of objects */
	nyo_string,  /*!< string */
	nyo_bool,    /*!< boolean (true/false) */
	nyo_i32,     /*!< signed integer 32 bits */
	nyo_i64,     /*!< signed integer 64 bits */
	nyo_u32,     /*!< unsigned integer 32 bits */
	nyo_u64,     /*!< unsigned integer 64 bits */
	nyo_f32,     /*!< floating point number 32 bits */
	nyo_f64,     /*!< floating point number 64 bits */
	nyo_color,   /*!< RGBA color */
};

/*! RGBA Color definition */
typedef union nycolor_t { uint8_t rgba[4]; uint32_t u32; } nycolor_t;

/*! Opaque type for Variant Object */
typedef struct
{
	uint32_t type;
	uintptr_t value;
}
nyobj_t;



/*! Increment internal reference count of an object */
NY_EXPORT void nyobj_ref(nyobj_t*);
/*! Decrement internal reference count of an object and destroys it if necessary */
NY_EXPORT void nyobj_unref(nyobj_t*);
/*! destroys an object (no refcount consideration) */
NY_EXPORT void nyobj_destroy_nocheck(nyobj_t*);

/*! Duplicate an object */
NY_EXPORT nyobj_t* nyobj_clone(const nyobj_t*);

/*! Swap the content of 2 objects */
NY_EXPORT void nyobj_swap(nyobj_t*, nyobj_t*);

/*! Copy the content of an object to another one */
NY_EXPORT void nyobj_copy(nyobj_t* dest, const nyobj_t* src);







/*! Create a null object */
NY_EXPORT nyobj_t* nyobj_null();
/*! Create a u64 object (unsigned 64bits integer) */
NY_EXPORT nyobj_t* nyobj_u64(uint64_t);
/*! Create a i64 object (signed 64bits integer) */
NY_EXPORT nyobj_t* nyobj_i64(int64_t);
/*! Create a u32 object (unsigned 32bits) */
NY_EXPORT nyobj_t* nyobj_u32(uint32_t);
/*! Create a i32 object (signed 32bits) */
NY_EXPORT nyobj_t* nyobj_i32(int32_t);
/*! Create a f32 object (floating point variable - 32bits IEEE 754) */
NY_EXPORT nyobj_t* nyobj_f32(float);
/*! Create a f64 object (floating point variable - 64bits IEEE 754) */
NY_EXPORT nyobj_t* nyobj_f64(double);
/*! Create a boolean object (!= 0) */
NY_EXPORT nyobj_t* nyobj_bool(int);
/*! Create a boolean object from a nybool_t */
NY_EXPORT nyobj_t* nyobj_bool(nybool_t);
/*! Create a char object (short string) */
NY_EXPORT nyobj_t* nyobj_char(char);
/*! Create a RGB object */
NY_EXPORT nyobj_t* nyobj_rgb(uint8_t, uint8_t, uint8_t);
/*! Create a RGBA object */
NY_EXPORT nyobj_t* nyobj_rgba(uint8_t, uint8_t, uint8_t, uint8_t);
/*! Create a RGBA object */
NY_EXPORT nyobj_t* nyobj_color(nycolor_t);
/*! Create a NaN object (Not A Number) */
NY_EXPORT nyobj_t* nyobj_nan();
/*! Create a string object from C-String (zero-terminated) */
NY_EXPORT nyobj_t* nyobj_str(const char* cstr);
/*! Create a string object from C-String (with a given length) */
NY_EXPORT nyobj_t* nyobj_str_ex(const char* cstr, uint32_t size);

/*! Create a dictionary */
NY_EXPORT nyobj_t* nyobj_dict();

/*! Create an array object */
NY_EXPORT nyobj_t* nyobj_array();
/*! Create an array object with a predefined array of C-strings */
NY_EXPORT nyobj_t* nyobj_array_of_strings(const char** argv, uint32_t count);
NY_EXPORT nyobj_t* nyobj_array_of_i32(const int32_t*, uint32_t count);
NY_EXPORT nyobj_t* nyobj_array_of_i64(const int64_t*, uint32_t count);
NY_EXPORT nyobj_t* nyobj_array_of_u32(const uint32_t* argv, uint32_t count);
NY_EXPORT nyobj_t* nyobj_array_of_u64(const uint64_t* argv, uint32_t count);
NY_EXPORT nyobj_t* nyobj_array_of_f32(const float* argv, uint32_t count);
NY_EXPORT nyobj_t* nyobj_array_of_f64(const double* argv, uint32_t count);
NY_EXPORT nyobj_t* nyobj_array_of_obj(nyobj_t** argv, uint32_t count);
NY_EXPORT nyobj_t* nyobj_array_of_clone_obj(const nyobj_t** argv, uint32_t count);



/*! Get the type og an object */
NY_EXPORT nyobj_type_t nyobj_type(const nyobj_t*);

/*! Get if the object is null */
NY_EXPORT nybool_t nyobj_is_null(const nyobj_t*);
/*! Get if the object is empty (null, empty string, empty dict, empty array) */
NY_EXPORT nybool_t nyobj_is_empty(const nyobj_t*);
/*! Get the number of elements (0 if the object is not an array or a dict or a string) */
NY_EXPORT uint32_t nyobj_size(const nyobj_t*);

/*! Get if the object is signed 32bits integer */
NY_EXPORT nybool_t nyobj_is_i32(const nyobj_t*);
/*! Get if the object is signed 64bits integer */
NY_EXPORT nybool_t nyobj_is_i64(const nyobj_t*);
/*! Get if the object is unsigned 32bits integer */
NY_EXPORT nybool_t nyobj_is_u32(const nyobj_t*);
/*! Get if the object is unsigned 64bits integer */
NY_EXPORT nybool_t nyobj_is_u64(const nyobj_t*);
/*! Get if the object is 32bits floating point number */
NY_EXPORT nybool_t nyobj_is_f32(const nyobj_t*);
/*! Get if the object is 64bits floating point number */
NY_EXPORT nybool_t nyobj_is_f64(const nyobj_t*);
/*! Get if the object is an array */
NY_EXPORT nybool_t nyobj_is_array(const nyobj_t*);
/*! Get if the object is an dictionary */
NY_EXPORT nybool_t nyobj_is_dict(const nyobj_t*);
/*! Get if the object is a string */
NY_EXPORT nybool_t nyobj_is_string(const nyobj_t*);
/*! Get if the object is a RGBA/RGB color */
NY_EXPORT nybool_t nyobj_is_color(const nyobj_t*);


/*! Mutate the object into an array (preserving the values as children) */
NY_EXPORT void nyobj_mutate_to_array(nyobj_t*);
/*! Mutate the object into an dict (preserving the values as children) */
NY_EXPORT void nyobj_mutate_to_dict(nyobj_t*);

/*! Clear the container held by the object (array, dict, or string) or reset to 0 */
NY_EXPORT void nyobj_clear(nyobj_t*);



/*! Append an object (as-it) to an array (transform the object to an array if needed) */
NY_EXPORT nybool_t nyobj_append(nyobj_t*, nyobj_t*);

/*! Reset the object at index N (transform the object to an array and expand it if needed) */
NY_EXPORT nybool_t nyobj_set_index(nyobj_t*, uint32_t index, nyobj_t*);
/*! Get the item at index N, or null if not found (refcount untouched) */
NY_EXPORT nyobj_t* nyobj_get_index(nyobj_t*, uint32_t index);


/*! Get the value object for the given key, if exists (refcount untouched) */
NY_EXPORT nyobj_t* nyobj_get(nyobj_t*, const char* key);
/*! Get the value object for the given key, if exists (refcount untouched) */
NY_EXPORT nyobj_t* nyobj_get_ex(nyobj_t*, const char* key, uint32_t);

/*! Set the value for the given key (transform the object into a dict if needed) */
NY_EXPORT nybool_t nyobj_set(nyobj_t*, const char* key, nyobj_t*);
/*! Set the value for the given key (transform the object into a dict if needed) */
NY_EXPORT nybool_t nyobj_set_ex(nyobj_t*, const char* key, uint32_t, nyobj_t*);

/*! Remove the key from a dict, if exists */
NY_EXPORT nybool_t nyobj_erase(nyobj_t*, const char* key);
/*! Remove the key from a dict, if exists */
NY_EXPORT nybool_t nyobj_erase_ex(nyobj_t*, const char* key, uint32_t);


/*! Get the value of an object as signed 32bits integer */
NY_EXPORT int32_t nyobj_as_i32(const nyobj_t*);
/*! Get the value of an object as signed 64bits integer */
NY_EXPORT int64_t nyobj_as_i64(const nyobj_t*);
/*! Get the value of an object as unsigned 32bits integer */
NY_EXPORT uint32_t nyobj_as_u32(const nyobj_t*);
/*! Get the value of an object as unsigned 64bits integer */
NY_EXPORT uint64_t nyobj_as_u64(const nyobj_t*);
/*! Get the value of an object as 32bits floating point number */
NY_EXPORT float nyobj_as_f32(const nyobj_t*);
/*! Get the value of an object as 64bits floating point number */
NY_EXPORT double nyobj_as_f64(const nyobj_t*);
/*! Get the value of an object as a bool (or a number != 0) */
NY_EXPORT nybool_t nyobj_as_bool(const nyobj_t*);
/*! Get the value of an object as a string */
NY_EXPORT const char* nyobj_as_str(const nyobj_t*);
/*! Get the value of an object as a string (and gets the size) */
NY_EXPORT const char* nyobj_as_str_ex(const nyobj_t*, uint32_t* size);
/*! Get the value of an object as a color (numbers will be converted string converted as hexa or rgb(r,g,b)) */
NY_EXPORT nycolor_t nyobj_as_color(const nyobj_t*);











static inline nybool_t nyobj_is_null(const nyobj_t* obj)
{
	return obj ? nyfalse : nytrue;
}

static inline nybool_t nyobj_is_i32(const nyobj_t* obj)
{
	return nyobj_type(obj) == nyo_i32;
}

static inline nybool_t nyobj_is_i64(const nyobj_t* obj)
{
	return nyobj_type(obj) == nyo_i64;
}

static inline nybool_t nyobj_is_u32(const nyobj_t* obj)
{
	return nyobj_type(obj) == nyo_u32;
}

static inline nybool_t nyobj_is_u64(const nyobj_t* obj)
{
	return nyobj_type(obj) == nyo_u64;
}

static inline nybool_t nyobj_is_f32(const nyobj_t* obj)
{
	return nyobj_type(obj) == nyo_f32;
}

static inline nybool_t nyobj_is_f64(const nyobj_t* obj)
{
	return nyobj_type(obj) == nyo_f64;
}

static inline nybool_t nyobj_is_array(const nyobj_t* obj)
{
	return nyobj_type(obj) == nyo_array;
}

static inline nybool_t nyobj_is_dict(const nyobj_t* obj)
{
	return nyobj_type(obj) == nyo_dict;
}

static inline nybool_t nyobj_is_string(const nyobj_t* obj)
{
	return nyobj_type(obj) == nyo_string;
}

static inline nybool_t nyobj_is_color(const nyobj_t* obj)
{
	return nyobj_type(obj) == nyo_color;
}

static inline nyobj_t* nyobj_get(nyobj_t* obj, const char* key)
{
	size_t length = key ? strlen(key) : 0;
	return (length < 65536) ? nyobj_get_ex(obj, key, (uint32_t) length) : NULL;
}

static inline nybool_t nyobj_set(nyobj_t*, const char* key, nyobj_t* item)
{
	size_t length = key ? strlen(key) : 0;
	return (length < 65536) ? nyobj_set_ex(obj, key, (uint32_t) length, item) : nyfalse;
}

static inline nybool_t nyobj_erase(nyobj_t*, const char* key)
{
	size_t length = key ? strlen(key) : 0;
	return (length < 65536) ? nyobj_erase_ex(obj, key, (uint32_t) length) : nyfalse;
}


#ifdef __cplusplus
} // extern "C"
#endif

#endif /*e LIBEXTM_OBJECT_H */
