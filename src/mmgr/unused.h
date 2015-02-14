
#pragma once

#ifdef __cplusplus
# define UNUSED(x)
#elif (defined __GNUC__ || defined __ICC)
# define UNUSED(x) __attribute__((unused)) x
#else
# define UNUSED(x) x
#endif
