#ifndef GSPC_DRTS_CONTEXT_HPP
#define GSPC_DRTS_CONTEXT_HPP

#include <gspc/drts/context_fwd.hpp>
#include <list>
#include <string>

namespace gspc
{
  namespace drts
  {
    class context
    {
    public:
      context (std::list<std::string> const &worker_list)
        : m_worker_list (worker_list)
      {}

      const std::list<std::string> &worker_list () const { return m_worker_list; }
      std::string worker_to_hostname (std::string const &w) const
      {
        const std::string::size_type host_start = w.find ('-') + 1;
        const std::string::size_type host_end = w.find ('-', host_start);

        return w.substr (host_start, host_end-host_start);
      }
    private:
      std::list<std::string> m_worker_list;
    };
  }
}

#endif
