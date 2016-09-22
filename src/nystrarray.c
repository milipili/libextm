#include <nyextm/nyextm.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>




char** nystrarray_create(uint32_t count)
{
	char** array = (char**) malloc(sizeof(char*) * (count + 1));
	array[count] = NULL;
	return array;
}


void nystrarray_free(char** list)
{
	if (list)
	{
		for (char* it = *list; it != NULL; ++it)
			free(it);
		free(list);
	}
}


void nystrarray_set(char** list, uint32_t index, const char* str)
{
	if (list)
	{
		size_t slen = (str ? strlen(str) : 0);
		uint32_t len = (slen < UINT_MAX - 1) ? static_cast<uint32_t>(slen) : (UINT_MAX - 1);
		char* newelem = (char*) malloc(sizeof(char) * (len + 1));
		memcpy(newelem, str, sizeof(char) * len);
		newelem[len] = '\0';
		free(list[index]);
		list[index] = newelem;
	}
}


void nystrarray_set_ex(char** list, uint32_t index, const char* str, uint32_t len)
{
	if (list)
	{
		assert(len == 0 or str != NULL);
		char* newelem = (char*) malloc(sizeof(char) * (len + 1));
		memcpy(newelem, str, sizeof(char) * len);
		newelem[len] = '\0';
		free(list[index]);
		list[index] = newelem;
	}
}
