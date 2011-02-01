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
          env_check_failed,
          startup_failed,
          write_dma_failed,
          read_dma_failed,
          passive_send_failed,
          passive_recv_failed,
          internal_error,
        };
    }

    class code_t
    {
    public:
      explicit code_t (int val, std::string const & txt)
        : m_val (val)
        , m_txt (txt)
      {}

      virtual ~code_t () {}
      const char * name() const { return m_txt.c_str(); };
      int value () const { return m_val; }
    private:
      int m_val;
      std::string m_txt;
    };

#define MK_CODE_T(name)                         \
    struct name : public code_t                 \
    {                                           \
      name ()                                   \
        : code_t (errc::name, #name)            \
      {}                                        \
    }

    MK_CODE_T(success);
    MK_CODE_T(unknown);
    MK_CODE_T(timeout);
    MK_CODE_T(config_error);
    MK_CODE_T(env_check_failed);
    MK_CODE_T(startup_failed);
    MK_CODE_T(write_dma_failed);
    MK_CODE_T(read_dma_failed);
    MK_CODE_T(passive_send_failed);
    MK_CODE_T(passive_recv_failed);
    MK_CODE_T(internal_error);

#undef MK_CODE_T
  }
}

#endif
