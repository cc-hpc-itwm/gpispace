#ifndef SDPA_COM_MANAGER_HPP
#define SDPA_COM_MANAGER_HPP 1

#include <string>

#include <sdpa/memory.hpp>
#include <sdpa/com/message.hpp>
#include <fhgcom/session_manager.hpp>

namespace sdpa
{
  namespace com
  {
    /*! the manager  class provides the  logical mapping between names  and open
        connections.

        If  a message  should be  sent outwards,  a corresponding  connection is
        looked up internally,  if no connection could be  found, the endpoint is
        looked up  from the global key-value  store. If no entry  could be found
        there, the send fails, otherwise a new connection is opened.

        void send (message_t *m)
           try
           {
              get a message to send
              get_session (m->to())
           }
           catch (...)
           {
              lookup endpoint

              move message to blocked queue

              async_connect ()
                 handle_connect -> move message back to pending
                 handle_error   -> mark message as failed

     */

    template <class Session, class Listener>
    class manager : public fhg::com::session_manager<Session>
    {
    public:
      typedef Session session_type;
      typedef shared_ptr<session_type> session_ptr;

      virtual ~manager ();

      /*! send a message.

        The message is deallocated by the manager.
       */
      template <typename Handler>
      void async_send (message_t * msg, Handler handler);

      /*! recv a message.

        The message must be allocated and deallocated by the user.
       */
      template <typename Handler>
      void async_recv (message_t * msg, Handler handler);
    protected:
      virtual void on_add_hook (session_ptr);
      virtual void on_del_hook (session_ptr);
      virtual void on_data_hook (session_ptr);

    private:
      struct message_info_t
      {
        message_t * msg;
        boost::function<void (const boost::system::error_code & ec)> handler;
      };

      std::list<message_info_t> to_send_;
      std::list<message_info_t> to_recv_;
      std::list<message_t *> incoming_;

      Listener * listener_;
    };
  }
}

#include "manager.tcc"

#endif
