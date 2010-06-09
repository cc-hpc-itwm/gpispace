#include "filename.h"

const char* get_extension(const char* name) {
    if (name == NULL)
	return NULL;

    const int length = strlen(name);
    int start_extension = length;

    int i = length-1;
    while ( (i >= 0) && (start_extension == length) )
    {
	if ( name[i] == '.' )
	{
	    start_extension = i+1;
	}
	--i;
    }

    if ( start_extension == length)
	return NULL;
    else
	return name + start_extension;
};
