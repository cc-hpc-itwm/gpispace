#ifndef UFBMIG_ISIM_PLUGIN_HPP
#define UFBMIG_ISIM_PLUGIN_HPP

#include <cstdlib>

namespace isim
{
  typedef struct _msg_t msg_t;

  msg_t *msg_new     (int type, size_t size = 0);
  void   msg_destroy (msg_t **);

  int    msg_type (msg_t *);
  void  *msg_data (msg_t *);
  size_t msg_size (msg_t *);

  class ISIM
  {
  public:
    virtual ~ISIM () {}

    virtual msg_t *recv (int timeout) = 0;
    virtual int    send (msg_t **msg, int timeout) = 0;

    virtual void idle () = 0;
    virtual void busy () = 0;
  };
}

#endif
