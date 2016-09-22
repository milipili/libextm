#ifndef LIBEXTM_COMMON_HEADER_H
#define LIBEXTM_COMMON_HEADER_H
#include <string.h>
#include <stdint.h>

#ifndef NY_EXPORT
#if defined(_WIN32) || defined(__CYGWIN__)
#  ifdef __GNUC__
#    define LIBEXTMC_VISIBILITY_EXPORT   __attribute__ ((dllexport))
#    define LIBEXTMC_VISIBILITY_IMPORT   __attribute__ ((dllimport))
#  else
#    define LIBEXTMC_VISIBILITY_EXPORT   __declspec(dllexport) /* note: actually gcc seems to also supports this syntax */
#    define LIBEXTMC_VISIBILITY_IMPORT   __declspec(dllimport) /* note: actually gcc seems to also supports this syntax */
#  endif
#else
#  define LIBEXTMC_VISIBILITY_EXPORT     __attribute__((visibility("default")))
#  define LIBEXTMC_VISIBILITY_IMPORT     __attribute__((visibility("default")))
#endif

#if defined(_DLL) && !defined(LIBEXTMC_DLL_EXPORT)
#  define LIBEXTMC_DLL_EXPORT
#endif

/*!
** \macro NY_EXPORT
** \brief Export / import a libnany symbol (function)
*/
#if defined(LIBEXTMC_DLL_EXPORT)
#	define NY_EXPORT LIBEXTMC_VISIBILITY_EXPORT
#else
#	define NY_EXPORT LIBEXTMC_VISIBILITY_IMPORT
#endif
#endif /* NY_EXPORT */




#ifdef __cplusplus
extern "C" {
#endif

enum nyext_err_t
{
	/*! No error */
	nyexr_none,
	/*! Failed (generic error) */
	nyexr_failed,
	/*! Not found */
	nyexr_not_found,
	/*! Duplicate entry found */
	nyexr_duplicate,
	/*! When a source is not ready */
	nyexr_not_available,
	/*! Timeout has been reached */
	nyexr_timeout,
};


#ifndef LIBNANYC_ANYSTR_T
#define LIBNANYC_ANYSTR_T
typedef struct
{
	const char* cstr;
	uint32_t size;
}
nyanystr_t;

/*! Creat ean nyanystr_t from a c-string */
static inline nyanystr_t nycstr(const char* text)
{
	return nyanystr_t{text, strlen(text)};
}

/*! Creat ean nyanystr_t from a c-string */
static inline nyanystr_t nycstr(const char* text)
{
	return nyanystr_t{text, strlen(text)};
}

/*! Creat ean nyanystr_t from a c-string */
static inline void nycstr_duplicate(nyanystr_t* out, const nyanystr_t* src)
{
	uint32_t len = src->size;
	const char* srcstr = src->cstr;
	char* str = malloc(sizeof(char) * (len + 1));
	memcpy(str, srcstr, len);
	str[len] = '\0';
	out->cstr = str;
	out->size = len;
}

#endif

#ifndef LIBNANYC_NYBOOL_T
#define LIBNANYC_NYBOOL_T
enum nybool_t { nyfalse, nytrue };
#endif

#ifndef LIBNANYC_VERSION_T
#define LIBNANYC_VERSION_T
typedef struct
{
	uint32_t hi;
	uint32_t lo;
	uint32_t patch;
	char metadata[12]; /*zero-terminated*/
}
nyversion_t;
#endif

/*! Create an array of c-strings */
NY_EXPORT char** nystrarray_create(uint32_t count);
/*! Free an array of c-strings (and free all elements) */
NY_EXPORT void nystrarray_free(char**);
/*! Copy a string and assign it to the Nth element (free the previous if not null) */
NY_EXPORT void nystrarray_set(char**, uint32_t index, const char* str);
/*! Copy a string and assign it to the Nth element (free the previous if not null) */
NY_EXPORT void nystrarray_set_ex(char**, uint32_t index, const char* str, uint32_t len);
/*@}*/




#ifdef __cplusplus
} // extern "C"
#endif

#endif /* LIBEXTM_COMMON_HEADER_H */
