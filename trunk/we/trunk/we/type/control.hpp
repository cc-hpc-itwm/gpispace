// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_CONTROL_HPP
#define _WE_TYPE_CONTROL_HPP

struct control {};

inline bool operator == (const control &, const control &) { return true; }

#endif
