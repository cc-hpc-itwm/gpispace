#ifndef GSPC_CTL_LOG_HPP
#define GSPC_CTL_LOG_HPP

namespace gspc
{
  namespace ctl
  {
    enum log_category_t
      {
        DEFAULT
      , APPLICATION
      , NETWORK
      , AGENT
      , ENGINE
      , WORKER
      , SYSTEM
      , USER
      };

    void log ( log_category_t cat
             , const char *tag
             , int level
             , const char *file
             , const char *function
             , int line
             , const char *message
             );
  }
}

#endif
