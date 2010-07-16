#ifndef FILENAME_H
#define FILENAME_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * \brief Extract name extension (without '.') from file name
 *
 * 
 * \param name '\0'-terminated string or NULL
 * \return pointer to the extension in 'name' or NULL if 'name' contains no extension
 */
const char* get_extension(const char* name);

#endif
