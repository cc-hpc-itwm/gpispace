#ifndef GPI_SPACE_ERROR_HPP
#define GPI_SPACE_ERROR_HPP 1

#include <string>

namespace gpi
{
  namespace error
  {
    namespace errc
    {
      enum errc_t
        {
          success = 0,
          unknown,
          timeout,
          config_error,

          ping_check_failed,
          port_check_failed,
          libs_check_failed,
          ib_check_failed,

          startup_failed,
          kill_procs_failed,
          set_port_failed,
          set_mtu_failed,
          set_network_type_failed,

          send_dma_failed,
          recv_dma_failed,
          write_dma_failed,
          read_dma_failed,
          wait_dma_failed,

          send_passive_failed,
          recv_passive_failed,
          wait_passive_failed,

          open_dma_requests_failed,
          open_passive_requests_failed,

          global_lock_failed,
          global_unlock_failed,

          get_error_vector_failed,

          operation_not_permitted,
          operation_not_implemented,
          internal_error,
        };
    }

    class code_t
    {
    public:
      explicit code_t (int val, std::string const & txt, std::string const & detail)
        : m_val (val)
        , m_txt (txt)
        , m_detail (detail)
      {}

      virtual ~code_t () {}
      const std::string & name() const    { return m_txt; }
      const std::string & detail () const { return m_detail; }
      int value () const { return m_val; }
    private:
      int m_val;
      std::string m_txt;
      std::string m_detail;
    };

#define MK_CODE_T(name)                         \
    struct name : public code_t                 \
    {                                           \
      name (std::string const & detail="")      \
        : code_t (errc::name, #name, detail)    \
      {}                                        \
    }

    MK_CODE_T(success);
    MK_CODE_T(unknown);
    MK_CODE_T(timeout);
    MK_CODE_T(config_error);

    MK_CODE_T(ping_check_failed);
    MK_CODE_T(port_check_failed);
    MK_CODE_T(libs_check_failed);
    MK_CODE_T(ib_check_failed);

    MK_CODE_T(startup_failed);
    MK_CODE_T(kill_procs_failed);

    MK_CODE_T(set_port_failed);
    MK_CODE_T(set_mtu_failed);
    MK_CODE_T(set_network_type_failed);

    MK_CODE_T(send_dma_failed);
    MK_CODE_T(recv_dma_failed);
    MK_CODE_T(write_dma_failed);
    MK_CODE_T(read_dma_failed);
    MK_CODE_T(wait_dma_failed);

    MK_CODE_T(send_passive_failed);
    MK_CODE_T(recv_passive_failed);
    MK_CODE_T(wait_passive_failed);

    MK_CODE_T(open_dma_requests_failed);
    MK_CODE_T(open_passive_requests_failed);

    MK_CODE_T(global_lock_failed);
    MK_CODE_T(global_unlock_failed);

    MK_CODE_T(get_error_vector_failed);

    MK_CODE_T(operation_not_permitted);
    MK_CODE_T(operation_not_implemented);
    MK_CODE_T(internal_error);

#undef MK_CODE_T
  }
}

#endif
