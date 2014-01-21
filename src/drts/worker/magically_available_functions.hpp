// bernd.loerwald@itwm.fraunhofer.de

#ifndef DRTS_WORKER_MAGICALLY_AVAILABLE_FUNCTIONS_HPP
#define DRTS_WORKER_MAGICALLY_AVAILABLE_FUNCTIONS_HPP

//! \todo These functions should obviously be non-magically-available
//! via drts_context or alike.

#ifdef __cplusplus
extern "C" {
#endif
  typedef void (*DrtsCancelHandler)(void*);
  extern int drts_on_cancel_add (DrtsCancelHandler, void*);
#ifdef __cplusplus
}
#endif

#endif
