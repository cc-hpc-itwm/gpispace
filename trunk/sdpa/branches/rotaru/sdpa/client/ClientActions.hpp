#ifndef SDPA_CLIENT_ACTIONS_HPP
#define SDPA_CLIENT_ACTIONS_HPP 1

namespace sdpa { namespace client {
  class ClientActions {
  public:
    virtual void action_configure() = 0;
    virtual void action_config_ok() = 0;
    virtual void action_config_nok() = 0;
    virtual void action_shutdown() = 0;

    virtual void action_submit(const job_desc_t &) = 0;
    virtual void action_cancel(const job_id_t &) = 0;
    virtual void action_query(const job_id_t &) = 0;
    virtual void action_retrieve(const job_id_t &) = 0;
    virtual void action_delete(const job_id_t &) = 0;
  };
}}

#endif
