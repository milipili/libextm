#ifndef LIBEXTM_OBJECT_H
#define LIBEXTM_OBJECT_H


/* Object types */
enum nyobj_type_t: uint32_t
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
typedef struct nyobj_t nyobj_t;



/*! Increment internal reference count of an object */
NY_EXPORT void nyobj_ref(nyobj_t*);
/*! Decrement internal reference count of an object and destroys it if necessary */
NY_EXPORT void nyobj_unref(nyobj_t*);

/*! Duplicate an object */
NYEXPORT nyobj_t* nyobj_clone(const nyobj_t*);

/*! Swap the content of 2 objects */
NY_EXPORT void nyobj_swap(nyobj_t*, nyobj_t*);

/*! Copy the content of an object to another one */
NY_EXPORT void nyobj_copy(nyobj_t* dest, const nyobj_t* src);







/*! Create a null object */
nyobj_t* nyobj_null();
/*! Create a u64 object (unsigned 64bits integer) */
nyobj_t* nyobj_u64(uint64_t);
/*! Create a i64 object (signed 64bits integer) */
nyobj_t* nyobj_i64(int64_t);
/*! Create a u32 object (unsigned 32bits) */
nyobj_t* nyobj_u32(uint32_t);
/*! Create a i32 object (signed 32bits) */
nyobj_t* nyobj_i32(int32_t);
/*! Create a f32 object (floating point variable - 32bits IEEE 754) */
nyobj_t* nyobj_f32(float);
/*! Create a f64 object (floating point variable - 64bits IEEE 754) */
nyobj_t* nyobj_f64(double);
/*! Create a boolean object (!= 0) */
nyobj_t* nyobj_bool(int);
/*! Create a char object (short string) */
nyobj_t* nyobj_char(char);
/*! Create a RGB object */
nyobj_t* nyobj_rgb(uint8_t, uint8_t, uint8_t);
/*! Create a RGBA object */
nyobj_t* nyobj_rgba(uint8_t, uint8_t, uint8_t, uint8_t);
/*! Create a RGBA object */
nyobj_t* nyobj_color(nycolor_t);
/*! Create a NaN object (Not A Number) */
nyobj_t* nyobj_nan();
/*! Create a string object from C-String (zero-terminated) */
nyobj_t* nyobj_str(const char* cstr);
/*! Create a string object from C-String (with a given length) */
nyobj_t* nyobj_str_ex(const char* cstr, uint32_t size);
/*! Create a dictionary */
nyobj_t* nyobj_dict();
/*! Create an array object */
nyobj_t* nyobj_array();
/*! Create an array object with a predefined array of C-strings */
nyobj_t* nyobj_array_of_strings(const char** argv, uint32_t count);
nyobj_t* nyobj_array_of_i32(const int32_t*, uint32_t count);
nyobj_t* nyobj_array_of_i64(const int64_t*, uint32_t count);
nyobj_t* nyobj_array_of_u32(const uint32_t* argv, uint32_t count);
nyobj_t* nyobj_array_of_u64(const uint64_t* argv, uint32_t count);
nyobj_t* nyobj_array_of_f32(const float* argv, uint32_t count);
nyobj_t* nyobj_array_of_f64(const double* argv, uint32_t count);
nyobj_t* nyobj_array_of_obj(nyobj_t** argv, uint32_t count);
nyobj_t* nyobj_array_of_clone_obj(const nyobj_t** argv, uint32_t count);



/*! Get the type og an object */
nyobj_type_t nyobj_type(const nyobj_t*);
/*! Get if the object is null */
nybool_t nyobj_is_null(const nyobj_t*);
/*! Get if the object is empty (null, empty string, empty dict, empty array) */
nybool_t nyobj_is_empty(const nyobj_t*);

/*! Get if the object is signed 32bits integer */
nybool_t nyobj_is_i32(const nyobj_t*);
/*! Get if the object is signed 64bits integer */
nybool_t nyobj_is_i64(const nyobj_t*);
/*! Get if the object is unsigned 32bits integer */
nybool_t nyobj_is_u32(const nyobj_t*);
/*! Get if the object is unsigned 64bits integer */
nybool_t nyobj_is_u64(const nyobj_t*);
/*! Get if the object is 32bits floating point number */
nybool_t nyobj_is_f32(const nyobj_t*);
/*! Get if the object is 64bits floating point number */
nybool_t nyobj_is_f64(const nyobj_t*);
/*! Get if the object is an array */
nybool_t nyobj_is_array(const nyobj_t*);
/*! Get if the object is an dictionary */
nybool_t nyobj_is_dict(const nyobj_t*);
/*! Get if the object is a string */
nybool_t nyobj_is_string(const nyobj_t*);
/*! Get if the object is a RGBA/RGB color */
nybool_t nyobj_is_color(const nyobj_t*);
/*! Get if the object is a RGB color */
nybool_t nyobj_is_rgb(const nyobj_t*);
/*! Get if the object is a RGBA color (has opacity) */
nybool_t nyobj_is_rgba(const nyobj_t*);

int32_t nyobj_as_i32(nyobj_t*);
int64_t nyobj_as_i64(nyobj_t*);
uint32_t nyobj_as_u32(nyobj_t*);
uint64_t nyobj_as_u64(nyobj_t*);
float nyobj_as_f32(nyobj_t*);
double nyobj_as_f64(nyobj_t*);
nybool_t nyobj_as_bool(nyobj_t*);
const char* nyobj_as_str(nyobj_t*);
const char* nyobj_as_str_ex(nyobj_t*, uint32_t*);
nycolor_t nyobj_as_color(const nyobj_t*);

void nyobj_mutate_to_array(nyobj_t*);
void nyobj_mutate_to_dict(nyobj_t*);

void nyobj_clear(nyobj_t*);

nyobj_t* nyobj_array_get(nyobj_t*, uint32_t index);
void nyobj_array_set(nyobj_t*, uint32_t index, nyobj_t*);
void nyobj_array_add(nyobj_t*, nyobj_t*);
void nyobj_array_reserve(nyobj_t*, nyobj_t*);

nyobj_t* nyobj_get(nyobj_t*, const char* key);
void nyobj_set(nyobj_t*, const char* key, nyobj_t*);
void nyobj_set_ex(nyobj_t*, const char* key, uint32_t keylen, nyobj_t*);
void nyobj_erase(nyobj_t*, const char* key);
void nyobj_erase_ex(nyobj_t*, const char* key, uint32_t keylen);
uint32_t nyobj_size(const nyobj_t*);





static inline nyobj_t* nyobj_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	return nyobj_color({r, g, b, a});
}

static inline nyobj_t* nyobj_rgb(uint8_t r, uint8_t g, uint8_t b)
{
	return nyobj_rgba(r, g, b, 255);
}

static inline nybool_t nyobj_is_null(const nyobj_t* obj)
{
	return !obj ? nyfalse : nytrue;
}


#endif /*e LIBEXTM_OBJECT_H */
