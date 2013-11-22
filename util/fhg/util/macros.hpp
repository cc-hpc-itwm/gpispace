// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_MACROS_HPP
#define FHG_UTIL_MACROS_HPP

//! \note A function that never returns, i.e. always throws or calls exit()
#if defined (__GNUC__) || defined (__clang__)
#define FHG_ATTRIBUTE_NORETURN __attribute__ ((noreturn))
#else
#if __cplusplus >= 201103L
#define FHG_ATTRIBUTE_NORETURN [[noreturn]]
#endif
#define FHG_ATTRIBUTE_NORETURN
#endif

#endif
