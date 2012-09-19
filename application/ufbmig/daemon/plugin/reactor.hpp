#ifndef UFBMIG_ISIM_REACTOR_HPP
#define UFBMIG_ISIM_REACTOR_HPP

namespace isim
{
  typedef struct _msg_t msg_t;

  typedef msg_t* (*MessageReactor)(msg_t *);

  class Reactor
  {
  public:
    virtual void set_reactor (MessageReactor r) = 0;
  };
}

#endif
