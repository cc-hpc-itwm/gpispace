#include "kvs.hpp"

#include <gspc/kvs/impl/kvs_impl.hpp>
#include <gspc/kvs/impl/kvs_net_frontend.hpp>

namespace gspc
{
  namespace kvs
  {
    struct global_state_t
    {
    public:
      global_state_t ()
        : m_kvs (0)
      {}

      ~global_state_t ()
      {
        delete m_kvs;
      }

      int reset (std::string const &url)
      {
        delete m_kvs;
        m_kvs = create (url);
        return 0;
      }

      int shutdown ()
      {
        delete m_kvs;
        m_kvs = 0;
        return 0;
      }

      api_t & api ()
      {
        assert (m_kvs);
        return *m_kvs;
      }
    private:
      api_t* m_kvs;
    };

    static global_state_t &s_global ()
    {
      static global_state_t s;
      return s;
    }

    api_t *create (std::string const &url)
    {
      if (url.find ("inproc://") == 0)
      {
        return new gspc::kvs::kvs_t (url);
      }
      else
      {
        return new gspc::kvs::kvs_net_frontend_t (url);
      }
    }

    int initialize (std::string const &url)
    {
      return s_global ().reset (url);
    }

    int shutdown ()
    {
      return s_global ().shutdown ();
    }

    api_t &get ()
    {
      return s_global ().api ();
    }
  }
}
