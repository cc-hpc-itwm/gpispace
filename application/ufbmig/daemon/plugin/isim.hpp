#ifndef UFBMIG_ISIM_PLUGIN_HPP
#define UFBMIG_ISIM_PLUGIN_HPP

#include <cstdlib>

namespace isim
{
  typedef struct _msg_t msg_t;

  class ISIM
  {
  public:
    virtual ~ISIM () {}

    virtual msg_t *recv (int timeout) = 0;
    virtual int    send (msg_t **msg, int timeout) = 0;
    virtual void   stop () = 0;

    virtual void idle () = 0;
    virtual void busy () = 0;

    virtual msg_t *msg_new     (int type, size_t size = 0) = 0;
    virtual void   msg_destroy (msg_t **) = 0;

    virtual int    msg_type (msg_t *) = 0;
    virtual void  *msg_data (msg_t *) = 0;
    virtual size_t msg_size (msg_t *) = 0;
  };
}

#endif
