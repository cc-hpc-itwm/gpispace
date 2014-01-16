#ifndef FHG_PLUGIN_DRTS_CB_HANDLERS_HPP_21196ea158024861a992549070ca530c
#define FHG_PLUGIN_DRTS_CB_HANDLERS_HPP_21196ea158024861a992549070ca530c

#ifdef __cplusplus
extern "C" {
#endif
  typedef void (*DrtsCancelHandler)(void*);
  extern int drts_on_cancel_add (DrtsCancelHandler, void*);
  extern void drts_on_cancel_clear ();
  extern int drts_on_cancel ();
#ifdef __cplusplus
}
#endif

#endif
