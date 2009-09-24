#ifndef SDPA_CLIENT_ACTIONS_HPP
#define SDPA_CLIENT_ACTIONS_HPP 1

#include <seda/IEvent.hpp>

namespace sdpa { namespace client {
  class ClientActions {
    public:
      virtual void action_configure(const std::string &) = 0;
      virtual void action_config_ok() = 0;
      virtual void action_config_nok() = 0;
      virtual void action_shutdown() = 0;

      virtual void action_submit(const seda::IEvent::Ptr &) = 0;
      virtual void action_cancel(const seda::IEvent::Ptr &) = 0;
      virtual void action_query(const seda::IEvent::Ptr &) = 0;
      virtual void action_retrieve(const seda::IEvent::Ptr &) = 0;
      virtual void action_delete(const seda::IEvent::Ptr &) = 0;

      virtual void action_store_reply(const seda::IEvent::Ptr &) = 0;
  };
}}

#endif
