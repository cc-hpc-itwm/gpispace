#ifndef UFBMIG_ISIM_REACTOR_HPP
#define UFBMIG_ISIM_REACTOR_HPP

namespace isim
{
  typedef struct _msg_t msg_t;

  class IMessageHandler
  {
  public:
    virtual ~IMessageHandler () {}

    virtual msg_t *on_message (msg_t *) = 0;
  };

  class Reactor
  {
  public:
    virtual ~Reactor () {}

    virtual void set_handler (IMessageHandler *) = 0;
  };
}

#endif
