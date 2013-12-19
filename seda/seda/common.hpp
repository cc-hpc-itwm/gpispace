/* 
   Copyright (C) 2009 Alexander Petry <alexander.petry@itwm.fraunhofer.de>.

   This file is part of seda.

   seda is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   seda is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with seda; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  

*/

/* ensure this file gets parsed just once during every compile use the name of
 * this identifier the same as the file gets installed, i.e. if the header file
 * gets installed into /usr/include/mylib/myheader.h, name it MYLIB_MYHEADER_H
 */
#ifndef SEDA_COMMON_HPP
#define SEDA_COMMON_HPP 1

/*
 * Purpose of this file:
 *
 *   function declarations, portability code, include files or macros used in
 *   nearly every other place of the project.
 */

/* Include some often used header files */
#include <stdio.h>
#include <sys/types.h>

#if STDC_HEADERS
#  include <stdlib.h>
#  include <string.h>
#elif HAVE_STRINGS_H
#  include <strings.h>
#endif /*STDC_HEADERS*/

#if HAVE_UNISTD_H
#  include <unistd.h>
#endif

#if HAVE_ERRNO_H
#  include <errno.h>
#endif /*HAVE_ERRNO_H*/
#ifndef errno
/* Some systems #define this! */
extern int errno;
#endif

/* provide a replacement for bzero if it is not available */
#if !HAVE_BZERO && HAVE_MEMSET
#  define bzero(buf, bytes) ((void) memset(buf, 0, bytes))
#endif

/* if including C source from within C++ code, the function definitions must be
 * enclosed in "extern "C"" { ... }, so provide a often used macro */

#ifdef __cplusplus
#  define BEGIN_C_DECLS extern "C" {
#  define END_C_DECLS   }
#else /* !__cplusplus */
#  define BEGIN_C_DECLS
#  define END_C_DECLS
#endif /* __cplusplus */

/* for readability of the code, we provide two general defines for exitcodes */
#ifndef EXIT_SUCCESS
#  define EXIT_SUCCESS 0
#  define EXIT_FAILURE 1
#endif

#include <seda/logging.hpp>

#endif /* !SEDA_COMMON_HPP */
