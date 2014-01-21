#ifndef SDPA_PLUGINS_LOGC_HPP
#define SDPA_PLUGINS_LOGC_HPP 1

#include <cstddef>

  void fhg_emit_log_message ( const char *filename
                            , const char *function
                            , size_t line
                            , const char * msg
                            );

#endif
